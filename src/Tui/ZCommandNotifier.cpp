#include "ZCommandNotifier.h"

#include "ZCommandNotifier_p.h"

#include <Tui/ZCommandManager.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZCommandNotifier::ZCommandNotifier(ZImplicitSymbol command, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZCommandNotifierPrivate>())
{
    auto *const p = tuiwidgets_impl();
    p->command = command;
    probeParents();
}

ZCommandNotifier::~ZCommandNotifier() {
    // Notify ZCommandManager
    setEnabled(false);
}

ZSymbol ZCommandNotifier::command() {
    auto *const p = tuiwidgets_impl();
    return p->command;
}

bool ZCommandNotifier::isEnabled() {
    auto *const p = tuiwidgets_impl();
    return p->enabled;
}

void ZCommandNotifier::setEnabled(bool s) {
    auto *const p = tuiwidgets_impl();
    if (p->enabled == s) {
        return;
    }
    p->enabled = s;
    enabledChanged(s);
}

void ZCommandNotifier::probeParents() {
    ZWidget* w = qobject_cast<ZWidget*>(parent());
    while (w) {
        if (w->commandManager()) {
            w->commandManager()->registerCommandNotifier(this);
        }
        w = w->parentWidget();
    }
}

bool ZCommandNotifier::event(QEvent *event) {
    return QObject::event(event);
}

bool ZCommandNotifier::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZCommandNotifier::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZCommandNotifier::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZCommandNotifier::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZCommandNotifier::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZCommandNotifier::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
