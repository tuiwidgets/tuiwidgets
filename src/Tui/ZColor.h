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

class TUIWIDGETS_EXPORT ZColor {
public:
    ZColor() = default;
    ZColor(int r, int g, int b);
    ZColor(Private::GlobalColorRGB globalColor);

    int red() const;
    void setRed(int blue);
    int green() const;
    void setGreen(int blue);
    int blue() const;
    void setBlue(int blue);

    uint32_t nativeValue();

    static ZColor fromRgb(int r, int g, int b);
    static ZColor defaultColor();
    //static ZColor fromIndexed();

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
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOLOR_INCLUDED
