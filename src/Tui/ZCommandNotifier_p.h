// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
#define TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED

#include "ZCommandNotifier.h"

#include <QSet>

#include "ZCommandManager.h"
#include "ZSymbol.h"

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCommandNotifierPrivate {
public:
    ZCommandNotifierPrivate();
    virtual ~ZCommandNotifierPrivate();

public:
    void connectToTerminal(ZCommandNotifier *pub);
    void updateContextSatisfied(ZCommandNotifier *pub);

    ZSymbol command;
    bool enabled = true;
    bool contextSatisfied = true;
    ShortcutContext context = ApplicationShortcut;
    QMetaObject::Connection focusChangedConnection;
    QMetaObject::Connection beforeRenderingConnection;
    QSet<ZCommandManager*> registrations;
};

TUIWIDGETS_NS_END



#endif // TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
