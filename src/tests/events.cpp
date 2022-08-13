// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZEvent.h>

#include <QSet>

#include <Tui/ZPainter.h>
#include <Tui/ZSymbol.h>

#include "catchwrapper.h"

#include "Testhelper.h"

TEST_CASE("ZRawSequenceEvent") {
    SECTION("pending") {
        Tui::ZRawSequenceEvent e{Tui::ZRawSequenceEvent::pending, "something"};
        CHECK(e.type() == Tui::ZEventType::pendingRawSequence());
        CHECK(e.sequence() == "something");
    }

    SECTION("not pending") {
        Tui::ZRawSequenceEvent e{"something2"};
        CHECK(e.type() == Tui::ZEventType::rawSequence());
        CHECK(e.sequence() == "something2");
    }
}

TEST_CASE("ZTerminalNativeEvent") {
    int dummyEventPlaceholder;
    Tui::ZTerminalNativeEvent e{&dummyEventPlaceholder};
    CHECK(e.type() == Tui::ZEventType::terminalNativeEvent());
    CHECK(e.nativeEventPointer() == &dummyEventPlaceholder);
}

TEST_CASE("ZPaintEvent") {
    Testhelper t("unused", "unused", 80, 50);
    Tui::ZPainter painter = t.terminal->painter();

    auto checkPainter = [&t](Tui::ZPainter p) {
        // Check that painter didn't gain unexpected transformation or clipping
        p.writeWithColors(0, 0, "A", Tui::Colors::red, Tui::Colors::black);
        p.writeWithColors(79, 49, "B", Tui::Colors::red, Tui::Colors::black);
        auto terminalImage = t.terminal->grabCurrentImage();
        CHECK(terminalImage.peekText(0, 0, nullptr, nullptr) == "A");
        CHECK(terminalImage.peekText(79, 49, nullptr, nullptr) == "B");
    };

    SECTION("paint") {
        Tui::ZPaintEvent e{&painter};
        CHECK(e.type() == Tui::ZEventType::paint());
        checkPainter(*e.painter());
    }

    SECTION("update request") {
        Tui::ZPaintEvent e{Tui::ZPaintEvent::update, &painter};
        CHECK(e.type() == Tui::ZEventType::updateRequest());
        checkPainter(*e.painter());
    }
}

TEST_CASE("ZKeyEvent") {
    struct TestCase { int key; Qt::KeyboardModifiers modifiers; QString text; };

    auto testCase = GENERATE(TestCase{Qt::Key_Escape, Qt::NoModifier, ""},
                             TestCase{Qt::Key_Space, Qt::ControlModifier, ""},
                             TestCase{Qt::Key_Backspace, Qt::NoModifier, ""},
                             TestCase{Qt::Key_Enter, Qt::NoModifier, ""},
                             TestCase{Qt::Key_unknown, Qt::ShiftModifier, "B"},
                             TestCase{Qt::Key_unknown, Qt::NoModifier, "ä"},
                             TestCase{Qt::Key_unknown, Qt::NoModifier, "😎"},
                             TestCase{Qt::Key_unknown, Qt::NoModifier, "あ"},
                             TestCase{Qt::Key_unknown, Qt::NoModifier, "a\u0308"});
    CAPTURE(testCase.key);
    CAPTURE(testCase.modifiers);
    CAPTURE(testCase.text);

    Tui::ZKeyEvent e{testCase.key, testCase.modifiers, testCase.text};
    CHECK(e.type() == Tui::ZEventType::key());
    CHECK(e.key() == testCase.key);
    CHECK(e.modifiers() == testCase.modifiers);
    CHECK(e.text() == testCase.text);
}

TEST_CASE("ZPasteEvent") {
    Tui::ZPasteEvent e{"paste text"};
    CHECK(e.type() == Tui::ZEventType::paste());
}

TEST_CASE("ZFocusEvent") {
    SECTION("focus-in") {
        Tui::ZFocusEvent e{Tui::ZFocusEvent::focusIn};
        CHECK(e.type() == Tui::ZEventType::focusIn());
        CHECK(e.reason() == Qt::OtherFocusReason);
    }
    SECTION("focus-in tab focus") {
        Tui::ZFocusEvent e{Tui::ZFocusEvent::focusIn, Qt::TabFocusReason};
        CHECK(e.type() == Tui::ZEventType::focusIn());
        CHECK(e.reason() == Qt::TabFocusReason);
    }
    SECTION("focus-out") {
        Tui::ZFocusEvent e{Tui::ZFocusEvent::focusOut};
        CHECK(e.type() == Tui::ZEventType::focusOut());
        CHECK(e.reason() == Qt::OtherFocusReason);
    }
    SECTION("focus-out tab focus") {
        Tui::ZFocusEvent e{Tui::ZFocusEvent::focusOut, Qt::TabFocusReason};
        CHECK(e.type() == Tui::ZEventType::focusOut());
        CHECK(e.reason() == Qt::TabFocusReason);
    }
}

TEST_CASE("ZResizeEvent") {
    Tui::ZResizeEvent e{QSize{2, 4}, QSize{5, 9}};
    CHECK(e.type() == Tui::ZEventType::resize());
    CHECK(e.size() == QSize{2, 4});
    CHECK(e.oldSize() == QSize{5, 9});
}

TEST_CASE("ZMoveEvent") {
    Tui::ZMoveEvent e{QPoint{2, 4}, QPoint{5, 9}};
    CHECK(e.type() == Tui::ZEventType::move());
    CHECK(e.pos() == QPoint{2, 4});
    CHECK(e.oldPos() == QPoint{5, 9});
}

TEST_CASE("ZCloseEvent") {
    SECTION("no skip") {
        Tui::ZCloseEvent e{QStringList()};
        CHECK(e.type() == Tui::ZEventType::close());
        CHECK(e.skipChecks() == QStringList());
    }

    SECTION("skip blah") {
        Tui::ZCloseEvent e{QStringList("blah")};
        CHECK(e.type() == Tui::ZEventType::close());
        CHECK(e.skipChecks() == QStringList("blah"));
    }
}

TEST_CASE("ZOtherChangeEvent") {
    SECTION("empty set") {
        Tui::ZOtherChangeEvent e{QSet<Tui::ZSymbol>()};
        CHECK(e.type() == Tui::ZEventType::otherChange());
        CHECK(e.unchanged() == QSet<Tui::ZSymbol>());
    }

    SECTION("all") {
        Tui::ZOtherChangeEvent e{Tui::ZOtherChangeEvent::all()};
        CHECK(e.type() == Tui::ZEventType::otherChange());
        CHECK(e.unchanged() == Tui::ZOtherChangeEvent::all());
    }

    SECTION("two item set") {
        Tui::ZOtherChangeEvent e{QSet<Tui::ZSymbol>({Tui::ZSymbol("custom"), Tui::ZSymbol("thing")})};
        CHECK(e.type() == Tui::ZEventType::otherChange());
        CHECK(e.unchanged() == QSet<Tui::ZSymbol>({Tui::ZSymbol("custom"), Tui::ZSymbol("thing")}));
    }

}
