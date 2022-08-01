#ifndef TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
#define TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED

#include <Tui/tuiwidgets_internal.h>

#include "ZSymbol.h"

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
    Qt::ShortcutContext context = Qt::ApplicationShortcut;
    QMetaObject::Connection focusChangedConnection;
    QMetaObject::Connection beforeRenderingConnection;
};

TUIWIDGETS_NS_END



#endif // TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
