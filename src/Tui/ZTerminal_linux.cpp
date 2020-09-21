#include <Tui/ZTerminal_p.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <QElapsedTimer>
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
#define container_of(ptr, type, member) (reinterpret_cast<type *>(reinterpret_cast<char *>(ptr) - offsetof(type, member)))

#ifdef __cpp_lib_atomic_is_always_lock_free
#define STATIC_ASSERT_ALWAYS_LOCKFREE(type) static_assert (type::is_always_lock_free)
#else
#define STATIC_ASSERT_ALWAYS_LOCKFREE(type) /* not supported */
#endif

// signal based terminal restore...
static bool systemRestoreInited = false;
STATIC_ASSERT_ALWAYS_LOCKFREE(std::atomic<int>);
static std::atomic<int> systemRestoreFd { -1 }; // only written by non signal handling code
static std::atomic<int> systemPausedFd { -1 }; // written by signal handling and non signal handling code
static termios systemOriginalTerminalAttributes;
STATIC_ASSERT_ALWAYS_LOCKFREE(std::atomic<const char *>);
static std::atomic<const char *> systemRestoreEscape { nullptr }; // only written by non signal handling code
STATIC_ASSERT_ALWAYS_LOCKFREE(std::atomic<bool>);
static std::atomic<bool> systemTerminalPaused { false }; // only written by non signal handling code
static termios systemPresuspendTerminalAttributes;
static std::unique_ptr<PosixSignalNotifier> systemTerminalResumeNotifier;
static std::unique_ptr<PosixSignalNotifier> systemTerminalSizeChangeNotifier;
static QPointer<ZTerminal> systemTerminal;

static void restoreSystemHandler(const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    if (systemTerminalPaused.load()) return;

    int restoreFd = systemRestoreFd.load();
    tcsetattr(restoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
    const char *restoreEscape = systemRestoreEscape.load();
    if (restoreEscape) {
        write(restoreFd, restoreEscape, strlen(restoreEscape));
    }
}

static void suspendHelper(bool tcattr) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    int restoreFd = systemRestoreFd.load();
    int duppedFd = fcntl(restoreFd, F_DUPFD_CLOEXEC, 0);
    if (duppedFd == -1) {
        // There's not much we can do if basics like this fail,
        // stuff will be broken after this. But likely the user just needs to regain control.
        if (tcattr && tcgetpgrp(restoreFd) == getpgrp()) {
            tcsetattr(restoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
        }

        const char *restoreEscape = systemRestoreEscape.load();
        if (restoreEscape) {
            write(restoreFd, restoreEscape, strlen(restoreEscape));
        }
        write(restoreFd, "F_DUPFD_CLOEXEC failed, resume might be unreliable\r\n", strlen("F_DUPFD_CLOEXEC failed, resume might be unreliable\r\n"));
        return;
    }
    int fd = -1;
    if (systemPausedFd.compare_exchange_strong(fd, duppedFd)) { // 'signal paused' state was not yet entered
        if (tcattr && tcgetpgrp(duppedFd) == getpgrp()) {
            // resave if this process is still in the foreground process group
            tcgetattr(duppedFd, &systemPresuspendTerminalAttributes);
        }

        // make sure nothing is output to the teminal until it's properly restored
        int nullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
        if (nullfd == -1) {
            systemPausedFd.store(-1);
            close(duppedFd);
            // There's not much we can do if basics like this fail,
            // stuff will be broken after this. But likely the user just needs to regain control.
            if (tcattr && tcgetpgrp(restoreFd) == getpgrp()) {
                tcsetattr(restoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
            }

            const char *restoreEscape = systemRestoreEscape.load();
            if (restoreEscape) {
                write(restoreFd, restoreEscape, strlen(restoreEscape));
            }
            write(restoreFd, "opening of /dev/null failed, resume might be unreliable\r\n", strlen("opening of /dev/null failed, resume might be unreliable\r\n"));
            return;
        } else {
            int tmp;
            do {
#ifndef __APPLE__
                tmp = dup3(nullfd, restoreFd, O_CLOEXEC);
#else
                tmp = dup2(nullfd, restoreFd);
                fcntl(tmp, F_SETFD, O_CLOEXEC);
#endif
            } while (tmp == -1 && errno == EINTR);
            close(nullfd);

            const char *restoreEscape = systemRestoreEscape.load();
            if (restoreEscape) {
                write(duppedFd, restoreEscape, strlen(restoreEscape));
            }

            if (tcattr && tcgetpgrp(duppedFd) == getpgrp()) {
                tcsetattr(duppedFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
            }
        }
    } else {
        close(duppedFd);
    }
}

/*
 * Needed manual tests:
 * 1) send TSTP, resume via shell
 * 2) send STOP, resume via shell
 * 3) send CONT without STOP/TSTP
 * 4) send TSTP, send CONT, resume via shell
 * 5) send STOP, send CONT, resume via shell
 *
 * Use terminal title and push/pop to make sure sequences paired
 *
 * prepare: echo -ne "\033[22t\033];test1\033\\\033[22t\033];test2\033\\"
 * do signal things
 * check that title is "test2", echo -ne "\033[23t", check title is "test1"
 * for next test either use a fresh terminal and prepare it or do echo -ne "\033[22t"; echo -ne "\033];test2\033\\"
 */

static void suspendHandler(PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    if (!systemTerminalPaused.load() && systemRestoreFd.load() != -1) {
        suspendHelper(true);
    }

    flags.reraise();
}

static void resumeHandler(PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(info);
    Q_UNUSED(context);

    // This handler can be called either on
    // a) resume from TSTP. In that case the state was prepared by suspendHandler
    // b) resume from STOP. As stop is uncatchable, the state is normal steady application state
    // c) whenever someone sends a CONT signal in other cases. This is just like b) but with less
    //    likelyhood that someone changed the terminal settings inbetween.

    int restoreFd = systemRestoreFd.load();
    if (systemTerminalPaused.load() || restoreFd == -1) return;

    int pausedFd = systemPausedFd.load();
    int fd = (pausedFd != -1) ? pausedFd : restoreFd;
    if (tcgetpgrp(fd) != getpgrp()) {
        // spurious wakeup, this process is still not in the foreground process group,
        // restoring the terminal state will just halt again anyway.
        raise(SIGTTOU);
        return;
    }

    // move into 'signal paused' state if not already.
    suspendHelper(false);

    pausedFd = systemPausedFd.load(); // reload, suspendHelper will have changed this
    if (pausedFd != -1) {
        tcsetattr(pausedFd, TCSAFLUSH, &systemPresuspendTerminalAttributes);
    } else {
        tcsetattr(restoreFd, TCSAFLUSH, &systemPresuspendTerminalAttributes);
    }
}
// end of signal handling part

namespace  {
    bool isFileRw(int fd) {
        int ret = fcntl(fd, F_GETFL);
        return ret != -1 && (ret & O_ACCMODE) == O_RDWR;
    }
}

bool ZTerminalPrivate::terminalAvailable() {
    bool from_std_fd = false;
    from_std_fd = (isatty(0) && isFileRw(0))
            || (isatty(1) && isFileRw(1))
            || (isatty(2) && isFileRw(2));
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

    if (isatty(0) && isFileRw(0)) {
        fd = 0;
    } else if (isatty(1) && isFileRw(1)) {
        fd = 1;
    } else if (isatty(2) && isFileRw(2)) {
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
    if (initState == ZTerminalPrivate::InitState::Deinit) {
        // already done
        return;
    }
    inputNotifier = nullptr; // ensure no more notifications from this point
    termpaint_terminal_reset_attributes(terminal);
    if (initState == ZTerminalPrivate::InitState::Paused) {
        termpaint_terminal_free(terminal);
    } else {
        termpaint_terminal_free_with_restore(terminal);
    }
    termpaint_integration_deinit(&integration);
    if (fd != -1 && fd == systemRestoreFd.load()) {
        const char *old = systemRestoreEscape.load();
        systemRestoreEscape.store(nullptr);
        PosixSignalManager::instance()->barrier();
        delete old;
    }
    terminal = nullptr;
    if (awaitingResponse) {
        // If terminal auto detection or another operation with response is cut short
        // the reponse will leak out into the next application.
        // We can't reliably prevent that here, but this kludge can reduce the likelyhood
        // by just discarding input for a short amount of time.
        QElapsedTimer timer;
        timer.start();
        while (!timer.hasExpired(100)) {
            int ret;
            struct pollfd info;
            info.fd = fd;
            info.events = POLLIN;
            ret = poll(&info, 1, std::max((qint64)1, 100 - timer.elapsed()));
            if (ret == 1) {
                char buff[1000];
                int amount = (int)read(fd, buff, 999);
                if (amount < 0) {
                    break;
                }
            }
        }
    }
    if (fd != -1) {
        tcsetattr (fd, TCSAFLUSH, &originalTerminalAttributes);
    }
    initState = ZTerminalPrivate::InitState::Deinit;
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

static void event_handler(void *user_data, termpaint_event *event) {
    ZTerminal* that = static_cast<ZTerminal*>(user_data);
    ZTerminalNativeEvent tuiEvent{event};
    QCoreApplication::sendEvent(that, &tuiEvent);
}

bool ZTerminalPrivate::commonStuff(ZTerminal::Options options) {
    init_fns();
    callbackRequested = false;
    terminal = termpaint_terminal_new(&integration);
    surface = termpaint_terminal_get_surface(terminal);

    struct winsize s;
    if (isatty(fd) && ioctl(fd, TIOCGWINSZ, &s) >= 0) {
        termpaint_surface_resize(surface, s.ws_col, s.ws_row);
    } else {
        termpaint_surface_resize(surface, 80, 24);
    }

    tcgetattr(fd, &originalTerminalAttributes);
    // The following call sends a TTOU signal if this process is in a background job.
    // This cleanly stops terminal init, until after the process's job is moved to the foreground.
    tcsetattr(fd, TCSANOW, &originalTerminalAttributes);

    if (!systemRestoreInited) {
        // TODO this only really works well for the first terminal in an process.
        // Thats ok for now, but destructing a terminal should reset it enough to
        // connect to a newly created instance.
        systemOriginalTerminalAttributes = originalTerminalAttributes;
        systemRestoreInited = true;
        systemRestoreFd.store(fd);
        if (!PosixSignalManager::isCreated()) {
            PosixSignalManager::create();
        }
        // The initial restore sequence update was in termpaint_terminal_new, we missed that
        // because systemRestoreFd was not set yet, trigger update again
        const char* tmp = termpaint_terminal_restore_sequence(terminal);
        integration_restore_sequence_updated(tmp, static_cast<int>(strlen(tmp)));
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
            // This handler can be called either on
            // a) resume from TSTP. In that case the state was prepared by suspendHandler
            // b) resume from STOP. As stop is uncatchable, the state is normal steady application state
            // c) whenever someone sends a CONT signal in other cases. This is just like b)

            int pausedFd = systemPausedFd.load();
            if (pausedFd != -1 && systemPausedFd.compare_exchange_strong(pausedFd, -1)) {
                // TSTP ran at least once since init or the last invocation of this handler
                dup2(pausedFd, systemRestoreFd.load());
                close(pausedFd);
                if (systemTerminal) {
                    termpaint_terminal_unpause(ZTerminalPrivate::get(systemTerminal)->terminal);
                }
            }
            systemTerminal->forceRepaint();
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
    tcgetattr (fd, &tattr);
    tattr.c_iflag |= IGNBRK|IGNPAR;
    tattr.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tattr.c_oflag &= ~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET);
    tattr.c_lflag &= ~(ICANON|IEXTEN|ECHO);
    // Don't set c_lflag |= TOSTOP, because that would cause other background applications on the same tty
    // to be stopped, which would be unexpected for non fullscreen applications. We would like to get
    // this application to stop when it's in the background and tries to write, but that not possible separately.
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

    tcsetattr (fd, TCSAFLUSH, &tattr);

    if (systemRestoreFd.load() == fd) {
        tcgetattr(systemRestoreFd.load(), &systemPresuspendTerminalAttributes);
    }

    termpaint_terminal_set_raw_input_filter_cb(terminal, raw_filter, pub());
    termpaint_terminal_set_event_cb(terminal, event_handler, pub());

    if (!options.testFlag(ZTerminal::DisableAutoDetectTimeoutMessage)) {
        autoDetectTimeoutTimer.reset(new QTimer(pub()));
        autoDetectTimeoutTimer->setSingleShot(true);
        autoDetectTimeoutTimer->start(10000);
        QObject::connect(autoDetectTimeoutTimer.get(), &QTimer::timeout, pub(), [this] {
            QByteArray utf8 = autoDetectTimeoutMessage.toUtf8();
            integration_write(utf8.data(), utf8.size());
            integration_flush();
        });
    }
    termpaint_terminal_auto_detect(terminal);

    callbackTimer.setSingleShot(true);
    QObject::connect(&callbackTimer, &QTimer::timeout, pub(), [terminal=terminal] {
        termpaint_terminal_callback(terminal);
    });

    inputNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
    QObject::connect(inputNotifier.get(), &QSocketNotifier::activated,
                     pub(), [this] (int socket) -> void { integration_terminalFdHasData(socket); });

    return true;
}

void ZTerminalPrivate::pauseTerminal() {
    tcgetattr(fd, &prepauseTerminalAttributes);

    inputNotifier->setEnabled(false);
    termpaint_terminal_pause(terminal);
    tcsetattr(fd, TCSAFLUSH, &originalTerminalAttributes);
    if (fd == systemRestoreFd.load()) {
        systemTerminalPaused.store(true);
    }
}

void ZTerminalPrivate::unpauseTerminal() {
    if (fd == systemRestoreFd.load()) {
        systemTerminalPaused.store(false);
    }
    tcsetattr(fd, TCSAFLUSH, &prepauseTerminalAttributes);
    inputNotifier->setEnabled(true);
    termpaint_terminal_unpause(terminal);
}

void ZTerminalPrivate::integration_terminalFdHasData(int socket) {
    callbackTimer.stop();
    callbackRequested = false;
    awaitingResponse = false;
    char buff[100];
    int amount = read (socket, buff, 99);
    termpaint_terminal_add_input_data(terminal, buff, amount);
    QString peek = QString::fromUtf8(termpaint_terminal_peek_input_buffer(terminal), termpaint_terminal_peek_input_buffer_length(terminal));
    if (peek.length()) {
        ZRawSequenceEvent event(ZRawSequenceEvent::pending, peek);
        QCoreApplication::sendEvent(pub(), &event);
    }
    if (callbackRequested) {
        callbackTimer.start(100);
    }
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

void ZTerminalPrivate::integration_write(const char *data, int length) {
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

void ZTerminalPrivate::integration_request_callback() {
    callbackRequested = true;
}

void ZTerminalPrivate::integration_awaiting_response() {
    awaitingResponse = true;
}

void ZTerminalPrivate::integration_restore_sequence_updated(const char *data, int len) {
    if (fd == systemRestoreFd.load()) {
        unsigned length = static_cast<unsigned>(len);
        const char *old = systemRestoreEscape.load();
        char *update = new char[length + 1];
        memcpy(update, data, length);
        update[length] = 0;
        systemRestoreEscape.store(update);
        PosixSignalManager::instance()->barrier();
        delete old;
    }
}

void ZTerminalPrivate::init_fns() {
    memset(&integration, 0, sizeof(integration));
    auto free = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_free();
    };
    auto write = [] (termpaint_integration* ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_write(data, length);
    };
    auto flush = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_flush();
    };

    termpaint_integration_init(&integration, free, write, flush);

    termpaint_integration_set_is_bad(&integration, [] (termpaint_integration* ptr) {
            return container_of(ptr, ZTerminalPrivate, integration)->integration_is_bad();
    });
    termpaint_integration_set_request_callback(&integration, [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_request_callback();
    });
    termpaint_integration_set_awaiting_response(&integration, [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_awaiting_response();
    });
    termpaint_integration_set_restore_sequence_updated(&integration, [] (termpaint_integration* ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_restore_sequence_updated(data, length);
    });
}

TUIWIDGETS_NS_END
