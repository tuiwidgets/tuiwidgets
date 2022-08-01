#include "ZDefaultWidgetManager.h"

TUIWIDGETS_NS_START

ZDefaultWidgetManager::ZDefaultWidgetManager() {
}

ZDefaultWidgetManager::~ZDefaultWidgetManager() = default;

bool ZDefaultWidgetManager::event(QEvent *event) {
    return QObject::event(event);
}

bool ZDefaultWidgetManager::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZDefaultWidgetManager::timerEvent(QTimerEvent *event) {
    return QObject::timerEvent(event);
}

void ZDefaultWidgetManager::childEvent(QChildEvent *event) {
    return QObject::childEvent(event);
}

void ZDefaultWidgetManager::customEvent(QEvent *event) {
    return QObject::customEvent(event);
}

void ZDefaultWidgetManager::connectNotify(const QMetaMethod &signal) {
    return QObject::connectNotify(signal);
}

void ZDefaultWidgetManager::disconnectNotify(const QMetaMethod &signal) {
    return QObject::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
