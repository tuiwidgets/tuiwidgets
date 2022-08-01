#ifndef TUIWIDGETS_ZMENU_INCLUDED
#define TUIWIDGETS_ZMENU_INCLUDED

#include <Tui/ZMenuItem.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZMenuPrivate;

class TUIWIDGETS_EXPORT ZMenu : public ZWidget {
    Q_OBJECT

public:
    explicit ZMenu(ZWidget *parent = nullptr);
    ~ZMenu();

public:
    void setItems(QVector<ZMenuItem> items);
    QVector<ZMenuItem> items() const;

    void setParentMenu(ZMenubar *menu);

    void popup(const QPoint &p);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QObject *facet(const QMetaObject &metaObject) const override;

Q_SIGNALS:
    void aboutToHide();
    void aboutToShow();

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QRect layoutArea() const override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;


private:
    TUIWIDGETS_DECLARE_PRIVATE(ZMenu)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENU_INCLUDED
