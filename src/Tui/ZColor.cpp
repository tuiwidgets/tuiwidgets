#include "ZColor.h"

#include <array>

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

ZColor::ZColor(TerminalColor color) : val(ZColor::fromTerminalColor(color).val) {
}

ZColor::ColorType ZColor::colorType() const {
    static_assert ((TERMPAINT_NAMED_COLOR & 0xff000000) == (TERMPAINT_INDEXED_COLOR & 0xff000000),
                   "TERMPAINT_NAMED_COLOR or TERMPAINT_INDEXED_COLOR not as expected");
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return ColorType::RGB;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return ColorType::Terminal;
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                return ColorType::TerminalIndexed;
            }
            [[fallthrough]];
        default:
            return ColorType::Default;
    }
}

int ZColor::red() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val >> 16 & 0xff;
}

void ZColor::setRed(int red) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(red, 0, 0);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0x00ffff) | static_cast<uint8_t>(red) << 16;
    }
}

int ZColor::green() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val >> 8 & 0xff;
}

void ZColor::setGreen(int green) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(0, green, 0);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xff00ff) | static_cast<uint8_t>(green) << 8;
    }
}

int ZColor::blue() const {
    if (colorType() != ColorType::RGB) {
        return 0;
    }
    return val & 0xff;
}

void ZColor::setBlue(int blue) {
    if (colorType() != ColorType::RGB) {
        val = TERMPAINT_RGB_COLOR(0, 0, blue);
    } else {
        val = TERMPAINT_RGB_COLOR_OFFSET | (val & 0xffff00) | static_cast<uint8_t>(blue);
    }
}

int ZColor::terminalColorIndexed() {
    if (colorType() == ColorType::TerminalIndexed) {
        return val & 0xff;
    } else {
        return 0;
    }
}

TerminalColor ZColor::terminalColor() {
    if (colorType() == ColorType::Terminal) {
        return static_cast<TerminalColor>(val & 0xf);
    } else {
        return TerminalColor::black;
    }
}

static const std::array<uint8_t, 16> guessR = {
    0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa,
    0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff
};

static const std::array<uint8_t, 16> guessG = {
    0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00, 0xaa, 0xaa,
    0x55, 0x55, 0xff, 0xff, 0x55, 0x55, 0xff, 0xff
};

static const std::array<uint8_t, 16> guessB = {
    0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa,
    0x55, 0x55, 0x55, 0x55, 0xff, 0xff, 0xff, 0xff
};

static const std::array<uint8_t, 6> guessCube = {
    0, 95, 135, 175, 215, 255
};

static const std::array<uint8_t, 24> guessGrey = {
    8,   18,  28,  38,  48,  58,  68,  78,  88,  98,  108, 118,
    128, 138, 148, 158, 168, 178, 188, 198, 208, 218, 228, 238
};

int ZColor::redOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val >> 16 & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessR[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessR[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[(idx-16) / 36];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
}

int ZColor::greenOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val >> 8 & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessG[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessG[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[((idx-16) / 6) % 6];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
}

int ZColor::blueOrGuess() const {
    switch (val & 0xff000000) {
        case TERMPAINT_RGB_COLOR_OFFSET:
            return val & 0xff;
        case TERMPAINT_NAMED_COLOR & 0xff000000:
            if ((val & 0xfff00000) == TERMPAINT_NAMED_COLOR) {
                return guessB[val & 0xf];
            } else if ((val & 0xfff00000) == TERMPAINT_INDEXED_COLOR) {
                int idx = val & 0xff;
                if (idx < 16) {
                    return guessB[idx];
                } else if (idx >= 232) {
                    return guessGrey[idx - 232];
                } else {
                    return guessCube[(idx-16) % 6];
                }
            }
            [[fallthrough]];
        default:
            // nothing much we can do here
            return 0x80;
    }
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

ZColor ZColor::fromTerminalColor(TerminalColor color) {
    return fromTerminalColor(static_cast<int>(color));
}

ZColor ZColor::fromTerminalColor(int color) {
    ZColor col;
    col.val = TERMPAINT_NAMED_COLOR | (color & 0xf);
    return col;
}

ZColor ZColor::fromTerminalColorIndexed(int color) {
    ZColor col;
    col.val = TERMPAINT_INDEXED_COLOR | (color & 0xff);
    return col;
}

bool ZColor::operator==(const ZColor &other) const {
    return val == other.val;
}

bool ZColor::operator!=(const ZColor &other) const {
    return val != other.val;
}

TUIWIDGETS_NS_END
