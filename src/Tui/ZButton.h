#ifndef TUIWIDGETS_ZBUTTON_INCLUDED
#define TUIWIDGETS_ZBUTTON_INCLUDED

#include <Tui/ZShortcut.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZButtonPrivate;

class TUIWIDGETS_EXPORT ZButton : public ZWidget {
    Q_OBJECT
public:
    explicit ZButton(ZWidget *parent=nullptr);
    explicit ZButton(const QString &text, ZWidget *parent=nullptr);
    explicit ZButton(WithMarkupTag, const QString &markup, ZWidget *parent=nullptr);
    ~ZButton() override;

public:
    QString text() const;
    void setText(const QString &t);

    QString markup() const;
    void setMarkup(const QString &m);

    void setShortcut(const ZKeySequence &key);

    void setDefault(bool d);
    bool isDefault() const;

    bool event(QEvent *event) override;
    QSize sizeHint() const override;

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void click();

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool eventFilter(QObject *watched, QEvent *event) override;
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
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

private:
    void removeShortcut();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZButton)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBUTTON_INCLUDED
