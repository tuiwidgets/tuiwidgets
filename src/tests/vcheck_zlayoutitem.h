// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZLAYOUTITEM_H
#define VCHECK_ZLAYOUTITEM_H

#include "catchwrapper.h"

#include "vcheck_base.h"

VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, setGeometry)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, sizeHint)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, sizePolicyH)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, sizePolicyV)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, isVisible)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, isSpacer)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, widget)
VIRTUAL_INDEX_PROBE(ZLayoutItem, Tui::ZLayoutItem, layout)

static inline void checkZLayoutItemOverrides(Tui::ZLayoutItem *base, Tui::ZLayoutItem *derived) {
    CHECK_VIRTUAL(ZLayoutItem, setGeometry);
    CHECK_VIRTUAL(ZLayoutItem, sizeHint);
    CHECK_VIRTUAL(ZLayoutItem, sizePolicyH);
    CHECK_VIRTUAL(ZLayoutItem, sizePolicyV);
    CHECK_VIRTUAL(ZLayoutItem, isVisible);
    CHECK_VIRTUAL(ZLayoutItem, isSpacer);
    CHECK_VIRTUAL(ZLayoutItem, widget);
    CHECK_VIRTUAL(ZLayoutItem, layout);
}

#endif // VCHECK_ZLAYOUTITEM_H
