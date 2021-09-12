#include <../../third-party/catch.hpp>

#include <Tui/ZStyledTextLine.h>

#include "../Testhelper.h"

#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZWindow.h>

class TestWidget : public Tui::ZWidget {
    public:
        using Tui::ZWidget::ZWidget;
        Tui::ZStyledTextLine *stl = nullptr;
        int w = 10;
        int x = 0;
        int y = 0;

    public:
        void paintEvent(Tui::ZPaintEvent *event) override {
            Tui::ZPainter *painter = event->painter();
            if (stl) {
                stl->write(painter, x, y, w);
            }
        }
};


TEST_CASE("styledtextline-base", "") {
    Testhelper t("styledtextline", "styledtextline-base", 9, 2);
    std::unique_ptr<TestWidget> w = std::make_unique<TestWidget>(t.root);
    std::unique_ptr<Tui::ZStyledTextLine> stl = std::make_unique<Tui::ZStyledTextLine>();
    w->stl = stl.get();

    SECTION("defaults") {
        CHECK(stl->text() == "");
        CHECK(stl->markup() == "");
        CHECK(stl->mnemonic() == "");
    }

    SECTION("get-set-text") {
        stl->setText("m");
        CHECK(stl->text() == "m");
        CHECK(stl->markup() == "");
        CHECK(stl->mnemonic() == "");
    }

    SECTION("get-set-markup") {
        stl->setMarkup("<m>m</m>");
        CHECK(stl->markup() == "<m>m</m>");
        CHECK(stl->text() == "");
    }

    SECTION("setBaseStyle") {
        Tui::ZTextStyle ts({0, 0, 0},{0xFF, 0xFF, 0xFF});
        stl->setBaseStyle(ts);
    }

    SECTION("setMnemonicStyle") {
        Tui::ZTextStyle ts({0, 0, 0},{0xFF, 0xFF, 0xFF});
        stl->setMnemonicStyle(ts, ts);
    }

    SECTION("mnemonic") {
        CHECK(stl->mnemonic() == "");
        stl->setMarkup("<m>H</m>allo Welt");
        CHECK(stl->mnemonic() == "H");
    }

    SECTION("width-setText") {
        CHECK(stl->width(t.terminal->textMetrics()) == 0);
        stl->setText("Hallo");
        CHECK(stl->width(t.terminal->textMetrics()) == 5);
        stl->setText("😇😇");
        CHECK(stl->width(t.terminal->textMetrics()) == 4);
        stl->setText("M😇H");
        CHECK(stl->width(t.terminal->textMetrics()) == 4);
        stl->setText("aあ");
        CHECK(stl->width(t.terminal->textMetrics()) == 3);
        // a, COMBINING GRAVE ACCENT BELOW, COMBINING TILDE
        stl->setText("a\u0316\u0303");
        CHECK(stl->width(t.terminal->textMetrics()) == 1);
        stl->setText("&amp;");
        CHECK(stl->width(t.terminal->textMetrics()) == 5);
        stl->setText("A<b>C</b>D");
        CHECK(stl->width(t.terminal->textMetrics()) == 10);
    }
    SECTION("width-setMarkup") {
        CHECK(stl->width(t.terminal->textMetrics()) == 0);
        stl->setMarkup("Hallo");
        CHECK(stl->width(t.terminal->textMetrics()) == 5);
        stl->setMarkup("😇😇");
        CHECK(stl->width(t.terminal->textMetrics()) == 4);
        stl->setMarkup("M😇H");
        CHECK(stl->width(t.terminal->textMetrics()) == 4);
        stl->setMarkup("aあ");
        CHECK(stl->width(t.terminal->textMetrics()) == 3);
        // a, COMBINING GRAVE ACCENT BELOW, COMBINING TILDE
        stl->setMarkup("a\u0316\u0303");
        CHECK(stl->width(t.terminal->textMetrics()) == 1);
        stl->setMarkup("&amp;");
        CHECK(stl->width(t.terminal->textMetrics()) == 1);
        stl->setMarkup("A<b>C</b>D");
        CHECK(stl->width(t.terminal->textMetrics()) == 3);
    }

    SECTION("write") {
        t.render();
    }
}


TEST_CASE("styledtextline-visual", "") {
    Testhelper t("styledtextline", "styledtextline-visual", 9, 2);
    TestBackground *background = new TestBackground(t.root);
    background->setGeometry({0, 0, 9, 2});
    background->setBorderEdges({});
    std::unique_ptr<TestWidget> w = std::make_unique<TestWidget>(background);
    std::unique_ptr<Tui::ZStyledTextLine> stl = std::make_unique<Tui::ZStyledTextLine>();
    w->stl = stl.get();
    w->setGeometry({0, 0, 9, 2});
    w->setFocus();

    SECTION("text-6-chars") {
        stl->setText("Hallo1");
        t.compare();
    }
    SECTION("text-5-chars") {
        w->w = 5;
        stl->setText("Hallo1");
        t.compare();
    }
    SECTION("text-5-chars-2-1") {
        w->x = 2;
        w->y = 1;
        w->w = 5;
        stl->setText("Hallo1");
        t.compare();
    }
    SECTION("text-2-utf8") {
        stl->setText("😇😇");
        t.compare();
    }
    SECTION("text-1-utf8") {
        w->w = 3;
        stl->setText("😇😇");
        t.compare();
    }
    SECTION("markup") {
        stl->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }
    SECTION("markup-ha") {
        w->w = 2;
        stl->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }
    SECTION("markup-ha-2-1") {
        w->x = 2;
        w->y = 1;
        w->w = 2;
        stl->setMarkup("<m>H</m>allo Welt");
        t.compare();
    }
    SECTION("markup-as-text") {
        stl->setText("<m>H</m>allo Welt");
        t.compare();
    }
    SECTION("markup-2-utf8") {
        stl->setMarkup("<m>😇</m>😇");
        t.compare("text-2-utf8");
    }

    SECTION("BaseStyle") {
        Tui::ZTextStyle ts({0xAA, 0xAA, 0xAA},{0xFF, 0xFF, 0xFF});
        stl->setBaseStyle(ts);
        SECTION("text-6-chars") {
            stl->setText("Hallo1");
            t.compare();
        }
        SECTION("text-2-utf8") {
            stl->setText("😇😇");
            t.compare("BaseStyle-text-2-utf8");
        }
        SECTION("markup") {
            stl->setMarkup("<m>H</m>allo Welt");
            t.compare();
        }
        SECTION("markup-2-utf8") {
            stl->setMarkup("<m>😇</m>😇");
            t.compare("BaseStyle-text-2-utf8");
        }
    }

    SECTION("MnemonicStyle") {
        Tui::ZTextStyle tsTxt({0xAA, 0xAA, 0xAA},{0xFF, 0xFF, 0xFF});
        Tui::ZTextStyle tsMnem({0x00, 0x00, 0x00},{0xEE, 0xEE, 0xEE});
        tsMnem.setAttributes(Tui::ZPainter::Attribute::Strike);
        stl->setMnemonicStyle(tsTxt, tsMnem);
        SECTION("text-6-chars") {
            stl->setText("Hallo1");
            t.compare("BaseStyle-text-6-chars");
        }
        SECTION("text-2-utf8") {
            stl->setText("😇😇");
            t.compare("BaseStyle-text-2-utf8");
        }
        SECTION("markup") {
            stl->setMarkup("<m>H</m>allo Welt");
            CHECK(stl->mnemonic() == "H");
            t.compare();
        }
        SECTION("markup-2-utf8") {
            stl->setMarkup("<m>😇</m>😇");
            t.compare();
        }
    }

    SECTION("text-html") {
        stl->setMarkup("&amp;");
        t.compare();
        stl->setMarkup("<body>&amp;</body>");
        t.compare();
    }


    SECTION("write-3-1") {
        w->x = 3;
        w->y = 1;
        stl->setText("456");
        t.compare();
    }
    SECTION("write-10-10") {
        w->x = -10;
        w->y = -10;
        stl->setText("456");
        t.compare();
    }
    SECTION("write-6-1") {
        w->x = 6;
        w->y = 1;
        stl->setText("7890");
        t.compare();
    }
    SECTION("write-6-0-utf") {
        w->x = 6;
        w->y = 0;
        stl->setText("78😇");
        t.compare();
    }
    SECTION("rewrite-AB") {
        w->y = 0;
        w->x = 2;
        stl->setText("A");
        w->update();
        w->y = 1;
        w->x = 3;
        stl->setText("B");
        t.compare();
    }

    SECTION("write-setMarkup-3-1") {
        w->x = 3;
        w->y = 1;
        stl->setMarkup("456");
        t.compare("write-3-1");
    }
    SECTION("write-setMarkup-10-10") {
        w->x = -10;
        w->y = -10;
        stl->setMarkup("456");
        t.compare("write-10-10");
    }
    SECTION("write-setMarkup-6-1") {
        w->x = 6;
        w->y = 1;
        stl->setMarkup("7890");
        t.compare("write-6-1");
    }
    SECTION("write-setMarkup-6-0-utf") {
        w->x = 6;
        w->y = 0;
        stl->setMarkup("78😇");
        t.compare("write-6-0-utf");
    }
    SECTION("rewrite-setMarkup-AB") {
        w->y = 0;
        w->x = 2;
        stl->setMarkup("A");
        w->update();
        w->y = 1;
        w->x = 3;
        stl->setMarkup("B");
        t.compare("rewrite-AB");
    }
}