// SPDX-License-Identifier: BSL-1.0

#include "ZRoot.h"
#include "ZRoot_p.h"

#include <QSize>

#include <Tui/ZCommon.h>
#include <Tui/ZPalette.h>
#include <Tui/ZPainter.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindowFacet.h>

#include <Tui/Utils_p.h>

TUIWIDGETS_NS_START

ZRoot::ZRoot() : ZWidget(nullptr, std::make_unique<ZRootPrivate>(this)) {
    setPalette(ZPalette::classic());
    setMinimumSize(40, 7);
}

ZRoot::~ZRoot() {
}

void ZRoot::setFillChar(int fillChar) {
    auto *const p = tuiwidgets_impl();
    p->fillChar = fillChar;
}

int ZRoot::fillChar() const {
    auto *const p = tuiwidgets_impl();
    return p->fillChar;
}

void ZRoot::activateNextWindow() {
    focusWindowHelper(true);
}

void ZRoot::activatePreviousWindow() {
    focusWindowHelper(false);
}

void ZRoot::raiseOnActivate(ZWidget *w) {
    w->raise();
}

void ZRoot::focusWindowHelper(bool forward) {
    auto *const p = tuiwidgets_impl();
    ZWidget *first = nullptr;
    bool arm = false;
    bool found = false;

    QList<ZWidget*> childWindows;
    for (QObject *obj : p->windows) {
        auto childWidget = qobject_cast<ZWidget*>(obj);
        if (childWidget && childWidget->facet(ZWindowFacet::staticMetaObject) && childWidget->isVisible()) {
            childWindows.append(childWidget);
        }
    }

    if (!forward) {
        std::reverse(childWindows.begin(), childWindows.end());
    }
    for(ZWidget *win : childWindows) {
        if (!first && win->placeFocus()) {
            first = win;
        }
        if (arm) {
            ZWidget *w = win->placeFocus();
            if (w) {
                w->setFocus(FocusReason::ActiveWindowFocusReason);
                raiseOnActivate(win);
                found = true;
                break;
            }
        }
        if (win->isInFocusPath()) {
            arm = true;
        }
    }
    if (!found && first) {
        first->placeFocus()->setFocus(FocusReason::ActiveWindowFocusReason);
        raiseOnActivate(first);
    }
}

void ZRoot::paintEvent(ZPaintEvent *event) {
    auto *painter = event->painter();
    painter->clearWithChar(getColor("root.fg"), getColor("root.bg"), fillChar());
}

void ZRoot::keyEvent(ZKeyEvent *event) {
    if (event->key() == Key_F6 && (event->modifiers() == 0 || event->modifiers() == ShiftModifier)) {
        focusWindowHelper(event->modifiers() != ShiftModifier);
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZRoot::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == ZEventType::otherChange()) {
        if (!static_cast<ZOtherChangeEvent*>(event)->unchanged().contains(TUISYM_LITERAL("terminal"))) {
            if (p->connectedTerminal != terminal()) {
                p->connectedTerminal = terminal();
                terminalChanged();
            }
        }
    }
    return ZWidget::event(event);
}

void ZRoot::resizeEvent(ZResizeEvent *event) {
    const int height = geometry().height();
    const int width = geometry().width();
    for (ZWidget *childWidget: toQPointerListWithCast<ZWidget>(children())) {
        if (!childWidget) continue;
        ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(childWidget->facet(ZWindowFacet::staticMetaObject));
        if (windowFacet) {
            if (!windowFacet->isManuallyPlaced() && !windowFacet->container()) {
                windowFacet->autoPlace({width, height}, childWidget);
                continue;
            }
        }
    }
    ZWidget::resizeEvent(event);
}

bool ZRoot::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

void ZRoot::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

QSize ZRoot::minimumSizeHint() const {
    QSize hint;
    for (QObject *o: children()) {
        auto childWidget = qobject_cast<ZWidget*>(o);
        if (childWidget && childWidget->isVisible()) {
            ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(childWidget->facet(ZWindowFacet::staticMetaObject));
            if (windowFacet) {
                if (windowFacet->isExtendViewport()) {
                    hint = hint.expandedTo(QSize(childWidget->geometry().right() + 1, childWidget->geometry().bottom() + 1));
                }
            }
        }
    }
    return hint;
}

QRect ZRoot::layoutArea() const {
    QRect tmp = geometry();
    if (terminal() && terminal()->mainWidget() == this) {
        // keep base layout inside the area we would get without children with extend viewport enabled.
        QSize terminalSize = { terminal()->width(), terminal()->height() };

        tmp = { {0, 0}, terminalSize.expandedTo(minimumSize()) };
    }
    tmp.moveTo(0, 0);
    tmp = tmp.marginsRemoved(contentsMargins());
    return QRect(tmp.topLeft(), tmp.size().expandedTo({0, 0}));
}

void ZRoot::childEvent(QChildEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->added()) {
        p->windows.append(event->child());
    }
    if (event->removed()) {
        p->windows.removeOne(event->child());
    }
    ZWidget::childEvent(event);
}

void ZRoot::terminalChanged() {
    // for derived classes to override
}

void ZRoot::customEvent(QEvent *event) {
    ZWidget::customEvent(event);
}

void ZRoot::connectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::connectNotify(signal);
}

void ZRoot::disconnectNotify(const QMetaMethod &signal) {
    // XXX needs to be thread-safe
    ZWidget::disconnectNotify(signal);
}

QObject *ZRoot::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

QSize ZRoot::sizeHint() const {
    return ZWidget::sizeHint();
}

ZWidget *ZRoot::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZRoot::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZRoot::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZRoot::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

void ZRoot::pasteEvent(ZPasteEvent *event) {
    ZWidget::pasteEvent(event);
}

ZRootPrivate::ZRootPrivate(ZRoot *pub) : ZWidgetPrivate(pub) {

}

ZRootPrivate::~ZRootPrivate() {
}

TUIWIDGETS_NS_END
