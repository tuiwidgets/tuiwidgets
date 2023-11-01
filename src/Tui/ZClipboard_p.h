// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCLIPBOARD_P_INCLUDED
#define TUIWIDGETS_ZCLIPBOARD_P_INCLUDED

#include <Tui/ZClipboard.h>

#include <QPointer>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZClipboardPrivate {
public:
    virtual ~ZClipboardPrivate();

public:
    QString contents;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCLIPBOARD_P_INCLUDED
