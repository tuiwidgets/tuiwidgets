#ifndef TUIWIDGETS_ZTERMINAL_INCLUDED
#define TUIWIDGETS_ZTERMINAL_INCLUDED

#include <memory>
#include <termios.h>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;
class ZTerminalNativeEvent;
class ZPainter;
class ZKeyEvent;

class ZTerminalPrivate;

class TUIWIDGETS_EXPORT ZTerminal : public QObject {
    Q_OBJECT
public:
    enum Option {
        AllowInterrupt = 1 << 0,
        AllowSuspend = 1 << 1,
        AllowQuit = 1 << 2,
        DisableAutoResize = 1 << 3,
    };
    Q_DECLARE_FLAGS(Options, Option)

public:
    explicit ZTerminal(QObject *parent = 0);
    explicit ZTerminal(Options options, QObject *parent = 0);
    virtual ~ZTerminal();

public:
    ZPainter painter();
    ZWidget* mainWidget();
    void setMainWidget(ZWidget* w);

    ZWidget *focusWidget();

    void update();
    void forceRepaint();

    void resize(int width, int height);

    void updateOutput();
    void updateOutputForceFullRepaint();

protected:
    std::unique_ptr<ZKeyEvent> translateKeyEvent(const ZTerminalNativeEvent &nativeEvent);


public Q_SLOTS:

protected:
    std::unique_ptr<ZTerminalPrivate> tuiwidgets_pimpl_ptr;

    // override everything for later ABI compatibility
public:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    Q_DISABLE_COPY(ZTerminal)
    TUIWIDGETS_DECLARE_PRIVATE(ZTerminal)

};
Q_DECLARE_OPERATORS_FOR_FLAGS(ZTerminal::Options)

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_INCLUDED
