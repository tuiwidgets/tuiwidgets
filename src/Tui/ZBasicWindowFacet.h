#ifndef TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED
#define TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED

#include <QObject>

#include <Tui/ZWindowFacet.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicWindowFacetPrivate;
class TUIWIDGETS_EXPORT ZBasicWindowFacet : public ZWindowFacet {
    Q_OBJECT
public:
    ZBasicWindowFacet();
    ~ZBasicWindowFacet() override;

public:
    bool isExtendViewport() const override;
    bool isManuallyPlaced() const override;
    void autoPlace(const QSize &available, ZWidget *self) override;

    void setDefaultPlacement(Qt::Alignment align, QPoint displace);
    void setExtendViewport(bool extend);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    ZWindowContainer *container() const override;
    void setContainer(ZWindowContainer *container) override;
    void setManuallyPlaced(bool manual) override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZBasicWindowFacet)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICWINDOWFACET_INCLUDED
