#include <Tui/ZWidget.h>

#include <Tui/ZWidget_p.h>

TUIWIDGETS_NS_START


ZWidgetPrivate::ZWidgetPrivate(ZWidget *pub)
    : pub_ptr(pub)
{

}

ZWidgetPrivate::~ZWidgetPrivate()
{

}

ZWidget::ZWidget(ZWidget *parent) :
    QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZWidgetPrivate>(this))
{
    // ??? maybe delay parenting to after some code here was run?
    // ??? should there be a posted event after creation? Could be useful for two phase init, but could be to late anyway
}

ZWidget::~ZWidget() {
    // TODO if focused make sure focus is not dangling
    // ??? any use in deleting children here manually instead of leaving it to QObject?
}

void ZWidget::setParent(ZWidget *parent) {
    // QEvent::ParentAboutToChange
    // TODO care about focus
    QObject::setParent(parent);
    // TODO care about caches for everything (e.g. visibiltiy, enabled, etc)
    // QEvent::ParentChange
}

bool ZWidget::event(QEvent *event) {
    return QObject::event(event);
}

bool ZWidget::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZWidget::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZWidget::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZWidget::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZWidget::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZWidget::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    QObject::disconnectNotify(signal);
}

TUIWIDGETS_NS_END

