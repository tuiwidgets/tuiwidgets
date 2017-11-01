#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>

#include <QCoreApplication>
#include <QPointer>

#include <Tui/ZEvent.h>
#include <Tui/ZPainter_p.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>

TUIWIDGETS_NS_START

ZTerminalPrivate::ZTerminalPrivate(ZTerminal *pub) {
    pub_ptr = pub;
}

ZTerminalPrivate::~ZTerminalPrivate() {
    deinitTerminal();
}

ZTerminalPrivate *ZTerminalPrivate::get(ZTerminal *terminal) {
    return terminal->tuiwidgets_impl();
}

void ZTerminalPrivate::setFocus(ZWidget *w) {
    focusWidget = ZWidgetPrivate::get(w);
}

ZWidget *ZTerminalPrivate::focus() {
    return focusWidget ? focusWidget->pub() : nullptr;
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
        ZWidgetPrivate::get(tuiwidgets_impl()->mainWidget.get())->unsetTerminal();
        tuiwidgets_impl()->mainWidget.release();
    }
    tuiwidgets_impl()->mainWidget.reset(w);
    ZWidgetPrivate::get(w)->setManagingTerminal(this);
    update();
}

ZWidget *ZTerminal::focusWidget() {
    return tuiwidgets_impl()->focusWidget->pub();
}

void ZTerminal::update() {
    if (tuiwidgets_impl()->updateRequested) {
        return;
    }

    // XXX ZTerminal uses updateRequest with null painter internally
    QCoreApplication::postEvent(this, new ZPaintEvent(ZPaintEvent::update, nullptr), Qt::LowEventPriority);
}

std::unique_ptr<ZKeyEvent> ZTerminal::translateKeyEvent(const ZTerminalNativeEvent &nativeEvent) {
    termpaint_input_event* native = static_cast<termpaint_input_event*>(nativeEvent.nativeEventPointer());

    Qt::KeyboardModifiers modifiers = 0;

    if (native->modifier & TERMPAINT_MOD_SHIFT) {
        modifiers |= Qt::ShiftModifier;
    }
    if (native->modifier & TERMPAINT_MOD_CTRL) {
        modifiers |= Qt::ControlModifier;
    }
    if (native->modifier & TERMPAINT_MOD_ALT) {
        modifiers |= Qt::AltModifier;
    }

    if (native->type == TERMPAINT_EV_KEY) {
        int key = Qt::Key_unknown;
        if (native->atom_or_string == termpaint_input_i_resync()) {
            return nullptr;
        } else if (native->atom_or_string == termpaint_input_arrow_right()) {
            key = Qt::Key_Right;
        } else if (native->atom_or_string == termpaint_input_arrow_left()) {
            key = Qt::Key_Left;
        } else if (native->atom_or_string == termpaint_input_arrow_down()) {
            key = Qt::Key_Down;
        } else if (native->atom_or_string == termpaint_input_arrow_up()) {
            key = Qt::Key_Up;
        } else if (native->atom_or_string == termpaint_input_tab()) {
            key = Qt::Key_Tab;
        } else if  (native->atom_or_string == termpaint_input_enter()) {
            key = Qt::Key_Enter;
        } else if  (native->atom_or_string == termpaint_input_backspace()) {
            key = Qt::Key_Backspace;
        }
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, QString()) };
    } else if (native->type == TERMPAINT_EV_CHAR) {
        int key = Qt::Key_unknown;
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, QString::fromUtf8(native->atom_or_string, native->length)) };
    }

    return nullptr;
}

bool ZTerminal::event(QEvent *event) {
    if (event->type() == ZEventType::rawSequence()) {
        return false;
    }
    if (event->type() == ZEventType::terminalNativeEvent()) {
        std::unique_ptr<ZKeyEvent> translated = translateKeyEvent(*static_cast<Tui::ZTerminalNativeEvent*>(event));
        if (translated) {
            translated->accept();
            QPointer<ZWidget> w = tuiwidgets_impl()->focus();
            while (w) {
                bool processed = QCoreApplication::sendEvent(w, translated.get());
                if (processed && translated->isAccepted()) {
                    break;
                }
                w = w->parentWidget();
            }
        }
        return true; // ???
    }
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
