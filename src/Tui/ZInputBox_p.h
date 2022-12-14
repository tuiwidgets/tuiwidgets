// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZINPUTBOX_P_INCLUDED
#define TUIWIDGETS_ZINPUTBOX_P_INCLUDED

#include <Tui/ZInputBox.h>

#include "ZTextLayout.h"
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZInputBoxPrivate : public ZWidgetPrivate {
public:
    ZInputBoxPrivate(ZWidget *pub);
    ~ZInputBoxPrivate() override;

public:
    void adjustScrollPosition();
    ZTextLayout getTextLayout();
    ZTextLayout getTextLayoutForPaint();
    void insertOrReplaceCharacterAtCursorPosition(const QString &text);

public:
    QString text;
    int cursorPosition = 0;
    int scrollPosition = 0;
    ZInputBox::EchoMode echoMode = ZInputBox::Normal;
    bool overwriteMode = false;

    TUIWIDGETS_DECLARE_PUBLIC(ZInputBox)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZINPUTBOX_P_INCLUDED
