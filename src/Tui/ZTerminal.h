#ifndef TUIWIDGETS_ZTERMINAL_INCLUDED
#define TUIWIDGETS_ZTERMINAL_INCLUDED

#include <memory>
#include <termios.h>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;
class ZPainter;

class ZTerminalPrivate;

class TUIWIDGETS_EXPORT ZTerminal : public QObject {
    Q_OBJECT
public:
    explicit ZTerminal(QObject *parent = 0);
    virtual ~ZTerminal();

public:
    ZPainter painter();
    ZWidget* mainWidget();
    void setMainWidget(ZWidget* w);

    void update();


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

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_INCLUDED
