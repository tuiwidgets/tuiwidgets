// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZBUTTON_P_INCLUDED
#define TUIWIDGETS_ZBUTTON_P_INCLUDED

#include <Tui/ZButton.h>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZButtonPrivate : public ZWidgetPrivate {
public:
    ZButtonPrivate(ZWidget *pub);
    ~ZButtonPrivate() override;

public:
    ZStyledTextLine styledText;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBUTTON_P_INCLUDED
