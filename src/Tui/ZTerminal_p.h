#ifndef TUIWIDGETS_ZTERMINAL_P_INCLUDED
#define TUIWIDGETS_ZTERMINAL_P_INCLUDED

#include <termios.h>

#include <QByteArray>
#include <QMap>
#include <QPoint>
#include <QPointer>
#include <QSocketNotifier>
#include <QTimer>

#include <termpaint.h>

#include <Tui/ListNode_p.h>
#include <Tui/ZTerminal.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;
class ZWidgetPrivate;
class ZShortcutManager;
class ZSymbol;

struct FocusHistoryTag;

class ZTerminalPrivate {
public:
    ZTerminalPrivate(ZTerminal *pub, ZTerminal::Options options);
    virtual ~ZTerminalPrivate();
    static ZTerminalPrivate *get(ZTerminal *terminal);
    static const ZTerminalPrivate *get(const ZTerminal *terminal);

    void setFocus(ZWidget *w);
    ZWidget *focus();

    void setKeyboardGrab(ZWidget *w);
    ZWidget *keyboardGrab();

    ZShortcutManager* ensureShortcutManager();

    void sendOtherChangeEvent(QSet<ZSymbol> unchanged);

    void adjustViewportOffset();
    bool viewportKeyEvent(ZKeyEvent *translated);

    void processPaintingAndUpdateOutput(bool fullRepaint);
    void updateNativeTerminalState();

    bool initTerminal(ZTerminal::Options options, ZTerminal::FileDescriptor *fd);
    void initOffscreen(const ZTerminal::OffScreen &offscreen);
    void initExternal(ZTerminal::TerminalConnectionPrivate *connection, ZTerminal::Options options);
    void deinitTerminal();

    // internal connection
    void initIntegrationForInternalConnection();
    static bool terminalAvailableForInternalConnection();
    bool setupInternalConnection(ZTerminal::Options options, ZTerminal::FileDescriptor *explicitFd);
    bool setupFromControllingTerminal(ZTerminal::Options options);
    bool commonInitForInternalConnection(ZTerminal::Options options);
    void deinitTerminalForInternalConnection();
    void pauseTerminalForInternalConnection();
    void unpauseTerminalForInternalConnection();

    void internalConnection_integration_free();
    void internalConnection_integration_write(const char *data, int length);
    void internalConnection_integration_write_unbuffered(char *data, int length);
    void internalConnection_integration_flush();
    bool internalConnection_integration_is_bad();
    void internalConnection_integration_restore_sequence_updated(const char *data, int len, bool force);
    void internalConnectionTerminalFdHasData(int socket);
    // ^^

    // external connection
    void externalWasResized();
    // ^^

    // common integration
    void initIntegrationCommon();
    void initCommon();
    static _Bool raw_filter(void *user_data, const char *data, unsigned length, _Bool overflow);
    static void event_handler(void *user_data, termpaint_event *event);
    void integration_request_callback();
    void integration_awaiting_response();
    void inputFromConnection(const char *data, int length);
    // ^^

    termpaint_surface *surface = nullptr; // TODO use ref counted ptr of some kind
    termpaint_terminal *terminal = nullptr;
    QPoint terminalCursorPosition;
    CursorStyle terminalCursorStyle = CursorStyle::Unset;
    bool terminalCursorVisible = true;
    int terminalCursorR = -1, terminalCursorG = -1, terminalCursorB = -1;
    termpaint_integration integration;
    std::unique_ptr<QSocketNotifier> inputNotifier;

    bool updateRequested = false;

    std::unique_ptr<ZWidget> mainWidget;
    QPoint cursorPosition = {-1, -1};
    ZWidgetPrivate *focusWidget = nullptr;
    ListHead<ZWidgetPrivate, FocusHistoryTag> focusHistory;
    QPointer<ZWidget> keyboardGrabWidget;
    std::unique_ptr<ZShortcutManager> shortcutManager;
    ZTerminal::Options options;
    QString title;
    bool titleNeedsUpdate;
    QString iconTitle;
    bool iconTitleNeedsUpdate;
    QString pasteTemp;

    QList<QPointer<ZWidget>> layoutPendingWidgets;
    bool layoutRequested = false;
    int layoutGeneration = -1;

    bool viewportActive = false;
    bool viewportUI = false;
    QPoint viewportOffset = {0, 0};
    QPoint viewportRange = {0, 0};

    enum class InitState {
        InInitWithoutPendingPaintRequest,
        InInitWithPendingPaintRequest,
        Ready,
        Paused,
        Deinit
    } initState = InitState::InInitWithoutPendingPaintRequest;

    // stuff from internal integration
    int fd = -1;
    bool auto_close = false;
    QByteArray output_buffer;
    termios originalTerminalAttributes;
    termios prepauseTerminalAttributes;
    // ^^

    // external integration
    ZTerminal::TerminalConnectionPrivate *externalConnection = nullptr;
    // ^^

    // common integration
    QTimer callbackTimer;
    bool backspaceIsX08 = false;
    bool callbackRequested = false;
    bool awaitingResponse = false;

    QString autoDetectTimeoutMessage = QStringLiteral("Terminal auto detection is taking unusually long, press space to abort.");
    std::unique_ptr<QTimer> autoDetectTimeoutTimer;
    // ^^

    ZTerminal* pub_ptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZTerminal)
};

class ZTerminal::OffScreenData : public QSharedData {
public:
    OffScreenData(int width, int height);
public:
    int width;
    int height;

    QMap<ZSymbol, bool> capabilities;

    // back door
    static OffScreenData* get(ZTerminal::OffScreen *data) { return data->tuiwidgets_pimpl_ptr.get(); }
    static const OffScreenData* get(const ZTerminal::OffScreen *data) { return data->tuiwidgets_pimpl_ptr.get(); }
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_P_INCLUDED
