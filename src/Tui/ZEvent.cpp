#include "ZEvent.h"
#include "ZEvent_p.h"

TUIWIDGETS_NS_START

#define CALL_ONCE_REGISTEREVENTTYPE     \
    static QEvent::Type event = (QEvent::Type)QEvent::registerEventType(); \
    return event

TUIWIDGETS_EXPORT QEvent::Type ZEventType::paint() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::rawSequence() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::pendingRawSequence() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

ZEventPrivate::~ZEventPrivate() {
}

ZEvent::ZEvent(Type type, std::unique_ptr<ZEventPrivate> pimpl)
    : QEvent(type), tuiwidgets_pimpl_ptr(move(pimpl))
{
}


ZEvent::~ZEvent() {
}


ZRawSequenceEvent::ZRawSequenceEvent(QString seq)
    : ZEvent(ZEventType::rawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

ZRawSequenceEvent::ZRawSequenceEvent(Pending, QString seq)
    : ZEvent(ZEventType::pendingRawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

QString ZRawSequenceEvent::sequence() {
    return tuiwidgets_impl()->sequence;
}

Tui::ZRawSequenceEventPrivate::ZRawSequenceEventPrivate(QString seq)
    : sequence(seq)
{
}



TUIWIDGETS_NS_END
