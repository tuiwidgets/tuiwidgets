#ifndef TUIWIDGETS_ZCOMMANDMANAGER_INCLUDED
#define TUIWIDGETS_ZCOMMANDMANAGER_INCLUDED

#include <memory>

#include <QObject>
#include <QPointer>

#include <Tui/ZSymbol.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCommandNotifier;

class ZCommandManagerPrivate;
class TUIWIDGETS_EXPORT ZCommandManager : public QObject {
    Q_OBJECT
public:
    explicit ZCommandManager(QObject *parent = nullptr);
    ~ZCommandManager() override;

    void registerCommandNotifier(ZCommandNotifier *notifier);
    bool isCommandEnabled(ZSymbol command);
    void activateCommand(ZSymbol command);

Q_SIGNALS:
    void commandStateChanged(ZSymbol command);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    std::unique_ptr<ZCommandManagerPrivate> tuiwidgets_pimpl_ptr;

    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private Q_SLOTS:
    void _tui_enabledChanged(bool s);

private:
    void compact(ZSymbol command);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZCommandManager)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOMMANDMANAGER_INCLUDED
