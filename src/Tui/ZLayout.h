// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLAYOUT_INCLUDED
#define TUIWIDGETS_ZLAYOUT_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/ZWidget.h>
#include <Tui/ZLayoutItem.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZLayoutPrivate;
class TUIWIDGETS_EXPORT ZLayout : public QObject, public ZLayoutItem {
    Q_OBJECT
public:
    explicit ZLayout(QObject *parent = nullptr);
    ~ZLayout() override;

public:
    virtual void widgetEvent(QEvent *event);
    virtual void removeWidgetRecursively(ZWidget *widget) = 0;
    ZLayout *layout() override;

    ZWidget *parentWidget() const;

    void setGeometry(QRect r) override = 0;
    QSize sizeHint() const override;
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    ZWidget *widget() override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool isSpacer() const override;

protected:
    void relayout();
    bool removeWidgetRecursivelyHelper(ZLayoutItem *layoutItem, ZWidget *widget);

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

protected:
    ZLayout(QObject *parent, std::unique_ptr<ZLayoutPrivate> pimpl);

    std::unique_ptr<ZLayoutPrivate> tuiwidgets_pimpl_ptr;

private:
    Q_DISABLE_COPY(ZLayout)
    TUIWIDGETS_DECLARE_PRIVATE(ZLayout)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUT_INCLUDED
