// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDefaultWidgetManager.h>

#include "catchwrapper.h"

#include "vcheck_zdefaultwindowmanager.h"

namespace {

class TestClass : public Tui::ZDefaultWidgetManager {
public:
    void setDefaultWidget(Tui::ZWidget*) override {};
    Tui::ZWidget *defaultWidget() const override { return nullptr; }
    bool isDefaultWidgetActive() const override { return false; }
};

}

TEST_CASE("defaultwidgetmananger", "") {
    Tui::ZWidget w;

    TestClass manager;

    SECTION("abi-vcheck") {
        QObject base;
        checkQObjectOverrides(&base, &manager);
    }
}
