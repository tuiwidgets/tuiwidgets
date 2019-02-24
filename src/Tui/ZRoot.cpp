#include "ZRoot.h"

#include <QSize>

#include <Tui/ZPalette.h>
#include <Tui/ZPainter.h>

TUIWIDGETS_NS_START

ZRoot::ZRoot() {
    setPalette(ZPalette::classic());
}

void ZRoot::paintEvent(ZPaintEvent *event) {
    auto *painter = event->painter();
    painter->clear(getColor("root.fg"), getColor("root.bg"));
}

void ZRoot::keyEvent(ZKeyEvent *event) {
    if (event->key() == Qt::Key_F6 && (event->modifiers() == 0 || event->modifiers() == Qt::Modifier::SHIFT)) {
        ZWidget *first = nullptr;
        bool arm = false;
        bool found = false;
        QList<ZWidget*> childWindows = findChildren<ZWidget*>(QStringLiteral(""), Qt::FindDirectChildrenOnly);
        QMutableListIterator<ZWidget*> it(childWindows);
        while (it.hasNext()) {
            ZWidget* w = it.next();
            if (!w->paletteClass().contains(QStringLiteral("window"))) {
                it.remove();
            }
            if (!w->isVisible()) {
                it.remove();
            }
        }
        if (event->modifiers() == Qt::Modifier::SHIFT) {
            std::reverse(childWindows.begin(), childWindows.end());
        }
        for(ZWidget *win : childWindows) {
            if (!first) {
                first = win->placeFocus();
            }
            if (arm) {
                ZWidget *w = win->placeFocus();
                if (w) {
                    w->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
                    found = true;
                    break;
                }
            }
            if (win->isInFocusPath()) {
                arm = true;
            }
        }
        if (!found && first) {
            first->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
        }
    }
}

bool ZRoot::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZRoot::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

void ZRoot::timerEvent(QTimerEvent *event) {
    ZWidget::timerEvent(event);
}

void ZRoot::childEvent(QChildEvent *event) {
    ZWidget::childEvent(event);
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

QSize ZRoot::sizeHint() const {
    return ZWidget::sizeHint();
}

QObject *ZRoot::facet(const QMetaObject metaObject) {
    return ZWidget::facet(metaObject);
}

void ZRoot::focusInEvent(ZFocusEvent *event) {
    ZWidget::focusInEvent(event);
}

void ZRoot::focusOutEvent(ZFocusEvent *event) {
    ZWidget::focusOutEvent(event);
}

void ZRoot::resizeEvent(ZResizeEvent *event) {
    ZWidget::resizeEvent(event);
}

void ZRoot::moveEvent(ZMoveEvent *event) {
    ZWidget::moveEvent(event);
}

TUIWIDGETS_NS_END
