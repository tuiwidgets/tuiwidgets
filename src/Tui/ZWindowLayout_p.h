// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOWLAYOUT_P_INCLUDED
#define TUIWIDGETS_ZWINDOWLAYOUT_P_INCLUDED

#include <Tui/ZWindowLayout.h>

#include <Tui/ZLayout_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowLayoutPrivate : public ZLayoutPrivate {
public:
    ZWindowLayoutPrivate();
    ~ZWindowLayoutPrivate() override = default;

public:
    std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout> m_centralItem;
    std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout> m_rightBorder;
    Alignment m_rightBorderAlign = Alignment();
    std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout> m_bottomBorder;
    Alignment m_bottomBorderAlign = Alignment();
    std::unique_ptr<ZLayoutItem, Private::DeleteUnlessLayout> m_topBorder;
    Alignment m_topBorderAlign = Alignment();
    int m_topBorderLeftAdjust = 0;
    int m_topBorderRightAdjust = 0;
    int m_rightBorderTopAdjust = 0;
    int m_rightBorderBottomAdjust = 0;
    int m_bottomBorderLeftAdjust = 0;
    int m_bottomBorderRightAdjust = 0;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWLAYOUT_P_INCLUDED
