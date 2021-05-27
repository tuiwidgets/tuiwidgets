#ifndef TUIWIDGETS_ZWINDOW_INCLUDED
#define TUIWIDGETS_ZWINDOW_INCLUDED

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowPrivate;

class TUIWIDGETS_EXPORT ZWindow : public ZWidget {
    Q_OBJECT

public:
    enum Option {
        CloseButton = 0x1,
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
    Qt::Edges borderEdges() const;
    void setBorderEdges(Qt::Edges borders);

    QSize sizeHint() const override;
    QRect layoutArea() const override;

public Q_SLOTS:
    void setWindowTitle(const QString &title);

Q_SIGNALS:
    void windowTitleChanged(const QString &title);

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;

public:
    // public virtuals from base classes: override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QObject *facet(const QMetaObject metaObject) override;

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
    void resizeEvent(ZResizeEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindow)
};

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZWindow::Options)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZWindow::Options)

#endif // TUIWIDGETS_ZWINDOW_INCLUDED
