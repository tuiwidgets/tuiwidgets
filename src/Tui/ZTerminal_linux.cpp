#include <Tui/ZTerminal_p.h>

#include <unistd.h>
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

#include <Tui/ZEvent.h>

// TODO remove or rework
TUIWIDGETS_EXPORT std::string peek_buffer;

TUIWIDGETS_NS_START

#ifdef Q_CC_GNU
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

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

bool ZTerminalPrivate::setup() {
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

    return commonStuff();
}

void ZTerminalPrivate::deinitTerminal() {
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &originalTerminalAttributes);
    termpaint_surface_reset_attributes(surface);
}

bool ZTerminalPrivate::setupFromControllingTerminal() {
    if (fd != -1) {
        return false;
    }

    fd = -1;
    fd = open("/dev/tty", O_RDWR | O_NOCTTY | FD_CLOEXEC);
    if (fd == -1) {
        return false;
    }
    auto_close = true;

    return commonStuff();
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

bool ZTerminalPrivate::commonStuff() {
    // TODO zero out integration
    init_fns();
    awaiting_response = false;
    surface = termpaint_surface_new(&integration);
    termpaint_auto_detect(surface);

    struct winsize s;
    if (isatty(fd) && ioctl(fd, TIOCGWINSZ, &s) >= 0) {
        termpaint_surface_resize(surface, s.ws_col, s.ws_row);
    } else {
        termpaint_surface_resize(surface, 80, 24);
    }
    //termpaint_surface_clear(surface, 0x1000000);
    termpaint_surface_clear(surface, 0xff0000);
    termpaint_surface_flush(surface);

    struct termios tattr;

    tcgetattr (STDIN_FILENO, &originalTerminalAttributes);
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_iflag |= IGNBRK|IGNPAR;
    tattr.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    tattr.c_oflag &= ~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET);
    tattr.c_lflag &= ~(ICANON|IEXTEN|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);

    input = termpaint_input_new();
    termpaint_input_set_raw_filter_cb(input, raw_filter, pub());
    termpaint_input_set_event_cb(input, event_handler, pub());

    auto notifier = new QSocketNotifier(fd, QSocketNotifier::Read); // TODO don't leak
    QObject::connect(notifier, &QSocketNotifier::activated, [input=this->input, that=pub()] (int socket) -> void {
        char buff[100];
        int amount = read (socket, buff, 99);
        termpaint_input_add_data(input, buff, amount);
        QString peek = QString::fromUtf8(termpaint_input_peek_buffer(input), termpaint_input_peek_buffer_length(input));
        if (peek.length()) {
            ZRawSequenceEvent event(ZRawSequenceEvent::pending, peek);
            bool ret = QCoreApplication::sendEvent(that, &event);
            //if (!ret) {
                // FIXME
                write(0, "\e[5n", 4);
            //}
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

void ZTerminalPrivate::integration_write(char *data, int length) {
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

void ZTerminalPrivate::integration_flush() {

}

bool ZTerminalPrivate::integration_is_bad() {
    return fd == -1;
}

void ZTerminalPrivate::integration_expect_response() {
    awaiting_response = true;
}


void ZTerminalPrivate::init_fns() {
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
    integration.expect_response = [] (termpaint_integration* ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_expect_response();
    };
}

TUIWIDGETS_NS_END
