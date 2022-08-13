// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZRADIOBUTTON_P_INCLUDED
#define TUIWIDGETS_ZRADIOBUTTON_P_INCLUDED

#include <Tui/ZRadioButton.h>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZRadioButtonPrivate : public ZWidgetPrivate {
public:
    ZRadioButtonPrivate(ZWidget *pub);
    ~ZRadioButtonPrivate() override;

public:
    ZStyledTextLine styledText;
    bool checked = false;
    bool tristate = false;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZRADIOBUTTON_P_INCLUDED
