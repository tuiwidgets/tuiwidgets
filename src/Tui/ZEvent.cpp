#include "ZEvent.h"
#include "ZEvent_p.h"

#include <QPoint>
#include <QSize>
#include <QSet>

#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

#define CALL_ONCE_REGISTEREVENTTYPE     \
    static QEvent::Type event = static_cast<QEvent::Type>(QEvent::registerEventType()); \
    return event

TUIWIDGETS_EXPORT QEvent::Type ZEventType::paint() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::key() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::queryAcceptsEnter() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::focusIn() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::focusOut() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::move() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::resize() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::otherChange() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::updateRequest() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::terminalNativeEvent() {
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

QString ZRawSequenceEvent::sequence() const {
    return tuiwidgets_impl()->sequence;
}

Tui::ZRawSequenceEventPrivate::ZRawSequenceEventPrivate(QString seq)
    : sequence(seq)
{
}

ZPaintEvent::ZPaintEvent(ZPainter *painter)
    : ZEvent(ZEventType::paint(), std::make_unique<ZPaintEventPrivate>(painter))
{
}

ZPaintEvent::ZPaintEvent(ZPaintEvent::Update, ZPainter *painter)
    : ZEvent(ZEventType::updateRequest(), std::make_unique<ZPaintEventPrivate>(painter))
{
}

ZPainter *ZPaintEvent::painter() const {
    return tuiwidgets_impl()->painter;
}

ZPaintEventPrivate::ZPaintEventPrivate(ZPainter *painter)
    : painter(painter)
{
}

ZTerminalNativeEvent::ZTerminalNativeEvent(void *native)
    : ZEvent(ZEventType::terminalNativeEvent(), std::make_unique<ZTerminalNativeEventPrivate>(native))
{
}

void *ZTerminalNativeEvent::nativeEventPointer() const {
    return tuiwidgets_impl()->native;
}

ZTerminalNativeEventPrivate::ZTerminalNativeEventPrivate(void *native)
    : native(native)
{
}

ZKeyEventPrivate::ZKeyEventPrivate(int key, Qt::KeyboardModifiers modifiers, const QString &text)
    : key(key), text(text), modifiers(modifiers)
{
}

ZKeyEvent::ZKeyEvent(int key, Qt::KeyboardModifiers modifiers, const QString &text)
    : ZEvent(ZEventType::key(), std::make_unique<ZKeyEventPrivate>(key, modifiers, text))
{
}

int ZKeyEvent::key() const {
    return tuiwidgets_impl()->key;
}

QString ZKeyEvent::text() const {
    return tuiwidgets_impl()->text;
}

Qt::KeyboardModifiers ZKeyEvent::modifiers() const {
    return tuiwidgets_impl()->modifiers;
}

ZFocusEventPrivate::ZFocusEventPrivate(Qt::FocusReason reason)
    : reason(reason)
{
}

ZFocusEvent::ZFocusEvent(ZFocusEvent::FocusIn, Qt::FocusReason reason)
    : ZEvent(ZEventType::focusIn(), std::make_unique<ZFocusEventPrivate>(reason))
{

}

ZFocusEvent::ZFocusEvent(ZFocusEvent::FocusOut, Qt::FocusReason reason)
    : ZEvent(ZEventType::focusOut(), std::make_unique<ZFocusEventPrivate>(reason))
{
}

ZResizeEventPrivate::ZResizeEventPrivate(QSize size, QSize oldSize)
    : size(size), oldSize(oldSize)
{
}

ZResizeEvent::ZResizeEvent(QSize size, QSize oldSize)
    : ZEvent(ZEventType::resize(), std::make_unique<ZResizeEventPrivate>(size, oldSize))
{
}

QSize ZResizeEvent::size() const {
    return tuiwidgets_impl()->size;
}

QSize ZResizeEvent::oldSize() const {
    return tuiwidgets_impl()->oldSize;
}

ZMoveEventPrivate::ZMoveEventPrivate(QPoint pos, QPoint oldPos)
    : pos(pos), oldPos(oldPos)
{
}

ZMoveEvent::ZMoveEvent(QPoint pos, QPoint oldPos)
    : ZEvent(ZEventType::move(), std::make_unique<ZMoveEventPrivate>(pos, oldPos))
{
}

QPoint ZMoveEvent::pos() const {
    return tuiwidgets_impl()->pos;
}

QPoint ZMoveEvent::oldPos() const {
    return tuiwidgets_impl()->oldPos;
}

ZOtherChangeEvent::ZOtherChangeEvent(QSet<ZSymbol> unchanged)
    : ZEvent(ZEventType::otherChange(), std::make_unique<ZOtherChangeEventPrivate>(unchanged))
{
}

QSet<ZSymbol> ZOtherChangeEvent::all() {
    static QSet<ZSymbol> a = { TUISYM_LITERAL("terminal") };
    return a;
}

QSet<ZSymbol> ZOtherChangeEvent::unchanged() const {
    return tuiwidgets_impl()->unchanged;
}

ZOtherChangeEventPrivate::ZOtherChangeEventPrivate(QSet<ZSymbol> unchanged)
     : unchanged(unchanged)
{
}

TUIWIDGETS_NS_END
