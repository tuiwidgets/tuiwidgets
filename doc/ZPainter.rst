.. ZPainter:

ZPainter
========

ZPainter is responsible for getting characters and colors onto the terminal or memory buffers.

A ZPainter instance can be obtained from a :cpp:func:`~Tui::ZEventType::paint()` event, an :cpp:class:`Tui::ZImage` or
when not using widgets from a :cpp:class:`Tui::ZTerminal`.

..
  REM
  describe painting model.

.. cpp:class:: Tui::ZPainter

   This class is copyable.  It does not define comparison operators.

   **Constructors**

   (only copy constructor)

   **Functions**

   | :cpp:func:`void clear(Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})`
   | :cpp:func:`void clearRect(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})`
   | :cpp:func:`void clearRectWithChar(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})`
   | :cpp:func:`void clearSoftwrapMarker(int x, int y)`
   | :cpp:func:`void clearWithChar(Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})`
   | :cpp:func:`void drawImage(int x, int y, const Tui::ZImage &sourceImage, int sourceX = 0, int sourceY = 0, int width = -1, int height = -1)`
   | :cpp:func:`void setBackground(int x, int y, Tui::ZColor bg)`
   | :cpp:func:`void setCursor(int x, int y)`
   | :cpp:func:`void setForeground(int x, int y, Tui::ZColor fg)`
   | :cpp:func:`void setSoftwrapMarker(int x, int y)`
   | :cpp:func:`void setWidget(Tui::ZWidget *widget)`
   | :cpp:func:`Tui::ZTextMetrics textMetrics() const`
   | :cpp:func:`Tui::ZPainter translateAndClip(QRect transform)`
   | :cpp:func:`Tui::ZPainter translateAndClip(int x, int y, int width, int height)`
   | :cpp:func:`void writeWithAttributes(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithAttributes(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)`
   | :cpp:func:`void writeWithColors(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg)`
   | :cpp:func:`void writeWithColors(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg)`


Members
-------

.. cpp:namespace:: Tui::ZPainter

.. cpp:function:: Tui::ZPainter translateAndClip(QRect transform)
.. cpp:function:: Tui::ZPainter translateAndClip(int x, int y, int width, int height)

   Returns a new ZPainter instance TODO.

.. cpp:function:: void writeWithColors(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg)

   |noconv|

.. cpp:function:: void writeWithColors(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg)

   |noconv|

.. cpp:function:: void writeWithColors(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg)

   |noconv|

.. cpp:function:: void writeWithAttributes(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)

   |noconv|

.. cpp:function:: void writeWithAttributes(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)

   |noconv|

.. cpp:function:: void writeWithAttributes(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)

   |noconv|

.. cpp:function:: void clear(Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})
.. cpp:function:: void clearWithChar(Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})
.. cpp:function:: void clearRect(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})
.. cpp:function:: void clearRectWithChar(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})
.. cpp:function:: void setSoftwrapMarker(int x, int y)
.. cpp:function:: void clearSoftwrapMarker(int x, int y)
.. cpp:function:: void drawImage(int x, int y, const Tui::ZImage &sourceImage, int sourceX = 0, int sourceY = 0, int width = -1, int height = -1)
.. cpp:function:: void setForeground(int x, int y, Tui::ZColor fg)
.. cpp:function:: void setBackground(int x, int y, Tui::ZColor bg)
.. cpp:function:: void setCursor(int x, int y)
.. cpp:function:: void setWidget(Tui::ZWidget *widget)

   Used by in the paint event handling in :cpp:class:`Tui::ZWidget`.
   It should not be needed to call this from normal application code.

.. cpp:function:: Tui::ZTextMetrics textMetrics() const

   Returns a :cpp:class:`Tui::ZTextMetrics` instance matching the painter.

.. |noconv| replace:: This overload participates in overload resolution only if the ``string`` parameter matches without
   implicit conversion.
