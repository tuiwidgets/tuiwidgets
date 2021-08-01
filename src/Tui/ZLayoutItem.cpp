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
    widget->setGeometry(r);
}

QSize ZWidgetLayoutItem::sizeHint() const {
    return widget->effectiveSizeHint();
}

SizePolicy ZWidgetLayoutItem::sizePolicyH() const {
    return widget->sizePolicyH();
}

SizePolicy ZWidgetLayoutItem::sizePolicyV() const {
    return widget->sizePolicyV();
}

bool ZWidgetLayoutItem::isVisible() const {
    return widget->isLocallyVisible();
}

TUIWIDGETS_NS_END
