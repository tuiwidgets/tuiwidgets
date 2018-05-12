#include <Tui/ZTerminal_p.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <QSocketNotifier>
#include <QCoreApplication>
#include <QPointer>
#include <QRect>

#include <PosixSignalManager.h>

#include <Tui/ZEvent.h>
#include <Tui/ZWidget_p.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

#ifdef Q_CC_GNU
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

// signal based terminal restore...
static bool systemRestoreInited = false;
static int systemRestoreFd = -1;
static termios systemOriginalTerminalAttributes;
static const char *systemRestoreEscape = nullptr;
static termios systemPresuspendTerminalAttributes;
static std::unique_ptr<PosixSignalNotifier> systemTerminalResumeNotifier;
static std::unique_ptr<PosixSignalNotifier> systemTerminalSizeChangeNotifier;
static QPointer<ZTerminal> systemTerminal;

static void restoreSystemHandler(const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    tcsetattr(systemRestoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
    if (systemRestoreEscape) {
        write(systemRestoreFd, systemRestoreEscape, strlen(systemRestoreEscape));
    }
}

static void suspendHandler(PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    tcgetattr(systemRestoreFd, &systemPresuspendTerminalAttributes);

    tcsetattr(systemRestoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
    if (systemRestoreEscape) {
        write(systemRestoreFd, systemRestoreEscape, strlen(systemRestoreEscape));
    }

    flags.reraise();
}

static void resumeHandler(PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    tcsetattr(systemRestoreFd, TCSAFLUSH, &systemPresuspendTerminalAttributes);
}
// end of signal handling part

bool ZTerminalPrivate::terminalAvailable() {
    bool from_std_fd = false;
    from_std_fd = isatty(0) || isatty(1) || isatty(2);
    if (from_std_fd) {
        return true;
    }
    // also try controlling terminal
    int fd = open("/dev/tty", O_RDONLY | O_NOCTTY | FD_CLOEXEC);
    if (fd != -1) {
        close(fd);
        return true;
    }
    return false;
}

bool ZTerminalPrivate::setup(ZTerminal::Options options) {
    if (fd != -1) {
        return false;
    }

    fd = -1;
    auto_close = false;

    if (isatty(0)) {
        fd = 0;
    } else if (isatty(1)) {
        fd = 1;
    } else if (isatty(2)) {
        fd = 2;
    } else {
        fd = open("/dev/tty", O_RDWR | O_NOCTTY | FD_CLOEXEC);
        auto_close = true;
        if (fd == -1) {
            return false;
        }
    }

    return commonStuff(options);
}

void ZTerminalPrivate::deinitTerminal() {
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &originalTerminalAttributes);
    termpaint_terminal_reset_attributes(terminal);
}

bool ZTerminalPrivate::setupFromControllingTerminal(ZTerminal::Options options) {
    if (fd != -1) {
        return false;
    }

    fd = -1;
    fd = open("/dev/tty", O_RDWR | O_NOCTTY | FD_CLOEXEC);
    if (fd == -1) {
        return false;
    }
    auto_close = true;

    return commonStuff(options);
}



static _Bool raw_filter(void *user_data, const char *data, unsigned length, _Bool overflow) {
    ZTerminal* that = static_cast<ZTerminal*>(user_data);
    QString rawSequence = QString::fromUtf8(data, length);
    ZRawSequenceEvent event{rawSequence};
    return QCoreApplication::sendEvent(that, &event);
}

static void event_handler(void *user_data, termpaint_input_event *event) {
    ZTerminal* that = static_cast<ZTerminal*>(user_data);
    ZTerminalNativeEvent tuiEvent{event};
    QCoreApplication::sendEvent(that, &tuiEvent);
}

bool ZTerminalPrivate::commonStuff(ZTerminal::Options options) {
    // TODO zero out integration
    init_fns();
    awaiting_response = false;
    terminal = termpaint_terminal_new(&integration);
    surface = termpaint_terminal_get_surface(terminal);
    //termpaint_terminal_auto_detect(terminal);

    struct winsize s;
    if (isatty(fd) && ioctl(fd, TIOCGWINSZ, &s) >= 0) {
        termpaint_surface_resize(surface, s.ws_col, s.ws_row);
    } else {
        termpaint_surface_resize(surface, 80, 24);
    }
    //termpaint_surface_clear(surface, 0x1000000);
    termpaint_surface_clear(surface, 0xffffff, 0xff0000);
    termpaint_terminal_flush(terminal, false);


    tcgetattr(STDIN_FILENO, &originalTerminalAttributes);

    if (!systemRestoreInited) {
        // TODO this only really works well for the first terminal in an process.
        // Thats ok for now, but destructing a terminal should reset it enough to
        // connect to a newly created instance.
        systemOriginalTerminalAttributes = originalTerminalAttributes;
        systemRestoreInited = true;
        systemRestoreFd = fd;
        if (!PosixSignalManager::isCreated()) {
            PosixSignalManager::create();
        }
        systemRestoreEscape = "\e[0m\r\n"; // TODO this should come from termpaint
        PosixSignalManager::instance()->addSyncTerminationHandler(restoreSystemHandler);
        PosixSignalManager::instance()->addSyncCrashHandler(restoreSystemHandler);
        PosixSignalManager::instance()->addSyncSignalHandler(SIGTSTP, suspendHandler);
        PosixSignalManager::instance()->addSyncSignalHandler(SIGTTIN, suspendHandler);
        PosixSignalManager::instance()->addSyncSignalHandler(SIGTTOU, suspendHandler);
        // resume is two step. A synchronous part which restores terminal mode
        PosixSignalManager::instance()->addSyncSignalHandler(SIGCONT, resumeHandler);
        // and a notifier part that triggers repaint in the next main loop interation
        systemTerminalResumeNotifier = std::unique_ptr<PosixSignalNotifier>(new PosixSignalNotifier(SIGCONT));
        QObject::connect(systemTerminalResumeNotifier.get(), &PosixSignalNotifier::activated, [] {
            if (systemTerminal) {
                systemTerminal->forceRepaint();
            }
        });

        systemTerminalSizeChangeNotifier = std::unique_ptr<PosixSignalNotifier>(new PosixSignalNotifier(SIGWINCH));
        QObject::connect(systemTerminalSizeChangeNotifier.get(), &PosixSignalNotifier::activated, [] {
            if (systemTerminal) {
                auto *const p = systemTerminal->tuiwidgets_impl();
                if (p->options.testFlag(ZTerminal::DisableAutoResize)) {
                    return;
                }
                struct winsize s;
                if (isatty(p->fd) && ioctl(p->fd, TIOCGWINSZ, &s) >= 0) {
                    systemTerminal->resize(s.ws_col, s.ws_row);
                }
            }
        });

        systemTerminal = pub();
    }

    struct termios tattr;
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_iflag |= IGNBRK|IGNPAR;
    tattr.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tattr.c_oflag &= ~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET);
    tattr.c_lflag &= ~(ICANON|IEXTEN|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;

    if ((options & (ZTerminal::AllowInterrupt | ZTerminal::AllowQuit | ZTerminal::AllowSuspend)) == 0) {
        tattr.c_lflag &= ~ISIG;
    } else {
        if (!options.testFlag(ZTerminal::AllowInterrupt)) {
            tattr.c_cc[VINTR] = 0;
        }
        if (!options.testFlag(ZTerminal::AllowQuit)) {
            tattr.c_cc[VQUIT] = 0;
        }
        if (!options.testFlag(ZTerminal::AllowSuspend)) {
            tattr.c_cc[VSUSP] = 0;
        }
    }

    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);

    termpaint_terminal_set_raw_input_filter_cb(terminal, raw_filter, pub());
    termpaint_terminal_set_event_cb(terminal, event_handler, pub());

    inputNotifier = new QSocketNotifier(fd, QSocketNotifier::Read);
    QObject::connect(inputNotifier, &QSocketNotifier::activated, [terminal=terminal, that=pub()] (int socket) -> void {
        char buff[100];
        int amount = read (socket, buff, 99);
        termpaint_terminal_add_input_data(terminal, buff, amount);
        QString peek = QString::fromUtf8(termpaint_terminal_peek_input_buffer(terminal), termpaint_terminal_peek_input_buffer_length(terminal));
        if (peek.length()) {
            ZRawSequenceEvent event(ZRawSequenceEvent::pending, peek);
            bool ret = QCoreApplication::sendEvent(that, &event);
        }
    });


    return true;
}

void ZTerminalPrivate::integration_free() {
    // this does not really free, because ZTerminalPrivate is externally owned
    if (auto_close && fd != -1) {
        close(fd);
    }
}

void ZTerminalPrivate::integration_write_uncached(char *data, int length) {
    int written = 0;
    int ret;
    errno = 0;
    while (written != length) {
        ret = write(fd, data+written, length-written);
        if (ret > 0) {
            written += ret;
        } else {
            // error handling?
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // fatal, non blocking is not supported by this integration
                fd = -1;
                return;
            }
            if (errno == EIO || errno == ENOSPC) {
                // fatal?
                fd = -1;
                return;
            }
            if (errno == EBADF || errno == EINVAL || errno == EPIPE) {
                // fatal, or fd is gone bad
                fd = -1;
                return;
            }
            if (errno == EINTR) {
                continue;
            }
        }
    }
}

void ZTerminalPrivate::integration_write(char *data, int length) {
    output_buffer.append(data, length);
    if (output_buffer.size() > 512) {
        integration_flush();
    }
}

void ZTerminalPrivate::integration_flush() {
    integration_write_uncached(output_buffer.data(), output_buffer.size());
    output_buffer.clear();
}

bool ZTerminalPrivate::integration_is_bad() {
    return fd == -1;
}

void ZTerminalPrivate::integration_expect_response() {
    awaiting_response = true;
}


void ZTerminalPrivate::init_fns() {
    memset(&integration, 0, sizeof(integration));
    integration.free = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_free();
    };
    integration.write = [] (termpaint_integration* ptr, char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_write(data, length);
    };
    integration.flush = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_flush();
    };
    integration.is_bad = [] (termpaint_integration* ptr) {
        return container_of(ptr, ZTerminalPrivate, integration)->integration_is_bad();
    };
    /*integration.expect_response = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_expect_response();
    };*/
}

TUIWIDGETS_NS_END
