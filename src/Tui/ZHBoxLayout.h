// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZHBOXLAYOUT_INCLUDED
#define TUIWIDGETS_ZHBOXLAYOUT_INCLUDED

#include <Tui/ZLayout.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZHBoxLayoutPrivate;

class TUIWIDGETS_EXPORT ZHBoxLayout : public ZLayout {
    Q_OBJECT

public:
    ZHBoxLayout();
    ZHBoxLayout(const ZHBoxLayout&) = delete;
    ZHBoxLayout(const ZHBoxLayout&&) = delete;
    ~ZHBoxLayout() override;

    ZHBoxLayout &operator=(const ZHBoxLayout&) = delete;
    ZHBoxLayout &operator=(const ZHBoxLayout&&) = delete;

public:
    int spacing() const;
    void setSpacing(int sp);

    void addWidget(ZWidget *w);
    void add(ZLayout *l);
    void addSpacing(int size);
    void addStretch();

    void setGeometry(QRect toFill) override;
    void removeWidgetRecursively(ZWidget *widget) override;
    QSize sizeHint() const override;
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    ZWidget *widget() override;
    ZLayout *layout() override;
    bool isSpacer() const override;;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void widgetEvent(QEvent *event) override;

protected:
    void childEvent(QChildEvent *event) override;

    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZHBoxLayout)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZHBOXLAYOUT_INCLUDED
