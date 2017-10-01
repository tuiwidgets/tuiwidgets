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

private:
    uint32_t val = 0;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCOLOR_INCLUDED
