#include "ZLayout.h"
#include "ZLayout_p.h"

#include <QEvent>
#include <QPoint>
#include <QRect>
#include <QSize>

#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZLayout::ZLayout(QObject *parent) : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZLayoutPrivate>())
{
}

ZLayout::~ZLayout() {
}

void ZLayout::widgetEvent(QEvent *event) {
    if (event->type() == ZEventType::resize()) {
        setGeometry(widget()->layoutArea());
    }
}

ZWidget *ZLayout::widget() {
    QObject *p = parent();
    while (p) {
        ZWidget *w = qobject_cast<ZWidget*>(p);
        if (w) return w;
        p = p->parent();
    }
    return nullptr;
}

QSize ZLayout::sizeHint() const {
    return QSize();
}

SizePolicy ZLayout::sizePolicyH() const {
    return SizePolicy::Preferred;
}

SizePolicy ZLayout::sizePolicyV() const {
    return SizePolicy::Preferred;
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
