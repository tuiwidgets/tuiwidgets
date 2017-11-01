#include <Tui/ZWidget.h>

#include <Tui/ZWidget_p.h>

#include <QCoreApplication>

#include <Tui/ZPainter.h>
#include <Tui/ZTerminal_p.h>

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

QRect ZWidget::geometry() const {
    return tuiwidgets_impl()->rect;
}

void ZWidget::setGeometry(const QRect &rect) {
    auto *const p = tuiwidgets_impl();
    QRect oldRect = p->rect;
    p->rect = rect;
    if (oldRect.topLeft() != rect.topLeft()) {
        ZMoveEvent e {rect.topLeft(), oldRect.topLeft()};
        QCoreApplication::sendEvent(this, &e);
    }
    if (oldRect.size() != rect.size()) {
        ZResizeEvent e {rect.size(), oldRect.size()};
        QCoreApplication::sendEvent(this, &e);
    }
}

bool ZWidget::isVisible() const {
    return tuiwidgets_impl()->visible;

}

void ZWidget::setVisible(bool v) {
    auto *const p = tuiwidgets_impl();
    p->visible = v;
    // TODO care about focus
    // TODO cache effect in hierarchy
    // TODO send events (QShowEvent  QHideEvent? QEvent::HideToParent? QEvent::ShowToParent?)
    // TODO trigger repaint (Qt does not use events here)
}

ZTerminal *ZWidget::terminal() {
    return tuiwidgets_impl()->findTerminal();
}

void ZWidget::update() {
    auto *terminal = tuiwidgets_impl()->findTerminal();
    if (terminal) terminal->update();
}

bool ZWidget::isAncestorOf(const ZWidget *child) const {
    while (child) {
        if (child == this) {
            return true;
        }
        child = child->parentWidget();
    }
    return true;
}

bool ZWidget::isVisibleTo(const ZWidget *ancestor) const {
    const ZWidget *w = this;
    while (w) {
        if (w == ancestor) {
            return true;
        }
        if (!w->isVisible()) {
            return false;
        }
        w = w->parentWidget();
    }
    return false;
}

bool ZWidget::event(QEvent *event) {
    if (event->type() == ZEventType::paint()) {
        paintEvent(static_cast<ZPaintEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::updateRequest()) {
        tuiwidgets_impl()->updateRequestEvent(static_cast<ZPaintEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::resize()) {
        resizeEvent(static_cast<ZResizeEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::move()) {
        moveEvent(static_cast<ZMoveEvent*>(event));
        return true;
    } else {
        return QObject::event(event);
    }
}

bool ZWidget::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZWidgetPrivate::updateRequestEvent(ZPaintEvent *event)
{
    auto *painter = event->painter();
    // TODO think about mitigations about childs beeing deleted while in paint event
    {
        Tui::ZPaintEvent nestedEvent(painter);
        QCoreApplication::instance()->sendEvent(pub(), &nestedEvent);
    }
    for (QObject *childQObj : pub()->children()) {
        ZWidget* child = qobject_cast<ZWidget*>(childQObj);
        if (!child) {
            continue;
        }
        const QRect &childRect = child->tuiwidgets_impl()->rect;
        ZPainter transformedPainter = painter->translateAndClip(childRect);
        Tui::ZPaintEvent nestedEvent(ZPaintEvent::update, &transformedPainter);
        QCoreApplication::instance()->sendEvent(child, &nestedEvent);
    }
}

ZTerminal *ZWidgetPrivate::findTerminal() const {
    ZWidget const *w = pub();
    while (w) {
        if (w->tuiwidgets_impl()->terminal) {
            return w->tuiwidgets_impl()->terminal;
        }
        w = w->parentWidget();
    };
    return nullptr;
}

void ZWidgetPrivate::unsetTerminal() {
    terminal = nullptr;
}

void ZWidgetPrivate::setManagingTerminal(ZTerminal *terminal) {
    this->terminal = terminal;
}

void ZWidget::paintEvent(ZPaintEvent *event) {
    Q_UNUSED(event);
}

void ZWidget::resizeEvent(ZResizeEvent *event)
{
}

void ZWidget::moveEvent(ZMoveEvent *event)
{
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

