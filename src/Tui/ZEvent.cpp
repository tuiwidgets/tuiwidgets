// SPDX-License-Identifier: BSL-1.0

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

TUIWIDGETS_EXPORT QEvent::Type ZEventType::paste() {
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

TUIWIDGETS_EXPORT QEvent::Type ZEventType::show() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::hide() {
    CALL_ONCE_REGISTEREVENTTYPE;
}

TUIWIDGETS_EXPORT QEvent::Type ZEventType::close() {
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


ZRawSequenceEvent::ZRawSequenceEvent(QByteArray seq)
    : ZEvent(ZEventType::rawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

ZRawSequenceEvent::ZRawSequenceEvent(Pending, QByteArray seq)
    : ZEvent(ZEventType::pendingRawSequence(), std::make_unique<ZRawSequenceEventPrivate>(seq))
{
}

ZRawSequenceEvent::~ZRawSequenceEvent() {
}

QByteArray ZRawSequenceEvent::sequence() const {
    return tuiwidgets_impl()->sequence;
}

ZRawSequenceEventPrivate::ZRawSequenceEventPrivate(QByteArray seq)
    : sequence(seq)
{
}

ZRawSequenceEventPrivate::~ZRawSequenceEventPrivate() {
}

ZPaintEvent::ZPaintEvent(ZPainter *painter)
    : ZEvent(ZEventType::paint(), std::make_unique<ZPaintEventPrivate>(painter))
{
}

ZPaintEvent::ZPaintEvent(ZPaintEvent::Update, ZPainter *painter)
    : ZEvent(ZEventType::updateRequest(), std::make_unique<ZPaintEventPrivate>(painter))
{
}

ZPaintEvent::~ZPaintEvent() {
}

ZPainter *ZPaintEvent::painter() const {
    return tuiwidgets_impl()->painter;
}

ZPaintEventPrivate::ZPaintEventPrivate(ZPainter *painter)
    : painter(painter)
{
}

ZPaintEventPrivate::~ZPaintEventPrivate() {
}

ZTerminalNativeEvent::ZTerminalNativeEvent(void *native)
    : ZEvent(ZEventType::terminalNativeEvent(), std::make_unique<ZTerminalNativeEventPrivate>(native))
{
}

ZTerminalNativeEvent::~ZTerminalNativeEvent() {
}

void *ZTerminalNativeEvent::nativeEventPointer() const {
    return tuiwidgets_impl()->native;
}

ZTerminalNativeEventPrivate::ZTerminalNativeEventPrivate(void *native)
    : native(native)
{
}

ZTerminalNativeEventPrivate::~ZTerminalNativeEventPrivate() {
}

ZKeyEventPrivate::ZKeyEventPrivate(int key, Tui::KeyboardModifiers modifiers, const QString &text)
    : key(key), text(text), modifiers(modifiers)
{
}

ZKeyEventPrivate::~ZKeyEventPrivate() {
}

ZKeyEvent::ZKeyEvent(int key, Tui::KeyboardModifiers modifiers, const QString &text)
    : ZEvent(ZEventType::key(), std::make_unique<ZKeyEventPrivate>(key, modifiers, text))
{
}

ZKeyEvent::~ZKeyEvent() {
}

int ZKeyEvent::key() const {
    return tuiwidgets_impl()->key;
}

QString ZKeyEvent::text() const {
    return tuiwidgets_impl()->text;
}

Tui::KeyboardModifiers ZKeyEvent::modifiers() const {
    return tuiwidgets_impl()->modifiers;
}

ZPasteEventPrivate::ZPasteEventPrivate(const QString &text)
    : text(text)
{
}

ZPasteEventPrivate::~ZPasteEventPrivate() {
}

ZPasteEvent::ZPasteEvent(const QString &text)
    : ZEvent(ZEventType::paste(), std::make_unique<ZPasteEventPrivate>(text))
{
}

ZPasteEvent::~ZPasteEvent() {
}

QString ZPasteEvent::text() const {
    return tuiwidgets_impl()->text;
}

ZFocusEventPrivate::ZFocusEventPrivate(Tui::FocusReason reason)
    : reason(reason)
{
}

ZFocusEventPrivate::~ZFocusEventPrivate() {
}

ZFocusEvent::ZFocusEvent(ZFocusEvent::FocusIn, Tui::FocusReason reason)
    : ZEvent(ZEventType::focusIn(), std::make_unique<ZFocusEventPrivate>(reason))
{

}

ZFocusEvent::ZFocusEvent(ZFocusEvent::FocusOut, Tui::FocusReason reason)
    : ZEvent(ZEventType::focusOut(), std::make_unique<ZFocusEventPrivate>(reason))
{
}

ZFocusEvent::~ZFocusEvent() {
}

Tui::FocusReason ZFocusEvent::reason() const {
    return tuiwidgets_impl()->reason;
}

ZResizeEventPrivate::ZResizeEventPrivate(QSize size, QSize oldSize)
    : size(size), oldSize(oldSize)
{
}

ZResizeEventPrivate::~ZResizeEventPrivate() {
}

ZResizeEvent::ZResizeEvent(QSize size, QSize oldSize)
    : ZEvent(ZEventType::resize(), std::make_unique<ZResizeEventPrivate>(size, oldSize))
{
}

ZResizeEvent::~ZResizeEvent() {
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

ZMoveEventPrivate::~ZMoveEventPrivate() {
}

ZMoveEvent::ZMoveEvent(QPoint pos, QPoint oldPos)
    : ZEvent(ZEventType::move(), std::make_unique<ZMoveEventPrivate>(pos, oldPos))
{
}

ZMoveEvent::~ZMoveEvent() {
}

QPoint ZMoveEvent::pos() const {
    return tuiwidgets_impl()->pos;
}

QPoint ZMoveEvent::oldPos() const {
    return tuiwidgets_impl()->oldPos;
}

ZCloseEvent::ZCloseEvent(QStringList skipChecks)
    : ZEvent(ZEventType::close(), std::make_unique<ZCloseEventPrivate>(skipChecks))
{
}

ZCloseEvent::~ZCloseEvent() {
}

QStringList ZCloseEvent::skipChecks() const {
    return tuiwidgets_impl()->skipChecks;
}

ZCloseEventPrivate::ZCloseEventPrivate(QStringList skipChecks) : skipChecks(skipChecks) {
}

ZCloseEventPrivate::~ZCloseEventPrivate() {
}

ZOtherChangeEvent::ZOtherChangeEvent(QSet<ZSymbol> unchanged)
    : ZEvent(ZEventType::otherChange(), std::make_unique<ZOtherChangeEventPrivate>(unchanged))
{
}

ZOtherChangeEvent::~ZOtherChangeEvent() {
}

QSet<ZSymbol> ZOtherChangeEvent::all() {
    static QSet<ZSymbol> a = { TUISYM_LITERAL("terminal") };
    return a;
}

QSet<ZSymbol> ZOtherChangeEvent::unchanged() const {
    return tuiwidgets_impl()->unchanged;
}

bool ZOtherChangeEvent::match(const QEvent *event, ZSymbol changed) {
    if (!event || event->type() != ZEventType::otherChange()) return false;

    return !static_cast<const ZOtherChangeEvent&>(*event).unchanged().contains(changed);
}

ZOtherChangeEventPrivate::ZOtherChangeEventPrivate(QSet<ZSymbol> unchanged)
     : unchanged(unchanged)
{
}

ZOtherChangeEventPrivate::~ZOtherChangeEventPrivate() {
}

TUIWIDGETS_NS_END
