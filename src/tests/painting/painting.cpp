
#include <Tui/ZPainter_p.h>

#include <string.h>
#include <map>
#include <limits>

#include <../../third-party/catch.hpp>

#include <termpaint.h>

#include "../termpaint_helpers.h"

namespace {

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

    const char* setup;
    const char* cleanup;
    bool optimize;
    termpaint_surface_peek_patch(surface, x, y, &setup, &cleanup, &optimize);
    if (setup || cleanup) {
        cell.setup = setup;
        cell.cleanup = cleanup;
        cell.optimize = optimize;
    }
    return cell;
}

static void checkEmptyPlusSome(termpaint_surface *surface, const std::map<std::tuple<int,int>, Cell> &some,
                               Cell empty = singleWideChar(TERMPAINT_ERASED)) {
    const int width = termpaint_surface_width(surface);
    const int height = termpaint_surface_height(surface);
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

void writeWithAttributes(Kind kind, Tui::ZPainter &painter, int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZPainter::Attributes attr) {
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

TEST_CASE("simple text") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    TermpaintFixture f{80, 6};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = Tui::ZPainterPrivate::createForTesting(f.surface);

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

TEST_CASE("simple text with colors") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    TermpaintFixture f{80, 6};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = Tui::ZPainterPrivate::createForTesting(f.surface);

    writeWithColorsWrapper(kind, painter, 10, 3, "Sample", Tui::ZColor::fromTerminalColor(Tui::TerminalColor::red),
                           Tui::ZColor::fromTerminalColor(Tui::TerminalColor::black));

    checkEmptyPlusSome(f.surface, {
        {{ 10, 3 }, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
        {{ 11, 3 }, singleWideChar("a").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
        {{ 12, 3 }, singleWideChar("m").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
        {{ 13, 3 }, singleWideChar("p").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
        {{ 14, 3 }, singleWideChar("l").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
        {{ 15, 3 }, singleWideChar("e").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK)},
    });
}


TEST_CASE("simple text with attributes") {
    auto kind = GENERATE(ALLKINDS);
    CAPTURE(kind);

    TermpaintFixture f{80, 6};
    termpaint_surface_clear(f.surface, TERMPAINT_DEFAULT_COLOR, TERMPAINT_DEFAULT_COLOR);

    Tui::ZPainter painter = Tui::ZPainterPrivate::createForTesting(f.surface);

    writeWithAttributes(kind, painter, 10, 3, "Sample", Tui::ZColor::fromTerminalColor(Tui::TerminalColor::red),
                        Tui::ZColor::fromTerminalColor(Tui::TerminalColor::black), Tui::ZPainter::Attribute::Bold);

    checkEmptyPlusSome(f.surface, {
        {{ 10, 3 }, singleWideChar("S").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
        {{ 11, 3 }, singleWideChar("a").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
        {{ 12, 3 }, singleWideChar("m").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
        {{ 13, 3 }, singleWideChar("p").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
        {{ 14, 3 }, singleWideChar("l").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
        {{ 15, 3 }, singleWideChar("e").withFg(TERMPAINT_COLOR_RED).withBg(TERMPAINT_COLOR_BLACK).withStyle(TERMPAINT_STYLE_BOLD)},
    });
}
