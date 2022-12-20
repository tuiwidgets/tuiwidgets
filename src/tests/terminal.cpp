// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTerminal.h>

#include <QCoreApplication>
#include <QTimer>
#include <QSet>
#include <QDeadlineTimer>

#include <Tui/ZImage.h>
#include <Tui/ZPainter.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTest.h>
#include <Tui/ZTextMetrics.h>

#include "catchwrapper.h"
#include "Testhelper.h"

#include "eventrecorder.h"

// This file only tests without an actual external terminal

// Lots of testing is done in other files
// * registerPendingKeySequenceCallbacks in shortcut.cpp
// * shortcut handling in shortcut.cpp
// * grabCursorPosition(), grabCursorColor() with regard to widget based painting in widget.cpp
// * focus / focusChanged in widget.cpp
// * viewport in moveviewport.cpp
// * layout generation and cycle handling in layoutinvalidation.cpp

TEST_CASE("terminal-base", "") {

    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};

    SECTION("defauls") {
        CHECK(terminal.mainWidget() == nullptr);
        CHECK(terminal.focusWidget() == nullptr);
        CHECK(terminal.width() == 20);
        CHECK(terminal.height() == 10);
        CHECK(terminal.title() == QString());
        CHECK(terminal.iconTitle() == QString());
        CHECK(terminal.autoDetectTimeoutMessage() == "Terminal auto detection is taking unusually long, press space to abort.");
    }

    SECTION("title") {
        terminal.setTitle("Some Test");
        CHECK(terminal.title() == "Some Test");
        terminal.updateOutput();

        // Trigger "no change" detection code path
        terminal.setTitle("Some Test");
        CHECK(terminal.title() == "Some Test");
    }

    SECTION("iconTitle") {
        terminal.setIconTitle("Some ICON Test");
        CHECK(terminal.iconTitle() == "Some ICON Test");
        terminal.updateOutput();

        // Trigger "no change" detection code path
        terminal.setIconTitle("Some ICON Test");
        CHECK(terminal.iconTitle() == "Some ICON Test");
    }

    SECTION("autoDetectTimeoutMessage") {
        terminal.setAutoDetectTimeoutMessage("Dummy Message");
        CHECK(terminal.autoDetectTimeoutMessage() == "Dummy Message");
    }

    SECTION("resize") {
        terminal.resize(30, 12);
        CHECK(terminal.width() == 30);
        CHECK(terminal.height() == 12);
    }

    SECTION("state offscreen") {
        CHECK(!terminal.isPaused());
        CHECK(terminal.hasCapability(TUISYM_LITERAL("extendedCharset")));
        CHECK(!terminal.hasCapability(TUISYM_LITERAL("somethingDummy")));
    }

    SECTION("hasCapability") {
        Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10).withCapability(TUISYM_LITERAL("somethingDummy"))};
        CHECK(terminal.hasCapability(TUISYM_LITERAL("extendedCharset")));
        CHECK(!terminal.hasCapability(TUISYM_LITERAL("somethingDummy")));
    }

}

namespace {

class PaintWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

    void paintEvent(Tui::ZPaintEvent *event) override {
        Tui::ZPainter painter = *event->painter();
        painter.clear(fg, Tui::Colors::brightWhite);
    }

    Tui::ZColor fg = Tui::Colors::blue;
};

class MinimumSizeHintWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

    QSize minimumSizeHint() const override {
        return {40, 20};
    }
};

class KeyEventAcceptingWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

    void keyEvent(Tui::ZKeyEvent *event) override {
        (void)event;
        // neither calling ignore nor forwarding to base class implementation should accept event.
    }
};

class PasteEventAcceptingWidget : public Tui::ZWidget {
public:
    using Tui::ZWidget::ZWidget;

    void pasteEvent(Tui::ZPasteEvent *event) override {
        (void)event;
        // neither calling ignore nor forwarding to base class implementation should accept event.
    }
};

bool waitForRenderingCycle(Tui::ZTerminal *terminal, int timeout) {
    QDeadlineTimer timer{timeout};

    bool triggered = false;
    QMetaObject::Connection connection = QObject::connect(terminal, &Tui::ZTerminal::afterRendering, terminal,
                             [&triggered] {
        triggered = true;
    });

    while (!triggered && !timer.hasExpired()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    QObject::disconnect(connection);

    return triggered;
}

}

TEST_CASE("terminal-paint-cycle", "") {

    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    EventRecorder recorder;

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};

    auto beforeRenderingSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::beforeRendering));
    auto afterRenderingSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::afterRendering));

    PaintWidget widget;

    auto widgetPaint = recorder.watchEvent(&widget, "Paint Event", [&recorder](std::shared_ptr<EventRecorder::RecorderEvent> eventRef, const QEvent *event) {
            if (event->type() == Tui::ZEventType::paint()) {
                recorder.recordEvent(eventRef);
            }
        });

    terminal.setMainWidget(&widget); // triggers a deferred render cycle
    CHECK(terminal.mainWidget() == &widget);

    CHECK(recorder.noMoreEvents());

    REQUIRE(waitForRenderingCycle(&terminal, 1000));

    CHECK(recorder.consumeFirst(beforeRenderingSignal));
    CHECK(recorder.consumeFirst(widgetPaint));
    CHECK(recorder.consumeFirst(afterRenderingSignal));
    CHECK(recorder.noMoreEvents());

    {
        Tui::ZImage image = terminal.grabCurrentImage();
        CHECK(image.peekForground(0, 0) == Tui::Colors::blue);
        CHECK(image.peekForground(19, 0) == Tui::Colors::blue);
        CHECK(image.peekForground(19, 9) == Tui::Colors::blue);
        CHECK(image.peekForground(0, 9) == Tui::Colors::blue);
    }

    // now manually trigger a render cycle
    widget.fg = Tui::Colors::brown;

    terminal.update();

    CHECK(recorder.noMoreEvents());

    REQUIRE(waitForRenderingCycle(&terminal, 1000));

    CHECK(recorder.consumeFirst(beforeRenderingSignal));
    CHECK(recorder.consumeFirst(widgetPaint));
    CHECK(recorder.consumeFirst(afterRenderingSignal));
    CHECK(recorder.noMoreEvents());

    {
        Tui::ZImage image = terminal.grabCurrentImage();
        CHECK(image.peekForground(0, 0) == Tui::Colors::brown);
        CHECK(image.peekForground(19, 0) == Tui::Colors::brown);
        CHECK(image.peekForground(19, 9) == Tui::Colors::brown);
        CHECK(image.peekForground(0, 9) == Tui::Colors::brown);
    }

    // in contrast forceRepaint does a non deferred repaint
    widget.fg = Tui::Colors::magenta;

    terminal.forceRepaint();

    CHECK(recorder.consumeFirst(beforeRenderingSignal));
    CHECK(recorder.consumeFirst(widgetPaint));
    CHECK(recorder.consumeFirst(afterRenderingSignal));
    CHECK(recorder.noMoreEvents());

    {
        Tui::ZImage image = terminal.grabCurrentImage();
        CHECK(image.peekForground(0, 0) == Tui::Colors::magenta);
        CHECK(image.peekForground(19, 0) == Tui::Colors::magenta);
        CHECK(image.peekForground(19, 9) == Tui::Colors::magenta);
        CHECK(image.peekForground(0, 9) == Tui::Colors::magenta);
    }

    REQUIRE(!waitForRenderingCycle(&terminal, 1000));

    // multiple triggers only end up with one rendering cycle
    terminal.update();
    terminal.update();
    terminal.update();

    REQUIRE(waitForRenderingCycle(&terminal, 1000));

    CHECK(recorder.consumeFirst(beforeRenderingSignal));
    CHECK(recorder.consumeFirst(widgetPaint));
    CHECK(recorder.consumeFirst(afterRenderingSignal));
    CHECK(recorder.noMoreEvents());

    REQUIRE(!waitForRenderingCycle(&terminal, 1000));
    CHECK(recorder.noMoreEvents());
}

TEST_CASE("terminal-usage-without-widget", "") {
    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    EventRecorder recorder;

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};

    CHECK(terminal.textMetrics().sizeInColumns("A") == 1);
    CHECK(terminal.textMetrics().sizeInColumns("x") == 1);
    CHECK(terminal.textMetrics().sizeInColumns("😇") == 2);
    CHECK(terminal.textMetrics().sizeInColumns("あ") == 2);

    Tui::ZPainter painter = terminal.painter();

    painter.setForeground(0, 0, Tui::Colors::brightCyan);
    painter.setForeground(19, 9, Tui::Colors::brightCyan);

    CHECK(terminal.grabCursorPosition() == QPoint{0, 0});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorPosition({-1, -1});

    CHECK(terminal.grabCursorPosition() == QPoint{-1, -1});
    CHECK(terminal.grabCursorVisibility() == false);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorPosition({4, 7});

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorColor(34, 65, 87);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(34, 65, 87));

    terminal.setCursorColor(-1, 65, 87);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorColor(34, -1, 87);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorColor(34, 65, -1);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(-1, -1, -1));

    terminal.setCursorColor(300, 400, 5001);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(255, 255, 255));

    terminal.setCursorStyle(Tui::CursorStyle::Bar);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Bar);
    CHECK(terminal.grabCursorColor() == std::make_tuple(255, 255, 255));

    terminal.setCursorStyle(Tui::CursorStyle::Unset);

    CHECK(terminal.grabCursorPosition() == QPoint{4, 7});
    CHECK(terminal.grabCursorVisibility() == true);
    CHECK(terminal.grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(terminal.grabCursorColor() == std::make_tuple(255, 255, 255));


    {
        Tui::ZImage image = terminal.grabCurrentImage();
        CHECK(image.peekForground(0, 0) == Tui::Colors::brightCyan);
        CHECK(image.peekForground(19, 0) == Tui::ZColor::defaultColor());
        CHECK(image.peekForground(19, 9) == Tui::Colors::brightCyan);
        CHECK(image.peekForground(0, 9) == Tui::ZColor::defaultColor());
    }
}

TEST_CASE("terminal-misc", "") {
    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    EventRecorder recorder;

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};

    SECTION("pause") {
        CHECK(!terminal.isPaused());
        terminal.pauseOperation();
        CHECK(terminal.isPaused());
        terminal.unpauseOperation();
        CHECK(!terminal.isPaused());
    }

    SECTION("pause-without-unpause") {
        CHECK(!terminal.isPaused());
        terminal.pauseOperation();
        CHECK(terminal.isPaused());
    }

    SECTION("forced-focus-restoration-restore-even-if-disabled-root") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
        Tui::ZWidget root;
        terminal.setMainWidget(&root);
        root.setFocus();
        CHECK(root.focus());
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(false);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        QTimer::singleShot(2000, [&] {
            FAIL_CHECK("Test timed out");
            QCoreApplication::instance()->exit(0);
        });

        auto connection = QObject::connect(&terminal, &Tui::ZTerminal::focusChanged, [] {
            QCoreApplication::instance()->exit(0);
        });

        CHECK(recorder.noMoreEvents());
        QCoreApplication::instance()->exec();

        QObject::disconnect(connection);

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());
        CHECK(root.focus());
        CHECK(terminal.focusWidget() == &root);
    }

    SECTION("forced-focus-restoration-restore-to-root") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
        Tui::ZWidget root;
        terminal.setMainWidget(&root);
        root.setFocus();
        CHECK(root.focus());
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(false);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(true);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);

        QTimer::singleShot(2000, [&] {
            FAIL_CHECK("Test timed out");
            QCoreApplication::instance()->exit(0);
        });

        auto connection = QObject::connect(&terminal, &Tui::ZTerminal::focusChanged, [] {
            QCoreApplication::instance()->exit(0);
        });

        CHECK(recorder.noMoreEvents());
        QCoreApplication::instance()->exec();

        QObject::disconnect(connection);

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());
        CHECK(root.focus());
        CHECK(terminal.focusWidget() == &root);
    }

    SECTION("forced-focus-restoration-restore-to-last-focused") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
        Tui::ZWidget root;
        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        terminal.setMainWidget(&root);

        // prime focus history
        w1.setFocus();
        w2.setFocus();
        w3.setFocus();
        CHECK(w3.focus());
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(false);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(true);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);

        QTimer::singleShot(2000, [&] {
            FAIL_CHECK("Test timed out");
            QCoreApplication::instance()->exit(0);
        });

        auto connection = QObject::connect(&terminal, &Tui::ZTerminal::focusChanged, [] {
            QCoreApplication::instance()->exit(0);
        });

        CHECK(recorder.noMoreEvents());
        QCoreApplication::instance()->exec();

        QObject::disconnect(connection);

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());
        CHECK(w3.focus());
        CHECK(terminal.focusWidget() == &w3);
    }

    SECTION("forced-focus-restoration-restore-by-history-order") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
        Tui::ZWidget root;
        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        terminal.setMainWidget(&root);

        // prime focus history
        w1.setFocus();
        w2.setFocus();
        w3.setFocus();
        CHECK(w3.focus());
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(false);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        root.setEnabled(true);
        w3.setEnabled(false);
        CHECK(!root.focus());
        CHECK(terminal.focusWidget() == nullptr);

        QTimer::singleShot(2000, [&] {
            FAIL_CHECK("Test timed out");
            QCoreApplication::instance()->exit(0);
        });

        auto connection = QObject::connect(&terminal, &Tui::ZTerminal::focusChanged, [] {
            QCoreApplication::instance()->exit(0);
        });

        CHECK(recorder.noMoreEvents());
        QCoreApplication::instance()->exec();

        QObject::disconnect(connection);

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());
        CHECK(w2.focus());
        CHECK(terminal.focusWidget() == &w2);
    }

    SECTION("setMainWidget-triggers-terminal-change-event") {
        Tui::ZWidget root;
        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        auto rootTerminalChanged = recorder.watchTerminalChangeEvent(&root, "root terminal change");
        auto w1TerminalChanged = recorder.watchTerminalChangeEvent(&w1, "w1 terminal change");
        auto w2TerminalChanged = recorder.watchTerminalChangeEvent(&w2, "w2 terminal change");
        auto w3TerminalChanged = recorder.watchTerminalChangeEvent(&w3, "w3 terminal change");

        CHECK(recorder.noMoreEvents());

        terminal.setMainWidget(&root);

        CHECK(recorder.consumeFirst(rootTerminalChanged));
        CHECK(recorder.consumeFirst(w3TerminalChanged));
        CHECK(recorder.consumeFirst(w2TerminalChanged));
        CHECK(recorder.consumeFirst(w1TerminalChanged));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("setMainWidget-triggers-focus-of-unattached") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        Tui::ZWidget root;
        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        w2.setFocus();

        // Special case, unattached widget can't have real focus
        CHECK(!w2.focus());

        // But attachment recovers focus
        terminal.setMainWidget(&root);

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        CHECK(w2.focus());
    }

    SECTION("setMainWidget-change-triggers-reset") {
        auto focusSignal = recorder.watchSignal(&terminal, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        Tui::ZWidget root2;

        w2.setFocus();

        auto w2FocusOut = recorder.watchFocusOutEvent(&w2, "w2 focus out");

        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        CHECK(w2.focus());
        CHECK(terminal.focusWidget() == &w2);

        w2.grabKeyboard();
        CHECK(terminal.keyboardGrabber() == &w2);

        terminal.setMainWidget(&root2);

        CHECK(recorder.consumeFirst(w2FocusOut, Tui::OtherFocusReason));
        CHECK(recorder.consumeFirst(focusSignal));
        CHECK(recorder.noMoreEvents());

        CHECK(terminal.focusWidget() == nullptr);
        CHECK(terminal.keyboardGrabber() == nullptr);
    }

    SECTION("resize-with-main-widget") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZTest::withLayoutRequestTracking(&terminal, [&](QSet<Tui::ZWidget*> *layoutRequests) {
            CHECK(layoutRequests->isEmpty());
            terminal.resize(30, 12);
            CHECK(terminal.width() == 30);
            CHECK(terminal.height() == 12);
            CHECK(root.geometry() == QRect{0, 0, 30, 12});
            CHECK(layoutRequests->contains(&root));
        });

    }

    SECTION("resize-with-main-widget-with-minium-size") {
        MinimumSizeHintWidget root;
        terminal.setMainWidget(&root);

        root.setMinimumSize(40, 20);

        terminal.resize(30, 12);
        CHECK(terminal.width() == 30);
        CHECK(terminal.height() == 12);
        CHECK(root.geometry() == QRect{0, 0, 40, 20});
    }

    SECTION("resize-with-main-widget-with-minium-sizehint") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        root.setMinimumSize(40, 20);

        terminal.resize(30, 12);
        CHECK(terminal.width() == 30);
        CHECK(terminal.height() == 12);
        CHECK(root.geometry() == QRect{0, 0, 40, 20});
    }


    SECTION("maybeRequestLayout") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZTest::withLayoutRequestTracking(&terminal, [&](QSet<Tui::ZWidget*> *layoutRequests) {
            CHECK(layoutRequests->isEmpty());
            terminal.maybeRequestLayout(&root);
            CHECK(layoutRequests->contains(&root));
        });

    }

    SECTION("requestLayout") {
        Tui::ZWidget root;
        Tui::ZWidget w1{&root};
        Tui::ZWidget w2{&root};
        Tui::ZWidget w3{&root};

        terminal.setMainWidget(&root);
        auto requestTerminal = recorder.watchLayoutRequestEvent(&terminal, "layout request on terminal");

        Tui::ZTest::withLayoutRequestTracking(&terminal, [&](QSet<Tui::ZWidget*> *layoutRequests) {
            CHECK(layoutRequests->isEmpty());
            terminal.requestLayout(&root);
            CHECK(layoutRequests->contains(&root));
        });
        CHECK(recorder.noMoreEvents());

        QCoreApplication::instance()->processEvents();

        CHECK(recorder.consumeFirst(requestTerminal));
        CHECK(recorder.noMoreEvents());

        // All layout request only trigger one layout cycle
        terminal.requestLayout(&root);
        terminal.requestLayout(&root);
        terminal.requestLayout(&w1);
        terminal.requestLayout(&w2);
        terminal.requestLayout(&w3);

        CHECK(recorder.noMoreEvents());

        QCoreApplication::instance()->processEvents();

        CHECK(recorder.consumeFirst(requestTerminal));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("setMainWidget-with-non-root-widget-noop") {
        Tui::ZWidget root;
        Tui::ZWidget w1{&root};

        terminal.setMainWidget(&w1);
        CHECK(terminal.mainWidget() == nullptr);

        Tui::ZWidget root2;

        terminal.setMainWidget(&root2);
        CHECK(terminal.mainWidget() == &root2);

        terminal.setMainWidget(&w1);
        CHECK(terminal.mainWidget() == &root2);
    }

    SECTION("setMainWidget-clears-main-of-previous-terminal") {
        Tui::ZTerminal terminal2{Tui::ZTerminal::OffScreen(20, 10)};

        Tui::ZWidget root;
        terminal.setMainWidget(&root);
        CHECK(terminal.mainWidget() == &root);

        terminal2.setMainWidget(&root);
        CHECK(terminal.mainWidget() == nullptr);
        CHECK(terminal2.mainWidget() == &root);
    }
}

TEST_CASE("terminal-key-events", "") {
    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    EventRecorder recorder;

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};


    struct TestCase { int key; QString text; Tui::KeyboardModifiers modifiers; };
    auto testCase = GENERATE(
                            TestCase{Tui::Key_F1, "", Tui::ControlModifier},
                            TestCase{Tui::Key_F2, "", Tui::ShiftModifier},
                            TestCase{Tui::Key_F3, "", Tui::AltModifier},
                            TestCase{Tui::Key_unknown, "a", Tui::ControlModifier},
                            TestCase{Tui::Key_unknown, "A", Tui::ShiftModifier},
                            TestCase{Tui::Key_unknown, "a", Tui::AltModifier},
                            TestCase{Tui::Key_unknown, "a", Tui::NoModifier},
                            TestCase{Tui::Key_unknown, "😎", Tui::AltModifier}
                        );

    auto sendEvent = [&] {
        Tui::ZKeyEvent event(testCase.key, testCase.modifiers, testCase.text);
        terminal.dispatchKeyboardEvent(event);
    };

    SECTION("no widget accepts - focus on root") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        root.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(rootKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("no widget accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(parentKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(rootKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("child accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        KeyEventAcceptingWidget child{&widget};

        child.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("widget accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        KeyEventAcceptingWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        Tui::ZWidget grabbingWidget{&parent};

        auto grabKeyEvent = recorder.watchKeyEvent(&grabbingWidget, "key event on grabbingWidget");

        grabbingWidget.grabKeyboard();
        CHECK(terminal.keyboardGrabber() == &grabbingWidget);

        sendEvent();

        CHECK(recorder.consumeFirst(grabKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());

        sendEvent();

        CHECK(recorder.consumeFirst(grabKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());

        grabbingWidget.releaseKeyboard();

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(parentKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(rootKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab+handler - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        Tui::ZWidget grabbingWidget{&parent};

        auto grabKeyEvent = recorder.watchKeyEvent(&grabbingWidget, "key event on grabbingWidget");

        auto grabHandlerEvent = recorder.createEvent("grab handler");
        grabbingWidget.grabKeyboard([&] (QEvent *ev) {
            if (ev->type() == Tui::ZEventType::key()) {
                auto &event = dynamic_cast<const Tui::ZKeyEvent&>(*ev);
                recorder.recordEvent(grabHandlerEvent, event.key(), event.text(), event.modifiers());
            } else {
                FAIL_CHECK("unexpected event");
            }
        });
        CHECK(terminal.keyboardGrabber() == &grabbingWidget);

        sendEvent();

        CHECK(recorder.consumeFirst(grabHandlerEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());

        sendEvent();

        CHECK(recorder.consumeFirst(grabHandlerEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());

        grabbingWidget.releaseKeyboard();

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(parentKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(rootKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab-deleted") {
        bool query = GENERATE(false, true);
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        KeyEventAcceptingWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        {
            Tui::ZWidget other{&root};
            other.grabKeyboard([&] (QEvent*) {});
        }

        if (query) {
            CHECK(terminal.keyboardGrabber() == nullptr);
        }

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab-handler-deleted") {
        bool query = GENERATE(false, true);
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        KeyEventAcceptingWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        {
            Tui::ZWidget other{&root};
            other.grabKeyboard();
        }

        if (query) {
            CHECK(terminal.keyboardGrabber() == nullptr);
        }

        auto rootKeyEvent = recorder.watchKeyEvent(&root, "key event on root");
        auto parentKeyEvent = recorder.watchKeyEvent(&parent, "key event on parent");
        auto widgetKeyEvent = recorder.watchKeyEvent(&widget, "key event on widget");
        auto childKeyEvent = recorder.watchKeyEvent(&child, "key event on child");

        sendEvent();

        CHECK(recorder.consumeFirst(childKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.consumeFirst(widgetKeyEvent, testCase.key, testCase.text, testCase.modifiers));
        CHECK(recorder.noMoreEvents());
    }

}

TEST_CASE("terminal-paste-events", "") {
    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    QCoreApplication app(argc, argv);

    EventRecorder recorder;

    Tui::ZTerminal terminal{Tui::ZTerminal::OffScreen(20, 10)};

    SECTION("no widget accepts - focus on root") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        root.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(rootPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("no widget accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(childPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(widgetPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(parentPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(rootPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("child accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        PasteEventAcceptingWidget child{&widget};

        child.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(childPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("widget accepts - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        PasteEventAcceptingWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(childPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(widgetPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZWidget grabbingWidget{&parent};

        auto grabPasteEvent = recorder.watchPasteEvent(&grabbingWidget, "paste event on grabbingWidget");

        grabbingWidget.grabKeyboard();
        CHECK(terminal.keyboardGrabber() == &grabbingWidget);

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(grabPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(grabPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());

        grabbingWidget.releaseKeyboard();

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(childPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(widgetPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(parentPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(rootPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("grab+handler - focus on child") {
        Tui::ZWidget root;
        terminal.setMainWidget(&root);

        Tui::ZWidget parent{&root};
        Tui::ZWidget widget{&parent};
        Tui::ZWidget child{&widget};

        child.setFocus();

        auto rootPasteEvent = recorder.watchPasteEvent(&root, "paste event on root");
        auto parentPasteEvent = recorder.watchPasteEvent(&parent, "paste event on parent");
        auto widgetPasteEvent = recorder.watchPasteEvent(&widget, "paste event on widget");
        auto childPasteEvent = recorder.watchPasteEvent(&child, "paste event on child");

        Tui::ZWidget grabbingWidget{&parent};

        auto grabPasteEvent = recorder.watchPasteEvent(&grabbingWidget, "paste event on grabbingWidget");

        auto grabHandlerEvent = recorder.createEvent("grab handler");
        grabbingWidget.grabKeyboard([&] (QEvent *ev) {
            if (ev->type() == Tui::ZEventType::paste()) {
                auto &event = dynamic_cast<const Tui::ZPasteEvent&>(*ev);
                recorder.recordEvent(grabHandlerEvent, event.text());
            } else {
                FAIL_CHECK("unexpected event");
            }
        });
        CHECK(terminal.keyboardGrabber() == &grabbingWidget);

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(grabHandlerEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(grabHandlerEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());

        grabbingWidget.releaseKeyboard();

        Tui::ZTest::sendPaste(&terminal, "paste text");

        CHECK(recorder.consumeFirst(childPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(widgetPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(parentPasteEvent, QString("paste text")));
        CHECK(recorder.consumeFirst(rootPasteEvent, QString("paste text")));
        CHECK(recorder.noMoreEvents());
    }

}

TEST_CASE("termial-FileDescriptor", "") {
    Tui::ZTerminal::FileDescriptor fd{23};
    CHECK(fd.fd() == 23);

    Tui::ZTerminal::FileDescriptor fd2 = fd;
    CHECK(fd2.fd() == 23);
}

TEST_CASE("termial-OffScreen", "") {
    Tui::ZTerminal::OffScreen offscreen{23, 56};
    Tui::ZTerminal::OffScreen offscreen2 = offscreen;
}

TEST_CASE("termial-TerminalConnection", "") {
    Tui::ZTerminal::TerminalConnection connection;
    connection.setBackspaceIsX08(false);
    connection.setSize(23, 56);
    // Trigger "no change" detection code path
    connection.setSize(23, 56);
    DiagnosticMessageChecker msg;
    msg.expectMessage("ZTerminal::TerminalConnection::terminalInput: No terminal associated!");
    connection.terminalInput("blub", 4);
    msg.tillHere();
}
