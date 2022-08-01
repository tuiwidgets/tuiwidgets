#ifndef TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED
#define TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED

#include <QObject>

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZDefaultWidgetManager : public QObject {
    Q_OBJECT

public:
    ZDefaultWidgetManager();

public:
    virtual void setDefaultWidget(ZWidget *w) = 0;
    virtual ZWidget *defaultWidget() const = 0;
    virtual bool isDefaultWidgetActive() const = 0;

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
    ~ZDefaultWidgetManager() override;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED
