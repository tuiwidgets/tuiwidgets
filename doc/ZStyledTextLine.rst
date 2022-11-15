.. _ZStyledTextLine:

ZStyledTextLine
===============

ZStyledTextLine is a helper class to implement display of text with :ref:`markup <ControlMarkup>`.

.. cpp:class:: Tui::ZStyledTextLine

   This class is copyable and movable. It does not define comparison operators.

   A helper class to implement display of text with :ref:`markup <ControlMarkup>`.

   .. cpp:function:: void setMarkup(const QString &markup)
   .. cpp:function:: QString markup() const

      Using these functions the text content can be set using markup or retrieved.

      When the content was most recently set using setText the returned markup is empty.

   .. cpp:function:: void setText(const QString &text)
   .. cpp:function:: QString text() const

      Using these functions the plain text content can be get or set.

      When the content was most recently set using setMarkup the returned text is empty.

   .. cpp:function:: void setBaseStyle(ZTextStyle newBaseStyle)

      Set the base style for display.

      The base style is used for text where no other style applies.

   .. cpp:function:: void setMnemonicStyle(ZTextStyle newBaseStyle, ZTextStyle newMnemonicStyle)

      Set the mnemonic style for display.

      The mnemonic style is used for text enclosed in a ``m`` element.

   .. cpp:function:: QString mnemonic() const

      Returns the code point enclosed in a ``m`` element in the markup.

      If more than one code point is in the ``m`` element or in multiple ``m`` elements the empty string is returned.

   .. cpp:function:: bool hasParsingError() const

      Returns true, iff there was a problem parsing the markup.

   .. cpp:function:: int width(const ZTextMetrics &metrics) const

      Return the width of the text to display.

   .. cpp:function:: void write(ZPainter *painter, int x, int y, int width) const

      Displays the text using ``painter`` at the :cpp:expr:`(x, y)` with a width ``width``.

      The total width ``width`` will be erased with the colors and attributes from the base style.
