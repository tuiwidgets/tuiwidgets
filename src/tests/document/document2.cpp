// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocumentCursor.h>
#include <Tui/ZDocumentLineMarker.h>
#include <Tui/ZDocumentSnapshot.h>

#include <random>
#include <array>

#include <QBuffer>
#include <QCoreApplication>

#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextOption.h>

#include "../catchwrapper.h"
#include "../eventrecorder.h"
#include "../Testhelper.h"

#include "../vcheck_qobject.h"

static QVector<QString> docToVec(const Tui::ZDocument &doc) {
    QVector<QString> ret;

    for (int i = 0; i < doc.lineCount(); i++) {
        ret.append(doc.line(i));
    }

    return ret;
}

static QVector<QString> snapToVec(const Tui::ZDocumentSnapshot &snap) {
    QVector<QString> ret;

    for (int i = 0; i < snap.lineCount(); i++) {
        ret.append(snap.line(i));
    }

    return ret;
}

TEST_CASE("ZDocument2") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    Tui::ZDocumentCursor wrappedCursor{&doc, [&textMetrics, &doc](int line, bool wrappingAllowed) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            if (wrappingAllowed) {
                Tui::ZTextOption option;
                option.setWrapMode(Tui::ZTextOption::WrapMode::WordWrap);
                lay.setTextOption(option);
            }
            lay.doLayout(40);
            return lay;
        }
    };

    SECTION("moveCharacterLeft") {
        cursor.insertText("test\ntest test\ntest test");

        SECTION("start of non first line") {
            cursor.setPosition({0, 1});
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 0});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line, no movement") {
            cursor.setPosition({0, 0});
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, remove selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, keep selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveCharacterLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
        }

        SECTION("not at the start of a line") {
            cursor.setPosition({2, 2});
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.verticalMovementColumn() == 1);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line with selection, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.verticalMovementColumn() == 1);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line, create and extend selection") {
            cursor.setPosition({4, 2});
            cursor.moveCharacterLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.verticalMovementColumn() == 3);
            cursor.moveCharacterLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
            CHECK(cursor.verticalMovementColumn() == 2);
        }

        SECTION("astral and wide") {
            cursor.insertText("\nabc😁あdef");

            cursor.setPosition({7, 3});
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 3});
            CHECK(cursor.verticalMovementColumn() == 7);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 3});
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 3});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);
        }
        SECTION("astral up down") {
            cursor.insertText("a\n😁\naあ");

            cursor.setPosition({1, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
            CHECK(cursor.verticalMovementColumn() == 1);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.verticalMovementColumn() == 1);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.verticalMovementColumn() == 1);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.verticalMovementColumn() == 1);

            //
            cursor.moveToEndOfLine();
            CAPTURE(cursor.position().line);
            CAPTURE(cursor.position().codeUnit);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1}); // das kann in keinem fall 9 sein
            CHECK(cursor.verticalMovementColumn() == 9);

            //cursor.moveDown();
            //CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});

        }
    }


    SECTION("moveCharacterLeft with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna

        SECTION("Move over soft line break") {
            wrappedCursor.setPosition({76, 0});
            wrappedCursor.moveCharacterLeft();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{75, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            wrappedCursor.moveCharacterLeft();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{74, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            // the cursor is placed before the space, so vertical movement column does not include the space.
            CHECK(wrappedCursor.verticalMovementColumn() == 34);
        }
    }


    SECTION("moveWordLeft") {
        cursor.insertText("test\ntest test\ntest test");

        SECTION("start of non first line") {
            cursor.setPosition({0, 1});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 0});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line, no movement") {
            cursor.setPosition({0, 0});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, remove selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, keep selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveWordLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
        }

        SECTION("not at the start of a line, in first word") {
            cursor.setPosition({2, 1});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line, in second word") {
            cursor.setPosition({9, 1});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 1});
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line with selection, in second word, remove selection") {
            cursor.setPosition({9, 1});
            cursor.setPosition({8, 1});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 1});
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line, create and extend selection") {
            cursor.setPosition({9, 2});
            cursor.moveWordLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 2});
            CHECK(cursor.verticalMovementColumn() == 5);
            cursor.moveWordLeft(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
        }

        SECTION("astral and wide") {
            cursor.insertText("\nabc 😁 あ def");

            cursor.setPosition({11, 3});
            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 3});
            CHECK(cursor.verticalMovementColumn() == 10);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 3});
            CHECK(cursor.verticalMovementColumn() == 7);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 3});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordLeft();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }
    }

    SECTION("moveWordLeft with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna

        SECTION("Move over soft line break") {
            wrappedCursor.setPosition({80, 0});
            wrappedCursor.moveWordLeft();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{75, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            wrappedCursor.moveWordLeft();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{68, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 28);
        }
    }


    SECTION("moveCharacterRight") {
        cursor.insertText("test test\ntest test\ntest");

        SECTION("end of non last line") {
            cursor.setPosition({9, 1});
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line, no movement") {
            cursor.setPosition({4, 2});
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line with selection, no movement, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({4, 2}, true);
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line with selection, no movement, keep selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({4, 2}, true);
            cursor.moveCharacterRight(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
        }

        SECTION("not at the end of a line") {
            cursor.setPosition({2, 2});
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line with selection, remove selection") {
            cursor.setPosition({1, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line, create and extend selection") {
            cursor.setPosition({1, 2});
            cursor.moveCharacterRight(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
            CHECK(cursor.verticalMovementColumn() == 2);
            cursor.moveCharacterRight(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.verticalMovementColumn() == 3);
        }

        SECTION("astral and wide") {
            cursor.insertText("\nabc😁あdef");

            cursor.setPosition({2, 3});
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 3});
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 3});
            CHECK(cursor.verticalMovementColumn() == 7);
            CHECK(cursor.hasSelection() == false);
            cursor.moveCharacterRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 3});
            CHECK(cursor.verticalMovementColumn() == 8);
            CHECK(cursor.hasSelection() == false);
        }
    }


    SECTION("moveCharacterRight with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna

        SECTION("Move over soft line break") {
            wrappedCursor.setPosition({73, 0});
            wrappedCursor.moveCharacterRight();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{74, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 34);
            wrappedCursor.moveCharacterRight();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{75, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
        }
    }


    SECTION("moveWordRight") {
        cursor.insertText("test test\ntest test\ntest");

        SECTION("end of non last line") {
            cursor.setPosition({9, 1});
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line, no movement") {
            cursor.setPosition({4, 2});
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line with selection, no movement, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({4, 2}, true);
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line") {
            cursor.setPosition({2, 1});
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line with selection, remove selection") {
            cursor.setPosition({1, 1});
            cursor.setPosition({2, 2}, true);
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line, create and extend selection") {
            cursor.setPosition({1, 1});
            cursor.moveWordRight(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
            CHECK(cursor.verticalMovementColumn() == 4);
            cursor.moveWordRight(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 9);
        }

        SECTION("astral and wide") {
            cursor.insertText("\nabc 😁 あ def");

            cursor.setPosition({0, 3});
            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 3});
            CHECK(cursor.verticalMovementColumn() == 6);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{8, 3});
            CHECK(cursor.verticalMovementColumn() == 9);
            CHECK(cursor.hasSelection() == false);

            cursor.moveWordRight();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{12, 3});
            CHECK(cursor.verticalMovementColumn() == 13);
            CHECK(cursor.hasSelection() == false);
        }
    }


    SECTION("moveWordRight with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna


        SECTION("Move over soft line break") {
            wrappedCursor.setPosition({70, 0});
            wrappedCursor.moveWordRight();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{74, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 34);
            wrappedCursor.moveWordRight();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{83, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 8);
        }
    }


    SECTION("moveToStartOfLine") {
        cursor.insertText("test\ntest test\ntest test");

        SECTION("start of line, no movement") {
            cursor.setPosition({0, 1});
            cursor.moveToStartOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of line with selection, no movement, remove selection") {
            cursor.setPosition({1, 1});
            cursor.setPosition({0, 1}, true);
            cursor.moveToStartOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line") {
            cursor.setPosition({2, 2});
            cursor.moveToStartOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line with selection, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveToStartOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line, create and keep selection") {
            cursor.setPosition({4, 2});
            cursor.moveToStartOfLine(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
            cursor.moveToStartOfLine(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
            CHECK(cursor.verticalMovementColumn() == 0);
        }
    }

    SECTION("moveToStartOfLine with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna

        SECTION("soft wraps are ignored") {
            wrappedCursor.setPosition({80, 0});
            wrappedCursor.moveToStartOfLine();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
        }
    }


    SECTION("moveToStartIndentedText") {

        cursor.insertText("test\n   test test\n\t\ttest test\n  \ttest\n      ");

        SECTION("no indent") {
            SECTION("start of line, no movement") {
                cursor.setPosition({0, 0});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of line with selection, no movement, remove selection") {
                cursor.setPosition({1, 0});
                cursor.setPosition({0, 0}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("not at the start of a line") {
                cursor.setPosition({2, 0});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("not at the start of a line with selection, remove selection") {
                cursor.setPosition({3, 0});
                cursor.setPosition({2, 0}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("not at the start of a line, create and keep selection") {
                cursor.setPosition({4, 0});
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 0});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 0});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor.verticalMovementColumn() == 0);
            }
        }

        SECTION("space indent") {
            SECTION("start of line") {
                cursor.setPosition({0, 1});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent, no movement") {
                cursor.setPosition({3, 1});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent with selection, no movement, remove selection") {
                cursor.setPosition({4, 1});
                cursor.setPosition({3, 1}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indent") {
                cursor.setPosition({1, 1});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text") {
                cursor.setPosition({5, 1});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text with selection, remove selection") {
                cursor.setPosition({6, 1});
                cursor.setPosition({5, 1}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text, create and keep selection") {
                cursor.setPosition({5, 1});
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 1});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 1});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
                CHECK(cursor.verticalMovementColumn() == 3);
            }
        }

        SECTION("tab indent") {
            SECTION("start of line") {
                cursor.setPosition({0, 2});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent, no movement") {
                cursor.setPosition({2, 2});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent with selection, no movement, remove selection") {
                cursor.setPosition({3, 2});
                cursor.setPosition({2, 2}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indent") {
                cursor.setPosition({1, 2});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text") {
                cursor.setPosition({5, 2});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text with selection, remove selection") {
                cursor.setPosition({6, 2});
                cursor.setPosition({5, 2}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text, create and keep selection") {
                cursor.setPosition({5, 2});
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 2});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 2});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 2});
                CHECK(cursor.verticalMovementColumn() == 16);
            }
        }

        SECTION("mixed indent") {
            SECTION("start of line") {
                cursor.setPosition({0, 3});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent, no movement") {
                cursor.setPosition({3, 3});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent with selection, no movement, remove selection") {
                cursor.setPosition({4, 3});
                cursor.setPosition({3, 3}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indent") {
                cursor.setPosition({1, 3});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text") {
                cursor.setPosition({5, 3});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text with selection, remove selection") {
                cursor.setPosition({6, 3});
                cursor.setPosition({5, 3}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indented text, create and keep selection") {
                cursor.setPosition({5, 3});
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 3});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
                cursor.moveToStartIndentedText(true);
                CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 3});
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});
                CHECK(cursor.verticalMovementColumn() == 8);
            }
        }

        SECTION("all spaces") {
            SECTION("start of line") {
                cursor.setPosition({0, 4});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 4});
                CHECK(cursor.verticalMovementColumn() == 6);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent, no movement") {
                cursor.setPosition({3, 4});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 4});
                CHECK(cursor.verticalMovementColumn() == 6);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("start of indent with selection, no movement, remove selection") {
                cursor.setPosition({5, 4});
                cursor.setPosition({6, 4}, true);
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 4});
                CHECK(cursor.verticalMovementColumn() == 6);
                CHECK(cursor.hasSelection() == false);
            }

            SECTION("in indent") {
                cursor.setPosition({1, 4});
                cursor.moveToStartIndentedText();
                CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 4});
                CHECK(cursor.verticalMovementColumn() == 6);
                CHECK(cursor.hasSelection() == false);
            }
        }
    }


    SECTION("moveToStartIndentedText with wrap") {
        SECTION("soft wraps are ignored") {
            cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
            // 000|Lorem ipsum dolor sit amet, consectetur
            // 040|adipisici elit, sed eiusmod tempor
            // 075|incidunt ut labore et dolore magna

            wrappedCursor.setPosition({80, 0});
            wrappedCursor.moveToStartIndentedText();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
        }

        SECTION("very long indent") {
            cursor.insertText("                                           pisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
            // line 0 is lots of spaces
            // 040|   pisici elit, sed eiusmod tempor
            // 075|incidunt ut labore et dolore magna

            wrappedCursor.setPosition({80, 0});
            wrappedCursor.moveToStartIndentedText();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{43, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 3);
        }
    }


    SECTION("moveToEndOfLine") {
        cursor.insertText("test\ntest test\ntest test");

        SECTION("end of line, no movement") {
            cursor.setPosition({9, 1});
            cursor.moveToEndOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 9);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of line with selection, no movement, remove selection") {
            cursor.setPosition({8, 1});
            cursor.setPosition({9, 1}, true);
            cursor.moveToEndOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 9);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line") {
            cursor.setPosition({2, 2});
            cursor.moveToEndOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.verticalMovementColumn() == 9);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line with selection, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveToEndOfLine();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.verticalMovementColumn() == 9);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line, create and keep selection") {
            cursor.setPosition({4, 2});
            cursor.moveToEndOfLine(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.verticalMovementColumn() == 9);
            cursor.moveToEndOfLine(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});
            CHECK(cursor.verticalMovementColumn() == 9);
        }
    }


    SECTION("moveToEndOfLine with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna

        SECTION("soft wraps are ignored") {
            wrappedCursor.setPosition({20, 0});
            wrappedCursor.moveToEndOfLine();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{109, 0});
            CHECK(wrappedCursor.hasSelection() == false);
            CHECK(wrappedCursor.verticalMovementColumn() == 34);
        }
    }


    SECTION("moveUp") {
        cursor.insertText("short\nlong line\nlongest line\n\nsome line");

        SECTION("first line") {
            cursor.setPosition({3, 0});
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("first line with selection") {
            cursor.setPosition({2, 0});
            cursor.setPosition({3, 0}, true);
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("vertical movement 7") {
            cursor.setPosition({7, 1});
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 0});
            CHECK(cursor.verticalMovementColumn() == 7);

            cursor.setPosition({7, 4});
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 7);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 2});
            CHECK(cursor.verticalMovementColumn() == 7);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 1});
            CHECK(cursor.verticalMovementColumn() == 7);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 0});
            CHECK(cursor.verticalMovementColumn() == 7);
        }

        SECTION("vertical movement 12") {
            cursor.setPosition({12, 2});
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 12);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 0});
            CHECK(cursor.verticalMovementColumn() == 12);
        }

        SECTION("vertical movement 100") {
            cursor.setPosition({7, 4});
            cursor.setVerticalMovementColumn(100);
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{12, 2});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 0});
            CHECK(cursor.verticalMovementColumn() == 100);
        }
    }


    SECTION("moveUp with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit in voluptate velit esse cillum dolore eu");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna
        // 110|aliqua. Ut enim ad minim veniam, quis
        // 148|nostrud exercitation ullamco laboris
        // 185|nisi ut aliquid ex ea commodi consequat.
        // 225| Quis aute iure reprehenderit in
        // 258|voluptate velit esse cillum dolore eu

        SECTION("intra line with wrapping - vert 0") {
            wrappedCursor.setPosition({258, 0});
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{225, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{185, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{148, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{110, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{75, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{40, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
        }

        SECTION("intra line with wrapping - vert 39") {
            wrappedCursor.setPosition({224, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{184, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{147, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{109, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{74, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{39, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
        }
    }


    SECTION("moveUp with wrap - wide and astral") {
        cursor.insertText("L😁rem ipsum d😁l😁r sit あmet, c😁nsectetur あdipisici elit, sed eiusm😁d temp😁r incidunt ut lあb😁re et d😁l😁re mあgnあ あliquあ. Ut enim あd minim veniあm, quis n😁strud exercitあti😁n ullあmc😁 lあb😁ris nisi ut あliquid ex eあ c😁mm😁di c😁nsequあt. Quis あute iure reprehenderit in v😁luptあte velit");
        //    |break ->                                v
        // 000|L😁rem ipsum d😁l😁r sit あmet, c😁
        // 034|nsectetur あdipisici elit, sed eiusm😁d
        // 073|temp😁r incidunt ut lあb😁re et d😁l😁re
        // 112| mあgnあ あliquあ. Ut enim あd minim
        // 144|veniあm, quis n😁strud exercitあti😁n
        // 180|ullあmc😁 lあb😁ris nisi ut あliquid ex
        // 217|eあ c😁mm😁di c😁nsequあt. Quis あute
        // 252|iure reprehenderit in v😁luptあte velit

        SECTION("intra line with wrapping - vert 0") {
            wrappedCursor.setPosition({252, 0});
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{217, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{180, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{144, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{112, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{73, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{34, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
        }

        SECTION("intra line with wrapping - vert 20") {
            wrappedCursor.setPosition({252 + 20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{217 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{180 + 18, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{144 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{112 + 16, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{73 + 20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{34 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
        }
    }


    SECTION("moveDown") {
        cursor.insertText("short\nlong line\nlongest line\n\nsome line");

        SECTION("last line") {
            cursor.setPosition({3, 4});
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 4});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("last line with selection") {
            cursor.setPosition({2, 4});
            cursor.setPosition({3, 4}, true);
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 4});
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("vertical movement 7") {
            cursor.setPosition({7, 1});
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 2});
            CHECK(cursor.verticalMovementColumn() == 7);
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 7);
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{7, 4});
            CHECK(cursor.verticalMovementColumn() == 7);
        }

        SECTION("vertical movement 12") {
            cursor.setPosition({12, 2});
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 12);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 4});
            CHECK(cursor.verticalMovementColumn() == 12);
        }

        SECTION("vertical movement 100") {
            cursor.setPosition({0, 0});
            cursor.setVerticalMovementColumn(100);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{12, 2});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
            CHECK(cursor.verticalMovementColumn() == 100);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 4});
            CHECK(cursor.verticalMovementColumn() == 100);
        }
    }


    SECTION("moveDown with wrap") {
        cursor.insertText("Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit in voluptate velit esse cillum dolore eu");
        // 000|Lorem ipsum dolor sit amet, consectetur
        // 040|adipisici elit, sed eiusmod tempor
        // 075|incidunt ut labore et dolore magna
        // 110|aliqua. Ut enim ad minim veniam, quis
        // 148|nostrud exercitation ullamco laboris
        // 185|nisi ut aliquid ex ea commodi consequat.
        // 225| Quis aute iure reprehenderit in
        // 258|voluptate velit esse cillum dolore eu

        SECTION("intra line with wrapping - vert 0") {
            wrappedCursor.setPosition({0, 0});
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{40, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{75, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{110, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{148, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{185, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{225, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{258, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
        }

        SECTION("intra line with wrapping - vert 39") {
            wrappedCursor.setPosition({39, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{74, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{109, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{147, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{184, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{224, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{257, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{295, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 39);
            CHECK(wrappedCursor.hasSelection() == false);
        }
    }


    SECTION("moveDown with wrap - wide and astral") {
        cursor.insertText("L😁rem ipsum d😁l😁r sit あmet, c😁nsectetur あdipisici elit, sed eiusm😁d temp😁r incidunt ut lあb😁re et d😁l😁re mあgnあ あliquあ. Ut enim あd minim veniあm, quis n😁strud exercitあti😁n ullあmc😁 lあb😁ris nisi ut あliquid ex eあ c😁mm😁di c😁nsequあt. Quis あute iure reprehenderit in v😁luptあte velit");
        //    |break ->                                v
        // 000|L😁rem ipsum d😁l😁r sit あmet, c😁
        // 034|nsectetur あdipisici elit, sed eiusm😁d
        // 073|temp😁r incidunt ut lあb😁re et d😁l😁re
        // 112| mあgnあ あliquあ. Ut enim あd minim
        // 144|veniあm, quis n😁strud exercitあti😁n
        // 180|ullあmc😁 lあb😁ris nisi ut あliquid ex
        // 217|eあ c😁mm😁di c😁nsequあt. Quis あute
        // 252|iure reprehenderit in v😁luptあte velit

        SECTION("intra line with wrapping - vert 0") {
            wrappedCursor.setPosition({0, 0});
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{34, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{73, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{112, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{144, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{180, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{217, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{252, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 0);
            CHECK(wrappedCursor.hasSelection() == false);
        }

        SECTION("intra line with wrapping - vert 20") {
            wrappedCursor.setPosition({20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{34 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{73 + 20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{112 + 16, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{144 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{180 + 18, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{217 + 19, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{252 + 20, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 20);
            CHECK(wrappedCursor.hasSelection() == false);
        }
    }


    SECTION("moveToStartOfDocument") {
        cursor.insertText("test\ntest test\ntest test");

        SECTION("start of non first line") {
            cursor.setPosition({0, 1});
            cursor.moveToStartOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line, no movement") {
            cursor.setPosition({0, 0});
            cursor.moveToStartOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, remove selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveToStartOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("start of first line with selection, no movement, keep selection") {
            cursor.setPosition({1, 0});
            cursor.setPosition({0, 0}, true);
            cursor.moveToStartOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
        }

        SECTION("not at the start of a line") {
            cursor.setPosition({2, 2});
            cursor.moveToStartOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line with selection, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveToStartOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the start of a line, create and keep selection") {
            cursor.setPosition({4, 2});
            cursor.moveToStartOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
            cursor.moveToStartOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 0);
        }
    }


    SECTION("moveToEndOfDocument") {
        cursor.insertText("test test\ntest test\ntest");

        SECTION("end of non last line") {
            cursor.setPosition({9, 1});
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line, no movement") {
            cursor.setPosition({4, 2});
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line with selection, no movement, remove selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({4, 2}, true);
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("end of last line with selection, no movement, keep selection") {
            cursor.setPosition({3, 2});
            cursor.setPosition({4, 2}, true);
            cursor.moveToEndOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
        }

        SECTION("not at the end of a line") {
            cursor.setPosition({2, 2});
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line with selection, remove selection") {
            cursor.setPosition({1, 2});
            cursor.setPosition({2, 2}, true);
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("not at the end of a line, create and keep selection") {
            cursor.setPosition({1, 2});
            cursor.moveToEndOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
            cursor.moveToEndOfDocument(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
            CHECK(cursor.verticalMovementColumn() == 4);
        }

        SECTION("astral and wide") {
            cursor.insertText("\nabc😁あdef");

            cursor.setPosition({2, 2});
            cursor.moveToEndOfDocument();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 3});
            CHECK(cursor.verticalMovementColumn() == 10);
        }
    }

    SECTION("moveUpDown") {
        cursor.insertText("1234\n1234");

        SECTION("up-down-verticalMovementColumn") {
            cursor.setPosition({2, 0});
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("selection") {
            cursor.setPosition({2, 1});
            cursor.setPosition({2, 0}, true);
            cursor.moveUp(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == true);

            cursor.moveDown(true);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.moveDown(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == true);

            cursor.moveUp(true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == true);
        }

        SECTION("reset-selection") {
            cursor.setPosition({2, 1});
            cursor.setPosition({2, 0}, true);
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.setPosition({2, 1});
            cursor.setPosition({2, 0}, true);
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.setPosition({2, 0});
            cursor.setPosition({2, 1}, true);
            cursor.moveDown();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);

            cursor.setPosition({2, 0});
            cursor.setPosition({2, 1}, true);
            cursor.moveUp();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(cursor.verticalMovementColumn() == 2);
            CHECK(cursor.hasSelection() == false);
        }

        SECTION("wrap") {
            for(int i = 0; i < 85; i++)
                cursor.insertText("a");

            wrappedCursor.setPosition({2, 0});
            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);

            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{42, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);

            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{82, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);

            wrappedCursor.moveDown();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{89, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);

            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{42, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);

            wrappedCursor.moveUp();
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);
        }

        SECTION("wrap-selected") {
            for(int i = 0; i < 85; i++)
                cursor.insertText("a");

            wrappedCursor.setPosition({2, 0});
            wrappedCursor.moveDown(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveDown(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{42, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveDown(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{82, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveDown(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{89, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveUp(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{42, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveUp(true);
            CHECK(wrappedCursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == true);

            wrappedCursor.moveUp(true);
            CHECK(wrappedCursor.position() == Tui::ZDocumentCursor::Position{2, 0});
            CHECK(wrappedCursor.verticalMovementColumn() == 2);
            CHECK(wrappedCursor.hasSelection() == false);
        }

    }
}

TEST_CASE("Cursor") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };
    cursor1.insertText(" \n \n \n \n");

    Tui::ZDocumentCursor cursor2{&doc, [textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{0, 4});
    CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{0, 0});


    SECTION("delete-previous-line") {
        cursor2.setPosition({0, 3});
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{0, 3});
        CHECK(doc.lineCount() == 5);
        cursor2.deleteCharacter();
        cursor2.deleteCharacter();
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{0, 3});
        CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{0, 3});
        CHECK(doc.lineCount() == 4);
    }

    SECTION("delete-on-cursor") {
        cursor1.setPosition({0, 2});
        cursor2.setPosition({0, 2});
        cursor1.deleteCharacter();
        CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{0, 2});
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{0, 2});
        cursor1.deletePreviousCharacter();
        CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{1, 1});
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{1, 1});
    }

    SECTION("sort") {
        cursor1.selectAll();
        cursor1.insertText("3\n4\n2\n1");
        cursor1.setPosition({0, 1});
        cursor1.moveCharacterRight(true);
        CHECK(cursor1.hasSelection() == true);
        CHECK(cursor1.selectedText() == "4");

        cursor2.selectAll();
        CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{1, 1});
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{1, 3});

        doc.sortLines(0, 4, &cursor2);
        CHECK(doc.lineCount() == 4);
        REQUIRE(doc.line(0) == "1");
        REQUIRE(doc.line(1) == "2");
        REQUIRE(doc.line(2) == "3");
        REQUIRE(doc.line(3) == "4");

        CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{1, 3});
        CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor1.hasSelection() == true);
        CHECK(cursor1.selectedText() == "4");
        CHECK(cursor2.hasSelection() == true);
    }


    SECTION("select") {
        cursor1.setPosition({0, 2});
        cursor1.insertText("hallo welt");

        bool selectionDirection = GENERATE(false, true);
        CAPTURE(selectionDirection);

        if (selectionDirection) {
            cursor1.moveToStartOfLine(true);
        } else {
            auto pos = cursor1.position();
            cursor1.moveToStartOfLine();
            cursor1.setPosition(pos, true);
        }

        CHECK(cursor1.selectedText() == "hallo welt");

        SECTION("hasSelection") {
            CHECK(cursor2.selectedText() == "");
            CHECK(cursor2.hasSelection() == false);
        }

        SECTION("in-selection") {
            cursor2.setPosition({5, 2});

            SECTION("insert") {
                cursor2.insertText("a");
                CHECK(cursor1.selectedText() == "halloa welt");
            }
            SECTION("insert-newline") {
                cursor2.insertText("\n");
                CHECK(cursor1.selectedText() == "hallo\n welt");
            }
            SECTION("tab") {
                cursor2.insertText("\t");
                CHECK(cursor1.selectedText() == "hallo\t welt");
            }
            SECTION("del") {
                cursor2.deleteCharacter();
                CHECK(cursor1.selectedText() == "hallowelt");
            }
            SECTION("del-word") {
                cursor2.deleteWord();
                CHECK(cursor1.selectedText() == "hallo");
            }
            SECTION("del-line") {
                cursor2.deleteLine();
                CHECK(cursor1.selectedText() == "");
                CHECK(cursor1.hasSelection() == false);
            }
            SECTION("del-previous") {
                cursor2.deletePreviousCharacter();
                CHECK(cursor1.selectedText() == "hall welt");
            }
            SECTION("del-previous-word") {
                cursor2.deletePreviousWord();
                CHECK(cursor1.selectedText() == " welt");
            }
            SECTION("del-previous-line") {
                cursor2.setPosition({0, 2});
                cursor2.deletePreviousCharacter();
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("del-all") {
                cursor2.selectAll();
                cursor2.deletePreviousCharacter();
                CHECK(cursor1.selectedText() == "");
                CHECK(cursor1.hasSelection() == false);
                CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{0, 0});
                CHECK(cursor2.position() == Tui::ZDocumentCursor::Position{0, 0});
            }
        }

        SECTION("before-selection") {
            cursor2.setPosition({0, 2});

            SECTION("insert") {
                cursor2.insertText("a");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("insert-newline") {
                cursor2.insertText("\n");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("tab") {
                cursor2.insertText("\t");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("del") {
                cursor2.deleteCharacter();
                CHECK(cursor1.selectedText() == "allo welt");
            }
            SECTION("del-word") {
                cursor2.deleteWord();
                CHECK(cursor1.selectedText() == " welt");
            }
        }
        SECTION("first-position-of-selection") {
            cursor2.setPosition({1, 2});

            SECTION("insert") {
                cursor2.insertText("a");
                CHECK(cursor1.selectedText() == "haallo welt");
            }
            SECTION("insert-newline") {
                cursor2.insertText("\n");
                CHECK(cursor1.selectedText() == "h\nallo welt");
            }
            SECTION("tab") {
                cursor2.insertText("\t");
                CHECK(cursor1.selectedText() == "h\tallo welt");
            }
            SECTION("del") {
                cursor2.deleteCharacter();
                CHECK(cursor1.selectedText() == "hllo welt");
            }
            SECTION("del-word") {
                cursor2.deleteWord();
                CHECK(cursor1.selectedText() == "h welt");
            }
        }

        SECTION("after-selection") {
            cursor2.setPosition({10, 2});

            SECTION("insert") {
                cursor2.insertText("a");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("insert-newline") {
                cursor2.insertText("\n");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("tab") {
                cursor2.insertText("\t");
                CHECK(cursor1.selectedText() == "hallo welt");
            }
            SECTION("del") {
                cursor2.deletePreviousCharacter();
                CHECK(cursor1.selectedText() == "hallo wel");
            }
            SECTION("del-word") {
                cursor2.deletePreviousWord();
                CHECK(cursor1.selectedText() == "hallo ");
            }
        }
        SECTION("last-position-of-selection") {
            cursor2.setPosition({9, 2});

            SECTION("insert") {
                cursor2.insertText("a");
                CHECK(cursor1.selectedText() == "hallo welat");
            }
            SECTION("insert-newline") {
                cursor2.insertText("\n");
                CHECK(cursor1.selectedText() == "hallo wel\nt");
            }
            SECTION("tab") {
                cursor2.insertText("\t");
                CHECK(cursor1.selectedText() == "hallo wel\tt");
            }
            SECTION("del") {
                cursor2.deletePreviousCharacter();
                CHECK(cursor1.selectedText() == "hallo wet");
            }
            SECTION("del-word") {
                cursor2.deletePreviousWord();
                CHECK(cursor1.selectedText() == "hallo t");
            }
        }

        SECTION("marker") {
            Tui::ZDocumentLineMarker marker(&doc);
            CHECK(marker.line() == 0);

            // will move after generating, not active
            Tui::ZDocumentLineMarker marker2(&doc);
            CHECK(marker2.line() == 0);

            // wrong marker
            marker.setLine(45);
            CHECK(marker.line() == 4);
            cursor1.setPosition({0, 0});
            cursor1.insertText("\n");
            CHECK(marker.line() == 5);
            CHECK(marker2.line() == 1);

            // cursor1 add lines
            marker.setLine(5);
            CHECK(marker.line() == 5);
            cursor1.setPosition({0, 5});
            cursor1.insertText(" ");
            CHECK(cursor1.position() == Tui::ZDocumentCursor::Position{1, 5});
            cursor1.insertText("\n");
            CHECK(marker.line() == 5);
            CHECK(marker2.line() == 1);

            // wrong marker
            marker.setLine(-1);
            CHECK(marker.line() == 0);
            cursor1.setPosition({0, 0});
            cursor1.insertText("\n");
            CHECK(marker.line() == 1);
            CHECK(marker2.line() == 2);

            // cursor1 add and remove line
            marker.setLine(1);
            CHECK(marker.line() == 1);
            cursor1.setPosition({0, 0});
            cursor1.insertText("\n");
            CHECK(marker.line() == 2);
            CHECK(marker2.line() == 3);

            // cursor2 add and remove line
            cursor2.setPosition({0, 0});
            cursor2.insertText("\n");
            CHECK(marker.line() == 3);
            cursor2.deleteLine();
            CHECK(marker.line() == 2);

            // delete the line with marker
            cursor1.setPosition({0, 2});
            cursor1.deleteLine();
            CHECK(marker.line() == 2);

            // delete the lines with marker
            cursor2.setPosition({0, 1});
            cursor2.setPosition({0, 3}, true);
            cursor2.insertText(" ");
            CHECK(marker.line() == 1);

            // selectAll and delete
            cursor2.selectAll();
            cursor2.insertText(" ");
            CHECK(marker.line() == 0);
            CHECK(marker2.line() == 0);
        }
    }
}
