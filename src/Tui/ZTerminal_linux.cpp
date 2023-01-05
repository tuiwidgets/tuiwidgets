// SPDX-License-Identifier: BSL-1.0

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
#include <QMetaMethod>

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
static bool systemRestoreInited = false; // global once only signal handler registration
STATIC_ASSERT_ALWAYS_LOCKFREE(std::atomic<int>);
static std::atomic<int> systemRestoreFd { -1 }; // only written by non signal handling code
                                                // atomic for release-aquire pair for systemOriginalTerminalAttributes
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

static bool terminal_is_disconnected(int fd) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.

    // Using poll seems to be the most portable way to detect if a tty is in hangup/disconnected state.
    // Other alternatives: tcgetattr and checking errno for EIO works on linux.
    //                     a zero bytes write and checking for EIO or ENXIO seems to work on may systems but posix
    //                     does not say anything about 0 byte writes on terminals (only on regular files)
    struct pollfd info;
    info.fd = fd;
    info.events = POLLIN;
    int ret = poll(&info, 1, 0);
    return ret == 1 && info.revents & POLLHUP;
}

static void restoreSystemHandler(void *data, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(data);
    Q_UNUSED(info);
    Q_UNUSED(context);

    if (systemTerminalPaused.load()) return;

    int restoreFd = systemRestoreFd.load();

    if (restoreFd == -1) return;

    tcsetattr(restoreFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
    const char *restoreEscape = systemRestoreEscape.load();
    if (restoreEscape) {
        (void)!write(restoreFd, restoreEscape, strlen(restoreEscape));
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
            (void)!write(restoreFd, restoreEscape, strlen(restoreEscape));
        }
        (void)!write(restoreFd, "F_DUPFD_CLOEXEC failed, resume might be unreliable\r\n", strlen("F_DUPFD_CLOEXEC failed, resume might be unreliable\r\n"));
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
                (void)!write(restoreFd, restoreEscape, strlen(restoreEscape));
            }
            (void)!write(restoreFd, "opening of /dev/null failed, resume might be unreliable\r\n", strlen("opening of /dev/null failed, resume might be unreliable\r\n"));
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
                (void)!write(duppedFd, restoreEscape, strlen(restoreEscape));
            }

            if (tcattr && tcgetpgrp(duppedFd) == getpgrp()) {
                tcsetattr(duppedFd, TCSAFLUSH, &systemOriginalTerminalAttributes);
            }
        }
    } else {
        close(duppedFd);
    }
}

static void hupHandler(void *data, PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(data);
    Q_UNUSED(info);
    Q_UNUSED(context);

    // It seems impossible to detect if this SIGHUP is really from a disconnected terminal or send for other reasons.
    // That's because many applications that use a pty send a manual SIGHUP shortly before actually disconnecting
    // the terminal. Thus no way to differentiate between a stray kill -HUP or a real terminal hangup without using
    // fragile timeouts.
    //
    // So we just do nothing here and rely on the event loop integration to get a zero byte read and then detect the
    // disconnected terminal.
    //
    // On the other side this mean that per default applications will ignore stray SIGHUP which should be ok. If some
    // other handling is wanted the application is free to setup a sync handler using PosixSignalManager with
    // `flags.reraise();` if terminating behavior is wanted or an async handler if event loop based handling is wanted.
    //
    // Ignore the signal via an empty handler instead of using SIG_IGN to not mess with state inheritable through execve
    flags.clearReraise();
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

static void suspendHandler(void *data, PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(data);
    Q_UNUSED(info);
    Q_UNUSED(context);

    if (!systemTerminalPaused.load() && systemRestoreFd.load() != -1) {
        suspendHelper(true);
    }

    flags.reraise();
}

static void resumeHandler(void *data, PosixSignalFlags &flags, const siginfo_t *info, void *context) {
    // !!! signal handler code, only use async-safe calls (see signal-safety(7)) , no Qt at all.
    Q_UNUSED(flags);
    Q_UNUSED(data);
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

    if (terminal_is_disconnected(fd)) {
        return;
    }

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

bool ZTerminalPrivate::terminalAvailableForInternalConnection() {
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

bool ZTerminalPrivate::setupInternalConnection(ZTerminal::Options options, ZTerminal::FileDescriptor *explicitFd) {
    if (fd != -1) {
        return false;
    }

    fd = -1;
    auto_close = false;

    if (explicitFd) {
        if (isatty(explicitFd->fd()) && isFileRw(explicitFd->fd())) {
            fd = explicitFd->fd();
            return commonInitForInternalConnection(options);
        }
        return false;
    }

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

    return commonInitForInternalConnection(options);
}

void ZTerminalPrivate::deinitTerminalForInternalConnection() {
    inputNotifier = nullptr; // ensure no more notifications from this point
    if (fd != -1 && fd == systemRestoreFd.load()) {
        const char *old = systemRestoreEscape.load();
        systemRestoreEscape.store(nullptr);
        systemRestoreFd.store(-1);
        systemTerminal = nullptr;
        int pausedFd = systemPausedFd.load();
        if (pausedFd != -1) {
            dup2(pausedFd, fd);
            close(pausedFd);
            systemPausedFd.store(-1);
        }
        PosixSignalManager::instance()->barrier();
        delete [] old;
    }
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

    return commonInitForInternalConnection(options);
}

void ZTerminalPrivate::showErrorWithoutTerminal(const QByteArray utf) {
    // Try on stderr first
    if (write(2, utf.data(), utf.size()) <= 0) {
        // Retry on stdout, but we can't really do anything about it if that fails too.
        (void)!write(1, utf.data(), utf.size());
    }
}

bool ZTerminalPrivate::commonInitForInternalConnection(ZTerminal::Options options) {
    initIntegrationForInternalConnection();
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
    backspaceIsX08 = (originalTerminalAttributes.c_cc[VERASE] == 0x08);
    // The following call sends a TTOU signal if this process is in a background job.
    // This cleanly stops terminal init, until after the process's job is moved to the foreground.
    tcsetattr(fd, TCSANOW, &originalTerminalAttributes);

    // determine if fd refers to the controlling tty
    const bool controllingTty = (tcgetpgrp(fd) != -1);
    if (controllingTty) {
        if (systemRestoreFd.load() != -1) {
            qWarning("Two ZTerminal instances connected to the controlling terminal at once is not supported");
        } else {
            if (!PosixSignalManager::isCreated()) {
                PosixSignalManager::create();
            }

            systemOriginalTerminalAttributes = originalTerminalAttributes;

            // The initial restore sequence update was in termpaint_terminal_new, we missed that
            // because systemRestoreFd was not set yet, trigger update again
            const char *tmp = termpaint_terminal_restore_sequence(terminal);
            internalConnection_integration_restore_sequence_updated(tmp, static_cast<int>(strlen(tmp)), true);

            // After this the signal handler pay attention to the just setup state
            // Also this forms a release-aquire pair with reads in the signal handler
            // to allow for race free access of systemOriginalTerminalAttributes.
            systemRestoreFd.store(fd);
            systemTerminal = pub();

            if (!systemRestoreInited) {
                systemRestoreInited = true;
                PosixSignalManager::instance()->addSyncTerminationHandler(restoreSystemHandler, nullptr);
                PosixSignalManager::instance()->addSyncCrashHandler(restoreSystemHandler, nullptr);
                PosixSignalManager::instance()->addSyncSignalHandler(SIGHUP, hupHandler, nullptr);
                PosixSignalManager::instance()->addSyncSignalHandler(SIGTSTP, suspendHandler, nullptr);
                PosixSignalManager::instance()->addSyncSignalHandler(SIGTTIN, suspendHandler, nullptr);
                PosixSignalManager::instance()->addSyncSignalHandler(SIGTTOU, suspendHandler, nullptr);
                // resume is two step. A synchronous part which restores terminal mode
                // and normalizes state in case suspendHandler was not called (SIGSTOP or manual SIGCONT)
                PosixSignalManager::instance()->addSyncSignalHandler(SIGCONT, resumeHandler, nullptr);
                // and a notifier part that resends escape sequences and triggers
                // repaint in the next main loop interation
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

                        if (terminal_is_disconnected(systemRestoreFd.load())) {
                            return;
                        }

                        if (systemTerminal) {
                            termpaint_terminal_unpause(ZTerminalPrivate::get(systemTerminal)->terminal);
                        }
                    } else {
                        if (terminal_is_disconnected(systemRestoreFd.load())) {
                            return;
                        }
                    }

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
            }
        }
    }

    struct termios tattr;
    tcgetattr (fd, &tattr);
    tattr.c_iflag |= IGNBRK | IGNPAR;
    tattr.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tattr.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONOCR | ONLRET);
    tattr.c_lflag &= ~(ICANON | IEXTEN | ECHO);
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

    tcsetattr(fd, TCSAFLUSH, &tattr);

    if (systemRestoreFd.load() == fd) {
        tcgetattr(systemRestoreFd.load(), &systemPresuspendTerminalAttributes);
    }

    initCommon();

    inputNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
    QObject::connect(inputNotifier.get(), &QSocketNotifier::activated,
                     pub(), [this] (int socket) -> void { internalConnectionTerminalFdHasData(socket); });

    return true;
}

void ZTerminalPrivate::pauseTerminalForInternalConnection() {
    tcgetattr(fd, &prepauseTerminalAttributes);

    inputNotifier->setEnabled(false);
    termpaint_terminal_pause(terminal);
    tcsetattr(fd, TCSAFLUSH, &originalTerminalAttributes);
    if (fd == systemRestoreFd.load()) {
        systemTerminalPaused.store(true);
    }
}

void ZTerminalPrivate::unpauseTerminalForInternalConnection() {
    if (fd == systemRestoreFd.load()) {
        systemTerminalPaused.store(false);
    }
    tcsetattr(fd, TCSAFLUSH, &prepauseTerminalAttributes);
    inputNotifier->setEnabled(true);
    termpaint_terminal_unpause(terminal);
}

void ZTerminalPrivate::internalConnectionTerminalFdHasData(int socket) {
    char buff[100];
    int amount = read(socket, buff, 99);
    if (amount == 0) {
        if (terminal_is_disconnected((socket))) {
            inputNotifier->setEnabled(false);
            QPointer<ZTerminal> weak = pub();
            if (pub()->isSignalConnected(QMetaMethod::fromSignal(&ZTerminal::terminalConnectionLost))) {
                QTimer::singleShot(0, [weak] {
                    if (!weak.isNull()) {
                        weak->terminalConnectionLost();
                    }
                });
            } else {
                QCoreApplication::quit();
            }
            return;
        }
    } else if (amount < 0) {
        return;
    }
    inputFromConnection(buff, amount);
}

void ZTerminalPrivate::internalConnection_integration_free() {
    // this does not really free, because ZTerminalPrivate which contains the integration struct is externally owned
    if (auto_close && fd != -1) {
        close(fd);
    }
}

void ZTerminalPrivate::internalConnection_integration_write_unbuffered(char *data, int length) {
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

void ZTerminalPrivate::internalConnection_integration_write(const char *data, int length) {
    output_buffer.append(data, length);
    if (output_buffer.size() > 512 || options.testFlag(ZTerminal::DebugDisableBufferedIo)) {
        internalConnection_integration_flush();
    }
}

void ZTerminalPrivate::internalConnection_integration_flush() {
    internalConnection_integration_write_unbuffered(output_buffer.data(), output_buffer.size());
    output_buffer.clear();
}

bool ZTerminalPrivate::internalConnection_integration_is_bad() {
    return fd == -1;
}

void ZTerminalPrivate::internalConnection_integration_restore_sequence_updated(const char *data, int len, bool force) {
    if (fd == systemRestoreFd.load() || force) {
        unsigned length = static_cast<unsigned>(len);
        const char *old = systemRestoreEscape.load();
        char *update = new char[length + 1];
        memcpy(update, data, length);
        update[length] = 0;
        systemRestoreEscape.store(update);
        PosixSignalManager::instance()->barrier();
        delete [] old;
    }
}

void ZTerminalPrivate::initIntegrationForInternalConnection() {
    memset(&integration, 0, sizeof(integration));
    auto free = [] (termpaint_integration *ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->internalConnection_integration_free();
    };
    auto write = [] (termpaint_integration *ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->internalConnection_integration_write(data, length);
    };
    auto flush = [] (termpaint_integration *ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->internalConnection_integration_flush();
    };

    termpaint_integration_init(&integration, free, write, flush);

    initIntegrationCommon();

    termpaint_integration_set_is_bad(&integration, [] (termpaint_integration *ptr) {
            return container_of(ptr, ZTerminalPrivate, integration)->internalConnection_integration_is_bad();
    });
    termpaint_integration_set_restore_sequence_updated(&integration, [] (termpaint_integration *ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->internalConnection_integration_restore_sequence_updated(data, length, false);
    });
}

TUIWIDGETS_NS_END
