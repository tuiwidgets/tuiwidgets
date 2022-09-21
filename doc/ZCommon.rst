.. _ZCommon:

ZCommon
=======

The ``ZCommon.h`` header contains types, enums and constants shared by various classes in Tui Widgets.

.. rst-class:: tw-invisible
.. cpp:class:: Tui

   This is actually a namespace.

.. rst-class:: tw-midspacebefore
.. cpp:enum:: Tui::ZItemDataRole

   .. cpp:enumerator:: LeftDecorationRole

      TODO

   .. cpp:enumerator:: LeftDecorationFgRole

      TODO

   .. cpp:enumerator:: LeftDecorationBgRole

      TODO

   .. cpp:enumerator:: LeftDecorationSpaceRole

      TODO


.. rst-class:: tw-midspacebefore
.. cpp:enum-class:: Tui::CursorStyle

   .. cpp:enumerator:: Unset

      Use terminal default cursor style

   .. cpp:enumerator:: Block

      Use block cursor style if supported

   .. cpp:enumerator:: Underline

      Use underline cursor style if supported

   .. cpp:enumerator:: Bar

      Use bar cursor style if supported

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui:: ZTextAttributes
.. cpp:enum-class:: Tui::ZTextAttribute

   .. cpp:enumerator:: Bold
   .. cpp:enumerator:: Italic
   .. cpp:enumerator:: Blink
   .. cpp:enumerator:: Overline
   .. cpp:enumerator:: Inverse
   .. cpp:enumerator:: Strike
   .. cpp:enumerator:: Underline
   .. cpp:enumerator:: UnderlineDouble
   .. cpp:enumerator:: UnderlineCurly

..
  TODO ZTextAttributes ?


.. rst-class:: tw-midspacebefore
.. cpp:var:: static constexpr int Tui::Erased = 127

   Character value for the special erased state.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::WithMarkupTag
.. cpp:var:: constexpr Tui::WithMarkupTag Tui::withMarkup {}

   Marker tag for overloads using markup for the following string arguement.

Aliases from Qt
---------------

TODO

.. cpp:type:: Tui::CheckState = Qt::CheckState
.. cpp:enumerator:: Tui::Unchecked = Qt::Unchecked
.. cpp:enumerator:: Tui::PartiallyChecked = Qt::PartiallyChecked
.. cpp:enumerator:: Tui::Checked = Qt::Checked

.. cpp:type:: Tui::KeyboardModifier = Qt::KeyboardModifier
.. cpp:type:: Tui::KeyboardModifiers = Qt::KeyboardModifiers
.. cpp:enumerator:: Tui::NoModifier = Qt::NoModifier
.. cpp:enumerator:: Tui::ShiftModifier = Qt::ShiftModifier
.. cpp:enumerator:: Tui::AltModifier = Qt::AltModifier
.. cpp:enumerator:: Tui::ControlModifier = Qt::ControlModifier
.. cpp:enumerator:: Tui::KeypadModifier = Qt::KeypadModifier


.. cpp:type:: Tui::Key = Qt::Key
.. cpp:enumerator:: Tui::Key_0 = Qt::Key_0
.. cpp:enumerator:: Tui::Key_1 = Qt::Key_1
.. cpp:enumerator:: Tui::Key_2 = Qt::Key_2
.. cpp:enumerator:: Tui::Key_3 = Qt::Key_3
.. cpp:enumerator:: Tui::Key_4 = Qt::Key_4
.. cpp:enumerator:: Tui::Key_5 = Qt::Key_5
.. cpp:enumerator:: Tui::Key_6 = Qt::Key_6
.. cpp:enumerator:: Tui::Key_7 = Qt::Key_7
.. cpp:enumerator:: Tui::Key_8 = Qt::Key_8
.. cpp:enumerator:: Tui::Key_9 = Qt::Key_9
.. cpp:enumerator:: Tui::Key_Backspace = Qt::Key_Backspace
.. cpp:enumerator:: Tui::Key_Delete = Qt::Key_Delete
.. cpp:enumerator:: Tui::Key_Down = Qt::Key_Down
.. cpp:enumerator:: Tui::Key_End = Qt::Key_End
.. cpp:enumerator:: Tui::Key_Enter = Qt::Key_Enter
.. cpp:enumerator:: Tui::Key_Escape = Qt::Key_Escape
.. cpp:enumerator:: Tui::Key_F1 = Qt::Key_F1
.. cpp:enumerator:: Tui::Key_F2 = Qt::Key_F2
.. cpp:enumerator:: Tui::Key_F3 = Qt::Key_F3
.. cpp:enumerator:: Tui::Key_F4 = Qt::Key_F4
.. cpp:enumerator:: Tui::Key_F5 = Qt::Key_F5
.. cpp:enumerator:: Tui::Key_F6 = Qt::Key_F6
.. cpp:enumerator:: Tui::Key_F7 = Qt::Key_F7
.. cpp:enumerator:: Tui::Key_F8 = Qt::Key_F8
.. cpp:enumerator:: Tui::Key_F9 = Qt::Key_F9
.. cpp:enumerator:: Tui::Key_F10 = Qt::Key_F10
.. cpp:enumerator:: Tui::Key_F11 = Qt::Key_F11
.. cpp:enumerator:: Tui::Key_F12 = Qt::Key_F12
.. cpp:enumerator:: Tui::Key_Home = Qt::Key_Home
.. cpp:enumerator:: Tui::Key_Insert = Qt::Key_Insert
.. cpp:enumerator:: Tui::Key_Left = Qt::Key_Left
.. cpp:enumerator:: Tui::Key_Menu = Qt::Key_Menu
.. cpp:enumerator:: Tui::Key_Minus = Qt::Key_Minus
.. cpp:enumerator:: Tui::Key_PageDown = Qt::Key_PageDown
.. cpp:enumerator:: Tui::Key_PageUp = Qt::Key_PageUp
.. cpp:enumerator:: Tui::Key_Period = Qt::Key_Period
.. cpp:enumerator:: Tui::Key_Plus = Qt::Key_Plus
.. cpp:enumerator:: Tui::Key_Right = Qt::Key_Right
.. cpp:enumerator:: Tui::Key_Space = Qt::Key_Space
.. cpp:enumerator:: Tui::Key_Tab = Qt::Key_Tab
.. cpp:enumerator:: Tui::Key_Up = Qt::Key_Up
.. cpp:enumerator:: Tui::Key_division = Qt::Key_division
.. cpp:enumerator:: Tui::Key_multiply = Qt::Key_multiply
.. cpp:enumerator:: Tui::Key_unknown = Qt::Key_unknown


.. cpp:type:: Tui::FocusReason = Qt::FocusReason
.. cpp:enumerator:: Tui::TabFocusReason = Qt::TabFocusReason
.. cpp:enumerator:: Tui::BacktabFocusReason = Qt::BacktabFocusReason
.. cpp:enumerator:: Tui::ActiveWindowFocusReason = Qt::ActiveWindowFocusReason
.. cpp:enumerator:: Tui::ShortcutFocusReason = Qt::ShortcutFocusReason
.. cpp:enumerator:: Tui::OtherFocusReason = Qt::OtherFocusReason


.. cpp:type:: Tui::FocusPolicy = Qt::FocusPolicy
.. cpp:enumerator:: Tui::NoFocus = Qt::NoFocus
.. cpp:enumerator:: Tui::StrongFocus = Qt::StrongFocus
.. cpp:enumerator:: Tui::TabFocus = Qt::TabFocus


.. cpp:type:: Tui::Alignment = Qt::Alignment
.. cpp:enumerator:: Tui::AlignHorizontal_Mask = Qt::AlignHorizontal_Mask
.. cpp:enumerator:: Tui::AlignLeft = Qt::AlignLeft
.. cpp:enumerator:: Tui::AlignRight = Qt::AlignRight
.. cpp:enumerator:: Tui::AlignVertical_Mask = Qt::AlignVertical_Mask
.. cpp:enumerator:: Tui::AlignTop = Qt::AlignTop
.. cpp:enumerator:: Tui::AlignBottom = Qt::AlignBottom
.. cpp:enumerator:: Tui::AlignVCenter = Qt::AlignVCenter
.. cpp:enumerator:: Tui::AlignHCenter = Qt::AlignHCenter


.. cpp:type:: Tui::ShortcutContext = Qt::ShortcutContext
.. cpp:enumerator:: Tui::WidgetShortcut = Qt::WidgetShortcut
.. cpp:enumerator:: Tui::WindowShortcut = Qt::WindowShortcut
.. cpp:enumerator:: Tui::ApplicationShortcut = Qt::ApplicationShortcut
.. cpp:enumerator:: Tui::WidgetWithChildrenShortcut = Qt::WidgetWithChildrenShortcut


.. cpp:type:: Tui::Edges = Qt::Edges
.. cpp:enumerator:: Tui::TopEdge = Qt::TopEdge
.. cpp:enumerator:: Tui::LeftEdge = Qt::LeftEdge
.. cpp:enumerator:: Tui::RightEdge = Qt::RightEdge
.. cpp:enumerator:: Tui::BottomEdge = Qt::BottomEdge


Private Types
-------------

.. cpp:class:: template<> Tui::Private::ZMoFunc<void(QEvent*)>
.. cpp:class:: template<> Tui::Private::ZMoFunc<bool()>

   TODO


.. cpp:class:: Tui::Private

   TODO


.. rst-class:: tw-invisible
.. cpp:class:: uint32_t

   Figure out how to suppress missing type warning regarding this
