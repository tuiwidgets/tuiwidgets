// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZWINDOW_H
#define VCHECK_ZWINDOW_H

#include "vcheck_zwidget.h"


VIRTUAL_INDEX_PROBE(ZWindow, Tui::ZWindow, systemMenu)
VIRTUAL_INDEX_PROBE(ZWindow, Tui::ZWindow, closeEvent)

static inline void checkZWindowOverrides(Tui::ZWindow *base, Tui::ZWindow *derived) {
    checkZWidgetOverrides(base, derived);
    CHECK_VIRTUAL(ZWindow, systemMenu);
    CHECK_VIRTUAL(ZWindow, closeEvent);
}

#endif // VCHECK_ZWINDOW_H
