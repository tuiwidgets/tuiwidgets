#include <Tui/ZTextLine.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"

#include <Tui/ZPalette.h>

TEST_CASE("textline-base", "") {

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("no-parent") {
        // The initialisation must not crash.
        delete new Tui::ZTextLine();
        delete new Tui::ZTextLine("m");
        delete new Tui::ZTextLine(Tui::withMarkup, "<m>m</m>");
    }

    SECTION("initialisation") {
        // The initialisation must not crash.
        delete new Tui::ZTextLine(w.get());
        delete new Tui::ZTextLine("m", w.get());
        delete new Tui::ZTextLine(Tui::withMarkup, "<m>m</m>", w.get());
    }

    auto checkDefaultState = [] (Tui::ZTextLine *t) {
        CHECK(t->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(t->sizePolicyV() == Tui::SizePolicy::Fixed);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(t, DefaultException::SizePolicyV
                                                   | DefaultException::SizePolicyH));
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZTextLine> t = std::make_unique<Tui::ZTextLine>(w.get());
        CHECK(t->text() == "");
        CHECK(t->markup() == "");
        checkDefaultState(t.get());
    }

    SECTION("constructor-with-text") {
        std::unique_ptr<Tui::ZTextLine> t = std::make_unique<Tui::ZTextLine>("m", w.get());
        CHECK(t->text() == "m");
        CHECK(t->markup() == "");
        checkDefaultState(t.get());
    }

    SECTION("constructor-with-markup") {
        std::unique_ptr<Tui::ZTextLine> t = std::make_unique<Tui::ZTextLine>(Tui::withMarkup, "<m>m</m>", w.get());
        CHECK(t->text() == "");
        CHECK(t->markup() == "<m>m</m>");
        checkDefaultState(t.get());
    }

    std::unique_ptr<Tui::ZTextLine> t = std::make_unique<Tui::ZTextLine>(w.get());

    SECTION("get-set-text") {
        t->setText("m");
        CHECK(t->text() == "m");
        CHECK(t->markup() == "");
    }

    SECTION("get-set-markup") {
        t->setMarkup("<m>m</m>");
        CHECK(t->markup() == "<m>m</m>");
        CHECK(t->text() == "");
    }
}

TEST_CASE("textline-visual", "") {

    Testhelper t("textline", "textline-visual", 9, 4);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 9, 4});
    w->setFocus();

    SECTION("ctor-clip") {
        Tui::ZTextLine *t1 = new Tui::ZTextLine("Hallo Welt", w);
        t1->setGeometry({1, 1, 7, 1});
        t.compare();
    }

    SECTION("ctor-clip-markup") {
        Tui::ZTextLine *t1 = new Tui::ZTextLine(Tui::withMarkup, "<m>H</m>allo Welt", w);
        t1->setGeometry({1, 1, 7, 1});
        t.compare("ctor-clip");
    }

    Tui::ZTextLine *t1 = new Tui::ZTextLine(w);
    SECTION("no-text-geometry0x0") {
        t.render();
    }

    t1->setGeometry({1, 1, 7, 1});
    SECTION("no-text-geometry") {
        t.compare();
    }

    SECTION("text-6-chars") {
        t1->setText("Hallo1");
        t.compare();
    }
    SECTION("text-7-chars") {
        t1->setText("Hallo12");
        t.compare();
    }
    SECTION("text-8-chars") {
        t1->setText("Hallo123");
        t.compare("text-7-chars");
    }

    SECTION("text-focus") {
        t1->setText("Hallo12");
        t1->setFocus();
        t.compare("text-7-chars");
    }
    SECTION("text-disable") {
        t1->setText("Hallo12");
        t1->setEnabled(false);
        t.compare("text-7-chars");
    }

    SECTION("text-2-utf8") {
        t1->setGeometry({1, 1, 3, 1});
        t1->setText("😇😇");
        t.compare();
    }

    SECTION("markup") {
        t1->setMarkup("<m>H</m>allo Welt");
        t.compare("ctor-clip");
    }

    SECTION("markup-focus") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setFocus();
        t.compare("ctor-clip");
    }

    SECTION("markup-disable") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setEnabled(false);
        t.compare("ctor-clip");
    }

    SECTION("markup-2-utf8") {
        t1->setGeometry({1, 1, 3, 1});
        t1->setFocus();
        t1->setMarkup("<m>😇</m>😇");
        t.compare("text-2-utf8");
    }

    SECTION("sendchar") {
        t1->setMarkup("<m>T</m>1");
        t.sendChar("t", Qt::AltModifier);
    }
}

TEST_CASE("textline-margin", "") {
    Testhelper t("textline", "textline-margin", 14, 7);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 14, 7});
    w->setFocus();
    Tui::ZTextLine *t1 = new Tui::ZTextLine(w);
    t1->setGeometry({1, 1, 12, 5});

    SECTION("2-2-2-2") {
        t1->setText("ABC");
        CHECK(t1->sizeHint().height() == 1);
        CHECK(t1->sizeHint().width() == 0 + t1->text().size());
        t1->setContentsMargins({2, 2, 2, 2});
        CHECK(t1->sizeHint().height() == 5);
        CHECK(t1->sizeHint().width() == 4 + t1->text().size());
        t.compare();
    }
    SECTION("2-2-2-2-focus") {
        t1->setText("ABC");
        t1->setFocus();
        t1->setContentsMargins({2, 2, 2, 2});
        CHECK(t1->sizeHint().height() == 5);
        CHECK(t1->sizeHint().width() == 4 + t1->text().size());
        t.compare("2-2-2-2");
    }
}

TEST_CASE("textline-palette", "") {
    Testhelper t("textline", "textline-palette", 10, 3);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 10, 3});
    w->setFocus();
    Tui::ZTextLine *t1 = new Tui::ZTextLine(w);
    t1->setGeometry({1, 1, 8, 1});


    // black
    t.root->setPalette(Tui::ZPalette::black());
    SECTION("black-text") {
        t1->setText("Hallo Welt");
        t.compare();
    }

    SECTION("black-focus") {
        t1->setText("Hallo Welt");
        t1->setFocus();
        t.compare("black-text");
    }

    SECTION("black-disable") {
        t1->setText("Hallo Welt");
        t1->setEnabled(false);
        t.compare("black-text");
    }

    SECTION("black-markup") {
        t1->setMarkup("<m>H</m>allo Welt");
        t.compare("black-text");
    }

    SECTION("black-markup-focus") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setFocus();
        t.compare("black-text");
    }

    SECTION("black-markup-disable") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setEnabled(false);
        t.compare("black-text");
    }

    SECTION("black-markup-disable-focus") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setFocus();
        t1->setEnabled(false);
        t.compare("black-text");
    }

    // cyan
    t.root->setPalette(Tui::ZPalette::classic());
    w->setPaletteClass({"window", "cyan"});

    SECTION("cyan-text") {
        t1->setText("Hallo Welt");
        t.compare();
    }

    SECTION("cyan-focus") {
        t1->setText("Hallo Welt");
        t1->setFocus();
        t.compare("cyan-text");
    }

    SECTION("cyan-disable") {
        t1->setText("Hallo Welt");
        t1->setEnabled(false);
        t.compare("cyan-text");
    }

    SECTION("cyan-markup") {
        t1->setMarkup("<m>H</m>allo Welt");
        t.compare("cyan-text");
    }

    SECTION("cyan-markup-focus") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setFocus();
        t.compare("cyan-text");
    }

    SECTION("cyan-markup-disable") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setEnabled(false);
        t.compare("cyan-text");
    }

    SECTION("cyan-markup-disable-focus") {
        t1->setMarkup("<m>H</m>allo Welt");
        t1->setFocus();
        t1->setEnabled(false);
        t.compare("cyan-text");
    }
}
