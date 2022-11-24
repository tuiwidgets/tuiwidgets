// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZVBOXLAYOUT_INCLUDED
#define TUIWIDGETS_ZVBOXLAYOUT_INCLUDED

#include <Tui/ZLayout.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZVBoxLayoutPrivate;

class TUIWIDGETS_EXPORT ZVBoxLayout : public ZLayout {
    Q_OBJECT

public:
    ZVBoxLayout();
    ZVBoxLayout(const ZVBoxLayout&) = delete;
    ZVBoxLayout(const ZVBoxLayout&&) = delete;
    ~ZVBoxLayout() override;

    ZVBoxLayout &operator=(const ZVBoxLayout&) = delete;
    ZVBoxLayout &operator=(const ZVBoxLayout&&) = delete;

public:
    int spacing() const;
    void setSpacing(int sp);

    void addWidget(ZWidget *w);
    void add(ZLayout *l);
    void addSpacing(int size);
    void addStretch();

    void setGeometry(QRect toFill) override;
    QSize sizeHint() const override;
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool isSpacer() const override;;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void widgetEvent(QEvent *event) override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZVBoxLayout)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZVBOXLAYOUT_INCLUDED
