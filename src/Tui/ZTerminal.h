// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTERMINAL_INCLUDED
#define TUIWIDGETS_ZTERMINAL_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/ZCommon.h>
#include <Tui/ZValuePtr.h>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZKeyEvent;
class ZPasteEvent;
class ZPendingKeySequenceCallbacks;
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
    enum Option : uint32_t {
        AllowInterrupt = 1 << 0,
        AllowSuspend = 1 << 1,
        AllowQuit = 1 << 2,
        DisableAutoResize = 1 << 3,
        DisableAlternativeScreen = 1 << 4,
        DisableAutoDetectTimeoutMessage = 1 << 5,
        ForceIncompatibleTerminals = 1 << 6,
        DisableTaggedPaste = 1 << 7,
        DebugDisableBufferedIo = 1 << 8,
        ConservativeTrueColorOutput = 1 << 9,
    };
    Q_DECLARE_FLAGS(Options, Option)

    struct FileDescriptor {
        explicit FileDescriptor(int fd) : _fd(fd) {}

        int fd() { return _fd; }

    private:
        int _fd;
    };

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

    class TerminalConnectionDelegate {
    public:
        TerminalConnectionDelegate();
        virtual ~TerminalConnectionDelegate();

        TerminalConnectionDelegate &operator=(const TerminalConnectionDelegate&) = delete;

    public:
        virtual void write(const char *data, int length) = 0;
        virtual void flush() = 0;
        virtual void restoreSequenceUpdated(const char *data, int len) = 0;
        virtual void deinit(bool awaitingResponse) = 0;
        virtual void pause();
        virtual void unpause();
    };

    class TerminalConnectionPrivate;
    class TerminalConnection {
    public:
        TerminalConnection();
        ~TerminalConnection();

    public:
        void terminalInput(const char *data, int length);
        void setDelegate(TerminalConnectionDelegate *delegate);
        void setBackspaceIsX08(bool val);
        void setSize(int width, int height);

    protected:
        std::unique_ptr<TerminalConnectionPrivate> tuiwidgets_pimpl_ptr;
    private:
        Q_DISABLE_COPY(TerminalConnection)
        TUIWIDGETS_DECLARE_PRIVATE(TerminalConnection)
    };

public:
    explicit ZTerminal(QObject *parent = nullptr);
    explicit ZTerminal(Options options, QObject *parent = nullptr);
    explicit ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr);
    explicit ZTerminal(const OffScreen& offscreen, QObject *parent = nullptr);
    explicit ZTerminal(TerminalConnection *connection, Options options, QObject *parent = nullptr);
    virtual ~ZTerminal() override;

public:
    static bool isDefaultTerminalAvailable();

public:
    ZPainter painter();
    ZTextMetrics textMetrics() const;
    ZWidget *mainWidget() const;
    void setMainWidget(ZWidget *w);

    ZWidget *focusWidget() const;
    ZWidget *keyboardGrabber() const;

    void update();
    void forceRepaint();

    ZImage grabCurrentImage() const;
    QPoint grabCursorPosition() const;
    bool grabCursorVisibility() const;
    CursorStyle grabCursorStyle() const;
    std::tuple<int, int, int> grabCursorColor() const;

    int width() const;
    int height() const;
    void resize(int width, int height);

    void updateOutput();
    void updateOutputForceFullRepaint();

    QString title() const;
    void setTitle(const QString &title);
    QString iconTitle() const;
    void setIconTitle(const QString &title);

    void setAutoDetectTimeoutMessage(const QString& message);
    QString autoDetectTimeoutMessage() const;

    bool hasCapability(ZSymbol cap) const;

    void pauseOperation();
    void unpauseOperation();
    bool isPaused() const;

    void setCursorStyle(CursorStyle style);
    void setCursorPosition(QPoint cursorPosition);
    void setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB);

    void registerPendingKeySequenceCallbacks(const ZPendingKeySequenceCallbacks &callbacks);

    void maybeRequestLayout(ZWidget *w);
    void requestLayout(ZWidget *w);
    bool isLayoutPending() const;
    void doLayout();
    int currentLayoutGeneration();

    void dispatchKeyboardEvent(ZKeyEvent &translated);
    void dispatchPasteEvent(ZPasteEvent &translated);

protected:
    std::unique_ptr<ZKeyEvent> translateKeyEvent(const ZTerminalNativeEvent &nativeEvent);


public Q_SLOTS:

Q_SIGNALS:
    void beforeRendering();
    void afterRendering();
    void incompatibleTerminalDetected();
    void terminalConnectionLost();
    void focusChanged();

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

static_assert (std::is_copy_constructible<ZTerminal::OffScreen>::value, "std::is_copy_constructible_v<ZTerminal::OffScreen>");

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZTerminal::Options)

#endif // TUIWIDGETS_ZTERMINAL_INCLUDED
