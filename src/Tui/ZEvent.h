#ifndef TUIWIDGETS_ZEVENT_INCLUDED
#define TUIWIDGETS_ZEVENT_INCLUDED

#include <memory>

#include <QEvent>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZEventPrivate;
class ZRawSequenceEventPrivate;

namespace ZEventType {
    QEvent::Type paint();
    QEvent::Type rawSequence();
    QEvent::Type pendingRawSequence();
}

class TUIWIDGETS_EXPORT ZEvent : public QEvent {
public:
    virtual ~ZEvent();

protected:
    ZEvent(Type type, std::unique_ptr<ZEventPrivate> pimpl);
    std::unique_ptr<ZEventPrivate> tuiwidgets_pimpl_ptr;
private:
    TUIWIDGETS_DECLARE_PRIVATE(ZEvent)
};

// TODO think about a nativeEventFilter like infrastructure on ZTerminal
class TUIWIDGETS_EXPORT ZRawSequenceEvent : public ZEvent {
public:
    class Pending{}; static constexpr Pending pending {};

    ZRawSequenceEvent(QString seq);
    ZRawSequenceEvent(Pending, QString seq);


public:
    QString sequence();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZRawSequenceEvent)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZEVENT_INCLUDED
