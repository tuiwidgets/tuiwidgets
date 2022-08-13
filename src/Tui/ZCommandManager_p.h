// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCOMMANDMANAGER_P_INCLUDED
#define TUIWIDGETS_ZCOMMANDMANAGER_P_INCLUDED

#include <QHash>
#include <QVector>

#include <Tui/ZSymbol.h>
#include <Tui/ZCommandNotifier.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCommandManagerPrivate {
public:
    ZCommandManagerPrivate();
    virtual ~ZCommandManagerPrivate();

public:
    QHash<ZSymbol, QVector<QPointer<ZCommandNotifier>>> commandNotifiers;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOMMANDMANAGER_P_INCLUDED
