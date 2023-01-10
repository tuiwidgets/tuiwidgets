// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWindow.h>
#include <Tui/ZTerminal.h>

#include "../catchwrapper.h"

#include "../Testhelper.h"

namespace {

class TestWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

public:
    void paintEvent(Tui::ZPaintEvent *event) override {
        Tui::ZPainter *painter = event->painter();
        paint(painter);
    }

    std::function<void(Tui::ZPainter *painter)> paint = [](Tui::ZPainter *painter) {
        painter->setCursor(12, 3);
    };
};
}

TEST_CASE("painter-setCursor", "") {
    Testhelper t("unused", "unused", 15, 5);

    SECTION("no-widget") {
        // this has no effect, but must not crash
        t.terminal->painter().setCursor(0, 0);
    }

    SECTION("full") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({0, 0, 15, 5});

        w->setFocus();
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == 12);
        CHECK(cursorPos.y() == 3);
    }

    SECTION("with-clip") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({0, 0, 15, 5});

        w->setFocus();
        w->paint = [](Tui::ZPainter *painter) {
            Tui::ZPainter clipped = painter->translateAndClip({0, 0, 13, 4});
            clipped.setCursor(12, 3);
        };
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == 12);
        CHECK(cursorPos.y() == 3);
    }

    SECTION("clipped-x") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({0, 0, 15, 5});

        w->setFocus();
        w->paint = [](Tui::ZPainter *painter) {
            Tui::ZPainter clipped = painter->translateAndClip({0, 0, 12, 4});
            clipped.setCursor(12, 3);
        };
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == -1);
        CHECK(cursorPos.y() == -1);
    }

    SECTION("clipped-y") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({0, 0, 15, 5});

        w->setFocus();
        w->paint = [](Tui::ZPainter *painter) {
            Tui::ZPainter clipped = painter->translateAndClip({0, 0, 13, 3});
            clipped.setCursor(12, 3);
        };
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == -1);
        CHECK(cursorPos.y() == -1);
    }

    SECTION("negative-offset") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({0, 0, 15, 5});

        w->setFocus();
        w->paint = [](Tui::ZPainter *painter) {
            Tui::ZPainter transformed = painter->translateAndClip({-2, -2, 13, 4});
            transformed.setCursor(12, 3);
        };
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == 10);
        CHECK(cursorPos.y() == 1);
    }


    SECTION("offset") {
        TestWidget *w = new TestWidget(t.root);
        w->setGeometry({2, 1, 13, 4});

        w->setFocus();
        t.render();

        QPoint cursorPos = t.terminal->grabCursorPosition();

        CHECK(cursorPos.x() == 14);
        CHECK(cursorPos.y() == 4);
    }
}
