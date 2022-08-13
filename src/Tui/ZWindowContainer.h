// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED
#define TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED

#include <QObject>

#include <Tui/ZMenuItem.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowContainerPrivate;
class TUIWIDGETS_EXPORT ZWindowContainer : public QObject {
    Q_OBJECT
public:
    ZWindowContainer();
    ~ZWindowContainer() override;

public:
    virtual QVector<ZMenuItem> containerMenuItems() const;

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
    ZWindowContainer(std::unique_ptr<ZWindowContainerPrivate> pimpl);

protected:
    std::unique_ptr<ZWindowContainerPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowContainer)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED
