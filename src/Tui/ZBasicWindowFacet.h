#ifndef TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED
#define TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED

#include <QObject>

#include <Tui/ZWindowFacet.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicWindowFacetPrivate;
class TUIWIDGETS_EXPORT ZBasicWindowFacet : public ZWindowFacet {
public:
    ZBasicWindowFacet();
    ~ZBasicWindowFacet();

public:
    bool isExtendViewport() const override;
    bool isManuallyPlaced() override;
    void autoPlace(const QSize &available, ZWidget *self) override;

    void setDefaultPlacement(Qt::Alignment align, QPoint displace);
    void setExtendViewport(bool extend);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZBasicWindowFacet)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED
