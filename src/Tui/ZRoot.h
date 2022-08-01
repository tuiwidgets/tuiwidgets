#ifndef TUIWIDGETS_ZROOT_INCLUDED
#define TUIWIDGETS_ZROOT_INCLUDED

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZRootPrivate;

class TUIWIDGETS_EXPORT ZRoot : public ZWidget {
    Q_OBJECT

public:
    ZRoot();
    ~ZRoot() override;

public:
    void setFillChar(int fillChar);
    int fillChar() const;

    void focusNextWindow();
    void focusPreviousWindow();

    virtual void raiseOnFocus(ZWidget *w);
    QRect layoutArea() const override;
    QSize minimumSizeHint() const override;

protected:

    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    virtual void terminalChanged();

private:
    void focusWindowHelper(bool forward);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QObject *facet(const QMetaObject &metaObject) const override;
    QSize sizeHint() const override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void connectNotify(const QMetaMethod &signal) override;
    void customEvent(QEvent *event) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZRoot)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZROOT_INCLUDED
