#include "ZPainter.h"
#include <Tui/ZPainter_p.h>

#include <QRect>

#include <Tui/ZColor.h>
#include <Tui/ZTerminal_p.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

namespace {
    int toTermPaintColor(ZColor color) {
        return color.nativeValue();
    }
}

ZPainterPrivate::ZPainterPrivate(termpaint_surface *surface, int width, int height, std::shared_ptr<char> token)
    : token(token), surface(surface), x(0), y(0), width(width), height(height)
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

void ZPainter::writeWithAttributes(int x, int y, QString string, ZColor fg, ZColor bg, Attributes attr) {
    auto *const pimpl = tuiwidgets_impl();
    if (y >= pimpl->height) return;

    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);
    termpaint_surface_write_with_attr_clipped(pimpl->surface,
                                             x + pimpl->x, y + pimpl->y,
                                             string.toUtf8().data(),
                                             termpaintAttr,
                                             pimpl->x, pimpl->x + pimpl->width - 1);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::clear(ZColor fg, ZColor bg, Attributes attr) {
    auto *const pimpl = tuiwidgets_impl();
    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);
    termpaint_surface_clear_rect_with_attr(pimpl->surface,
                                 pimpl->x, pimpl->y, pimpl->width, pimpl->height,
                                 termpaintAttr);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::clearRect(int x, int y, int width, int height, ZColor fg, ZColor bg, Attributes attr) {
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
    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);

    x += pimpl->x;
    y += pimpl->y;

    termpaint_surface_clear_rect_with_attr(pimpl->surface,
                                 x, y, width, height,
                                 termpaintAttr);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::setCursor(int x, int y) {
    auto *const pimpl = tuiwidgets_impl();

    if (pimpl->widget) {
        if (x >= 0 && x <= pimpl->width && y >= 0 && y <= pimpl->height) {
            QPoint widgetRelative = pimpl->widget->mapFromTerminal({pimpl->x + x, pimpl->y + y});
            pimpl->widget->showCursor(widgetRelative);
        }
    }
}

void ZPainter::setWidget(ZWidget *widget) {
    auto *const pimpl = tuiwidgets_impl();
    pimpl->widget = widget;
}


TUIWIDGETS_NS_END
