// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOWFACET_P_INCLUDED
#define TUIWIDGETS_ZWINDOWFACET_P_INCLUDED

#include <Tui/ZWindowFacet.h>

#include <QPointer>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowFacetPrivate {
public:
    virtual ~ZWindowFacetPrivate();

public:
    bool manuallyPlaced = true;
    QPointer<ZWindowContainer> container;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWFACET_P_INCLUDED
