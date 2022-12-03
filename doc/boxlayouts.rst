.. _boxlayouts:

Box Layouts
===========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/boxlayouts-intro-v.tpi

     vertical box layout with spacing 1

* .. figure:: tpi/boxlayouts-intro-h.tpi

     horizontal box layout with spacing 1

Box layouts are basic building blocks for dynamic widget layouting.

Box layouts either stack widgets vertically (:cpp:class:`Tui::ZVBoxLayout`) or place them side by side horizontally
(:cpp:class:`Tui::ZHBoxLayout`).

Additionally they allow to add automatic spacing between widgets, manual spacing between widgets and stretch spacing
that acts a bit like a spring.

..
  TODO: Add more details.

Stretch
-------

.. rst-class:: tw-figure-left
.. figure:: tpi/boxlayouts-stretch-start.tpi

   Horizontal box layout with stretch before 2 buttons

.. rst-class:: tw-figure-left
.. figure:: tpi/boxlayouts-stretch-mid.tpi

   Horizontal box layout with stretch between 2 buttons

Stretch elements allow creating space between items in the layout that expands to fill the available width.

They take space at the same priority as widgets using a expanding size policy.

Typical usage is placing a stretch element as left most element to align the following widgets to the right.

Placing one stretch element between widgets creates a space in the middle and pushes the widgets towards the edge
of the layout.

Nested
------

.. rst-class:: tw-figure-left
.. figure:: tpi/boxlayouts-nested1.tpi

   Horizontal box layout nested within vertial box layout.

.. rst-class:: tw-figure-left
.. figure:: tpi/boxlayouts-nested2.tpi

   Vertial box layout nested within horizontal box layout.


Nesting layouts allows for more complex arrangements of widgets.
It is important to consider in which order the items are nested, because nested box layouts do not create a grid, but
each layout's items are layouted individually.

ZVBoxLayout
-----------

.. cpp:class:: Tui::ZHBoxLayout

   This layout arranges items horizontally.
   Items are placed from left to right.

   .. cpp:function:: int spacing() const
   .. cpp:function:: void setSpacing(int sp)

      The ``spacing`` determines the amount of blank cells betweeen layouted items.

      It does not apply if one of the adjacent items itself is a spacer (according to
      :cpp:func:`bool Tui::ZLayoutItem::isSpacer() const`).

   .. cpp:function:: void addWidget(ZWidget *w)

      Adds a widget to be layouted.

   .. cpp:function:: void add(ZLayout *l)

      Adds a sub layout to be layouted.

   .. cpp:function:: void addSpacing(int size)

      Add manual spacing of ``size`` blank cells.
      Spacing set via :cpp:func::`void setSpacing(int sp)`` does not apply for this spacing.

   .. cpp:function:: void addStretch()

      Add a stretch item that acts like a spring in the layout and takes additional space with the same priority as
      :cpp:enumerator:`Expanding <Tui::SizePolicy::Expanding>` items.


ZHBoxLayout
-----------

.. cpp:class:: Tui::ZVBoxLayout

   This layout arranges items vertically.
   Items are placed from top to bottom.

   .. cpp:function:: int spacing() const
   .. cpp:function:: void setSpacing(int sp)

      The ``spacing`` determines the amount of blank cells betweeen layouted items.

      It does not apply if one of the adjacent items itself is a spacer (according to
      :cpp:func:`bool Tui::ZLayoutItem::isSpacer() const`).

   .. cpp:function:: void addWidget(ZWidget *w)

      Adds a widget to be layouted.

   .. cpp:function:: void add(ZLayout *l)

      Adds a sub layout to be layouted.

   .. cpp:function:: void addSpacing(int size)

      Add manual spacing of ``size`` blank cells.
      Spacing set via :cpp:func::`void setSpacing(int sp)` does not apply for this spacing.

   .. cpp:function:: void addStretch()

      Add a stretch item that acts like a spring in the layout and takes additional space with the same priority as
      :cpp:enumerator::`Expanding <Tui::SizePolicy::Expanding>` items.
