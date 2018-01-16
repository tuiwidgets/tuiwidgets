#ifndef TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
#define TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED

#include <Tui/tuiwidgets_internal.h>

#include "ZSymbol.h"

TUIWIDGETS_NS_START

class ZCommandNotifierPrivate {
public:
    Tui::ZSymbol command;
    bool enabled = true;
};

TUIWIDGETS_NS_END



#endif // TUIWIDGETS_ZCOMMANDNOTIFIER_P_INCLUDED
