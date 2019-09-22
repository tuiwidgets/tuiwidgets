#ifndef TUIWIDGETS_ZTERMINAL_INCLUDED
#define TUIWIDGETS_ZTERMINAL_INCLUDED

#include <memory>
#include <termios.h>

#include <QObject>

#include <Tui/ZCommon.h>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZKeyEvent;
class ZPainter;
class ZTerminalNativeEvent;
class ZTextMetrics;
class ZImage;
class ZWidget;

class ZTerminalPrivate;

class TUIWIDGETS_EXPORT ZTerminal : public QObject {
    Q_OBJECT
public:
    enum Option {
        AllowInterrupt = 1 << 0,
        AllowSuspend = 1 << 1,
        AllowQuit = 1 << 2,
        DisableAutoResize = 1 << 3,
        DisableAlternativeScreen = 1 << 4,
        DisableAutoDetectTimeoutMessage = 1 << 5,
    };
    Q_DECLARE_FLAGS(Options, Option)

public:
    explicit ZTerminal(QObject *parent = 0);
    explicit ZTerminal(Options options, QObject *parent = 0);
    virtual ~ZTerminal();

public:
    ZPainter painter();
    ZTextMetrics textMetrics() const;
    ZWidget* mainWidget();
    void setMainWidget(ZWidget* w);

    ZWidget *focusWidget();

    void update();
    void forceRepaint();

    ZImage grabCurrentImage() const;

    int width() const;
    int height() const;
    void resize(int width, int height);

    void updateOutput();
    void updateOutputForceFullRepaint();

    void setTitle(QString title);
    void setIconTitle(QString title);

    void setAutoDetectTimeoutMessage(const QString& message);
    QString autoDetectTimeoutMessage() const;

    void pauseOperation();
    void unpauseOperation();
    bool isPaused();

    void setCursorStyle(CursorStyle style);
    void setCursorPosition(QPoint cursorPosition);
    void setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB);

protected:
    std::unique_ptr<ZKeyEvent> translateKeyEvent(const ZTerminalNativeEvent &nativeEvent);


public Q_SLOTS:

Q_SIGNALS:
    void afterRendering();

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
    void dispatcherIsAboutToBlock();

private:
    Q_DISABLE_COPY(ZTerminal)
    TUIWIDGETS_DECLARE_PRIVATE(ZTerminal)

};
Q_DECLARE_OPERATORS_FOR_FLAGS(ZTerminal::Options)

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_INCLUDED
