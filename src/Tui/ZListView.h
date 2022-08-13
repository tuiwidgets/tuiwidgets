// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLISTVIEW_INCLUDED
#define TUIWIDGETS_ZLISTVIEW_INCLUDED

#include <QAbstractItemModel>
#include <QItemSelectionModel>

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZListViewPrivate;

class TUIWIDGETS_EXPORT ZListView : public ZWidget {
    Q_OBJECT

public:
    enum ScrollHint : int {
        EnsureVisible, PositionAtTop, PositionAtBottom, PositionAtCenter
    };

public:
    explicit ZListView(ZWidget *parent=nullptr);
    ~ZListView() override;

public:
    void setItems(const QStringList& newItems);
    QStringList items() const;
    QString currentItem() const;
    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;
    void setCurrentIndex(QModelIndex index);
    QModelIndex currentIndex() const;
    QItemSelectionModel *selectionModel() const;

    void scrollTo(const QModelIndex& index, ScrollHint hint=EnsureVisible);

Q_SIGNALS:
    void enterPressed(int selected);

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize sizeHint() const override;
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
    void moveEvent(ZMoveEvent *event) override;

private:
    void detachModel();
    void attachModel();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZListView)
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLISTVIEW_INCLUDED
