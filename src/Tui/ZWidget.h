#ifndef TUIWIDGETS_ZWIDGET_INCLUDED
#define TUIWIDGETS_ZWIDGET_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidgetPrivate;

class TUIWIDGETS_EXPORT ZWidget : public QObject {
    Q_OBJECT
public:
    explicit ZWidget(ZWidget *parent = 0);
    ~ZWidget();

protected:
    explicit ZWidget(QObject *parent, std::unique_ptr<ZWidgetPrivate> pimpl);

protected:
    std::unique_ptr<ZWidgetPrivate> tuiwidgets_pimpl_ptr;

    // override everything for later ABI compatibility
public:
    ZWidget* parentWidget() const { return static_cast<ZWidget*>(parent()); }
    void setParent(ZWidget *parent);

    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    Q_DISABLE_COPY(ZWidget)
    TUIWIDGETS_DECLARE_PRIVATE(ZWidget)
};

TUIWIDGETS_NS_END
#endif // TUIWIDGETS_ZWIDGET_INCLUDED
