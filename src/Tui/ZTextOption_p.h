// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTOPTION_P_INCLUDED
#define TUIWIDGETS_ZTEXTOPTION_P_INCLUDED

#include <QList>

#include <Tui/ZTextOption.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextOptionPrivate {
public:
    ZTextOption::Flags flags {};
    ZTextOption::WrapMode wrapMode = ZTextOption::WrapAnywhere;
    int tabDistance = 8;
    QList<ZTextOption::Tab> tabs;
    std::function<ZTextStyle(const ZTextStyle&, const ZTextStyle&, const ZFormatRange*)> trailingWhitespaceColor;
    std::function<ZTextStyle(int, int, int, const ZTextStyle&, const ZTextStyle&, const ZFormatRange*)> tabColor;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTOPTION_P_INCLUDED
