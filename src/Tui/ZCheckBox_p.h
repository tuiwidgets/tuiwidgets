// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCHECKBOX_P_INCLUDED
#define TUIWIDGETS_ZCHECKBOX_P_INCLUDED

#include <Tui/ZCheckBox.h>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCheckBoxPrivate : public ZWidgetPrivate {
public:
    ZCheckBoxPrivate(ZWidget *pub);
    ~ZCheckBoxPrivate() override;

public:
    ZStyledTextLine styledText;
    Tui::CheckState state = Tui::Unchecked;
    bool tristate = false;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCHECKBOX_P_INCLUDED
