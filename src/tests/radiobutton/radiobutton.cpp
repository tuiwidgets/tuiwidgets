#include <Tui/ZRadioButton.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"

#include <Tui/ZPalette.h>

TEST_CASE("radiobutton-base", "") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("constructor without parent") {
        // The initialisation without parent must not crash.
        delete new Tui::ZRadioButton();
        delete new Tui::ZRadioButton("m");
        delete new Tui::ZRadioButton(Tui::withMarkup, "<m>m</m>");
    }

    SECTION("constructor with parent") {
        // The initialisation must not crash.
        delete new Tui::ZRadioButton(w.get());
        delete new Tui::ZRadioButton("m", w.get());
        delete new Tui::ZRadioButton(Tui::withMarkup, "<m>m</m>", w.get());
    }

    auto checkDefaultState = [] (Tui::ZRadioButton *rb1) {
        CHECK(rb1->checked() == false);
        CHECK(rb1->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(rb1->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(rb1->focusPolicy() == Qt::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(rb1, DefaultException::SizePolicyV
                                                     | DefaultException::SizePolicyH
                                                     | DefaultException::FocusPolicy));
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZRadioButton> rb1 = std::make_unique<Tui::ZRadioButton>(w.get());
        CHECK(rb1->markup() == "");
        CHECK(rb1->text() == "");
        checkDefaultState(rb1.get());
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZRadioButton> rb1 = std::make_unique<Tui::ZRadioButton>("hallo", w.get());
        CHECK(rb1->markup() == "");
        CHECK(rb1->text() == "hallo");
        checkDefaultState(rb1.get());
    }

    SECTION("constructor-with-markup") {
        std::unique_ptr<Tui::ZRadioButton> rb1 = std::make_unique<Tui::ZRadioButton>(Tui::withMarkup, "<m>b</m>lub", w.get());
        CHECK(rb1->markup() == "<m>b</m>lub");
        CHECK(rb1->text() == "");
        checkDefaultState(rb1.get());
    }

    std::unique_ptr<Tui::ZRadioButton> rb = std::make_unique<Tui::ZRadioButton>(w.get());

    SECTION("toggle") {
        rb->toggle();
    }

    SECTION("click") {
        rb->click();
    }

    QObject::connect(rb.get(), &Tui::ZRadioButton::toggled, [] {
        FAIL("unexpected ZRadioButton::toggled signal emission");
    });

    SECTION("get-set-text") {
        rb->setText("m");
        CHECK(rb->text() == "m");
        CHECK(rb->markup() == "");
    }

    SECTION("get-set-markup") {
        rb->setMarkup("<m>m</m>");
        CHECK(rb->markup() == "<m>m</m>");
        CHECK(rb->text() == "");
    }

    SECTION("get-set-checked") {
        rb->setChecked(true);
        CHECK(rb->checked() == true);
        rb->setChecked(false);
        CHECK(rb->checked() == false);
    }

    SECTION("set-shortcut") {
        rb->setShortcut(Tui::ZKeySequence::forKey(Qt::Key_F1));
    }

    SECTION("content-margins-without-terminal") {
        rb->setContentsMargins({1, 1, 1, 1});
        QSize sh = rb->sizeHint();
        CHECK(sh.width() == -1);
        CHECK(sh.height() == -1);
    }
}

TEST_CASE("radiobutton", "") {

    Testhelper t("radiobutton", "radiobutton", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    SECTION("single-without-parent") {
        // The initialisation must not crash.
        delete new Tui::ZRadioButton();
    }

    SECTION("single-default") {
        // The initialisation must not crash.
        new Tui::ZRadioButton(w);
        t.compare();
    }

    SECTION("single-checked") {
        Tui::ZRadioButton *rb2 = new Tui::ZRadioButton("rb2", w);
        rb2->setGeometry({1, 1, 13, 1});
        rb2->setChecked(true);
        CHECK(rb2->checked());
        t.compare();
    }

    SECTION("single-unchecked") {
        Tui::ZRadioButton *rb3 = new Tui::ZRadioButton("rb3", w);
        rb3->setGeometry({1, 1, 13, 1});
        rb3->setChecked(false);
        CHECK(!rb3->checked());
        t.compare();
    }

    SECTION("single-settext") {
        Tui::ZRadioButton *rb4 = new Tui::ZRadioButton(w);
        rb4->setGeometry({1, 1, 13, 1});
        rb4->setText("rb4");
        CHECK(rb4->text() == "rb4");
        t.compare();
    }

    SECTION("single-focus") {
        Tui::ZRadioButton *rb5 = new Tui::ZRadioButton("rb5", w);
        rb5->setGeometry({1, 1, 13, 1});
        rb5->setFocus();
        CHECK(rb5->focus());
        t.compare();
    }

    SECTION("single-clicked") {
        Tui::ZRadioButton *rb6 = new Tui::ZRadioButton("rb6", w);
        rb6->setGeometry({1, 1, 13, 1});
        CHECK(!rb6->focus());
        rb6->click();
        CHECK(rb6->focus());
        CHECK(rb6->checked());
        rb6->click();
        CHECK(rb6->checked());
    }
}

TEST_CASE("radiobutton-two", "") {

    Testhelper t("radiobutton", "radiobutton-two", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
    rb1->setGeometry({1, 1, 13, 1});

    Tui::ZRadioButton *rb2 = new Tui::ZRadioButton("rb2", w);
    rb2->setGeometry({1, 2, 13, 1});

    SECTION("send-space") {
        rb1->setFocus();
        rb2->setChecked(true);

        CHECK(!rb1->checked());
        CHECK(rb2->checked());

        t.sendKey(Qt::Key_Space);
        CHECK(rb1->checked());
        CHECK(!rb2->checked());
        t.compare();
    }

    SECTION("send-tab") {
        rb1->setFocus();

        t.sendKey(Qt::Key_Tab);
        CHECK(rb2->focus());
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        t.compare();
    }

    SECTION("send-tab-enter") {
        rb1->setFocus();

        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Enter);
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        t.compare("send-tab");
    }


    SECTION("send-tab-disable") {
        rb1->setFocus();
        rb2->setEnabled(false);
        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Space);
        CHECK(rb1->checked());
        t.compare();
    }

    SECTION("toggle") {
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        rb1->setChecked(true);
        CHECK(rb1->checked());
        rb2->toggle();
        CHECK(!rb1->checked());
        CHECK(rb2->checked());
        CHECK(!rb1->focus());
        CHECK(!rb2->focus());
    }
}

TEST_CASE("radiobutton-withMarkup", "") {

    Testhelper t("radiobutton", "radiobutton-withMarkup", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    SECTION("checked") {
        w->setFocus();
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        Tui::ZRadioButton *rb2 = new Tui::ZRadioButton(Tui::withMarkup, "r<m>B</m>2", w);
        rb2->setGeometry({1, 2, 13, 1});
        Tui::ZRadioButton *rb3 = new Tui::ZRadioButton(Tui::withMarkup, "rb<m>2</m>", w);
        rb3->setGeometry({1, 3, 13, 1});

        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        t.sendChar("r", Qt::AltModifier);
        CHECK(rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        t.sendChar("B", Qt::AltModifier);
        CHECK(!rb1->checked());
        CHECK(rb2->checked());
        CHECK(!rb3->checked());
        t.sendChar("2", Qt::AltModifier);
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(rb3->checked());
        t.compare();
    }

    SECTION("disable") {
        w->setFocus();
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        Tui::ZRadioButton *rb2 = new Tui::ZRadioButton(Tui::withMarkup, "r<m>B</m>2", w);
        rb2->setGeometry({1, 2, 13, 1});
        Tui::ZRadioButton *rb3 = new Tui::ZRadioButton(w);
        rb3->setMarkup("rb<m>3</m>");
        rb3->setGeometry({1, 3, 13, 1});

        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        FAIL_CHECK_VEC(t.checkCharEventBubbles("r", Qt::AltModifier));
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        FAIL_CHECK_VEC(t.checkCharEventBubbles("B", Qt::AltModifier | Qt::ControlModifier));
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        FAIL_CHECK_VEC(t.checkCharEventBubbles("3"));
        CHECK(!rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        t.compare();
    }
}

TEST_CASE("radiobutton-emit", "") {

    Testhelper t("", "", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
    rb1->setGeometry({1, 1, 13, 1});
    rb1->setEnabled(true);
    rb1->setChecked(false);
    rb1->setFocus();

    enum { NoSignalExpected = -1, ToggleExpectedFalse = 0, ToggleExpectedTrue = 1, ToggleDone = 2} triggerState = NoSignalExpected;

    QObject::connect(rb1, &Tui::ZRadioButton::toggled, [&triggerState] (bool value) {
        if (triggerState == NoSignalExpected) {
            FAIL_CHECK("Unexpected clicked signal: " << value);
        } else if (triggerState == ToggleDone) {
            FAIL_CHECK("Duplicate clicked signal: " << value);
        } else if (triggerState == ToggleExpectedFalse) {
            if (value) {
                FAIL_CHECK("Got toggled with true, expected false");
                triggerState = NoSignalExpected;
            } else {
                triggerState = ToggleDone;
            }
        } else if (triggerState == ToggleExpectedTrue) {
            if (!value) {
                FAIL_CHECK("Got toggled with false, expected true: " << value);
                triggerState = NoSignalExpected;
            } else {
                triggerState = ToggleDone;
            }
        } else {
            FAIL("unknown value in triggerState");
        }
    });

    SECTION("Shortcut match") {
        rb1->setChecked(false);
        triggerState = ToggleExpectedTrue;
        t.sendChar("r", Qt::AltModifier);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("Shortcut match setMarkup") {
        rb1->setMarkup("r<m>b</m>1-setText");
        rb1->setChecked(false);
        triggerState = ToggleExpectedTrue;
        t.sendChar("b", Qt::AltModifier);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("space triggers toggle") {
        rb1->setText("rb1-new");
        rb1->setChecked(false);
        triggerState = ToggleExpectedTrue;
        t.sendKey(Qt::Key_Space);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("setShortcut forKey") {
        rb1->setText("rb1-new");
        rb1->setChecked(false);
        rb1->setShortcut(Tui::ZKeySequence::forKey(Qt::Key_F1));
        triggerState = ToggleExpectedTrue;
        t.sendKey(Qt::Key_F1);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("setShortcut forShortcut") {
        rb1->setText("rb1-new");
        rb1->setChecked(false);
        rb1->setShortcut(Tui::ZKeySequence::forShortcut("a"));
        triggerState = ToggleExpectedTrue;
        t.sendChar("a", Qt::ControlModifier);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("setShortcut forMnemonic") {
        rb1->setText("rb1-new");
        rb1->setChecked(false);
        rb1->setShortcut(Tui::ZKeySequence::forMnemonic("a"));
        triggerState = ToggleExpectedTrue;
        t.sendChar("a", Qt::AltModifier);
        CHECK(triggerState == ToggleDone);
    }

    SECTION("toggle of radiobutton resets other buttons") {
        Tui::ZRadioButton *rb2 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b2", w);
        rb2->setGeometry({1, 2, 13, 1});
        triggerState = ToggleExpectedFalse;
        rb2->toggle();
        CHECK(triggerState == ToggleDone);
    }

    SECTION("mnemonic-via-markup-reset-via-text") {
        rb1->setMarkup("<m>O</m>K");
        rb1->setText("OK");
        FAIL_CHECK_VEC(t.checkCharEventBubbles("o", Qt::AltModifier));
    }

    SECTION("mnemonic-via-markup-reset-via-markup") {
        rb1->setMarkup("<m>O</m>K");
        rb1->setMarkup("OK");
        FAIL_CHECK_VEC(t.checkCharEventBubbles("o", Qt::AltModifier));
    }
}

TEST_CASE("radiobutton-color", "") {

    Testhelper t("radiobutton", "radiobutton-color", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    w->setGeometry({0, 0, 15, 5});

    SECTION("single-black") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-black-focus-checked") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setChecked(true);
        rb1->setFocus();
        t.compare();
    }

    SECTION("single-black-focus-disable") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setFocus();
        rb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-black-markup") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-black-disable-markup") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        t.compare();
    }


    // cyan
    SECTION("single-cyan") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-cyan-focus-checked") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setFocus();
        rb1->setChecked(true);
        t.compare();
    }

    SECTION("single-cyan-disable") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-cyan-focus-markup") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setFocus();
        t.compare();
    }

    SECTION("single-cyan-markup-disable") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        t.compare("single-cyan-disable");
    }


    // gray
    SECTION("single-gray") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-gray-focus-checked") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setChecked(true);
        rb1->setFocus();
        t.compare();
    }

    SECTION("single-gray-disable") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-gray-markup") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setFocus();
        t.compare();
    }

    SECTION("single-gray-markup-disable") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton(Tui::withMarkup, "<m>r</m>b1", w);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setEnabled(false);
        t.compare("single-gray-disable");
    }
}

TEST_CASE("radiobutton-margin", "") {

    Testhelper t("radiobutton", "radiobutton-margin", 15, 7);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 7});

    SECTION("single-margin-2-2-2-2") {
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", w);
        rb1->setGeometry({1, 1, 13, 5});
        CHECK(rb1->sizeHint().height() == 1);
        CHECK(rb1->sizeHint().width() == 8);
        rb1->setContentsMargins({2, 2, 2, 2});
        CHECK(rb1->checked() == false);
        CHECK(rb1->sizeHint().height() == 5);
        CHECK(rb1->sizeHint().width() == 12);
        t.compare();
    }
}

TEST_CASE("radiobutton-multiparent", "") {

    Testhelper t("radiobutton", "radiobutton-multiparent", 15, 10);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 10});

    SECTION("AandBButton") {
        Tui::ZWidget *a = new Tui::ZWidget(w);
        //a->setText("A");
        a->setGeometry({1, 1, 13, 4});
        Tui::ZRadioButton *rb1 = new Tui::ZRadioButton("rb1", a);
        rb1->setGeometry({1, 1, 13, 1});
        rb1->setFocus();
        Tui::ZRadioButton *rb2 = new Tui::ZRadioButton("rb2", a);
        rb2->setGeometry({1, 2, 13, 1});

        Tui::ZWidget *b = new Tui::ZWidget(w);
        //b->setText("B");
        b->setGeometry({1, 4, 13, 4});
        Tui::ZRadioButton *rb3 = new Tui::ZRadioButton("rb3", b);
        rb3->setGeometry({1, 1, 13, 1});
        Tui::ZRadioButton *rb4 = new Tui::ZRadioButton("rb4", b);
        rb4->setGeometry({1, 2, 13, 1});

        CHECK(!rb1->checked());
        t.sendKey(Qt::Key_Space);
        CHECK(rb1->checked());
        CHECK(!rb2->checked());
        CHECK(!rb3->checked());
        CHECK(!rb4->checked());

        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Space);
        CHECK(!rb1->checked());
        CHECK(rb2->checked());
        CHECK(!rb3->checked());
        CHECK(!rb4->checked());

        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Space);
        CHECK(!rb1->checked());
        CHECK(rb2->checked());
        CHECK(rb3->checked());
        CHECK(!rb4->checked());

        t.sendKey(Qt::Key_Tab);
        t.sendKey(Qt::Key_Space);
        CHECK(!rb1->checked());
        CHECK(rb2->checked());
        CHECK(!rb3->checked());
        CHECK(rb4->checked());
    }
}

