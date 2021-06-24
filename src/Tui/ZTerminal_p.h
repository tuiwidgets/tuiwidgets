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

    bool initTerminal(ZTerminal::Options options);
    void initOffscreen(const ZTerminal::OffScreen &offscreen);
    void deinitTerminal();
    void pauseTerminal();
    void unpauseTerminal();

    void init_fns();
    bool terminalAvailable();
    bool setup(ZTerminal::Options options);
    bool setupFromControllingTerminal(ZTerminal::Options options);
    bool commonStuff(ZTerminal::Options options);

    void integration_free();
    void integration_write(const char *data, int length);
    void integration_write_uncached(char *data, int length);
    void integration_flush();
    bool integration_is_bad();
    void integration_request_callback();
    void integration_awaiting_response();
    void integration_restore_sequence_updated(const char *data, int len, bool force);
    void integration_terminalFdHasData(int socket);

    termpaint_surface *surface = nullptr; // TODO use ref counted ptr of some kind
    termpaint_terminal *terminal = nullptr;
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
    // stuff from integration
    int fd = -1;
    bool auto_close = false;
    bool callbackRequested = false;
    bool awaitingResponse = false;
    QByteArray output_buffer;
    termios originalTerminalAttributes;
    termios prepauseTerminalAttributes;
    bool backspaceIsX08 = false;
    QTimer callbackTimer;
    // ^^

    QString autoDetectTimeoutMessage = QStringLiteral("Terminal auto detection is taking unusually long, press space to abort.");
    std::unique_ptr<QTimer> autoDetectTimeoutTimer;

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
