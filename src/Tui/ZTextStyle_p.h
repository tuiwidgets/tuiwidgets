#ifndef TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED
#define TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED

#include <Tui/ZTextStyle.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextStylePrivate {
public:
    Tui::ZColor foregroundColor = Tui::ZColor::defaultColor();
    Tui::ZColor backgroundColor = Tui::ZColor::defaultColor();
    Tui::ZTextAttributes attributes = {};
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED
