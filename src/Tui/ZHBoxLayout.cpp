#include "ZHBoxLayout.h"

#include "Layout_p.h"

TUIWIDGETS_NS_START

using Private::SpacerLayoutItem;
using Private::placeWidgetInCell;
using Private::boxLayouter;
using Private::mergePolicy;


class ZHBoxLayoutPrivate {
public:
    std::vector<std::unique_ptr<ZLayoutItem>> items;
    int spacing = 0;
};

ZHBoxLayout::ZHBoxLayout()
    : p(std::make_unique<ZHBoxLayoutPrivate>())
{
}

ZHBoxLayout::~ZHBoxLayout() {
}

int ZHBoxLayout::spacing() const {
    return p->spacing;
}

void ZHBoxLayout::setSpacing(int sp) {
    p->spacing = std::max(sp, 0);
    relayout();
}

void ZHBoxLayout::addWidget(ZWidget *w) {
    p->items.emplace_back(ZLayoutItem::wrapWidget(w));
    relayout();
}

void ZHBoxLayout::add(ZLayout *l) {
    p->items.emplace_back(l);
    l->setParent(this);
    relayout();
}

void ZHBoxLayout::addSpacing(int size) {
    p->items.emplace_back(std::make_unique<SpacerLayoutItem>(size, 0, SizePolicy::Fixed, SizePolicy::Minimum));
    relayout();
}

void ZHBoxLayout::addStretch() {
    p->items.emplace_back(std::make_unique<SpacerLayoutItem>(0, 0, SizePolicy::Expanding, SizePolicy::Minimum));
    relayout();
}


void ZHBoxLayout::setGeometry(QRect toFill) {
    const int width = toFill.width();
    const int height = toFill.height();

    std::vector<ZLayoutItem*> visibleItems;

    for (auto &item : p->items) {
        if (item->isVisible()) {
            visibleItems.push_back(item.get());
        }
    }

    auto place = [&visibleItems, height, toFill] (int x, int wi, int idx) {
        placeWidgetInCell(toFill.x() + x, toFill.y(), wi, height, visibleItems[idx], Qt::Alignment());
    };
    auto getHint = [&visibleItems] (int idx) {
        return visibleItems[idx]->sizeHint().width();
    };

    auto getPolicy = [&visibleItems] (int idx) {
        return visibleItems[idx]->sizePolicyH();
    };

    auto getIsSpacer = [&visibleItems] (int idx) {
        return visibleItems[idx]->isSpacer();
    };

    boxLayouter(place, getHint, getPolicy, getIsSpacer, visibleItems.size(), width, p->spacing);
}

QSize ZHBoxLayout::sizeHint() const {
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
        hintSize += std::max(sh.width(), 0);
        hintOther = std::max(std::max(sh.height(), 0), hintOther);
        if (p->items[i]->isSpacer()) {
            numSpacer += 1;
        }
    }

    hintSize += p->spacing * (visibleItems - 1 - numSpacer);

    return { hintSize, hintOther };
}

SizePolicy ZHBoxLayout::sizePolicyH() const {
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

SizePolicy ZHBoxLayout::sizePolicyV() const {
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

bool ZHBoxLayout::isVisible() const {
    for (auto &item: p->items) {
        if (item->isVisible()) {
            return true;
        }
    }
    return false;
}

bool ZHBoxLayout::isSpacer() const {
    return false;
}

bool ZHBoxLayout::event(QEvent *event) {
    return ZLayout::event(event);
}

bool ZHBoxLayout::eventFilter(QObject *watched, QEvent *event) {
    return ZLayout::eventFilter(watched, event);
}

void ZHBoxLayout::timerEvent(QTimerEvent *event) {
    return ZLayout::timerEvent(event);
}

void ZHBoxLayout::childEvent(QChildEvent *event) {
    return ZLayout::childEvent(event);
}

void ZHBoxLayout::customEvent(QEvent *event) {
    return ZLayout::customEvent(event);
}

void ZHBoxLayout::connectNotify(const QMetaMethod &signal) {
    return ZLayout::connectNotify(signal);
}

void ZHBoxLayout::disconnectNotify(const QMetaMethod &signal) {
    return ZLayout::disconnectNotify(signal);
}


TUIWIDGETS_NS_END
