// SPDX-License-Identifier: BSL-1.0

#include "ZWindowLayout.h"
#include "ZWindowLayout_p.h"

#include <Tui/ZWindow.h>
#include <Tui/ZWindowFacet.h>

#include "Layout_p.h"

TUIWIDGETS_NS_START

using Private::placeWidgetInCell;

ZWindowLayoutPrivate::ZWindowLayoutPrivate() {
}

ZWindowLayout::ZWindowLayout() : ZLayout(nullptr, std::make_unique<ZWindowLayoutPrivate>())
{
}

ZWindowLayout::~ZWindowLayout() {
}

void ZWindowLayout::setCentralWidget(ZWidget *w) {
    auto *const p = tuiwidgets_impl();
    p->m_centralItem.reset(ZLayoutItem::wrapWidget(w).release());
}

void ZWindowLayout::setCentral(ZLayoutItem *li) {
    auto *const p = tuiwidgets_impl();
    if (li->layout()) {
        li->layout()->setParent(this);
    }
    p->m_centralItem.reset(li);
    relayout();
}

void ZWindowLayout::setRightBorderWidget(ZWidget *w, Alignment align) {
    auto *const p = tuiwidgets_impl();
    p->m_rightBorder.reset(ZLayoutItem::wrapWidget(w).release());
    p->m_rightBorderAlign = align;
    relayout();
}

void ZWindowLayout::setBottomBorderWidget(ZWidget *w, Alignment align) {
    auto *const p = tuiwidgets_impl();
    p->m_bottomBorder.reset(ZLayoutItem::wrapWidget(w).release());
    p->m_bottomBorderAlign = align;
    relayout();
}

void ZWindowLayout::setTopBorderWidget(ZWidget *w, Alignment align) {
    auto *const p = tuiwidgets_impl();
    p->m_topBorder.reset(ZLayoutItem::wrapWidget(w).release());
    p->m_topBorderAlign = align;
    relayout();
}

void ZWindowLayout::setGeometry(QRect toFill) {
    auto *const p = tuiwidgets_impl();
    ZWidget* w = parentWidget();
    if (!w) return; // inconsistent state, just bail out

    if (p->m_centralItem) {
        p->m_centralItem->setGeometry(toFill);
    }
    ZWindow *win = qobject_cast<ZWindow*>(w);
    if (parent() != w) {
        // Not toplevel layout
        return;
    }

    // Border widgets are placed ignoring layoutArea and contentsMargins
    const int width = w->geometry().width();
    const int height = w->geometry().height();

    if (p->m_rightBorder) {
        placeWidgetInCell(width - 1, 1 + p->m_rightBorderTopAdjust,
                          width > 1 ? 1 : 0, height - 2 - p->m_rightBorderTopAdjust - p->m_rightBorderBottomAdjust,
                          p->m_rightBorder.get(), p->m_rightBorderAlign);
    }

    if (p->m_bottomBorder) {
        placeWidgetInCell(1 + p->m_bottomBorderLeftAdjust, height - 1,
                          width - 2 - p->m_bottomBorderLeftAdjust - p->m_bottomBorderRightAdjust, 1,
                          p->m_bottomBorder.get(), p->m_bottomBorderAlign);
    }

    if (p->m_topBorder) {
        QRect r {1, 0, width - 2, 1};
        if (win && win->options() & ZWindow::CloseButton) {
            r.setLeft(5);
        }
        placeWidgetInCell(r.x() + p->m_topBorderLeftAdjust, r.y(),
                          r.width() - p->m_topBorderLeftAdjust - p->m_topBorderRightAdjust, r.height(),
                          p->m_topBorder.get(), p->m_topBorderAlign);
    }
}

void ZWindowLayout::removeWidgetRecursively(ZWidget *widget) {
    auto *const p = tuiwidgets_impl();

    auto apply = [&](std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout> &layoutItem) {
        if (layoutItem && removeWidgetRecursivelyHelper(layoutItem.get(), widget)) {
            layoutItem.release();
        }
    };

    apply(p->m_centralItem);
    apply(p->m_rightBorder);
    apply(p->m_bottomBorder);
    apply(p->m_topBorder);

    relayout();
}

QSize ZWindowLayout::sizeHint() const {
    auto *const p = tuiwidgets_impl();
    QSize size;

    ZWindow* w = qobject_cast<ZWindow*>(parentWidget());

    const bool closeBtn = w && w->borderEdges() & TopEdge && w->options() & ZWindow::CloseButton;

    int tbBorder = 0;
    int rlBorder = 0;

    if (w) {
        if (w->borderEdges() & TopEdge) {
            tbBorder += 1;
        }
        if (w->borderEdges() & RightEdge) {
            rlBorder += 1;
        }
        if (w->borderEdges() & BottomEdge) {
            tbBorder += 1;
        }
        if (w->borderEdges() & LeftEdge) {
            rlBorder += 1;
        }
    }

    if (p->m_centralItem) {
        size = p->m_centralItem->sizeHint().expandedTo({0, 0});
        size.rwidth() += rlBorder;
        size.rheight() += tbBorder;
    } else {
        size = {rlBorder, tbBorder};
    }
    if (closeBtn) {
        size.setWidth(std::max(size.width(), 6));
    }
    if (p->m_rightBorder) {
        size.setHeight(std::max(size.height(), p->m_rightBorder->sizeHint().expandedTo({0, 0}).height()
                                + 2 + p->m_rightBorderTopAdjust + p->m_rightBorderBottomAdjust));
        size.setWidth(std::max(size.width(), 1));
    }
    int tbBorderWidgetsHeight = 0;
    if (p->m_bottomBorder) {
        size.setWidth(std::max(size.width(), p->m_bottomBorder->sizeHint().expandedTo({0, 0}).width()
                               + 2 + p->m_bottomBorderLeftAdjust + p->m_bottomBorderRightAdjust));
        tbBorderWidgetsHeight += 1;
    }
    if (p->m_topBorder) {
        size.setWidth(std::max(size.width(), p->m_topBorder->sizeHint().expandedTo({0, 0}).width()
                               + (closeBtn ? 6 : 2) + p->m_topBorderLeftAdjust + p->m_topBorderRightAdjust));
        tbBorderWidgetsHeight += 1;
    }
    size.setHeight(std::max(size.height(), tbBorderWidgetsHeight));
    return size - QSize{rlBorder, tbBorder};
}

int ZWindowLayout::topBorderLeftAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_topBorderLeftAdjust;
}

void ZWindowLayout::setTopBorderLeftAdjust(int topBorderLeftAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_topBorderLeftAdjust = topBorderLeftAdjust;
    relayout();
}

int ZWindowLayout::topBorderRightAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_topBorderRightAdjust;
}

void ZWindowLayout::setTopBorderRightAdjust(int topBorderRightAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_topBorderRightAdjust = topBorderRightAdjust;
    relayout();
}

int ZWindowLayout::rightBorderTopAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_rightBorderTopAdjust;
}

void ZWindowLayout::setRightBorderTopAdjust(int rightBorderTopAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_rightBorderTopAdjust = rightBorderTopAdjust;
    relayout();
}

int ZWindowLayout::rightBorderBottomAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_rightBorderBottomAdjust;
}

void ZWindowLayout::setRightBorderBottomAdjust(int rightBorderBottomAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_rightBorderBottomAdjust = rightBorderBottomAdjust;
    relayout();
}

int ZWindowLayout::bottomBorderLeftAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_bottomBorderLeftAdjust;
}

void ZWindowLayout::setBottomBorderLeftAdjust(int bottomBorderLeftAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_bottomBorderLeftAdjust = bottomBorderLeftAdjust;
    relayout();
}

int ZWindowLayout::bottomBorderRightAdjust() const {
    auto *const p = tuiwidgets_impl();
    return p->m_bottomBorderRightAdjust;
}

void ZWindowLayout::setBottomBorderRightAdjust(int bottomBorderRightAdjust) {
    auto *const p = tuiwidgets_impl();
    p->m_bottomBorderRightAdjust = bottomBorderRightAdjust;
    relayout();
}

ZWidget *ZWindowLayout::widget() {
    return ZLayout::widget();
}

ZLayout *ZWindowLayout::layout() {
    return ZLayout::layout();
}

void ZWindowLayout::childEvent(QChildEvent *event) {
    auto *const p = tuiwidgets_impl();

    ZLayout *removedLayout;
    if (event->removed() && (removedLayout = qobject_cast<ZLayout*>(event->child()))) {
        if (removedLayout == p->m_centralItem.get()) {
            p->m_centralItem.release();
            p->m_centralItem = nullptr;
            relayout();
        }
    }


    ZLayout::childEvent(event);
}

SizePolicy ZWindowLayout::sizePolicyH() const {
    return ZLayout::sizePolicyH();
}

SizePolicy ZWindowLayout::sizePolicyV() const {
    return ZLayout::sizePolicyV();
}

bool ZWindowLayout::isVisible() const {
    return ZLayout::isVisible();
}

bool ZWindowLayout::isSpacer() const {
    return ZLayout::isSpacer();
}

bool ZWindowLayout::event(QEvent *event) {
    return ZLayout::event(event);
}

bool ZWindowLayout::eventFilter(QObject *watched, QEvent *event) {
    return ZLayout::eventFilter(watched, event);
}

void ZWindowLayout::widgetEvent(QEvent *event) {
    ZLayout::widgetEvent(event);
}

void ZWindowLayout::timerEvent(QTimerEvent *event) {
    ZLayout::timerEvent(event);
}

void ZWindowLayout::customEvent(QEvent *event) {
    ZLayout::customEvent(event);
}

void ZWindowLayout::connectNotify(const QMetaMethod &signal) {
    ZLayout::connectNotify(signal);
}

void ZWindowLayout::disconnectNotify(const QMetaMethod &signal) {
    ZLayout::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
