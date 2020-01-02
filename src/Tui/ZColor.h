#ifndef TUIWIDGETS_ZCOLOR_INCLUDED
#define TUIWIDGETS_ZCOLOR_INCLUDED

#include <cstdint>

#include <QtGlobal>
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

enum class TerminalColor {
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

class TUIWIDGETS_EXPORT ZColor {
    enum ColorType {
        RGB,
        Default,
        Terminal,
        TerminalIndexed
    };

public:
    ZColor() = default;
    ZColor(int r, int g, int b);
    ZColor(Private::GlobalColorRGB globalColor);
    ZColor(TerminalColor color);

    ColorType colorType() const;

    // Only for colorType() == ColorType::RGB
    int red() const;
    void setRed(int red);
    int green() const;
    void setGreen(int green);
    int blue() const;
    void setBlue(int blue);

    // Only for colorType() == ColorType::TerminalIndexed
    int terminalColorIndexed();

    // Only for colorType() == ColorType::Terminal
    TerminalColor terminalColor();

    uint32_t nativeValue();

    static ZColor fromRgb(int r, int g, int b);
    static ZColor defaultColor();
    static ZColor fromTerminalColor(TerminalColor color);
    static ZColor fromTerminalColor(int color);
    static ZColor fromTerminalColorIndexed(int color);

    bool operator==(const ZColor &other) const;
    bool operator!=(const ZColor &other) const;

private:
    uint32_t val = 0;
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
    static constexpr Private::GlobalColorRGB brightBlue = {0x55, 0x55, 0xff};
    static constexpr Private::GlobalColorRGB brightGreen = {0x55, 0xff, 0x55};
    static constexpr Private::GlobalColorRGB brightCyan = {0x55, 0xff, 0xff};
    static constexpr Private::GlobalColorRGB brightRed = {0xff, 0x55, 0x55};
    static constexpr Private::GlobalColorRGB brightMagenta = {0xff, 0x55, 0xff};
    static constexpr Private::GlobalColorRGB brightYellow = {0xff, 0xff, 0x55};
}

namespace Color = Colors;

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOLOR_INCLUDED
