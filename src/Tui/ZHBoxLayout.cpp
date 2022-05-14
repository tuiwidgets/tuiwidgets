#include "ZHBoxLayout.h"
#include "ZVBoxLayout.h"

// this file also contains the implementation of ZVBoxLayout due to shared private code

TUIWIDGETS_NS_START

class ZVBoxLayoutPrivate {
public:
    std::vector<std::unique_ptr<ZLayoutItem>> items;
    int spacing = 0;
};

class ZHBoxLayoutPrivate {
public:
    std::vector<std::unique_ptr<ZLayoutItem>> items;
    int spacing = 0;
};

namespace {

class SpacerLayoutItem : public ZLayoutItem {
public:
    explicit SpacerLayoutItem(int width, int height, SizePolicy sizePolicyH, SizePolicy sizePolicyV);

public:
    void setGeometry(QRect r) override;
    QSize sizeHint() const override;
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;

    bool isSpacer() const override;

private:
    SizePolicy _sizePolicyH;
    SizePolicy _sizePolicyV;
    QSize _size;
};

SpacerLayoutItem::SpacerLayoutItem(int width, int height, SizePolicy sizePolicyH, SizePolicy sizePolicyV)
    : _sizePolicyH(sizePolicyH), _sizePolicyV(sizePolicyV), _size({width, height})
{
}

void SpacerLayoutItem::setGeometry(QRect) {
    // Nothing to do
}

QSize SpacerLayoutItem::sizeHint() const {
    return _size;
}

SizePolicy SpacerLayoutItem::sizePolicyH() const {
    return _sizePolicyH;
}

SizePolicy SpacerLayoutItem::sizePolicyV() const {
    return _sizePolicyV;
}

bool SpacerLayoutItem::isVisible() const {
    return true;
}

bool SpacerLayoutItem::isSpacer() const {
    return true;
}

static void placeWidgetInCell(int x, int y, int width, int height, ZLayoutItem *w, Qt::Alignment align) {
    QSize sizeHint = w->sizeHint().expandedTo({0, 0});
    QRect allocation;

    if (height <= 0 || width <= 0) {
        allocation = {x, y, 0, 0};
        w->setGeometry(allocation);
        return;
    }

    // allocate in x
    if (sizeHint.width() >= width) { // Not big enough anyway, just squeeze it in.
        allocation.moveLeft(x);
        allocation.setWidth(width);
    } else {
        if ((align & Qt::AlignHorizontal_Mask)
            || (w->sizePolicyH() == SizePolicy::Fixed || w->sizePolicyH() == SizePolicy::Maximum)) {
            // use sizeHint without expansion for aligned elements and non expanding policies.
            allocation.setWidth(sizeHint.width());
            if (align & Qt::AlignHCenter) {
                int spaceRemaining = width - sizeHint.width();
                allocation.moveLeft(x + spaceRemaining / 2);
            } else if (align & Qt::AlignRight) {
                allocation.moveLeft(x + width - sizeHint.width());
            } else {
                allocation.moveLeft(x);
            }
        } else {
            // allocate everything
            allocation.moveLeft(x);
            allocation.setWidth(width);
        }
    }

    // allocate in y
    if (sizeHint.height() >= height) { // Not big enough anyway, just squeeze it in.
        allocation.moveTop(y);
        allocation.setHeight(height);
    } else {
        if ((align & Qt::AlignVertical_Mask)
            || (w->sizePolicyV() == SizePolicy::Fixed || w->sizePolicyV() == SizePolicy::Maximum)) {
            // use sizeHint without expansion for aligned elements and non expanding policies.
            allocation.setHeight(sizeHint.height());
            if (align & Qt::AlignVCenter) {
                int spaceRemaining = height - sizeHint.height();
                allocation.moveTop(y + spaceRemaining / 2);
            } else if (align & Qt::AlignBottom) {
                allocation.moveTop(y + height - sizeHint.height());
            } else {
                allocation.moveTop(y);
            }
        } else {
            // allocate everything
            allocation.moveTop(y);
            allocation.setHeight(height);
        }
    }

    // place
    w->setGeometry(allocation);
}

template <typename Place, typename GetHint, typename GetPolicy, typename GetIsSpacer>
static void boxLayouter(Place &place, GetHint &getHint, GetPolicy &getPolicy, GetIsSpacer &getIsSpacer,
                        int items, int totalSpace, int spacing) {
    int hintSize = 0;
    int numExpanding = 0;
    int numCanGrow = 0;
    int numSpacer = 0;
    int shrinkableSize = 0;
    int spacerSize = 0;

    for (int i = 0; i < items; i++) {
        int hint = std::max(getHint(i), 0);
        hintSize += hint;
        if (getPolicy(i) == SizePolicy::Expanding) {
            numExpanding += 1;
        } else if (getPolicy(i) != SizePolicy::Fixed && getPolicy(i) != SizePolicy::Maximum) {
            numCanGrow += 1;
        }
        if (getPolicy(i) != SizePolicy::Fixed && getPolicy(i) != SizePolicy::Minimum) {
            shrinkableSize += hint;
        }
        if (getIsSpacer(i)) {
            numSpacer += 1;
            spacerSize += hint;
        }
    }

    const int spaceBetween = spacing * (items - 1 - numSpacer);
    const int hintAndSpacing = hintSize + spaceBetween;

    if (hintAndSpacing == totalSpace) {
        int pos = 0;

        for (int i = 0; i < items; i++) {
            int size = std::max(getHint(i), 0);
            place(pos, size, i);
            pos += size;
            if (!getIsSpacer(i)) {
                pos += spacing;
            }
        }
    } else if (hintAndSpacing > totalSpace) {
        if (hintAndSpacing - spacerSize - spaceBetween <= totalSpace) { // if removing spacing makes this fit
            int remainingSpacingSize = spacerSize + spaceBetween;
            int toDistribute = totalSpace - (hintAndSpacing - spacerSize - spaceBetween);

            int pos = 0;

            for (int i = 0; i < items; i++) {
                int size = std::max(getHint(i), 0);
                if (getIsSpacer(i)) {
                    int allocated = remainingSpacingSize ? ((size * toDistribute + remainingSpacingSize / 2) / remainingSpacingSize) : 0;
                    remainingSpacingSize -= size;
                    toDistribute -= allocated;

                    place(pos, allocated, i);
                    pos += allocated;
                } else {
                    place(pos, size, i);
                    pos += size;

                    int allocated = remainingSpacingSize ? ((spacing * toDistribute + remainingSpacingSize / 2) / remainingSpacingSize) : 0;
                    remainingSpacingSize -= size;
                    toDistribute -= allocated;
                    pos += allocated;
                }
            }
        } else {
            int contractionNeeded = hintAndSpacing - spacerSize - spaceBetween - totalSpace;
            if (contractionNeeded >= shrinkableSize) {
                int pos = 0;
                for (int i = 0; i < items; i++) {
                    if (getIsSpacer(i)) {
                        place(pos, 0, i);
                        continue;
                    }
                    int size = std::max(getHint(i), 0);
                    if (getPolicy(i) == SizePolicy::Fixed || getPolicy(i) == SizePolicy::Minimum) {
                        place(pos, size, i);
                        pos += size;
                    } else {
                        place(pos, 0, i);
                    }
                }
            } else {
                int toDistribute = shrinkableSize - contractionNeeded;
                int remainingShrinkableSize = shrinkableSize;

                int pos = 0;
                for (int i = 0; i < items; i++) {
                    if (getIsSpacer(i)) {
                        place(pos, 0, i);
                        continue;
                    }
                    int size = std::max(getHint(i), 0);
                    if (getPolicy(i) == SizePolicy::Fixed || getPolicy(i) == SizePolicy::Minimum) {
                        place(pos, size, i);
                        pos += size;
                    } else {
                        int allocated = remainingShrinkableSize ? ((size * toDistribute + remainingShrinkableSize / 2) / remainingShrinkableSize) : 0;
                        remainingShrinkableSize -= size;
                        toDistribute -= allocated;
                        place(pos, allocated, i);
                        pos += allocated;
                    }
                }
            }
        }
    } else if (numExpanding) {
        // Distribute space only into expanding items
        int pos = 0;

        const int perItemExpansion = (totalSpace - hintAndSpacing) / numExpanding;
        int remainder = (totalSpace - hintAndSpacing) % numExpanding;

        for (int i = 0; i < items; i++) {
            int size = std::max(getHint(i), 0);
            if (getPolicy(i) == SizePolicy::Expanding) {
                size += perItemExpansion;
                if (remainder) {
                    remainder -= 1;
                    size += 1;
                }
            }

            place(pos, size, i);
            pos += size;
            if (!getIsSpacer(i)) {
                pos += spacing;
            }
        }
    } else {
        // Distribute space into items that can grow, if none just not allocate remaining space

        int pos = 0;

        const int perItemExpansion = numCanGrow ? (totalSpace - hintAndSpacing) / numCanGrow : 0;
        int remainder = numCanGrow ? (totalSpace - hintAndSpacing) % numCanGrow : 0;

        for (int i = 0; i < items; i++) {
            int size = std::max(getHint(i), 0);
            if (getPolicy(i) != SizePolicy::Fixed
                && getPolicy(i) != SizePolicy::Maximum) {
                size += perItemExpansion;
                if (remainder) {
                    remainder -= 1;
                    size += 1;
                }
            }

            place(pos, size, i);
            pos += size;
            if (!getIsSpacer(i)) {
                pos += spacing;
            }
        }
    }
}

static SizePolicy mergePolicy(SizePolicy current, SizePolicy itemPolicy) {
    if (itemPolicy == SizePolicy::Fixed) {
        // Nothing to do
    } else if (itemPolicy == SizePolicy::Minimum) {
    } else if (itemPolicy == SizePolicy::Maximum) {
    } else if (itemPolicy == SizePolicy::Preferred) {
        if (current != SizePolicy::Expanding) {
            current = SizePolicy::Preferred;
        }
    } else if (itemPolicy == SizePolicy::Expanding) {
        current = SizePolicy::Expanding;
    }
    return current;
}

}

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


ZVBoxLayout::ZVBoxLayout()
    : p(std::make_unique<ZVBoxLayoutPrivate>())
{
}

ZVBoxLayout::~ZVBoxLayout() {
}

int ZVBoxLayout::spacing() const {
    return p->spacing;
}

void ZVBoxLayout::setSpacing(int sp) {
    p->spacing = std::max(sp, 0);
    relayout();
}

void ZVBoxLayout::addWidget(ZWidget *w) {
    p->items.emplace_back(ZLayoutItem::wrapWidget(w));
    relayout();
}

void ZVBoxLayout::add(ZLayout *l) {
    p->items.emplace_back(l);
    l->setParent(this);
    relayout();
}

void ZVBoxLayout::addSpacing(int size) {
    p->items.emplace_back(std::make_unique<SpacerLayoutItem>(0, size, SizePolicy::Minimum, SizePolicy::Fixed));
    relayout();
}

void ZVBoxLayout::addStretch() {
    p->items.emplace_back(std::make_unique<SpacerLayoutItem>(0, 0, SizePolicy::Minimum, SizePolicy::Expanding));
    relayout();
}

void ZVBoxLayout::setGeometry(QRect toFill) {
    const int width = toFill.width();
    const int height = toFill.height();

    std::vector<ZLayoutItem*> visibleItems;

    for (auto &item : p->items) {
        if (item->isVisible()) {
            visibleItems.push_back(item.get());
        }
    }

    auto place = [&visibleItems, width, toFill] (int y, int h, int idx) {
        placeWidgetInCell(toFill.x(), toFill.y() + y, width, h, visibleItems[idx], Qt::Alignment());
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

QSize ZVBoxLayout::sizeHint() const {
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
    for (auto &item: p->items) {
        if (item->isVisible()) {
            return true;
        }
    }
    return false;
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

void ZVBoxLayout::timerEvent(QTimerEvent *event) {
    return ZLayout::timerEvent(event);
}

void ZVBoxLayout::childEvent(QChildEvent *event) {
    return ZLayout::childEvent(event);
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
