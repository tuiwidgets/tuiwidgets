// SPDX-License-Identifier: BSL-1.0

#include "Layout_p.h"

TUIWIDGETS_NS_START

namespace Private {

SpacerLayoutItem::SpacerLayoutItem(int width, int height, SizePolicy sizePolicyH, SizePolicy sizePolicyV)
    : _sizePolicyH(sizePolicyH), _sizePolicyV(sizePolicyV), _size({width, height})
{
}

SpacerLayoutItem::~SpacerLayoutItem() {
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


void placeWidgetInCell(int x, int y, int width, int height, ZLayoutItem *w, Alignment align) {
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
        if ((align & AlignHorizontal_Mask)
            || (w->sizePolicyH() == SizePolicy::Fixed || w->sizePolicyH() == SizePolicy::Maximum)) {
            // use sizeHint without expansion for aligned elements and non expanding policies.
            allocation.setWidth(sizeHint.width());
            if (align & AlignHCenter) {
                int spaceRemaining = width - sizeHint.width();
                allocation.moveLeft(x + spaceRemaining / 2);
            } else if (align & AlignRight) {
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
        if ((align & AlignVertical_Mask)
            || (w->sizePolicyV() == SizePolicy::Fixed || w->sizePolicyV() == SizePolicy::Maximum)) {
            // use sizeHint without expansion for aligned elements and non expanding policies.
            allocation.setHeight(sizeHint.height());
            if (align & AlignVCenter) {
                int spaceRemaining = height - sizeHint.height();
                allocation.moveTop(y + spaceRemaining / 2);
            } else if (align & AlignBottom) {
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

}

TUIWIDGETS_NS_END
