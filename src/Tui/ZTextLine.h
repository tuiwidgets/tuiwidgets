#ifndef TUIWIDGETS_ZTEXTLINE_INCLUDED
#define TUIWIDGETS_ZTEXTLINE_INCLUDED

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextLinePrivate;

class TUIWIDGETS_EXPORT ZTextLine : public ZWidget {
    Q_OBJECT
public:
    explicit ZTextLine(ZWidget *parent = nullptr);
    explicit ZTextLine(const QString &text, ZWidget *parent = nullptr);
    explicit ZTextLine(WithMarkupTag, const QString &markup, ZWidget *parent = nullptr);
    ~ZTextLine();

public:
    QString text() const;
    void setText(const QString &text);

    QString markup() const;
    void setMarkup(const QString &markup);

    QSize sizeHint() const override;

protected:
    void paintEvent(ZPaintEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
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
    void keyEvent(ZKeyEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTextLine)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTLINE_INCLUDED
