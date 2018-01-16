#ifndef TUIWIDGETS_ZCOMMANDNOTIFIER_INCLUDED
#define TUIWIDGETS_ZCOMMANDNOTIFIER_INCLUDED

#include <memory>
#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZImplicitSymbol;
class ZSymbol;

class ZCommandNotifierPrivate;
class TUIWIDGETS_EXPORT ZCommandNotifier : public QObject {
    Q_OBJECT
public:
    explicit ZCommandNotifier(Tui::ZImplicitSymbol command, QObject *parent = 0);
    ~ZCommandNotifier() override;

public:
    ZSymbol command();

    bool isEnabled();
    void setEnabled(bool s);

Q_SIGNALS:
    void activated();
    void enabledChanged(bool s);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    std::unique_ptr<ZCommandNotifierPrivate> tuiwidgets_pimpl_ptr;

    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    void probeParents();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZCommandNotifier)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOMMANDNOTIFIER_INCLUDED
