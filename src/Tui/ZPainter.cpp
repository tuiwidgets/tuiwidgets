#include "ZPainter.h"

#include <Tui/ZPainter_p.h>


TUIWIDGETS_NS_START

ZPainterPrivate::ZPainterPrivate(termpaint_surface *surface, int width, int height)
    : surface(surface), x(0), y(0), width(width), height(height)
{

}

ZPainterPrivate::~ZPainterPrivate()
{

}

ZPainter::ZPainter(std::unique_ptr<ZPainterPrivate> impl)
    : tuiwidgets_pimpl_ptr(move(impl))
{
}


ZPainter::ZPainter(const ZPainter &other)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZPainterPrivate>(*other.tuiwidgets_pimpl_ptr)) // FIXME proper clone support
{
}

ZPainter::~ZPainter() {
}

ZPainter ZPainter::translateAndClip(int x, int y, int width, int height) {
    ZPainter ret = *this;

    auto pimpl = ret.tuiwidgets_impl();
    pimpl->width -= x;
    pimpl->height -= y;
    pimpl->x += x;
    pimpl->y += y;

    return ret;
}

void ZPainter::writeWithColors(int x, int y, QString string, int fg, int bg) {
    auto pimpl = tuiwidgets_impl();
    termpaint_surface_write_with_colors(pimpl->surface, x + pimpl->x, y + pimpl->y, string.toUtf8().data(), fg, bg);
}

void ZPainter::clear(int bg) {
    auto pimpl = tuiwidgets_impl();
    termpaint_surface_clear(pimpl->surface, bg);
}


void ZPainter::flush() {
    termpaint_surface_flush(tuiwidgets_impl()->surface);
}


TUIWIDGETS_NS_END
