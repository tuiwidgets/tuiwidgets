// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZCheckBox.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"

#include <Tui/ZPalette.h>

TEST_CASE("checkbox-base", "") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("constructor without parent") {
        // The initialisation without parent must not crash.
        delete new Tui::ZCheckBox();
        delete new Tui::ZCheckBox("m");
        delete new Tui::ZCheckBox(Tui::withMarkup, "<m>m</m>");
    }

    SECTION("constructor with parent") {
        // The initialisation must not crash.
        delete new Tui::ZCheckBox(w.get());
        delete new Tui::ZCheckBox("m", w.get());
        delete new Tui::ZCheckBox(Tui::withMarkup, "<m>m</m>", w.get());
    }

    auto checkDefaultState = [] (Tui::ZCheckBox *cb1) {
        CHECK(!cb1->isTristate());
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb1->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(cb1->sizePolicyV() == Tui::SizePolicy::Fixed);
        CHECK(cb1->focusPolicy() == Tui::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(cb1, DefaultException::SizePolicyV
                                                     | DefaultException::SizePolicyH
                                                     | DefaultException::FocusPolicy));
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZCheckBox> cb1 = std::make_unique<Tui::ZCheckBox>(w.get());
        CHECK(cb1->markup() == "");
        CHECK(cb1->text() == "");
        checkDefaultState(cb1.get());
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZCheckBox> cb1 = std::make_unique<Tui::ZCheckBox>("hallo", w.get());
        CHECK(cb1->markup() == "");
        CHECK(cb1->text() == "hallo");
        checkDefaultState(cb1.get());
    }

    SECTION("constructor-with-markup") {
        std::unique_ptr<Tui::ZCheckBox> cb1 = std::make_unique<Tui::ZCheckBox>(Tui::withMarkup, "<m>b</m>lub", w.get());
        CHECK(cb1->markup() == "<m>b</m>lub");
        CHECK(cb1->text() == "");
        checkDefaultState(cb1.get());
    }

    std::unique_ptr<Tui::ZCheckBox> cb = std::make_unique<Tui::ZCheckBox>(w.get());

    SECTION("toggle") {
        cb->toggle();
    }

    SECTION("click") {
        cb->click();
    }

    QObject::connect(cb.get(), &Tui::ZCheckBox::stateChanged, [] {
        FAIL("unexpected ZCheckBox::stateChanged signal emission");
    });

    SECTION("get-set-text") {
        cb->setText("m");
        CHECK(cb->text() == "m");
        CHECK(cb->markup() == "");
    }

    SECTION("get-set-markup") {
        cb->setMarkup("<m>m</m>");
        CHECK(cb->markup() == "<m>m</m>");
        CHECK(cb->text() == "");
    }

    SECTION("get-set-checkstate") {
        cb->setCheckState(Tui::Checked);
        CHECK(cb->checkState() == Tui::Checked);
        cb->setCheckState(Tui::Unchecked);
        CHECK(cb->checkState() == Tui::Unchecked);
        cb->setCheckState(Tui::PartiallyChecked);
        CHECK(cb->checkState() == Tui::PartiallyChecked);
    }

    SECTION("get-set-tristate") {
        cb->setTristate(true);
        CHECK(cb->isTristate() == true);
        cb->setTristate(false);
        CHECK(cb->isTristate() == false);
    }

    SECTION("set-shortcut") {
        cb->setShortcut(Tui::ZKeySequence::forKey(Tui::Key_F1));
    }

    SECTION("content-margins-without-terminal") {
        cb->setContentsMargins({1, 1, 1, 1});
        QSize sh = cb->sizeHint();
        CHECK(sh.width() == -1);
        CHECK(sh.height() == -1);
    }
}

TEST_CASE("checkbox", "") {
    Testhelper t("checkbox", "checkbox", 15, 5);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 15, 5});

    SECTION("single-default") {
        // The initialisation must not crash.
        new Tui::ZCheckBox(w);
        t.compare();
    }

    Tui::ZCheckBox *cb = new Tui::ZCheckBox("cb1", w);
    cb->setGeometry({1, 1, 13, 1});

    SECTION("single-geometry") {
        CHECK(cb->checkState() == Tui::CheckState::Unchecked);
    }

    SECTION("single-checked") {
        cb->setCheckState(Tui::CheckState::Checked);
        CHECK(cb->checkState() == Tui::CheckState::Checked);
        t.compare();
    }

    SECTION("single-unchecked") {
        cb->setCheckState(Tui::CheckState::Unchecked);
        CHECK(cb->checkState() == Tui::CheckState::Unchecked);
        t.compare();
    }

    SECTION("single-settext") {
        cb->setText("cb4");
        CHECK(cb->text() == "cb4");
        t.compare();
    }

    SECTION("single-focus") {
        cb->setFocus();
        CHECK(cb->focus());
        t.compare();
    }

    SECTION("single-click") {
        CHECK(!cb->focus());
        cb->click();
        CHECK(cb->focus());
        CHECK(cb->checkState() == Tui::CheckState::Checked);
        cb->click();
        CHECK(cb->checkState() == Tui::CheckState::Unchecked);
    }

    SECTION("single-space") {
        cb->setFocus();
        t.sendKey(Tui::Key_Space);
        CHECK(cb->checkState() == Tui::CheckState::Checked);
    }

    SECTION("single-enter") {
        cb->setFocus();
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Enter));
        CHECK(cb->checkState() == Tui::CheckState::Unchecked);
    }

    SECTION("single-tristate") {
        cb->setCheckState(Tui::CheckState::PartiallyChecked);
        CHECK(cb->checkState() == Tui::CheckState::PartiallyChecked);
        CHECK(!cb->isTristate());
        t.compare();
    }

    SECTION("single-tristate-focus") {
        cb->setFocus();
        CHECK(!cb->isTristate());
        cb->setCheckState(Tui::CheckState::PartiallyChecked);
        CHECK(cb->checkState() == Tui::CheckState::PartiallyChecked);
        CHECK(!cb->isTristate());
        t.compare();
    }

    SECTION("single-tristate-disable") {
        cb->setCheckState(Tui::CheckState::PartiallyChecked);
        cb->setEnabled(false);
        CHECK(cb->checkState() == Tui::CheckState::PartiallyChecked);
        t.compare();
    }

    SECTION("single-tristate-space") {
        cb->setFocus();
        cb->setTristate(true);
        CHECK(cb->isTristate());
        cb->setCheckState(Tui::CheckState::Checked);
        t.sendKey(Tui::Key_Space);
        CHECK(cb->checkState() == Tui::CheckState::PartiallyChecked);
    }
}

TEST_CASE("checkbox-two", "") {
    Testhelper t("checkbox", "checkbox-two", 15, 5);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
    cb1->setGeometry({1, 1, 13, 1});

    Tui::ZCheckBox *cb2 = new Tui::ZCheckBox("cb2", w);
    cb2->setGeometry({1, 2, 13, 1});

    SECTION("send-tab") {
        cb1->setFocus();
        t.sendKey(Tui::Key_Tab);
        CHECK(cb2->focus());
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        t.compare();
    }

    SECTION("send-tab-disable") {
        cb1->setFocus();
        cb2->setEnabled(false);
        t.sendKey(Tui::Key_Tab);
        t.sendKey(Tui::Key_Space);
        CHECK(cb1->checkState() == Tui::CheckState::Checked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        t.compare();
    }

    SECTION("toggle") {
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        cb2->toggle();
        cb1->setCheckState(Tui::CheckState::Checked);
        CHECK(cb1->checkState() == Tui::CheckState::Checked);
        CHECK(cb2->checkState() == Tui::CheckState::Checked);
        cb2->toggle();
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(!cb1->focus());
        CHECK(!cb2->focus());
    }
}

TEST_CASE("checkbox-withMarkup", "") {
    Testhelper t("checkbox", "checkbox-withMarkup", 15, 5);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 15, 5});

    SECTION("checked") {
        w->setFocus();
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        Tui::ZCheckBox *cb2 = new Tui::ZCheckBox(Tui::withMarkup, "c<m>B</m>2", w);
        cb2->setGeometry({1, 2, 13, 1});
        Tui::ZCheckBox *cb3 = new Tui::ZCheckBox(Tui::withMarkup, "cb<m>2</m>", w);
        cb3->setGeometry({1, 3, 13, 1});

        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        t.sendChar("c", Tui::AltModifier);
        CHECK(cb1->checkState() == Tui::CheckState::Checked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        t.sendChar("B", Tui::AltModifier);
        CHECK(cb1->checkState() == Tui::CheckState::Checked);
        CHECK(cb2->checkState() == Tui::CheckState::Checked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        t.sendChar("2", Tui::AltModifier);
        CHECK(cb1->checkState() == Tui::CheckState::Checked);
        CHECK(cb2->checkState() == Tui::CheckState::Checked);
        CHECK(cb3->checkState() == Tui::CheckState::Checked);
        t.compare();
    }

    SECTION("disable") {
        w->setFocus();
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        Tui::ZCheckBox *cb2 = new Tui::ZCheckBox(Tui::withMarkup, "c<m>B</m>2", w);
        cb2->setGeometry({1, 2, 13, 1});
        Tui::ZCheckBox *cb3 = new Tui::ZCheckBox(w);
        cb3->setMarkup("cb<m>2</m>");
        cb3->setGeometry({1, 3, 13, 1});

        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        FAIL_CHECK_VEC(t.checkCharEventBubbles("c", Tui::AltModifier));
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        FAIL_CHECK_VEC(t.checkCharEventBubbles("B", Tui::AltModifier | Tui::ControlModifier));
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        FAIL_CHECK_VEC(t.checkCharEventBubbles("2"));
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb2->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb3->checkState() == Tui::CheckState::Unchecked);
        t.compare();
    }

    SECTION("mnemonic-via-markup-reset-via-text") {
        w->setFocus();
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setText("cb1");
        cb1->setGeometry({1, 1, 13, 1});

        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        FAIL_CHECK_VEC(t.checkCharEventBubbles("c", Tui::AltModifier));
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
    }

    SECTION("mnemonic-via-markup-reset-via-markup") {
        w->setFocus();
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setMarkup("cb1");
        cb1->setGeometry({1, 1, 13, 1});

        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        FAIL_CHECK_VEC(t.checkCharEventBubbles("c", Tui::AltModifier));
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
    }

}

TEST_CASE("checkbox-emit", "") {
    Testhelper t("", "", 15, 5);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
    cb1->setGeometry({1, 1, 13, 1});
    cb1->setEnabled(true);
    cb1->setCheckState(Tui::CheckState::Unchecked);
    cb1->setFocus();

    int triggerCountC = 0;
    int triggerCountU = 0;
    int triggerCountP = 0;

    QObject::connect(cb1, &Tui::ZCheckBox::stateChanged, [&triggerCountC, &triggerCountU, &triggerCountP](Tui::CheckState value) {
        switch (value) {
            case Tui::CheckState::Checked:
                triggerCountC++;
                break;
            case Tui::CheckState::Unchecked:
                triggerCountU++;
                break;
            case Tui::CheckState::PartiallyChecked:
                triggerCountP++;
                break;
            default:
                FAIL("bogus");
        }
    });

    CHECK(cb1->focusPolicy() == Tui::StrongFocus);
    CHECK(cb1->maximumSize().width() == Tui::tuiMaxSize);
    CHECK(cb1->maximumSize().height() == Tui::tuiMaxSize);
    CHECK(cb1->sizePolicyV() == Tui::SizePolicy::Fixed);
    CHECK(cb1->sizePolicyH() == Tui::SizePolicy::Expanding);

    CHECK(cb1->text() == "");
    CHECK(cb1->markup() == "<m>c</m>b1");

    cb1->setCheckState(Tui::CheckState::Unchecked);
    t.sendChar("c", Tui::AltModifier);
    CHECK(triggerCountC == 1);
    CHECK(triggerCountU == 0);

    cb1->setMarkup("c<m>b</m>1-setText");
    CHECK(cb1->text() == "");
    CHECK(cb1->markup() == "c<m>b</m>1-setText");

    cb1->setCheckState(Tui::CheckState::Unchecked);
    t.sendChar("b", Tui::AltModifier);
    CHECK(triggerCountC == 2);
    CHECK(triggerCountU == 0);

    cb1->setText("cb1-new");
    CHECK(cb1->text() == "cb1-new");
    CHECK(cb1->markup() == "");

    cb1->setCheckState(Tui::CheckState::Unchecked);
    t.sendKey(Tui::Key_Space);
    CHECK(triggerCountC == 3);
    CHECK(triggerCountU == 0);

    cb1->setCheckState(Tui::CheckState::Unchecked);
    cb1->setShortcut(Tui::ZKeySequence::forKey(Tui::Key_F1));
    t.sendKey(Tui::Key_F1);
    CHECK(triggerCountC == 4);
    CHECK(triggerCountU == 0);

    cb1->setCheckState(Tui::CheckState::Unchecked);
    cb1->setShortcut(Tui::ZKeySequence::forShortcut("a"));
    t.sendChar("a", Tui::ControlModifier);
    CHECK(triggerCountC == 5);
    CHECK(triggerCountU == 0);

    cb1->setCheckState(Tui::CheckState::Unchecked);
    cb1->setShortcut(Tui::ZKeySequence::forMnemonic("a"));
    t.sendChar("a", Tui::AltModifier);
    CHECK(triggerCountC == 6);
    CHECK(triggerCountU == 0);

    cb1->toggle();
    CHECK(triggerCountC == 6);
    CHECK(triggerCountU == 1);
    CHECK(triggerCountP == 0);

    cb1->setTristate();
    cb1->toggle();
    CHECK(triggerCountC == 7);
    CHECK(triggerCountU == 1);
    CHECK(triggerCountP == 0);

    cb1->toggle();
    CHECK(triggerCountC == 7);
    CHECK(triggerCountU == 1);
    CHECK(triggerCountP == 1);

    cb1->toggle();
    CHECK(triggerCountC == 7);
    CHECK(triggerCountU == 2);
    CHECK(triggerCountP == 1);
}

TEST_CASE("checkbox-color", "") {
    Testhelper t("checkbox", "checkbox-color", 15, 5);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    w->setGeometry({0, 0, 15, 5});

    SECTION("single-black") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-black-focus-checked") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setCheckState(Tui::CheckState::Checked);
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-black-focus-unchecked") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setCheckState(Tui::CheckState::Unchecked);
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-black-focus-partiallychecked") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setCheckState(Tui::CheckState::PartiallyChecked);
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-black-focus-disable") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setFocus();
        cb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-black-markup") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-black-disable-markup") {
        t.root->setPalette(Tui::ZPalette::black());
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        t.compare();
    }

    // cyan
    SECTION("single-cyan") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-cyan-focus-checked") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setFocus();
        cb1->setCheckState(Tui::CheckState::Checked);
        t.compare();
    }

    SECTION("single-cyan-disable") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-cyan-focus-markup") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-cyan-markup-disable") {
        w->setPaletteClass({"window", "cyan"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        t.compare();
    }

    // gray
    SECTION("single-gray") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    SECTION("single-gray-focus-checked") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setCheckState(Tui::CheckState::Checked);
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-gray-disable") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        t.compare();
    }

    SECTION("single-gray-markup") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setFocus();
        t.compare();
    }

    SECTION("single-gray-markup-disable") {
        w->setPaletteClass({"window", "dialog"});
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox(Tui::withMarkup, "<m>c</m>b1", w);
        cb1->setGeometry({1, 1, 13, 1});
        cb1->setEnabled(false);
        t.compare();
    }
}

TEST_CASE("checkbox-margin", "") {
    Testhelper t("checkbox", "checkbox-margin", 15, 7);
    Tui::ZWindow *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 15, 7});

    SECTION("single-margin-2-2-2-2") {
        Tui::ZCheckBox *cb1 = new Tui::ZCheckBox("cb1", w);
        cb1->setGeometry({1, 1, 13, 5});
        CHECK(cb1->sizeHint().height() == 1);
        CHECK(cb1->sizeHint().width() == 8);
        cb1->setContentsMargins({2, 2, 2, 2});
        CHECK(cb1->checkState() == Tui::CheckState::Unchecked);
        CHECK(cb1->sizeHint().height() == 5);
        CHECK(cb1->sizeHint().width() == 12);
        t.compare();
    }
}
