// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZClipboard.h>

#include "catchwrapper.h"


#include "vcheck_qobject.h"


TEST_CASE("ZClipboard") {

    Tui::ZClipboard clipboard;

    SECTION("abi-vcheck") {
        QObject base;
        checkQObjectOverrides(&base, &clipboard);
    }

    SECTION("content") {
        CHECK(clipboard.contents() == QString(""));
        clipboard.setContents("test");
        CHECK(clipboard.contents() == QString("test"));
        clipboard.clear();
        CHECK(clipboard.contents() == QString(""));
    }
}
