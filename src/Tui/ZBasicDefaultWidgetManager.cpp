#include "ZBasicDefaultWidgetManager.h"
#include "ZBasicDefaultWidgetManager_p.h"

#include <QCoreApplication>

#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZBasicDefaultWidgetManagerPrivate::ZBasicDefaultWidgetManagerPrivate(ZWidget *dialog) : _dialog(dialog) {
}

ZBasicDefaultWidgetManagerPrivate::~ZBasicDefaultWidgetManagerPrivate() {
}

ZBasicDefaultWidgetManager::ZBasicDefaultWidgetManager(ZWidget *dialog)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZBasicDefaultWidgetManagerPrivate>(dialog)) {
}

ZBasicDefaultWidgetManager::~ZBasicDefaultWidgetManager() {
}

void ZBasicDefaultWidgetManager::setDefaultWidget(ZWidget *w) {
    auto *const p = tuiwidgets_impl();
    p->_defaultWidget = w;
}

ZWidget *ZBasicDefaultWidgetManager::defaultWidget() const {
    auto *const p = tuiwidgets_impl();
    return p->_defaultWidget;
}

bool ZBasicDefaultWidgetManager::isDefaultWidgetActive() const {
    auto *const p = tuiwidgets_impl();
    if (!p->_defaultWidget) {
        return false;
    }

    ZTerminal *term = p->_dialog->terminal();
    if (term) {
        ZWidget *w = term->focusWidget();
        if (p->_dialog->isAncestorOf(w) && !p->_defaultWidget->isAncestorOf(w)) {
            while (w != p->_dialog) {
                QEvent e{ZEventType::queryAcceptsEnter()};
                e.ignore();
                QCoreApplication::sendEvent(w, &e);
                if (e.isAccepted()) {
                    return false;
                }
                w = w->parentWidget();
            }
        }
    }
    return true;
}

bool ZBasicDefaultWidgetManager::event(QEvent *event) {
    return ZDefaultWidgetManager::event(event);
}

bool ZBasicDefaultWidgetManager::eventFilter(QObject *watched, QEvent *event) {
    return ZDefaultWidgetManager::eventFilter(watched, event);
}

void ZBasicDefaultWidgetManager::timerEvent(QTimerEvent *event) {
    return ZDefaultWidgetManager::timerEvent(event);
}

void ZBasicDefaultWidgetManager::childEvent(QChildEvent *event) {
    return ZDefaultWidgetManager::childEvent(event);
}

void ZBasicDefaultWidgetManager::customEvent(QEvent *event) {
    return ZDefaultWidgetManager::customEvent(event);
}

void ZBasicDefaultWidgetManager::connectNotify(const QMetaMethod &signal) {
    return ZDefaultWidgetManager::connectNotify(signal);
}

void ZBasicDefaultWidgetManager::disconnectNotify(const QMetaMethod &signal) {
    return ZDefaultWidgetManager::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
