// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED
#define TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED

#include <Tui/ZBasicDefaultWidgetManager.h>

#include <QPointer>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZBasicDefaultWidgetManagerPrivate {
public:
    ZBasicDefaultWidgetManagerPrivate(ZWidget *dialog);
    virtual ~ZBasicDefaultWidgetManagerPrivate();

public:
    QPointer<ZWidget> _defaultWidget;
    ZWidget *_dialog = nullptr;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBASICDEFAULTWINDOWMANAGER_P_INCLUDED
