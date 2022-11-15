.. _ZFormatRange:

ZFormatRange
============

.. cpp:class:: Tui::ZFormatRange

   This class is copyable and movable. It does not define comparison operators.

   A range of text to be associated with a set of formating parameters.

   This is used by :cpp:class:`Tui::ZTextLayout` for coloring specific parts of the text.

   **Constructors**

   .. cpp:function:: ZFormatRange()

      Creates an empty format range.

   .. cpp:function:: ZFormatRange(int start, int length, ZTextStyle format, ZTextStyle formattingChar, int userData = 0)

      Constructs a format range and sets its data.

      This is a short form that has the same result as calling
      :cpp:func:`void setStart(int start)`, :cpp:func:`void setLength(int length)`,
      :cpp:func:`void setFormat(const ZTextStyle &format)`,
      :cpp:func:`void setFormattingChar(const ZTextStyle &formattingChar)` and
      :cpp:func:`void setUserData(int userData)` with the passed parameters.

   **Functions**

   .. cpp:function:: int start() const
   .. cpp:function:: void setStart(int start)

      The start of the range in code units.

   .. cpp:function:: int length() const
   .. cpp:function:: void setLength(int length)

      The length of the range in code units.

   .. cpp:function:: ZTextStyle format() const
   .. cpp:function:: void setFormat(const ZTextStyle &format)

      The text style

   .. cpp:function:: ZTextStyle formattingChar() const
   .. cpp:function:: void setFormattingChar(const ZTextStyle &formattingChar)

      The style to use for displaying tabs and spaces when :cpp:enumerator:`Tui::ZTextOption::ShowTabsAndSpaces` is
      used.

   .. cpp:function:: int userData() const
   .. cpp:function:: void setUserData(int userData)

      The user data property can be used by the application to store additional data in a format range.

      This may serve as a additional input for the color mapping functions in :cpp:class:`ZTextOption`.
