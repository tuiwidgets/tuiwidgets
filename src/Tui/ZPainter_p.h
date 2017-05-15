#ifndef TUIWIDGETS_ZPAINTER_P_INCLUDED
#define TUIWIDGETS_ZPAINTER_P_INCLUDED

#include <termpaint.h>

#include <Tui/ZPainter.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZPainterPrivate {
public:
    ZPainterPrivate(termpaint_surface *surface, int width, int height);
    virtual ~ZPainterPrivate();

    termpaint_surface *surface;
    int x, y, width, height;

    ZPainter* pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZPainter)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPAINTER_P_INCLUDED
