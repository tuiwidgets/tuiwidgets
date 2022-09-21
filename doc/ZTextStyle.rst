.. _ZTextStyle:

ZTextStyle
==========

.. cpp:class:: Tui::ZTextStyle

   This class is copyable and movable. It does define comparison operators for equality.

   A container to combine foreground color, background color and text attributes.

   **Constructors**

   .. cpp:function:: ZTextStyle(Tui::ZColor fg, Tui::ZColor bg)
   .. cpp:function:: ZTextStyle(Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attributes)

      Construct an instance of the class using the given parameters as colors and if given as attributes.

      If no attributes are given, all attributes are cleared.

   **Functions**

   .. cpp:function:: Tui::ZColor foregroundColor() const
   .. cpp:function:: void setForegroundColor(const Tui::ZColor &foregroundColor)

      Get or set the stored foreground color.

   .. cpp:function:: Tui::ZColor backgroundColor() const
   .. cpp:function:: void setBackgroundColor(const Tui::ZColor &backgroundColor)

      Get or set the stored background color.

   .. cpp:function:: Tui::ZTextAttributes attributes() const
   .. cpp:function:: void setAttributes(Tui::ZTextAttributes attributes)

      Get or set the stored attributes.
