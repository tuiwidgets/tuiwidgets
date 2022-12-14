// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOW_INCLUDED
#define TUIWIDGETS_ZWINDOW_INCLUDED

#include <QPoint>

#include <Tui/ZWidget.h>
#include <Tui/ZMenuItem.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowPrivate;

class TUIWIDGETS_EXPORT ZWindow : public ZWidget {
    Q_OBJECT

public:
    enum Option : int {
        CloseOption = (1 << 0),
        CloseButton = CloseOption,
        MoveOption = (1 << 1),
        ResizeOption = (1 << 2),
        AutomaticOption = (1 << 3),
        ContainerOptions = (1 << 4),
        DeleteOnClose = (1 << 5),
    };
    Q_DECLARE_FLAGS(Options, Option)
public:
    explicit ZWindow(ZWidget *parent = nullptr);
    explicit ZWindow(const QString &title, ZWidget *parent = nullptr);
    ~ZWindow() override;

public:
    QString windowTitle() const;

    Options options() const;
    void setOptions(Options options);
    Edges borderEdges() const;
    void setBorderEdges(Edges borders);

    void setDefaultPlacement(Alignment align, QPoint displace = {0, 0});
    void setAutomaticPlacement();

    QSize sizeHint() const override;
    QRect layoutArea() const override;
    QObject *facet(const QMetaObject &metaObject) const override;
    bool event(QEvent *event) override;

public Q_SLOTS:
    void setWindowTitle(const QString &title);
    bool showSystemMenu();
    void startInteractiveMove();
    void startInteractiveResize();

    void close();
    void closeSkipCheck(QStringList skipChecks);

Q_SIGNALS:
    void windowTitleChanged(const QString &title);

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;
    virtual QVector<ZMenuItem> systemMenu();
    virtual void closeEvent(ZCloseEvent *event);

public:
    // public virtuals from base classes: override everything for later ABI compatibility
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize minimumSizeHint() const override;
    ZWidget *resolveSizeHintChain() override;

protected:
    // protected virtuals from base classes: override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void pasteEvent(ZPasteEvent *event) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

protected:
    explicit ZWindow(ZWidget *parent, std::unique_ptr<ZWidgetPrivate> pimpl);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindow)
};

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZWindow::Options)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZWindow::Options)

#endif // TUIWIDGETS_ZWINDOW_INCLUDED
