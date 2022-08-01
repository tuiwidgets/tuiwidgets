#ifndef TUIWIDGETS_ZMENUBAR_INCLUDED
#define TUIWIDGETS_ZMENUBAR_INCLUDED

#include <Tui/ZMenuItem.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZMenubarPrivate;

class TUIWIDGETS_EXPORT ZMenubar : public ZWidget {
    Q_OBJECT

public:
    explicit ZMenubar(ZWidget *parent = nullptr);
    ~ZMenubar();

public:
    void setItems(QVector<ZMenuItem> items);
    QVector<ZMenuItem> items() const;

public Q_SLOTS:
    void left();
    void right();
    void close();

public:
    void keyActivate(ZKeyEvent *event);
    QSize sizeHint() const override;
    bool event(QEvent *event) override;

private Q_SLOTS:
    void commandStateChanged(ZSymbol command);

protected:
    void respawnMenu();

    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;

private:
    void updateCacheAndRegrabKeys();

public:
    // public virtuals from base class override everything for later ABI compatibility
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
    TUIWIDGETS_DECLARE_PRIVATE(ZMenubar)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENUBAR_INCLUDED
