// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLABEL_P_INCLUDED
#define TUIWIDGETS_ZLABEL_P_INCLUDED

#include <Tui/ZLabel.h>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZLabelPrivate : public ZWidgetPrivate {
public:
    ZLabelPrivate(ZWidget *pub);
    ~ZLabelPrivate() override;

public:
    ZStyledTextLine styledText;
    QPointer<ZWidget> buddy;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLABEL_P_INCLUDED
