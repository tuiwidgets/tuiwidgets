// SPDX-License-Identifier: BSL-1.0

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
    explicit ZCommandNotifier(ZImplicitSymbol command, QObject *parent = nullptr);
    explicit ZCommandNotifier(ZImplicitSymbol command, QObject *parent, Qt::ShortcutContext context /*= Qt::ApplicationShortcut*/);
    ~ZCommandNotifier() override;

public:
    ZSymbol command() const;
    Qt::ShortcutContext context() const;

    bool isEnabled() const;
    void setEnabled(bool s);
    bool isContextSatisfied() const;

Q_SIGNALS:
    void activated();
    void enabledChanged(bool s);

public:
    bool event(QEvent *event) override;
    // public virtuals from base class override everything for later ABI compatibility
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
