#include "ZWindowContainer.h"
#include "ZWindowContainer_p.h"

#include <QVector>

TUIWIDGETS_NS_START

ZWindowContainer::ZWindowContainer() {
}

ZWindowContainer::ZWindowContainer(std::unique_ptr<ZWindowContainerPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZWindowContainer::~ZWindowContainer() {
}

QVector<ZMenuItem> ZWindowContainer::containerMenuItems() const {
    return {};
}

bool ZWindowContainer::event(QEvent *event) {
    return QObject::event(event);
}

bool ZWindowContainer::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZWindowContainer::timerEvent(QTimerEvent *event) {
    return QObject::timerEvent(event);
}

void ZWindowContainer::childEvent(QChildEvent *event) {
    return QObject::childEvent(event);
}

void ZWindowContainer::customEvent(QEvent *event) {
    return QObject::customEvent(event);
}

void ZWindowContainer::connectNotify(const QMetaMethod &signal) {
    return QObject::connectNotify(signal);
}

void ZWindowContainer::disconnectNotify(const QMetaMethod &signal) {
    return QObject::disconnectNotify(signal);
}

ZWindowContainerPrivate::~ZWindowContainerPrivate() = default;

TUIWIDGETS_NS_END
