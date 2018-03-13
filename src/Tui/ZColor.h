#ifndef TUIWIDGETS_ZCOLOR_INCLUDED
#define TUIWIDGETS_ZCOLOR_INCLUDED

#include <cstdint>

#include <QtGlobal>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZColor {
public:
    ZColor() = default;
    ZColor(int r, int g, int b);

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
    // TODO:
    extern Tui::ZColor black;
    extern Tui::ZColor darkGray;
    extern Tui::ZColor lightGray;
    extern Tui::ZColor brightWhite;
    extern Tui::ZColor blue;
    extern Tui::ZColor green;
    extern Tui::ZColor cyan;
    extern Tui::ZColor red;
    extern Tui::ZColor magenta;
    extern Tui::ZColor brown;
    extern Tui::ZColor brightBlue;
    extern Tui::ZColor brightGreen;
    extern Tui::ZColor brightCyan;
    extern Tui::ZColor brightRed;
    extern Tui::ZColor brightMagenta;
    extern Tui::ZColor brightYellow;
}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOLOR_INCLUDED
