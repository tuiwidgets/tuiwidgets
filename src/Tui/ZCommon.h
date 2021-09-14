#ifndef TUIWIDGETS_ZCOMMON_INCLUDED
#define TUIWIDGETS_ZCOMMON_INCLUDED

#include <Tui/tuiwidgets_internal.h>


TUIWIDGETS_NS_START

enum class CursorStyle {
    Unset = -1,
    Block = 0,
    Underline = 1,
    Bar = 2
};

enum class ZTextAttribute {
    Bold = (1<<0),
    Italic = (1<<1),
    Blink = (1<<4),
    Overline = (1<<5),
    Inverse = (1<<6),
    Strike = (1<<7),
    Underline = (1<<16),
    UnderlineDouble = (1<<17),
    UnderlineCurly = (1<<18)
};
Q_DECLARE_FLAGS(ZTextAttributes, ZTextAttribute)

static constexpr int Erased = 127;

constexpr class WithMarkupTag {} withMarkup {};

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZTextAttributes)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZTextAttributes)

#endif // TUIWIDGETS_ZCOMMON_INCLUDED
