.. _ZTextLayout:

ZTextLayout
===========

ZTextLayout allows outputting text with features not directly supported by the text output functions in
:cpp:class:`Tui::ZPainter` and also allows mapping from visual coordinates to text position
and adjusting text positions by letter/cluster or words.
It supports using different color and attributes for part of the text, line breaks, line wrapping and more.

It is a low level class and is intended for use in widgets that render text manually.

The most basic usage is to construct the class using the terminal's or painter's text metrics together with the text to
display and then calling :cpp:func:`void Tui::ZTextLayout::doLayout(int width)`.

After the layouting is done, the other instance methods can be used to display the text and to query information from
the layout.

Displaying the text can be done with :cpp:func:`draw <void Tui::ZTextLayout::draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle style, const Tui::ZTextStyle *formattingChars = nullptr, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const>`
passing it the position and the base text style.

..
  TODO more details

.. cpp:class:: Tui::ZTextLayout

   This class is copyable. It does not define comparison operators.

   Layouts and paints text.
   Also allows to query information about the layout of the text.

   **Constructors**

   .. cpp:function:: ZTextLayout(Tui::ZTextMetrics metrics)

      Construct a text layout using ``metrics`` as text metrics.

      When using this constructor, the text must be set using :cpp:func:`void setText(const QString &text)` and the
      text must be layouted before the layout can be used to draw the text or query layout details.

   .. cpp:function:: ZTextLayout(Tui::ZTextMetrics metrics, const QString &text)

      Construct a text layout using ``metrics`` as text metrics and ``text`` as the initial text.

      When using this constructor, the text must be layouted before the layout can be used to draw the text or query
      layout details.

   **Enums**

   .. cpp:enum:: CursorMode

      .. cpp:enumerator:: SkipCharacters

         Adjust cursor position by letters/clusters.

      .. cpp:enumerator:: SkipWords

         Adjust cursor postion by words.

   .. cpp:enum:: Edge

      .. cpp:enumerator:: Leading

         Use the start of the cluster.

      .. cpp:enumerator:: Trailing

         Use the end of the cluster.

   **Functions**

   .. cpp:function:: QString text() const
   .. cpp:function:: void setText(const QString &text)

      The text that is to be layouted.

      When the text is changed, it must be layouted again before using any functions to draw using the layout or to
      query layout details.

   .. cpp:function:: const Tui::ZTextOption &textOption() const
   .. cpp:function:: void setTextOption(const Tui::ZTextOption &option)

      The text option describes various details on how the text should be layouted.

      See :cpp:class:`Tui::ZTextOption` for details.

      When the text option is changed the text must to be layouted again before using any functions to draw using the
      layout or to query layout details.

   .. cpp:function:: void doLayout(int width)

      Layouts the text using ``width`` as layout width.

      This function creates as many lines as needed to display the text using width ``width``.
      Lines are created at position ``(x, y) = (0, line number)`` where line number increments for each new line.

      After calling this function, functions to draw using the layout or to query layout details can be used.

      When using this function do not call :cpp:func:`void beginLayout()` or :cpp:func:`void endLayout()` manually.

   .. cpp:function:: void beginLayout()

      Begins manual layout of the text.

      After calling this function the application can create and layout lines using
      :cpp:func:`Tui::ZTextLineRef createLine()`.

      After all lines are created and layouted the application must call :cpp:func:`void endLayout()`.

   .. cpp:function:: void endLayout()

      Must be called when using manual layout after layouting all lines.

   .. cpp:function:: Tui::ZTextLineRef createLine()

      Creates a new line to be layouted when using manual layout.

      If all text to layout is already layouted, no further lines will be added and a
      :cpp:func:`invalid <bool Tui::ZTextLineRef::isValid() const>` :cpp:class:`Tui::ZTextLineRef` is returned.

      Following the call to this function the application code should call
      :cpp:func:`void Tui::ZTextLineRef::setLineWidth(int width)` to trigger layouting the line.

      In addition to layouting the line, the line should be assigned a relative position using
      :cpp:func:`void Tui::ZTextLineRef::setPosition(const QPoint &pos)`

   .. cpp:function:: void draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle style, const Tui::ZTextStyle *formattingChars = nullptr, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const

      Draws the whole text layout at position ``pos`` using the painter ``painter``.

      It uses ``style`` for text not covered by a item in ``ranges`` and ``formattingChars`` for formating characters
      not covered by a item in ``ranges`` (if enabled in the options).

      If :cpp:expr:`nullptr` is passed as ``formattingChars``, it defaults to ``style``.

      The last format range in ``ranges`` that matches a part of the text is used to format that text.

   .. cpp:function:: void showCursor(Tui::ZPainter painter, const QPoint &pos, int cursorPosition) const

      Show the terminal cursor at visual position that corresponds to cursor position ``cursorPosition`` in the
      layouted text.

      This is intended for calling after calling
      :cpp:func:`draw <void draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle style, const Tui::ZTextStyle *formattingChars = nullptr, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const>`
      using the same ``pos``.

   .. cpp:function:: QRect boundingRect() const

      After layout, returns the smallest rectangle that contains all the layouted text.

   .. cpp:function:: int lineCount() const

      After layout, returns the number of lines created from the text.

   .. cpp:function:: int maximumWidth() const

      After layout, returns the maxium of the width of all lines.

      The width does not contain the x offset of the lines.

   .. cpp:function:: Tui::ZTextLineRef lineAt(int i) const

      After layout, returns the the ``i``-th line of the layouted text.

      If ``i`` is out of range returns a :cpp:func:`invalid <bool Tui::ZTextLineRef::isValid() const>`
      :cpp:class:`Tui::ZTextLineRef`.

   .. cpp:function:: Tui::ZTextLineRef lineForTextPosition(int pos) const

      After layout, returns the line that contains the character at position ``pos`` (in code units) in the layout's
      text.

      If ``pos`` is the position after the last code point in the layout's text it returns the last line.

      If ``pos`` is out of range returns a :cpp:func:`invalid <bool Tui::ZTextLineRef::isValid() const>`
      :cpp:class:`Tui::ZTextLineRef`.

   .. cpp:function:: int previousCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const

      After layout, returns the previous cursor position in code units relative to ``oldPos``.

      When ``mode`` is :cpp:enumerator:`CursorMode::SkipCharacters <Tui::ZTextLayout::CursorMode::SkipCharacters>` the
      cursor position will be one letter/cluster before ``oldPos`` (or 0 no previous position exists).

      When ``mode`` is :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>` the
      cursor position will be one word before ``oldPos`` (or 0 no previous position exists).

   .. cpp:function:: int nextCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const

      After layout, returns the next cursor position in code units relative to ``oldPos``.

      When ``mode`` is :cpp:enumerator:`CursorMode::SkipCharacters <Tui::ZTextLayout::CursorMode::SkipCharacters>` the
      cursor position will be one letter/cluster after ``oldPos`` (or the position after the last code point in
      the text if no next position exists).

      When ``mode`` is :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>` the
      cursor position will be one word after ``oldPos`` (or the position after the last code point in
      the text if no next position exists).

   .. cpp:function:: bool isValidCursorPosition(int pos) const

      After layout, returns :cpp:expr:`true` iff the position ``pos`` (in code units) is a valid cursor position in the current
      text.

      A cursor position is valid if it is neither negative nor greater than the text lenght and it falls on a cluster
      boundary.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZTextLineRef

   This class is copyable. It does not define comparison operators.

   This class represents a reference to a line in a :cpp:class:`Tui::ZTextLayout`.
   It can be used to render a line of the layout or query information about the line.

   .. cpp:function:: bool isValid() const

      Returns :cpp:expr:`true` when the instance references a line.
      Otherwise returns :cpp:expr:`false`.
      Invalid instances are used to represent either that no line exists or that no new line can be created because all
      the text in the text layout is already layouted.

   .. cpp:function:: void draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle color, Tui::ZTextStyle formattingChars, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const

      Draws the current line using the painter ``painter``.
      The line is drawn using its relative position in the layout plus the position ``pos``.
      That is when drawing all lines each draw call usually uses the same ``pos`` as the other draw calls.

      It uses ``style`` for text not covered by a item in ``ranges`` and ``formattingChars`` for formating characters
      not covered by a item in ``ranges`` (if enabled in the options).

      The last format range in ``ranges`` that matches a part of the text is used to format that text.

   .. cpp:function:: int cursorToX(int *cursorPos, Tui::ZTextLayout::Edge edge) const
   .. cpp:function:: int cursorToX(int cursorPos, Tui::ZTextLayout::Edge edge) const

      Translate the cursor position ``cursorPos`` to the relative x position in the current line.

      If the cursor position in not on the current line it is adjusted to the nearest position that is on the line.
      if the cursor position is not a valid cursor position the position is adjusted to the nearest valid cursor
      position before the given position.

      In the variant taking ``cursorPos`` as pointer the pointed to value will be updated to the cursor postion used
      for the x position calculation.

      If ``edge`` is :cpp:enumerator:`ZTextLayout::Leading <Tui::ZTextLayout::Edge::Leading>` the return is the
      position at the start of the cluster.
      If ``edge`` is :cpp:enumerator:`ZTextLayout::Trailing <Tui::ZTextLayout::Edge::Trailing>` the return is the
      position after the cluster.
      If the cursor points to the ``\n`` that terminates the line the trailing edge is the same as the leading edge,
      because the line break character does not take any space in the layout.

   .. cpp:function:: int xToCursor(int x) const

      Translates a relative x position to the corresponding cursor position (in code units).

      The returned cursor position is the nearest cursor position for the given ``x``.
      For single cell clusters the position will be before the cluster.

      The combination ``xToCursor(cursorToX(cursor, Tui::ZTextLayout::Edge::Leading))`` returns ``cursor`` again for
      valid cursor positions on the current line.

   .. cpp:function:: void setPosition(const QPoint &pos)
   .. cpp:function:: QPoint position() const

      The position of a line is the relative position in the layout of the line.

   .. cpp:function:: int x() const

      Return the x part of the relative position of the line.

   .. cpp:function:: int y() const

      Return the y part of the relative position of the line.

   .. cpp:function:: int width() const

      Return the width of the line.

   .. cpp:function:: int height() const

      Always return 1.

   .. cpp:function:: QRect rect() const

      Returns ``QRect(position(), QSize{width(), height()})``.

   .. cpp:function:: int lineNumber() const

      Returns the number of this line in its text layout.

      See also: :cpp:func:`Tui::ZTextLineRef Tui::ZTextLayout::lineAt(int i) const`

   .. cpp:function:: int textStart() const

      Return the start position in the layout's text of this line in code units.

   .. cpp:function:: int textLength() const

      Return the length of text from the layout's text covered by this line in code units.

   .. cpp:function:: void setLineWidth(int width)

      Triggers layout of a line with remaining text of the text layout.
