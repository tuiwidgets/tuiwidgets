#ifndef TUIWIDGETS_ZWIDGET_P_INCLUDED
#define TUIWIDGETS_ZWIDGET_P_INCLUDED

#include <QRect>

#include <Tui/tuiwidgets_internal.h>

#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

#define tui_magic_v0 0xbdf78943

class ZWidgetPrivate {
public:
    ZWidgetPrivate(ZWidget *pub);
    virtual ~ZWidgetPrivate();


    // variables
    QRect rect;

    // internal
    const unsigned int tui_magic = tui_magic_v0;
    ZWidget* pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZWidget)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWIDGET_P_INCLUDED
