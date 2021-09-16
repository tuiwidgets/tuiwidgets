#include <Tui/ZLabel.h>

#include <../../third-party/catch.hpp>
#include "../Testhelper.h"

#include <Tui/ZPalette.h>

TEST_CASE("label-base", "") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("no-parent") {
        // The initialisation must not crash.
        delete new Tui::ZLabel();
        delete new Tui::ZLabel("m");
        delete new Tui::ZLabel(Tui::withMarkup, "<m>m</m>");
    }

    SECTION("initialisation") {
        // The initialisation must not crash.
        delete new Tui::ZLabel(w.get());
        delete new Tui::ZLabel("m", w.get());
        delete new Tui::ZLabel(Tui::withMarkup, "<m>m</m>", w.get());
    }

    auto checkDefaultState = [] (Tui::ZLabel *l) {
        CHECK(l->buddy() == nullptr);
        CHECK(l->sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(l->sizePolicyV() == Tui::SizePolicy::Fixed);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(l, DefaultException::SizePolicyV));
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZLabel> l = std::make_unique<Tui::ZLabel>(w.get());
        CHECK(l->text() == "");
        CHECK(l->markup() == "");
        checkDefaultState(l.get());
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZLabel> l = std::make_unique<Tui::ZLabel>("m", w.get());
        CHECK(l->text() == "m");
        CHECK(l->markup() == "");
        checkDefaultState(l.get());
    }

    SECTION("constructor-with-markup") {
        std::unique_ptr<Tui::ZLabel> l = std::make_unique<Tui::ZLabel>(Tui::withMarkup, "<m>m</m>", w.get());
        CHECK(l->text() == "");
        CHECK(l->markup() == "<m>m</m>");
        checkDefaultState(l.get());
    }

    std::unique_ptr<Tui::ZLabel> l = std::make_unique<Tui::ZLabel>(w.get());

    SECTION("get-set-text") {
        l->setText("m");
        CHECK(l->text() == "m");
        CHECK(l->markup() == "");
    }

    SECTION("get-set-markup") {
        l->setMarkup("<m>m</m>");
        CHECK(l->markup() == "<m>m</m>");
        CHECK(l->text() == "");
    }

    SECTION("get-set-buddy") {
        Tui::ZWidget buddy;
        l->setBuddy(&buddy);
        CHECK(l->buddy() == &buddy);
    }

    SECTION("buddy-not-dangling") {
        {
            Tui::ZWidget buddy;
            l->setBuddy(&buddy);
        }
        CHECK(l->buddy() == nullptr);
    }
}

TEST_CASE("label-visual", "") {

    Testhelper t("label", "label-visual", 10, 4);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 10, 4});
    w->setFocus();

    SECTION("ctor-clip") {
        Tui::ZLabel *l1 = new Tui::ZLabel("Hallo Welt", w);
        l1->setGeometry({1, 1, 8, 1});
        t.compare();
    }

    SECTION("ctor-clip-markup") {
        Tui::ZLabel *l1 = new Tui::ZLabel(Tui::withMarkup, "<m>H</m>allo Welt", w);
        l1->setGeometry({1, 1, 8, 1});
        t.compare("markup");
    }

    Tui::ZLabel *l = new Tui::ZLabel(w);
    SECTION("no-text-geometry0x0") {
        t.render();
    }

    l->setGeometry({1, 1, 8, 1});
    SECTION("no-text-geometry") {
        t.compare();
    }

    SECTION("text-6-chars") {
        l->setText("Hallo1");
        t.compare();
    }
    SECTION("text-7-chars") {
        l->setText("Hallo12");
        t.compare();
    }
    SECTION("text-8-chars") {
        l->setText("Hallo123");
        t.compare("text-7-chars");
    }

    SECTION("text-focus") {
        l->setText("Hallo12");
        l->setFocus();
        t.compare("text-7-chars");
    }
    SECTION("text-disable") {
        l->setText("Hallo12");
        l->setEnabled(false);
        t.compare();
    }

    SECTION("text-2-utf8") {
        l->setGeometry({1, 1, 4, 1});
        l->setText("😇😇");
        t.compare();
    }

    SECTION("markup") {
        l->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }

    SECTION("markup-focus") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setFocus();
        t.compare("markup");
    }

    SECTION("markup-focus-buddy") {
        StubWidget *sw1 = new StubWidget(w);
        sw1->setFocus();
        l->setMarkup("<m>H</m>allo Welt");
        l->setBuddy(sw1);
        t.compare();
    }

    SECTION("markup-disable") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setEnabled(false);
        t.compare();
    }

    SECTION("markup-2-utf8") {
        l->setGeometry({1, 1, 4, 1});
        l->setFocus();
        l->setMarkup("<m>😇</m>😇");
        t.compare();
    }

    SECTION("buddy") {
        StubWidget *sw1 = new StubWidget(w);
        StubWidget *sw2 = new StubWidget(w);

        l->setMarkup("<m>L</m>1");
        l->setGeometry({1, 1, 3, 1});
        l->setBuddy(sw1);

        Tui::ZLabel *l2 = new Tui::ZLabel(w);
        l2->setMarkup("L<m>2</m>");
        l2->setGeometry({1, 2, 3, 1});
        l2->setBuddy(sw2);

        t.sendChar("l", Qt::AltModifier);
        CHECK(sw1->focus());
        CHECK(sw2->focus() == false);
        t.sendChar("2", Qt::AltModifier);
        CHECK(sw1->focus() == false);
        CHECK(sw2->focus());
        t.compare();
    }

    SECTION("buddy-disable-label") {
        StubWidget *sw1 = new StubWidget(w);

        l->setMarkup("<m>L</m>1");
        l->setGeometry({1, 1, 3, 1});
        l->setBuddy(sw1);
        l->setEnabled(false);
        t.sendChar("l", Qt::AltModifier);
        CHECK(sw1->focus() == false);
        t.compare("buddy-disable-label");
    }

    SECTION("buddy-disable-buddy") {
        StubWidget *sw1 = new StubWidget(w);
        sw1->setEnabled(false);

        l->setMarkup("<m>L</m>1");
        l->setGeometry({1, 1, 3, 1});
        l->setBuddy(sw1);
        t.sendChar("l", Qt::AltModifier);
        CHECK(sw1->focus() == false);
        t.compare("buddy-disable-label");
    }

    SECTION("sendchar-without-buddy") {
        l->setMarkup("<m>L</m>1");
        t.sendChar("l", Qt::AltModifier);
    }

    SECTION("buddy-reset-text") {
        StubWidget *sw1 = new StubWidget(w);

        l->setMarkup("<m>L</m>1");
        l->setText("L1");
        l->setGeometry({1, 1, 3, 1});
        l->setBuddy(sw1);
        t.sendChar("l", Qt::AltModifier);
        CHECK(sw1->focus() == false);
    }

    SECTION("buddy-reset-markup") {
        StubWidget *sw1 = new StubWidget(w);

        l->setMarkup("<m>L</m>1");
        l->setMarkup("L1");
        l->setGeometry({1, 1, 3, 1});
        l->setBuddy(sw1);
        t.sendChar("l", Qt::AltModifier);
        CHECK(sw1->focus() == false);
    }
}

TEST_CASE("label-margin", "") {
    Testhelper t("label", "label-margin", 14, 7);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 14, 7});
    w->setFocus();
    Tui::ZLabel *l = new Tui::ZLabel(w);
    l->setGeometry({1, 1, 12, 5});

    SECTION("2-2-2-2") {
        l->setText("ABC");
        CHECK(l->sizeHint().height() == 1);
        CHECK(l->sizeHint().width() == 1 + l->text().size());
        l->setContentsMargins({2, 2, 2, 2});
        CHECK(l->sizeHint().height() == 5);
        CHECK(l->sizeHint().width() == 5 + l->text().size());
        t.compare();
    }
    SECTION("2-2-2-2-focus") {
        l->setText("ABC");
        l->setFocus();
        l->setContentsMargins({2, 2, 2, 2});
        CHECK(l->sizeHint().height() == 5);
        CHECK(l->sizeHint().width() == 5 + l->text().size());
        t.compare("2-2-2-2");
    }
}

TEST_CASE("label-palette", "") {
    Testhelper t("label", "label", 15, 3);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 10, 3});
    w->setFocus();
    Tui::ZLabel *l = new Tui::ZLabel(w);
    l->setGeometry({1, 1, 8, 1});


    // black
    t.root->setPalette(Tui::ZPalette::black());
    SECTION("black-text") {
        l->setText("Hallo Welt");
        t.compare();
    }

    SECTION("black-focus") {
        l->setText("Hallo Welt");
        l->setFocus();
        t.compare("black-text");
    }

    SECTION("black-disable") {
        l->setText("Hallo Welt");
        l->setEnabled(false);
        t.compare();
    }

    SECTION("black-markup") {
        l->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }

    SECTION("black-markup-focus") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setFocus();
        t.compare("black-markup");
    }

    SECTION("black-markup-focus-buddy") {
        StubWidget *sw1 = new StubWidget(w);
        sw1->setFocus();
        l->setMarkup("<m>H</m>allo Welt");
        l->setBuddy(sw1);
        t.compare();
    }

    SECTION("black-markup-disable") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setEnabled(false);
        t.compare("black-disable");
    }

    SECTION("black-markup-disable-focus") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setFocus();
        l->setEnabled(false);
        t.compare("black-disable");
    }

    // cyan
    t.root->setPalette(Tui::ZPalette::classic());
    w->setPaletteClass({"window", "cyan"});

    SECTION("cyan-text") {
        l->setText("Hallo Welt");
        t.compare();
    }

    SECTION("cyan-focus") {
        l->setText("Hallo Welt");
        l->setFocus();
        t.compare("cyan-text");
    }

    SECTION("cyan-disable") {
        l->setText("Hallo Welt");
        l->setEnabled(false);
        t.compare();
    }

    SECTION("cyan-markup") {
        l->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }

    SECTION("cyan-markup-focus") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setFocus();
        t.compare("cyan-markup");
    }

    SECTION("cyan-markup-focus-buddy") {
        StubWidget *sw1 = new StubWidget(w);
        sw1->setFocus();
        l->setMarkup("<m>H</m>allo Welt");
        l->setBuddy(sw1);
        t.compare();
    }

    SECTION("cyan-markup-disable") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setEnabled(false);
        t.compare("cyan-disable");
    }

    SECTION("cyan-markup-disable-focus") {
        l->setMarkup("<m>H</m>allo Welt");
        l->setFocus();
        l->setEnabled(false);
        t.compare("cyan-disable");
    }
}
