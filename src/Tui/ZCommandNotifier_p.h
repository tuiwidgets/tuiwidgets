#ifndef TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
#define TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED

#include <Tui/tuiwidgets_internal.h>

#include "ZSymbol.h"

TUIWIDGETS_NS_START

class ZCommandNotifierPrivate {
public:
    void connectToTerminal(Tui::ZCommandNotifier *pub);
    void updateContextSatisfied(Tui::ZCommandNotifier *pub);

    Tui::ZSymbol command;
    bool enabled = true;
    bool contextSatisfied = true;
    Qt::ShortcutContext context = Qt::ApplicationShortcut;
    QMetaObject::Connection focusChangedConnection;
    QMetaObject::Connection beforeRenderingConnection;
};

TUIWIDGETS_NS_END



#endif // TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
