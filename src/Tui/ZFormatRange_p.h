#ifndef TUIWIDGETS_ZFORMATRANGE_P_INCLUDED
#define TUIWIDGETS_ZFORMATRANGE_P_INCLUDED

#include <Tui/ZFormatRange.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZFormatRangePrivate {
public:
    ZFormatRangePrivate();
    ZFormatRangePrivate(int start, int length, ZTextStyle format, ZTextStyle formatingChar, int userData);
    ~ZFormatRangePrivate();

public:
    int _start = 0;
    int _length = 0;
    ZTextStyle _format;
    ZTextStyle _formattingChar = _format;
    int _userData = 0;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZFORMATRANGE_P_INCLUDED
