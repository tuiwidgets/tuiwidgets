.. _ZWindowFacet:

ZWindowFacet
============

Widgets that act like a type of window should return a implementation of ``ZWindowFacet`` from
:cpp:func:`QObject *Tui::ZWidget::facet(const QMetaObject &metaObject) const` when queried for this facet.
This enables additional window behavior.

The window facet also is the interface to place a window into a :cpp:class:`Tui::ZWindowContainer` which allows the
container to take control over the placement of the window and to add items to the window's system menu.

ZWindowFacet
------------

.. rst-class:: tw-facet
.. cpp:class:: Tui::ZWindowFacet : public QObject

   A facet for window related information and behavior.

   .. rst-class:: tw-virtual
   .. cpp:function:: bool isExtendViewport() const

      If this function returns :cpp:expr:`true` and the window does not fit into the terminal size, the viewport mode of
      :cpp:class:`Tui::ZTerminal` will be enabled to allow scrolling to view all parts of the window.

      The base class always returns :cpp:expr:`false`.

   .. rst-class:: tw-virtual
   .. cpp:function:: bool isManuallyPlaced() const

      If this function returns :cpp:expr:`true`, the window will be considered as manually placed and automatic placement
      will be disabled.

      The base class always returns the last value passed to :cpp:func:`void setManuallyPlaced(bool manual)` or
      :cpp:expr:`true` if that function was not called yet.

   .. rst-class:: tw-virtual
   .. cpp:function:: void autoPlace(const QSize &available, ZWidget *self)

      For implementations of this facet that support this method and when automatic placement is enabled, it will
      apply automatic placement to the window which must be passed as ``self`` using ``available`` as the full size
      of the container.

      The base class will center the window in ``available``.

   .. rst-class:: tw-virtual
   .. cpp:function:: Tui::ZWindowContainer *container() const

      Return the current window container of the window associated with this facet instance.

      The base class always returns the last value passed to :cpp:func:`void setContainer(ZWindowContainer *container)`
      or :cpp:expr:`nullptr` if that function was not called yet.

   .. rst-class:: tw-virtual
   .. cpp:function:: void setContainer(ZWindowContainer *container)

      Set the current window container of the window associated with this facet instance.

      The base class saves the passed ``container`` and returns it when
      :cpp:func:`Tui::ZWindowContainer *Tui::ZWindowFacet::container() const` is called.

   .. rst-class:: tw-virtual
   .. cpp:function:: void setManuallyPlaced(bool manual)

      For implementations of this facet that support this method, it will set the value returned by
      :cpp:func:`bool isManuallyPlaced() const`.
      Otherwise calls to this function might be ignored.

      The base class save the passed ``manual`` and returns it when :cpp:func:`bool isManuallyPlaced() const`
      is called.
