// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZMENUBAR_P_INCLUDED
#define TUIWIDGETS_ZMENUBAR_P_INCLUDED

#include <QMap>
#include <QVector>

#include <Tui/ZMenu.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZMenubarPrivate : public ZWidgetPrivate {
public:
    ZMenubarPrivate(ZWidget *pub);
    ~ZMenubarPrivate() override;

public:
    QVector<ZMenuItem> items;
    QVector<int> itemX;
    int width = 0;
    int selected = 0;
    QPointer<ZMenu> activeMenu;
    bool active = false;
    QMap<ZSymbol, QVector<ZShortcut*>> shortcuts;
    QMetaObject::Connection commandStateChangedConnection;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENUBAR_P_INCLUDED
