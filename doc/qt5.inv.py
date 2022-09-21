#! /usr/bin/python3

import zlib

preamble = """
# Sphinx inventory version 2
# Project: Qt
# Version: 5.15
# The remainder of this file is compressed using zlib.
"""[1:]

data = """
QObject cpp:class 1 qobject.html QObject
QObject::event cpp:function 1 qobject.html#event QObject::event
QString cpp:class 1 qstring.html QString
QPoint cpp:class 1 qpoint.html QPoint
QFlags cpp:class 1 qflags.html QFlags
QEvent cpp:class 1 qevent.html QEvent
QRect cpp:class 1 qrect.html QRect
QSize cpp:class 1 qsize.html QSize
Qt::FocusPolicy cpp:enum 1 qt.html#FocusPolicy-enum FocusPolicy
Qt::NoFocus cpp:enum 1 qt.html#FocusPolicy-enum NoFocus
Qt::StrongFocus cpp:enum 1 qt.html#FocusPolicy-enum StrongFocus
Qt::TabFocus cpp:enum 1 qt.html#FocusPolicy-enum TabFocus
QMargins cpp:class 1 qmargins.html QMargins
QStringList cpp:class 1 qstringlist.html QStringList
Qt::FocusReason cpp:enum 1 qt.html#FocusReason-enum FocusReason
Qt::TabFocusReason cpp:enum 1 qt.html#FocusReason-enum TabFocusReason
Qt::BacktabFocusReason cpp:enum 1 qt.html#FocusReason-enum BacktabFocusReason
Qt::ActiveWindowFocusReason cpp:enum 1 qt.html#FocusReason-enum ActiveWindowFocusReason
Qt::ShortcutFocusReason cpp:enum 1 qt.html#FocusReason-enum ShortcutFocusReason
Qt::OtherFocusReason cpp:class 1 qt.html#FocusReason-enum OtherFocusReason
QMetaObject cpp:class 1 qmetaobject.html QMetaObject
Qt cpp:class 1 qt.html Qt
QStringView cpp:class 1 qstringview.html QStringView
QChar cpp:class 1 qchar.html QChar
Qt::KeyboardModifier cpp:enum 1 qt.html#KeyboardModifier-enum KeyboardModifier
Qt::KeyboardModifiers cpp:enum 1 qt.html#KeyboardModifier-enum KeyboardModifiers
Qt::NoModifier cpp:enum 1 qt.html#KeyboardModifier-enum NoModifier
Qt::ShiftModifier cpp:enum 1 qt.html#KeyboardModifier-enum ShiftModifier
Qt::AltModifier cpp:enum 1 qt.html#KeyboardModifier-enum AltModifier
Qt::ControlModifier cpp:enum 1 qt.html#KeyboardModifier-enum ControlModifier
Qt::KeypadModifier cpp:enum 1 qt.html#KeyboardModifier-enum KeypadModifier
Qt::Key cpp:enum 1 qt.html#Key-enum Key
Qt::Key_0 cpp:enum 1 qt.html#Key-enum Key_0
Qt::Key_1 cpp:enum 1 qt.html#Key-enum Key_1
Qt::Key_2 cpp:enum 1 qt.html#Key-enum Key_2
Qt::Key_3 cpp:enum 1 qt.html#Key-enum Key_3
Qt::Key_4 cpp:enum 1 qt.html#Key-enum Key_4
Qt::Key_5 cpp:enum 1 qt.html#Key-enum Key_5
Qt::Key_6 cpp:enum 1 qt.html#Key-enum Key_6
Qt::Key_7 cpp:enum 1 qt.html#Key-enum Key_7
Qt::Key_8 cpp:enum 1 qt.html#Key-enum Key_8
Qt::Key_9 cpp:enum 1 qt.html#Key-enum Key_9
Qt::Key_Backspace cpp:enum 1 qt.html#Key-enum Key_Backspace
Qt::Key_Delete cpp:enum 1 qt.html#Key-enum Key_Delete
Qt::Key_Down cpp:enum 1 qt.html#Key-enum Key_Down
Qt::Key_End cpp:enum 1 qt.html#Key-enum Key_End
Qt::Key_Enter cpp:enum 1 qt.html#Key-enum Key_Enter
Qt::Key_Escape cpp:enum 1 qt.html#Key-enum Key_Escape
Qt::Key_F1 cpp:enum 1 qt.html#Key-enum Key_F1
Qt::Key_F2 cpp:enum 1 qt.html#Key-enum Key_F2
Qt::Key_F3 cpp:enum 1 qt.html#Key-enum Key_F3
Qt::Key_F4 cpp:enum 1 qt.html#Key-enum Key_F4
Qt::Key_F5 cpp:enum 1 qt.html#Key-enum Key_F5
Qt::Key_F6 cpp:enum 1 qt.html#Key-enum Key_F6
Qt::Key_F7 cpp:enum 1 qt.html#Key-enum Key_F7
Qt::Key_F8 cpp:enum 1 qt.html#Key-enum Key_F8
Qt::Key_F9 cpp:enum 1 qt.html#Key-enum Key_F9
Qt::Key_F10 cpp:enum 1 qt.html#Key-enum Key_F10
Qt::Key_F11 cpp:enum 1 qt.html#Key-enum Key_F11
Qt::Key_F12 cpp:enum 1 qt.html#Key-enum Key_F12
Qt::Key_Home cpp:enum 1 qt.html#Key-enum Key_Home
Qt::Key_Insert cpp:enum 1 qt.html#Key-enum Key_Insert
Qt::Key_Left cpp:enum 1 qt.html#Key-enum Key_Left
Qt::Key_Menu cpp:enum 1 qt.html#Key-enum Key_Menu
Qt::Key_Minus cpp:enum 1 qt.html#Key-enum Key_Minus
Qt::Key_PageDown cpp:enum 1 qt.html#Key-enum Key_PageDown
Qt::Key_PageUp cpp:enum 1 qt.html#Key-enum Key_PageUp
Qt::Key_Period cpp:enum 1 qt.html#Key-enum Key_Period
Qt::Key_Plus cpp:enum 1 qt.html#Key-enum Key_Plus
Qt::Key_Right cpp:enum 1 qt.html#Key-enum Key_Right
Qt::Key_Space cpp:enum 1 qt.html#Key-enum Key_Space
Qt::Key_Tab cpp:enum 1 qt.html#Key-enum Key_Tab
Qt::Key_Up cpp:enum 1 qt.html#Key-enum Key_Up
Qt::Key_division cpp:enum 1 qt.html#Key-enum Key_division
Qt::Key_multiply cpp:enum 1 qt.html#Key-enum Key_multiply
Qt::Key_unknown cpp:enum 1 qt.html#Key-enum Key_unknown
QSet cpp:class 1 qset.html QSet
QCoreApplication::quit cpp:function 1 qcoreapplication.html#quit QCoreApplication::quit
QEvent::Type cpp:enum 1 qevent.html#Type-enum QEvent::Type
Qt::CheckState cpp:enum 1 qt.html#CheckState-enum CheckState
Qt::Unchecked cpp:enum 1 qt.html#CheckState-enum Unchecked
Qt::PartiallyChecked cpp:enum 1 qt.html#CheckState-enum PartiallyChecked
Qt::Checked cpp:enum 1 qt.html#CheckState-enum Checked
Qt::Alignment cpp:enum 1 qt.html#AlignmentFlag-enum Alignment
Qt::AlignHorizontal_Mask cpp:enum 1 qt.html#AlignmentFlag-enum AlignHorizontal_Mask
Qt::AlignLeft cpp:enum 1 qt.html#AlignmentFlag-enum AlignLeft
Qt::AlignRight cpp:enum 1 qt.html#AlignmentFlag-enum AlignRight
Qt::AlignVertical_Mask cpp:enum 1 qt.html#AlignmentFlag-enum AlignVertical_Mask
Qt::AlignTop cpp:enum 1 qt.html#AlignmentFlag-enum AlignTop
Qt::AlignBottom cpp:enum 1 qt.html#AlignmentFlag-enum AlignBottom
Qt::AlignVCenter cpp:enum 1 qt.html#AlignmentFlag-enum AlignVCenter
Qt::AlignHCenter cpp:enum 1 qt.html#AlignmentFlag-enum AlignHCenter
Qt::ShortcutContext cpp:enum 1 qt.html#ShortcutContext-enum ShortcutContext
Qt::WidgetShortcut cpp:enum 1 qt.html#ShortcutContext-enum WidgetShortcut
Qt::WindowShortcut cpp:enum 1 qt.html#ShortcutContext-enum WindowShortcut
Qt::ApplicationShortcut cpp:enum 1 qt.html#ShortcutContext-enum ApplicationShortcut
Qt::WidgetWithChildrenShortcut cpp:enum 1 qt.html#ShortcutContext-enum WidgetWithChildrenShortcut
Qt::Edges cpp:enum 1 qt.html#Edge-enum Edges
Qt::TopEdge cpp:enum 1 qt.html#Edge-enum TopEdge
Qt::LeftEdge cpp:enum 1 qt.html#Edge-enum LeftEdge
Qt::RightEdge cpp:enum 1 qt.html#Edge-enum RightEdge
Qt::BottomEdge cpp:enum 1 qt.html#Edge-enum BottomEdge
QEvent::isAccepted cpp:function 1 qevent.html#accepted-prop QEvent::isAccepted
QByteArray cpp:class 1 qbytearray.html QByteArray
"""[1:]

# Templates don't seem to work with intershinx without complaining about missing references or adding entries for
# each template with all used template parameters...
template_hacks = """
QFlags<Tui::ZTerminal::Option> cpp:class 1 qflags.html QFlags
QSet<Tui::ZWidget*> cpp:class 1 qset.html QSet
QSet<Tui::ZSymbol> cpp:class 1 qset.html QSet
QSet<QString> cpp:class 1 qset.html QSet
QList<Tui::ZPalette::RuleCmd> cpp:class 1 qlist.html QList
QList<Tui::ZPalette::ColorDef> cpp:class 1 qlist.html QList
QList<Tui::ZPalette::RuleDef> cpp:class 1 qlist.html QList
"""[1:]

data += template_hacks

f = open('qt5.inv', 'wb')
f.write(preamble.encode())
f.write(zlib.compress(data.encode(), 9))
