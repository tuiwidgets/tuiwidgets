// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZEVENT_INCLUDED
#define TUIWIDGETS_ZEVENT_INCLUDED

#include <memory>

#include <QEvent>
#include <QStringList>

#include <Tui/ZCommon.h>

#include <Tui/tuiwidgets_internal.h>

class QPoint;
class QSize;

TUIWIDGETS_NS_START

class ZPainter;
class ZSymbol;

class ZEventPrivate;
class ZRawSequenceEventPrivate;
class ZTerminalNativeEventPrivate;
class ZPaintEventPrivate;
class ZKeyEventPrivate;
class ZPasteEventPrivate;
class ZFocusEventPrivate;
class ZMoveEventPrivate;
class ZResizeEventPrivate;
class ZCloseEventPrivate;
class ZOtherChangeEventPrivate;

namespace ZEventType {
    QEvent::Type paint();
    QEvent::Type key();
    QEvent::Type paste();
    QEvent::Type queryAcceptsEnter();
    QEvent::Type focusIn();
    QEvent::Type focusOut();
    QEvent::Type move();
    QEvent::Type resize();
    QEvent::Type otherChange();
    QEvent::Type show();
    QEvent::Type hide();
    QEvent::Type close();
    QEvent::Type terminalChange();

    QEvent::Type updateRequest();
    QEvent::Type terminalNativeEvent();
    QEvent::Type rawSequence();
    QEvent::Type pendingRawSequence();
}

class TUIWIDGETS_EXPORT ZEvent : public QEvent {
public:
    ZEvent(const ZEvent&) = delete; // to allow this (like QEvent) this class would need to be rewritten to support copying
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

    ZRawSequenceEvent(QByteArray seq);
    ZRawSequenceEvent(Pending, QByteArray seq);
    ~ZRawSequenceEvent() override;


public:
    QByteArray sequence() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZRawSequenceEvent)
};

class TUIWIDGETS_EXPORT ZTerminalNativeEvent : public ZEvent {
public:
    ZTerminalNativeEvent(void *native);
    ~ZTerminalNativeEvent() override;

public:
    void *nativeEventPointer() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTerminalNativeEvent)
};

class TUIWIDGETS_EXPORT ZPaintEvent : public ZEvent {
public:
    class Update{}; static constexpr Update update {};

    ZPaintEvent(ZPainter *painter);
    ZPaintEvent(Update, ZPainter *painter);
    ~ZPaintEvent() override;

public:
    ZPainter *painter() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPaintEvent)
};

class TUIWIDGETS_EXPORT ZKeyEvent : public ZEvent {
public:
    ZKeyEvent(int key, KeyboardModifiers modifiers, const QString &text);
    ~ZKeyEvent() override;

public:
    int key() const;
    QString text() const;
    KeyboardModifiers modifiers() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZKeyEvent)
};

class TUIWIDGETS_EXPORT ZPasteEvent : public ZEvent {
public:
    ZPasteEvent(const QString &text);
    ~ZPasteEvent() override;

public:
    QString text() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPasteEvent)
};

class TUIWIDGETS_EXPORT ZFocusEvent : public ZEvent {
public:
    class FocusIn{}; static constexpr FocusIn focusIn {};
    class FocusOut{}; static constexpr FocusOut focusOut {};
    ZFocusEvent(FocusIn, FocusReason reason = OtherFocusReason);
    ZFocusEvent(FocusOut, FocusReason reason = OtherFocusReason);
    ~ZFocusEvent() override;

public:
    FocusReason reason() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZFocusEvent)
};

class TUIWIDGETS_EXPORT ZResizeEvent : public ZEvent {
public:
    ZResizeEvent(QSize size, QSize oldSize);
    ~ZResizeEvent() override;

public:
    QSize size() const;
    QSize oldSize() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZResizeEvent)
};

class TUIWIDGETS_EXPORT ZMoveEvent : public ZEvent {
public:
    ZMoveEvent(QPoint pos, QPoint oldPos);
    ~ZMoveEvent() override;

public:
    QPoint pos() const;
    QPoint oldPos() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZMoveEvent)
};

class TUIWIDGETS_EXPORT ZCloseEvent : public ZEvent {
public:
    ZCloseEvent(QStringList skipChecks);
    ~ZCloseEvent() override;

public:
    QStringList skipChecks() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZCloseEvent)
};

class TUIWIDGETS_EXPORT ZOtherChangeEvent : public ZEvent {
public:
    ZOtherChangeEvent(QSet<ZSymbol> unchanged);
    ~ZOtherChangeEvent() override;
    static QSet<ZSymbol> all();

public:
    QSet<ZSymbol> unchanged() const;

    static bool match(const QEvent *event, ZSymbol changed);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZOtherChangeEvent)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZEVENT_INCLUDED
