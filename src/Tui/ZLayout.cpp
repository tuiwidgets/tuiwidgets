#include "ZLayout.h"
#include "ZLayout_p.h"

#include <QEvent>

TUIWIDGETS_NS_START

ZLayout::ZLayout(QObject *parent) : QObject(parent)
{

}

ZLayout::~ZLayout() {
}

void ZLayout::widgetEvent(QEvent *event) {
    /*if (event->type() == ZEventType::resize()) {
        resizeEvent(static_cast<ZResizeEvent*>(event));
    }*/
}

bool ZLayout::event(QEvent *event) {
    return QObject::event(event);
}

bool ZLayout::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZLayout::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZLayout::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZLayout::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZLayout::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZLayout::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
