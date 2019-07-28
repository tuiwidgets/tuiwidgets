#ifndef TUIWIDGETS_ZTERMINAL_P_INCLUDED
#define TUIWIDGETS_ZTERMINAL_P_INCLUDED

#include <QByteArray>
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

    void processPaintingAndUpdateOutput(bool fullRepaint);

    bool initTerminal(ZTerminal::Options options);
    void maybeSystemTerminalSetup();
    void deinitTerminal();

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
    void integration_expect_response();

    termpaint_surface *surface = nullptr; // TODO use ref counted ptr of some kind
    termpaint_terminal *terminal = nullptr;
    termpaint_integration integration;
    QSocketNotifier *inputNotifier = nullptr;

    bool updateRequested = false;

    std::unique_ptr<ZWidget> mainWidget;
    QPoint cursorPosition = {-1, -1};
    ZWidgetPrivate *focusWidget = nullptr;
    ListHead<ZWidgetPrivate, FocusHistoryTag> focusHistory;
    QPointer<ZWidget> keyboardGrabWidget;
    std::unique_ptr<ZShortcutManager> shortcutManager;
    ZTerminal::Options options;

    enum class InitState {
        InInitWithoutPendingPaintRequest,
        InInitWithPendingPaintRequest,
        Ready
    } initState = InitState::InInitWithoutPendingPaintRequest;
    // stuff from integration
    int fd = -1;
    bool auto_close = false;
    bool awaiting_response = false;
    QByteArray output_buffer;
    termios originalTerminalAttributes;

    void terminalFdHasData(int socket);
    // ^^

    QString autoDetectTimeoutMessage = QStringLiteral("Terminal auto detection is taking unusually long, press space to abort.");
    std::unique_ptr<QTimer> autoDetectTimeoutTimer;

    ZTerminal* pub_ptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZTerminal)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_P_INCLUDED
