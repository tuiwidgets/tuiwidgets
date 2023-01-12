// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_ZWINDOWFACET_H
#define VCHECK_ZWINDOWFACET_H

#include "vcheck_qobject.h"

VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, isExtendViewport);
VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, isManuallyPlaced);
VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, autoPlace);
VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, container);
VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, setContainer);
VIRTUAL_INDEX_PROBE(ZWindowFacet, Tui::ZWindowFacet, setManuallyPlaced);

static inline void checkZWindowFacetOverrides(Tui::ZWindowFacet *base, Tui::ZWindowFacet *derived) {
    checkQObjectOverrides(base, derived);
    CHECK_VIRTUAL(ZWindowFacet, isExtendViewport);
    CHECK_VIRTUAL(ZWindowFacet, isManuallyPlaced);
    CHECK_VIRTUAL(ZWindowFacet, autoPlace);
    CHECK_VIRTUAL(ZWindowFacet, container);
    CHECK_VIRTUAL(ZWindowFacet, setContainer);
    CHECK_VIRTUAL(ZWindowFacet, setManuallyPlaced);
}

#endif // VCHECK_ZWINDOWFACET_H
