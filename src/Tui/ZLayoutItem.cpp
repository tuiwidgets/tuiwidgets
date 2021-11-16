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

std::unique_ptr<ZLayoutItem> ZLayoutItem::wrapWidget(ZWidget *widget) {
    return std::make_unique<ZWidgetLayoutItem>(widget);
}

ZWidgetLayoutItem::~ZWidgetLayoutItem() {
}

void ZWidgetLayoutItem::setGeometry(QRect r) {
    if (widget) {
        widget->setGeometry(r);
    }
}

QSize ZWidgetLayoutItem::sizeHint() const {
    if (widget) {
        return widget->effectiveSizeHint();
    } else {
        return {};
    }
}

SizePolicy ZWidgetLayoutItem::sizePolicyH() const {
    if (widget) {
        return widget->sizePolicyH();
    } else {
        return SizePolicy::Fixed;
    }
}

SizePolicy ZWidgetLayoutItem::sizePolicyV() const {
    if (widget) {
        return widget->sizePolicyV();
    } else {
        return SizePolicy::Fixed;
    }
}

bool ZWidgetLayoutItem::isVisible() const {
    if (widget) {
        return widget->isLocallyVisible();
    } else {
        return false;
    }
}

TUIWIDGETS_NS_END
