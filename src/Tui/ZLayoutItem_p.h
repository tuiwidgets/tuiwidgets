#ifndef TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED
#define TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED

#include <Tui/ZLayoutItem.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZWidgetLayoutItem : public ZLayoutItem {
public:
    ZWidgetLayoutItem(ZWidget *widget) : widget(widget) {}
    ~ZWidgetLayoutItem();

public:
    void setGeometry(QRect r) override;
    QSize sizeHint() const  override;
    SizePolicy sizePolicyH() const  override;
    SizePolicy sizePolicyV() const  override;
    bool isVisible() const override;

private:
    ZWidget *widget;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUTITEM_P_INCLUDED
