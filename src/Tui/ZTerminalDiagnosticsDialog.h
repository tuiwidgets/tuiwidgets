// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_INCLUDED
#define TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_INCLUDED

#include <Tui/ZDialog.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTerminalDiagnosticsDialogPrivate;

class TUIWIDGETS_EXPORT ZTerminalDiagnosticsDialog : public ZDialog {
    Q_OBJECT

public:
    explicit ZTerminalDiagnosticsDialog(ZWidget *parent);
    ~ZTerminalDiagnosticsDialog() override;

public:
    bool event(QEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QRect layoutArea() const override;
    QObject *facet(const QMetaObject &metaObject) const override;
    ZWidget *resolveSizeHintChain() override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;
    QVector<ZMenuItem> systemMenu() override;
    void closeEvent(ZCloseEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTerminalDiagnosticsDialog)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTERMINALDIAGNOSTICSDIALOG_INCLUDED
