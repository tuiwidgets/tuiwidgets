// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTLINE_P_INCLUDED
#define TUIWIDGETS_ZTEXTLINE_P_INCLUDED

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextLinePrivate : public ZWidgetPrivate {
public:
    ZTextLinePrivate(ZWidget *pub);
    ~ZTextLinePrivate() override;

public:
    ZStyledTextLine styledText;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTLINE_P_INCLUDED
