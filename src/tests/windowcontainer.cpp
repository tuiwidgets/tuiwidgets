#include <Tui/ZWindowContainer.h>

#include <QVector>

#include "catchwrapper.h"

TEST_CASE("ZWindowContainer") {
    Tui::ZWindowContainer container;

    SECTION("containerMenuItems") {
        CHECK(container.containerMenuItems().size() == 0);
    }
}
