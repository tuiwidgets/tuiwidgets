#include <Tui/ZBasicWindowFacet.h>

#include <../../third-party/catch.hpp>


#include "../Testhelper.h"

TEST_CASE("basicwindowfacet-base") {
    Tui::ZBasicWindowFacet facet;

    CHECK(facet.isManuallyPlaced() == true);

    Tui::ZWidget w;
    const QRect originalGeometry = {5, 7, 11, 13};
    w.setGeometry(originalGeometry);

    facet.autoPlace({100, 30}, &w);

    CHECK(w.geometry() == originalGeometry);

    struct TestCase { std::string name; Qt::Alignment align; int x; int y; };
    auto testCase = GENERATE(
                TestCase{"top-left", Qt::AlignTop | Qt::AlignLeft, 0, 0},
                TestCase{"top-center", Qt::AlignTop | Qt::AlignHCenter, 50 - 5, 0},
                TestCase{"top-right", Qt::AlignTop | Qt::AlignRight, 89, 0},
                TestCase{"center-left", Qt::AlignVCenter | Qt::AlignLeft, 0, 15 - 6},
                TestCase{"center", Qt::AlignCenter, 50 - 5, 15 - 6},
                TestCase{"center-right", Qt::AlignVCenter | Qt::AlignRight, 89, 15 - 6},
                TestCase{"bottom-left", Qt::AlignBottom | Qt::AlignLeft, 0, 17},
                TestCase{"bottom-center", Qt::AlignBottom | Qt::AlignHCenter, 50 - 5, 17},
                TestCase{"bottom-right", Qt::AlignBottom | Qt::AlignRight, 89, 17}
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
