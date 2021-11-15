// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZPainter_p.h>
#include <Tui/ZImage.h>
#include <Tui/ZImage_p.h>

#include <string.h>
#include <map>
#include <limits>

#include <QRect>
#include <QVector>

#include "../catchwrapper.h"

#include <termpaint.h>

#include "../termpaint_helpers.h"

namespace {

struct TermpaintFixtureImg : public TermpaintFixture {
    TermpaintFixtureImg(int width, int height, bool useImage) : TermpaintFixture(width, height) {
        _useImage = useImage;
        if (useImage) {
            _img = std::make_unique<Tui::ZImage>(Tui::ZImageData::createForTesting(terminal.get(), width, height));
            surface = Tui::ZImageData::get(_img.get())->surface;
        }
    }

    Tui::ZPainter testPainter() {
        if (_useImage) {
            return _img->painter();
        } else {
            return Tui::ZPainterPrivate::createForTesting(surface);
        }
    }

private:
    std::unique_ptr<Tui::ZImage> _img;
    bool _useImage = false;
};

class Cell {
public:
    std::string data;
    uint32_t fg = TERMPAINT_DEFAULT_COLOR;
    uint32_t bg = TERMPAINT_DEFAULT_COLOR;
    uint32_t deco = TERMPAINT_DEFAULT_COLOR;
    int style = 0;
    int width = 1;

    std::string setup;
    std::string cleanup;
    bool optimize = false;

    bool softWrapMarker = false;

public:
    Cell withFg(uint32_t val) {
        auto r = *this;
        r.fg = val;
        return r;
    }

    Cell withBg(uint32_t val) {
        auto r = *this;
        r.bg = val;
        return r;
    }

    Cell withDeco(uint32_t val) {
        auto r = *this;
        r.deco = val;
        return r;
    }

    Cell withStyle(int val) {
        auto r = *this;
        r.style = val;
        return r;
    }

    Cell withPatch(bool o, std::string s, std::string c) {
        auto r = *this;
        r.setup = s;
        r.cleanup = c;
        r.optimize = o;
        return r;
    }

    Cell withSoftWrapMarker() {
        auto r = *this;
        r.softWrapMarker = true;
        return r;
    }
};


Cell singleWideChar(std::string ch) {
    Cell c;
    c.data = ch;
    return c;
}


Cell doubleWideChar(std::string ch) {
    Cell c;
    c.data = ch;
    c.width = 2;
    return c;
}


Cell readCell(termpaint_surface *surface, int x, int y) {
    Cell cell;
    int len, left, right;
    const char *text = termpaint_surface_peek_text(surface, x, y, &len, &left, &right);
    if (left != x) {
        std::terminate();
    }
    cell.data = std::string(text, len);

    cell.width = right - left + 1;

    cell.fg = termpaint_surface_peek_fg_color(surface, x, y);
    cell.bg = termpaint_surface_peek_bg_color(surface, x, y);
    cell.deco = termpaint_surface_peek_deco_color(surface, x, y);
    cell.style = termpaint_surface_peek_style(surface, x, y);
    cell.softWrapMarker = termpaint_surface_peek_softwrap_marker(surface, x, y);

    const char *setup;
    const char *cleanup;
    bool optimize;
    termpaint_surface_peek_patch(surface, x, y, &setup, &cleanup, &optimize);
    if (setup || cleanup) {
        cell.setup = setup;
        cell.cleanup = cleanup;
        cell.optimize = optimize;
    }
    return cell;
}

static void checkEmptyPlusSome(termpaint_surface *surface,
                               const std::map<std::tuple<int, int>, Cell> &some,
                               Cell empty = singleWideChar(TERMPAINT_ERASED)) {
    const int width = termpaint_surface_width(surface);
    const int height = termpaint_surface_height(surface);
    for (auto it = some.begin(); it != some.end(); it++) {
       std::tuple<int, int> key = it->first;
       CHECK(std::get<0>(key) < width);
       CHECK(std::get<0>(key) >= 0);
       CHECK(std::get<1>(key) < height);
       CHECK(std::get<1>(key) >= 0);
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; /* see loop body */ ) {
            Cell cell = readCell(surface, x, y);
            CAPTURE(x);
            CAPTURE(y);
            const Cell *expected;;
            if (some.count({x, y})) {
                expected = &some.at({x, y});
            } else {
                expected = &empty;
            }
            CHECK(cell.bg == expected->bg);
            CHECK(cell.fg == expected->fg);
            CHECK(cell.deco == expected->deco);
            CHECK(cell.data == expected->data);
            CHECK(cell.style == expected->style);
            CHECK(cell.width == expected->width);
            CHECK(cell.setup == expected->setup);
            CHECK(cell.cleanup == expected->cleanup);
            CHECK(cell.optimize == expected->optimize);
            CHECK(cell.softWrapMarker == expected->softWrapMarker);
            x += cell.width;
        }
    }
}

enum Kind {
    KindQString, KindQChar, KindChar16, KindUtf, KindQStringView, KindU16StringView, KindStringView
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindQStringView , KindQStringView
#else
#define MAYBEKindQStringView
#endif

#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
#define MAYBEKindStringView , KindU16StringView, KindStringView
#else
#define MAYBEKindStringView
#endif

#define ALLKINDS KindQString, KindQChar, KindChar16, KindUtf MAYBEKindQStringView MAYBEKindStringView

void writeWithColorsWrapper(Kind kind, Tui::ZPainter &painter, int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg) {
    switch (kind) {
        case KindQString:
            painter.writeWithColors(x, y, string, fg, bg);
            break;
        case KindQChar:
            painter.writeWithColors(x, y, string.data(), string.size(), fg, bg);
            break;
        case KindChar16:
            painter.writeWithColors(x, y, reinterpret_cast<const char16_t*>(string.data()), string.size(), fg, bg);
            break;
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                painter.writeWithColors(x, y, utf8.data(), utf8.size(), fg, bg);
            }
            break;
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            painter.writeWithColors(x, y, QStringView{string}, fg, bg);
#endif
            break;
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            painter.writeWithColors(x, y, std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()), fg, bg);
#endif
            break;
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                painter.writeWithColors(x, y, std::string_view(utf8.data(), utf8.size()), fg, bg);
            }
#endif
            break;
    }

}

void writeWithAttributesWrapper(Kind kind, Tui::ZPainter &painter, int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr) {
    switch (kind) {
        case KindQString:
            painter.writeWithAttributes(x, y, string, fg, bg, attr);
            break;
        case KindQChar:
            painter.writeWithAttributes(x, y, string.data(), string.size(), fg, bg, attr);
            break;
        case KindChar16:
            painter.writeWithAttributes(x, y, reinterpret_cast<const char16_t*>(string.data()), string.size(), fg, bg, attr);
            break;
        case KindUtf:
            {
                QByteArray utf8 = string.toUtf8();
                painter.writeWithAttributes(x, y, utf8.data(), utf8.size(), fg, bg, attr);
            }
            break;
        case KindQStringView:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            painter.writeWithAttributes(x, y, QStringView{string}, fg, bg, attr);
#endif
            break;
        case KindU16StringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            painter.writeWithAttributes(x, y, std::u16string_view(reinterpret_cast<const char16_t*>(string.data()), string.size()), fg, bg, attr);
#endif
            break;
        case KindStringView:
#if defined(__cpp_lib_string_view) && defined(TUIWIDGETS_ABI_FORCE_INLINE)
            {
                QByteArray utf8 = string.toUtf8();
                painter.writeWithAttributes(x, y, std::string_view(utf8.data(), utf8.size()), fg, bg, attr);
            }
#endif
            break;
    }

}

}

TEST_CASE("ZPainter: simple text") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    writeWithColorsWrapper(kind, painter, 10, 3, "Sample", Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());

    checkEmptyPlusSome(f.surface, {
        {{ 10, 3 }, singleWideChar("S")},
        {{ 11, 3 }, singleWideChar("a")},
        {{ 12, 3 }, singleWideChar("m")},
        {{ 13, 3 }, singleWideChar("p")},
        {{ 14, 3 }, singleWideChar("l")},
        {{ 15, 3 }, singleWideChar("e")},
    });
}

TEST_CASE("ZPainter: text with colors") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    struct Testcase {
        Tui::TerminalColor tuifg;
        uint32_t termfg;
        Tui::TerminalColor tuibg;
        uint32_t termbg;
    };

    auto testcase = GENERATE(
                Testcase{Tui::TerminalColor::magenta, TERMPAINT_COLOR_MAGENTA, Tui::TerminalColor::black, TERMPAINT_COLOR_BLACK},
                Testcase{Tui::TerminalColor::yellow, TERMPAINT_COLOR_YELLOW, Tui::TerminalColor::darkGray, TERMPAINT_COLOR_DARK_GREY},
                Testcase{Tui::TerminalColor::brightBlue, TERMPAINT_COLOR_BRIGHT_BLUE, Tui::TerminalColor::lightGray, TERMPAINT_COLOR_LIGHT_GREY},
                Testcase{Tui::TerminalColor::brightGreen, TERMPAINT_COLOR_BRIGHT_GREEN, Tui::TerminalColor::brightWhite, TERMPAINT_COLOR_WHITE},
                Testcase{Tui::TerminalColor::brightCyan, TERMPAINT_COLOR_BRIGHT_CYAN, Tui::TerminalColor::blue, TERMPAINT_COLOR_BLUE},
                Testcase{Tui::TerminalColor::brightRed, TERMPAINT_COLOR_BRIGHT_RED, Tui::TerminalColor::green, TERMPAINT_COLOR_GREEN},
                Testcase{Tui::TerminalColor::brightMagenta, TERMPAINT_COLOR_BRIGHT_MAGENTA, Tui::TerminalColor::cyan, TERMPAINT_COLOR_CYAN},
                Testcase{Tui::TerminalColor::brightYellow, TERMPAINT_COLOR_BRIGHT_YELLOW, Tui::TerminalColor::red, TERMPAINT_COLOR_RED}
    );
    CAPTURE(testcase.tuifg);

    writeWithColorsWrapper(kind, painter, 10, 3, "Sample", Tui::ZColor::fromTerminalColor(testcase.tuifg),
                           Tui::ZColor::fromTerminalColor(testcase.tuibg));

    checkEmptyPlusSome(f.surface, {
        {{ 10, 3 }, singleWideChar("S").withFg(testcase.termfg).withBg(testcase.termbg)},
        {{ 11, 3 }, singleWideChar("a").withFg(testcase.termfg).withBg(testcase.termbg)},
        {{ 12, 3 }, singleWideChar("m").withFg(testcase.termfg).withBg(testcase.termbg)},
        {{ 13, 3 }, singleWideChar("p").withFg(testcase.termfg).withBg(testcase.termbg)},
        {{ 14, 3 }, singleWideChar("l").withFg(testcase.termfg).withBg(testcase.termbg)},
        {{ 15, 3 }, singleWideChar("e").withFg(testcase.termfg).withBg(testcase.termbg)},
    });
}


TEST_CASE("ZPainter: text with attributes") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    struct Testcase {
        Tui::ZTextAttribute attr;
        int style;
    };

    auto testcase = GENERATE(
        Testcase{Tui::ZTextAttribute::Bold, TERMPAINT_STYLE_BOLD},
        Testcase{Tui::ZTextAttribute::Blink, TERMPAINT_STYLE_BLINK},
        Testcase{Tui::ZTextAttribute::Italic, TERMPAINT_STYLE_ITALIC},
        Testcase{Tui::ZTextAttribute::Strike, TERMPAINT_STYLE_STRIKE},
        Testcase{Tui::ZTextAttribute::Inverse, TERMPAINT_STYLE_INVERSE},
        Testcase{Tui::ZTextAttribute::Overline, TERMPAINT_STYLE_OVERLINE},
        Testcase{Tui::ZTextAttribute::Underline, TERMPAINT_STYLE_UNDERLINE},
        Testcase{Tui::ZTextAttribute::UnderlineCurly, TERMPAINT_STYLE_UNDERLINE_CURLY},
        Testcase{Tui::ZTextAttribute::UnderlineDouble, TERMPAINT_STYLE_UNDERLINE_DBL}
    );
    CAPTURE(testcase.attr);

    writeWithAttributesWrapper(kind, painter, 10, 3, "Sample🥚s",
                               Tui::ZColor::fromTerminalColor(Tui::TerminalColor::red),
                               Tui::ZColor::fromTerminalColor(Tui::TerminalColor::black), testcase.attr);

    checkEmptyPlusSome(f.surface, {
        {{ 10, 3 }, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 11, 3 }, singleWideChar("a").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 12, 3 }, singleWideChar("m").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 13, 3 }, singleWideChar("p").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 14, 3 }, singleWideChar("l").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 15, 3 }, singleWideChar("e").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 16, 3 }, doubleWideChar("\U0001F95A").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
        {{ 18, 3 }, singleWideChar("s").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(testcase.style)},
    });

}

TEST_CASE("ZPainter: clear") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    painter.clear(Tui::TerminalColor::black, Tui::TerminalColor::red);
    checkEmptyPlusSome(f.surface, {}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED));

    struct Testcase {
        Tui::ZTextAttribute attr;
        int style;
    };

    auto testcase = GENERATE(
        Testcase{Tui::ZTextAttribute::Bold, TERMPAINT_STYLE_BOLD},
        Testcase{Tui::ZTextAttribute::Blink, TERMPAINT_STYLE_BLINK},
        Testcase{Tui::ZTextAttribute::Italic, TERMPAINT_STYLE_ITALIC},
        Testcase{Tui::ZTextAttribute::Strike, TERMPAINT_STYLE_STRIKE},
        Testcase{Tui::ZTextAttribute::Inverse, TERMPAINT_STYLE_INVERSE},
        Testcase{Tui::ZTextAttribute::Overline, TERMPAINT_STYLE_OVERLINE},
        Testcase{Tui::ZTextAttribute::Underline, TERMPAINT_STYLE_UNDERLINE},
        Testcase{Tui::ZTextAttribute::UnderlineCurly, TERMPAINT_STYLE_UNDERLINE_CURLY},
        Testcase{Tui::ZTextAttribute::UnderlineDouble, TERMPAINT_STYLE_UNDERLINE_DBL}
    );
    CAPTURE(testcase.attr);

    painter.clear(Tui::TerminalColor::black, Tui::TerminalColor::red, testcase.attr);
    checkEmptyPlusSome(f.surface, {}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style));
}

TEST_CASE("ZPainter: clear default") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_COLOR_RED, TERMPAINT_COLOR_BLUE);

    Tui::ZPainter painter = f.testPainter();
    painter.clear(Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    checkEmptyPlusSome(f.surface, {});
}

TEST_CASE("ZPainter: clearWithChar") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    painter.clearWithChar(Tui::TerminalColor::black, Tui::TerminalColor::red, (int)'#', {});
    checkEmptyPlusSome(f.surface, {}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED));

}

TEST_CASE("ZPainter: clearRect") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    painter.clearRect(4, 1, 3, 2, Tui::TerminalColor::black, Tui::TerminalColor::red);
    checkEmptyPlusSome(f.surface, {
                           {{4, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{4, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{6, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{6, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                       });

    struct Testcase {
        Tui::ZTextAttribute attr;
        int style;
    };

    auto testcase = GENERATE(
        Testcase{Tui::ZTextAttribute::Bold, TERMPAINT_STYLE_BOLD},
        Testcase{Tui::ZTextAttribute::Blink, TERMPAINT_STYLE_BLINK},
        Testcase{Tui::ZTextAttribute::Italic, TERMPAINT_STYLE_ITALIC},
        Testcase{Tui::ZTextAttribute::Strike, TERMPAINT_STYLE_STRIKE},
        Testcase{Tui::ZTextAttribute::Inverse, TERMPAINT_STYLE_INVERSE},
        Testcase{Tui::ZTextAttribute::Overline, TERMPAINT_STYLE_OVERLINE},
        Testcase{Tui::ZTextAttribute::Underline, TERMPAINT_STYLE_UNDERLINE},
        Testcase{Tui::ZTextAttribute::UnderlineCurly, TERMPAINT_STYLE_UNDERLINE_CURLY},
        Testcase{Tui::ZTextAttribute::UnderlineDouble, TERMPAINT_STYLE_UNDERLINE_DBL}
    );
    CAPTURE(testcase.attr);

    painter.clearRect(4, 1, 3, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, testcase.attr);
    checkEmptyPlusSome(f.surface, {
                           {{4, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)},
                           {{4, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)},
                           {{5, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)},
                           {{5, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)},
                           {{6, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)},
                           {{6, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED).withStyle(testcase.style)}
                       });

}

TEST_CASE("ZPainter: clearRectWithChar") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    painter.clearRectWithChar(1, 1, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, (int)'#', {});
    checkEmptyPlusSome(f.surface, {
                           {{1, 1}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{1, 2}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{2, 1}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{2, 2}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                       });
}

TEST_CASE("ZPainter: clearRectWithChar U+1D160") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();

    painter.clearRectWithChar(1, 1, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, (int)0x1D160, {});
    checkEmptyPlusSome(f.surface, {
                           {{1, 1}, singleWideChar("\U0001D160").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{1, 2}, singleWideChar("\U0001D160").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{2, 1}, singleWideChar("\U0001D160").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{2, 2}, singleWideChar("\U0001D160").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                       });
}

TEST_CASE("ZPainter: translateAndClip") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);
    bool useQRect = GENERATE(false, true);
    CAPTURE(useQRect);

    Tui::ZPainter painterUnclipped = f.testPainter();
    Tui::ZPainter painter1x1 = useQRect ? painterUnclipped.translateAndClip({2, 3, 1, 1})
                                        : painterUnclipped.translateAndClip(2, 3, 1, 1);

    Tui::ZPainter painter4x4 = useQRect ? painterUnclipped.translateAndClip({20, 1, 4, 4})
                                        : painterUnclipped.translateAndClip(20, 1, 4, 4);

    Tui::ZPainter painterOffset = useQRect ? painter4x4.translateAndClip({-2, -1, 4, 3})
                                           : painter4x4.translateAndClip(-2, -1, 4, 3);

    SECTION("setForeground, setBackground and setSoftwrap outside") {
        painter1x1.setSoftwrapMarker(1, 1);
        painter1x1.setBackground(2, 2, Tui::TerminalColor::green);
        painter1x1.setForeground(3, 1, Tui::TerminalColor::yellow);
        checkEmptyPlusSome(f.surface, {});
    }

    SECTION("clear") {
        painter1x1.clear(Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clear with offset") {
        painterOffset.clear(Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{20, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{20, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{21, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{21, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                        });
    }


    SECTION("clearWithChar") {
        painter1x1.clearWithChar(Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearWithChar with offset") {
        painterOffset.clearWithChar(Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{20, 1}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{20, 2}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{21, 1}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{21, 2}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                        });
    }


    SECTION("clearRect") {
        painter1x1.clearRect(0, 0, 10, 10, Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRect-negativ") {
        painter1x1.clearRect(-10, -10, 11, 11, Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRect-inside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({2, 1, 4, 5})
                                         : painterUnclipped.translateAndClip(2, 1, 4, 5);
        painter.clearRect(2, 2, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{4, 3}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 3}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{4, 4}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 4}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRect-outside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({2, 1, 4, 5})
                                         : painterUnclipped.translateAndClip(2, 1, 4, 5);
        painter.clearRect(10, 10, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {});
    }

    SECTION("clearRect with offset") {
        painterOffset.clearRect(0, 0, 3, 2, Tui::TerminalColor::black, Tui::TerminalColor::red);
        checkEmptyPlusSome(f.surface, {
                           {{20, 1}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }


    SECTION("clearRectWithChar") {
        painter1x1.clearRectWithChar(0, 0, 10, 10, Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRectWithChar-inside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({2, 1, 4, 5})
                                         : painterUnclipped.translateAndClip(2, 1, 4, 5);
        painter.clearRectWithChar(2, 2, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{4, 3}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 3}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{4, 4}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)},
                           {{5, 4}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRectWithChar-outside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({2, 1, 4, 5})
                                         : painterUnclipped.translateAndClip(2, 1, 4, 5);
        painter.clearRectWithChar(10, 10, 2, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {});
    }

    SECTION("clearRectWithChar-negativ") {
        painter1x1.clearRectWithChar(-10, -10, 11, 11, Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }

    SECTION("clearRectWithChar with offset") {
        painterOffset.clearRectWithChar(0, 0, 3, 2, Tui::TerminalColor::black, Tui::TerminalColor::red, '#');
        checkEmptyPlusSome(f.surface, {
                           {{20, 1}, singleWideChar("#").withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED)}
                        });
    }


    SECTION("clearSoftwrapMarker") {
        std::map<std::tuple<int, int>, Cell> expected;
        for (int x = -2; x <= 3; x++) {
            for (int y = -2; y <= 3; y++) {
                painterUnclipped.setSoftwrapMarker(x, y);
                if (x >= 0 && y >= 0) {
                    expected[{x, y}] = singleWideChar(TERMPAINT_ERASED).withSoftWrapMarker();
                }
            }
        }
        checkEmptyPlusSome(f.surface, expected);

        for (int x = -2; x <= 3; x++) {
            for (int y = -2; y <= 3; y++) {
                painter1x1.clearSoftwrapMarker(x, y);
            }
        }
        expected[{2, 3}] = singleWideChar(TERMPAINT_ERASED);
        checkEmptyPlusSome(f.surface, expected);
    }

    SECTION("clearSoftwrapMarker with offset") {
        painterUnclipped.setSoftwrapMarker(21, 2);
        painterOffset.clearSoftwrapMarker(3, 2);
        checkEmptyPlusSome(f.surface, {});
    }

    SECTION("setSoftwrapMarker") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({1, 2, 3, 3})
                                         : painterUnclipped.translateAndClip(1, 2, 3, 3);
        std::map<std::tuple<int, int>, Cell> expected;
        for (int y = -2; y <= 3; y++) {
            for (int x = -2; x <= 3; x++) {
                painter.setSoftwrapMarker(x, y);
                if (x >= 0 && y >= 0) {
                    if (x < 3 && y < 3) {
                        expected[{x + 1, y + 2}] = singleWideChar(TERMPAINT_ERASED).withSoftWrapMarker();
                    }
                }
            }
        }
        checkEmptyPlusSome(f.surface, expected);
    }

    SECTION("setSoftwrapMarker with offset") {
        painterOffset.setSoftwrapMarker(3, 2);
        checkEmptyPlusSome(f.surface, {
                           {{21, 2}, singleWideChar(TERMPAINT_ERASED).withSoftWrapMarker()}
                        });
    }


    SECTION("setForeground") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({1, 2, 3, 3})
                                         : painterUnclipped.translateAndClip(1, 2, 3, 3);
        std::map<std::tuple<int, int>, Cell> expected;
        for (int y = -2; y <= 3; y++) {
            for (int x = -2; x <= 3; x++) {
                painter.setForeground(x, y, Tui::TerminalColor::red);
                if (x >= 0 && y >= 0) {
                    if (x < 3 && y < 3) {
                        expected[{x + 1, y + 2}] = singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_RED);
                    }
                }
            }
        }
        checkEmptyPlusSome(f.surface, expected);
    }

    SECTION("setForeground with offset") {
        painterOffset.setForeground(3, 2, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{21, 2}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_GREEN)}
                        });
    }


    SECTION("setBackground") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({1, 2, 3, 3})
                                         : painterUnclipped.translateAndClip(1, 2, 3, 3);
        std::map<std::tuple<int, int>, Cell> expected;
        for (int y = -2; y <= 3; y++) {
            for (int x = -2; x <= 3; x++) {
                painter.setBackground(x, y, Tui::TerminalColor::red);
                if (x >= 0 && y >= 0) {
                    if (x < 3 && y < 3) {
                        expected[{x + 1, y + 2}] = singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_RED);
                    }
                }
            }
        }
        checkEmptyPlusSome(f.surface, expected);
    }

    SECTION("setBackground with offset") {
        painterOffset.setBackground(3, 2, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{21, 2}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_GREEN)}
                        });
    }


    SECTION("drawImage") {
        Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 10, 10);
        image.painter().clear(Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        image.painter().setBackground(2, 2, Tui::TerminalColor::blue);
        painter1x1.drawImage(-2, -2, image);
        checkEmptyPlusSome(f.surface, {
                               {{2, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_BLUE).withFg(TERMPAINT_COLOR_CYAN)}
                           });
    }

    SECTION("drawImage-inside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({1, 2, 20, 3})
                                         : painterUnclipped.translateAndClip(1, 2, 20, 3);
        Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 5, 1);
        image.painter().clear(Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        image.painter().setBackground(2, 0, Tui::TerminalColor::blue);
        image.painter().setBackground(3, 0, Tui::TerminalColor::blue);
        image.painter().setBackground(4, 0, Tui::TerminalColor::blue);
        painter.drawImage(2, 1, image);
        checkEmptyPlusSome(f.surface, {
                               {{3, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_GREEN).withFg(TERMPAINT_COLOR_CYAN)},
                               {{4, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_GREEN).withFg(TERMPAINT_COLOR_CYAN)},
                               {{5, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_BLUE).withFg(TERMPAINT_COLOR_CYAN)},
                               {{6, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_BLUE).withFg(TERMPAINT_COLOR_CYAN)},
                               {{7, 3}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_BLUE).withFg(TERMPAINT_COLOR_CYAN)}
                           });
    }

    SECTION("drawImage-outside") {
        Tui::ZPainter painter = useQRect ? painterUnclipped.translateAndClip({1, 2, 10, 3})
                                         : painterUnclipped.translateAndClip(1, 2, 10, 3);
        Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 10, 10);
        image.painter().clear(Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        image.painter().setBackground(2, 2, Tui::TerminalColor::blue);
        painter.drawImage(12, 2, image);
        checkEmptyPlusSome(f.surface, {});
    }

    SECTION("drawImage with offset") {
        Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 10, 4);
        image.painter().clear(Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        image.painter().writeWithColors(0, 0, "abcdefghij", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 1, "ABCDEFGHJI", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 2, "0123456789", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 3, "xxxxxxxxxx", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);

        painterOffset.drawImage(0, 0, image);

        checkEmptyPlusSome(f.surface, {
                               {{20, 1}, singleWideChar("C").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{21, 1}, singleWideChar("D").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{20, 2}, singleWideChar("2").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{21, 2}, singleWideChar("3").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                        });
    }

    SECTION("drawImage with offset-2-1") {
        Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 10, 4);
        image.painter().clear(Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        image.painter().writeWithColors(0, 0, "abcdefghij", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 1, "ABCDEFGHJI", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 2, "0123456789", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);
        image.painter().writeWithColors(0, 3, "xxxxxxxxxx", Tui::TerminalColor::blue, Tui::TerminalColor::brightMagenta);

        painterOffset.drawImage(2, 1, image);

        checkEmptyPlusSome(f.surface, {
                               {{20, 1}, singleWideChar("a").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{21, 1}, singleWideChar("b").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{20, 2}, singleWideChar("A").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                               {{21, 2}, singleWideChar("B").withFg(TERMPAINT_COLOR_BLUE).withBg(TERMPAINT_COLOR_BRIGHT_MAGENTA)},
                        });
    }

}

TEST_CASE("ZPainter: translateAndClip private") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    SECTION("extend") {
        // translateAndClip can't make the clipping area extend
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({-1, -1, 1000, 1000});
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 80);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 24);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == -1);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == -1);
    }
    SECTION("nestedclip") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({2, 4, 78, 20}).translateAndClip(6, 8, 71, 12);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 8);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 12);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 71);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 12);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("top-left") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({2, 5, 1000, 1000});
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 2);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 5);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 78);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 19);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("bottom-right") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({0, 0, 79, 23});
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 79);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 23);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("top-left-bottom-right") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({2, 5, 77, 18});
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 2);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 5);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 77);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 18);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("negativ-size") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({10, 10, -2, -2});
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 10);
    }
    SECTION("negativ-size-offset") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({-1, -1, -2, -2}).translateAndClip(0, 0, 10, 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == -1);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == -1);
    }
    SECTION("negativ-offset") {
        // negative translation reduces size of clipping area and adjusts offset
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20}).translateAndClip(-1, -1, 3, 3);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 15);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 2);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 2);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == -1);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == -1);
    }
    SECTION("negativ-offset-2times") {
        // negative translation reduces size of clipping area and adjusts offset
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20})
                .translateAndClip({-2, -2, 3, 1}).translateAndClip(-2, -2, 3, 1);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 0);
    }
    SECTION("negativ-balanced") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20})
                .translateAndClip({-2, -2, 20, 20}).translateAndClip(2, 2, 10, 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 15);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("negativ-less") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20})
                .translateAndClip({-2, -2, 20, 20}).translateAndClip(1, 1, 10, 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 15);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 9);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 9);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == -1);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == -1);
    }
    SECTION("negativ-more") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20})
                .translateAndClip({-2, -2, 20, 20}).translateAndClip(3, 3, 10, 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 16);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 11);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
    SECTION("negativ-more-clip") {
        TermpaintFixtureImg f{80, 24, useImage};
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({15, 10, 26, 20})
                .translateAndClip({-2, -2, 12, 12}).translateAndClip(3, 3, 10, 10);
        CHECK(Tui::ZPainterPrivate::get(&painter)->x == 16);
        CHECK(Tui::ZPainterPrivate::get(&painter)->y == 11);
        CHECK(Tui::ZPainterPrivate::get(&painter)->width == 9);
        CHECK(Tui::ZPainterPrivate::get(&painter)->height == 9);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetX == 0);
        CHECK(Tui::ZPainterPrivate::get(&painter)->offsetY == 0);
    }
}


TEST_CASE("ZPainter: SoftwrapMarker") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.setSoftwrapMarker(79, 1);
    checkEmptyPlusSome(f.surface, {
                           {{79, 1}, singleWideChar(TERMPAINT_ERASED).withSoftWrapMarker()}
                       });
}

TEST_CASE("ZPainter: SoftwrapMarker clear") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.setSoftwrapMarker(79, 1);
    painter.clear(Tui::TerminalColor::black, Tui::TerminalColor::red);
    checkEmptyPlusSome(f.surface, {}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_BLACK).withBg(TERMPAINT_COLOR_RED));
}

TEST_CASE("ZPainter: clearSoftwrapMarker") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.setSoftwrapMarker(79, 1);
    painter.clearSoftwrapMarker(79, 1);
    checkEmptyPlusSome(f.surface, {
                           {{79, 1}, singleWideChar(TERMPAINT_ERASED)}
                       });
}

TEST_CASE("ZPainter: drawImage") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    Tui::ZImage image = Tui::ZImageData::createForTesting(f.terminal, 1, 1);
    image.painter().setBackground(0, 0, Tui::TerminalColor::blue);
    painter.drawImage(0, 0, image);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_BLUE)}
                       });

    image = Tui::ZImageData::createForTesting(f.terminal, 3, 2);
    image.painter().writeWithColors(0, 0, "ASD", Tui::TerminalColor::red, Tui::TerminalColor::cyan);
    image.painter().writeWithColors(0, 1, "123", Tui::TerminalColor::red, Tui::TerminalColor::cyan);
    painter.drawImage(0, 0, image, 0, 0, -1, -1);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar("A").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{1, 0}, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{2, 0}, singleWideChar("D").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{0, 1}, singleWideChar("1").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{1, 1}, singleWideChar("2").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{2, 1}, singleWideChar("3").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)}
                       });

    painter.clear(Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    painter.drawImage(2, 3, image);
    checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("A").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{3, 3}, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{4, 3}, singleWideChar("D").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{2, 4}, singleWideChar("1").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{3, 4}, singleWideChar("2").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{4, 4}, singleWideChar("3").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)}
                       });

    painter.clear(Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    painter.drawImage(0, 0, image, 2, 1);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar("3").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)}
                       });

    painter.clear(Tui::ZColor::defaultColor(), Tui::ZColor::defaultColor());
    painter.drawImage(0, 0, image, 0, 0, 2, 1);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar("A").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                           {{1, 0}, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_CYAN)},
                       });

}

TEST_CASE("ZPainter: setForeground") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.setForeground(0, 0, Tui::TerminalColor::brightWhite);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar(TERMPAINT_ERASED).withFg(TERMPAINT_COLOR_WHITE)}
                       });
}
TEST_CASE("ZPainter: write text with setForeground") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.writeWithAttributes(0, 0, "B", Tui::TerminalColor::red, Tui::TerminalColor::blue, Tui::ZTextAttribute::Bold);
    painter.setForeground(0, 0, Tui::TerminalColor::brightWhite);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar("B").withFg(TERMPAINT_COLOR_WHITE).withBg(TERMPAINT_COLOR_BLUE).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
}

TEST_CASE("ZPainter: setBackground") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.setBackground(0, 0, Tui::TerminalColor::red);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar(TERMPAINT_ERASED).withBg(TERMPAINT_COLOR_RED)}
                       });
}

TEST_CASE("ZPainter: write text with setBackground") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);
    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = f.testPainter();
    painter.writeWithAttributes(0, 0, "B", Tui::TerminalColor::cyan, Tui::TerminalColor::blue, Tui::ZTextAttribute::Bold);
    painter.setBackground(0, 0, Tui::TerminalColor::red);
    checkEmptyPlusSome(f.surface, {
                           {{0, 0}, singleWideChar("B").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_RED).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
}

TEST_CASE("ZPainter: translateAndClip write") {
    bool useImage = GENERATE(false, true);
    CAPTURE(useImage);

    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    TermpaintFixtureImg f{80, 6, useImage};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    SECTION("writeWithColors") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 1, 1});

        writeWithColorsWrapper(kind, painter, 0, -1, "blub", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {});

        for (int y = -2; y <= 3; y++) {
            for (int x = -2; x <= 3; x++) {
                if(y == 0) continue;
                INFO("x, y: " << x << y);
                writeWithColorsWrapper(kind, painter, x, y, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
                checkEmptyPlusSome(f.surface, {});
            }
        }

        writeWithColorsWrapper(kind, painter, -2, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("2").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                       });
        writeWithColorsWrapper(kind, painter, -1, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("1").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                       });
        writeWithColorsWrapper(kind, painter, 0, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                       });
        writeWithColorsWrapper(kind, painter, 2, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                       });
    }
    SECTION("writeWithColors-inside") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({1, 2, 21, 11});
        writeWithColorsWrapper(kind, painter, 4, 3, "01234", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{5, 5}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{6, 5}, singleWideChar("1").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{7, 5}, singleWideChar("2").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{8, 5}, singleWideChar("3").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{9, 5}, singleWideChar("4").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                           });
    }

    SECTION("writeWithColors-utf8") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 2, 1});
        writeWithColorsWrapper(kind, painter, 0, 0, "😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{2, 3}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                           });
    }
    SECTION("writeWithColors-utf8-cut") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 1, 1});
        writeWithColorsWrapper(kind, painter, 0, 0, "😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{2, 3}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                           });
    }

    SECTION("writeWithAttributes") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 1, 1});

        for (int y = -2; y <= 3; y++) {
            for (int x = -2; x <= 3; x++) {
                if(y == 0) continue;
                INFO("x, y: " << x << y);
                writeWithAttributesWrapper(kind, painter, x, y, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
                checkEmptyPlusSome(f.surface, {});
            }
        }

        writeWithAttributesWrapper(kind, painter, -2, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("2").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
        writeWithAttributesWrapper(kind, painter, -1, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("1").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
        writeWithAttributesWrapper(kind, painter, 0, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
        writeWithAttributesWrapper(kind, painter, 2, 0, "0123456789", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
        checkEmptyPlusSome(f.surface, {
                           {{2, 3}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                       });
    }

    SECTION("writeWithAttributes-inside") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({1, 2, 21, 11});
        writeWithAttributesWrapper(kind, painter, 4, 3, "01234", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);
        checkEmptyPlusSome(f.surface, {
                               {{5, 5}, singleWideChar("0").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)},
                               {{6, 5}, singleWideChar("1").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)},
                               {{7, 5}, singleWideChar("2").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)},
                               {{8, 5}, singleWideChar("3").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)},
                               {{9, 5}, singleWideChar("4").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                           });
    }


    SECTION("writeWithAttributes-utf8") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 2, 1});
        writeWithAttributesWrapper(kind, painter, 0, 0, "😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);

        checkEmptyPlusSome(f.surface, {
                               {{2, 3}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                           });
    }
    SECTION("writeWithAttributes-utf8-cut") {
        Tui::ZPainter painter = f.testPainter().translateAndClip({2, 3, 1, 1});
        writeWithAttributesWrapper(kind, painter, 0, 0, "😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green, Tui::ZTextAttribute::Bold);

        checkEmptyPlusSome(f.surface, {
                               {{2, 3}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN).withStyle(TERMPAINT_STYLE_BOLD)}
                           });
    }

    SECTION("writeWithColors-utf8-negativ-short") {
        // translateAndClip with negativ offsets displaces written text as if negative coordinates have
        // been used when calling writeText...
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({-1, 0, 3, 1});
        writeWithColorsWrapper(kind, painter, 0, 0, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{0, 0}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{1, 0}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                           });
    }

    SECTION("writeWithColors-utf8-negativ") {
        // translateAndClip with negativ offsets displaces written text as if negative coordinates have
        // been used when calling writeText...
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({-1, 0, 4, 1});
        writeWithColorsWrapper(kind, painter, 0, 0, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{0, 0}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{1, 0}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                           });
    }

    SECTION("writeWithColors-double-translateAndClip-negativ") {
        // translateAndClip with negativ offsets displaces written text as if negative coordinates have
        // been used when calling writeText...
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({3, 0, 30, 1}).translateAndClip(-1, 0, 30, 1);
        writeWithColorsWrapper(kind, painter, 0, 0, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{3, 0}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{4, 0}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{5, 0}, singleWideChar(" ").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                           });
    }

    SECTION("writeWithColors-double-translateAndClip") {
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({3, 1, 30, 5}).translateAndClip(3, 2, 30, 1);
        writeWithColorsWrapper(kind, painter, 0, 0, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{6, 3}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{8, 3}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                           });
    }

    SECTION("writeWithColors-translateAndClip-negativ-y") {
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({3, -2, 30, 5});
        writeWithColorsWrapper(kind, painter, 0, 0, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                           });
    }

    SECTION("writeWithColors-translateAndClip-negativ-y-tight") {
        Tui::ZPainter painterUnclipped = f.testPainter();
        Tui::ZPainter painter = painterUnclipped.translateAndClip({3, -2, 30, 3});
        writeWithColorsWrapper(kind, painter, 0, 2, "😎😎", Tui::TerminalColor::cyan, Tui::TerminalColor::green);
        checkEmptyPlusSome(f.surface, {
                               {{3, 0}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)},
                               {{5, 0}, doubleWideChar("😎").withFg(TERMPAINT_COLOR_CYAN).withBg(TERMPAINT_COLOR_GREEN)}
                           });
    }
}
