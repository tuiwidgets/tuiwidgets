#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>

#include <Tui/ZPainter_p.h>

TUIWIDGETS_NS_START

ZTerminalPrivate::ZTerminalPrivate(ZTerminal *pub) {
    pub_ptr = pub;
}

ZTerminalPrivate::~ZTerminalPrivate() {
    deinitTerminal();
}

ZTerminal::ZTerminal(QObject *parent) :
    QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this))
{
    tuiwidgets_impl()->initTerminal();
}


bool ZTerminalPrivate::initTerminal() {
    return setup();
}

ZTerminal::~ZTerminal() {
}

ZPainter ZTerminal::painter() {
    return ZPainter(std::make_unique<ZPainterPrivate>(tuiwidgets_impl()->surface, 80, 25)); // TODO get size from somewhere
}

bool ZTerminal::event(QEvent *event) {
    return QObject::event(event);
}

bool ZTerminal::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZTerminal::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZTerminal::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZTerminal::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZTerminal::connectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZTerminal::disconnectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe

}


TUIWIDGETS_NS_END
