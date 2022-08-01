#ifndef TUIWIDGETS_ZDIALOG_INCLUDED
#define TUIWIDGETS_ZDIALOG_INCLUDED

#include <Tui/ZWindow.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDialogPrivate;

class TUIWIDGETS_EXPORT ZDialog : public ZWindow {
    Q_OBJECT

public:
    explicit ZDialog(ZWidget *parent);
    ~ZDialog() override;

public:
    QObject *facet(const QMetaObject &metaObject) const override;

public Q_SLOTS:
    void reject();

Q_SIGNALS:
    void rejected();

public:
    bool event(QEvent *event) override;

protected:
    void keyEvent(ZKeyEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QRect layoutArea() const override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void paintEvent(ZPaintEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;
    QVector<ZMenuItem> systemMenu() override;
    void closeEvent(ZCloseEvent *event) override;

private:
    void showHandler();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDialog)    
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDIALOG_INCLUDED
