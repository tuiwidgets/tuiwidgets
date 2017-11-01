#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>

#include <QCoreApplication>
#include <Tui/ZEvent.h>
#include <Tui/ZPainter_p.h>
#include <Tui/ZWidget.h>

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
    auto *surface = tuiwidgets_impl()->surface;
    return ZPainter(std::make_unique<ZPainterPrivate>(surface,
                                                            termpaint_surface_width(surface),
                                                            termpaint_surface_height(surface)));
}

ZWidget *ZTerminal::mainWidget() {
    return tuiwidgets_impl()->mainWidget.get();
}

void ZTerminal::setMainWidget(ZWidget *w) {
    if (tuiwidgets_impl()->mainWidget) {
        tuiwidgets_impl()->mainWidget.release();
    }
    tuiwidgets_impl()->mainWidget.reset(w);
    update();
}

void ZTerminal::update() {
    if (tuiwidgets_impl()->updateRequested) {
        return;
    }

    // XXX ZTerminal uses updateRequest with null painter internally
    QCoreApplication::postEvent(this, new ZPaintEvent(ZPaintEvent::update, nullptr), Qt::LowEventPriority);
}

bool ZTerminal::event(QEvent *event) {
    if (event->type() == ZEventType::updateRequest()) {
        // XXX ZTerminal uses updateRequest with null painter internally
        tuiwidgets_impl()->updateRequested = false;
        ZPainter p = painter();
        ZPaintEvent event(ZPaintEvent::update, &p);
        QCoreApplication::sendEvent(tuiwidgets_impl()->mainWidget.get(), &event);
        p.flush();
    }

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
