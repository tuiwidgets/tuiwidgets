// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_P_INCLUDED
#define TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_P_INCLUDED

#include <Tui/ZTerminalDiagnosticsDialog.h>
#include <Tui/ZDialog_p.h>

#include <Tui/ZInputBox.h>
#include <Tui/ZTextLine.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTerminalDiagnosticsDialogPrivate : public ZDialogPrivate {
public:
    ZTerminalDiagnosticsDialogPrivate(ZWidget *pub);
    ~ZTerminalDiagnosticsDialogPrivate() override;

public:
    void updateInfo();
    void keyboardTest();

public:
    ZInputBox *terminalInfo = nullptr;
    ZInputBox *terminalCaps = nullptr;
    ZInputBox *terminalSelfId = nullptr;
    ZTextLine *keyHeader = nullptr;
    ZInputBox *keyRaw = nullptr;
    ZTextLine *keyParsed = nullptr;

public:
    TUIWIDGETS_DECLARE_PUBLIC(ZTerminalDiagnosticsDialog)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_P_INCLUDED
