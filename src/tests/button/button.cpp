#include <Tui/ZButton.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"

#include <Tui/ZPalette.h>
#include <Tui/ZDefaultWidgetManager.h>

namespace {
    class StubDefaultWidgetManager : public Tui::ZDefaultWidgetManager {
    public:
        StubDefaultWidgetManager() {}

    public:
        void setDefaultWidget(Tui::ZWidget *w) override {
            CHECK(expectSetDefault);
            CHECK(expectedNewDefault == w);
            defaultWidgetValue = w;
            expectSetDefault = false;
            expectedNewDefault = nullptr;
        }

        Tui::ZWidget *defaultWidget() override {
            return defaultWidgetValue;
        }

        bool isDefaultWidgetActive() override {
            return defaultWidgetActiveValue;
        }

        bool expectSetDefault = false;
        Tui::ZWidget *expectedNewDefault = nullptr;

        Tui::ZWidget *defaultWidgetValue = nullptr;
        bool defaultWidgetActiveValue = false;
    };


    class TestBackgroundWithDlgManager : public TestBackground {
    public:
        explicit TestBackgroundWithDlgManager(Tui::ZWidget *parent) : TestBackground(parent) {}

        QObject *facet(const QMetaObject &metaObject) override {
            if (hasDefaultManager && (metaObject.className()  == Tui::ZDefaultWidgetManager::staticMetaObject.className())) {
                return &defaultWidgetManager;
            } else {
                return TestBackground::facet(metaObject);
            }
        }
        bool hasDefaultManager = true;

    public:
        StubDefaultWidgetManager defaultWidgetManager;
    };
}

TEST_CASE("button-base") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("constructor without parent") {
        // The initialisation without parent must not crash.
        delete new Tui::ZButton();
        delete new Tui::ZButton("m");
        delete new Tui::ZButton(Tui::withMarkup, "<m>m</m>");
    }

    SECTION("constructor with parent") {
        // The initialisation must not crash.
        delete new Tui::ZButton(w.get());
        delete new Tui::ZButton("m", w.get());
        delete new Tui::ZButton(Tui::withMarkup, "<m>m</m>", w.get());
    }

    SECTION("constructor") {
        std::unique_ptr<Tui::ZButton> button = std::make_unique<Tui::ZButton>(w.get());
        CHECK(button->text() == "");
        CHECK(button->markup() == "");
        CHECK(button->sizePolicyH() == Tui::SizePolicy::Minimum);
        CHECK(button->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(button->focusPolicy() == Qt::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(button.get(), DefaultException::SizePolicyV
                                                           | DefaultException::SizePolicyH
                                                           | DefaultException::FocusPolicy));
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZButton> button = std::make_unique<Tui::ZButton>("m", w.get());
        CHECK(button->text() == "m");
        CHECK(button->markup() == "");
        CHECK(button->sizePolicyH() == Tui::SizePolicy::Minimum);
        CHECK(button->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(button->focusPolicy() == Qt::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(button.get(), DefaultException::SizePolicyV
                                                           | DefaultException::SizePolicyH
                                                           | DefaultException::FocusPolicy));
    }

    SECTION("constructor-with-markup") {
        std::unique_ptr<Tui::ZButton> button = std::make_unique<Tui::ZButton>(Tui::withMarkup, "<m>m</m>", w.get());
        CHECK(button->text() == "");
        CHECK(button->markup() == "<m>m</m>");
        CHECK(button->sizePolicyH() == Tui::SizePolicy::Minimum);
        CHECK(button->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(button->focusPolicy() == Qt::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(button.get(), DefaultException::SizePolicyV
                                                           | DefaultException::SizePolicyH
                                                           | DefaultException::FocusPolicy));
    }

    std::unique_ptr<Tui::ZButton> button = std::make_unique<Tui::ZButton>(w.get());

    QObject::connect(button.get(), &Tui::ZButton::clicked, [] {
        FAIL("unexpected ZButton::clicked signal emission");
    });

    SECTION("get-set-text") {
        button->setText("m");
        CHECK(button->text() == "m");
        CHECK(button->markup() == "");
    }

    SECTION("get-set-markup") {
        button->setMarkup("<m>m</m>");
        CHECK(button->markup() == "<m>m</m>");
        CHECK(button->text() == "");
    }

    SECTION("content-margins-without-terminal") {
        // TODO contents margins vs sizeHint vs no terminal
        button->setContentsMargins({1, 1, 1, 1});
        QSize sh = button->sizeHint();
        CHECK(sh.width() == -1);
        CHECK(sh.height() == -1);
    }
}

TEST_CASE("button-default-manager") {
    Testhelper t("", "", 15, 5);

    TestBackgroundWithDlgManager *w = new TestBackgroundWithDlgManager(t.root);
    w->hasDefaultManager = true;

    Tui::ZButton *button = new Tui::ZButton(w);

    QObject::connect(button, &Tui::ZButton::clicked, [] {
        FAIL("unexpected ZButton::clicked signal emission");
    });

    SECTION("no-default-manager") {
        w->hasDefaultManager = false;
        CHECK(button->isDefault() == false);
        // no default manager means this is a no-op.
        button->setDefault(true);
        CHECK(button->isDefault() == false);
    }

    SECTION("set-and-unset-default") {
        CHECK(button->isDefault() == false);
        w->defaultWidgetManager.expectSetDefault = true;
        w->defaultWidgetManager.expectedNewDefault = button;
        button->setDefault(true);
        CHECK(w->defaultWidgetManager.expectSetDefault == false);
        CHECK(w->defaultWidgetManager.defaultWidgetValue == button);
        CHECK(button->isDefault() == true);

        w->defaultWidgetManager.expectSetDefault = true;
        w->defaultWidgetManager.expectedNewDefault = nullptr;
        button->setDefault(false);
        CHECK(w->defaultWidgetManager.expectSetDefault == false);
        CHECK(w->defaultWidgetManager.defaultWidgetValue == nullptr);
        CHECK(button->isDefault() == false);
    }

    SECTION("unset-default-when-other-widget-is-default") {
        Tui::ZWidget dummyWidget;
        CHECK(button->isDefault() == false);
        w->defaultWidgetManager.defaultWidgetValue = &dummyWidget;

        w->defaultWidgetManager.expectSetDefault = false;
        button->setDefault(false);
        CHECK(w->defaultWidgetManager.defaultWidgetValue == &dummyWidget);
        CHECK(button->isDefault() == false);
    }
}


TEST_CASE("button-display", "") {

    Testhelper t("button", "button-display", 15, 5);

    TestBackgroundWithDlgManager *w = new TestBackgroundWithDlgManager(t.root);
    w->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    w->hasDefaultManager = false;

    QString windowType = GENERATE(as<QString>(), "", "dialog", "cyan");

    if (windowType.size()) {
        t.namePrefix += "-" + windowType;
        w->setPaletteClass({"window", windowType });
        if (windowType.size()) {
            UNSCOPED_INFO(windowType.toStdString());
        }
    }

    w->setGeometry({0, 0, 15, 5});

    Tui::ZButton *button = new Tui::ZButton(w);

    QObject::connect(button, &Tui::ZButton::clicked, [] {
        FAIL("unexpected ZButton::clicked signal emission");
    });

    bool hasFocus = GENERATE(false, true);

    if (hasFocus) {
        t.namePrefix += " focus";
        button->setFocus();
        UNSCOPED_INFO("focus");
    }

    SECTION("empty") {
        button->setText("");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 6);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare();
    }

    SECTION("content-margins-empty") {
        button->setContentsMargins({1, 1, 1, 1});
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 3);
    }

    SECTION("O") {
        button->setText("O");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 7);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare();
    }

    SECTION("OK") {
        w->defaultWidgetManager.defaultWidgetActiveValue = GENERATE(false, true);
        CAPTURE(w->defaultWidgetManager.defaultWidgetActiveValue);
        button->setText("OK");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare();
    }

    SECTION("clipped-cancel") {
        button->setText("Cancel");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 12);
        CHECK(sh.height() == 1);
        button->setGeometry({2, 2, 8, 1});
        t.compare();
    }

    SECTION("clipped-double-width") {
        button->setText("あい");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 10);
        CHECK(sh.height() == 1);
        button->setGeometry({2, 2, 7, 1});
        t.compare();
    }

    SECTION("OK-with-mnemonic") {
        w->defaultWidgetManager.defaultWidgetActiveValue = GENERATE(false, true);
        CAPTURE(w->defaultWidgetManager.defaultWidgetActiveValue);
        button->setMarkup("<m>O</m>K");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare();
    }

    SECTION("OK-default") {
        w->hasDefaultManager = true;
        w->defaultWidgetManager.defaultWidgetValue = button;
        w->defaultWidgetManager.defaultWidgetActiveValue = true;
        button->setText("OK");
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        CHECK(button->isDefault() == true);
        button->setGeometry({{2, 2}, sh});

        if (hasFocus) {
            // When focused the default button looks just like a non default button.
            t.compare("OK");
        } else {
            t.compare();
        }
    }

    SECTION("OK-default-suppressed") {
        if (hasFocus) {
            // focus state is meaningless for this test, pick hasFocus to avoid duplicate results
            w->hasDefaultManager = true;
            w->defaultWidgetManager.defaultWidgetValue = button;
            w->defaultWidgetManager.defaultWidgetActiveValue = false;
            button->setText("OK");
            Tui::ZWidget fake(w);
            fake.setFocus();

            QSize sh = button->sizeHint();
            CHECK(sh.width() == 8);
            CHECK(sh.height() == 1);
            button->setGeometry({{2, 2}, sh});
            t.compare();
        }
    }

    SECTION("OK-disabled") {
        w->defaultWidgetManager.defaultWidgetActiveValue = GENERATE(false, true);
        CAPTURE(w->defaultWidgetManager.defaultWidgetActiveValue);
        button->setText("OK");
        button->setEnabled(false);
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare();
    }

    SECTION("OK-with-mnemonic-disabled") {
        w->defaultWidgetManager.defaultWidgetActiveValue = GENERATE(false, true);
        CAPTURE(w->defaultWidgetManager.defaultWidgetActiveValue);
        button->setMarkup("<m>O</m>K");
        button->setEnabled(false);
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        button->setGeometry({{2, 2}, sh});
        t.compare("OK-disabled");
    }

    SECTION("OK-default-disabled") {
        w->hasDefaultManager = true;
        w->defaultWidgetManager.defaultWidgetValue = button;
        w->defaultWidgetManager.defaultWidgetActiveValue = true;
        button->setText("OK");
        button->setEnabled(false);
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 8);
        CHECK(sh.height() == 1);
        CHECK(button->isDefault() == true);
        button->setGeometry({{2, 2}, sh});
        t.compare("OK-disabled");
    }

    SECTION("OK-content-margins") {
        button->setText("OK");
        button->setContentsMargins({3, 1, 4, 2});
        QSize sh = button->sizeHint();
        CHECK(sh.width() == 15);
        CHECK(sh.height() == 4);
        button->setGeometry({{1, 1}, sh});
        t.compare();
    }
}

TEST_CASE("button-size", "") {

    Testhelper t("button", "button-size", 15, 5);
    TestBackgroundWithDlgManager *w = new TestBackgroundWithDlgManager(t.root);
    w->hasDefaultManager = false;
    w->setGeometry({0, 0, 15, 5});

    Tui::ZButton *button = new Tui::ZButton(w);

    QObject::connect(button, &Tui::ZButton::clicked, [] {
        FAIL("unexpected ZButton::clicked signal emission");
    });

    button->setFocus();

    SECTION("OK-01") {
        button->setText("OK");
        button->setGeometry({2, 2, 1, 1});
        t.compare();
    }

    SECTION("OK-02") {
        button->setText("OK");
        button->setGeometry({2, 2, 2, 1});
        t.compare();
    }

    SECTION("OK 03") {
        button->setText("OK");
        button->setGeometry({2, 2, 3, 1});
        t.compare();
    }

    SECTION("OK-04") {
        button->setText("OK");
        button->setGeometry({2, 2, 4, 1});
        t.compare();
    }

    SECTION("OK-05") {
        button->setText("OK");
        button->setGeometry({2, 2, 5, 1});
        t.compare();
    }

    SECTION("OK-06") {
        button->setText("OK");
        button->setGeometry({2, 2, 6, 1});
        t.compare();
    }

    SECTION("OK-07") {
        button->setText("OK");
        button->setGeometry({2, 2, 7, 1});
        t.compare();
    }

    SECTION("OK-08") {
        button->setText("OK");
        button->setGeometry({2, 2, 8, 1});
        t.compare();
    }

    SECTION("OK-09") {
        button->setText("OK");
        button->setGeometry({2, 2, 9, 1});
        t.compare();
    }

    SECTION("OK-10") {
        button->setText("OK");
        button->setGeometry({2, 2, 10, 1});
        t.compare();
    }
}

TEST_CASE("button-behavior", "") {

    Testhelper t("", "", 15, 5);
    TestBackgroundWithDlgManager *w = new TestBackgroundWithDlgManager(t.root);
    w->hasDefaultManager = false;
    w->setGeometry({0, 0, 15, 5});

    Tui::ZButton *button = new Tui::ZButton(w);

    enum { NoSignalExpected = -1, ClickedExpected = 0, ClickedDone = 1} triggerState = NoSignalExpected;

    QObject::connect(button, &Tui::ZButton::clicked, [&triggerState] {
        if (triggerState == NoSignalExpected) {
            FAIL_CHECK("Unexpected clicked signal");
        } else if (triggerState == ClickedDone) {
            FAIL_CHECK("Duplicate clicked signal");
        } else if (triggerState == ClickedExpected) {
            triggerState = ClickedDone;
        } else {
            FAIL("unknown value in triggerState");
        }
    });

    button->setText("OK");
    button->setGeometry({2, 2, 8, 1});

    SECTION("focused") {
        button->setFocus();
        SECTION("enter") {
            triggerState = ClickedExpected;
            t.sendKey(Qt::Key_Enter);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("enter-with-modifier") {
            FAIL_CHECK_VEC(t.checkKeyEventBubbles(Qt::Key_Enter, Qt::KeyboardModifier::AltModifier));
            FAIL_CHECK_VEC(t.checkKeyEventBubbles(Qt::Key_Enter, Qt::KeyboardModifier::ShiftModifier));
            FAIL_CHECK_VEC(t.checkKeyEventBubbles(Qt::Key_Enter, Qt::KeyboardModifier::ControlModifier));
        }

        SECTION("enter-on-disabled") {
            button->setEnabled(false);
            t.sendKeyToWidget(button, Qt::Key_Enter);
            t.sendKeyToWidget(button, Qt::Key_Enter, Qt::KeyboardModifier::AltModifier);
            t.sendKeyToWidget(button, Qt::Key_Enter, Qt::KeyboardModifier::ShiftModifier);
            t.sendKeyToWidget(button, Qt::Key_Enter, Qt::KeyboardModifier::ControlModifier);
        }

        SECTION("space") {
            triggerState = ClickedExpected;
            t.sendKey(Qt::Key_Space);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("space-with-modifier") {
            FAIL_CHECK_VEC(t.checkCharEventBubbles(" ", Qt::KeyboardModifier::AltModifier));
            FAIL_CHECK_VEC(t.checkCharEventBubbles(" ", Qt::KeyboardModifier::ShiftModifier));
            FAIL_CHECK_VEC(t.checkCharEventBubbles(" ", Qt::KeyboardModifier::ControlModifier));
        }

        SECTION("space-on-disabled") {
            button->setEnabled(false);
            t.sendKeyToWidget(button, Qt::Key_Space);
            t.sendKeyToWidget(button, Qt::Key_Space, Qt::KeyboardModifier::AltModifier);
            t.sendKeyToWidget(button, Qt::Key_Space, Qt::KeyboardModifier::ShiftModifier);
            t.sendKeyToWidget(button, Qt::Key_Space, Qt::KeyboardModifier::ControlModifier);
        }
    }

    SECTION("window-focus") {
        w->setFocus();
        SECTION("mnemonic-via-markup") {
            button->setMarkup("<m>O</m>K");
            triggerState = ClickedExpected;
            t.sendChar("o", Qt::AltModifier);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("mnemonic-via-markup-and-captial-letter") {
            button->setMarkup("<m>O</m>K");
            triggerState = ClickedExpected;
            t.sendChar("O", Qt::AltModifier);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("mnemonic-via-setShortcut") {
            button->setShortcut(Tui::ZKeySequence::forMnemonic("o"));
            triggerState = ClickedExpected;
            t.sendChar("o", Qt::AltModifier);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("mnemonic-via-setShortcut-with-focus-and-captial-letter") {
            button->setShortcut(Tui::ZKeySequence::forMnemonic("o"));
            triggerState = ClickedExpected;
            t.sendChar("O", Qt::AltModifier);
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("mnemonic-via-markup-reset-via-text") {
            button->setMarkup("<m>O</m>K");
            button->setText("OK");
            FAIL_CHECK_VEC(t.checkCharEventBubbles("o", Qt::AltModifier));
        }

        SECTION("mnemonic-via-markup-reset-via-markup") {
            button->setMarkup("<m>O</m>K");
            button->setMarkup("OK");
            FAIL_CHECK_VEC(t.checkCharEventBubbles("o", Qt::AltModifier));
        }

    }

    SECTION("unfocused") {
        SECTION("click-method") {
            triggerState = ClickedExpected;
            button->click();
            CHECK(triggerState == ClickedDone);
            CHECK(t.terminal->focusWidget() == button);
        }

        SECTION("claims-enter-event") {
            {
                QEvent e {Tui::ZEventType::queryAcceptsEnter()};
                e.ignore();
                QCoreApplication::sendEvent(button, &e);
                CHECK(e.isAccepted() == true);
            }
            button->setEnabled(false);
            {
                QEvent e {Tui::ZEventType::queryAcceptsEnter()};
                e.ignore();
                QCoreApplication::sendEvent(button, &e);
                CHECK(e.isAccepted() == false);
            }
        }
    }

}
