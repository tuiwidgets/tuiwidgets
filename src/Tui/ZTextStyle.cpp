#include "ZTextStyle.h"
#include "ZTextStyle_p.h"

TUIWIDGETS_NS_START

ZTextStyle::ZTextStyle() {
}

ZTextStyle::ZTextStyle(ZColor fg, ZColor bg) {
    auto *const p = tuiwidgets_impl();
    p->foregroundColor = fg;
    p->backgroundColor = bg;
}

ZTextStyle::ZTextStyle(ZColor fg, ZColor bg, ZTextAttributes attributes) {
    auto *const p = tuiwidgets_impl();
    p->foregroundColor = fg;
    p->backgroundColor = bg;
    p->attributes = attributes;
}

ZTextStyle::ZTextStyle(const ZTextStyle&) = default;
ZTextStyle::ZTextStyle(ZTextStyle&&) = default;
ZTextStyle::~ZTextStyle() = default;

ZTextStyle &ZTextStyle::operator=(const ZTextStyle&) = default;
ZTextStyle &ZTextStyle::operator=(ZTextStyle&&) = default;

ZColor ZTextStyle::foregroundColor() const {
    auto *const p = tuiwidgets_impl();
    return p->foregroundColor;
}

void ZTextStyle::setForegroundColor(const ZColor &foregroundColor) {
    auto *const p = tuiwidgets_impl();
    p->foregroundColor = foregroundColor;
}

ZColor ZTextStyle::backgroundColor() const {
    auto *const p = tuiwidgets_impl();
    return p->backgroundColor;
}

void ZTextStyle::setBackgroundColor(const ZColor &backgroundColor) {
    auto *const p = tuiwidgets_impl();
    p->backgroundColor = backgroundColor;
}

ZTextAttributes ZTextStyle::attributes() const {
    auto *const p = tuiwidgets_impl();
    return p->attributes;
}

void ZTextStyle::setAttributes(ZTextAttributes attributes) {
    auto *const p = tuiwidgets_impl();
    p->attributes = attributes;
}

bool ZTextStyle::operator==(const ZTextStyle &other) const {
    return foregroundColor() == other.foregroundColor()
        && backgroundColor() == other.backgroundColor()
        && attributes() == other.attributes();
}

TUIWIDGETS_NS_END
