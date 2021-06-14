#ifndef TUIWIDGETS_ZTERMINAL_INCLUDED
#define TUIWIDGETS_ZTERMINAL_INCLUDED

#include <memory>
#include <termios.h>

#include <QObject>

#include <Tui/ZCommon.h>
#include <Tui/ZValuePtr.h>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZKeyEvent;
class ZPasteEvent;
class ZSymbol;
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
        ForceIncompatibleTerminals = 1 << 6,
        DisableTaggedPaste = 1 << 7,
    };
    Q_DECLARE_FLAGS(Options, Option)

    class OffScreenData;
    class OffScreen {
    public:
        OffScreen(int width, int height);
        OffScreen(const OffScreen&);
        ~OffScreen();

    public:
        OffScreen& operator=(const OffScreen&);

    public:
        OffScreen withCapability(ZSymbol capability);
        OffScreen withoutCapability(ZSymbol capability);

    private:
        friend class OffScreenData;
        ZValuePtr<OffScreenData> tuiwidgets_pimpl_ptr;
    };

public:
    explicit ZTerminal(QObject *parent = nullptr);
    explicit ZTerminal(Options options, QObject *parent = nullptr);
    explicit ZTerminal(const OffScreen& offscreen, QObject *parent = nullptr);
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

    bool hasCapability(ZSymbol cap);

    void pauseOperation();
    void unpauseOperation();
    bool isPaused();

    void setCursorStyle(CursorStyle style);
    void setCursorPosition(QPoint cursorPosition);
    void setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB);

    void maybeRequestLayout(ZWidget *w);
    void requestLayout(ZWidget *w);
    bool isLayoutPending();
    void doLayout();

    void dispatchKeyboardEvent(ZKeyEvent &translated);
    void dispatchPasteEvent(ZPasteEvent &translated);

protected:
    std::unique_ptr<ZKeyEvent> translateKeyEvent(const ZTerminalNativeEvent &nativeEvent);


public Q_SLOTS:

Q_SIGNALS:
    void afterRendering();
    void incompatibleTerminalDetected();
    void terminalConnectionLost();

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
TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZTerminal::Options)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZTerminal::Options)

#endif // TUIWIDGETS_ZTERMINAL_INCLUDED
