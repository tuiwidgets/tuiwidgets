#ifndef TUIWIDGETS_ZCOMMON_INCLUDED
#define TUIWIDGETS_ZCOMMON_INCLUDED

#include <Tui/tuiwidgets_internal.h>

#include <Qt>

TUIWIDGETS_NS_START

enum ZItemDataRole : int {
    TUI_ITEM_ROLE_BASE = 0x854374,
    LeftDecorationRole = (TUI_ITEM_ROLE_BASE + 0),
    LeftDecorationFgRole = (TUI_ITEM_ROLE_BASE + 1),
    LeftDecorationBgRole = (TUI_ITEM_ROLE_BASE + 2),
    LeftDecorationSpaceRole = (TUI_ITEM_ROLE_BASE + 3),
};

// import some vales/types from Qt
using Qt::CheckState;
using Qt::Unchecked;
using Qt::PartiallyChecked;
using Qt::Checked;

using Qt::KeyboardModifier;
using Qt::KeyboardModifiers;
using Qt::NoModifier;
using Qt::ShiftModifier;
using Qt::AltModifier;
using Qt::ControlModifier;
using Qt::KeypadModifier;

using Qt::Key;
using Qt::Key_0;
using Qt::Key_1;
using Qt::Key_2;
using Qt::Key_3;
using Qt::Key_4;
using Qt::Key_5;
using Qt::Key_6;
using Qt::Key_7;
using Qt::Key_8;
using Qt::Key_9;
using Qt::Key_Backspace;
using Qt::Key_Delete;
using Qt::Key_Down;
using Qt::Key_End;
using Qt::Key_Enter;
using Qt::Key_Escape;
using Qt::Key_F1;
using Qt::Key_F2;
using Qt::Key_F3;
using Qt::Key_F4;
using Qt::Key_F5;
using Qt::Key_F6;
using Qt::Key_F7;
using Qt::Key_F8;
using Qt::Key_F9;
using Qt::Key_F10;
using Qt::Key_F11;
using Qt::Key_F12;
using Qt::Key_Home;
using Qt::Key_Insert;
using Qt::Key_Left;
using Qt::Key_Menu;
using Qt::Key_Minus;
using Qt::Key_PageDown;
using Qt::Key_PageUp;
using Qt::Key_Period;
using Qt::Key_Plus;
using Qt::Key_Right;
using Qt::Key_Space;
using Qt::Key_Tab;
using Qt::Key_Up;
using Qt::Key_division;
using Qt::Key_multiply;
using Qt::Key_unknown;

using Qt::FocusReason;
using Qt::TabFocusReason;
using Qt::BacktabFocusReason;
using Qt::ActiveWindowFocusReason;
using Qt::ShortcutFocusReason;
using Qt::OtherFocusReason;
// Other focus reasons are not meaningful at the moment.

using Qt::FocusPolicy;
using Qt::NoFocus;
using Qt::StrongFocus;
using Qt::TabFocus;
// Other focus policies are not meaningful at the moment.

using Qt::Alignment;
using Qt::AlignHorizontal_Mask;
using Qt::AlignLeft;
using Qt::AlignRight;
using Qt::AlignVertical_Mask;
using Qt::AlignTop;
using Qt::AlignBottom;
using Qt::AlignVCenter;
using Qt::AlignHCenter;

using Qt::ShortcutContext;
using Qt::WidgetShortcut;
using Qt::WindowShortcut;
using Qt::ApplicationShortcut;
using Qt::WidgetWithChildrenShortcut;

using Qt::Edges;
using Qt::Edge;
using Qt::TopEdge;
using Qt::LeftEdge;
using Qt::RightEdge;
using Qt::BottomEdge;

enum class CursorStyle : int {
    Unset = -1,
    Block = 0,
    Underline = 1,
    Bar = 2
};

enum class ZTextAttribute : int {
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
