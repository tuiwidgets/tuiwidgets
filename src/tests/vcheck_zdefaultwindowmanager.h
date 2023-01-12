// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZDEFAULTWIDGETMANAGER_H
#define VCHECK_ZDEFAULTWIDGETMANAGER_H

#include "vcheck_qobject.h"

VIRTUAL_INDEX_PROBE(ZDefaultWidgetManager, Tui::ZDefaultWidgetManager, setDefaultWidget)
VIRTUAL_INDEX_PROBE(ZDefaultWidgetManager, Tui::ZDefaultWidgetManager, defaultWidget)
VIRTUAL_INDEX_PROBE(ZDefaultWidgetManager, Tui::ZDefaultWidgetManager, isDefaultWidgetActive)

static inline void checkZDefaultWidgetManagerOverrides(Tui::ZDefaultWidgetManager *base, Tui::ZDefaultWidgetManager *derived) {
    checkQObjectOverrides(base, derived);
    CHECK_VIRTUAL(ZDefaultWidgetManager, setDefaultWidget);
    CHECK_VIRTUAL(ZDefaultWidgetManager, defaultWidget);
    CHECK_VIRTUAL(ZDefaultWidgetManager, isDefaultWidgetActive);
}

#endif // VCHECK_ZDEFAULTWIDGETMANAGER_H
