// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDIALOG_P_INCLUDED
#define TUIWIDGETS_ZDIALOG_P_INCLUDED

#include <Tui/ZDialog.h>

#include <Tui/ZBasicDefaultWidgetManager.h>
#include <Tui/ZWindow_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDialogPrivate : public ZWindowPrivate {
public:
    ZDialogPrivate(ZWidget *pub);
    ~ZDialogPrivate() override;

public:
    mutable ZBasicDefaultWidgetManager defaultWidgetManager{pub()};
    bool alreadyShown = false;

    TUIWIDGETS_DECLARE_PUBLIC(ZDialog)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDIALOG_P_INCLUDED
