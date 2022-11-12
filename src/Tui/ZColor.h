// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCOLOR_INCLUDED
#define TUIWIDGETS_ZCOLOR_INCLUDED

#include <cstdint>

#include <QtGlobal>
#include <QMetaType>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZColor;

namespace Private {
    class GlobalColorRGB {
    public:
        constexpr GlobalColorRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    private:
        friend class ::TUIWIDGETS_NS_FULL::ZColor;

        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
}

enum class TerminalColor : int {
    black = 0,
    darkGray = 8,
    lightGray = 7,
    brightWhite = 15,
    blue = 4,
    green = 2,
    cyan = 6,
    red = 1,
    magenta = 5,
    yellow = 3,
    brightBlue = 12,
    brightGreen = 10,
    brightCyan = 14,
    brightRed = 9,
    brightMagenta = 13,
    brightYellow = 11
};

class TUIWIDGETS_EXPORT ZColorHSV {
public:
    ZColorHSV();
    ZColorHSV(const ZColorHSV&);
    ZColorHSV(double hue, double saturation, double value);
    ~ZColorHSV();

    ZColorHSV& operator=(const ZColorHSV&);

public:
    double hue() const;
    void setHue(double hue);

    double saturation() const;
    void setSaturation(double saturation);

    double value() const;
    void setValue(double value);

    bool operator==(const ZColorHSV &other) const;
    bool operator!=(const ZColorHSV &other) const;

private:
    double _hue = 0;
    double _saturation = 0;
    double _value = 0;
};

class TUIWIDGETS_EXPORT ZColor {
public:
    enum ColorType : int {
        RGB,
        Default,
        Terminal,
        TerminalIndexed
    };

public:
    ZColor();
    ZColor(const ZColor&);
    ZColor(int r, int g, int b);
    ZColor(Private::GlobalColorRGB globalColor);
    ZColor(TerminalColor color);
    ~ZColor();

    ZColor& operator=(const ZColor&);

public:
    ColorType colorType() const;

    // Only for colorType() == ColorType::RGB
    int red() const;
    void setRed(int red);
    int green() const;
    void setGreen(int green);
    int blue() const;
    void setBlue(int blue);

    // Only for colorType() == ColorType::TerminalIndexed
    int terminalColorIndexed() const;

    // Only for colorType() == ColorType::Terminal
    TerminalColor terminalColor() const;

    int redOrGuess() const;
    int greenOrGuess() const;
    int blueOrGuess() const;

    ZColorHSV toHsv() const;

    uint32_t nativeValue() const;

    static ZColor defaultColor();
    static ZColor fromRgb(int r, int g, int b);
    static ZColor fromTerminalColor(TerminalColor color);
    static ZColor fromTerminalColor(int color);
    static ZColor fromTerminalColorIndexed(int color);

    static ZColor fromHsvStrict(double hue, double saturation, double value);
    static ZColor fromHsvStrict(const ZColorHSV &hsv);
    static ZColor fromHsv(double hue, double saturation, double value);
    static ZColor fromHsv(const ZColorHSV &hsv);

    bool operator==(const ZColor &other) const;
    bool operator!=(const ZColor &other) const;

private:
    uint32_t val = 0;
    friend class ZImage;
};

namespace Colors {
    static constexpr Private::GlobalColorRGB black = {0, 0, 0};
    static constexpr Private::GlobalColorRGB darkGray = {0x55, 0x55, 0x55};
    static constexpr Private::GlobalColorRGB lightGray = {0xaa, 0xaa, 0xaa};
    static constexpr Private::GlobalColorRGB brightWhite = {0xff, 0xff, 0xff};
    static constexpr Private::GlobalColorRGB blue = {0, 0, 0xaa};
    static constexpr Private::GlobalColorRGB green = {0, 0xaa, 0};
    static constexpr Private::GlobalColorRGB cyan = {0, 0xaa, 0xaa};
    static constexpr Private::GlobalColorRGB red = {0xaa, 0, 0};
    static constexpr Private::GlobalColorRGB magenta = {0xaa, 0, 0xaa};
    static constexpr Private::GlobalColorRGB brown = {0xaa, 0x55, 0};
    static constexpr Private::GlobalColorRGB yellow = {0xaa, 0xaa, 0};
    static constexpr Private::GlobalColorRGB brightBlue = {0x55, 0x55, 0xff};
    static constexpr Private::GlobalColorRGB brightGreen = {0x55, 0xff, 0x55};
    static constexpr Private::GlobalColorRGB brightCyan = {0x55, 0xff, 0xff};
    static constexpr Private::GlobalColorRGB brightRed = {0xff, 0x55, 0x55};
    static constexpr Private::GlobalColorRGB brightMagenta = {0xff, 0x55, 0xff};
    static constexpr Private::GlobalColorRGB brightYellow = {0xff, 0xff, 0x55};
}

namespace Color = Colors;

TUIWIDGETS_NS_END

Q_DECLARE_METATYPE(TUIWIDGETS_NS_FULL::ZColor);

#endif // TUIWIDGETS_ZCOLOR_INCLUDED
