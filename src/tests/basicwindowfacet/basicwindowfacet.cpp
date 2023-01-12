// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZBasicWindowFacet.h>

#include "../catchwrapper.h"

#include "../Testhelper.h"
#include "../vcheck_zwindowfacet.h"

namespace {
class BaseWrapper : public Tui::ZWindowFacet {
public:
    using Tui::ZWindowFacet::ZWindowFacet;
};

}

TEST_CASE("basicwindowfacet-base") {
    Tui::ZBasicWindowFacet facet;

    CHECK(facet.isManuallyPlaced() == true);
    CHECK(facet.isExtendViewport() == false);

    SECTION("abi-vcheck") {
        BaseWrapper base;
        checkZWindowFacetOverrides(&base, &facet);
    }

    SECTION("extendViewport") {
        facet.setExtendViewport(true);
        CHECK(facet.isExtendViewport() == true);

        facet.setExtendViewport(false);
        CHECK(facet.isExtendViewport() == false);
    }

    SECTION("placement") {
        Tui::ZWidget w;
        const QRect originalGeometry = {5, 7, 11, 13};
        w.setGeometry(originalGeometry);

        facet.autoPlace({100, 30}, &w);

        CHECK(w.geometry() == originalGeometry);

        struct TestCase { std::string name; Tui::Alignment align; int x; int y; };
        auto testCase = GENERATE(
                    TestCase{"top-left", Tui::AlignTop | Tui::AlignLeft, 0, 0},
                    TestCase{"top-center", Tui::AlignTop | Tui::AlignHCenter, 50 - 5, 0},
                    TestCase{"top-right", Tui::AlignTop | Tui::AlignRight, 89, 0},
                    TestCase{"center-left", Tui::AlignVCenter | Tui::AlignLeft, 0, 15 - 6},
                    TestCase{"center", Tui::AlignCenter, 50 - 5, 15 - 6},
                    TestCase{"center-right", Tui::AlignVCenter | Tui::AlignRight, 89, 15 - 6},
                    TestCase{"bottom-left", Tui::AlignBottom | Tui::AlignLeft, 0, 17},
                    TestCase{"bottom-center", Tui::AlignBottom | Tui::AlignHCenter, 50 - 5, 17},
                    TestCase{"bottom-right", Tui::AlignBottom | Tui::AlignRight, 89, 17}
                    );

        CAPTURE(testCase.name);

        facet.setDefaultPlacement(testCase.align, {0, 0});
        CHECK(facet.isManuallyPlaced() == false);

        facet.autoPlace({100, 30}, &w);

        CHECK(w.geometry().size() == originalGeometry.size());
        CHECK(w.geometry().x() == testCase.x);
        CHECK(w.geometry().y() == testCase.y);

        QPoint displacement = {-10, 11};
        facet.setDefaultPlacement(testCase.align, displacement);
        facet.autoPlace({100, 30}, &w);

        CHECK(w.geometry().size() == originalGeometry.size());
        CHECK(w.geometry().x() == std::max(0, testCase.x + displacement.x()));
        CHECK(w.geometry().y() == std::max(0, testCase.y + displacement.y()));

        displacement = {5, -7};
        facet.setDefaultPlacement(testCase.align, displacement);
        facet.autoPlace({100, 30}, &w);

        CHECK(w.geometry().size() == originalGeometry.size());
        CHECK(w.geometry().x() == std::max(0, testCase.x + displacement.x()));
        CHECK(w.geometry().y() == std::max(0, testCase.y + displacement.y()));
    }
}
