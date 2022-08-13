// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOWFACET_INCLUDED
#define TUIWIDGETS_ZWINDOWFACET_INCLUDED

#include <QObject>

#include <Tui/ZWidget.h>
#include <Tui/ZWindowContainer.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowFacetPrivate;
class TUIWIDGETS_EXPORT ZWindowFacet : public QObject {
    Q_OBJECT
public:
    virtual bool isExtendViewport() const;
    virtual bool isManuallyPlaced() const;
    virtual void autoPlace(const QSize &available, ZWidget *self);
    virtual ZWindowContainer *container() const;
    virtual void setContainer(ZWindowContainer *container);

    virtual void setManuallyPlaced(bool manual);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

protected:
    ZWindowFacet();
    ZWindowFacet(std::unique_ptr<ZWindowFacetPrivate> pimpl);
    ~ZWindowFacet() override;

    std::unique_ptr<ZWindowFacetPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowFacet)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWFACET_INCLUDED
