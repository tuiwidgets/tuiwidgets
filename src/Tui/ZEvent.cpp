#include "ZEvent.h"
#include "ZEvent_p.h"

TUIWIDGETS_NS_START


ZEventPrivate::~ZEventPrivate() {
}

ZEvent::ZEvent(Type type, std::unique_ptr<ZEventPrivate> pimpl)
    : QEvent(type), tuiwidgets_pimpl_ptr(move(pimpl))
{
}


ZEvent::~ZEvent() {
}

#define CALL_ONCE_REGISTEREVENTTYPE     \
    static Type event = (Type)QEvent::registerEventType(); \
    return event

QEvent::Type ZEvent::paint() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

QEvent::Type ZEvent::rawSequence() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

QEvent::Type ZEvent::pendingRawSequence() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

ZRawSequenceEvent::ZRawSequenceEvent(QString seq)
    : ZEvent(rawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

ZRawSequenceEvent::ZRawSequenceEvent(Pending, QString seq)
    : ZEvent(pendingRawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

QString ZRawSequenceEvent::sequence() {
    return tuiwidgets_impl()->sequence;
}

Tui::v0::ZRawSequenceEventPrivate::ZRawSequenceEventPrivate(QString seq)
    : sequence(seq)
{
}


TUIWIDGETS_NS_END
