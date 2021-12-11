#include "ZPainter.h"
#include <Tui/ZPainter_p.h>

#include <QRect>
#include <QTextCodec>

#include <Tui/ZColor.h>
#include <Tui/ZImage_p.h>
#include <Tui/ZTerminal_p.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextMetrics_p.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

namespace {
    int toTermPaintColor(ZColor color) {
        return color.nativeValue();
    }
}

ZPainterPrivate::ZPainterPrivate(termpaint_surface *surface, int width, int height, std::shared_ptr<char> token)
    : token(token), surface(surface), x(0), y(0), width(width), height(height), offsetX(0), offsetY(0)
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

    QRect startingClip = {pimpl->x, pimpl->y, pimpl->width, pimpl->height};

    QPoint offset = {pimpl->x + pimpl->offsetX, pimpl->y + pimpl->offsetY};
    offset += {x, y};

    QRect effectiveClip = startingClip.intersected({offset, QSize{std::max(width, 0), std::max(height, 0)}});
    pimpl->x = effectiveClip.x();
    pimpl->y = effectiveClip.y();
    pimpl->width = effectiveClip.width();
    pimpl->height = effectiveClip.height();

    offset -= QPoint{pimpl->x, pimpl->y};
    pimpl->offsetX = offset.x();
    pimpl->offsetY = offset.y();

    return ret;
}

void ZPainter::writeWithColors(int x, int y, const QString &string, ZColor fg, ZColor bg) {
    auto utf8 = string.toUtf8();
    writeWithColors(x, y, utf8.data(), utf8.size(), fg, bg);
}

void ZPainter::writeWithColors(int x, int y, const char *stringUtf8, int utf8CodeUnits, ZColor fg, ZColor bg) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (y >= pimpl->height || y < 0) return;

    termpaint_surface_write_with_len_colors_clipped(pimpl->surface,
                                                    x + pimpl->x, y + pimpl->y,
                                                    stringUtf8, utf8CodeUnits,
                                                    toTermPaintColor(fg), toTermPaintColor(bg),
                                                    pimpl->x, pimpl->x + pimpl->width - 1);
}

void ZPainter::writeWithColors(int x, int y, const QChar *string, int size, ZColor fg, ZColor bg) {
    QByteArray utf8 = QTextCodec::codecForMib(106)->fromUnicode(string, size);
    writeWithColors(x, y, utf8.data(), utf8.size(), fg, bg);
}

void ZPainter::writeWithColors(int x, int y, const char16_t *string, int size, ZColor fg, ZColor bg) {
    QByteArray utf8 = QTextCodec::codecForMib(106)->fromUnicode(reinterpret_cast<const QChar*>(string), size);
    writeWithColors(x, y, utf8.data(), utf8.size(), fg, bg);
}

void ZPainter::writeWithAttributes(int x, int y, const QString &string, ZColor fg, ZColor bg, Attributes attr) {
    auto utf8 = string.toUtf8();
    writeWithAttributes(x, y, utf8.data(), utf8.size(), fg, bg, attr);
}

void ZPainter::writeWithAttributes(int x, int y, const char *stringUtf8, int utf8CodeUnits, ZColor fg, ZColor bg, Attributes attr) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (y >= pimpl->height || y < 0) return;

    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);
    termpaint_surface_write_with_len_attr_clipped(pimpl->surface,
                                                  x + pimpl->x, y + pimpl->y,
                                                  stringUtf8, utf8CodeUnits,
                                                  termpaintAttr,
                                                  pimpl->x, pimpl->x + pimpl->width - 1);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::writeWithAttributes(int x, int y, const QChar *string, int size, ZColor fg, ZColor bg, Attributes attr) {
    QByteArray utf8 = QTextCodec::codecForMib(106)->fromUnicode(string, size);
    writeWithAttributes(x, y, utf8.data(), utf8.size(), fg, bg, attr);
}

void ZPainter::writeWithAttributes(int x, int y, const char16_t *string, int size, ZColor fg, ZColor bg, Attributes attr) {
    QByteArray utf8 = QTextCodec::codecForMib(106)->fromUnicode(reinterpret_cast<const QChar*>(string), size);
    writeWithAttributes(x, y, utf8.data(), utf8.size(), fg, bg, attr);
}

void ZPainter::clear(ZColor fg, ZColor bg, Attributes attr) {
    clearWithChar(fg, bg, Erased, attr);
}

void ZPainter::clearWithChar(ZColor fg, ZColor bg, int fillChar, Attributes attr) {
    auto *const pimpl = tuiwidgets_impl();
    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);
    termpaint_surface_clear_rect_with_attr_char(pimpl->surface,
                                 pimpl->x, pimpl->y, pimpl->width, pimpl->height,
                                 termpaintAttr, fillChar);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::clearRectWithChar(int x, int y, int width, int height, ZColor fg, ZColor bg, int fillChar, Attributes attr) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    width = std::min(pimpl->width - x, width);
    height = std::min(pimpl->height - y, height);
    if (width < 0 || height < 0) {
        return;
    }
    termpaint_attr *termpaintAttr = termpaint_attr_new(toTermPaintColor(fg), toTermPaintColor(bg));
    termpaint_attr_set_style(termpaintAttr, attr);

    x += pimpl->x;
    y += pimpl->y;

    termpaint_surface_clear_rect_with_attr_char(pimpl->surface,
                                 x, y, width, height,
                                 termpaintAttr, fillChar);
    termpaint_attr_free(termpaintAttr);
}

void ZPainter::clearRect(int x, int y, int width, int height, ZColor fg, ZColor bg, Attributes attr) {
    clearRectWithChar(x, y, width, height, fg, bg, Erased, attr);
}

void ZPainter::setSoftwrapMarker(int x, int y) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (x < 0) return;
    if (y < 0) return;
    if (x >= pimpl->width) return;
    if (y >= pimpl->height) return;
    termpaint_surface_set_softwrap_marker(pimpl->surface, pimpl->x + x, pimpl->y + y, true);
}

void ZPainter::clearSoftwrapMarker(int x, int y) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (x < 0) return;
    if (y < 0) return;
    if (x >= pimpl->width) return;
    if (y >= pimpl->height) return;
    termpaint_surface_set_softwrap_marker(pimpl->surface, pimpl->x + x, pimpl->y + y, false);
}

void ZPainter::drawImage(int x, int y, const ZImage &sourceImage, int sourceX, int sourceY, int width, int height) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (width == -1) {
        width = sourceImage.width();
    }
    if (height == -1) {
        height = sourceImage.height();
    }

    if (x < 0) {
        width += x;
        sourceX -= x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        sourceY -= y;
        y = 0;
    }

    width = std::min(pimpl->width - x, width);
    height = std::min(pimpl->height - y, height);

    if (width < 0 || height < 0) {
        return;
    }

    termpaint_surface_copy_rect(ZImageData::get(&sourceImage)->surface, sourceX, sourceY, width, height,
                                pimpl->surface, pimpl->x + x, pimpl->y + y,
                                TERMPAINT_COPY_NO_TILE, TERMPAINT_COPY_NO_TILE);
}

void ZPainter::setForeground(int x, int y, ZColor fg) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (x < 0) return;
    if (y < 0) return;
    if (x >= pimpl->width) return;
    if (y >= pimpl->height) return;
    termpaint_surface_set_fg_color(pimpl->surface, pimpl->x + x, pimpl->y + y, toTermPaintColor(fg));
}

void ZPainter::setBackground(int x, int y, ZColor bg) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (x < 0) return;
    if (y < 0) return;
    if (x >= pimpl->width) return;
    if (y >= pimpl->height) return;
    termpaint_surface_set_bg_color(pimpl->surface, pimpl->x + x, pimpl->y + y, toTermPaintColor(bg));
}

void ZPainter::setCursor(int x, int y) {
    auto *const pimpl = tuiwidgets_impl();

    x += pimpl->offsetX;
    y += pimpl->offsetY;

    if (pimpl->widget) {
        if (x >= 0 && x < pimpl->width && y >= 0 && y < pimpl->height) {
            QPoint widgetRelative = pimpl->widget->mapFromTerminal({pimpl->x + x, pimpl->y + y});
            pimpl->widget->showCursor(widgetRelative);
        }
    }
}

void ZPainter::setWidget(ZWidget *widget) {
    auto *const pimpl = tuiwidgets_impl();
    pimpl->widget = widget;
}

ZTextMetrics ZPainter::textMetrics() {
    auto *const pimpl = tuiwidgets_impl();
    return ZTextMetrics(std::make_shared<ZTextMetricsPrivate>(pimpl->surface));
}


ZPainter ZPainterPrivate::createForTesting(termpaint_surface *surface) {
    std::shared_ptr<char> token = std::make_shared<char>();
    return ZPainter(std::make_unique<ZPainterPrivate>(surface,
                                                      termpaint_surface_width(surface),
                                                      termpaint_surface_height(surface),
                                                      token));
}


TUIWIDGETS_NS_END
