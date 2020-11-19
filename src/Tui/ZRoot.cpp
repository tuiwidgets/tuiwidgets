#include "ZRoot.h"
#include "ZRoot_p.h"

#include <QSize>

#include <Tui/ZPalette.h>
#include <Tui/ZPainter.h>

TUIWIDGETS_NS_START

ZRoot::ZRoot() : ZWidget(nullptr, std::make_unique<ZRootPrivate>(this)) {
    setPalette(ZPalette::classic());
    setMinimumSize(40, 7);
}

void ZRoot::setFillChar(int fillChar) {
    auto *const p = tuiwidgets_impl();
    p->fillChar = fillChar;
}

int ZRoot::fillChar() {
    auto *const p = tuiwidgets_impl();
    return p->fillChar;
}

void ZRoot::paintEvent(ZPaintEvent *event) {
    auto *painter = event->painter();
    painter->clear(getColor("root.fg"), getColor("root.bg"), fillChar());
}

void ZRoot::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->key() == Qt::Key_F6 && (event->modifiers() == 0 || event->modifiers() == Qt::Modifier::SHIFT)) {
        ZWidget *first = nullptr;
        bool arm = false;
        bool found = false;

        QList<ZWidget*> childWindows;
        for (QObject* obj : p->windows) {
            auto childWidget = qobject_cast<ZWidget*>(obj);
            if (childWidget && childWidget->paletteClass().contains(QStringLiteral("window")) && childWidget->isVisible()) {
                childWindows.append(childWidget);
            }
        }

        if (event->modifiers() == Qt::Modifier::SHIFT) {
            std::reverse(childWindows.begin(), childWindows.end());
        }
        for(ZWidget *win : childWindows) {
            if (!first && win->placeFocus()) {
                first = win;
            }
            if (arm) {
                ZWidget *w = win->placeFocus();
                if (w) {
                    w->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
                    if(win->paletteClass().contains(QStringLiteral("dialog"))) {
                        win->raise();
                    }
                    found = true;
                    break;
                }
            }
            if (win->isInFocusPath()) {
                arm = true;
            }
        }
        if (!found && first) {
            first->placeFocus()->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
            if(first->paletteClass().contains(QStringLiteral("dialog"))) {
                first->raise();
            }
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
    auto *const p = tuiwidgets_impl();
    if (event->added()) {
        p->windows.prepend(event->child());
    }
    if (event->removed()) {
        p->windows.removeOne(event->child());
    }
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

QObject *ZRoot::facet(const QMetaObject metaObject) {
    return ZWidget::facet(metaObject);
}

QRect ZRoot::layoutArea() const {
    return ZWidget::layoutArea();
}

QSize ZRoot::sizeHint() const {
    return ZWidget::sizeHint();
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

void ZRoot::resizeEvent(ZResizeEvent *event) {
    ZWidget::resizeEvent(event);
}

Tui::ZRootPrivate::ZRootPrivate(ZRoot *pub) : ZWidgetPrivate(pub) {

}

TUIWIDGETS_NS_END
