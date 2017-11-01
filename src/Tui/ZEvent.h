#ifndef TUIWIDGETS_ZEVENT_INCLUDED
#define TUIWIDGETS_ZEVENT_INCLUDED

#include <memory>

#include <QEvent>

#include <Tui/tuiwidgets_internal.h>

class QPoint;
class QSize;

TUIWIDGETS_NS_START

class ZPainter;

class ZEventPrivate;
class ZRawSequenceEventPrivate;
class ZPaintEventPrivate;
class ZMoveEventPrivate;
class ZResizeEventPrivate;

namespace ZEventType {
    QEvent::Type paint();
    QEvent::Type move();
    QEvent::Type resize();

    QEvent::Type updateRequest();
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

class TUIWIDGETS_EXPORT ZPaintEvent : public ZEvent {
public:
    class Update{}; static constexpr Update update {};

    ZPaintEvent(ZPainter *painter);
    ZPaintEvent(Update, ZPainter *painter);

public:
    ZPainter *painter() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPaintEvent)
};

class TUIWIDGETS_EXPORT ZResizeEvent : public ZEvent {
public:
    ZResizeEvent(QSize size, QSize oldSize);

public:
    QSize size() const;
    QSize oldSize() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZResizeEvent)
};

class TUIWIDGETS_EXPORT ZMoveEvent : public ZEvent {
public:
    ZMoveEvent(QPoint pos, QPoint oldPos);

public:
    QPoint pos() const;
    QPoint oldPos() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZMoveEvent)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZEVENT_INCLUDED
