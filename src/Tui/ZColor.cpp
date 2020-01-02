#include "ZColor.h"

#include <termpaint.h>

TUIWIDGETS_NS_START

ZColor::ZColor(int r, int g, int b)
    : val(TERMPAINT_RGB_COLOR_OFFSET | static_cast<uint8_t>(r) << 16 | static_cast<uint8_t>(g) << 8 | static_cast<uint8_t>(b))
{
}

ZColor::ZColor(Private::GlobalColorRGB globalColor)
    : ZColor(globalColor.r, globalColor.g, globalColor.b)
{
}

int ZColor::red() const {
    return val >> 16 & 0xff;
}

void ZColor::setRed(int red) {
    val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0x00ffff) | static_cast<uint8_t>(red) << 16;
}

int ZColor::green() const {
    return val >> 8 & 0xff;
}

void ZColor::setGreen(int green) {
    val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xff00ff) | static_cast<uint8_t>(green) << 8;
}

int ZColor::blue() const {
    return val & 0xff;
}

void ZColor::setBlue(int blue) {
    val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xffff00) | static_cast<uint8_t>(blue);
}

uint32_t ZColor::nativeValue() {
    return val;
}

ZColor ZColor::fromRgb(int r, int g, int b) {
    return ZColor(r, g, b);
}

ZColor ZColor::defaultColor() {
    ZColor col;
    col.val = TERMPAINT_DEFAULT_COLOR;
    return col;
}

bool ZColor::operator==(const ZColor &other) const {
    return val == other.val;
}

bool ZColor::operator!=(const ZColor &other) const {
    return val != other.val;
}

TUIWIDGETS_NS_END
