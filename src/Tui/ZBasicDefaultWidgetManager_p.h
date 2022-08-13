// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED
#define TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED

#include <Tui/ZBasicDefaultWidgetManager.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicDefaultWidgetManagerPrivate {
public:
    ZBasicDefaultWidgetManagerPrivate(ZWidget *dialog);
    virtual ~ZBasicDefaultWidgetManagerPrivate();

public:
    ZWidget *_defaultWidget = nullptr;
    ZWidget *_dialog = nullptr;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED
