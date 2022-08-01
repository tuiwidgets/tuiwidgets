#ifndef TUIWIDGETS_ZWINDOWLAYOUT_INCLUDED
#define TUIWIDGETS_ZWINDOWLAYOUT_INCLUDED

#include <Tui/ZLayout.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowLayoutPrivate;

class TUIWIDGETS_EXPORT ZWindowLayout : public ZLayout {
    Q_OBJECT

public:
    ZWindowLayout();
    ~ZWindowLayout() override;

    void setCentralWidget(ZWidget *w);
    void setCentral(ZLayoutItem *li);
    void setRightBorderWidget(ZWidget *w, Qt::Alignment align = Qt::Alignment());
    void setBottomBorderWidget(ZWidget *w, Qt::Alignment align = Qt::Alignment());
    void setTopBorderWidget(ZWidget *w, Qt::Alignment align = Qt::AlignHCenter);

    void setGeometry(QRect toFill) override;
    QSize sizeHint() const override;

    int topBorderLeftAdjust() const;
    void setTopBorderLeftAdjust(int topBorderLeftAdjust);

    int topBorderRightAdjust() const;
    void setTopBorderRightAdjust(int topBorderRightAdjust);

    int rightBorderTopAdjust() const;
    void setRightBorderTopAdjust(int rightBorderTopAdjust);

    int rightBorderBottomAdjust() const;
    void setRightBorderBottomAdjust(int rightBorderBottomAdjust);

    int bottomBorderLeftAdjust() const;
    void setBottomBorderLeftAdjust(int bottomBorderLeftAdjust);

    int bottomBorderRightAdjust() const;
    void setBottomBorderRightAdjust(int bottomBorderRightAdjust);

public:
    // public virtuals from base class override everything for later ABI compatibility
    SizePolicy sizePolicyH() const override;
    SizePolicy sizePolicyV() const override;
    bool isVisible() const override;
    bool isSpacer() const override;
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
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowLayout)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWLAYOUT_INCLUDED
