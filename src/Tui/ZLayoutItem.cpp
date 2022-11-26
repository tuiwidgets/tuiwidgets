// SPDX-License-Identifier: BSL-1.0

#include "ZLayoutItem.h"
#include "ZLayoutItem_p.h"

TUIWIDGETS_NS_START

ZLayoutItem::ZLayoutItem() {
}

ZLayoutItem::~ZLayoutItem() {
}

bool ZLayoutItem::isSpacer() const {
    return false;
}

ZWidget *ZLayoutItem::widget() {
    return nullptr;
}

ZLayout *ZLayoutItem::layout() {
    return nullptr;
}

std::unique_ptr<ZLayoutItem> ZLayoutItem::wrapWidget(ZWidget *widget) {
    return std::make_unique<ZWidgetLayoutItem>(widget);
}

ZWidgetLayoutItem::~ZWidgetLayoutItem() {
}

void ZWidgetLayoutItem::setGeometry(QRect r) {
    if (_widget) {
        _widget->setGeometry(r);
    }
}

QSize ZWidgetLayoutItem::sizeHint() const {
    if (_widget) {
        return _widget->effectiveSizeHint();
    } else {
        return {};
    }
}

SizePolicy ZWidgetLayoutItem::sizePolicyH() const {
    if (_widget) {
        return _widget->sizePolicyH();
    } else {
        return SizePolicy::Fixed;
    }
}

SizePolicy ZWidgetLayoutItem::sizePolicyV() const {
    if (_widget) {
        return _widget->sizePolicyV();
    } else {
        return SizePolicy::Fixed;
    }
}

bool ZWidgetLayoutItem::isVisible() const {
    if (_widget) {
        return _widget->isLocallyVisible();
    } else {
        return false;
    }
}

ZWidget *ZWidgetLayoutItem::widget() {
    return _widget.data();
}

TUIWIDGETS_NS_END
