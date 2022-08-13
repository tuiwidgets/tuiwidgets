// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWindowFacet.h>

#include <QRect>

#include <Tui/ZWidget.h>

#include "catchwrapper.h"

TEST_CASE("ZWindowFacet") {
    class TestClass : public Tui::ZWindowFacet {
    public:
        using Tui::ZWindowFacet::ZWindowFacet;
    };

    TestClass f;

    SECTION("defaults-and-fixed") {
        CHECK(f.isExtendViewport() == false);
        CHECK(f.isManuallyPlaced() == true);
        CHECK(f.container() == nullptr);
    }

    SECTION("setManuallyPlaced") {
        f.setManuallyPlaced(false);
        CHECK(f.isManuallyPlaced() == false);
        f.setManuallyPlaced(true);
        CHECK(f.isManuallyPlaced() == true);
    }

    SECTION("autoPlace") {
        f.setManuallyPlaced(false);
        Tui::ZWidget w;
        w.setGeometry({200, 200, 10, 20});
        f.autoPlace({20, 30}, &w);
        CHECK(w.geometry() == QRect(6, 6, 10, 20));
        f.setManuallyPlaced(true);

        f.setManuallyPlaced(true);
        w.setGeometry({200, 200, 10, 20});
        f.autoPlace({20, 30}, &w);
        CHECK(w.geometry() == QRect(200, 200, 10, 20));
        f.setManuallyPlaced(true);
    }

    SECTION("setContainer") {
        Tui::ZWindowContainer container;
        f.setContainer(&container);
        CHECK(f.container() == &container);
        f.setContainer(nullptr);
        CHECK(f.container() == nullptr);
    }

}
