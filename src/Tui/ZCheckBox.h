// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCHECKBOX_INCLUDED
#define TUIWIDGETS_ZCHECKBOX_INCLUDED

#include <Tui/ZShortcut.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCheckBoxPrivate;

class TUIWIDGETS_EXPORT ZCheckBox : public ZWidget {
    Q_OBJECT
public:
    explicit ZCheckBox(ZWidget *parent=nullptr);
    explicit ZCheckBox(const QString &text, ZWidget *parent=nullptr);
    explicit ZCheckBox(WithMarkupTag, const QString &markup, ZWidget *parent=nullptr);
    ~ZCheckBox() override;

public:
    QString text() const;
    void setText(const QString &text);

    QString markup() const;
    void setMarkup(const QString &markup);

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

    bool isTristate() const;
    void setTristate(bool tristate = true);

    void setShortcut(const ZKeySequence &key);

    QSize sizeHint() const override;

public Q_SLOTS:
    void click();
    void toggle();

Q_SIGNALS:
    void stateChanged(Qt::CheckState state);

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
    TUIWIDGETS_DECLARE_PRIVATE(ZCheckBox)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCHECKBOX_INCLUDED
