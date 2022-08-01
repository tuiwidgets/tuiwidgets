#ifndef TUIWIDGETS_ZRADIOBUTTON_INCLUDED
#define TUIWIDGETS_ZRADIOBUTTON_INCLUDED

#include <Tui/ZShortcut.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZRadioButtonPrivate;

class TUIWIDGETS_EXPORT ZRadioButton : public ZWidget {
    Q_OBJECT
public:
    explicit ZRadioButton(ZWidget *parent = nullptr);
    explicit ZRadioButton(const QString &text, ZWidget *parent = nullptr);
    explicit ZRadioButton(WithMarkupTag, const QString &markup, ZWidget *parent = nullptr);
    ~ZRadioButton() override;

public:
    QString text() const;
    void setText(const QString &text);

    QString markup() const;
    void setMarkup(const QString &markup);

    bool checked() const;
    void setChecked(bool state);

    void setShortcut(const ZKeySequence &key);

    QSize sizeHint() const override;

public Q_SLOTS:
    void click();
    void toggle();

Q_SIGNALS:
    void toggled(bool state);

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
    TUIWIDGETS_DECLARE_PRIVATE(ZRadioButton)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZRADIOBUTTON_INCLUDED
