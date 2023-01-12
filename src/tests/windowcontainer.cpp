// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWindowContainer.h>

#include <QVector>

#include "catchwrapper.h"
#include "vcheck_qobject.h"

TEST_CASE("ZWindowContainer") {
    Tui::ZWindowContainer container;

    SECTION("abi-vcheck") {
        QObject base;
        checkQObjectOverrides(&base, &container);
    }

    SECTION("containerMenuItems") {
        CHECK(container.containerMenuItems().size() == 0);
    }
}
