#include "ZColor.h"

TUIWIDGETS_NS_START

ZColor::ZColor(int r, int g, int b)
    : val(static_cast<uint8_t>(r) << 16 | static_cast<uint8_t>(g) << 8 | static_cast<uint8_t>(b))
{
}

int ZColor::red() const {
    return val >> 16 & 0xff;
}

void ZColor::setRed(int red) {
    val = (val & 0x00ffff) | static_cast<uint8_t>(red) << 16;
}

int ZColor::green() const {
    return val >> 8 & 0xff;
}

void ZColor::setGreen(int green) {
    val = (val & 0xff00ff) | static_cast<uint8_t>(green) << 8;
}

int ZColor::blue() const {
    return val & 0xff;
}

void ZColor::setBlue(int blue) {
    val = (val & 0xffff00) | static_cast<uint8_t>(blue);
}

uint32_t ZColor::nativeValue() {
    return val;
}

ZColor ZColor::fromRgb(int r, int g, int b) {
    return ZColor(r, g, b);
}

ZColor ZColor::defaultColor() {
    ZColor col;
    col.val = 0x10000000;
    return col;
}

bool ZColor::operator==(const ZColor &other) const {
    return val == other.val;
}

bool ZColor::operator!=(const ZColor &other) const {
    return val != other.val;
}

namespace Colors {
    // TODO:
    Tui::ZColor black = {0, 0, 0};
    Tui::ZColor darkGray = {0x55, 0x55, 0x55};
    Tui::ZColor lightGray = {0xaa, 0xaa, 0xaa};
    Tui::ZColor brightWhite = {0xff, 0xff, 0xff};
    Tui::ZColor blue = {0, 0, 0xaa};
    Tui::ZColor green = {0, 0xaa, 0};
    Tui::ZColor cyan = {0, 0xaa, 0xaa};
    Tui::ZColor red = {0xaa, 0, 0};
    Tui::ZColor magenta = {0xaa, 0, 0xaa};
    Tui::ZColor brown = {0xaa, 0x55, 0};
    Tui::ZColor brightBlue = {0x55, 0x55, 0xff};
    Tui::ZColor brightGreen = {0x55, 0xff, 0x55};
    Tui::ZColor brightCyan = {0x55, 0xff, 0xff};
    Tui::ZColor brightRed = {0xff, 0x55, 0x55};
    Tui::ZColor brightMagenta = {0xff, 0x55, 0xff};
    Tui::ZColor brightYellow = {0xff, 0xff, 0x55};
}

TUIWIDGETS_NS_END
