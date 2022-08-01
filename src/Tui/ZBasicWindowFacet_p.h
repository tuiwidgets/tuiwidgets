#ifndef TUIWIDGETS_ZBASICWINDOWFACET_P_INCLUDED
#define TUIWIDGETS_ZBASICWINDOWFACET_P_INCLUDED

#include <QPoint>

#include <Tui/ZBasicWindowFacet.h>
#include <Tui/ZWindowFacet_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicWindowFacetPrivate : public ZWindowFacetPrivate {
public:
    ZBasicWindowFacetPrivate();
    ~ZBasicWindowFacetPrivate() override;

public:
    bool extendViewport = false;
    Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignHCenter;
    QPoint displacement = {0, 0};
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICWINDOWFACET_P_INCLUDED
