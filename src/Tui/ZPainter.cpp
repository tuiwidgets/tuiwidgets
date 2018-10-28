#include "ZPainter.h"
#include <Tui/ZPainter_p.h>

#include <QRect>

#include <Tui/ZColor.h>
#include <Tui/ZTerminal_p.h>

TUIWIDGETS_NS_START

namespace {
    int toTermPaintColor(ZColor color) {
        return color.nativeValue();
    }
}

ZPainterPrivate::ZPainterPrivate(ZTerminalPrivate *terminal, termpaint_surface *surface, int width, int height)
    : terminal(terminal), surface(surface), x(0), y(0), width(width), height(height)
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

ZPainter ZPainter::translateAndClip(QRect transform) {
    return translateAndClip(transform.x(), transform.y(), transform.width(), transform.height());
}

ZPainter ZPainter::translateAndClip(int x, int y, int width, int height) {
    ZPainter ret = *this;

    auto *const pimpl = ret.tuiwidgets_impl();

    // translate
    pimpl->width -= x;
    pimpl->height -= y;
    pimpl->x += x;
    pimpl->y += y;

    // clip
    pimpl->width = std::min(pimpl->width, width);
    pimpl->height = std::min(pimpl->height, height);

    return ret;
}

void ZPainter::writeWithColors(int x, int y, QString string, ZColor fg, ZColor bg) {
    auto *const pimpl = tuiwidgets_impl();
    if (y >= pimpl->height) return;

    termpaint_surface_write_with_colors_clipped(pimpl->surface,
                                             x + pimpl->x, y + pimpl->y,
                                             string.toUtf8().data(),
                                             toTermPaintColor(fg), toTermPaintColor(bg),
                                             pimpl->x, pimpl->x + pimpl->width - 1);
}

void ZPainter::clear(ZColor fg, ZColor bg) {
    auto *const pimpl = tuiwidgets_impl();
    termpaint_surface_clear_rect(pimpl->surface,
                                 pimpl->x, pimpl->y, pimpl->width, pimpl->height,
                                 toTermPaintColor(fg), toTermPaintColor(bg));
}

void ZPainter::clearRect(int x, int y, int width, int height, ZColor fg, ZColor bg) {
    auto *const pimpl = tuiwidgets_impl();
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    width = std::min(pimpl->width - x, width);
    height = std::min(pimpl->height, height);
    if (width < 0 || height < 0) {
        return;
    }
    x += pimpl->x;
    y += pimpl->y;

    termpaint_surface_clear_rect(pimpl->surface,
                                 x, y, width, height,
                                 toTermPaintColor(fg), toTermPaintColor(bg));
}

void ZPainter::setCursor(int x, int y) {
    auto *const pimpl = tuiwidgets_impl();

    if (x >= 0 && x <= pimpl->width && y >= 0 && y <= pimpl->height) {
        pimpl->terminal->cursorPosition = {pimpl->x + x, pimpl->y + y};
    }
}


TUIWIDGETS_NS_END
