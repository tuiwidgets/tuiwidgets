// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED
#define TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED

#include <QPointer>

#include <Tui/ZLayoutItem.h>

TUIWIDGETS_NS_START

class ZWidgetLayoutItem : public ZLayoutItem {
public:
    ZWidgetLayoutItem(ZWidget *widget) : _widget(widget) {}
    ~ZWidgetLayoutItem() override;

public:
    void setGeometry(QRect r) override;
    QSize sizeHint() const override;
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;
    ZWidget *widget() override;

private:
    QPointer<ZWidget> _widget;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED
