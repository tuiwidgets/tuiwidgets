.. _ZTextLayout:

ZTextLayout
===========

TODO

.. cpp:class:: Tui::ZTextLayout

   This class is copyable. It does not define comparison operators.

   TODO

   **Constructors**

   .. cpp:function:: ZTextLayout(Tui::ZTextMetrics metrics)

      TODO

   .. cpp:function:: ZTextLayout(Tui::ZTextMetrics metrics, const QString &text)

      TODO

   **Enums**

   .. cpp:enum:: CursorMode

      .. cpp:enumerator:: SkipCharacters

         TODO

      .. cpp:enumerator:: SkipWords

         TODO

   .. cpp:enum:: Edge

      .. cpp:enumerator:: Leading

         TODO

      .. cpp:enumerator:: Trailing

         TODO

   **Functions**

   .. cpp:function:: QString text() const
   .. cpp:function:: void setText(const QString &text)

      TODO

   .. cpp:function:: const Tui::ZTextOption &textOption() const
   .. cpp:function:: void setTextOption(const Tui::ZTextOption &option)

      TODO

   .. cpp:function:: void beginLayout()

      TODO

   .. cpp:function:: void doLayout(int width)

      TODO

   .. cpp:function:: void endLayout()

      TODO

   .. cpp:function:: Tui::ZTextLineRef createLine()

      TODO

   .. cpp:function:: void draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle style, const Tui::ZTextStyle *formattingChars = nullptr, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const

      TODO

   .. cpp:function:: void showCursor(Tui::ZPainter painter, const QPoint &pos, int cursorPosition) const

      TODO

   .. cpp:function:: QRect boundingRect() const

      TODO

   .. cpp:function:: int lineCount() const

      TODO

   .. cpp:function:: int maximumWidth() const

      TODO

   .. cpp:function:: Tui::ZTextLineRef lineAt(int i) const

      TODO

   .. cpp:function:: Tui::ZTextLineRef lineForTextPosition(int pos) const

      TODO

   .. cpp:function:: int previousCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const

      TODO

   .. cpp:function:: int nextCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const

      TODO

   .. cpp:function:: bool isValidCursorPosition(int pos) const

      TODO

.. cpp:class:: Tui::ZTextLineRef

   This class is copyable. It does not define comparison operators.

   TODO

   .. cpp:function:: bool isValid() const

      TODO

   .. cpp:function:: void draw(Tui::ZPainter painter, const QPoint &pos, Tui::ZTextStyle color, Tui::ZTextStyle formattingChars, const QVector<Tui::ZFormatRange> &ranges = QVector<Tui::ZFormatRange>()) const

      TODO

   .. cpp:function:: int cursorToX(int *cursorPos, Tui::ZTextLayout::Edge edge) const

      TODO

   .. cpp:function:: int cursorToX(int cursorPos, Tui::ZTextLayout::Edge edge) const

      TODO

   .. cpp:function:: int xToCursor(int x) const

      TODO

   .. cpp:function:: void setPosition(const QPoint &pos)

      TODO

   .. cpp:function:: QPoint position() const

      TODO

   .. cpp:function:: int x() const

      TODO

   .. cpp:function:: int y() const

      TODO

   .. cpp:function:: int width() const

      TODO

   .. cpp:function:: int height() const

      TODO

   .. cpp:function:: QRect rect() const

      TODO

   .. cpp:function:: int lineNumber() const

      TODO

   .. cpp:function:: int textStart() const

      TODO

   .. cpp:function:: int textLength() const

      TODO

   .. cpp:function:: void setLineWidth(int width)

      TODO
