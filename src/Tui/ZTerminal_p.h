#ifndef TUIWIDGETS_ZTERMINAL_P_INCLUDED
#define TUIWIDGETS_ZTERMINAL_P_INCLUDED

#include <termpaint.h>
#include <termpaint_input.h>

#include <Tui/ZTerminal.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTerminalPrivate {
public:
    ZTerminalPrivate(ZTerminal *pub);
    virtual ~ZTerminalPrivate();

    bool initTerminal();
    void deinitTerminal();

    void init_fns();
    bool terminalAvailable();
    bool setup();
    bool setupFromControllingTerminal();
    bool commonStuff();

    void integration_free();
    void integration_write(char *data, int length);
    void integration_flush();
    bool integration_is_bad();
    void integration_expect_response();

    termpaint_surface *surface = nullptr; // TODO use ref counted ptr of some kind
    termpaint_integration integration;
    termpaint_input *input = nullptr;

    bool updateRequested = false;

    std::unique_ptr<ZWidget> mainWidget;

    // stuff from integration
    int fd = -1;
    bool auto_close = false;
    bool awaiting_response = false;
    termios originalTerminalAttributes;
    // ^^

    ZTerminal* pub_ptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZTerminal)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINAL_P_INCLUDED
