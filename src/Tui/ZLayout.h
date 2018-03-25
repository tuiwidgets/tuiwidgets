#ifndef TUIWIDGETS_ZLAYOUT_INCLUDED
#define TUIWIDGETS_ZLAYOUT_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZLayoutPrivate;
class TUIWIDGETS_EXPORT ZLayout : public QObject {
    Q_OBJECT
public:
    explicit ZLayout(QObject *parent = nullptr);
    ~ZLayout() override;

public:
    virtual void widgetEvent(QEvent *event);

    ZWidget* widget();

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

protected:
    std::unique_ptr<ZLayoutPrivate> tuiwidgets_pimpl_ptr;

private:
    Q_DISABLE_COPY(ZLayout)
    TUIWIDGETS_DECLARE_PRIVATE(ZLayout)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUT_INCLUDED
