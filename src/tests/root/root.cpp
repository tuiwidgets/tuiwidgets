// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZRoot.h>

#include <QSet>

#include <Tui/ZEvent.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWindowFacet.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../signalrecorder.h"
#include "../vcheck_zwidget.h"

TEST_CASE("root-base") {
    Tui::ZRoot root;
    CHECK(root.fillChar() == 127);

    SECTION("abi-vcheck") {
        Tui::ZWidget base;
        checkZWidgetOverrides(&base, &root);
    }

    SECTION("fillChar") {
        char space = ' ';
        root.setFillChar((int)space);
        CHECK(root.fillChar() == 32);
    }

    root.activateNextWindow();
    root.activatePreviousWindow();
    Tui::ZWidget w;
    root.raiseOnActivate(&w);

    CHECK(root.sizeHint().height() == -1);
    CHECK(root.sizeHint().width() == -1);
    CHECK(root.maximumSize() == QSize{16777215, 16777215});
    CHECK(root.minimumSize() == QSize{40, 7});
    CHECK(root.minimumSizeHint() == QSize{-1, -1});
    CHECK(root.layoutArea() == QRect{0, 0, 0, 0});

    FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(&root, DefaultException::Palette | DefaultException::MinimumSize));
}

namespace {
class RootStubWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;
    QObject *facet(const QMetaObject &metaObject) const override {
        if (metaObject.className() == Tui::ZWindowFacet::staticMetaObject.className()) {
            return &windowFacet;
        } else {
            return Tui::ZWidget::facet(metaObject);
        }
    }

    mutable TestZWindowFacet windowFacet;
};
}

TEST_CASE("root-resizeEvent") {

    Testhelper t("unsued", "unused", 15, 5);
    Tui::ZRoot root;
    root.setParent(t.root);

    RootStubWidget sw(&root);

    // Add a non ZWidget object to check if that confuses anything
    QObject nonWidget(&root);

    // Add a non window widget to check if that confuses anything
    Tui::ZWidget nonWindow(&root);
    nonWindow.setGeometry({0, 0, 1, 1});

    t.render();
    CHECK(sw.windowFacet.autoPlaceSelf == nullptr);

    auto placed = GENERATE(true, false);
    CAPTURE(placed);
    sw.windowFacet.setManuallyPlaced(placed);

    auto containered = GENERATE(true, false);
    CAPTURE(containered);
    Tui::ZWindowContainer zwc;
    if (containered) {
        sw.windowFacet.setContainer(&zwc);
    }
    CHECK(sw.windowFacet.autoPlaceCount == 0);

    auto mained = GENERATE(true, false);
    CAPTURE(mained);
    if (mained) {
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        if (placed || containered) {
            CHECK(sw.windowFacet.autoPlaceCount == 0);
        } else {
            CHECK(sw.windowFacet.autoPlaceCount == 1);
        }
    }
    sw.windowFacet.autoPlaceCount = 0;

    if (placed || containered) {
        root.setGeometry({0, 0, 10, 11});
        CHECK(sw.windowFacet.autoPlaceSelf == nullptr);
        sw.setGeometry({0, 0, 15, 16});
        CHECK(sw.windowFacet.geometry.width() == -1);
        CHECK(sw.windowFacet.geometry.height() == -1);
        CHECK(sw.windowFacet.autoPlaceSelf == nullptr);
        root.setGeometry({0, 0, 12, 13});
        CHECK(sw.windowFacet.geometry.width() == -1);
        CHECK(sw.windowFacet.geometry.height() == -1);
        CHECK(sw.windowFacet.autoPlaceSelf == nullptr);
        root.setGeometry({0, 0, -1, -1});
        CHECK(sw.windowFacet.geometry.width() == -1);
        CHECK(sw.windowFacet.geometry.height() == -1);
        CHECK(sw.windowFacet.autoPlaceSelf == nullptr);
        root.setGeometry({-1, -1, 1, 1});
        CHECK(sw.windowFacet.geometry.width() == -1);
        CHECK(sw.windowFacet.geometry.height() == -1);
        CHECK(sw.windowFacet.autoPlaceSelf == nullptr);
        CHECK(sw.windowFacet.autoPlaceCount == 0);
    } else {
        root.setGeometry({0, 0, 10, 11});
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        sw.setGeometry({0, 0, 15, 16});
        CHECK(sw.windowFacet.autoPlaceCount == 1);
        sw.windowFacet.autoPlaceCount = 0;
        CHECK(sw.windowFacet.geometry.width() == 10);
        CHECK(sw.windowFacet.geometry.height() == 11);
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        root.setGeometry({0, 0, 12, 13});
        CHECK(sw.windowFacet.autoPlaceCount == 1);
        sw.windowFacet.autoPlaceCount = 0;
        CHECK(sw.windowFacet.geometry.width() == 12);
        CHECK(sw.windowFacet.geometry.height() == 13);
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        root.setGeometry({0, 0, 50, 50});
        CHECK(sw.windowFacet.autoPlaceCount == 1);
        sw.windowFacet.autoPlaceCount = 0;
        CHECK(sw.windowFacet.geometry.width() == 50);
        CHECK(sw.windowFacet.geometry.height() == 50);
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        root.setGeometry({0, 0, -1, -1});
        CHECK(sw.windowFacet.autoPlaceCount == 1);
        sw.windowFacet.autoPlaceCount = 0;
        CHECK(sw.windowFacet.geometry.width() == 0);
        CHECK(sw.windowFacet.geometry.height() == 0);
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        root.setGeometry({-1, -1, 1, 1});
        CHECK(sw.windowFacet.autoPlaceCount == 1);
        sw.windowFacet.autoPlaceCount = 0;
        CHECK(sw.windowFacet.geometry.width() == 1);
        CHECK(sw.windowFacet.geometry.height() == 1);
        CHECK(sw.windowFacet.autoPlaceSelf == &sw);
        CHECK(sw.windowFacet.autoPlaceCount == 0);
    }
}

TEST_CASE("root-viewport") {

    Testhelper t("unsued", "unused", 25, 25);

    Tui::ZRoot root;
    root.setParent(t.root);
    root.setGeometry({0, 0, 25, 25});
    RootStubWidget sw(&root);

    // Add a non ZWidget object to check if that confuses anything
    QObject nonWidget(&root);

    // Add a non window widget to check if that confuses anything
    Tui::ZWidget nonWindow(&root);
    nonWindow.setGeometry({0, 0, 1, 1});

    CHECK(root.minimumSizeHint() == QSize{-1, -1});
    root.setMinimumSize({}); //invalid
    sw.setGeometry({0, 0, 50, 50});
    sw.windowFacet.setManuallyPlaced(true);
    sw.windowFacet.extendViewport = true;
    CHECK(sw.windowFacet.isExtendViewport() == true);
    CHECK(root.minimumSizeHint() == QSize{50, 50});

    sw.setVisible(false);
    CHECK(sw.windowFacet.isExtendViewport() == true);
    CHECK(root.minimumSizeHint() == QSize{-1, -1});
}

TEST_CASE("root-sendkey-from-root") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    RootStubWidget sw(&root);
    sw.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw.focus() == false);

    root.setFocus();
    CHECK(root.focus() == true);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw.focus() == true);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw.focus() == true);
}

TEST_CASE("root-sendkey-to-root") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    RootStubWidget sw(&root);
    sw.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw.focus() == false);

    sw.setFocus();
    CHECK(sw.focus() == true);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw.focus() == true);
}

TEST_CASE("root-sendkey-to-root3") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    RootStubWidget sw1(&root);
    sw1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    // Add a non ZWidget object to check if that confuses anything
    QObject nonWidget(&root);

    // Add a non window widget to check if that confuses anything
    Tui::ZWidget nonWindow(&root);
    nonWindow.setGeometry({0, 0, 1, 1});

    RootStubWidget sw2(&root);
    sw2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    RootStubWidget sw3(&root);
    sw3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    t.sendKey(Tui::Key_F6);
    CHECK(root.focus() == false);
    CHECK(sw1.focus() == false);
    CHECK(sw2.focus() == false);
    CHECK(sw3.focus() == false);

    root.setFocus();
    SECTION("forward-and-backward") {
        auto usingKey = GENERATE(true, false);
        CAPTURE(usingKey);

        {
            for(int i = 1, r = 0; i <= 13; i++) {
                if (usingKey) {
                    t.sendKey(Tui::Key_F6);
                } else {
                    root.activateNextWindow();
                }

                if (i - r == 4) {
                    r += 3;
                }

                CHECK(root.focus() == false);
                bool c1 = (i - r == 1);
                bool c2 = (i - r == 2);
                bool c3 = (i - r == 3);
                CHECK(sw1.focus() == c1);
                CHECK(sw2.focus() == c2);
                CHECK(sw3.focus() == c3);
            }
        }
        {
            for(int i = 1, r = 0; i <= 12; i++) {
                if (usingKey) {
                    t.sendKey(Tui::Key_F6, Tui::ShiftModifier);
                } else {
                    root.activatePreviousWindow();
                }

                if (i - r == 4) {
                    r += 3;
                }

                CHECK(root.focus() == false);
                bool c1 = (i - r == 3);
                bool c2 = (i - r == 2);
                bool c3 = (i - r == 1);
                CHECK(sw1.focus() == c1);
                CHECK(sw2.focus() == c2);
                CHECK(sw3.focus() == c3);
            }
        }
    }

    SECTION("the-first-one") {
        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == true);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);
    }

    SECTION("remove-window") {
        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == true);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);

        sw1.setParent(nullptr);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == true);
        CHECK(sw3.focus() == false);

        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == true);

        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == true);
        CHECK(sw3.focus() == false);
    }

    SECTION("remove-all-window") {
        t.sendKey(Tui::Key_F6);
        sw3.setParent(nullptr);
        sw2.setParent(nullptr);
        sw1.setParent(nullptr);

        t.sendKey(Tui::Key_F6);
        //Starting from here, nothing has focus
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);
    }

    SECTION("unfocusable") {
        int policy = GENERATE(0, 1, 2, 3, 4);
        CAPTURE(policy);
        if (policy == 0) {
            sw3.setFocusPolicy(Tui::FocusPolicy::NoFocus);
        } else if (policy == 1) {
            sw3.setParent(nullptr);
        } else if (policy == 2) {
            sw3.setEnabled(false);
        } else if (policy == 3) {
            sw3.setVisible(false);
        } else if (policy == 4) {
            sw3.setFocusPolicy(Tui::FocusPolicy::ClickFocus);
        }

        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == true);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);

        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == true);
        CHECK(sw3.focus() == false);

        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == true);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);
    }

    SECTION("change-focuspolicy") {
        Tui::FocusPolicy policy = GENERATE(Tui::FocusPolicy::TabFocus, Tui::FocusPolicy::WheelFocus, Tui::FocusPolicy::StrongFocus);
        CAPTURE(policy);
        sw3.setFocusPolicy(policy);
        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == true);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);

        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == true);
        CHECK(sw3.focus() == false);

        t.sendKey(Tui::Key_F6);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == true);
    }

    SECTION("add-window") {
        //We go through all the window so that we are at the top of the list.
        t.sendKey(Tui::Key_F6);
        t.sendKey(Tui::Key_F6);
        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == true);

        //We create a new window.
        RootStubWidget sw4(&root);
        sw4.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        //Since it is at the end of the list, it becomes active next.
        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == false);
        CHECK(sw3.focus() == false);
        CHECK(sw4.focus() == true);

        //We delete the next window.
        sw1.setParent(nullptr);
        t.sendKey(Tui::Key_F6);
        CHECK(root.focus() == false);
        CHECK(sw1.focus() == false);
        CHECK(sw2.focus() == true);
        CHECK(sw3.focus() == false);
        CHECK(sw4.focus() == false);

    }
}

TEST_CASE("root-terminalChanged") {
    Testhelper t("unsued", "unused", 15, 5);
    CHECK(t.root->terminalChangedTrigger == true);

    SECTION("manual-event") {
        t.root->terminalChangedTrigger = false;
        auto e = QEvent(Tui::ZEventType::terminalChange());
        t.app->sendEvent(t.root, &e);
        CHECK(t.root->terminalChangedTrigger == false);
    }

    SECTION("switch to new terminal") {
        auto terminal2 = std::make_unique<Tui::ZTerminal>(Tui::ZTerminal::OffScreen{2, 2});
        t.root->terminalChangedTrigger = false;
        terminal2->setMainWidget(t.root);
        CHECK(t.root->terminalChangedTrigger == true);
        t.root->terminalChangedTrigger = false;
        terminal2->setMainWidget(t.root);
        CHECK(t.root->terminalChangedTrigger == false);
    }

    SECTION("initial terminal") {
        RootStub root2;
        auto terminal2 = std::make_unique<Tui::ZTerminal>(Tui::ZTerminal::OffScreen{2, 2});
        root2.terminalChangedTrigger = false;
        terminal2->setMainWidget(&root2);
        CHECK(root2.terminalChangedTrigger == true);
    }
}

TEST_CASE("root-keyEventBubbles") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    RootStubWidget sw(&root);
    sw.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

    sw.setFocus();
    auto mod = GENERATE(Tui::NoModifier, Tui::ShiftModifier);
    CAPTURE(mod);
    FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_F5, mod));
    root.setFocus();
    FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_F5, mod));
    FAIL_CHECK_VEC(t.checkCharEventBubbles("A", mod));
}

TEST_CASE("root-pasteEventBubbles") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    root.setFocus();

    FAIL_CHECK_VEC(t.checkPasteEventBubbles("a text is sent here"));
}

TEST_CASE("root-window") {

    Testhelper t("root", "root-window", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);

    SECTION("setGeometry-1x1") {
        int i = (int)'#';
        root.setFillChar(i);
        root.setGeometry({1, 1, 1, 1});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("setGeometry-268435455x268435455") {
        int i = (int)'#';
        root.setFillChar(i);
        root.setGeometry({0, 0, 0xFFFFFFF, 0xFFFFFFF});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("setGeometry-4x4") {
        int i = (int)'#';
        root.setFillChar(i);
        root.setGeometry({-1, -1, 4, 4});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("fillChar-#") {
        int i = (int)'#';
        root.setFillChar(i);
        root.setGeometry({1, 1, 5, 5});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("fillChar-0xad") {
        int i = 0xad;
        root.setFillChar(i);
        root.setGeometry({1, 1, 5, 5});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("fillChar-0x80") {
        //Characters smaller than 32 and between 0x7f - 0xa0 are not displayed.
        int i = 0x80;
        root.setFillChar(i);
        root.setGeometry({1, 1, 5, 5});
        CHECK(root.fillChar() == i);
        t.compare();
    }

    SECTION("fillChar-disable") {
        int i = (int)'#';
        root.setFillChar(i);
        root.setGeometry({1, 1, 5, 5});
        root.setEnabled(false);
        t.compare("fillChar-#");
    }
}

TEST_CASE("root-layoutArea") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    t.root->setMinimumSize(QSize{40, 7}); // Default: must be set because testhelper modifies this.
    root.setGeometry({1, 1, 14, 4});

    struct TestCase { int l; int t; int r; int b; };
    auto cm = GENERATE(
                            TestCase{0, 0, 0, 0},
                            TestCase{1, 0, 0, 0},
                            TestCase{0, 1, 0, 0},
                            TestCase{0, 0, 1, 0},
                            TestCase{0, 0, 0, 1},
                            TestCase{-1, -1, -1, -1}
    );
    CAPTURE(cm.l);
    CAPTURE(cm.t);
    CAPTURE(cm.r);
    CAPTURE(cm.b);

    int cmh = cm.l + cm.r;
    int cmv = cm.t + cm.b;

    t.root->setContentsMargins(QMargins{cm.l, cm.t, cm.r, cm.b});

    SECTION("outer is main, base") {
        t.terminal->setMainWidget(t.root);
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 14, 4}); //not main
        CHECK(t.root->layoutArea() == QRect{0 + cm.l, 0 + cm.t, 40 - cmh, 7 - cmv});
    }

    SECTION("inner is main, base") {
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 40, 7});
        CHECK(t.root->layoutArea() == QRect{0 + cm.l, 0 + cm.t, 15 - cmh, 5 - cmv});
    }

    SECTION("inner not main, render cycle does not change layout area") {
        CHECK(root.layoutArea() == QRect{0, 0, 14, 4});
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 14, 4});
    }

    SECTION("outer is main with expanding") {
        RootStubWidget sw(t.root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);
        t.terminal->setMainWidget(t.root);
        t.render();
        CHECK(t.root->geometry() == QRect{0, 0, 50, 50});
        CHECK(t.root->layoutArea() == QRect{0 + cm.l, 0 + cm.t, 40 - cmh, 7 - cmv});
    }

    SECTION("inner is main with expanding") {
        RootStubWidget sw(&root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        t.render();
        CHECK(root.geometry() == QRect{0, 0, 50, 50});
        CHECK(root.layoutArea() == QRect{0, 0, 40, 7});
    }

    SECTION("outer is main, not expanding inner") {
        RootStubWidget sw(&root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);

        t.render();
        CHECK(root.geometry() == QRect{1, 1, 14, 4});
        CHECK(root.layoutArea() == QRect{0, 0, 14, 4});
    }
}

TEST_CASE("root-layoutArea-minimum") {

    Testhelper t("unsued", "unused", 15, 5);

    Tui::ZRoot root;
    root.setParent(t.root);
    t.root->setMinimumSize(QSize{3, 3});
    root.setMinimumSize(QSize{3, 3});

    root.setGeometry({1, 1, 2, 2});

    SECTION("outer is main") {
        t.terminal->setMainWidget(t.root);
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 2, 2});
        CHECK(t.root->layoutArea() == QRect{0, 0, 15, 5});
    }

    SECTION("inner is main") {
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 15, 5});
        CHECK(t.root->layoutArea() == QRect{0, 0, 15, 5});
    }

    SECTION("inner not main, render cycle does not change layout area") {
        CHECK(root.layoutArea() == QRect{0, 0, 2, 2});
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 2, 2});
    }

    SECTION("inner is main, inner layout area == terminal size, outer as set") {
        t.root->setGeometry({1, 1, 2, 2});
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        t.render();
        CHECK(root.layoutArea() == QRect{0, 0, 15, 5});
        CHECK(t.root->layoutArea() == QRect{0, 0, 2, 2});
    }

    SECTION("outer is main with expanding in outer") {
        RootStubWidget sw(t.root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);
        t.terminal->setMainWidget(t.root);
        t.render();
        CHECK(t.root->geometry() == QRect{0, 0, 50, 50});
        CHECK(t.root->layoutArea() == QRect{0, 0, 15, 5});
    }

    SECTION("inner is main with expanding") {
        // Add a non ZWidget object to check if that confuses anything
        QObject nonWidget(&root);

        // Add a non window widget to check if that confuses anything
        Tui::ZWidget nonWindow(&root);
        nonWindow.setGeometry({0, 0, 1, 1});

        RootStubWidget sw(&root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);
        // remove from current widget tree
        root.setParent(nullptr);
        t.terminal->setMainWidget(&root);
        t.render();
        CHECK(root.geometry() == QRect{0, 0, 50, 50});
        CHECK(root.layoutArea() == QRect{0, 0, 15, 5});
    }

    SECTION("outer is main, not expanding inner") {
        RootStubWidget sw(&root);
        sw.setGeometry({0, 0, 50, 50});
        sw.windowFacet.extendViewport = true;
        CHECK(sw.windowFacet.isExtendViewport() == true);

        t.render();
        CHECK(root.geometry() == QRect{1, 1, 2, 2});
        CHECK(root.layoutArea() == QRect{0, 0, 2, 2});
    }
}


TEST_CASE("raiseOnActivate") {
    Testhelper t("unsued", "unused", 15, 5);

    SECTION("raiseOnActivate in called") {
        auto usingKey = GENERATE(true, false);
        CAPTURE(usingKey);

        t.root->setFocus();
        CHECK(t.root->raiseCount == 0);

        RootStubWidget sw1(t.root);
        sw1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        if (usingKey) {
            t.sendKey(Tui::Key_F6);
        } else {
            t.root->activateNextWindow();
        }
        CHECK(sw1.focus());
        CHECK(t.root->raiseCount == 1);
        t.root->raiseCount = 0;
        CHECK(t.root->raiseWidget == &sw1);

        RootStubWidget sw2(t.root);
        sw2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        if (usingKey) {
            t.sendKey(Tui::Key_F6);
        } else {
            t.root->activateNextWindow();
        }
        CHECK(sw2.focus());
        CHECK(t.root->raiseCount == 1);
        t.root->raiseCount = 0;
        CHECK(t.root->raiseWidget == &sw2);

        RootStubWidget sw3(t.root);
        sw3.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        if (usingKey) {
            t.sendKey(Tui::Key_F6);
        } else {
            t.root->activateNextWindow();
        }
        CHECK(sw3.focus());
        CHECK(t.root->raiseCount == 1);
        t.root->raiseCount = 0;
        CHECK(t.root->raiseWidget == &sw3);

        if (usingKey) {
            t.sendKey(Tui::Key_F6);
        } else {
            t.root->activateNextWindow();
        }
        CHECK(sw1.focus());
        CHECK(t.root->raiseCount == 1);
        t.root->raiseCount = 0;
        CHECK(t.root->raiseWidget == &sw1);
    }

    SECTION("raiseOnActivate raises") {
        bool disableRaise = GENERATE(false, true);
        CAPTURE(disableRaise);
        t.root->disableRaiseOnFocus = disableRaise;
        RootStubWidget sw1(t.root);
        sw1.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        RootStubWidget sw2(t.root);
        sw2.setFocusPolicy(Tui::FocusPolicy::StrongFocus);

        t.root->activateNextWindow();

        int count = 0;
        int posSw1 = -1;
        int posSw2 = -1;
        for (auto ch: t.root->children()) {
            if (ch == &sw1) {
                posSw1 = count;
                ++count;
            }
            if (ch == &sw2) {
                posSw2 = count;
                ++count;
            }
        }
        if (disableRaise) {
            CHECK(posSw1 == 0);
            CHECK(posSw2 == 1);
        } else {
            CHECK(posSw1 == 1);
            CHECK(posSw2 == 0);
        }
    }

}
