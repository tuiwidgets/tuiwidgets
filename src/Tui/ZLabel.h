#ifndef TUIWIDGETS_ZLABEL_INCLUDED
#define TUIWIDGETS_ZLABEL_INCLUDED

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZLabelPrivate;

class TUIWIDGETS_EXPORT ZLabel : public ZWidget {
    Q_OBJECT
public:
    explicit ZLabel(ZWidget *parent = nullptr);
    explicit ZLabel(const QString &text, ZWidget *parent = nullptr);
    explicit ZLabel(WithMarkupTag, const QString &markup, ZWidget *parent = nullptr);
    ~ZLabel();

public:
    QString text() const;
    void setText(const QString &text);

    QString markup() const;
    void setMarkup(const QString &markup);

    ZWidget *buddy() const;
    void setBuddy(ZWidget *buddy);

    QSize sizeHint() const override;

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;

private:
    void removeShortcut();

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize minimumSizeHint() const override;
    QRect layoutArea() const override;
    QObject *facet(const QMetaObject &metaObject) const override;

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
    TUIWIDGETS_DECLARE_PRIVATE(ZLabel)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLABEL_INCLUDED
