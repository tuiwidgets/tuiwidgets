#ifndef TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_INCLUDED
#define TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_INCLUDED

#include <QObject>

#include <Tui/ZDefaultWidgetManager.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicDefaultWidgetManagerPrivate;
class TUIWIDGETS_EXPORT ZBasicDefaultWidgetManager : public ZDefaultWidgetManager {
    Q_OBJECT

public:
    explicit ZBasicDefaultWidgetManager(ZWidget *dialog);
    ~ZBasicDefaultWidgetManager() override;

public:
    void setDefaultWidget(ZWidget *w) override;
    ZWidget *defaultWidget() const override;
    bool isDefaultWidgetActive() const override;

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
    ZBasicDefaultWidgetManager(std::unique_ptr<ZBasicDefaultWidgetManagerPrivate> pimpl);

    std::unique_ptr<ZBasicDefaultWidgetManagerPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZBasicDefaultWidgetManager)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_INCLUDED
