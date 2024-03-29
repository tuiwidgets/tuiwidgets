// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWindow.h>
#include <Tui/ZDialog.h>

#include "../catchwrapper.h"

#include <QPointer>
#include <QRect>
#include <QVector>

#include <Tui/ZBasicWindowFacet.h>
#include <Tui/ZColor.h>
#include <Tui/ZCommandManager.h>
#include <Tui/ZCommandNotifier.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTest.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWindowFacet.h>

#include "../Testhelper.h"
#include "../vcheck_zwidget.h"

namespace {

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

    class CustomWindowFacet : public Tui::ZWindowFacet {
    };

    class CustomWindow : public Tui::ZWindow {
    public:
        using Tui::ZWindow::ZWindow;

        QObject *facet(const QMetaObject &metaObject) const override {
            if (metaObject.className() == Tui::ZWindowFacet::staticMetaObject.className()) {
                return &winFacet;
            } else {
                return Tui::ZWindow::facet(metaObject);
            }
        }

        mutable CustomWindowFacet winFacet;
    };

    class CustomWindowContainer : public Tui::ZWindowContainer {
    public:
        QVector<Tui::ZMenuItem> containerMenuItems() const override {
            return {{ QStringLiteral("SomeContainerStuff"), QString(), QStringLiteral("ContainerStuff"), {}}};
        }
    };

}


TEST_CASE("window-base", "") {

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    SECTION("no parent") {
        delete new Tui::ZWindow();
        delete new Tui::ZWindow("some title");
    }

    Tui::ZWidget parentWidget;

    Tui::ZWindow w(parent ? &parentWidget : nullptr);

    auto checkDefaultState = [] (Tui::ZWindow *w) {
        CHECK(w->options() == Tui::ZWindow::Options{});
        CHECK(w->borderEdges() == (Tui::TopEdge | Tui::RightEdge | Tui::BottomEdge | Tui::LeftEdge));
        CHECK(w->focusMode() == Tui::FocusContainerMode::Cycle);
        CHECK(w->paletteClass() == QStringList{"window"});
        CHECK(w->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(w->sizePolicyV() == Tui::SizePolicy::Expanding);
        auto windowFacet = w->facet(Tui::ZWindowFacet::staticMetaObject);
        CHECK(windowFacet != nullptr);
        CHECK(windowFacet->metaObject()->className() == Tui::ZBasicWindowFacet::staticMetaObject.className());
        CHECK(static_cast<Tui::ZWindowFacet*>(windowFacet)->isManuallyPlaced() == true);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(w, DefaultException::SizePolicyV
                                                   | DefaultException::SizePolicyH
                                                   | DefaultException::FocusMode
                                                   | DefaultException::PaletteClass));
    };


    SECTION("constructor") {
        CHECK(w.windowTitle() == "");
        checkDefaultState(&w);
    }

    SECTION("constructor-with-title") {
        Tui::ZWindow w2("Breakpoints");
        CHECK(w2.windowTitle() == "Breakpoints");
        checkDefaultState(&w);
    }

    SECTION("abi-vcheck") {
        Tui::ZWidget base;
        checkZWidgetOverrides(&base, &w);
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
        CHECK(w.borderEdges() == Tui::Edges{});
        w.setBorderEdges(Tui::Edge::TopEdge);
        CHECK(w.borderEdges() == Tui::Edge::TopEdge);
    }

    SECTION("setDefaultPlacement") {
        w.setDefaultPlacement(Tui::AlignCenter);
        auto windowFacet = static_cast<Tui::ZWindowFacet*>(w.facet(Tui::ZWindowFacet::staticMetaObject));
        CHECK(windowFacet->isManuallyPlaced() == false);
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

TEST_CASE("window-close") {
    class TestApp : public QCoreApplication {
    public:
        using QCoreApplication::QCoreApplication;

        bool notify(QObject *o, QEvent *e) override {
            if (fBefore) {
                fBefore(o, e);
            }
            bool ret = QCoreApplication::notify(o, e);
            if (fAfter) {
                fAfter(o, e);
            }
            return ret;
        }

        std::function<void(QObject*, QEvent*)> fBefore, fAfter;
    };

    class TestWindow : public Tui::ZWindow {
    public:
        using Tui::ZWindow::ZWindow;

        bool ignoreInSpecificEvent = false;
        bool ignoreInGenericEvent = false;
        QStringList expectedSkipChecks;

    protected:
        bool event(QEvent *event) override {
            if (ignoreInGenericEvent) {
                if (event->type() == Tui::ZEventType::close()) {
                    event->ignore();
                }
            }
            // always calling base class, should not change anything
            return ZWindow::event(event);
        }

        void closeEvent(Tui::ZCloseEvent *event) override {
            CHECK(event->skipChecks() == expectedSkipChecks);
            if (ignoreInSpecificEvent) {
                event->ignore();
            } else {
                Tui::ZWindow::closeEvent(event);
            }
        }
    };

    static char prgname[] = "test";
    static char *argv[] = {prgname, nullptr};
    int argc = 1;
    TestApp app(argc, argv);

    SECTION("without-skip") {
        bool useSkipCheck = GENERATE(false, true);

        SECTION("close-hides") {
            QPointer<TestWindow> w = new TestWindow();

            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);
            CHECK(w->isVisible());
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(!w->isVisible());

            delete w.data();
        }
        SECTION("close-deletes") {
            QPointer<TestWindow> w = new TestWindow();

            w->setOptions(w->options() | Tui::ZWindow::DeleteOnClose);
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            CHECK(w == nullptr);

            delete w.data();
        }

        SECTION("close-uses-event") {
            QPointer<TestWindow> w = new TestWindow();
            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

            enum State {
                Start, GotEvent
            } state = Start;

            app.fBefore = [&state, wCopy = w.data()] (QObject *o, QEvent *e) {
                if (o == wCopy) {
                    if (e->type() == Tui::ZEventType::close()) {
                        if (state == Start) {
                            state = GotEvent;
                        } else {
                            FAIL("duplicate event");
                        }
                    }
                }
            };
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(state == GotEvent);

            delete w.data();
        }

        SECTION("close-default-accepted") {
            QPointer<TestWindow> w = new TestWindow();
            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

            enum State {
                Start, GotEvent
            } state = Start;

            app.fAfter = [&state, wCopy = w.data()] (QObject *o, QEvent *e) {
                if (o == wCopy) {
                    if (e->type() == Tui::ZEventType::close()) {
                        if (state == Start) {
                            state = GotEvent;
                            CHECK(static_cast<Tui::ZCloseEvent*>(e)->isAccepted() == true);
                        } else {
                            FAIL("duplicate event");
                        }
                    }
                }
            };
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(state == GotEvent);

            delete w.data();
        }

        SECTION("close-ignore-in-notify-blocks-close") {
            // this checks that the event is properly dispatched. Should covery notify and event filters
            QPointer<TestWindow> w = new TestWindow();
            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

            enum State {
                Start, GotEvent
            } state = Start;

            app.fAfter = [&state, wCopy = w.data()] (QObject *o, QEvent *e) {
                if (o == wCopy) {
                    if (e->type() == Tui::ZEventType::close()) {
                        if (state == Start) {
                            state = GotEvent;
                            e->ignore();
                        } else {
                            FAIL("duplicate event");
                        }
                    }
                }
            };
            CHECK(w->isVisible());
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(w->isVisible());
            CHECK(state == GotEvent);

            delete w.data();
        }

        SECTION("close-ignore-event-specific-event-handler") {
            QPointer<TestWindow> w = new TestWindow();
            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

            CHECK(w->isVisible());
            w->ignoreInSpecificEvent = true;
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(w->isVisible());

            delete w.data();
        }

        SECTION("close-ignore-event-generic-event-handler") {
            QPointer<TestWindow> w = new TestWindow();
            w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

            CHECK(w->isVisible());
            w->ignoreInGenericEvent = true;
            if (!useSkipCheck) {
                w->close();
            } else {
                w->closeSkipCheck({});
            }
            CHECK(w->isVisible());

            delete w.data();
        }

    }

    SECTION("close-skipChecks") {
        QPointer<TestWindow> w = new TestWindow();
        w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);

        w->expectedSkipChecks = QStringList{ "one", "two" };
        w->closeSkipCheck({ "one", "two" });

        delete w.data();
    }

    SECTION("close-command") {
        Tui::ZTerminal terminal(Tui::ZTerminal::OffScreen{10, 10});
        Tui::ZRoot root;
        terminal.setMainWidget(&root);
        Tui::ZCommandManager *const cmdMgr = root.ensureCommandManager();
        QPointer<TestWindow> w = new TestWindow(&root);
        w->setFocus();
        w->setOptions(w->options() & ~Tui::ZWindow::DeleteOnClose);
        auto closeSym = TUISYM_LITERAL("ZWindowClose");
        CHECK(cmdMgr->isCommandEnabled(closeSym));
        cmdMgr->activateCommand(closeSym);
        CHECK(!w->isVisible());

        delete w.data();
    }

}

TEST_CASE("window-set-automatic-placement") {
    Testhelper t("window", "window-set-automatic-placement", 25, 10);
    Tui::ZCommandManager *const cmdMgr = t.root->ensureCommandManager();

    bool useCustomWindowFacet = GENERATE(false, true);

    Tui::ZWindow *w;
    if (useCustomWindowFacet) {
        w = new CustomWindow(t.root);
    } else {
        w = new Tui::ZWindow(t.root);
    }

    w->setFocus();
    w->setGeometry({0, 0, 21, 8});

    auto *windowFacet = qobject_cast<Tui::ZWindowFacet*>(w->facet(Tui::ZWindowFacet::staticMetaObject));
    windowFacet->setManuallyPlaced(true);

    auto ensureAndTriggerCommand = [cmdMgr](Tui::ZSymbol sym) {
        CHECK(cmdMgr->isCommandEnabled(sym));
        cmdMgr->activateCommand(sym);
    };

    CHECK(w->geometry() == QRect{0, 0, 21, 8});


    SECTION("method") {
        w->setAutomaticPlacement();
        CHECK(w->geometry() == QRect{2, 2, 21, 8});
    }

    SECTION("command") {
        ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowAutomaticPlacement"));
        CHECK(w->geometry() == QRect{2, 2, 21, 8});
    }

    SECTION("method-container") {
        // should be no-op when container is set.
        Tui::ZWindowContainer container;
        windowFacet->setContainer(&container);
        w->setAutomaticPlacement();
        CHECK(w->geometry() == QRect{0, 0, 21, 8});
    }
}

TEST_CASE("window-systemmenu", "") {
    class TestWindow : public Tui::ZWindow {
    public:
        using Tui::ZWindow::ZWindow;

        QVector<Tui::ZMenuItem> retrieveSystemMenu() {
            return ZWindow::systemMenu();
        }

        QVector<Tui::ZMenuItem> menuItems;

    protected:
        QVector<Tui::ZMenuItem> systemMenu() override {
            return menuItems;
        }
    };

    Testhelper t("window", "window-systemmenu", 25, 10);

    SECTION("menu-empty-options") {
        TestWindow w;
        w.setOptions({});
        CHECK(w.retrieveSystemMenu().size() == 0);
    }

    SECTION("menu-close-option") {
        TestWindow w;
        w.setOptions({ Tui::ZWindow::CloseOption });
        QVector<Tui::ZMenuItem> menu = w.retrieveSystemMenu();
        REQUIRE(menu.size() == 1);
        CHECK(menu[0].markup() == "<m>C</m>lose");
        CHECK(menu[0].fakeShortcut() == "");
        auto closeSym = TUISYM_LITERAL("ZWindowClose");
        CHECK(menu[0].command() == closeSym);
        CHECK(menu[0].subitems().isEmpty());
    }

    SECTION("menu-move-option") {
        TestWindow w;
        w.setOptions({ Tui::ZWindow::MoveOption });
        QVector<Tui::ZMenuItem> menu = w.retrieveSystemMenu();
        REQUIRE(menu.size() == 1);
        CHECK(menu[0].markup() == "<m>M</m>ove");
        CHECK(menu[0].fakeShortcut() == "");
        auto moveSym = TUISYM_LITERAL("ZWindowInteractiveMove");
        CHECK(menu[0].command() == moveSym);
        CHECK(menu[0].subitems().isEmpty());
    }

    SECTION("menu-resize-option") {
        TestWindow w;
        w.setOptions({ Tui::ZWindow::ResizeOption });
        QVector<Tui::ZMenuItem> menu = w.retrieveSystemMenu();
        REQUIRE(menu.size() == 1);
        CHECK(menu[0].markup() == "<m>R</m>esize");
        CHECK(menu[0].fakeShortcut() == "");
        auto resizeSym = TUISYM_LITERAL("ZWindowInteractiveResize");
        CHECK(menu[0].command() == resizeSym);
        CHECK(menu[0].subitems().isEmpty());
    }

    SECTION("menu-automatic-option") {
        TestWindow w;
        w.setOptions({ Tui::ZWindow::AutomaticOption });
        QVector<Tui::ZMenuItem> menu = w.retrieveSystemMenu();
        REQUIRE(menu.size() == 1);
        CHECK(menu[0].markup() == "<m>A</m>utomatic");
        CHECK(menu[0].fakeShortcut() == "");
        auto automaticSym = TUISYM_LITERAL("ZWindowAutomaticPlacement");
        CHECK(menu[0].command() == automaticSym);
        CHECK(menu[0].subitems().isEmpty());
    }

    SECTION("menu-all-options") {
        TestWindow w;
        w.setOptions({ Tui::ZWindow::MoveOption  | Tui::ZWindow::ResizeOption | Tui::ZWindow::CloseOption
                     | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ContainerOptions });

        CustomWindowContainer container;
        auto *windowFacet = qobject_cast<Tui::ZWindowFacet*>(w.facet(Tui::ZWindowFacet::staticMetaObject));
        windowFacet->setContainer(&container);

        QVector<Tui::ZMenuItem> menu = w.retrieveSystemMenu();
        REQUIRE(menu.size() == 6);

        CHECK(menu[0].markup() == "<m>M</m>ove");
        CHECK(menu[0].fakeShortcut() == "");
        auto moveSym = TUISYM_LITERAL("ZWindowInteractiveMove");
        CHECK(menu[0].command() == moveSym);
        CHECK(menu[0].subitems().isEmpty());

        CHECK(menu[1].markup() == "<m>R</m>esize");
        CHECK(menu[1].fakeShortcut() == "");
        auto resizeSym = TUISYM_LITERAL("ZWindowInteractiveResize");
        CHECK(menu[1].command() == resizeSym);
        CHECK(menu[1].subitems().isEmpty());

        CHECK(menu[2].markup() == "<m>A</m>utomatic");
        CHECK(menu[2].fakeShortcut() == "");
        auto automaticSym = TUISYM_LITERAL("ZWindowAutomaticPlacement");
        CHECK(menu[2].command() == automaticSym);
        CHECK(menu[2].subitems().isEmpty());

        CHECK(menu[3].markup() == "SomeContainerStuff");
        CHECK(menu[3].fakeShortcut() == "");
        auto containerSym = TUISYM_LITERAL("ContainerStuff");
        CHECK(menu[3].command() == containerSym);
        CHECK(menu[3].subitems().isEmpty());

        CHECK(menu[4].markup() == "");
        CHECK(menu[4].fakeShortcut() == "");
        CHECK(menu[4].command() == Tui::ZSymbol());
        CHECK(menu[4].subitems().isEmpty());

        CHECK(menu[5].markup() == "<m>C</m>lose");
        CHECK(menu[5].fakeShortcut() == "");
        auto closeSym = TUISYM_LITERAL("ZWindowClose");
        CHECK(menu[5].command() == closeSym);
        CHECK(menu[5].subitems().isEmpty());
    }

    SECTION("show-empty") {
        TestWindow w(t.root);
        w.setFocus();
        w.setGeometry({2, 0, 23, 10});

        CHECK(w.showSystemMenu() == false);
        t.compare();
    }

    SECTION("show-empty-seperator") {
        TestWindow w(t.root);
        w.setFocus();
        w.setGeometry({2, 0, 23, 10});
        w.menuItems = {
            {},
        };
        CHECK(w.showSystemMenu() == false);
        t.compare("show-empty");
    }

    SECTION("show-empty-key") {
        TestWindow w(t.root);
        w.setFocus();
        w.setGeometry({2, 0, 23, 10});

        FAIL_CHECK_VEC(t.checkCharEventBubbles("-", Tui::AltModifier));
        t.compare("show-empty");
    }

    SECTION("show-empty-seperator-key") {
        TestWindow w(t.root);
        w.setFocus();
        w.setGeometry({2, 0, 23, 10});
        w.menuItems = {
            {},
        };
        FAIL_CHECK_VEC(t.checkCharEventBubbles("-", Tui::AltModifier));
        t.compare("show-empty");
    }

    SECTION("show-alpha") {
        TestWindow w(t.root);
        w.setFocus();
        t.root->ensureCommandManager();
        w.setGeometry({2, 0, 23, 10});
        w.menuItems = {
            { "<m>A</m>lpha", "Ctrl-A", "SomeCommand", {}},
        };
        bool triggered = false;
        QObject::connect(new Tui::ZCommandNotifier("SomeCommand", &w), &Tui::ZCommandNotifier::activated,
                         [&triggered] {
            if (triggered) {
                FAIL("Command triggered twice");
            } else {
                triggered = true;
            }
        });

        CHECK(w.showSystemMenu() == true);
        t.compare();
        t.sendChar("a");
        CHECK(triggered == true);
    }

    SECTION("show-alpha-key") {
        TestWindow w(t.root);
        w.setFocus();
        t.root->ensureCommandManager();
        w.setGeometry({2, 0, 23, 10});
        w.menuItems = {
            { "<m>A</m>lpha", "Ctrl-A", "SomeCommand", {}},
        };
        bool triggered = false;
        QObject::connect(new Tui::ZCommandNotifier("SomeCommand", &w), &Tui::ZCommandNotifier::activated,
                         [&triggered] {
            if (triggered) {
                FAIL("Command triggered twice");
            } else {
                triggered = true;
            }
        });

        t.sendChar("-", Tui::AltModifier);
        t.compare("show-alpha");
        t.sendChar("a");
        CHECK(triggered == true);
    }

    SECTION("show-alpha-beta") {
        TestWindow w(t.root);
        t.root->ensureCommandManager();
        w.setGeometry({0, 0, 25, 10});
        w.menuItems = {
            { "<m>A</m>lpha", "Ctrl-A", "SomeCommand", {}},
            { "<m>B</m>eta", "Ctrl-B", "SomeCommand2", {}},
        };
        bool triggered = false;
        QObject::connect(new Tui::ZCommandNotifier("SomeCommand2", &w), &Tui::ZCommandNotifier::activated,
                         [&triggered] {
            if (triggered) {
                FAIL("Command triggered twice");
            } else {
                triggered = true;
            }
        });

        CHECK(w.showSystemMenu() == true);
        t.compare();
        t.sendChar("b");
        CHECK(triggered == true);
    }

}

TEST_CASE("window-interactivegeometry", "") {
    Testhelper t("window", "window-interactivegeometry", 25, 10);

    Tui::ZCommandManager *const cmdMgr = t.root->ensureCommandManager();

    bool useCustomWindowFacet = GENERATE(false, true);

    Tui::ZWindow *w;
    if (useCustomWindowFacet) {
        w = new CustomWindow(t.root);
    } else {
        w = new Tui::ZWindow(t.root);
    }

    w->setWindowTitle("Some");
    w->setFocus();
    w->setGeometry({2, 1, 21, 8});
    auto *windowFacet = qobject_cast<Tui::ZWindowFacet*>(w->facet(Tui::ZWindowFacet::staticMetaObject));

    bool useCommand = GENERATE(false, true);
    bool initialManually = GENERATE(false, true);
    windowFacet->setManuallyPlaced(initialManually);

    auto ensureAndTriggerCommand = [cmdMgr](Tui::ZSymbol sym) {
        CHECK(cmdMgr->isCommandEnabled(sym));
        cmdMgr->activateCommand(sym);
    };

    SECTION("move-activate-esc") {
        if (!useCommand) {
            w->startInteractiveMove();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveMove"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Escape);
        t.compare("base-noninteractive");
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("move-left-up-esc") {
        if (!useCommand) {
            w->startInteractiveMove();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveMove"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Left);
        t.compare("move-up-left");
        t.sendKey(Tui::Key_Escape);
        t.compare("base-noninteractive");
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("move-left-up-enter") {
        if (!useCommand) {
            w->startInteractiveMove();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveMove"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Left);
        t.compare("move-up-left");
        t.sendKey(Tui::Key_Enter);
        t.compare("noninteractive-left-up");
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
        CHECK(w->geometry() == QRect{1, 0, 21, 8});
    }

    SECTION("resize-activate-esc") {
        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Escape);
        t.compare("base-noninteractive");
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-smaller-xy-esc") {
        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Left);
        t.compare("smaller-xy");
        t.sendKey(Tui::Key_Escape);
        t.compare("base-noninteractive");
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-smaller-xy-enter") {
        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.compare("base-interactive");
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Left);
        t.compare("smaller-xy");
        t.sendKey(Tui::Key_Enter);
        t.compare("noninteractive-smaller-xy");
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
        CHECK(w->geometry() == QRect{2, 1, 20, 7});
    }

    SECTION("move-keys-and-no-limits") {
        if (!useCommand) {
            w->startInteractiveMove();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveMove"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        CHECK(w->geometry() == QRect{2, 1, 21, 8});
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{1, 1, 21, 8});
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{0, 1, 21, 8});

        // now crossing into clipping
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{-1, 1, 21, 8});
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{-2, 1, 21, 8});
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{3, 1, 21, 8});
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{4, 1, 21, 8});

        // now crossing into clipping
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{5, 1, 21, 8});
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{6, 1, 21, 8});

        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{2, 1, 21, 8});

        t.sendKey(Tui::Key_Up);
        CHECK(w->geometry() == QRect{2, 0, 21, 8});

        // now crossing into clipping
        t.sendKey(Tui::Key_Up);
        CHECK(w->geometry() == QRect{2, -1, 21, 8});
        t.sendKey(Tui::Key_Up);
        CHECK(w->geometry() == QRect{2, -2, 21, 8});

        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);

        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 2, 21, 8});

        // now crossing into clipping
        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 3, 21, 8});
        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 4, 21, 8});

        t.sendKey(Tui::Key_Escape);
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-keys") {
        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        CHECK(w->geometry() == QRect{2, 1, 21, 8});

        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{2, 1, 20, 8});
        t.sendKey(Tui::Key_Left);
        CHECK(w->geometry() == QRect{2, 1, 19, 8});

        t.sendKey(Tui::Key_Right);
        t.sendKey(Tui::Key_Right);

        t.sendKey(Tui::Key_Up);
        CHECK(w->geometry() == QRect{2, 1, 21, 7});
        t.sendKey(Tui::Key_Up);
        CHECK(w->geometry() == QRect{2, 1, 21, 6});

        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);

        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 1, 21, 9});

        // now crossing into clipping
        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 1, 21, 10});
        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 1, 21, 11});
        t.sendKey(Tui::Key_Down);
        CHECK(w->geometry() == QRect{2, 1, 21, 12});

        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{2, 1, 22, 12});
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{2, 1, 23, 12});
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{2, 1, 24, 12});
        t.sendKey(Tui::Key_Right);
        CHECK(w->geometry() == QRect{2, 1, 25, 12});

        t.sendKey(Tui::Key_Escape);
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-limit-3x3") {
        w->setMinimumSize(1, 1);

        // auto placement is potentially on here and might center w
        w->setGeometry({10, 3, 5, 5});
        REQUIRE(w->geometry() == QRect{10, 3, 5, 5});

        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{10, 3, 4, 5});

        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{10, 3, 3, 5});

        // Now at absolute minimum allowed width
        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{10, 3, 3, 5});

        t.sendKey(Tui::Key_Up);
        REQUIRE(w->geometry() == QRect{10, 3, 3, 4});
        t.sendKey(Tui::Key_Up);
        REQUIRE(w->geometry() == QRect{10, 3, 3, 3});

        // Now at absolute minimum allowed height
        t.sendKey(Tui::Key_Up);
        REQUIRE(w->geometry() == QRect{10, 3, 3, 3});

        t.sendKey(Tui::Key_Escape);
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-limit-miniumSize") {
        w->setMinimumSize(5, 6);

        // auto placement is potentially on here and might center w
        w->setGeometry({9, 2, 7, 7});
        REQUIRE(w->geometry() == QRect{9, 2, 7, 7});

        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{9, 2, 6, 7});

        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{9, 2, 5, 7});

        // Now at minimum allowed width
        t.sendKey(Tui::Key_Left);
        REQUIRE(w->geometry() == QRect{9, 2, 5, 7});

        t.sendKey(Tui::Key_Up);
        REQUIRE(w->geometry() == QRect{9, 2, 5, 6});

        // Now at minimum allowed height
        t.sendKey(Tui::Key_Up);
        REQUIRE(w->geometry() == QRect{9, 2, 5, 6});

        t.sendKey(Tui::Key_Escape);
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
    }

    SECTION("resize-limit-maximumSize") {
        w->setMaximumSize(9, 8);

        // auto placement is potentially on here and might center w
        w->setGeometry({9, 2, 7, 7});
        REQUIRE(w->geometry() == QRect{9, 2, 7, 7});

        if (!useCommand) {
            w->startInteractiveResize();
        } else {
            ensureAndTriggerCommand(TUISYM_LITERAL("ZWindowInteractiveResize"));
        }
        CHECK(windowFacet->isManuallyPlaced() == true);
        CHECK(t.terminal->keyboardGrabber() == w);

        t.sendKey(Tui::Key_Right);
        REQUIRE(w->geometry() == QRect{9, 2, 8, 7});

        t.sendKey(Tui::Key_Right);
        REQUIRE(w->geometry() == QRect{9, 2, 9, 7});

        // Now at maximum allowed width
        t.sendKey(Tui::Key_Right);
        REQUIRE(w->geometry() == QRect{9, 2, 9, 7});

        t.sendKey(Tui::Key_Down);
        REQUIRE(w->geometry() == QRect{9, 2, 9, 8});

        // Now at maximum allowed height
        t.sendKey(Tui::Key_Down);
        REQUIRE(w->geometry() == QRect{9, 2, 9, 8});

        t.sendKey(Tui::Key_Escape);
        CHECK(windowFacet->isManuallyPlaced() == initialManually);
        CHECK(t.terminal->keyboardGrabber() == nullptr);
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
            w->setBorderEdges(Tui::TopEdge);
            t->compare();
        }

        SECTION("bottom-border") {
            w->setBorderEdges(Tui::BottomEdge);
            t->compare();
        }

        SECTION("left-border") {
            w->setBorderEdges(Tui::LeftEdge);
            t->compare();
        }

        SECTION("right-border") {
            w->setBorderEdges(Tui::RightEdge);
            t->compare();
        }

        SECTION("top-left-border") {
            w->setBorderEdges(Tui::TopEdge | Tui::LeftEdge);
            t->compare();
        }

        SECTION("top-right-border") {
            w->setBorderEdges(Tui::TopEdge | Tui::RightEdge);
            t->compare();
        }

        SECTION("bottom-left-border") {
            w->setBorderEdges(Tui::BottomEdge | Tui::LeftEdge);
            t->compare();
        }

        SECTION("bottom-right-border") {
            w->setBorderEdges(Tui::BottomEdge | Tui::RightEdge);
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

    SECTION("dialog-full-charset") {
        t = std::make_unique<Testhelper>("window", "window-visual", 25, 5);

        QString kind = GENERATE("dialog", "window-with-dialog-style");
        CAPTURE(kind);

        TestBackgroundWidget background(t->root);
        background.setGeometry({0, 0, 25, 5});

        if (kind == "dialog") {
            w = new Tui::ZDialog(&background);
        } else {
            w = new Tui::ZWindow(&background);
            w->addPaletteClass("dialog");
        }
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

    SECTION("dialog-reduced-charset") {
        t = std::make_unique<Testhelper>("window", "window-visual", 25, 5, Testhelper::ReducedCharset);

        QString kind = GENERATE("dialog", "window-with-dialog-style");
        CAPTURE(kind);

        TestBackgroundWidget background(t->root);
        background.setGeometry({0, 0, 25, 5});

        if (kind == "dialog") {
            w = new Tui::ZDialog(&background);
        } else {
            w = new Tui::ZWindow(&background);
            w->addPaletteClass("dialog");
        }
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

    QString kind = GENERATE("window", "dialog");
    CAPTURE(kind);

    Tui::ZWindow *w;
    if (kind == "window") {
        w = new Tui::ZWindow(t.root);
    } else {
        w = new Tui::ZDialog(t.root);
    }

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
        Tui::Edges borderEdges;
        int left;
        int top;
        int horizontal;
        int vertical;
    };

    auto border = GENERATE(B{"all",  Tui::TopEdge | Tui::RightEdge | Tui::BottomEdge | Tui::LeftEdge, 1, 1, 2, 2},
                           B{"none",         {}, 0, 0, 0, 0},
                           B{"top",          Tui::TopEdge, 0, 1, 0, 1},
                           B{"bottom",       Tui::BottomEdge, 0, 0, 0, 1},
                           B{"left",         Tui::LeftEdge, 1, 0, 1, 0},
                           B{"right",        Tui::RightEdge, 0, 0, 1, 0},
                           B{"top+left",     Tui::TopEdge | Tui::LeftEdge, 1, 1, 1, 1},
                           B{"top+right",    Tui::TopEdge | Tui::RightEdge, 0, 1, 1, 1},
                           B{"bottom+left",  Tui::BottomEdge | Tui::LeftEdge, 1, 0, 1, 1},
                           B{"bottom+right", Tui::BottomEdge | Tui::RightEdge, 0, 0, 1, 1}
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
        t.sendKey(Tui::Key_Tab);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
    }

    SECTION("one child") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Tui::StrongFocus);
        child1->setFocus();

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child1);
    }

    SECTION("three children") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Tui::StrongFocus);
        child1->setFocus();
        auto child2 = new StubWidget(w);
        child2->setFocusPolicy(Tui::StrongFocus);
        auto child3 = new StubWidget(w);
        child3->setFocusPolicy(Tui::StrongFocus);

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child1);
    }

    SECTION("three children with order set") {
        auto child1 = new StubWidget(w);
        child1->setFocusPolicy(Tui::StrongFocus);
        child1->setFocus();
        child1->setFocusOrder(12);
        auto child2 = new StubWidget(w);
        child2->setFocusPolicy(Tui::StrongFocus);
        child2->setFocusOrder(6);
        auto child3 = new StubWidget(w);
        child3->setFocusPolicy(Tui::StrongFocus);
        child3->setFocusOrder(32);

        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Tui::Key_Tab);
        CHECK(t.terminal->focusWidget() == child1);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child2);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(t.terminal->focusWidget() == child3);
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
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

TEST_CASE("window-relayout", "") {
    Testhelper t("window", "unused", 15, 5);
    Tui::ZWindow w(t.root);
    w.setLayout(new StubLayout());

    SECTION("setBorderEdges with change") {
        Tui::ZTest::withLayoutRequestTracking(t.terminal.get(), [&](QSet<Tui::ZWidget*> *requests) {
            w.setBorderEdges({});
            CHECK(requests->contains(&w));
            requests->clear();
        });
    }

    SECTION("setBorderEdges without change") {
        Tui::ZTest::withLayoutRequestTracking(t.terminal.get(), [&](QSet<Tui::ZWidget*> *requests) {
            w.setBorderEdges(w.borderEdges());
            CHECK(!requests->contains(&w));
            requests->clear();
        });
    }

}

TEST_CASE("window-auto-placement", "") {
    Testhelper t("window", "unused", 100, 30);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 10, 4});

    SECTION("setup") {
        w->setDefaultPlacement(Tui::AlignCenter);
        CHECK(w->geometry() == QRect{46, 14, 10, 4});
    }

    SECTION("resize") {
        w->setDefaultPlacement(Tui::AlignCenter);
        CHECK(w->geometry() == QRect{46, 14, 10, 4});

        w->setGeometry({46, 14, 20, 6});
        CHECK(w->geometry() == QRect{41, 13, 20, 6});
    }

    SECTION("reparented") {
        Tui::ZWidget testParent;
        testParent.setGeometry({0, 0, 40, 40});

        w->setDefaultPlacement(Tui::AlignCenter);

        w->setParent(&testParent);
        CHECK(w->geometry() == QRect{16, 19, 10, 4});
    }

    SECTION("newly visible") {
        w->setDefaultPlacement(Tui::AlignCenter);

        w->setVisible(false);

        w->setGeometry({0, 0, 10, 4});

        w->setVisible(true);
        CHECK(w->geometry() == QRect{46, 14, 10, 4});
    }
}

TEST_CASE("window-auto-placement-container", "") {
    // with container set auto placement is no longer done by ZWindow.
    Testhelper t("window", "unused", 100, 30);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    Tui::ZWindowContainer container;
    auto *windowFacet = qobject_cast<Tui::ZWindowFacet*>(w->facet(Tui::ZWindowFacet::staticMetaObject));
    windowFacet->setContainer(&container);
    w->setGeometry({0, 0, 10, 4});

    SECTION("setup") {
        w->setDefaultPlacement(Tui::AlignCenter);
        CHECK(w->geometry() == QRect{0, 0, 10, 4});
    }

    SECTION("resize") {
        w->setDefaultPlacement(Tui::AlignCenter);
        CHECK(w->geometry() == QRect{0, 0, 10, 4});

        w->setGeometry({46, 14, 20, 6});
        CHECK(w->geometry() == QRect{46, 14, 20, 6});
    }

    SECTION("reparented") {
        Tui::ZWidget testParent;
        testParent.setGeometry({0, 0, 40, 40});

        w->setDefaultPlacement(Tui::AlignCenter);

        w->setParent(&testParent);
        CHECK(w->geometry() == QRect{0, 0, 10, 4});
    }

    SECTION("newly visible") {
        w->setDefaultPlacement(Tui::AlignCenter);

        w->setVisible(false);

        w->setGeometry({0, 0, 10, 4});

        w->setVisible(true);
        CHECK(w->geometry() == QRect{0, 0, 10, 4});
    }
}

namespace {
    class WindowWithOwnFacet : public Tui::ZWindow {
    public:
        using Tui::ZWindow::ZWindow;

    public:
        QObject *facet(const QMetaObject &metaObject) const override {
            if (metaObject.className() == Tui::ZWindowFacet::staticMetaObject.className()) {
                return &f;
            } else {
                return Tui::ZWindow::facet(metaObject);
            }
        }

        mutable Tui::ZBasicWindowFacet f;
    };
}

TEST_CASE("window-auto-placement-overridden-facet", "") {
    Testhelper t("window", "unused", 100, 30);
    WindowWithOwnFacet *w = new WindowWithOwnFacet(t.root);

    DiagnosticMessageChecker msg;
    msg.expectMessage("ZWindow::setDefaultPlacement calls with overridden WindowFacet do nothing.");
    w->setDefaultPlacement(Tui::AlignCenter);
    msg.tillHere();

    w->setGeometry({0, 0, 10, 4});

    SECTION("setup") {
        w->f.setDefaultPlacement(Tui::AlignCenter, {0, 0});
        // this does not trigger automatic placement on its own
        CHECK(w->geometry().x() == 0);
        CHECK(w->geometry().y() == 0);
    }

    SECTION("resize") {
        w->f.setDefaultPlacement(Tui::AlignCenter, {0, 0});
        // this does not trigger automatic placement on its own
        CHECK(w->geometry().x() == 0);
        CHECK(w->geometry().y() == 0);

        w->setGeometry({46, 14, 20, 6});
        CHECK(w->geometry().x() == 41);
        CHECK(w->geometry().y() == 13);
    }

    SECTION("reparented") {
        Tui::ZWidget testParent;
        testParent.setGeometry({0, 0, 40, 40});

        w->f.setDefaultPlacement(Tui::AlignCenter, {0, 0});

        w->setParent(&testParent);
        CHECK(w->geometry().x() == 16);
        CHECK(w->geometry().y() == 19);
    }

    SECTION("newly visible") {
        w->f.setDefaultPlacement(Tui::AlignCenter, {0, 0});

        w->setVisible(false);

        w->setGeometry({0, 0, 10, 4});

        w->setVisible(true);
        CHECK(w->geometry().x() == 46);
        CHECK(w->geometry().y() == 14);
    }
}
