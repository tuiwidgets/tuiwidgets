// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZCommandNotifier.h>
#include <Tui/ZCommandManager.h>

#include "catchwrapper.h"

#include "Tui/ZTest.h"

#include "Testhelper.h"

#include "eventrecorder.h"

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

TEST_CASE("commandnotifier-base", "") {
    Tui::ZWidget w;

    auto checkDefaults = [] (Tui::ZCommandNotifier *notifier) {
        CHECK(notifier->isEnabled());
        CHECK(notifier->command() == TUISYM_LITERAL("dummy"));
    };

    SECTION("constructor without context, without parent") {
        std::unique_ptr<Tui::ZCommandNotifier> notifier = std::make_unique<Tui::ZCommandNotifier>("dummy");
        CHECK(notifier->context() == Tui::ApplicationShortcut);
        checkDefaults(notifier.get());
    }

    SECTION("constructor without context, null parent") {
        std::unique_ptr<Tui::ZCommandNotifier> notifier = std::make_unique<Tui::ZCommandNotifier>("dummy", nullptr);
        CHECK(notifier->context() == Tui::ApplicationShortcut);
        checkDefaults(notifier.get());
    }

    SECTION("constructor without context") {
        std::unique_ptr<Tui::ZCommandNotifier> notifier = std::make_unique<Tui::ZCommandNotifier>("dummy", &w);
        CHECK(notifier->context() == Tui::ApplicationShortcut);
        checkDefaults(notifier.get());
    }

    SECTION("constructor with context, nullparent") {
        std::unique_ptr<Tui::ZCommandNotifier> notifier = std::make_unique<Tui::ZCommandNotifier>("dummy", nullptr,
                                                                                                  Tui::WidgetShortcut);
        CHECK(notifier->context() == Tui::WidgetShortcut);
        checkDefaults(notifier.get());
    }

    SECTION("constructor with context") {
        std::unique_ptr<Tui::ZCommandNotifier> notifier = std::make_unique<Tui::ZCommandNotifier>("dummy", &w,
                                                                                                  Tui::WidgetShortcut);
        CHECK(notifier->context() == Tui::WidgetShortcut);
        checkDefaults(notifier.get());
    }

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w2 = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    Tui::ZCommandNotifier notifier("dummy", w2.get(), Tui::WidgetShortcut);

    SECTION("enable") {
        CHECK(notifier.isEnabled() == true);
        notifier.setEnabled(false);
        CHECK(notifier.isEnabled() == false);
        notifier.setEnabled(true);
        CHECK(notifier.isEnabled() == true);
    }

    SECTION("isContextSatisfied") {
        CHECK(!notifier.isContextSatisfied());
    }

}

TEST_CASE("command generated", "") {
    Testhelper t("unsued", "unused", 16, 5);

    Tui::ZWidget *focus = nullptr;
    Tui::ZWidget *widget = nullptr;
    Tui::ShortcutContext context = Tui::WidgetShortcut;

    EventRecorder recorder;

    std::string attachment = GENERATE("direct", "late", "crossterminal");
    CAPTURE(attachment);

    Tui::ZWidget pseudoRoot;
    pseudoRoot.ensureCommandManager();

    Tui::ZWidget *root = attachment == "direct" ? t.root : new Tui::ZWidget(&pseudoRoot);

    auto terminal2 = std::make_unique<Tui::ZTerminal>(Tui::ZTerminal::OffScreen{16, 5});
    Tui::ZRoot root2;
    terminal2->setMainWidget(&root2);

    Tui::ZTerminal *terminal = attachment != "crossterminal" ? t.terminal.get() : terminal2.get();

    auto attachAndFocus = [&] {
        if (attachment == "late") {
            root->setParent(t.root);
        }
        if (attachment == "crossterminal") {
            root->setParent(&root2);
        }

        if (focus) {
            focus->setFocus();
        }
    };

    terminal->mainWidget()->ensureCommandManager();

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

        SECTION("active via cmdmanager") {
            bool withRender = GENERATE(false, true);
            CAPTURE(withRender);

            auto *commandNotifier = new Tui::ZCommandNotifier("somecmd", widget, context);
            auto activatedEvent = recorder.watchSignal(commandNotifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            attachAndFocus();

            if (withRender) {
                terminal->forceRepaint();
            }

            terminal->mainWidget()->commandManager()->activateCommand(TUISYM_LITERAL("somecmd"));

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(commandNotifier->isContextSatisfied());
            CHECK(terminal->mainWidget()->commandManager()->isCommandEnabled(TUISYM_LITERAL("somecmd")));

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

        SECTION("active via cmdmanager") {
            bool withRender = GENERATE(false, true);
            CAPTURE(withRender);

            auto *commandNotifier = new Tui::ZCommandNotifier("somecmd", widget, context);
            auto activatedEvent = recorder.watchSignal(commandNotifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            attachAndFocus();

            if (withRender) {
                terminal->forceRepaint();
            }

            terminal->mainWidget()->commandManager()->activateCommand(TUISYM_LITERAL("somecmd"));
            CHECK(recorder.noMoreEvents());

            CHECK(!commandNotifier->isContextSatisfied());
            CHECK(terminal->mainWidget()->commandManager()->isCommandEnabled(TUISYM_LITERAL("somecmd")) == false);
            CHECK(recorder.noMoreEvents());
        }

    }

    SECTION("context selected") {
        bool withRender = GENERATE(false, true);
        CAPTURE(withRender);

        SECTION("Widget") {
            auto *w = new Tui::ZWidget(root);
            auto *w2 = new Tui::ZWidget(root);

            context = Tui::WidgetShortcut;

            focus = w;

            auto *commandNotifier = new Tui::ZCommandNotifier("somecmd", w, context);
            commandNotifier->setObjectName("w");
            auto activatedEvent = recorder.watchSignal(commandNotifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            auto *commandNotifier2 = new Tui::ZCommandNotifier("somecmd", w2, context);
            commandNotifier2->setObjectName("w2");
            auto activatedEvent2 = recorder.watchSignal(commandNotifier2, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            attachAndFocus();

            if (withRender) {
                terminal->forceRepaint();
            }

            terminal->mainWidget()->commandManager()->activateCommand(TUISYM_LITERAL("somecmd"));

            CHECK(recorder.consumeFirst(activatedEvent));
            CHECK(recorder.noMoreEvents());

            CHECK(commandNotifier->isContextSatisfied());
            CHECK(!commandNotifier2->isContextSatisfied());
            CHECK(terminal->mainWidget()->commandManager()->isCommandEnabled(TUISYM_LITERAL("somecmd")));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("WindowShortcut") {
            auto *win = new StubWindowWidget(root);
            auto *win2 = new StubWindowWidget(root);

            context = Tui::WindowShortcut;

            focus = win2;

            auto *commandNotifier = new Tui::ZCommandNotifier("somecmd", win, context);
            commandNotifier->setObjectName("win");
            auto activatedEvent = recorder.watchSignal(commandNotifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            auto *commandNotifier2 = new Tui::ZCommandNotifier("somecmd", win2, context);
            commandNotifier2->setObjectName("win2");
            auto activatedEvent2 = recorder.watchSignal(commandNotifier2, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            attachAndFocus();

            if (withRender) {
                terminal->forceRepaint();
            }

            terminal->mainWidget()->commandManager()->activateCommand(TUISYM_LITERAL("somecmd"));

            CHECK(recorder.consumeFirst(activatedEvent2));
            CHECK(recorder.noMoreEvents());

            CHECK(!commandNotifier->isContextSatisfied());
            CHECK(commandNotifier2->isContextSatisfied());
            CHECK(terminal->mainWidget()->commandManager()->isCommandEnabled(TUISYM_LITERAL("somecmd")));

            CHECK(recorder.noMoreEvents());
        }

        SECTION("WidgetWithChildrenShortcut") {
            auto *w = new Tui::ZWidget(root);
            auto *w2 = new Tui::ZWidget(root);
            auto *w3 = new Tui::ZWidget(root);

            context = Tui::WidgetWithChildrenShortcut;

            focus = w2;

            auto *commandNotifier = new Tui::ZCommandNotifier("somecmd", w, context);
            commandNotifier->setObjectName("w");
            auto activatedEvent = recorder.watchSignal(commandNotifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            auto *commandNotifier2 = new Tui::ZCommandNotifier("somecmd", w2, context);
            commandNotifier2->setObjectName("w2");
            auto activatedEvent2 = recorder.watchSignal(commandNotifier2, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            auto *commandNotifier3 = new Tui::ZCommandNotifier("somecmd", w3, context);
            commandNotifier3->setObjectName("w3");
            auto activatedEvent3 = recorder.watchSignal(commandNotifier3, RECORDER_SIGNAL(&Tui::ZCommandNotifier::activated));

            attachAndFocus();

            if (withRender) {
                terminal->forceRepaint();
            }

            terminal->mainWidget()->commandManager()->activateCommand(TUISYM_LITERAL("somecmd"));

            CHECK(recorder.consumeFirst(activatedEvent2));
            CHECK(recorder.noMoreEvents());

            CHECK(!commandNotifier->isContextSatisfied());
            CHECK(commandNotifier2->isContextSatisfied());
            CHECK(!commandNotifier3->isContextSatisfied());
            CHECK(terminal->mainWidget()->commandManager()->isCommandEnabled(TUISYM_LITERAL("somecmd")));

            CHECK(recorder.noMoreEvents());
        }

    }

}

TEST_CASE("command signals") {
    Testhelper t("unsued", "unused", 16, 5);

    t.root->ensureCommandManager();

    Tui::ZWidget w1{t.root};
    Tui::ZWidget w2{t.root};

    const Tui::ZSymbol dummy = TUISYM_LITERAL("dummy");

    Tui::ZCommandNotifier *notifier = new Tui::ZCommandNotifier(dummy, &w1, Tui::WidgetShortcut);

    w1.setFocus();

    EventRecorder recorder;
    auto enabledSignal = recorder.watchSignal(notifier, RECORDER_SIGNAL(&Tui::ZCommandNotifier::enabledChanged));
    auto mgrEnabledSignal = recorder.watchSignal(t.root->commandManager(), RECORDER_SIGNAL(&Tui::ZCommandManager::commandStateChanged));

    notifier->setEnabled(false);

    CHECK(recorder.consumeFirst(mgrEnabledSignal, dummy));
    CHECK(recorder.consumeFirst(enabledSignal, false));
    CHECK(recorder.noMoreEvents());

    notifier->setEnabled(false);
    CHECK(recorder.noMoreEvents());


    notifier->setEnabled(true);

    CHECK(recorder.consumeFirst(mgrEnabledSignal, dummy));
    CHECK(recorder.consumeFirst(enabledSignal, true));
    CHECK(recorder.noMoreEvents());

    notifier->setEnabled(true);
    CHECK(recorder.noMoreEvents());

    w2.setFocus();

    CHECK(recorder.consumeFirst(mgrEnabledSignal, dummy));
    CHECK(recorder.consumeFirst(enabledSignal, false));
    CHECK(recorder.noMoreEvents());
}

