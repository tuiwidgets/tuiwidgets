.. _ZColor:

ZColor
======

Color handling in terminals works in distinct color kinds:

* default color
* named colors
* indexed colors
* rgb/direct colors

The default color further has the special property that it is displayed differently when uses as foreground color
versus background color.
That is using the default color as both foreground and background color usually yields readable high contrast text.
The default color is often configurable separatly for the user of the terminal.

The named colors are 8 basic colors plus their bright variants.
Terminal usually allow users to configure these colors and many users make use of that configability.

The indexed colors are divided into 3 parts.
Color indicies 0-15 often correspond to the named colors, possible with some differences in handling
(i.e. bright named colors might default to bold, while the same indicies might not use auto bold).
The following color indicies are usually divided into a color cube and a ramp of grey tones.

The rgb colors allow direct usage of given rgb color values.
The colors use 8 bit resolution for each color channel.
But of course some terminals internally use lower resolution.
If Tui Widgets know the terminal does not support rgb color it internally translates them to indexed colors, most
terminals support those.

Of course terminals differ in support for color kinds.
Also in some situations users expect the application to use named colors to fit with their selected color theming.
Thus Tui Widgets exposes all color kinds discusses and lets the application select which colors to use.
Although the default palette uses rgb colors and the automatic conversion.

If the application uses RGB colors, it can additionally use the :cpp:class:`Tui::ZColorHSV` class to ease calculation of
related colors.

.. cpp:enum:: Tui::TerminalColor

  The ``TerminalColor`` enum represents the named terminal colors.

  .. rst-class:: tw-invisible
  .. cpp:enumerator:: black
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: darkGray
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: lightGray
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightWhite
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: blue
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: green
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: cyan
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: red
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: magenta
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: yellow
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightBlue
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightGreen
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightCyan
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightRed
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightMagenta
  .. rst-class:: tw-invisible
  .. cpp:enumerator:: brightYellow

.. list-table:: named colors
   :header-rows: 1
   :align: left

   * - enum
     - description
     - enum (bright)
     - description

   * - black (0)
     - black
     - darkGray (8)
     - dark grey

   * - red (1)
     - red
     - brightRed (9)
     - bright red

   * - green (2)
     - green
     - brightGreen (10)
     - bright green

   * - yellow (3)
     - yellow
     - brightYellow (11)
     - bright yellow

   * - blue (4)
     - blue
     - brightBlue (12)
     - bright blue

   * - magenta (5)
     - magenta
     - brightMagenta (13)
     - bright magenta

   * - cyan (6)
     - cyan
     - brightCyan (14)
     - bright cyan

   * - lightGray (7)
     - light gray
     - brightWhite (15)
     - white


The following RGB colors are predefined as constants for application use in Tui::Colors:

.. list-table:: predefined rgb values
   :header-rows: 1
   :align: left

   * - name
     - description
     - name (bright)
     - description

   * - black
     - :colorchip:`#000000` black (#000000)
     - darkGray
     - :colorchip:`#555555` dark grey (#555555)

   * - red
     - :colorchip:`#aa0000` red (#aa0000)
     - brightRed
     - :colorchip:`#ff5555` bright red (#ff5555)

   * - green
     - :colorchip:`#00aa00` green (#00aa00)
     - brightGreen
     - :colorchip:`#55ff55` bright green (#55ff55)

   * - yellow
     - :colorchip:`#aaaa00` yellow (#aaaa00)
     - brightYellow
     - :colorchip:`#ffff55` bright yellow (#ffff55)

   * - blue
     - :colorchip:`#0000aa` blue (#0000aa)
     - brightBlue
     - :colorchip:`#5555ff` bright blue (#5555ff)

   * - magenta
     - :colorchip:`#aa00aa` magenta (#aa00aa)
     - brightMagenta
     - :colorchip:`#ff55ff` bright magenta (#ff55ff)

   * - cyan
     - :colorchip:`#00aaaa` cyan (#00aaaa)
     - brightCyan
     - :colorchip:`#55ffff` bright cyan (#55ffff)

   * - lightGray
     - :colorchip:`#aaaaaa` light gray (#aaaaaa)
     - brightWhite
     - :colorchip:`#ffffff` white (#ffffff)

.. cpp:class:: Tui::ZColor

   This class represents colors in Tui Widgets.
   It's copyable, assignable and supports equality comparasion.

   .. cpp:enum:: ColorType

      .. cpp:enumerator:: RGB
      .. cpp:enumerator:: Default
      .. cpp:enumerator:: Terminal
      .. cpp:enumerator:: TerminalIndexed

   **constructors**

   | :cpp:func:`Tui::ZColor::ZColor(int r, int g, int b) <void Tui::ZColor::ZColor(int r, int g, int b)>`

   **static functions**

   | :cpp:func:`~Tui::ZColor Tui::ZColor::defaultColor()`
   | :cpp:func:`fromHsv(const Tui::ZColorHSV &hsv) <Tui::ZColor Tui::ZColor::fromHsv(const Tui::ZColorHSV &hsv)>`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromHsv(double hue, double saturation, double value)`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromHsvStrict(double hue, double saturation, double value)`
   | :cpp:func:`fromHsvStrict(const Tui::ZColorHSV &hsv) <Tui::ZColor Tui::ZColor::fromHsvStrict(const Tui::ZColorHSV &hsv)>`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromRgb(int r, int g, int b)`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromTerminalColor(TerminalColor color)`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromTerminalColor(int color)`
   | :cpp:func:`~Tui::ZColor Tui::ZColor::fromTerminalColorIndexed(int color)`

   **functions**

   | :cpp:func:`~int Tui::ZColor::blue() const`
   | :cpp:func:`~int Tui::ZColor::blueOrGuess() const`
   | :cpp:func:`~int Tui::ZColor::green() const`
   | :cpp:func:`~int Tui::ZColor::greenOrGuess() const`
   | :cpp:func:`~uint32_t Tui::ZColor::nativeValue() const`
   | :cpp:func:`~int Tui::ZColor::red() const`
   | :cpp:func:`~int Tui::ZColor::redOrGuess() const`
   | :cpp:func:`~void Tui::ZColor::setBlue(int blue)`
   | :cpp:func:`~void Tui::ZColor::setGreen(int green)`
   | :cpp:func:`~void Tui::ZColor::setRed(int red)`
   | :cpp:func:`~Tui::TerminalColor Tui::ZColor::terminalColor() const`
   | :cpp:func:`~int Tui::ZColor::terminalColorIndexed() const`
   | :cpp:func:`~Tui::ZColorHSV Tui::ZColor::toHsv() const`

|

.. cpp:namespace:: Tui::ZColor

.. cpp:function:: ZColor(int r, int g, int b)

   Creates an ZColor instance of type RGB using ``r``, ``g`` and ``b`` as the color values.

.. cpp:function:: Tui::ZColor::ColorType colorType() const

   Returns the type of color the instance represents.

.. cpp:function:: int red() const
.. cpp:function:: void setRed(int red)
.. cpp:function:: int green() const
.. cpp:function:: void setGreen(int green)
.. cpp:function:: int blue() const
.. cpp:function:: void setBlue(int blue)

   Only for :cpp:expr:`colorType() == ColorType::RGB`.

   Getters and setters for RGB values.
   If the setters are used on a color of another type the type will be changed to RGB and all other channels are set to
   zero.

   Use :cpp:func:`Tui::ZColor fromRgb(int r, int g, int b)` to create a new instance with a specific rgb color.

.. cpp:function:: int terminalColorIndexed() const

   Only for :cpp:expr:`colorType() == ColorType::TerminalIndexed`.

   Returns the index of the indexed color.

   Use :cpp:func:`Tui::ZColor fromTerminalColorIndexed(int color)` to create a new instance with a specific indexed color.

.. cpp:function:: Tui::TerminalColor terminalColor() const

   Only for :cpp:expr:`colorType() == ColorType::Terminal`.

   Returns the enum value of the named color.

   Use :cpp:func:`Tui::ZColor fromTerminalColor(TerminalColor color)` to create a new instance with a specific named color.

.. cpp:function:: int redOrGuess() const
.. cpp:function:: int greenOrGuess() const
.. cpp:function:: int blueOrGuess() const

   Returns the color component for RGB type colors or a guess of the color component for other color types.

.. cpp:function:: Tui::ZColorHSV toHsv() const

   Returns the color converted to HSV.
   For non RGB type colors it uses guesses for the color like :cpp:func:`~int Tui::ZColor::redOrGuess() const` and co.

.. cpp:function:: uint32_t nativeValue() const

   Returns the color as a termpaint color value.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor defaultColor()

   Returns a ``ZColor`` with the default color.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromRgb(int r, int g, int b)

   Returns a ``ZColor`` with the RGB color with components ``r``, ``g`` and ``b``.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromTerminalColor(TerminalColor color)

   Returns a ``ZColor`` with the terminal color ``color``.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromTerminalColor(int color)

   Returns a ``ZColor`` with the terminal color ``static_cast<Tui::TerminalColor>(color)``.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromTerminalColorIndexed(int color)

   Returns a ``ZColor`` with the indexed color ``color``.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromHsv(const Tui::ZColorHSV &hsv)

   Returns a ``ZColor`` with RGB type converted from the HSV color ``hsv``.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromHsv(double hue, double saturation, double value)

   Returns a ``ZColor`` with RGB type converted from the HSV color components ``hue``, ``saturation`` and ``value``.

   The ranges for the components are:

   * 0 <= hue <= 360
   * 0 <= saturation <= 1
   * 0 <= value <= 1

   Values outside are clipped for saturation and value. Values outside for hue are wrapped back into the allowed range.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromHsvStrict(double hue, double saturation, double value)

   Like :cpp:func:`~Tui::ZColor Tui::ZColor::fromHsv(double hue, double saturation, double value)` but only valid if
   the parameters are in the allowed range.

.. rst-class:: tw-static
.. cpp:function:: Tui::ZColor fromHsvStrict(const Tui::ZColorHSV &hsv)

   Like :cpp:func:`~Tui::ZColor Tui::ZColor::fromHsv(const Tui::ZColorHSV &hsv)` but only valid if the components in
   ``hsv`` are in the allowed range.

.. cpp:namespace:: NULL

HSV
---

.. cpp:class:: Tui::ZColorHSV

   ``ZColorHSV`` is a simple class to store a color as hue, saturation and value.

   It's copyable, assignable and supports equality comparasion.

   .. cpp:function:: double hue() const
   .. cpp:function:: void setHue(double hue)
   .. cpp:function:: double saturation() const
   .. cpp:function:: void setSaturation(double saturation)
   .. cpp:function:: double value() const
   .. cpp:function:: void setValue(double value)

      All components have getters and setters.
