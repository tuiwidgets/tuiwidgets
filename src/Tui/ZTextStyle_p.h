// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED
#define TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED

#include <Tui/ZTextStyle.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextStylePrivate {
public:
    ZColor foregroundColor = ZColor::defaultColor();
    ZColor backgroundColor = ZColor::defaultColor();
    ZTextAttributes attributes = {};
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTSTYLE_P_INCLUDED
