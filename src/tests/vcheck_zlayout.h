// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZLAYOUT_H
#define VCHECK_ZLAYOUT_H

#include "vcheck_qobject.h"
#include "vcheck_zlayoutitem.h"


VIRTUAL_INDEX_PROBE(ZLayout, Tui::ZLayout, widgetEvent)
VIRTUAL_INDEX_PROBE(ZLayout, Tui::ZLayout, removeWidgetRecursively)

static inline void checkZLayoutOverrides(Tui::ZLayout *base, Tui::ZLayout *derived) {
    checkQObjectOverrides(base, derived);
    checkZLayoutItemOverrides(base, derived);
    CHECK_VIRTUAL(ZLayout, widgetEvent);
    CHECK_VIRTUAL(ZLayout, removeWidgetRecursively);
}

namespace {

class ZLayoutBaseWrapper : public Tui::ZLayout {
public:
    void removeWidgetRecursively(Tui::ZWidget*) override {}
    void setGeometry(QRect) override {}
};

}


#endif // VCHECK_ZLAYOUT_H
