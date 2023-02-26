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
   | :cpp:func:`void drawImageWithTiling(int x, int y, const Tui::ZImage &sourceImage, int sourceX, int sourceY, int width, int height, Tui::ZTilingMode tileLeft, Tui::ZTilingMode tileRight)`
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

   Returns a new ZPainter instance with adjusted transform and clip.

   The new clipping region is the intersection of the previous clipping region with the rectangle specified as parameter,
   relative to the current transform.

   The new transform is created such, that in the new painter :cpp:expr:`(0, 0)` refers to the same global point as
   :cpp:expr:`(x, y)` did with the previous transform.

.. cpp:function:: void writeWithColors(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg)
.. cpp:function:: void writeWithColors(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithColors(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithColors(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithColors(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg)

   |noconv|

   Write the string ``string`` starting from position :cpp:expr:`(x, y)` using foreground color ``fg`` and background
   color ``bg`` without attributes.

   |clipandtransform|

   When using the overloads using ``std::string`` or ``char*`` the string has to be passed in utf-8 form.
   When using the overload using ``char16_t`` the string has to be passed in utf-16 form.

.. cpp:function:: void writeWithAttributes(int x, int y, const QString &string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const QChar *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const char16_t *string, int size, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. cpp:function:: void writeWithAttributes(int x, int y, const char *stringUtf8, int utf8CodeUnits, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithAttributes(int x, int y, QStringView string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithAttributes(int x, int y, std::u16string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)
.. rst-class:: tw-noconv
.. cpp:function:: void writeWithAttributes(int x, int y, std::string_view string, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr)

   |noconv|

   Write the string ``string`` starting from position :cpp:expr:`(x, y)` using foreground color ``fg`` and background
   color ``bg`` with the attributes given in ``attr``.

   |clipandtransform|

   When using the overloads using ``std::string`` or ``char*`` the string has to be passed in utf-8 form.
   When using the overload using ``char16_t`` the string has to be passed in utf-16 form.


.. cpp:function:: void clear(Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})
.. cpp:function:: void clearWithChar(Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})

   Paints all cells inside the clip region using foreground color ``fg`` and background color ``bg`` with
   attributes ``attr``.

   The ``WithChar`` variant uses ``fillChar`` to paint cells inside the clip region unless ``fillChar`` is 127.
   Otherwise the cells set to the cleared state.

.. cpp:function:: void clearRect(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, Tui::ZTextAttributes attr = {})
.. cpp:function:: void clearRectWithChar(int x, int y, int width, int height, Tui::ZColor fg, Tui::ZColor bg, int fillChar, Tui::ZTextAttributes attr = {})

   Paints all cells inside the rectangle starting at ``x``, ``y`` with width ``width`` and height ``height``
   using foreground color ``fg`` and background color ``bg`` with attributes ``attr``.

   |clipandtransform|

   The ``WithChar`` variant uses ``fillChar`` to paint cells inside the rectangle unless ``fillChar`` is 127.
   Otherwise the cells set to the cleared state.

.. cpp:function:: void setSoftwrapMarker(int x, int y)

   Sets a softwarp marker at position :cpp:expr:`(x, y)`.

   If the right-most cell of a line of the whole terminal and the left-most cell of the next line of the whole terminal
   have a softwrap marker, both lines will be send to the terminal as wrapped in supported terminals.
   This influences for example how hard line breaks are handled in text copied from the terminal.

   |clipandtransform|

.. cpp:function:: void clearSoftwrapMarker(int x, int y)

   Removes a softwarp marker at position :cpp:expr:`(x, y)`.

   |clipandtransform|

.. cpp:function:: void drawImage(int x, int y, const Tui::ZImage &sourceImage, int sourceX = 0, int sourceY = 0, int width = -1, int height = -1)
.. cpp:function:: void drawImageWithTiling(int x, int y, const Tui::ZImage &sourceImage, int sourceX, int sourceY, int width, int height, Tui::ZTilingMode tileLeft, Tui::ZTilingMode tileRight)

   Draws the rectangle starting at ``sourceX``, ``sourceY`` with width ``width`` and height ``height`` of the image
   given as ``sourceImage``.
   If ``width`` is :cpp:expr:`-1` then the maximal width still inside the source image is used.
   If ``height`` is :cpp:expr:`-1` then the maximal height still inside the source image is used.
   The image is drawn starting at the point ``x``, ``y``.

   |clipandtransform|

   Handling of wide characters on the left or right edge of the source rectangle is selected using ``tileLeft`` and
   ``tileRight``. :cpp:enumerator:`Tui::ZTilingMode::NoTiling` is used in the variant without these parameters.

.. cpp:function:: void setForeground(int x, int y, Tui::ZColor fg)

   Set the foreground of the cell at :cpp:expr:`(x, y)` to the color ``fg`` without changing the cell contents or
   other attributes.

   |clipandtransform|

.. cpp:function:: void setBackground(int x, int y, Tui::ZColor bg)

   Set the background of the cell at :cpp:expr:`(x, y)` to the color ``fg`` without changing the cell contents or
   other attributes.

   |clipandtransform|

.. cpp:function:: void setCursor(int x, int y)

   Sets the cursor position to :cpp:expr:`(x, y)`.

   This needs the painter to have an associated widget and the cursor position is only changed if that widget has
   focus (and no keyboard grab of another widget is active)

   Positioning is subject to the current coordiante transform.

.. cpp:function:: void setWidget(Tui::ZWidget *widget)

   Used by in the paint event handling in :cpp:class:`Tui::ZWidget`.
   It should not be needed to call this from normal application code.

   Sets the associated widget of the painter.

.. cpp:function:: Tui::ZTextMetrics textMetrics() const

   Returns a :cpp:class:`Tui::ZTextMetrics` instance matching the painter.

.. |noconv| replace:: The overloads marked with ``noconv`` participates in overload resolution only if the ``string``
   parameter matches without implicit conversion.

.. |clipandtransform| replace:: Actual changes are subject to the current clipping region and positioning to the current
   coordiante transform.
