// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZShortcut.h>

#include "catchwrapper.h"

#include "Tui/ZTest.h"

#include "Testhelper.h"

#include "eventrecorder.h"

static std::unique_ptr<Tui::ZKeyEvent> createKeyEvent(Tui::Key key, Tui::KeyboardModifiers modifiers) {
    return std::make_unique<Tui::ZKeyEvent>(key, modifiers, QString());
}

static std::unique_ptr<Tui::ZKeyEvent> createCharEvent(const QString &text, Tui::KeyboardModifiers modifiers) {
    return std::make_unique<Tui::ZKeyEvent>(Tui::Key_unknown, modifiers, text);
}

namespace {

class StubWindowWidget : public Tui::ZWidget {
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

TEST_CASE("shortcut-base", "") {
    Tui::ZWidget w;
    Tui::ZShortcut shortcut(Tui::ZKeySequence::forMnemonic("x"), &w, Tui::WidgetShortcut);

    SECTION("enable") {
        CHECK(shortcut.isEnabled() == true);
        shortcut.setEnabled(false);
        CHECK(shortcut.isEnabled() == false);
        shortcut.setEnabled(true);
        CHECK(shortcut.isEnabled() == true);
    }

    SECTION("enable-delegate") {
        bool externalState = false;
        shortcut.setEnabledDelegate([&externalState] () {
            return externalState;
        });

        CHECK(shortcut.isEnabled() == false);

        externalState = true;
        CHECK(shortcut.isEnabled() == true);

        shortcut.setEnabled(false);
        externalState = false;

        CHECK(shortcut.isEnabled() == false);

        externalState = true;
        CHECK(shortcut.isEnabled() == false);
    }

    SECTION("matches") {
        CHECK(shortcut.matches(&w, createCharEvent("x", Tui::AltModifier).get()));
        CHECK(!shortcut.matches(nullptr, createCharEvent("x", Tui::AltModifier).get()));
        CHECK(!shortcut.matches(&w, createCharEvent("b", Tui::AltModifier).get()));
        CHECK(!shortcut.matches(nullptr, createCharEvent("b", Tui::AltModifier).get()));
    }
}

TEST_CASE("shortcut", "") {
    Testhelper t("unsued", "unused", 16, 5);

    SECTION("Remove from terminal") {
        Tui::ZWidget *w = new Tui::ZWidget(t.root);
        new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w, Tui::WidgetShortcut);
        w->setParent(nullptr);
        delete w;
    }
}

TEST_CASE("shortcut-enable", "") {
    Testhelper t("unsued", "unused", 16, 5);

    enum DelegateState { DelegateNotSet, DelegateEnables, DelegateDisables };

    struct TestCase { int line; bool effective; bool shortcutEnabled; bool widgetEnabled; DelegateState delegate; };

    auto testCase = GENERATE(
                //                 eff    short  widget delegate
                TestCase{__LINE__, false, false, false, DelegateNotSet},
                TestCase{__LINE__, false, false, true,  DelegateNotSet},
                TestCase{__LINE__, false, true,  false, DelegateNotSet},
                TestCase{__LINE__, true,  true,  true,  DelegateNotSet},

                TestCase{__LINE__, false, false, false, DelegateEnables},
                TestCase{__LINE__, false, false, true,  DelegateEnables},
                TestCase{__LINE__, true,  true,  false, DelegateEnables},
                TestCase{__LINE__, true,  true,  true,  DelegateEnables},

                TestCase{__LINE__, false, false, false, DelegateDisables},
                TestCase{__LINE__, false, false, true,  DelegateDisables},
                TestCase{__LINE__, false, true,  false, DelegateDisables},
                TestCase{__LINE__, false, true,  true,  DelegateDisables}
                );

    CAPTURE(testCase.line);

    Tui::ShortcutContext context = GENERATE(Tui::WidgetShortcut, Tui::ApplicationShortcut, Tui::WindowShortcut);
    CAPTURE(context);

    Tui::ZWidget *w;
    Tui::ZWidget *win = nullptr;
    if (context != Tui::WindowShortcut) {
        w = new Tui::ZWidget(t.root);
    } else {
        win = new StubWindowWidget(t.root);
        w = new Tui::ZWidget(win);
    }

    Tui::ZShortcut *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w, context);

    w->setEnabled(testCase.widgetEnabled);
    shortcut->setEnabled(testCase.shortcutEnabled);
    if (testCase.delegate == DelegateEnables) {
        shortcut->setEnabledDelegate([] { return true; });
    } else if (testCase.delegate == DelegateDisables) {
        shortcut->setEnabledDelegate([] { return false; });
    }

    SECTION("query") {
        CHECK(shortcut->isEnabled() == testCase.effective);
        CHECK(shortcut->matches(w, createCharEvent("x", Tui::AltModifier).get()) == testCase.effective);
    }

    SECTION("signal") {
        EventRecorder recorder;
        auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));
        if (context != Tui::WindowShortcut) {
            w->setFocus();
        } else {
            win->setFocus();
        }
        Tui::ZTest::sendText(t.terminal.get(), "x", Tui::AltModifier);
        if (testCase.effective && (testCase.widgetEnabled || context != Tui::WidgetShortcut)) {
            CHECK(recorder.consumeFirst(activatedEvent));
        }
        CHECK(recorder.noMoreEvents());
    }
}

TEST_CASE("shortcut generated", "") {
    Testhelper t("unsued", "unused", 16, 5);

    Tui::ZWidget *focus = nullptr;
    Tui::ZWidget *widget = nullptr;
    Tui::ShortcutContext context = Tui::WidgetShortcut;

    EventRecorder recorder;

    std::string attachment = GENERATE("direct", "late", "crossterminal");
    CAPTURE(attachment);

    Tui::ZWidget *root = attachment != "late" ? t.root : new Tui::ZWidget();

    auto terminal2 = std::make_unique<Tui::ZTerminal>(Tui::ZTerminal::OffScreen{16, 5});

    Tui::ZTerminal *terminal = attachment != "crossterminal" ? t.terminal.get() : terminal2.get();

    auto attachAndFocus = [&] {
        if (attachment == "late") {
            root->setParent(t.root);
        }
        if (attachment == "crossterminal") {
            terminal->setMainWidget(root);
        }

        if (focus) {
            focus->setFocus();
        }
    };


    auto seqStartedEvent = recorder.createEvent("PendingSequenceStarted");
    auto seqFinishedEvent = recorder.createEvent("PendingSequenceFinished");

    Tui::ZPendingKeySequenceCallbacks callbacks;
    callbacks.setPendingSequenceStarted([&recorder, seqStartedEvent] {
        recorder.recordEvent(seqStartedEvent);
    });
    callbacks.setPendingSequenceFinished([&recorder, seqFinishedEvent](bool matched) {
        recorder.recordEvent(seqFinishedEvent, matched);
    });
    terminal->registerPendingKeySequenceCallbacks(callbacks);

    SECTION("matching") {

        std::string testCase = GENERATE("application, no focus",
                                        "application, focus on root",
                                        "widget, focus on root",
                                        "widget, focus on child",
                                        "widget with children, focus on child",
                                        "widget with children, focus on root",
                                        "window, focus on window",
                                        "window, focus on child",
                                        "window, both on child");
        CAPTURE(testCase);

        if (testCase == "application, no focus") {
            focus = nullptr;
            widget = root;
            context = Tui::ApplicationShortcut;
        } else if (testCase == "application, focus on root") {
            focus = root;
            widget = root;
            context = Tui::ApplicationShortcut;
        } else if (testCase == "widget, focus on root") {
            focus = root;
            widget = root;
            context = Tui::WidgetShortcut;
        } else if (testCase == "widget, focus on child") {
            widget = new Tui::ZWidget(root);
            focus = widget;
            context = Tui::WidgetShortcut;
        } else if (testCase == "widget with children, focus on child") {
            auto *w = new Tui::ZWidget(root);
            widget = root;
            focus = w;
            context = Tui::WidgetWithChildrenShortcut;
        } else if (testCase == "widget with children, focus on root") {
            new Tui::ZWidget(root);
            widget = root;
            focus = root;
            context = Tui::WidgetWithChildrenShortcut;
        } else if (testCase == "window, focus on window") {
            auto *win = new StubWindowWidget(root);
            widget = win;
            focus = win;
            context = Tui::WindowShortcut;
        } else if (testCase == "window, focus on child") {
            auto *win = new StubWindowWidget(root);
            auto *w = new Tui::ZWidget(win);
            widget = win;
            focus = w;
            context = Tui::WindowShortcut;
        } else if (testCase == "window, both on child") {
            auto *win = new StubWindowWidget(root);
            auto *w = new Tui::ZWidget(win);
            auto *w2 = new Tui::ZWidget(win);
            widget = w2;
            focus = w;
            context = Tui::WindowShortcut;
        } else {
            FAIL("test case setup failed");
        }

        SECTION("simple mnemonic") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));


            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});
            Tui::ZTest::sendText(terminal, "x", Tui::ControlModifier);

            CHECK(recorder.noMoreEvents());
        }
        SECTION("simple key") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forKey(Tui::Key_F1), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);
            Tui::ZTest::sendText(terminal, "x", Tui::ControlModifier);

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forShortcut("x"), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::ControlModifier);

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));

            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});
            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut + shortcut") {
            auto seq = Tui::ZKeySequence::forShortcutSequence("a", Tui::ControlModifier,
                                                              "b", Tui::ControlModifier);
            auto *shortcut = new Tui::ZShortcut(seq, widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "a", Tui::ControlModifier);
            CHECK(recorder.consumeFirst(seqStartedEvent));
            Tui::ZTest::sendText(terminal, "b", Tui::ControlModifier);

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.consumeFirst(seqFinishedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));
            // sequence shortcuts never "match"
            CHECK(!shortcut->matches(focus, createCharEvent("a", Tui::ControlModifier).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("b", Tui::ControlModifier).get()));

            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});
            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut + key") {
            auto seq = Tui::ZKeySequence::forShortcutSequence("a", Tui::ControlModifier,
                                                              Tui::Key_Up, Tui::ControlModifier);
            auto *shortcut = new Tui::ZShortcut(seq, widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "a", Tui::ControlModifier);
            CHECK(recorder.consumeFirst(seqStartedEvent));
            Tui::ZTest::sendKey(terminal, Tui::Key_Up, Tui::ControlModifier);

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.consumeFirst(seqFinishedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));
            // sequence shortcuts never "match"
            CHECK(!shortcut->matches(focus, createCharEvent("a", Tui::ControlModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_Up, Tui::ControlModifier).get()));

            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});
            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.noMoreEvents());
        }
    }

    SECTION("context not matching") {
        std::string testCase = GENERATE("widget, focus on root",
                                        "widget, focus on child",
                                        "widget, focus on sibling",
                                        "widget with children, focus on root",
                                        "widget with children, focus on sibling",
                                        "window, focus on root",
                                        "window, focus on non window sibling",
                                        "window, focus on sibling window");
        CAPTURE(testCase);

        if (testCase == "widget, focus on root") {
            focus = root;
            widget = new Tui::ZWidget(root);
            context = Tui::WidgetShortcut;
        } else if (testCase == "widget, focus on child") {
            widget = new Tui::ZWidget(root);
            focus = new Tui::ZWidget(widget);
            context = Tui::WidgetShortcut;
        } else if (testCase == "widget, focus on sibling") {
            focus = new Tui::ZWidget(root);
            widget = new Tui::ZWidget(root);
            context = Tui::WidgetShortcut;
        } else if (testCase == "widget with children, focus on root") {
            widget = new Tui::ZWidget(root);
            focus = root;
            context = Tui::WidgetWithChildrenShortcut;
        } else if (testCase == "widget with children, focus on sibling") {
            widget = new Tui::ZWidget(root);
            focus = new Tui::ZWidget(root);
            context = Tui::WidgetWithChildrenShortcut;
        } else if (testCase == "window, focus on root") {
            auto *win = new StubWindowWidget(root);
            widget = win;
            focus = root;
            context = Tui::WindowShortcut;
        } else if (testCase == "window, focus on non window sibling") {
            auto *win = new StubWindowWidget(root);
            auto *win2 = new Tui::ZWidget(root);
            widget = win;
            focus = win2;
            context = Tui::WindowShortcut;
        } else if (testCase == "window, focus on sibling window") {
            auto *win = new StubWindowWidget(root);
            auto *win2 = new StubWindowWidget(root);
            widget = win;
            focus = win2;
            context = Tui::WindowShortcut;
        } else {
            FAIL("test case setup failed");
        }

        SECTION("simple mnemonic") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple key") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forKey(Tui::Key_F1), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendKey(terminal, Tui::Key_F1, {});

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut") {
            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forShortcut("x"), widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::ControlModifier);

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut + shortcut") {
            auto seq = Tui::ZKeySequence::forShortcutSequence("a", Tui::ControlModifier,
                                                              "b", Tui::ControlModifier);
            auto *shortcut = new Tui::ZShortcut(seq, widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "a", Tui::ControlModifier);
            CHECK(recorder.consumeFirst(seqStartedEvent));
            Tui::ZTest::sendText(terminal, "b", Tui::ControlModifier);

            CHECK(recorder.consumeFirst(seqFinishedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));
            // sequence shortcuts never "match"
            CHECK(!shortcut->matches(focus, createCharEvent("a", Tui::ControlModifier).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("b", Tui::ControlModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("simple shortcut + key") {
            auto seq = Tui::ZKeySequence::forShortcutSequence("a", Tui::ControlModifier,
                                                              Tui::Key_Up, Tui::ControlModifier);
            auto *shortcut = new Tui::ZShortcut(seq, widget, context);
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "a", Tui::ControlModifier);
            CHECK(recorder.consumeFirst(seqStartedEvent));
            Tui::ZTest::sendKey(terminal, Tui::Key_Up, Tui::ControlModifier);

            CHECK(recorder.consumeFirst(seqFinishedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_F1, {}).get()));
            CHECK(!shortcut->matches(focus, createCharEvent("x", Tui::ControlModifier).get()));
            // sequence shortcuts never "match"
            CHECK(!shortcut->matches(focus, createCharEvent("a", Tui::ControlModifier).get()));
            CHECK(!shortcut->matches(focus, createKeyEvent(Tui::Key_Up, Tui::ControlModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

    }

    SECTION("context selected") {
        SECTION("Widget") {
            auto *w = new Tui::ZWidget(root);
            auto *w2 = new Tui::ZWidget(root);

            context = Tui::WidgetShortcut;

            focus = w;

            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w, context);
            shortcut->setObjectName("w");
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            auto *shortcut2 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w2, context);
            shortcut2->setObjectName("w2");
            auto activatedEvent2 = recorder.watchSignal(shortcut2, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());


            CHECK(shortcut->matches(w, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut2->matches(w, createCharEvent("x", Tui::AltModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("WindowShortcut") {
            auto *win = new StubWindowWidget(root);
            auto *win2 = new StubWindowWidget(root);

            context = Tui::WindowShortcut;

            focus = win2;

            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), win, context);
            shortcut->setObjectName("win");
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            auto *shortcut2 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), win2, context);
            shortcut2->setObjectName("win2");
            auto activatedEvent2 = recorder.watchSignal(shortcut2, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.consumeFirst(activatedEvent2));
            CHECK(recorder.noMoreEvents());


            CHECK(!shortcut->matches(win2, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(shortcut2->matches(win2, createCharEvent("x", Tui::AltModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("WidgetWithChildrenShortcut") {
            auto *w = new Tui::ZWidget(root);
            auto *w2 = new Tui::ZWidget(root);
            auto *w3 = new Tui::ZWidget(root);

            context = Tui::WidgetWithChildrenShortcut;

            focus = w2;

            auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w, context);
            shortcut->setObjectName("w");
            auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            auto *shortcut2 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w2, context);
            shortcut2->setObjectName("w2");
            auto activatedEvent2 = recorder.watchSignal(shortcut2, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            auto *shortcut3 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w3, context);
            shortcut2->setObjectName("w3");
            auto activatedEvent3 = recorder.watchSignal(shortcut3, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

            attachAndFocus();

            Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

            CHECK(recorder.consumeFirst(activatedEvent2));
            CHECK(recorder.noMoreEvents());


            CHECK(shortcut->matches(w, createCharEvent("x", Tui::AltModifier).get()));
            CHECK(!shortcut2->matches(w, createCharEvent("x", Tui::AltModifier).get()));

            CHECK(recorder.noMoreEvents());
        }

    }

    SECTION("ambiguous") {
        auto *w = new Tui::ZWidget(root);
        auto *w2 = new Tui::ZWidget(root);
        auto *w3 = new Tui::ZWidget(root);

        context = Tui::ApplicationShortcut;

        focus = w2;

        auto *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w, context);
        shortcut->setObjectName("w");
        auto activatedEvent = recorder.watchSignal(shortcut, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

        auto *shortcut2 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w2, context);
        shortcut2->setObjectName("w2");
        auto activatedEvent2 = recorder.watchSignal(shortcut2, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

        auto *shortcut3 = new Tui::ZShortcut(Tui::ZKeySequence::forMnemonic("x"), w3, context);
        shortcut3->setObjectName("w3");
        auto activatedEvent3 = recorder.watchSignal(shortcut3, RECORDER_SIGNAL(&Tui::ZShortcut::activated));

        attachAndFocus();

        Tui::ZTest::sendText(terminal, "x", Tui::AltModifier);

        CHECK(recorder.noMoreEvents());
    }

}
