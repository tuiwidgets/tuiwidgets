#ifndef TUIWIDGETS_ZINPUTBOX_INCLUDED
#define TUIWIDGETS_ZINPUTBOX_INCLUDED

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZInputBoxPrivate;

class TUIWIDGETS_EXPORT ZInputBox : public ZWidget {
    Q_OBJECT

public:
    explicit ZInputBox(ZWidget *parent=nullptr);
    explicit ZInputBox(const QString &contents, ZWidget *parent=nullptr);

public:
    QString text() const;

    enum EchoMode : int {
        Normal = 0,
        NoEcho = 1,
        Password = 2
    };
    void setEchoMode(EchoMode echoMode);
    EchoMode echoMode() const;

    int cursorPosition() const;
    void setCursorPosition(int pos);
    void insertAtCursorPosition(const QString &text);

    QSize sizeHint() const override;

public Q_SLOTS:
    void setText(const QString &text);

Q_SIGNALS:
    void textChanged(const QString &text);

public:
    bool event(QEvent *event) override;

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;


public:
    // public virtuals from base class override everything for later ABI compatibility
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize minimumSizeHint() const override;
    QRect layoutArea() const override;
    QObject *facet(const QMetaObject &metaObject) override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZInputBox)
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZINPUTBOX_INCLUDED
