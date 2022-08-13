// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZMENUITEM_P_INCLUDED
#define TUIWIDGETS_ZMENUITEM_P_INCLUDED

#include <QVector>

#include <Tui/ZMenuItem.h>
#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

class ZMenuItemPrivate {
public:
    QString markup;
    QString fakeShortcut;
    ZImplicitSymbol command;
    QVector<ZMenuItem> subitems;
    std::function<QVector<ZMenuItem>()> subitemsGenerator;
    QMetaObject::Connection subitemsGeneratorResetConnection;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENUITEM_P_INCLUDED
