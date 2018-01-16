#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>

#include <limits>

#include <QPointer>
#include <QCoreApplication>

#include <Tui/ZCommandManager.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
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
    update();
}

bool ZWidget::isEnabled() const {
    return tuiwidgets_impl()->enabled;
}

void ZWidget::setEnabled(bool e) {
    auto *const p = tuiwidgets_impl();
    p->enabled = e;
    // TODO care about focus
    // TODO cache effect in hierarchy
    // TODO send events (QEvent::EnabledChange ? )
    // TODO trigger repaint (qt does this in the specific virtual for the event)
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

void ZWidget::showCursor(QPoint position) {
    ZTerminal *term = terminal();
    if (term && term->focusWidget() == this) {
        ZTerminalPrivate *termp = ZTerminalPrivate::get(term);
        if (position.x() >= 0 && position.x() < geometry().width()
         && position.y() >= 0 && position.y() < geometry().height()) {
            ZWidget *w = this;
            while (w) {
                position += w->geometry().topLeft();
                w = w->parentWidget();
            }
            termp->cursorPosition = position;
        } else {
            termp->cursorPosition = QPoint{-1, -1};
        }
    }
}

ZTerminal *ZWidget::terminal() const {
    return tuiwidgets_impl()->findTerminal();
}

void ZWidget::update() {
    auto *terminal = tuiwidgets_impl()->findTerminal();
    if (terminal) terminal->update();
}

void ZWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    auto *const p = tuiwidgets_impl();
    p->focusPolicy = policy;
}

Qt::FocusPolicy ZWidget::focusPolicy() const {
    auto *const p = tuiwidgets_impl();
    return p->focusPolicy;
}

void ZWidget::setFocusMode(FocusContainerMode mode) {
    auto *const p = tuiwidgets_impl();
    p->focusMode = mode;
}

FocusContainerMode ZWidget::focusMode() const {
    auto *const p = tuiwidgets_impl();
    return p->focusMode;
}

void ZWidget::setFocusOrder(int order) {
    auto *const p = tuiwidgets_impl();
    p->focusOrder = order;
}

int ZWidget::focusOrder() const {
    auto *const p = tuiwidgets_impl();
    return p->focusOrder;
}

const ZPalette &ZWidget::palette() const {
    auto *const p = tuiwidgets_impl();
    return p->palette;
}

void ZWidget::setPalette(const ZPalette &pal) {
    auto *const p = tuiwidgets_impl();
    p->palette = pal;
}

ZColor ZWidget::getColor(const ZImplicitSymbol &x) {
    auto *const p = tuiwidgets_impl();

    return p->palette.getColor(this, x);
}

QStringList ZWidget::paletteClass() {
    auto *const p = tuiwidgets_impl();
    return p->paletteClass;
}

void ZWidget::setPaletteClass(QStringList classes) {
    auto *const p = tuiwidgets_impl();
    classes.removeDuplicates();
    // TODO some event
    p->paletteClass = classes;
}

void ZWidget::addPaletteClass(QString clazz) {
    QStringList cls = paletteClass();
    cls.append(clazz);
    setPaletteClass(cls);
}

void ZWidget::removePaletteClass(QString clazz) {
    QStringList cls = paletteClass();
    cls.removeAll(clazz);
    setPaletteClass(cls);
}

void ZWidget::setFocus(Qt::FocusReason reason) {
    ZTerminalPrivate *termp = ZTerminalPrivate::get(tuiwidgets_impl()->findTerminal());
    QPointer<ZWidget> previousFocus = termp->focus();
    if (this == previousFocus) {
        return;
    }
    if (previousFocus) {
        ZFocusEvent e {ZFocusEvent::focusOut, reason};
        QCoreApplication::sendEvent(previousFocus, &e);
    }
    termp->setFocus(this);
    {
        ZFocusEvent e {ZFocusEvent::focusIn, reason};
        QCoreApplication::sendEvent(this, &e);
    }
    // TODO trigger repaint(qt does this in the specific virtual for the event)
}

void ZWidget::grabKeyboard() {
    ZTerminalPrivate *termp = ZTerminalPrivate::get(tuiwidgets_impl()->findTerminal());
    termp->setKeyboardGrab(this);
}

void ZWidget::releaseKeyboard() {
    ZTerminalPrivate *termp = ZTerminalPrivate::get(tuiwidgets_impl()->findTerminal());
    if (termp->keyboardGrab() == this) {
        termp->setKeyboardGrab(nullptr);
    }
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

bool ZWidget::isEnabledTo(const ZWidget *ancestor) const {
    const ZWidget *w = this;
    while (w) {
        if (w == ancestor) {
            return true;
        }
        if (!w->isEnabled()) {
            return false;
        }
        w = w->parentWidget();
    }
    return false;
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

bool ZWidget::focus() const {
    return terminal() ? (terminal()->focusWidget() == this) : false;
}

bool ZWidget::isInFocusPath() const {
    if (!terminal()) {
        return false;
    }
    ZWidget *w = terminal()->focusWidget();
    while (w) {
        if (w == this) {
            return true;
        }
        w = w->parentWidget();
    }
    return false;
}

/*
    class FocusSearchIterator {
    public:
        FocusSearchIterator(ZWidget *w) : w(w), start(w) {}

        void nextInSub() {
            QObjectList children;
            children = w->children();
            if (children.size()) {
                ZWidget* tmp = nullptr;
                for (int i = 0; i < children.size(); i++) {
                    tmp = qobject_cast<ZWidget*>(children[i]);
                    if (tmp) {
                        w = tmp;
                        return;
                    }
                }
            }

            ZWidget *p = w;
            while (p->parentWidget()) {
                p = p->parentWidget();
                children = p->children();

                int i = 0;
                for (; i < children.size(); i++) {
                    if (children[i] == w) {
                        ++i;
                        break;
                    }
                }
                ZWidget* tmp = nullptr;
                for (; i < children.size(); i++) {
                    tmp = qobject_cast<ZWidget*>(children[i]);
                    if (tmp) {
                        w = tmp;
                        return;
                    }
                }
                w = p;
            }
        }

        bool groupCompleted() {
            return true;
        }

        ZWidget *w;
        ZWidget *start;
    };
*/

namespace {
    template <typename Func>
    void forTree(ZWidget *w, Func func) {
        func(w);
        const QObjectList &children = w->children();
        for (int i = 0; i < children.size(); i++) {
            ZWidget* tmp = qobject_cast<ZWidget*>(children[i]);
            if (!tmp) continue;
            forTree(tmp, func);
        }
    }

    template <typename Func>
    void forFocusTree(const ZWidget *w, Func func) {
        func(w, false);
        const QObjectList &children = w->children();
        for (int i = 0; i < children.size(); i++) {
            ZWidget* tmp = qobject_cast<ZWidget*>(children[i]);
            if (!tmp) continue;
            if (tmp->focusMode() != FocusContainerMode::None) {
                func(tmp, true);
            }
            forFocusTree(tmp, func);
        }
    }

    const ZWidget *focusTreeRoot(const ZWidget *w) {
        while (w->parentWidget() && w->focusMode() == FocusContainerMode::None) {
            w = w->parentWidget();
        }
        return w;
    }

    bool canFocusAndUpdateTarget(ZWidget const *&c, bool subTree, bool last) {
        if (!subTree) {
            if (!ZWidgetPrivate::get(c)->isFocusable()) {
                return false;
            }
        } else {
            // focus probe for sub ordering
            c = c->placeFocus(last);
            if (!c) return false;
        }
        return true;
    }

    const ZWidget *searchBackwardInGroup(const ZWidget* currentFocus, const ZWidget *currentTree, FocusContainerMode focusMode) {

        unsigned int position = 0;

        const qint64 currentFocusOrder = (static_cast<qint64>(currentFocus->focusOrder()) << 32)
                + 0x80000000u;

        qint64 lowerFocusOrder = std::numeric_limits<qint64>::min();
        const ZWidget *lowerFocusWidget = nullptr;
        qint64 highestFocusOrder = std::numeric_limits<qint64>::min();
        const ZWidget *highestFocusWidget = nullptr;

        forFocusTree(currentTree, [&] (const ZWidget *c, bool subTree) {
            if (c == currentFocus) {
                position = 0x80000000u + 1;
                return;
            }
            const qint64 focusOrder = (static_cast<qint64>(c->focusOrder()) << 32)
                    + position;
            ++position;
            if (!canFocusAndUpdateTarget(c, subTree, false)) {
                return;
            }

            if (focusOrder < currentFocusOrder && focusOrder > lowerFocusOrder) {
                lowerFocusOrder = focusOrder;
                lowerFocusWidget = c;
            }
            if (focusOrder > highestFocusOrder) {
                highestFocusOrder = focusOrder;
                highestFocusWidget = c;
            }
        });

        if (lowerFocusWidget) {
            return lowerFocusWidget;
        }
        if (focusMode == FocusContainerMode::SubOrdering) {
            // back out of sub ordering
            if (currentTree->parentWidget()) {
                const ZWidget *parentTree = focusTreeRoot(currentTree->parentWidget());
                if (parentTree->parentWidget()) {
                    return searchBackwardInGroup(currentTree, parentTree, currentTree->focusMode());
                } else {
                    return searchBackwardInGroup(currentTree, parentTree, FocusContainerMode::Cycle);
                }
            }
        }
        if (highestFocusWidget && highestFocusOrder > currentFocusOrder) {
            return highestFocusWidget;
        }
        return currentFocus;
    }

    ZWidget const *searchForwardInGroup(const ZWidget* currentFocus,
                     const ZWidget *currentTree, FocusContainerMode focusMode) {

        unsigned int position = 0;

        const qint64 currentFocusOrder = (static_cast<qint64>(currentFocus->focusOrder()) << 32)
                + 0x80000000u;

        qint64 higherFocusOrder = std::numeric_limits<qint64>::max();
        ZWidget const *higherFocusWidget = nullptr;
        qint64 lowestFocusOrder = std::numeric_limits<qint64>::max();
        ZWidget const *lowestFocusWidget = nullptr;

        forFocusTree(currentTree, [&] (ZWidget const *c, bool subTree) {
            if (c == currentFocus) {
                position = 0x80000000u + 1;
                return;
            }
            const qint64 focusOrder = (static_cast<qint64>(c->focusOrder()) << 32)
                    + position;
            ++position;
            if (!canFocusAndUpdateTarget(c, subTree, false)) {
                return;
            }

            if (focusOrder > currentFocusOrder && focusOrder < higherFocusOrder) {
                higherFocusOrder = focusOrder;
                higherFocusWidget = c;
            }
            if (focusOrder < lowestFocusOrder) {
                lowestFocusOrder = focusOrder;
                lowestFocusWidget = c;
            }

        });

        if (higherFocusWidget) {
            return higherFocusWidget;
        }
        if (focusMode == FocusContainerMode::SubOrdering) {
            // back out of sub ordering
            if (currentTree->parentWidget()) {
                ZWidget const *parentTree = focusTreeRoot(currentTree->parentWidget());

                if (parentTree->parentWidget()) {
                    return searchForwardInGroup(currentTree, parentTree, currentTree->focusMode());
                } else {
                    return searchForwardInGroup(currentTree, parentTree, FocusContainerMode::Cycle);
                }
            }
        }
        if (lowestFocusWidget && lowestFocusOrder < currentFocusOrder) {
            return lowestFocusWidget;
        }
        return currentFocus;
    }
}

ZWidget const *ZWidget::prevFocusable() const {
    const ZWidget *currentTree = focusTreeRoot(this);
    FocusContainerMode focusMode = currentTree->focusMode();

    if (!currentTree->parentWidget()) {
        focusMode = FocusContainerMode::Cycle;
    }

    if (focusMode == FocusContainerMode::Cycle || focusMode == FocusContainerMode::SubOrdering) {
        return searchBackwardInGroup(this, currentTree, focusMode);
    } else {
        Q_UNREACHABLE();
    }
}

ZWidget *ZWidget::prevFocusable() {
    return const_cast<ZWidget *>(static_cast<ZWidget const *>(this)->prevFocusable());
}


ZWidget const *ZWidget::nextFocusable() const {
    const ZWidget *currentTree = focusTreeRoot(this);
    FocusContainerMode focusMode = currentTree->focusMode();

    if (!currentTree->parentWidget()) {
        focusMode = FocusContainerMode::Cycle;
    }

    if (focusMode == FocusContainerMode::Cycle || focusMode == FocusContainerMode::SubOrdering) {
        return searchForwardInGroup(this, currentTree, focusMode);
    } else {
        Q_UNREACHABLE();
    }
}

ZWidget *ZWidget::nextFocusable() {
    return const_cast<ZWidget *>(static_cast<ZWidget const *>(this)->nextFocusable());
}

ZWidget const *ZWidget::placeFocus(bool last) const {
    int bestFocusOrder = last ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    ZWidget const *bestFocusWidget = nullptr;

    forFocusTree(this, [&] (ZWidget const *c, bool subTree) {
        const int focusOrder = ZWidgetPrivate::get(c)->focusOrder;
        if (last) {
            if (focusOrder <= bestFocusOrder) {
                return;
            }
        } else {
            if (focusOrder >= bestFocusOrder) {
                return;
            }
        }
        if (!canFocusAndUpdateTarget(c, subTree, last)) {
            return;
        }
        bestFocusOrder = focusOrder;
        bestFocusWidget = c;
    });
    return bestFocusWidget;
}

ZWidget *ZWidget::placeFocus(bool last) {
    return const_cast<ZWidget *>(static_cast<ZWidget const *>(this)->placeFocus(last));
}

/*
ZWidget *ZWidget::nextFocusable(bool outside) {
    auto *const p = tuiwidgets_impl();

    const int currentFocusOrder = p->focusOrder;

    ZWidget *currentTree = focusTreeRoot(this);
    if (currentTree == this && outside) {
        if (this->parentWidget()) {
            currentTree = focusTreeRoot(this->parentWidget());
        } else {
            return this->placeFocus();
        }
    }
    FocusContainerMode focusMode = ZWidgetPrivate::get(currentTree)->focusMode;

    if (focusMode == FocusContainerMode::Cycle || focusMode == FocusContainerMode::SubOrdering) {
        struct {
            int higherFocusOrder = std::numeric_limits<int>::max();
            ZWidget *higherFocusWidget = nullptr;
            int lowestFocusOrder = std::numeric_limits<int>::max();
            ZWidget *lowestFocusWidget = nullptr;
        } before, after;

        bool crossed = false;

        forFocusTree(currentTree, [&] (ZWidget *c, bool subTree) {
            if (c == this) {
                crossed = true;
                return;
            }
            auto updateHalf = [&](auto &half) {
                const int focusOrder = ZWidgetPrivate::get(c)->focusOrder;
                if (!canFocusAndUpdateTarget(c, subTree, false)) {
                    return;
                }

                if (focusOrder >= currentFocusOrder && focusOrder < half.higherFocusOrder) {
                    half.higherFocusOrder = focusOrder;
                    half.higherFocusWidget = c;
                }
                if (focusOrder < currentFocusOrder && focusOrder < half.lowestFocusOrder) {
                    half.lowestFocusOrder = focusOrder;
                    half.lowestFocusWidget = c;
                }
            };

            if (crossed) {
                updateHalf(after);
            } else {
                updateHalf(before);
            }
        });

        if (before.higherFocusOrder < after.higherFocusOrder) {
            return before.higherFocusWidget;
        }
        if (after.higherFocusWidget) {
            return after.higherFocusWidget;
        }
        if (focusMode == FocusContainerMode::SubOrdering) {
            // back out of sub ordering
            return currentTree->nextFocusable(true);
        }
        if (before.lowestFocusOrder < after.lowestFocusOrder) {
            return before.lowestFocusWidget;
        }
        if (after.lowestFocusWidget) {
            return after.lowestFocusWidget;
        }
        return this;
    } else {
        Q_UNREACHABLE();
    }
    return nullptr;
}
*/


/*ZWidget *ZWidget::placeFocus(bool last) {
    if (last) {
        int highestFocusOrder = std::numeric_limits<int>::min();
        ZWidget *highestFocusWidget = nullptr;

        forFocusTree(this, [&] (ZWidget *c, bool subTree) {
            const int focusOrder = ZWidgetPrivate::get(c)->focusOrder;
            if (focusOrder <= highestFocusOrder) {
                return;
            }
            if (!canFocusAndUpdateTarget(c, subTree, last)) {
                return;
            }
            highestFocusOrder = focusOrder;
            highestFocusWidget = c;
        });
        return highestFocusWidget;
    } else {
        int lowestFocusOrder = std::numeric_limits<int>::max();
        ZWidget *lowestFocusWidget = nullptr;

        forFocusTree(this, [&] (ZWidget *c, bool subTree) {
            const int focusOrder = ZWidgetPrivate::get(c)->focusOrder;
            if (focusOrder >= lowestFocusOrder) {
                return;
            }
            if (!canFocusAndUpdateTarget(c, subTree, last)) {
                return;
            }
            lowestFocusOrder = focusOrder;
            lowestFocusWidget = c;
        });
        return lowestFocusWidget;
    }
}*/

bool ZWidget::event(QEvent *event) {
    if (event->type() == ZEventType::paint()) {
        paintEvent(static_cast<ZPaintEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::key()) {
        keyEvent(static_cast<ZKeyEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::updateRequest()) {
        tuiwidgets_impl()->updateRequestEvent(static_cast<ZPaintEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::focusIn()) {
        focusInEvent(static_cast<ZFocusEvent*>(event));
        return true;
    } else if (event->type() == ZEventType::focusOut()) {
        focusOutEvent(static_cast<ZFocusEvent*>(event));
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

void ZWidget::keyEvent(ZKeyEvent *event) {
    event->ignore();
}

void ZWidget::focusInEvent(ZFocusEvent *event) {
    if (focusPolicy() != Qt::NoFocus) {
        update();
    }
}

void ZWidget::focusOutEvent(ZFocusEvent *event) {
    if (focusPolicy() != Qt::NoFocus) {
        update();
    }
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

ZCommandManager *ZWidget::commandManager() {
    auto *const p = tuiwidgets_impl();
    return p->commandManager;
}

ZCommandManager *ZWidget::ensureCommandManager() {
    auto *const p = tuiwidgets_impl();
    if (!p->commandManager) {
        p->commandManager = new ZCommandManager(this);
    }
    return p->commandManager;
}

void ZWidget::setCommandManager(ZCommandManager *cmd) {
    auto *const p = tuiwidgets_impl();
    p->commandManager = cmd;
}

TUIWIDGETS_NS_END

