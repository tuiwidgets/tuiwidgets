#include <Tui/ZWindow.h>

#include <../../third-party/catch.hpp>

#include <QRect>

#include <Tui/ZColor.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWidget.h>

#include "../Testhelper.h"

class TestBackgroundWidget : public Tui::ZWidget {
public:
    explicit TestBackgroundWidget(Tui::ZWidget *parent) : Tui::ZWidget(parent) {}

protected:
    void paintEvent(Tui::ZPaintEvent *event) override {
        Tui::ZWidget::paintEvent(event);
        auto *painter = event->painter();
        int w = geometry().width();
        int h = geometry().height();
        for(int i = 0; i <= h; i++) {
            painter->writeWithColors(0, i, QStringLiteral("␥").repeated(w),
                                     Tui::ZColor::fromRgb(0xfe, 0xfd, 0xfc), Tui::ZColor::fromRgb(0x23, 0x23, 0x23));
        }
    }
};

TEST_CASE("window-base", "") {

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    SECTION("no parent") {
        delete new Tui::ZWindow();
        delete new Tui::ZWindow("some title");
    }

    Tui::ZWidget parentWidget;

    Tui::ZWindow w(parent ? &parentWidget : nullptr);

    SECTION("defaults") {
        CHECK(w.windowTitle() == "");
        CHECK(w.options() == Tui::ZWindow::Options{});
        CHECK(w.borderEdges() == (Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge | Qt::LeftEdge));
        CHECK(w.focusMode() == Tui::FocusContainerMode::Cycle);
        CHECK(w.paletteClass() == QStringList{"window"});
        CHECK(w.sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(w.sizePolicyV() == Tui::SizePolicy::Expanding);
        Tui::ZWindow w2("Breakpoints");
        CHECK(w2.windowTitle() == "Breakpoints");
    }

    SECTION("get-set-title") {
        w.setWindowTitle("some title");
        CHECK(w.windowTitle() == "some title");
    }

    SECTION("get-set-options") {
        w.setOptions({});
        CHECK(w.options() == Tui::ZWindow::Options{});
        w.setOptions(Tui::ZWindow::CloseButton);
        CHECK(w.options() == Tui::ZWindow::CloseButton);
    }

    SECTION("get-set-borderEdges") {
        w.setBorderEdges({});
        CHECK(w.borderEdges() == Qt::Edges{});
        w.setBorderEdges(Qt::Edge::TopEdge);
        CHECK(w.borderEdges() == Qt::Edge::TopEdge);
    }
}

TEST_CASE("window-signals", "") {
    Testhelper t("window", "unused", 15, 5);

    bool withParent = GENERATE(false, true);
    CAPTURE(withParent);

    Tui::ZWindow w(withParent ? t.root : nullptr);

    SECTION("window title") {
        const QString inactive = "inactive placeholer";
        QString expectedTitleParameter = inactive;

        QObject::connect(&w, &Tui::ZWindow::windowTitleChanged, [&] (QString titleParameter) {
            if (expectedTitleParameter == inactive) {
                FAIL("unexpected emit of windowTitleChanged signal with parameter: " << titleParameter.toStdString());
            }
            CHECK(titleParameter == expectedTitleParameter);
            expectedTitleParameter = inactive;
        });

        expectedTitleParameter = "Colors";
        w.setWindowTitle("Colors");
        CHECK(expectedTitleParameter == inactive);

        // setting to same value does not emit signal
        expectedTitleParameter = inactive;
        w.setWindowTitle("Colors");
        CHECK(expectedTitleParameter == inactive);

        expectedTitleParameter = "";
        w.setWindowTitle("");
        CHECK(expectedTitleParameter == inactive);
    }
}

TEST_CASE("window-palette", "") {
    Testhelper t("window", "window-palette", 25, 5);
    Tui::ZPalette pal;

    pal.setColors({
                      { "window.frame.focused.bg", Tui::Color::green },
                      { "window.frame.focused.fg", Tui::Color::green },
                      { "window.frame.focused.control.bg", Tui::Color::green },
                      { "window.frame.focused.control.fg", Tui::Color::green },
                      { "window.frame.unfocused.bg", Tui::Color::green },
                      { "window.frame.unfocused.fg", Tui::Color::green },
                  });

    t.root->setPalette(pal);
    TestBackgroundWidget background(t.root);
    background.setGeometry({0, 0, 25, 5});

    auto w = new Tui::ZWindow(&background);
    w->setGeometry({1, 1, 23, 3});
    w->setWindowTitle("Oranges");
    w->setOptions(Tui::ZWindow::CloseButton);

    SECTION("allgreen") {
        t.compare();
    }

    SECTION("focused-allgreen") {
        w->setFocus();
        t.compare();
    }

    SECTION("focused-bg-magenta") {
        w->setFocus();
        pal.setColors({{ "window.frame.focused.bg", Tui::Color::magenta }});
        t.root->setPalette(pal);
        t.compare();
    }

    SECTION("focused-fg-magenta") {
        w->setFocus();
        pal.setColors({{ "window.frame.focused.fg", Tui::Color::magenta }});
        t.root->setPalette(pal);
        t.compare();
    }

    SECTION("focused-control-bg-magenta") {
        w->setFocus();
        pal.setColors({{ "window.frame.focused.control.bg", Tui::Color::magenta }});
        t.root->setPalette(pal);
        t.compare();
    }

    SECTION("focused-control-fg-magenta") {
        w->setFocus();
        pal.setColors({{ "window.frame.focused.control.fg", Tui::Color::magenta }});
        t.root->setPalette(pal);
        t.compare();
    }

    SECTION("unfocused-bg-yellow") {
        pal.setColors({{ "window.frame.unfocused.bg", Tui::Color::yellow }});
        t.root->setPalette(pal);
        t.compare();
    }

    SECTION("unfocused-fg-yellow") {
        pal.setColors({{ "window.frame.unfocused.fg", Tui::Color::yellow }});
        t.root->setPalette(pal);
        t.compare();
    }

}

TEST_CASE("window-visual", "") {
    std::unique_ptr<Testhelper> t;
    Tui::ZWindow *w = nullptr;

    const std::vector<QPoint> close_button_mask = {
        {3, 1}, // "[" of close button
        {4, 1}, // "■" of close button
        {5, 1}, // "]" of close button
    };

    auto tests = [&] {
        SECTION("default") {
            t->compare();
        }

        w->setWindowTitle("Oranges");
        SECTION("withtitle") {
            t->compare();
        }

        SECTION("withtitle-20") {
            w->setGeometry({1, 1, 20, 3});
            t->compare();
        }

        SECTION("withtitle-19") {
            w->setGeometry({1, 1, 19, 3});
            t->compare();
        }

        SECTION("withtitle-18") {
            w->setGeometry({1, 1, 18, 3});
            t->compare();
        }

        SECTION("withtitle-13") {
            w->setGeometry({1, 1, 13, 3});
            t->compare();
        }

        SECTION("withtitle-12") {
            w->setGeometry({1, 1, 12, 3});
            t->compare();
        }

        SECTION("withtitle-11") {
            w->setGeometry({1, 1, 11, 3});
            t->compare();
        }

        SECTION("withtitle-10") {
            w->setGeometry({1, 1, 10, 3});
            t->compare();
        }

        SECTION("withtitle-9") {
            w->setGeometry({1, 1, 9, 3});
            t->compare();
        }

        SECTION("withtitle-8") {
            w->setGeometry({1, 1, 8, 3});
            t->compare();
        }

        SECTION("withtitle-7") {
            w->setGeometry({1, 1, 7, 3});
            t->compare();
        }

        SECTION("withtitle-4") {
            w->setGeometry({1, 1, 4, 3});
            t->compare();
        }

        w->setOptions(Tui::ZWindow::CloseButton);
        SECTION("withtitle-close") {
            t->compare();
            t->crossCheckWithMask({"withtitle"}, close_button_mask);
        }

        // smallest width with space before title
        SECTION("withtitle-close-20") {
            w->setGeometry({1, 1, 20, 3});
            t->compare();
            t->crossCheckWithMask({"withtitle-close-20"}, close_button_mask);
        }

        // alignment matches between variant with and without close button, but no space before title yet
        SECTION("withtitle-close-19") {
            w->setGeometry({1, 1, 19, 3});
            t->compare();
            t->crossCheckWithMask({"withtitle-close-19"}, close_button_mask);
        }

        // alignment matches between variant with and without close button, but no space before title yet
        SECTION("withtitle-close-18") {
            w->setGeometry({1, 1, 18, 3});
            t->compare();
            t->crossCheckWithMask({"withtitle-close-18"}, close_button_mask);
        }

        // smallest width where alignment matches between variant with and without close button.
        SECTION("withtitle-close-17") {
            w->setGeometry({1, 1, 17, 3});
            t->compare();
            t->crossCheckWithMask({"withtitle-close-17"}, close_button_mask);
        }

        // largest width without space after title
        SECTION("withtitle-close-14") {
            w->setGeometry({1, 1, 14, 3});
            t->compare();
        }

        // largest width without h-line before title
        SECTION("withtitle-close-13") {
            w->setGeometry({1, 1, 13, 3});
            t->compare();
        }

        // top right corner goes away
        SECTION("withtitle-close-12") {
            w->setGeometry({1, 1, 12, 3});
            t->compare();
        }

        // clipping of title starts
        SECTION("withtitle-close-11") {
            w->setGeometry({1, 1, 11, 3});
            t->compare();
        }

        // Title clipped to 2 cells, close button still visible
        SECTION("withtitle-close-7") {
            w->setGeometry({1, 1, 7, 3});
            t->compare();
        }

        // Title no longer visible, part of close button clipped
        SECTION("withtitle-close-4") {
            w->setGeometry({1, 1, 4, 3});
            t->compare();
        }

        SECTION("no-border") {
            w->setBorderEdges({});
            t->compare();
        }

        SECTION("top-border") {
            w->setBorderEdges(Qt::TopEdge);
            t->compare();
        }

        SECTION("bottom-border") {
            w->setBorderEdges(Qt::BottomEdge);
            t->compare();
        }

        SECTION("left-border") {
            w->setBorderEdges(Qt::LeftEdge);
            t->compare();
        }

        SECTION("right-border") {
            w->setBorderEdges(Qt::RightEdge);
            t->compare();
        }

        SECTION("top-left-border") {
            w->setBorderEdges(Qt::TopEdge | Qt::LeftEdge);
            t->compare();
        }

        SECTION("top-right-border") {
            w->setBorderEdges(Qt::TopEdge | Qt::RightEdge);
            t->compare();
        }

        SECTION("bottom-left-border") {
            w->setBorderEdges(Qt::BottomEdge | Qt::LeftEdge);
            t->compare();
        }

        SECTION("bottom-right-border") {
            w->setBorderEdges(Qt::BottomEdge | Qt::RightEdge);
            t->compare();
        }
    };

    auto fullcharsTests = [&] {
        SECTION("withwidetitle") {
            w->setWindowTitle("オレンジ");
            w->setOptions({});
            t->compare();
        }
        SECTION("withwidetitle-7") {
            w->setGeometry({1, 1, 7, 3});
            w->setWindowTitle("オレンジ");
            w->setOptions({});
            t->compare();
        }
    };

    SECTION("full-charset") {
        t = std::make_unique<Testhelper>("window", "window-visual", 25, 5);

        TestBackgroundWidget background(t->root);
        background.setGeometry({0, 0, 25, 5});

        w = new Tui::ZWindow(&background);
        w->setGeometry({1, 1, 23, 3});

        SECTION("unfocused") {
            tests();
            fullcharsTests();
        }

        SECTION("focused") {
            w->setFocus();
            tests();
            fullcharsTests();
        }
    }

    SECTION("reduced-charset") {
        t = std::make_unique<Testhelper>("window", "window-visual", 25, 5, Testhelper::ReducedCharset);

        TestBackgroundWidget background(t->root);
        background.setGeometry({0, 0, 25, 5});

        w = new Tui::ZWindow(&background);
        w->setGeometry({1, 1, 23, 3});

        SECTION("unfocused") {
            tests();
        }

        SECTION("focused") {
            w->setFocus();
            tests();
        }
    }
}

TEST_CASE("window-layout", "") {
    Testhelper t("window", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);

    auto contentsMargins = GENERATE(QMargins(1, 0, 0, 0),
                                    QMargins(0, 1, 0, 0),
                                    QMargins(0, 0, 1, 0),
                                    QMargins(0, 0, 0, 1));

    CAPTURE(contentsMargins.left());
    CAPTURE(contentsMargins.top());
    CAPTURE(contentsMargins.right());
    CAPTURE(contentsMargins.bottom());

    struct B {
        std::string name;
        Qt::Edges borderEdges;
        int left;
        int top;
        int horizontal;
        int vertical;
    };

    auto border = GENERATE(B{"all",  Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge | Qt::LeftEdge, 1, 1, 2, 2},
                           B{"none",         {}, 0, 0, 0, 0},
                           B{"top",          Qt::TopEdge, 0, 1, 0, 1},
                           B{"bottom",       Qt::BottomEdge, 0, 0, 0, 1},
                           B{"left",         Qt::LeftEdge, 1, 0, 1, 0},
                           B{"right",        Qt::RightEdge, 0, 0, 1, 0},
                           B{"top+left",     Qt::TopEdge | Qt::LeftEdge, 1, 1, 1, 1},
                           B{"top+right",    Qt::TopEdge | Qt::RightEdge, 0, 1, 1, 1},
                           B{"bottom+left",  Qt::BottomEdge | Qt::LeftEdge, 1, 0, 1, 1},
                           B{"bottom+right", Qt::BottomEdge | Qt::RightEdge, 0, 0, 1, 1}
                  );

    bool focus = GENERATE(true, false);

    CAPTURE(focus);

    auto layout = new StubLayout();
    const int layoutWidth = 12;
    const int layoutHeight = 3;
    layout->stubSizeHint = {layoutWidth, layoutHeight};

    w->setLayout(layout);

    w->setContentsMargins(contentsMargins);
    w->setBorderEdges(border.borderEdges);

    int cmLeft = contentsMargins.left();
    int cmTop = contentsMargins.top();
    int cmHorizontal = contentsMargins.left() + contentsMargins.right();
    int cmVertical = contentsMargins.top() + contentsMargins.bottom();

    CHECK(w->layoutArea().x() - cmLeft == border.left);
    CHECK(w->layoutArea().y() - cmTop == border.top);
    CHECK(w->geometry().width() - w->layoutArea().width() - cmHorizontal == border.horizontal);
    CHECK(w->geometry().height() - w->layoutArea().height() - cmVertical == border.vertical);

    CHECK(w->sizeHint().width() == layoutWidth + cmHorizontal + border.horizontal);
    CHECK(w->sizeHint().height() == layoutHeight + cmVertical + border.vertical);
}

TEST_CASE("window-tab", "") {
    Testhelper t("window", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);

    SECTION("no children") {
        w->setFocus();
        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
    }

    SECTION("one child") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Qt::StrongFocus);
        child1->setFocus();

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child1);
    }

    SECTION("three children") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Qt::StrongFocus);
        child1->setFocus();
        auto child2 = new StubWidget(w);
        child2->setFocusPolicy(Qt::StrongFocus);
        auto child3 = new StubWidget(w);
        child3->setFocusPolicy(Qt::StrongFocus);

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child1);
    }

    SECTION("three children with order set") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Qt::StrongFocus);
        child1->setFocus();
        child1->setFocusOrder(12);
        auto child2 = new StubWidget(w);
        child2->setFocusPolicy(Qt::StrongFocus);
        child2->setFocusOrder(6);
        auto child3 = new StubWidget(w);
        child3->setFocusPolicy(Qt::StrongFocus);
        child3->setFocusOrder(32);

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Qt::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Qt::Key_Tab, Qt::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child1);
    }
}

TEST_CASE("window-misc", "") {
    Testhelper t("window", "unused", 15, 5);
    new Tui::ZWindow(t.root);

    SECTION("0x0-allocation") {
        t.render();
        // shall not crash
    }
}
