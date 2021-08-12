#include "ZCommandManager.h"

#include "ZCommandManager_p.h"

#include <Tui/ZCommandNotifier.h>

TUIWIDGETS_NS_START

ZCommandManager::ZCommandManager(QObject *parent) :
    QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZCommandManagerPrivate>())
{

}

ZCommandManager::~ZCommandManager() {

}

void ZCommandManager::registerCommandNotifier(ZCommandNotifier *notifier) {
    auto *const p = tuiwidgets_impl();
    p->commandNotifiers[notifier->command()].append(notifier);
    connect(notifier, &ZCommandNotifier::enabledChanged, this, &ZCommandManager::_tui_enabledChanged);
}

void ZCommandManager::_tui_enabledChanged(bool s) {
    (void)s;
    ZSymbol command = static_cast<ZCommandNotifier*>(sender())->command();
    compact(command);
    commandStateChanged(command);
}

void ZCommandManager::compact(ZSymbol command) {
    auto *const p = tuiwidgets_impl();
    if (!p->commandNotifiers.contains(command)) {
        return;
    }
    QMutableVectorIterator<QPointer<ZCommandNotifier>> it(p->commandNotifiers[command]);
    while (it.hasNext()) {
        auto &ptr = it.next();
        if (ptr.isNull()) {
            it.remove();
        }
    }
    if (p->commandNotifiers[command].isEmpty()) {
        p->commandNotifiers.remove(command);
    }
}

bool ZCommandManager::isCommandEnabled(ZSymbol command) {
    auto *const p = tuiwidgets_impl();
    bool enabled = false;
    if (p->commandNotifiers.contains(command)) {
        for (const auto &notifier : p->commandNotifiers[command]) {
            if (!notifier.isNull()) {
                if (notifier->isEnabled() && notifier->isContextSatisfied()) {
                    enabled = true;
                    break;
                }
            }
        }
    }
    return enabled;
}

void ZCommandManager::activateCommand(ZSymbol command) {
    auto *const p = tuiwidgets_impl();
    if (p->commandNotifiers.contains(command)) {
        for (const auto &notifier : p->commandNotifiers[command]) {
            if (!notifier.isNull()) {
                if (notifier->isEnabled() && notifier->isContextSatisfied()) {
                    notifier->activated();
                    break;
                }
            }
        }
    }
}

bool ZCommandManager::event(QEvent *event) {
    return QObject::event(event);
}

bool ZCommandManager::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZCommandManager::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZCommandManager::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZCommandManager::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZCommandManager::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZCommandManager::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}


TUIWIDGETS_NS_END
