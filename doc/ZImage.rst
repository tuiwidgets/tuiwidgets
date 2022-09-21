.. _ZImage:

ZImage
======

.. cpp:class:: Tui::ZImage

   ZImage allows keeping terminal output in a memory buffer for later usage or to store and load.

   **constructors**

   | :cpp:func:`ZImage(const Tui::ZTerminal *terminal, int width, int height)> <void Tui::ZImage::ZImage(const Tui::ZTerminal *terminal, int width, int height)>`

   **static functions**

   | :cpp:func:`~std::unique_ptr<Tui::ZImage> Tui::ZImage::fromFile(const Tui::ZTerminal *terminal, const QString &fileName)`
   | :cpp:func:`~std::unique_ptr<Tui::ZImage> Tui::ZImage::fromByteArray(const Tui::ZTerminal *terminal, QByteArray data)`

   **functions**

   | :cpp:func:`~int Tui::ZImage::height() const`
   | :cpp:func:`~Tui::ZPainter Tui::ZImage::painter()`
   | :cpp:func:`~Tui::ZTextAttributes Tui::ZImage::peekAttributes(int x, int y) const`
   | :cpp:func:`~Tui::ZColor Tui::ZImage::peekBackground(int x, int y) const`
   | :cpp:func:`~Tui::ZColor Tui::ZImage::peekDecoration(int x, int y) const`
   | :cpp:func:`~Tui::ZColor Tui::ZImage::peekForground(int x, int y) const`
   | :cpp:func:`~bool Tui::ZImage::peekSoftwrapMarker(int x, int y) const`
   | :cpp:func:`~QString Tui::ZImage::peekText(int x, int y, int *left, int *right) const`
   | :cpp:func:`~bool Tui::ZImage::save(const QString &fileName) const`
   | :cpp:func:`~QByteArray Tui::ZImage::saveToByteArray() const`
   | :cpp:func:`~QSize Tui::ZImage::size() const`
   | :cpp:func:`swap(Tui::ZImage &other) <void Tui::ZImage::swap(Tui::ZImage &other)>`
   | :cpp:func:`~int Tui::ZImage::width() const`

Members
-------

.. cpp:namespace:: Tui::ZImage

.. cpp:function:: ZImage(const Tui::ZTerminal *terminal, int width, int height)

   Construct an empty image using terminal characteristics from ``terminal`` with
   the width ``width`` and height ``height``.

.. rst-class:: tw-static
.. cpp:function:: std::unique_ptr<Tui::ZImage> fromFile(const Tui::ZTerminal *terminal, const QString &fileName)

   Load an terminal image from the file named ``fileName`` and using the terminal characteristics from ``terminal``.

   Returns ``nullptr`` if the file could not be read or parsing failed.

.. rst-class:: tw-static
.. cpp:function:: std::unique_ptr<Tui::ZImage> fromByteArray(const Tui::ZTerminal *terminal, QByteArray data)

   Load an terminal image from memory buffer ``data`` and using the terminal characteristics from ``terminal``.

   Returns ``nullptr`` if the parsing failed.

.. cpp:function:: int width() const
.. cpp:function:: int height() const
.. cpp:function:: QSize size() const

   Returns the size of the buffer.

.. cpp:function:: bool save(const QString &fileName) const

   Save the contents of the image to file ``fileName``.

   Returns true on success.

.. cpp:function:: QByteArray saveToByteArray() const

   Save the contents of the image to a memory buffer and returns it as QByteArray.

.. cpp:function:: QString peekText(int x, int y, int *left, int *right) const

   Get text of cluster covering the cell at position ``x``, ``y``.

   If not nullptr, ``left`` and ``right`` will be set to the left most respectivly right most cell covered
   by the cluster.

.. cpp:function:: Tui::ZColor peekForground(int x, int y) const

   Get the foreground color of the cell at position ``x``, ``y``.

.. cpp:function:: Tui::ZColor peekBackground(int x, int y) const

   Get the background color of the cell at position ``x``, ``y``.

.. cpp:function:: Tui::ZColor peekDecoration(int x, int y) const

   Get the decoration color of the cell at position ``x``, ``y``.

.. cpp:function:: bool peekSoftwrapMarker(int x, int y) const

   Returns ``true`` if the cell at position ``x``, ``y`` contains a softwrap marker.

.. cpp:function:: Tui::ZTextAttributes peekAttributes(int x, int y) const

   Get the attributes of the cell at position ``x``, ``y``.

.. cpp:function:: Tui::ZPainter painter()

   Returns a :cpp:class:`Tui::ZPainter` for painting on the image.

.. cpp:function:: void swap(Tui::ZImage &other)

   Like ``std::swap`` for ``Tui::ZImage``.

.. cpp:namespace:: NULL
