// SPDX-License-Identifier: BSL-1.0

#include "ZVBoxLayout.h"

#include "Layout_p.h"
#include "ZLayout_p.h"

TUIWIDGETS_NS_START

using Private::SpacerLayoutItem;
using Private::placeWidgetInCell;
using Private::boxLayouter;
using Private::mergePolicy;

class ZVBoxLayoutPrivate : public ZLayoutPrivate {
public:
    std::vector<std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout>> items;
    int spacing = 0;
};

ZVBoxLayout::ZVBoxLayout()
    : ZLayout(nullptr, std::make_unique<ZVBoxLayoutPrivate>())
{
}

ZVBoxLayout::~ZVBoxLayout() {
}

int ZVBoxLayout::spacing() const {
    auto *const p = tuiwidgets_impl();
    return p->spacing;
}

void ZVBoxLayout::setSpacing(int sp) {
    auto *const p = tuiwidgets_impl();
    p->spacing = std::max(sp, 0);
    relayout();
}

void ZVBoxLayout::addWidget(ZWidget *w) {
    auto *const p = tuiwidgets_impl();
    p->items.emplace_back(ZLayoutItem::wrapWidget(w).release());
    relayout();
}

void ZVBoxLayout::add(ZLayout *l) {
    auto *const p = tuiwidgets_impl();
    l->setParent(this);
    p->items.emplace_back(l);
    relayout();
}

void ZVBoxLayout::addSpacing(int size) {
    auto *const p = tuiwidgets_impl();
    p->items.emplace_back(new SpacerLayoutItem(0, size, SizePolicy::Minimum, SizePolicy::Fixed));
    relayout();
}

void ZVBoxLayout::addStretch() {
    auto *const p = tuiwidgets_impl();
    p->items.emplace_back(new SpacerLayoutItem(0, 0, SizePolicy::Minimum, SizePolicy::Expanding));
    relayout();
}

void ZVBoxLayout::setGeometry(QRect toFill) {
    auto *const p = tuiwidgets_impl();

    const int width = toFill.width();
    const int height = toFill.height();

    std::vector<ZLayoutItem*> visibleItems;

    for (auto &item : p->items) {
        if (item->isVisible()) {
            visibleItems.push_back(item.get());
        }
    }

    auto place = [&visibleItems, width, toFill] (int y, int h, int idx) {
        placeWidgetInCell(toFill.x(), toFill.y() + y, width, h, visibleItems[idx], Alignment());
    };
    auto getHint = [&visibleItems] (int idx) {
        return visibleItems[idx]->sizeHint().height();
    };

    auto getPolicy = [&visibleItems] (int idx) {
        return visibleItems[idx]->sizePolicyV();
    };

    auto getIsSpacer = [&visibleItems] (int idx) {
        return visibleItems[idx]->isSpacer();
    };

    boxLayouter(place, getHint, getPolicy, getIsSpacer, visibleItems.size(), height, p->spacing);
}

void ZVBoxLayout::removeWidgetRecursively(ZWidget *widget) {
    auto *const p = tuiwidgets_impl();

    for (size_t i = 0; i < p->items.size(); i++) {
        if (removeWidgetRecursivelyHelper(p->items[i].get(), widget)) {
            // item was already deleted
            p->items[i].release();
            p->items.erase(p->items.begin() + i);
            relayout();
            break;
        }
    }
}

QSize ZVBoxLayout::sizeHint() const {
    auto *const p = tuiwidgets_impl();

    int hintSize = 0;
    int hintOther = 0;
    int numSpacer = 0;
    int visibleItems = 0;

    for (size_t i = 0; i < p->items.size(); i++) {
        if (!p->items[i]->isVisible()) {
            continue;
        }
        ++visibleItems;
        auto sh = p->items[i]->sizeHint();
        hintSize += std::max(sh.height(), 0);
        hintOther = std::max(std::max(sh.width(), 0), hintOther);
        if (p->items[i]->isSpacer()) {
            numSpacer += 1;
        }
    }

    hintSize += p->spacing * (visibleItems - 1 - numSpacer);

    return { hintOther, hintSize };
}

SizePolicy ZVBoxLayout::sizePolicyH() const {
    auto *const p = tuiwidgets_impl();

    SizePolicy policy = SizePolicy::Fixed;
    for (auto &item: p->items) {
        if (!item->isVisible()) {
            continue;
        }
        SizePolicy itemPolicy = item->sizePolicyH();
        policy = mergePolicy(policy, itemPolicy);
    }
    return policy;
}

SizePolicy ZVBoxLayout::sizePolicyV() const {
    auto *const p = tuiwidgets_impl();

    SizePolicy policy = SizePolicy::Fixed;
    for (auto &item: p->items) {
        if (!item->isVisible()) {
            continue;
        }
        SizePolicy itemPolicy = item->sizePolicyV();
        policy = mergePolicy(policy, itemPolicy);
    }
    return policy;
}

bool ZVBoxLayout::isVisible() const {
    auto *const p = tuiwidgets_impl();

    for (auto &item: p->items) {
        if (item->isVisible()) {
            return true;
        }
    }
    return false;
}

ZWidget *ZVBoxLayout::widget() {
    return ZLayout::widget();
}

ZLayout *ZVBoxLayout::layout() {
    return ZLayout::layout();
}

void ZVBoxLayout::childEvent(QChildEvent *event) {
    auto *const p = tuiwidgets_impl();

    ZLayout *removedLayout;
    if (event->removed() && (removedLayout = qobject_cast<ZLayout*>(event->child()))) {
        for (size_t i = 0; i < p->items.size(); i++) {
            if (p->items[i].get() == removedLayout) {
                p->items.erase(p->items.begin() + i);
                relayout();
                break;
            }
        }
    }

    return ZLayout::childEvent(event);
}

bool ZVBoxLayout::isSpacer() const {
    return false;
}

bool ZVBoxLayout::event(QEvent *event) {
    return ZLayout::event(event);
}

bool ZVBoxLayout::eventFilter(QObject *watched, QEvent *event) {
    return ZLayout::eventFilter(watched, event);
}

void ZVBoxLayout::widgetEvent(QEvent *event) {
    ZLayout::widgetEvent(event);
}

void ZVBoxLayout::timerEvent(QTimerEvent *event) {
    return ZLayout::timerEvent(event);
}

void ZVBoxLayout::customEvent(QEvent *event) {
    return ZLayout::customEvent(event);
}

void ZVBoxLayout::connectNotify(const QMetaMethod &signal) {
    return ZLayout::connectNotify(signal);
}

void ZVBoxLayout::disconnectNotify(const QMetaMethod &signal) {
    return ZLayout::disconnectNotify(signal);
}

TUIWIDGETS_NS_END
