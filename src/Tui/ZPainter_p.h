#ifndef TUIWIDGETS_ZPAINTER_P_INCLUDED
#define TUIWIDGETS_ZPAINTER_P_INCLUDED

#include <QPointer>

#include <termpaint.h>

#include <Tui/ZPainter.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZTerminalPrivate;

class ZPainterPrivate {
public:
    ZPainterPrivate(termpaint_surface *surface, int width, int height, std::shared_ptr<char> token = nullptr);
    virtual ~ZPainterPrivate();

    std::shared_ptr<char> token;
    termpaint_surface *surface;
    // Clip rect and origin for translation
    int x = 0, y = 0, width = 0, height = 0;
    // Translation relative to top-left corner of clipping rect
    int offsetX = 0, offsetY = 0;

    QPointer<ZWidget> widget;

    // back door
    static ZPainterPrivate *get(ZPainter *painter) { return painter->tuiwidgets_impl(); }
    static ZPainter createForTesting(termpaint_surface *surface);

    ZPainter *pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZPainter)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPAINTER_P_INCLUDED
