// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZEVENT_P_INCLUDED
#define TUIWIDGETS_ZEVENT_P_INCLUDED

#include <QPoint>
#include <QSize>
#include <QSet>
#include <QString>

#include <Tui/ZEvent.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZPainter;

class ZEventPrivate {
public:
    virtual ~ZEventPrivate();
};

class ZRawSequenceEventPrivate : public ZEventPrivate {
public:
    ZRawSequenceEventPrivate(QByteArray seq);
    ~ZRawSequenceEventPrivate() override;
    QByteArray sequence;
};

class ZTerminalNativeEventPrivate : public ZEventPrivate {
public:
    ZTerminalNativeEventPrivate(void *native);
    ~ZTerminalNativeEventPrivate() override;
    void *native;
};

class ZPaintEventPrivate : public ZEventPrivate {
public:
    ZPaintEventPrivate(ZPainter *painter);
    ~ZPaintEventPrivate() override;
    ZPainter *painter;
};

class ZKeyEventPrivate : public ZEventPrivate {
public:
    ZKeyEventPrivate(int key, Tui::KeyboardModifiers modifiers, const QString &text);
    ~ZKeyEventPrivate() override;

public:
    int key = Tui::Key_unknown;
    QString text;
    Tui::KeyboardModifiers modifiers = {};
};

class ZPasteEventPrivate : public ZEventPrivate {
public:
    ZPasteEventPrivate(const QString &text);
    ~ZPasteEventPrivate() override;

public:
    QString text;
};

class ZFocusEventPrivate : public ZEventPrivate {
public:
    ZFocusEventPrivate(Tui::FocusReason reason);
    ~ZFocusEventPrivate() override;

public:
    Tui::FocusReason reason;
};


class ZResizeEventPrivate : public ZEventPrivate {
public:
    ZResizeEventPrivate(QSize size, QSize oldSize);
    ~ZResizeEventPrivate() override;

public:
    QSize size;
    QSize oldSize;
};

class ZMoveEventPrivate : public ZEventPrivate {
public:
    ZMoveEventPrivate(QPoint pos, QPoint oldPos);
    ~ZMoveEventPrivate() override;

public:
    QPoint pos;
    QPoint oldPos;
};

class ZCloseEventPrivate : public ZEventPrivate {
public:
    ZCloseEventPrivate(QStringList skipChecks);
    ~ZCloseEventPrivate() override;

public:
    QStringList skipChecks;
};

class ZOtherChangeEventPrivate : public ZEventPrivate {
public:
    ZOtherChangeEventPrivate(QSet<ZSymbol> unchanged);
    ~ZOtherChangeEventPrivate() override;

public:
    QSet<ZSymbol> unchanged;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZEVENT_P_INCLUDED
