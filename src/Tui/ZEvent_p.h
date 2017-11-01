#ifndef TUIWIDGETS_ZEVENT_P_INCLUDED
#define TUIWIDGETS_ZEVENT_P_INCLUDED

#include <QPoint>
#include <QSize>
#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZPainter;

class ZEventPrivate {
public:
    virtual ~ZEventPrivate();
};

class ZRawSequenceEventPrivate : public ZEventPrivate {
public:
    ZRawSequenceEventPrivate(QString seq);
    QString sequence;
};

class ZPaintEventPrivate : public ZEventPrivate {
public:
    ZPaintEventPrivate(ZPainter *painter);
    ZPainter *painter;
};

class ZResizeEventPrivate : public ZEventPrivate {
public:
    ZResizeEventPrivate(QSize size, QSize oldSize);

public:
    QSize size;
    QSize oldSize;
};

class ZMoveEventPrivate : public ZEventPrivate {
public:
    ZMoveEventPrivate(QPoint pos, QPoint oldPos);

public:
    QPoint pos;
    QPoint oldPos;
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZEVENT_P_INCLUDED
