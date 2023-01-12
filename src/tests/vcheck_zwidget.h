// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZWIDGET_H
#define VCHECK_ZWIDGET_H

#include "vcheck_qobject.h"

VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, sizeHint);
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, minimumSizeHint);
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, layoutArea);
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, resolveSizeHintChain);
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, facet);
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, paintEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, keyEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, pasteEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, focusInEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, focusOutEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, resizeEvent)
VIRTUAL_INDEX_PROBE(ZWidget, Tui::ZWidget, moveEvent)

static inline void checkZWidgetOverrides(Tui::ZWidget *base, Tui::ZWidget *derived) {
    checkQObjectOverrides(base, derived);
    CHECK_VIRTUAL(ZWidget, sizeHint);
    CHECK_VIRTUAL(ZWidget, minimumSizeHint);
    CHECK_VIRTUAL(ZWidget, layoutArea);
    CHECK_VIRTUAL(ZWidget, resolveSizeHintChain);
    CHECK_VIRTUAL(ZWidget, facet);
    CHECK_VIRTUAL(ZWidget, paintEvent);
    CHECK_VIRTUAL(ZWidget, keyEvent);
    CHECK_VIRTUAL(ZWidget, pasteEvent);
    CHECK_VIRTUAL(ZWidget, focusInEvent);
    CHECK_VIRTUAL(ZWidget, focusOutEvent);
    CHECK_VIRTUAL(ZWidget, resizeEvent);
    CHECK_VIRTUAL(ZWidget, moveEvent);
}

#endif // VCHECK_ZWIDGET_H
