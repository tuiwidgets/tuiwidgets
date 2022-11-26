// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZLayout.h>

#include <QSet>

#include <Tui/ZEvent.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZWidget.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../signalrecorder.h"

class Layout : public Tui::ZLayout {
public:
    QRect r = {-1, -1, -1, -1};
    int geometryCount = 0;

    void setGeometry(QRect r) override {
        geometryCount++;
        this->r = r;
    }

    void relayout() {
        Tui::ZLayout::relayout();
    }
};

TEST_CASE("layout-base") {
    Layout layout;
    CHECK(layout.parentWidget() == nullptr);
    layout.setGeometry({0, 0, 0, 0});
    CHECK(layout.sizeHint() == QSize());
    CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
    CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
    CHECK(layout.isVisible() == true);
    CHECK(layout.widget() == nullptr);
    CHECK(layout.layout() == &layout);

    layout.relayout();
}

TEST_CASE("layout-before-widget") {
    Layout layout;
    Tui::ZWidget w;
    //w.setLayout(&layout); //TODO
}

TEST_CASE("layout-widget") {
    Testhelper t("unsued", "unused", 16, 5);

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    Tui::ZWidget w(parent ? t.root : nullptr);
    Tui::ZWidget w2(nullptr);
    Layout layout;

    SECTION("setLayout") {
        w.setLayout(&layout);
        CHECK(layout.parentWidget() == &w);
    }

    SECTION("setLayout-stacked") {
        w.setLayout(&layout);

        Layout layout2;
        layout2.setParent(&layout);

        CHECK(layout2.parentWidget() == &w);
    }

    SECTION("setLayout-new-layout") {
        w.setLayout(&layout);
        w2.setLayout(&layout);

        CHECK(layout.parentWidget() == &w2);
        CHECK(layout.parentWidget() != &w);
    }
}

TEST_CASE("layout-relayout") {
    Testhelper t("unsued", "unused", 16, 5);
    Tui::ZWidget w(t.root);
    Layout layout;

    SECTION("nothing") {
        t.render();
        CHECK(layout.geometryCount == 0);
    }

    SECTION("setlayout") {
        w.setLayout(&layout);
        t.render();
        CHECK(layout.geometryCount == 1);
        t.render();
        CHECK(layout.geometryCount == 1);
    }

    SECTION("setLayout-and-geometry") {
        w.setLayout(&layout);
        w.setGeometry({5, 5, 5, 5});
        t.render();
        CHECK(layout.geometryCount == 1);
    }
    SECTION("geometry") {
        w.setLayout(&layout);
        t.render();
        CHECK(layout.geometryCount == 1);
        w.setGeometry({5, 5, 5, 5});
        t.render();
        CHECK(layout.geometryCount == 2);
    }

    SECTION("relayout") {
        w.setLayout(&layout);
        w.setGeometry({5, 5, 5, 5});
        t.render();
        CHECK(layout.geometryCount == 1);
        layout.relayout();
        CHECK(layout.geometryCount == 1);
        t.render();
        CHECK(layout.geometryCount == 2);
    }

    SECTION("switch to new terminal") {
        w.setLayout(&layout);
        w.setGeometry({5, 5, 5, 5});
        t.render();
        CHECK(layout.geometryCount == 1);

        auto terminal2 = std::make_unique<Tui::ZTerminal>(Tui::ZTerminal::OffScreen{2, 2});
        terminal2->setMainWidget(t.root);
        CHECK(layout.geometryCount == 1);

        // The t.render(); refers to the old terminal.
        t.render();
        CHECK(layout.geometryCount == 1);

        // The new terminal will be triggered directly.
        terminal2->forceRepaint();
        CHECK(layout.geometryCount == 2);
    }
}
