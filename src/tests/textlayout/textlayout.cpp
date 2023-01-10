// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTextLayout.h>
#include <Tui/ZTextOption.h>
#include <Tui/ZFormatRange.h>

#include <QTextBoundaryFinder>

#include <Tui/ZWidget.h>
#include <Tui/ZTextMetrics.h>

#include "../Testhelper.h"

#define U8(x) reinterpret_cast<const char*>(u8##x)

namespace {
    class Widget : public Tui::ZWidget {
    public:
        using Tui::ZWidget::ZWidget;

        Tui::ZTextLayout *lay = nullptr;
        int cursorPos = -1;
        int cursorPos2 = -1;

        void paintEvent(Tui::ZPaintEvent *event) override {
            lay->showCursor(*event->painter(), {1, 1}, cursorPos);
            if (cursorPos2 != -1) {
                lay->showCursor(*event->painter(), {1, 1}, cursorPos2);
            }
        }
    };
}


TEST_CASE("textlayout", "") {
    Testhelper t("textlayout", "textlayout", 32, 5);

    // The initialisation must not break down.
    SECTION("initialisation") {
        delete new Tui::ZTextLayout(t.terminal->textMetrics());
        delete new Tui::ZTextLayout(t.terminal->textMetrics(), "");
    }

    std::unique_ptr<Tui::ZTextLayout> layout = std::make_unique<Tui::ZTextLayout>(t.terminal->textMetrics());
    SECTION("defaults") {
        Tui::ZTextOption to = layout->textOption();
        CHECK(to.flags() == Tui::ZTextOption::Flag{});
        CHECK(to.wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
        CHECK(to.tabs().size() == 0);
        CHECK(layout->boundingRect() == QRect{0, 0, 0, 0});

        CHECK(layout->lineCount() == 0);
        CHECK(layout->maximumWidth() == 0);

        Tui::ZTextLineRef tlr2 = layout->lineAt(0);
        CHECK(tlr2.textStart() == 0);
        CHECK(tlr2.textLength() == 0);
        CHECK(tlr2.lineNumber() == -1);
        CHECK(tlr2.x() == 0);
        CHECK(tlr2.y() == 0);
        CHECK(tlr2.width() == 0);
        CHECK(tlr2.height() == 1);
        CHECK(tlr2.position() == QPoint{0, 0});
        CHECK(tlr2.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr2.isValid() == false);

        Tui::ZTextLineRef tlr1 = layout->createLine();
        CHECK(tlr1.textStart() == -1);
        CHECK(tlr1.textLength() == 0);
        CHECK(tlr1.lineNumber() == 0);
        CHECK(tlr1.x() == 0);
        CHECK(tlr1.y() == 0);
        CHECK(tlr1.width() == 0);
        CHECK(tlr1.height() == 1);
        CHECK(tlr1.position() == QPoint{0, 0});
        CHECK(tlr1.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr1.isValid() == true);

        tlr2 = layout->lineAt(0);
        CHECK(tlr2.textStart() == -1);
        CHECK(tlr2.textLength() == 0);
        CHECK(tlr2.lineNumber() == 0);
        CHECK(tlr2.x() == 0);
        CHECK(tlr2.y() == 0);
        CHECK(tlr2.width() == 0);
        CHECK(tlr2.height() == 1);
        CHECK(tlr2.position() == QPoint{0, 0});
        CHECK(tlr2.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr2.isValid() == true);

        Tui::ZTextLineRef tlr3 = layout->lineForTextPosition(0);
        CHECK(tlr3.textStart() == -1);
        CHECK(tlr3.textLength() == 0);
        CHECK(tlr3.lineNumber() == 0);
        CHECK(tlr3.x() == 0);
        CHECK(tlr3.y() == 0);
        CHECK(tlr3.width() == 0);
        CHECK(tlr3.height() == 1);
        CHECK(tlr3.position() == QPoint{0, 0});
        CHECK(tlr3.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr3.isValid() == true);

        CHECK(layout->text() == "");
        CHECK(layout->previousCursorPosition(0) == 0);
        CHECK(layout->nextCursorPosition(0) == 0);
        CHECK(layout->isValidCursorPosition(0) == true);

        tlr3 = layout->lineForTextPosition(-1);
        CHECK(tlr3.textStart() == 0);
        CHECK(tlr3.textLength() == 0);
        CHECK(tlr3.lineNumber() == -1);
        CHECK(tlr3.x() == 0);
        CHECK(tlr3.y() == 0);
        CHECK(tlr3.width() == 0);
        CHECK(tlr3.height() == 1);
        CHECK(tlr3.position() == QPoint{0, 0});
        CHECK(tlr3.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr3.isValid() == false);
    }


    SECTION("defaults-with-text") {
        layout = std::make_unique<Tui::ZTextLayout>(t.terminal->textMetrics(), "hello");
        CHECK(layout->text() == "hello");
        Tui::ZTextOption to = layout->textOption();
        CHECK(to.flags() == Tui::ZTextOption::Flag{});
        CHECK(to.wrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
    }

    SECTION("setText") {
        layout->setText("setText");
        CHECK(layout->text() == "setText");
    }


    SECTION("setTextOption") {
        Tui::ZTextOption to;
        to.setTabStopDistance(42);
        layout->setTextOption(to);
        CHECK(layout->textOption().tabStopDistance() == 42);
    }

    Tui::ZImage zi{t.terminal.get(), 32, 5};
    Tui::ZTextStyle styleBWhiteBlue{Tui::Colors::brightWhite, Tui::Colors::blue};
    Tui::ZTextStyle styleRedGreenBold{Tui::Colors::red, Tui::Colors::green, Tui::ZTextAttribute::Bold};
    Tui::ZTextStyle styleCyanYellowItalic{Tui::Colors::cyan, Tui::Colors::yellow, Tui::ZTextAttribute::Italic};
    Tui::ZTextStyle styleMagentaDGrayStrike{Tui::Colors::magenta, Tui::Colors::darkGray, Tui::ZTextAttribute::Strike};
    zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

    SECTION("textLineRef-invalid") {
        Tui::ZTextLineRef tlr1;
        CHECK(tlr1.isValid() == false);
        CHECK(tlr1.width() == 0);
        CHECK(tlr1.height() == 1);
        CHECK(tlr1.x() == 0);
        CHECK(tlr1.y() == 0);
        CHECK(tlr1.rect() == QRect{0, 0, 0, 1});
        CHECK(tlr1.position() == QPoint{0, 0});
        CHECK(tlr1.lineNumber() == -1);
        CHECK(tlr1.textStart() == 0);
        CHECK(tlr1.textLength() == 0);
        CHECK(tlr1.cursorToX(0, Tui::ZTextLayout::Leading) == 0);
        CHECK(tlr1.cursorToX(0, Tui::ZTextLayout::Trailing) == 0);
        CHECK(tlr1.cursorToX(-1, Tui::ZTextLayout::Leading) == 0);
        CHECK(tlr1.cursorToX(50, Tui::ZTextLayout::Leading) == 0);
        CHECK(tlr1.cursorToX(-1, Tui::ZTextLayout::Trailing) == 0);
        CHECK(tlr1.cursorToX(50, Tui::ZTextLayout::Trailing) == 0);

        int f = 50;
        CHECK(tlr1.cursorToX(&f, Tui::ZTextLayout::Leading) == 0);
        CHECK(f == 0);
        f = 50;
        CHECK(tlr1.cursorToX(&f, Tui::ZTextLayout::Trailing) == 0);
        CHECK(f == 0);

        CHECK(tlr1.xToCursor(-1) == 0);
        CHECK(tlr1.xToCursor(50) == 0);
        tlr1.draw(zi.painter(), {1, 1}, styleBWhiteBlue, styleBWhiteBlue, {});
        tlr1.setPosition({10, 10});
        tlr1.setLineWidth(10);
    }

    // double width, 1 code unit: あ
    // double width, 2 code unit: 😎
    // isolated non spacing mark: \u0308
    // nbsp + non spacing mark: \u00a0\u0308
    // cluster: a\u0308, a\u0308\u0304
    // low surrogate isolated 0xdd00 bis 0xdfff

    SECTION("doLayout-empty") {
        layout->doLayout(15);
        CHECK(layout->lineAt(0).isValid() == true);
        CHECK(layout->lineAt(0).width() == 0);
        CHECK(layout->lineAt(0).height() == 1);
        CHECK(layout->lineAt(0).cursorToX(0, Tui::ZTextLayout::Edge::Leading) == 0);
        CHECK(layout->lineAt(0).cursorToX(0, Tui::ZTextLayout::Edge::Trailing) == 0);
    }

    SECTION("linebreak-in-text") {
        // Check that \n at the end of a line that fills the whole layout width does not trigger an additional line break
        layout->setText("ha\tllo \n"
                        "123456789012345\n"
                        "ABC");
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 3);
        t.compare(zi);

        // operator=
        Tui::ZTextLayout layout2 = *layout;
        layout2.draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout2.lineCount() == 3);
        t.compare(zi);

        // copy constructor
        Tui::ZTextLayout layout3 = *layout;
        layout3.draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout3.lineCount() == 3);
        t.compare(zi);
    }

    SECTION("linebreak-last-character") {
        layout->setText("ha\tllo \n");
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 1);
        t.compare(zi);
    }

    SECTION("tab-1") {
        // Negative tab stop distance must not crash or loop infinitivly
        Tui::ZTextOption to;
        to.setTabStopDistance(-1);
        layout->setTextOption(to);
        layout->setText("\t");
        layout->doLayout(7);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
    }

    SECTION("tab-0") {
        // Zero tab stop distance must not crash or loop infinitivly
        Tui::ZTextOption to;
        to.setTabStopDistance(0);
        layout->setTextOption(to);
        layout->setText("\t");
        layout->doLayout(7);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
    }

    SECTION("tab-65000") {
        // Tab stop distance larger than layout width must not crash or loop infinitivly
        Tui::ZTextOption to;
        to.setTabStopDistance(65000);
        layout->setTextOption(to);
        layout->setText("\t");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
    }

    SECTION("tab-size4-lineref") {
        Tui::ZTextOption to;
        bool colorTabs = GENERATE(false, true);
        if (colorTabs) {
            to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        }
        to.setTabStopDistance(4);

        layout->setTextOption(to);
        layout->setText("1\t"
                        "22\t"
                        "333"
                        "\t");
        layout->doLayout(3);
        REQUIRE(layout->lineCount() == 4);
        CHECK(layout->lineAt(-1).x() == 0); // invalid
        CHECK(layout->lineAt(-1).y() == 0); // invalid
        CHECK(layout->lineAt(-1).height() == 1); // invalid
        CHECK(layout->lineAt(-1).width() == 0); // invalid
        CHECK(layout->lineAt(-1).position() == QPoint({0, 0}));
        CHECK(layout->lineAt(-1).rect() == QRect({0, 0, 0, 1}));

        CHECK(layout->lineAt(0).x() == 0);
        CHECK(layout->lineAt(0).y() == 0);
        CHECK(layout->lineAt(0).width() == 3);
        CHECK(layout->lineAt(0).height() == 1);
        CHECK(layout->lineAt(0).position() == QPoint({0, 0}));
        CHECK(layout->lineAt(0).rect() == QRect({0, 0, 3, 1}));

        CHECK(layout->lineAt(1).x() == 0);
        CHECK(layout->lineAt(1).y() == 1);
        CHECK(layout->lineAt(1).width() == 3);
        CHECK(layout->lineAt(1).height() == 1);
        CHECK(layout->lineAt(1).position() == QPoint({0, 1}));
        CHECK(layout->lineAt(1).rect() == QRect({0, 1, 3, 1}));

        CHECK(layout->lineAt(2).x() == 0);
        CHECK(layout->lineAt(2).y() == 2);
        CHECK(layout->lineAt(2).width() == 3);
        CHECK(layout->lineAt(2).height() == 1);
        CHECK(layout->lineAt(2).position() == QPoint({0, 2}));
        CHECK(layout->lineAt(2).rect() == QRect({0, 2, 3, 1}));

        CHECK(layout->lineAt(3).x() == 0);
        CHECK(layout->lineAt(3).y() == 3);
        CHECK(layout->lineAt(3).width() == 3);
        CHECK(layout->lineAt(3).height() == 1);
        CHECK(layout->lineAt(3).position() == QPoint({0, 3}));
        CHECK(layout->lineAt(3).rect() == QRect({0, 3, 3, 1}));

        CHECK(layout->lineAt(4).x() == 0); // invalid
        CHECK(layout->lineAt(4).y() == 0); // invalid
        CHECK(layout->lineAt(4).height() == 1); // invalid
        CHECK(layout->lineAt(4).width() == 0); // invalid
        CHECK(layout->lineAt(4).position() == QPoint({0, 0}));
        CHECK(layout->lineAt(4).rect() == QRect({0, 0, 0, 1}));
    }

    SECTION("tab-size4-ShowTabsAndSpacesWithColors-pos") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTabColor([styleBWhiteBlue] (int pos, int size, int hidden, const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            CHECK(size + hidden == 4);
            return Tui::ZTextStyle{Tui::Colors::brightWhite, {pos * 30, 0, 0}};
        });
        to.setTabStopDistance(4);

        layout->setTextOption(to);
        layout->setText("1\t"
                        "22\t"
                        "333"
                        "\t");
        layout->doLayout(3);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 4);

        CHECK(zi.peekBackground(1, 1) == Tui::Colors::blue);
        CHECK(zi.peekBackground(2, 1) == Tui::ZColor{2 * 30, 0, 0});
        CHECK(zi.peekBackground(3, 1) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(4, 1) == Tui::ZColor{0, 0, 0});

        CHECK(zi.peekBackground(1, 2) == Tui::Colors::blue);
        CHECK(zi.peekBackground(2, 2) == Tui::Colors::blue);
        CHECK(zi.peekBackground(3, 2) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(4, 2) == Tui::ZColor{0, 0, 0});

        CHECK(zi.peekBackground(1, 3) == Tui::Colors::blue);
        CHECK(zi.peekBackground(2, 3) == Tui::Colors::blue);
        CHECK(zi.peekBackground(3, 3) == Tui::Colors::blue);
        CHECK(zi.peekBackground(4, 3) == Tui::ZColor{00, 0, 0});

        CHECK(zi.peekBackground(1, 4) == Tui::ZColor{1 * 30, 0, 0});
        CHECK(zi.peekBackground(2, 4) == Tui::ZColor{2 * 30, 0, 0});
        CHECK(zi.peekBackground(3, 4) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(4, 4) == Tui::ZColor{0, 0, 0});

        t.compare(zi);
    }

    SECTION("tab-ShowTabsAndSpacesWithColors-pos") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTabColor([styleBWhiteBlue] (int pos, int size, int hidden, const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            (void)size; (void)hidden;
            return Tui::ZTextStyle{Tui::Colors::brightWhite, {pos * 30, 0, 0}};
        });
        layout->setTextOption(to);
        layout->setText("1\t"       "22\t"     "333\t"
                        "4444\t"    "55555\t"  "666666\t"
                        "7777777\t" "88888888" "\t");
        layout->doLayout(24);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 3);

        CHECK(zi.peekBackground(1, 1) == Tui::Colors::blue);
        CHECK(zi.peekBackground(2, 1) == Tui::ZColor{1 * 30, 0, 0});
        CHECK(zi.peekBackground(3, 1) == Tui::ZColor{2 * 30, 0, 0});
        CHECK(zi.peekBackground(4, 1) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(5, 1) == Tui::ZColor{4 * 30, 0, 0});
        CHECK(zi.peekBackground(6, 1) == Tui::ZColor{5 * 30, 0, 0});
        CHECK(zi.peekBackground(7, 1) == Tui::ZColor{6 * 30, 0, 0});
        CHECK(zi.peekBackground(8, 1) == Tui::ZColor{7 * 30, 0, 0});

        CHECK(zi.peekBackground(8 + 1, 1) == Tui::Colors::blue);
        CHECK(zi.peekBackground(8 + 2, 1) == Tui::Colors::blue);
        CHECK(zi.peekBackground(8 + 3, 1) == Tui::ZColor{2 * 30, 0, 0});
        CHECK(zi.peekBackground(8 + 4, 1) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(8 + 5, 1) == Tui::ZColor{4 * 30, 0, 0});
        CHECK(zi.peekBackground(8 + 6, 1) == Tui::ZColor{5 * 30, 0, 0});
        CHECK(zi.peekBackground(8 + 7, 1) == Tui::ZColor{6 * 30, 0, 0});
        CHECK(zi.peekBackground(8 + 8, 1) == Tui::ZColor{7 * 30, 0, 0});

        CHECK(zi.peekBackground(16 + 1, 3) == Tui::ZColor{0 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 2, 3) == Tui::ZColor{1 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 3, 3) == Tui::ZColor{2 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 4, 3) == Tui::ZColor{3 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 5, 3) == Tui::ZColor{4 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 6, 3) == Tui::ZColor{5 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 7, 3) == Tui::ZColor{6 * 30, 0, 0});
        CHECK(zi.peekBackground(16 + 8, 3) == Tui::ZColor{7 * 30, 0, 0});

        t.compare(zi);
    }

    SECTION("tab-ShowTabsAndSpacesWithColors-size") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTabColor([styleBWhiteBlue] (int pos, int size, int hidden, const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            (void)pos;
            CHECK(size + hidden == 8);
            return Tui::ZTextStyle{Tui::Colors::brightWhite, {0, size * 30, 0}};
        });
        layout->setTextOption(to);
        layout->setText("1\t"       "22\t"     "333\t"
                        "4444\t"    "55555\t"  "666666\t"
                        "7777777\t" "88888888" "\t");
        layout->doLayout(24);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 3);

        for (int i = 0; i < 8; i++) {
            CAPTURE(i);
            CHECK(zi.peekBackground( 1 + i, 1) == (i < 1 ? Tui::Colors::blue : Tui::ZColor{0, 7 * 30, 0}));
            CHECK(zi.peekBackground( 9 + i, 1) == (i < 2 ? Tui::Colors::blue : Tui::ZColor{0, 6 * 30, 0}));
            CHECK(zi.peekBackground(17 + i, 1) == (i < 3 ? Tui::Colors::blue : Tui::ZColor{0, 5 * 30, 0}));
            CHECK(zi.peekBackground( 1 + i, 2) == (i < 4 ? Tui::Colors::blue : Tui::ZColor{0, 4 * 30, 0}));
            CHECK(zi.peekBackground( 9 + i, 2) == (i < 5 ? Tui::Colors::blue : Tui::ZColor{0, 3 * 30, 0}));
            CHECK(zi.peekBackground(17 + i, 2) == (i < 6 ? Tui::Colors::blue : Tui::ZColor{0, 2 * 30, 0}));
            CHECK(zi.peekBackground( 1 + i, 3) == (i < 7 ? Tui::Colors::blue : Tui::ZColor{0, 1 * 30, 0}));
            CHECK(zi.peekBackground( 9 + i, 3) == Tui::Colors::blue);
            CHECK(zi.peekBackground(16 + 1 + i, 3) == Tui::ZColor{0, 8 * 30, 0});
        }

        t.compare(zi);
    }

    SECTION("tab-ShowTabsAndSpacesWithColors-hidden") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTabColor([styleBWhiteBlue] (int pos, int size, int hidden, const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            (void)pos; (void)size;
            return Tui::ZTextStyle{Tui::Colors::brightWhite, {0, 0, hidden * 30}};
        });
        layout->setTextOption(to);
        layout->setText("1\t"       "22\t"     "333\t"
                        "4444\t"    "55555\t"  "666666\t"
                        "7777777\t" "88888888" "\t");
        layout->doLayout(24);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 3);
        t.compare(zi);
    }

    SECTION("tab-distance") {
        for (int distance = 1; distance < 42; distance++) {
            CAPTURE(distance);
            Tui::ZTextOption to;
            to.setTabStopDistance(distance);
            layout->setTextOption(to);
            layout->setText("a\t");
            layout->doLayout(1000);
            if (distance == 1) {
                CHECK(layout->maximumWidth() == 2);
            } else {
                CHECK(layout->maximumWidth() == distance);
            }
        }
    }

    SECTION("tab-array") {
        Tui::ZTextOption to;
        to.setTabArray({2, 7, 13});
        layout->setTextOption(to);
        layout->setText("\tLorem\n\t\tipsumm\n\t\t\tdolor\n\t\t\t\tsus.");
        layout->doLayout(100);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("tab-widechar") {
        Tui::ZImage zi{t.terminal.get(), 26, 11};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

        layout->setText(
                        "あ\thiragana\n"
                        "😎\tsmilie\n"
                        "a\tlatin\n"
                        "ああ\thiragana\n"
                        "😎😎\tsmilie\n"
                        "a1\tlatin\n"
                        "あああ\thiragana\n"
                        "😎😎😎\tsmilie\n"
                        "a1-\tlatin\n"
                    );
        layout->doLayout(24);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("wrap-double") {
        layout->setText("\t\t\t😎 😎😎\t\t\t\t\t");
        layout->doLayout(30);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).isValid() == true);
        CHECK(layout->lineAt(0).lineNumber() == 0);
        CHECK(layout->lineAt(0).textStart() == 0);
        CHECK(layout->lineAt(0).textLength() == 8);
        CHECK(layout->lineAt(1).lineNumber() == 1);
        CHECK(layout->lineAt(1).textStart() == 8);
        CHECK(layout->lineAt(1).textLength() == 6);
        CHECK(layout->lineAt(2).textStart() == 14);
        CHECK(layout->lineAt(2).textLength() == 1);
        CHECK(layout->lineAt(3).lineNumber() == -1);
        CHECK(layout->lineAt(3).textStart() == 0);
        CHECK(layout->lineAt(3).textLength() == 0);
        CHECK(layout->lineAt(3).isValid() == false);

        CHECK(layout->lineForTextPosition(0).lineNumber() == 0);
        CHECK(layout->lineForTextPosition(7).lineNumber() == 0);
        CHECK(layout->lineForTextPosition(8).lineNumber() == 1);
        CHECK(layout->lineForTextPosition(13).lineNumber() == 1);
        CHECK(layout->lineForTextPosition(14).lineNumber() == 2);
        CHECK(layout->lineForTextPosition(15).lineNumber() == 2);
        CHECK(layout->lineForTextPosition(16).lineNumber() == -1);

        CHECK(layout->lineCount() == 3);
        t.compare(zi);
    }

    SECTION("wrap-visible-spaces") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::ShowTabsAndSpaces);
        to.setWrapMode(Tui::ZTextOption::WrapAnywhere);
        layout->setTextOption(to);
        layout->setText("testtext  amet");
        layout->doLayout(9);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("wrap-visible-spaces-width-0") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::ShowTabsAndSpaces);
        to.setWrapMode(Tui::ZTextOption::WrapAnywhere);
        layout->setTextOption(to);
        layout->setText("testtext  amet");
        layout->doLayout(0);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
    }

    SECTION("trailing-spaces") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([styleBWhiteBlue] (const Tui::ZTextStyle& baseStyle, const Tui::ZTextStyle& formattingStyle, const Tui::ZFormatRange *ranges) {
            CHECK(styleBWhiteBlue.foregroundColor() == baseStyle.foregroundColor());
            CHECK(styleBWhiteBlue.backgroundColor() == baseStyle.backgroundColor());
            CHECK(styleBWhiteBlue.attributes() == baseStyle.attributes());

            // FormattingStyle must default to baseStyle if not specified
            CHECK(styleBWhiteBlue.foregroundColor() == formattingStyle.foregroundColor());
            CHECK(styleBWhiteBlue.backgroundColor() == formattingStyle.backgroundColor());
            CHECK(styleBWhiteBlue.attributes() == formattingStyle.attributes());

            CHECK(ranges == nullptr);
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("ha\tllo \n"      // explicit line break: should be colored
                    "\t😎😎😎 "        // wrap at layout width: should not be colored
                    "morgen hab ich " // end of text is also end of line: should be colored
                    );
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 3);
        t.compare(zi);
    }

    SECTION("trailing-spaces-newline-as-last-character") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([] (const Tui::ZTextStyle& baseStyle, const Tui::ZTextStyle& formattingStyle, const Tui::ZFormatRange*) {
            (void)baseStyle; (void)formattingStyle;
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("ha\tllo \n"); // \n should not show up itself, but the space before counts as trailing
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 1);
        CHECK(layout->lineAt(0).width() == 12);
        CHECK(layout->maximumWidth() == 12);
        CHECK(layout->boundingRect() == QRect{0, 0, 12, 1});
        t.compare(zi);
    }

    SECTION("trailing-spaces-line-with-only-one-space") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([] (const Tui::ZTextStyle& baseStyle, const Tui::ZTextStyle& formattingStyle, const Tui::ZFormatRange*) {
            (void)baseStyle; (void)formattingStyle;
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("h a\tllo \n ");
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineCount() == 2);
        CHECK(layout->lineAt(1).width() == 1);
        CHECK(layout->maximumWidth() == 12);
        CHECK(layout->boundingRect() == QRect{0, 0, 12, 2});
        t.compare(zi);
    }

    SECTION("trailing-spaces-format-ranges") {
        // Check interactions between trailing space coloring and format ranges
        // The test setup has the first two characters not covered by any format ranges.
        // Following that the characters starting with "u" up until the first space on the
        // second line are covered by format range 2 (fr2).
        // On the last line format range 1 (fr) covers " da " leaving the last trainling
        // space uncovered again.

        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([styleBWhiteBlue, styleRedGreenBold, styleCyanYellowItalic, styleMagentaDGrayStrike] (const Tui::ZTextStyle& baseStyle, const Tui::ZTextStyle& formattingStyle, const Tui::ZFormatRange* fr) {
            CHECK(styleBWhiteBlue.foregroundColor() == baseStyle.foregroundColor());
            CHECK(styleBWhiteBlue.backgroundColor() == baseStyle.backgroundColor());
            CHECK(styleBWhiteBlue.attributes() == baseStyle.attributes());
            CHECK(styleRedGreenBold.foregroundColor() == formattingStyle.foregroundColor());
            CHECK(styleRedGreenBold.backgroundColor() == formattingStyle.backgroundColor());
            CHECK(styleRedGreenBold.attributes() == formattingStyle.attributes());

            if (fr && fr->userData() == 11) {
                CHECK(fr->start() == 22);
                CHECK(fr->length() == 4);
                CHECK(fr->format() == styleCyanYellowItalic);
                CHECK(fr->formattingChar() == styleMagentaDGrayStrike);
                return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::black};
            } else if (fr && fr->userData() == 32) {
                CHECK(fr->start() == 2);
                CHECK(fr->length() == 9);
                CHECK(fr->format() == styleMagentaDGrayStrike);
                CHECK(fr->formattingChar() == styleCyanYellowItalic);
                return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::brightGreen};
            } else {
                CHECK(!fr);
            }
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("h u \tllo \n  \n hallo du da  ");
        layout->doLayout(15);
        Tui::ZFormatRange fr;
        fr.setStart(layout->text().size() - 5);
        fr.setLength(4);
        fr.setFormat(styleCyanYellowItalic);
        fr.setFormattingChar(styleMagentaDGrayStrike);
        fr.setUserData(11);

        Tui::ZFormatRange fr2;
        fr2.setStart(2);
        fr2.setLength(9);
        fr2.setFormat(styleMagentaDGrayStrike);
        fr2.setFormattingChar(styleCyanYellowItalic);
        fr2.setUserData(32);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {fr, fr2});
        t.compare(zi);
    }

    SECTION("trailing-spaces-format-range-priority") {
        // Check interactions between trailing space coloring and format ranges.
        // Tests that the later formatting range wins over previous formatting ranges and
        // the base formats.
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([styleBWhiteBlue, styleRedGreenBold, styleCyanYellowItalic, styleMagentaDGrayStrike] (const Tui::ZTextStyle& baseStyle, const Tui::ZTextStyle& formattingStyle, const Tui::ZFormatRange* fr) {
            CHECK(styleBWhiteBlue.foregroundColor() == baseStyle.foregroundColor());
            CHECK(styleBWhiteBlue.backgroundColor() == baseStyle.backgroundColor());
            CHECK(styleBWhiteBlue.attributes() == baseStyle.attributes());
            CHECK(styleRedGreenBold.foregroundColor() == formattingStyle.foregroundColor());
            CHECK(styleRedGreenBold.backgroundColor() == formattingStyle.backgroundColor());
            CHECK(styleRedGreenBold.attributes() == formattingStyle.attributes());

            if (fr && fr->userData() == 11) {
                CHECK(fr->start() == 0);
                CHECK(fr->length() == 11);
                CHECK(fr->format() == styleBWhiteBlue);
                CHECK(fr->formattingChar() == styleRedGreenBold);
            } else if (fr && fr->userData() == 32) {
                CHECK(fr->start() == 0);
                CHECK(fr->length() == 11);
                CHECK(fr->format() == styleMagentaDGrayStrike);
                CHECK(fr->formattingChar() == styleCyanYellowItalic);
                return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::brightGreen};
            } else {
                CHECK(!fr);
            }
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("h u \tllo \n ");
        layout->doLayout(15);
        Tui::ZFormatRange fr;
        fr.setStart(0);
        fr.setLength(layout->text().size());
        fr.setFormat(styleBWhiteBlue);
        fr.setFormattingChar(styleRedGreenBold);
        fr.setUserData(11);

        Tui::ZFormatRange fr2;
        fr2.setStart(0);
        fr2.setLength(layout->text().size());
        fr2.setFormat(styleMagentaDGrayStrike);
        fr2.setFormattingChar(styleCyanYellowItalic);
        fr2.setUserData(32);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {fr, fr2});
        t.compare(zi);
    }

    SECTION("selections-invalid-position-mid-start") {
        layout->setText("ba\u0308\u0304c");
        layout->doLayout(100);
        Tui::ZFormatRange fr;
        fr.setStart(2);
        fr.setLength(layout->text().size() - fr.start() - 1);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);

        fr.setStart(3);
        fr.setLength(layout->text().size() - fr.start() - 1);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);
    }

    SECTION("selections-begin") {
        layout->setText("a\u0308\u0304bc");
        layout->doLayout(100);
        Tui::ZFormatRange fr;
        fr.setStart(0);
        fr.setLength(3);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);

        fr.setStart(-1);
        fr.setLength(4);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);
    }

    SECTION("selections-invalid-position-begin-start") {
        layout->setText("a\u0308\u0304bc");
        layout->doLayout(100);
        Tui::ZFormatRange fr;
        fr.setStart(1);
        fr.setLength(2);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare("selections-begin", zi);

        fr.setStart(2);
        fr.setLength(1);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare("selections-begin", zi);

        fr.setStart(1);
        fr.setLength(1);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare("selections-begin", zi);
    }

    SECTION("selections-invalid-position-begin-end") {
        layout->setText("a\u0308\u0304bc");
        layout->doLayout(100);
        Tui::ZFormatRange fr;
        fr.setStart(0);
        fr.setLength(1);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);

        fr.setStart(0);
        fr.setLength(2);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);
    }

    SECTION("selections-invalid-position-astral") {
        layout->setText("😎bc");
        layout->doLayout(100);
        Tui::ZFormatRange fr;
        fr.setStart(1);
        fr.setLength(1);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);

        fr.setStart(0);
        fr.setLength(1);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);
    }

    SECTION("selections-line-break") {
        // The selection must not paint over the third column which should keep transparent.
        layout->setText("😎😎");
        layout->doLayout(3);
        Tui::ZFormatRange fr;
        fr.setStart(0);
        fr.setLength(4);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(zi);
    }

    SECTION("selections-clipped-astral") {
        // Check invalid selection ranges also with -1 or 0 length
        struct TestCase { int sourceLine; int start; int length; QString tpi; };
        auto testCase = GENERATE( TestCase{__LINE__,  0, -1, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  1, -1, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  2, -1, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  3, -1, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__, -1,  0, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  0,  0, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  1,  0, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  2,  0, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  3,  0, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__, -1,  1, "selections-clipped-astral-blue-blue"},
                                  TestCase{__LINE__,  0,  1, "selections-clipped-astral-green-blue"},
                                  TestCase{__LINE__,  1,  1, "selections-clipped-astral-green-blue"},
                                  TestCase{__LINE__,  2,  1, "selections-clipped-astral-blue-green"},
                                  TestCase{__LINE__,  3,  1, "selections-clipped-astral-blue-green"},
                                  TestCase{__LINE__, -1,  2, "selections-clipped-astral-green-blue"},
                                  TestCase{__LINE__,  0,  2, "selections-clipped-astral-green-blue"},
                                  TestCase{__LINE__,  1,  2, "selections-clipped-astral-green-green"},
                                  TestCase{__LINE__,  2,  2, "selections-clipped-astral-blue-green"},
                                  TestCase{__LINE__,  3,  2, "selections-clipped-astral-blue-green"},
                                  TestCase{__LINE__, -1,  3, "selections-clipped-astral-green-blue"},
                                  TestCase{__LINE__,  0,  3, "selections-clipped-astral-green-green"},
                                  TestCase{__LINE__,  1,  3, "selections-clipped-astral-green-green"},
                                  TestCase{__LINE__,  2,  3, "selections-clipped-astral-blue-green"} );

        CAPTURE(testCase.sourceLine);
        CAPTURE(testCase.start);
        CAPTURE(testCase.length);
        layout->setText("😎😎");

        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::NoWrap);
        layout->setTextOption(to);

        layout->doLayout(3);
        Tui::ZFormatRange fr;
        fr.setStart(testCase.start);
        fr.setLength(testCase.length);
        fr.setFormat(styleRedGreenBold);
        fr.setFormattingChar(styleRedGreenBold);

        layout->draw(zi.painter(), {-1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {fr});
        t.compare(testCase.tpi, zi);
    }

    SECTION("word-wrap") {
        Tui::ZImage zi{t.terminal.get(), 12, 7};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::WordWrap);
        layout->setTextOption(to);
        layout->setText("Lorem ipsum dolor sit amet, conseteturis");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).width() == 6);
        CHECK(layout->lineAt(1).width() == 6);
        CHECK(layout->lineAt(2).width() == 10);
        CHECK(layout->lineAt(3).width() == 6);
        CHECK(layout->lineAt(4).width() == 10);
        CHECK(layout->lineAt(5).width() == 2);
        t.compare(zi);
    }

    SECTION("word-wrap-newline") {
        Tui::ZImage zi{t.terminal.get(), 12, 7};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::WordWrap);
        layout->setTextOption(to);
        layout->setText("ABCD ABCDE\nABCD ABCDEF");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("word-wrap-spaceruns") {
        Tui::ZImage zi{t.terminal.get(), 20, 7};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::WordWrap);
        layout->setTextOption(to);
        layout->setText("0123456789 Lorem sit  amet     consetetur");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).width() == 10);
        CHECK(layout->lineAt(1).width() == 10);
        CHECK(layout->lineAt(2).width() == 10);
        CHECK(layout->lineAt(3).width() == 1);
        CHECK(layout->lineAt(4).width() == 10);
        t.compare(zi);
    }

    SECTION("nowrap") {
        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::NoWrap);
        layout->setTextOption(to);
        layout->setText("Lorem ipsum dolor sit\namet, consetetur ");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).width() == 21);
        CHECK(layout->lineAt(1).width() == 17);
        t.compare(zi);
    }

    SECTION("nowrap-utf8") {
        Tui::ZTextOption to;
        to.setWrapMode(Tui::ZTextOption::NoWrap);
        layout->setTextOption(to);
        layout->setText("Lorem i😎😎 dolor sit\namet, cああtetur\nconsetas\taa");
        layout->doLayout(10);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).width() == 21);
        CHECK(layout->lineAt(1).width() == 16);
        t.compare(zi);
    }

    SECTION("special-combining-grapheme-joiner") {
        // Check that a zero width character in the special chars list is also detected after a non-zero width character
        layout->setText("a\u034f");
        layout->doLayout(100);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("special-combining-grapheme-joiner-2") {
        // Check that multiple zero width characters in the special chars list is also detected after a non-zero width character
        layout->setText("a\u034f\u034f");
        layout->doLayout(100);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(0).width() == 17);
    }

    SECTION("manual-layout") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([] (const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("Lorem ipsum dolor sit amet, consetetur ");

        layout->beginLayout();

        Tui::ZTextLineRef line = layout->createLine();
        line.setLineWidth(18);
        line.setPosition({0, 0});
        CHECK(line.width() == 18);

        line = layout->createLine();
        line.setLineWidth(10);
        line.setPosition({5, 1});

        CHECK(line.x() == 5);
        CHECK(line.y() == 1);
        CHECK(line.position() == QPoint({5, 1}));
        CHECK(line.rect() == QRect({5, 1, 10, 1}));

        line = layout->createLine();
        line.setLineWidth(11);
        line.setPosition({10, 2});

        CHECK(line.x() == 10);
        CHECK(line.y() == 2);
        CHECK(line.position() == QPoint({10, 2}));
        CHECK(line.rect() == QRect({10, 2, 11, 1}));

        layout->endLayout();

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);

        CHECK(layout->lineAt(0).width() == 18);
        CHECK(layout->maximumWidth() == 18);
        CHECK(layout->lineAt(1).textLength() == 10);
        CHECK(layout->boundingRect() == QRect{0, 0, 21, 3});
        CHECK(layout->lineCount() == 3);
        CHECK(layout->lineAt(2).textStart() == 28);
        CHECK(layout->lineAt(1).xToCursor(0) == 18);
        CHECK(layout->lineAt(1).xToCursor(5) == 18);
        CHECK(layout->lineAt(1).xToCursor(6) == 19);
        CHECK(layout->lineAt(0).cursorToX(18, Tui::ZTextLayout::Edge::Leading) == 18);
        CHECK(layout->lineAt(1).cursorToX(18, Tui::ZTextLayout::Edge::Leading) == 5);
        CHECK(layout->lineAt(1).cursorToX(18, Tui::ZTextLayout::Edge::Trailing) == 6);
        CHECK(layout->lineAt(2).cursorToX(18, Tui::ZTextLayout::Edge::Leading) == 10);
    }

    SECTION("ascii+latin") {
        Tui::ZImage zi{t.terminal.get(), 80, 16};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, Tui::ZColor::defaultColor()  , u'␥');
        for (int i = 0; i <= 0x2af; i++ ) {
            layout->setText(layout->text() + QString(QChar(i)));
        }
        layout->doLayout(78);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("kana") {
        Tui::ZImage zi{t.terminal.get(), 80, 8};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');
        // hiragana
        for (int i = 0x3041; i <= 0x3096; i++ ) {
            layout->setText(layout->text() + QString(QChar(i)));
        }
        layout->setText(layout->text() + "\n");
        // katakana
        for (int i = 0x30a0; i <= 0x30FF; i++ ) {
            layout->setText(layout->text() + QString(QChar(i)));
        }
        // This is not a surrogate escape
        layout->setText(layout->text() + QString("\U0002dc00"));
        layout->doLayout(78);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("astral-plain-emojis") {
        Tui::ZImage zi{t.terminal.get(), 80, 20};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');
        for (int i = 0x1f300; i <= 0x1f5ff; i++) {
            layout->setText(layout->text() + (QString(1, QChar::highSurrogate(i))+QString(1, QChar::lowSurrogate(i))));
        }
        layout->doLayout(78);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("emoticons-block") {
        Tui::ZImage zi{t.terminal.get(), 80, 5};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');
        for (int i = 0x1f600; i <= 0x1f64f; i++) {
            layout->setText(layout->text() + (QString(1, QChar::highSurrogate(i))+QString(1, QChar::lowSurrogate(i))));
        }
        layout->doLayout(78);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("non-printables") {
        Tui::ZImage zi{t.terminal.get(), 32, 15};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');
        QVector<QChar> text1 = {
            0, 1, 0x7f,
            0x80, 0xa0, 0xad, 0x034f, 0x061c, 0x070f, 0x115F, 0x1160, 0x17b4, 0x17b5, 0x180B, 0x180E, 0x2000, 0x200F,
            0x2028, 0x202F, 0x205F, 0x206F, 0x3164, 0xdc00, 0xdc10, 0xFDD0, 0xFDDF, 0xFDE0, 0xFDEF, 0xFE00, 0xFE0F,
            0xFEFF, 0xFFA0, 0xFFEF, 0xFFF0, 0xFFFE, 0xFFFF,
        };
        QString text2 = U8("\U000E0000\U000E0fff\U00016FE4\U0001BCA0\U0001BCAF\U0001D173\U0001D17A");
        layout->setText(QString(text1.data(), text1.size()) + text2);
        layout->doLayout(30);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("nonspacing-marks") {
        layout->setText("\u0308\n" // isolated non spacing mark
                        " \u0308\n" // space + non spacing mark
                        "\u00a0\u0308\n" // nbsp + non spacing mark
                        "a\u0308\n" // cluster 1 mark
                        "a\u0308\u0304\n" // cluster 2 marks
                        );
        layout->doLayout(40);
        layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {});
        t.compare(zi);
    }

    SECTION("space-nonspacing-marks") {
        layout->setText(" \u0308\n");
        layout->doLayout(40);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {});
        t.compare(zi);

        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::ShowTabsAndSpaces);
        layout->setTextOption(to);
        layout->doLayout(40);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {});
        t.compare(zi);
    }

    SECTION("tab-nonspacing-marks") {
        layout->setText("\t\u0308\n");
        layout->doLayout(40);
        CHECK(layout->lineAt(0).width() == 9);

        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::ShowTabsAndSpaces);
        layout->setTextOption(to);
        layout->doLayout(40);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {});
        CHECK(layout->lineAt(0).width() == 9);
    }

    SECTION("2space-nonspacing-marks") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::ShowTabsAndSpaces);
        layout->setTextOption(to);

        layout->setText("  \u0308\n");
        layout->doLayout(40);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold, {});
        t.compare(zi);
    }

    SECTION("non-printable-nospace") {
        // Non printable may not break if first of line to avoid infinte loop
        layout->setText(U8("\U0001d173"));
        layout->doLayout(5);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("non-printable-nospace2") {
        layout->setText(U8("a\U0001d173"));
        layout->doLayout(5);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        CHECK(layout->lineAt(1).width() == 10);
        t.compare(zi);
    }

    SECTION("non-printable-nospace3") {
        layout->setText(U8(" \U0001d173"));
        layout->doLayout(9);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }


    SECTION("non-printable-manual-layout") {
        // Check that clipping uses per line maximum width and not just the layout width of the first or last line
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([] (const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        layout->setTextOption(to);
        layout->setText("\n\U0001d173");
        layout->beginLayout();

        Tui::ZTextLineRef line = layout->createLine();
        line.setLineWidth(15);
        line.setPosition({0, 0});

        line = layout->createLine();
        line.setLineWidth(5);
        line.setPosition({5, 1});

        line = layout->createLine();
        line.setLineWidth(10);
        line.setPosition({10, 2});

        layout->endLayout();

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }


    SECTION("double-nospace") {
        // Non printable may not break if first of line to avoid infinte loop
        layout->setText("あ");
        layout->doLayout(1);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("double-nospace2") {
        layout->setText("aあ");
        layout->doLayout(1);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);
        t.compare(zi);
    }

    SECTION("relayout") {
        layout->setText("0123456789");
        layout->doLayout(40);
        layout->setText("ABCDE");
        layout->doLayout(40);
        CHECK(layout->maximumWidth() == 5);
        CHECK(layout->lineCount() == 1);
    }

    SECTION("source") {
        Tui::ZImage zi{t.terminal.get(), 40, 6};
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');

        auto tabColorizer = [] (int pos, int size, int hidden, const Tui::ZTextStyle &base, const Tui::ZTextStyle &formatting, const Tui::ZFormatRange* range) -> Tui::ZTextStyle {
            (void)size; (void)formatting; (void)range;
            if (pos == hidden) {
                return { base.foregroundColor(), {base.backgroundColor().red(),
                                base.backgroundColor().green() + 0x60,
                                base.backgroundColor().blue()} };

            }
            return { base.foregroundColor(), {base.backgroundColor().red(),
                            base.backgroundColor().green() + 0x40,
                            base.backgroundColor().blue()} };
        };
        layout->setText("void main() { \n"
                        "\tfor (int i = 0; i < 3; i++) {\n"
                        "\t\tputs(\"😎\");\n"
                        "    }    \n"
                        "}\n");
        Tui::ZTextOption to;
        to.setTabStopDistance(4);
        to.setTrailingWhitespaceColor([](const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*)
            { return Tui::ZTextStyle({0, 0, 0}, {255, 0, 0});});
        SECTION("plain") {
            layout->setTextOption(to);

            layout->doLayout(40);
            layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {});
            t.compare(zi);
        }

        SECTION("formatting-as-chars") {
            to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpaces);
            layout->setTextOption(to);

            layout->doLayout(40);
            layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {});
            t.compare(zi);
        }

        SECTION("formatting-as-colors") {
            to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
            to.setTabColor(tabColorizer);
            layout->setTextOption(to);

            layout->doLayout(40);
            layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {});
            t.compare(zi);
        }

        SECTION("formatting-as-both") {
            to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpaces | Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
            to.setTabColor(tabColorizer);
            layout->setTextOption(to);

            layout->doLayout(40);
            layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {});
            t.compare(zi);
        }

        SECTION("formattingranges") {
            to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpaces);
            layout->setTextOption(to);

            Tui::ZFormatRange fr;
            fr.setStart(7);
            fr.setLength(43);
            fr.setFormattingChar(Tui::ZTextStyle({11, 11, 11}, {245, 245, 245}, Tui::ZTextAttribute::Blink));
            fr.setFormat(Tui::ZTextStyle({220, 220, 0}, {100, 110, 120}, Tui::ZTextAttribute::Italic));

            Tui::ZFormatRange fr2 = fr;
            fr2.setStart(62);
            fr2.setLength(4);

            layout->doLayout(40);
            layout->draw(zi.painter(), {0, 0}, styleBWhiteBlue, &styleRedGreenBold, {fr, fr2});
            t.compare(zi);
        }

    }


    SECTION("isValidCursorPosition") {
        layout->setText("setText");
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(-1) == false);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == true);
        CHECK(layout->isValidCursorPosition(7) == true);
        CHECK(layout->isValidCursorPosition(8) == false);
    }

    SECTION("isValidCursorPosition-utf8") {
        layout->setText("😎\n😎");
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == false);
        CHECK(layout->isValidCursorPosition(2) == true);
        CHECK(layout->isValidCursorPosition(3) == true);
        CHECK(layout->isValidCursorPosition(4) == false);
        CHECK(layout->isValidCursorPosition(5) == true);
        CHECK(layout->lineAt(0).textLength() == 3);
        CHECK(layout->lineAt(1).textLength() == 2);

        layout->setText("\t😎\t😎\n");
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == true);
        CHECK(layout->isValidCursorPosition(2) == false);
        CHECK(layout->isValidCursorPosition(3) == true);
        CHECK(layout->isValidCursorPosition(4) == true);
        CHECK(layout->isValidCursorPosition(5) == false);
        CHECK(layout->isValidCursorPosition(6) == true);
        CHECK(layout->isValidCursorPosition(7) == true);
        CHECK(layout->isValidCursorPosition(8) == false);
        CHECK(layout->lineAt(0).textLength() == 7);
        CHECK(layout->lineAt(1).textLength() == 0);

        layout->setText("ああa\u0308a\u0308\u0304あ\u0308あ\u0308\u0304😎\u0308😎\u0308\u0304");
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == true);
        CHECK(layout->isValidCursorPosition(2) == true);
        CHECK(layout->isValidCursorPosition(3) == false);
        CHECK(layout->isValidCursorPosition(4) == true);
        CHECK(layout->isValidCursorPosition(5) == false);
        CHECK(layout->isValidCursorPosition(6) == false);
        CHECK(layout->isValidCursorPosition(7) == true);
        CHECK(layout->isValidCursorPosition(8) == false);
        CHECK(layout->isValidCursorPosition(9) == true);
        CHECK(layout->isValidCursorPosition(10) == false);
        CHECK(layout->isValidCursorPosition(11) == false);
        CHECK(layout->isValidCursorPosition(12) == true);
        CHECK(layout->isValidCursorPosition(13) == false);
        CHECK(layout->isValidCursorPosition(14) == false);
        CHECK(layout->isValidCursorPosition(15) == true);
        CHECK(layout->isValidCursorPosition(16) == false);
        CHECK(layout->isValidCursorPosition(17) == false);
        CHECK(layout->isValidCursorPosition(18) == false);
        CHECK(layout->isValidCursorPosition(19) == true);
        CHECK(layout->isValidCursorPosition(20) == false);

        CHECK(layout->lineAt(0).textLength() == 19);

        QVector<QChar> text1 = {
            0, 1, 0x7f, 0xdc00, 0xdc10
        };
        QString text2 = U8("\U0001d173");
        layout->setText(QString(text1.data(), text1.size()) + text2);
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == true);
        CHECK(layout->isValidCursorPosition(2) == true);
        CHECK(layout->isValidCursorPosition(3) == true);
        CHECK(layout->isValidCursorPosition(4) == true);
        CHECK(layout->isValidCursorPosition(5) == true);
        CHECK(layout->isValidCursorPosition(6) == false);
        CHECK(layout->isValidCursorPosition(7) == true);
        CHECK(layout->isValidCursorPosition(8) == false);
        CHECK(layout->lineAt(0).textLength() == 7);

        layout->setText("s\n\n\ns");
        layout->doLayout(100);
        CHECK(layout->isValidCursorPosition(0) == true);
        CHECK(layout->isValidCursorPosition(1) == true);
        CHECK(layout->isValidCursorPosition(2) == true);
        CHECK(layout->isValidCursorPosition(3) == true);
        CHECK(layout->isValidCursorPosition(4) == true);
        CHECK(layout->isValidCursorPosition(5) == true);
    }

    SECTION("previousCursorPosition") {
        layout->setText("setText");
        layout->doLayout(100);
        CHECK(layout->previousCursorPosition(-1) == 0);
        CHECK(layout->previousCursorPosition(0) == 0);
        CHECK(layout->previousCursorPosition(1) == 0);
        CHECK(layout->previousCursorPosition(2) == 1);
        CHECK(layout->previousCursorPosition(6) == 5);
        CHECK(layout->previousCursorPosition(7) == 6);
        CHECK(layout->previousCursorPosition(8) == 7);
        CHECK(layout->previousCursorPosition(9) == 7);
    }

    SECTION("previousCursorPosition-nl") {
        layout->setText("s\n\n\nu");
        layout->doLayout(100);
        CHECK(layout->previousCursorPosition(0) == 0);
        CHECK(layout->previousCursorPosition(1) == 0);
        CHECK(layout->previousCursorPosition(2) == 1);
        CHECK(layout->previousCursorPosition(3) == 2);
        CHECK(layout->previousCursorPosition(4) == 3);
        CHECK(layout->previousCursorPosition(5) == 4);
    }

    SECTION("nextCursorPosition") {
        layout->setText("setText");
        layout->doLayout(100);
        CHECK(layout->nextCursorPosition(-2) == 0);
        CHECK(layout->nextCursorPosition(-1) == 0);
        CHECK(layout->nextCursorPosition(0) == 1);
        CHECK(layout->nextCursorPosition(1) == 2);
        CHECK(layout->nextCursorPosition(7) == 7);
        CHECK(layout->nextCursorPosition(8) == 7);
    }

    SECTION("nextCursorPosition-nl") {
        layout->setText("s\n\n\nu");
        layout->doLayout(100);
        CHECK(layout->nextCursorPosition(0) == 1);
        CHECK(layout->nextCursorPosition(1) == 2);
        CHECK(layout->nextCursorPosition(2) == 3);
        CHECK(layout->nextCursorPosition(3) == 4);
        CHECK(layout->nextCursorPosition(4) == 5);
        CHECK(layout->nextCursorPosition(5) == 5);
    }

    SECTION("previousCursorPosition-skipword") {
        layout->setText("se tte xt");
        layout->doLayout(100);
        CHECK(layout->previousCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
        CHECK(layout->previousCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
        CHECK(layout->previousCursorPosition(6, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
        CHECK(layout->previousCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
        CHECK(layout->previousCursorPosition(8, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
        CHECK(layout->previousCursorPosition(9, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
        CHECK(layout->previousCursorPosition(10, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->isValidCursorPosition(10) == false);
    }

    SECTION("nextCursorPosition-skipword") {
        layout->setText("se tte xt");
        layout->doLayout(100);
        CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
        CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
        CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
        CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
        CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
        CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
        CHECK(layout->nextCursorPosition(6, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->nextCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->nextCursorPosition(8, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->nextCursorPosition(9, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->nextCursorPosition(10, Tui::ZTextLayout::CursorMode::SkipWords) == 9);
        CHECK(layout->isValidCursorPosition(10) == false);
    }

    SECTION("previousCursorPosition-skipword-space") {
        layout->setText("    t   Tt  ");
        layout->doLayout(100);
        CHECK(layout->previousCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->previousCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
        CHECK(layout->previousCursorPosition(6, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
        CHECK(layout->previousCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
        CHECK(layout->previousCursorPosition(8, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
        CHECK(layout->previousCursorPosition(9, Tui::ZTextLayout::CursorMode::SkipWords) == 8);
        CHECK(layout->previousCursorPosition(10, Tui::ZTextLayout::CursorMode::SkipWords) == 8);
        CHECK(layout->previousCursorPosition(11, Tui::ZTextLayout::CursorMode::SkipWords) == 8);
        CHECK(layout->previousCursorPosition(12, Tui::ZTextLayout::CursorMode::SkipWords) == 8);
        CHECK(layout->isValidCursorPosition(13) == false);
    }

    SECTION("nextCursorPosition-skipword-space") {
        layout->setText("    t   Tt  ");
        layout->doLayout(100);
        CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
        CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 10);
        CHECK(layout->nextCursorPosition(6, Tui::ZTextLayout::CursorMode::SkipWords) == 10);
        CHECK(layout->nextCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 10);
        CHECK(layout->nextCursorPosition(8, Tui::ZTextLayout::CursorMode::SkipWords) == 10);
        CHECK(layout->nextCursorPosition(9, Tui::ZTextLayout::CursorMode::SkipWords) == 10);
        CHECK(layout->nextCursorPosition(10, Tui::ZTextLayout::CursorMode::SkipWords) == 12);
        CHECK(layout->nextCursorPosition(11, Tui::ZTextLayout::CursorMode::SkipWords) == 12);
        CHECK(layout->nextCursorPosition(12, Tui::ZTextLayout::CursorMode::SkipWords) == 12);
        CHECK(layout->isValidCursorPosition(13) == false);
    }

    SECTION("nextCursorPosition-skipword-non-break-space-non-spacing-mark") {
        layout->setText("   \u00a0\u0308  ");
        layout->doLayout(100);
        CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
    }

    SECTION("previousCursorPosition-skipword-non-break-space-non-spacing-mark") {
        layout->setText("   \u00a0\u0308  ");
        layout->doLayout(100);
        CHECK(layout->previousCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
    }

    SECTION("nextCursorPosition-skip-word-breaking-space") {
        // Currently 0xa0 and 0x2007 are classified as normal spaces thus are included here
        // TODO: decide if \U0000200B should be included here too.
        QString spaces = U8("\t \u00a0\U00001680\U00002000\U00002001\U00002002\U00002003\U00002004\U00002005\U00002006"
                              "\U00002007\U00002008\U00002009\U0000200A\U0000205F\U00003000");
        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            CAPTURE(spaces.at(i).isSpace());
            layout->setText("AA" + QString(spaces.at(i)) + "AA");
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        }
        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            layout->setText("AA" + QString(spaces.at(i)) + QString(spaces.at(i)) + "AA");
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 6);
        }
        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            layout->setText(QString(spaces.at(i)) + QString(spaces.at(i)) + "AA");
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
        }
        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            layout->setText("A" + QString(spaces.at(i)) + QString(spaces.at(i)));
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 1);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 3);
        }
        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            layout->setText("aa" + QString(spaces.at(i)) + "A" + QString(spaces.at(i)) + "AA");
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 4);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
            CHECK(layout->nextCursorPosition(6, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
            CHECK(layout->nextCursorPosition(7, Tui::ZTextLayout::CursorMode::SkipWords) == 7);
        }
    }

    SECTION("nextCursorPosition-skip-word-non-breaking-space") {
        // Currently 0xa0 and 0x2007 are classified as normal spaces thus are not included here
        QString spaces;

        QTextBoundaryFinder bf(QTextBoundaryFinder::Word, "AA\u202FAA");
        bf.setPosition(2);
        if (!bf.isAtBoundary()) {
            spaces += U8("\U0000202F");
        } else {
            // Triggered by Debian Stretch
            WARN("Test deactivated because outdated unicode data");
        }

        for (int i = 0; i < spaces.size(); i++) {
            CAPTURE(spaces.at(i).unicode());
            CAPTURE(spaces.at(i).isSpace());
            layout->setText("AA" + QString(spaces.at(i)) + "AA");
            QTextBoundaryFinder bf(QTextBoundaryFinder::Word, layout->text());
            bf.setPosition(2);
            CAPTURE(bf.isAtBoundary());
            layout->doLayout(5);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
        }
    }

    SECTION("nextCursorPosition-skip-word-line-break") {
        QVector<QChar> point = { 10, 11, 12, 13, 133 };
        QString linebreak = QString(point.data(), point.size()) + U8("\U00002028\U00002029");

        for (int i = 0; i < linebreak.size(); i++) {
            CAPTURE(linebreak.at(i).unicode());
            CAPTURE(linebreak.at(i).isSpace());
            layout->setText("AA" + QString(linebreak.at(i)) + "AA");
            layout->doLayout(100);
            CHECK(layout->nextCursorPosition(-1, Tui::ZTextLayout::CursorMode::SkipWords) == 0);
            CHECK(layout->nextCursorPosition(0, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(1, Tui::ZTextLayout::CursorMode::SkipWords) == 2);
            CHECK(layout->nextCursorPosition(2, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(3, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(4, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            CHECK(layout->nextCursorPosition(5, Tui::ZTextLayout::CursorMode::SkipWords) == 5);
            if (linebreak.at(i) == '\n') {
                CHECK(layout->lineCount() == 2);
            } else {
                // TODO: Add option to interpret these are linebreak too
                CHECK(layout->lineCount() == 1);
            }
        }
    }

    SECTION("xToCursor") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([] (const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        QVector<QChar> text1 = {
            0, 1, 0x7f, 0xdc00, 0xdc10
        };
        QString text2 = U8("\U0001d173");
        layout->setText(QString(text1.data(), text1.size()) + text2 + "A\t\n\tAbcDあ😎");

        layout->setTextOption(to);
        layout->doLayout(15);

        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleRedGreenBold);
        CHECK(layout->lineAt(0).xToCursor(0) == 0);  // ^
        CHECK(layout->lineAt(0).xToCursor(1) == 1);  // @
        CHECK(layout->lineAt(0).xToCursor(2) == 1);  // ^
        CHECK(layout->lineAt(0).xToCursor(3) == 2);  // A
        CHECK(layout->lineAt(0).xToCursor(4) == 2);  // ^
        CHECK(layout->lineAt(0).xToCursor(5) == 3);  // ?
        CHECK(layout->lineAt(0).xToCursor(6) == 3);  // <
        CHECK(layout->lineAt(0).xToCursor(7) == 3);  // 0
        CHECK(layout->lineAt(0).xToCursor(8) == 4);  // 0
        CHECK(layout->lineAt(0).xToCursor(9) == 4);  // >
        CHECK(layout->lineAt(0).xToCursor(10) == 4); // <
        CHECK(layout->lineAt(0).xToCursor(11) == 4); // 1
        CHECK(layout->lineAt(0).xToCursor(12) == 5); // 0
        CHECK(layout->lineAt(0).xToCursor(13) == 5); // >
        CHECK(layout->lineAt(0).xToCursor(14) == 5); // transparent
        CHECK(layout->lineAt(0).xToCursor(15) == 5);

        CHECK(layout->lineAt(1).xToCursor(0) == 5);  // <
        CHECK(layout->lineAt(1).xToCursor(1) == 5);  // U
        CHECK(layout->lineAt(1).xToCursor(2) == 5);  // +
        CHECK(layout->lineAt(1).xToCursor(3) == 5);  // 0
        CHECK(layout->lineAt(1).xToCursor(4) == 5);  // 1
        CHECK(layout->lineAt(1).xToCursor(5) == 7);  // D
        CHECK(layout->lineAt(1).xToCursor(6) == 7);  // 1
        CHECK(layout->lineAt(1).xToCursor(7) == 7);  // 7
        CHECK(layout->lineAt(1).xToCursor(8) == 7);  // 3
        CHECK(layout->lineAt(1).xToCursor(9) == 7);  // >
        CHECK(layout->lineAt(1).xToCursor(10) == 7); // A
        CHECK(layout->lineAt(1).xToCursor(11) == 8); // TAB
        CHECK(layout->lineAt(1).xToCursor(12) == 8); // TAB
        CHECK(layout->lineAt(1).xToCursor(13) == 9); // TAB
        CHECK(layout->lineAt(1).xToCursor(14) == 9); // TAB
        CHECK(layout->lineAt(1).xToCursor(15) == 9);
        // \n
        CHECK(layout->lineAt(2).xToCursor(0) == 10);  // TAB
        CHECK(layout->lineAt(2).xToCursor(1) == 10);  // TAB
        CHECK(layout->lineAt(2).xToCursor(2) == 10);  // TAB
        CHECK(layout->lineAt(2).xToCursor(3) == 10);  // TAB
        CHECK(layout->lineAt(2).xToCursor(4) == 11);  // TAB
        CHECK(layout->lineAt(2).xToCursor(5) == 11);  // TAB
        CHECK(layout->lineAt(2).xToCursor(6) == 11);  // TAB
        CHECK(layout->lineAt(2).xToCursor(7) == 11);  // TAB
        CHECK(layout->lineAt(2).xToCursor(8) == 11);  // A
        CHECK(layout->lineAt(2).xToCursor(9) == 12);  // b
        CHECK(layout->lineAt(2).xToCursor(10) == 13); // c
        CHECK(layout->lineAt(2).xToCursor(11) == 14); // D
        CHECK(layout->lineAt(2).xToCursor(12) == 15); // あ
        CHECK(layout->lineAt(2).xToCursor(13) == 16); // あ
        CHECK(layout->lineAt(2).xToCursor(14) == 16); // transparent
        CHECK(layout->lineAt(2).xToCursor(15) == 16); //

        CHECK(layout->lineAt(3).xToCursor(0) == 16); // 😎
        CHECK(layout->lineAt(3).xToCursor(1) == 18); // 😎
        CHECK(layout->lineAt(3).xToCursor(2) == 18); // transparent
    }

    SECTION("cursorToX") {
        Tui::ZTextOption to;
        to.setFlags(Tui::ZTextOption::Flag::ShowTabsAndSpacesWithColors);
        to.setTrailingWhitespaceColor([styleBWhiteBlue] (const Tui::ZTextStyle&, const Tui::ZTextStyle&, const Tui::ZFormatRange*) {
            return Tui::ZTextStyle{Tui::Colors::brightWhite, Tui::Colors::red};
        });
        QVector<QChar> text1 = {
            0, 1, 0x7f, 0xdc00, 0xdc10
        };
        QString text2 = U8("\U0001d173");
        layout->setText(QString(text1.data(), text1.size()) + text2 + "A\t\n\tAbcDあ😎😎");

        layout->setTextOption(to);
        layout->doLayout(15);

        struct TestCase { int sourceLine; int line; int cursor; int fixedCursor; int leading; int trailing; };

        auto testCase = GENERATE(
                                TestCase{__LINE__, 0,  0,  0,  0,  2},
                                TestCase{__LINE__, 0,  1,  1,  2,  4},
                                TestCase{__LINE__, 0,  2,  2,  4,  6},
                                TestCase{__LINE__, 0,  3,  3,  6, 10},
                                TestCase{__LINE__, 0,  4,  4, 10, 14},
                                TestCase{__LINE__, 0,  5,  5, 14, 14}, // after last char on line
                                TestCase{__LINE__, 0,  6,  5, 14, 14}, // out of range
                                //
                                TestCase{__LINE__, 1,  4,  5,  0, 10}, // out of range
                                TestCase{__LINE__, 1,  5,  5,  0, 10},
                                TestCase{__LINE__, 1,  6,  5,  0, 10}, // moves to 5
                                TestCase{__LINE__, 1,  7,  7, 10, 11},
                                TestCase{__LINE__, 1,  8,  8, 11, 15},
                                TestCase{__LINE__, 1,  9,  9, 15, 15},
                                TestCase{__LINE__, 1, 10, 10, 15, 15}, // after \n on line
                                //
                                TestCase{__LINE__, 2,  9, 10,  0,  8}, // out of range
                                TestCase{__LINE__, 2, 10, 10,  0,  8},
                                TestCase{__LINE__, 2, 11, 11,  8,  9},
                                TestCase{__LINE__, 2, 12, 12,  9, 10},
                                TestCase{__LINE__, 2, 13, 13, 10, 11},
                                TestCase{__LINE__, 2, 14, 14, 11, 12},
                                TestCase{__LINE__, 2, 15, 15, 12, 14},
                                TestCase{__LINE__, 2, 16, 16, 14, 14}, // after last char on line
                                TestCase{__LINE__, 2, 17, 16, 14, 14}, // out of range
                                //
                                TestCase{__LINE__, 3, 15, 16,  0,  2}, // out of range
                                TestCase{__LINE__, 3, 16, 16,  0,  2},
                                TestCase{__LINE__, 3, 17, 16,  0,  2}, // moves to 16
                                TestCase{__LINE__, 3, 18, 18,  2,  4},
                                TestCase{__LINE__, 3, 19, 18,  2,  4}, // moves to 18
                                TestCase{__LINE__, 3, 20, 20,  4,  4},
                                TestCase{__LINE__, 3, 21, 20,  4,  4}  // out of range

        );
        CAPTURE(testCase.sourceLine);
        int fixedPos = testCase.cursor;
        CHECK(layout->lineAt(testCase.line).cursorToX(testCase.cursor, Tui::ZTextLayout::Edge::Leading) == testCase.leading);
        CHECK(layout->lineAt(testCase.line).cursorToX(testCase.cursor, Tui::ZTextLayout::Edge::Trailing) == testCase.trailing);
        CHECK(layout->lineAt(testCase.line).cursorToX(&fixedPos, Tui::ZTextLayout::Edge::Leading) == testCase.leading);
        CHECK(fixedPos == testCase.fixedCursor);
        CHECK(layout->lineAt(testCase.line).cursorToX(&fixedPos, Tui::ZTextLayout::Edge::Trailing) == testCase.trailing);
        CHECK(fixedPos == testCase.fixedCursor);
    }

    SECTION("cursorToX-newline") {
        layout->setText("a\n\n");
        layout->doLayout(15);
        int pos;
        pos = 1;
        CHECK(layout->lineAt(0).cursorToX(pos, Tui::ZTextLayout::Edge::Leading) == 1);
        CHECK(pos == 1);
        CHECK(layout->lineAt(0).cursorToX(pos, Tui::ZTextLayout::Edge::Trailing) == 1);
        CHECK(pos == 1);
        pos = 2;
        CHECK(layout->lineAt(1).cursorToX(pos, Tui::ZTextLayout::Edge::Leading) == 0);
        CHECK(pos == 2);
        CHECK(layout->lineAt(1).cursorToX(pos, Tui::ZTextLayout::Edge::Trailing) == 0);
        CHECK(pos == 2);
    }

    SECTION("x-cursor-consistency") {
        for (int i = 0; i <= 0x2af; i++ ) { // ascii + latin
            layout->setText(layout->text() + QString(QChar(i)));
        }
        QVector<QChar> text1 = {
            0xdc00
        };
        QString text2 = U8("\U0001d173");
        layout->setText(layout->text() + QString(text1.data(), text1.size()) + text2 + "あ");

        for (int i = 0x1f600; i <= 0x1f64f; i++) {
            layout->setText(layout->text() + (QString(1, QChar::highSurrogate(i))+QString(1, QChar::lowSurrogate(i))));
        }
        layout->doLayout(78);
        Tui::ZTextLineRef line;
        int pos;
        for (int i = 0; i < layout->text().size(); i++ ) {
            CAPTURE(layout->text().at(i).unicode());
            line = layout->lineForTextPosition(i);
            CAPTURE(line.lineNumber());
            int cursor = i;
            pos = line.cursorToX(&cursor, Tui::ZTextLayout::Edge::Leading);
            CAPTURE(pos);
            CHECK(line.xToCursor(pos) == cursor);
        }
    }


    SECTION("softwrapmarker") {
        Tui::ZImage zi{t.terminal.get(), 18, 7};
        layout->setText("11 22\n 333 4444 5555\n 6666\n 777 88 99 00\n");
        layout->doLayout(15);
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue);

        QVector<QPoint> array = {
            {5, 1},
            {1, 2},
            {14, 2},
            {1, 3},
            {5, 3},
            {1, 4},
        };

        for (int y = 0; y < zi.height(); y++) {
            for (int x = 0; x < zi.width(); x++) {
                INFO("x: " << x << " y: " << y);
                CHECK(zi.peekSoftwrapMarker(x, y) == array.contains({x, y}));
            }
        }
    }

    SECTION("createLine-after-doLayout") {
        // CreateLine after finished layout returns invalid ZTextLineRef
        layout->setText("ha\tllo \n"
                    "123456789012345\n"
                    "ABC"
                    );
        layout->doLayout(15);
        Tui::ZTextLineRef tlr1 = layout->createLine();
        tlr1.setLineWidth(100);
        CHECK(tlr1.isValid() == false);
        CHECK(tlr1.textLength() == 0);
    }

    SECTION("doLayout-0") {
        layout->setText("Hallo\nc++");
        layout->doLayout(0);
        CHECK(layout->lineAt(0).isValid() == true);
        CHECK(layout->lineAt(0).width() == 1);
        CHECK(layout->lineAt(0).height() == 1);
        CHECK(layout->lineAt(0).cursorToX(0, Tui::ZTextLayout::Edge::Leading) == 0);
        CHECK(layout->lineAt(0).cursorToX(0, Tui::ZTextLayout::Edge::Trailing) == 1);
    }


    SECTION("TextLineRef-draw-line-1") {
        layout->setText("one\ntwo");
        layout->doLayout(30);

        Tui::ZTextLineRef tlr1 = layout->lineAt(0);
        tlr1.draw(zi.painter(), {1, 1}, styleBWhiteBlue, styleBWhiteBlue, {});
        t.compare(zi);
    }

    SECTION("TextLineRef-draw-line-2") {
        layout->setText("one\ntwo");
        layout->doLayout(30);

        Tui::ZTextLineRef tlr2 = layout->lineAt(1);
        tlr2.draw(zi.painter(), {1, 1}, styleBWhiteBlue, styleBWhiteBlue, {});
        t.compare(zi);
    }

    SECTION("TextLineRef-draw-line-1-2") {
        layout->setText("one\ntwo");
        layout->doLayout(30);

        Tui::ZTextLineRef tlr1 = layout->lineAt(0);
        tlr1.draw(zi.painter(), {1, 1}, styleBWhiteBlue, styleBWhiteBlue, {});
        Tui::ZTextLineRef tlr2 = layout->lineAt(1);
        tlr2.draw(zi.painter(), {1, 1}, styleBWhiteBlue, styleBWhiteBlue, {});
        t.compare(zi);
        zi.painter().clearWithChar({0xff, 0xff, 0xff}, {0, 0, 0}, u'␥');
        layout->draw(zi.painter(), {1, 1}, styleBWhiteBlue, &styleBWhiteBlue, {});
        t.compare(zi);
    }

    SECTION("showCursor") {
        layout->setText("0123456789\nabc");
        layout->doLayout(40);
        Widget w{t.root};
        w.setFocus();
        w.setGeometry({0, 0, 32, 5});
        w.lay = layout.get();

        w.cursorPos = 0;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 0, 1});

        w.cursorPos = 4;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 4, 1});

        w.cursorPos = 5;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 5, 1});

        w.cursorPos = 11;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 0, 1 + 1});

        w.cursorPos = 14;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 3, 1 + 1});

        w.cursorPos = 11;
        w.cursorPos2 = 1000;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 0, 1 + 1});
    }

    SECTION("showCursor-empty") {
        layout->setText("");
        layout->beginLayout();
        layout->createLine().setPosition({4, 2});
        layout->endLayout();

        Widget w{t.root};
        w.setFocus();
        w.setGeometry({0, 0, 32, 5});
        w.lay = layout.get();

        w.cursorPos = 0;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 4, 1 + 2});
    }

    SECTION("showCursor-unlayouted") {
        Widget w{t.root};
        w.setFocus();
        w.setGeometry({0, 0, 32, 5});
        w.lay = layout.get();

        w.cursorPos = 0;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1, 1});
    }

    SECTION("showCursor-newline") {
        layout->setText("abc\n"
                        "def\n");
        layout->doLayout(10);

        Widget w{t.root};
        w.setFocus();
        w.setGeometry({0, 0, 32, 5});
        w.lay = layout.get();

        w.cursorPos = layout->text().size();
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 3, 1 + 1});

        w.cursorPos = 3;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 3, 1 + 0});

        w.cursorPos = 4;
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1 + 0, 1 + 1});
    }

}
