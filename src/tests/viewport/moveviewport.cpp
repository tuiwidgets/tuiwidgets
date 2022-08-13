
#include <Tui/ZWindow.h>
#include <Tui/ZTerminal.h>

#include "../catchwrapper.h"

#include "../Testhelper.h"

TEST_CASE("viewport-resize", "") {

    Testhelper t("viewport", "viewport-resize", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    SECTION("empty") {
        t.compare();
    }

    SECTION("empty-15-5") {
        t.root->setMinimumSize(15, 5);
        t.compare("empty");
    }

    SECTION("empty-16-5") {
        t.root->setMinimumSize(16, 5);
        t.compare("initial-prompt");
    }

    SECTION("empty-15-6") {
        t.root->setMinimumSize(15, 6);
        t.compare("initial-prompt");
    }

    SECTION("empty-16-6") {
        t.root->setMinimumSize(16, 6);
        t.compare("initial-prompt");
    }

    SECTION("empty-1-1") {
        t.root->setMinimumSize(-1,-1);
        t.compare("empty");
    }
}

TEST_CASE("viewport-f6", "") {

    Testhelper t("viewport", "viewport-f6", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});
    w->setFocusPolicy(Qt::StrongFocus);
    w->setFocus();

    Tui::ZWindow *w2 = new Tui::ZWindow(t.root);
    w2->setGeometry({4, 2, 4, 2});
    w2->setFocusPolicy(Qt::StrongFocus);

    // Check that F6 activates view port scroll mode
    SECTION("press-16-6") {
        t.root->setMinimumSize(16, 6);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.compare("movement-prompt");
    }

    // Check that when root widget fits viewport F6 is not intercepted by viewport logic and switches active window.
    SECTION("press-15-5") {
        t.root->setMinimumSize(15, 5);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.compare("second-window-focus");
    }

    SECTION("esc-press-16-6") {
        t.root->setMinimumSize(16, 6);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.sendKeyToZTerminal("ESC");
        t.compare("initial-prompt");
    }

    SECTION("f6-press-16-5") {
        t.root->setMinimumSize(16, 5);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.sendKeyToZTerminal("F6");
        t.compare("initial-prompt-second-window-focus");
    }
}

TEST_CASE("viewport-move", "") {

    Testhelper t("viewport", "viewport-move", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 16, 6});

    SECTION("top-right") {
        t.root->setMinimumSize(16, 6);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.sendKeyToZTerminal("↑");
        t.sendKeyToZTerminal("→");
        t.compare();
    }

    SECTION("left-down") {
        t.root->setMinimumSize(16, 6);
        t.render();
        t.sendKeyToZTerminal("F6");
        t.sendKeyToZTerminal("←");
        t.sendKeyToZTerminal("↓");
        t.compare();
    }
}


// TODO:
// Currently, cursor tests are not possible because the cursor is not listed in the TPI files.
