.. _ZLayout:

ZLayout
=======

Tui Widgets uses classes derived from :cpp:class:`Tui::ZLayout` to dynamically place widgets based on a specified
structure and the widgets requested size and layout policies.

Layouts can be nested to achieve a composed structure.
If a widget has a layout set (using :cpp:func:`void Tui::ZWidget::setLayout(ZLayout *l)` then children connected
to that layout will be automatically placed.

For layouts to properly layout widgets the parent of the widget and the widget ancestor of the layout need to match.

Using Layouts
-------------

Exact usage an layout class depends on the class.
But the common pattern is to add a layout to the parent widget using :cpp:func:`void Tui::ZWidget::setLayout(ZLayout *l)`
and then inserting widgets or further layout class instances into the layout using layout specific methods.
Widgets placed into a layout need to have the same parent widget as the layout.

For example when using a :cpp:class:`Tui::ZVBoxLayout` the widgets can be added using
:cpp:func:`void Tui::ZVBoxLayout::addWidget(ZWidget *w)` and layouts can be added using
:cpp:func:`void Tui::ZVBoxLayout::add(ZLayout *l)`.


Implementing Layouts
--------------------

Custom Layouts derive from :cpp:class:`Tui::ZLayout`.
They need to store the widgets and layouts added to them. For this storage it is useful to use
:cpp:class:`Tui::ZLayoutItem` as abstraction to handle widgets and sub-layouts with the same code.

The layout should reimplement :cpp:func:`QSize Tui::ZLayoutItem::sizeHint() const`,
:cpp:func:`SizePolicy Tui::ZLayoutItem::sizePolicyH() const` and
:cpp:func:`SizePolicy Tui::ZLayoutItem::sizePolicyH() const` to return the requested size and the combined layout
policies of its items and the parts added by the layout.

The other important function to reimplement is :cpp:func:`void Tui::ZLayoutItem::setGeometry(QRect r)` which is called
with the rectangle available for the layout to allocated its items.
Although it is named like a setter, typical implementations don't save ``r`` but sub allocate the space to the items
in the layout and call setGeometry with the individual allocation on each item.

When items of a layout change (in number or properties) or layout parameters (spacing, etc) change the layout
should call :cpp:func:`void Tui::ZLayout::relayout()` to trigger a new layout cycle and get called back later to
update its items' layout.

ZLayout
-------

.. cpp:class:: Tui::ZLayout : public QObject, public ZLayoutItem

   Base class for all layout classes.

   **Functions**

   .. rst-class:: tw-virtual
   .. cpp:function:: void widgetEvent(QEvent *event)

      If this layout is the top most layout of a widget this function is called for all event the widget recieves.

      This method handles necessary logic for layout cycle, terminal change and widget resize events.

      When overriding this function in a derived class make sure to always call the base function.

   .. cpp:function:: ZWidget *widget() const

      Returns the widget ancestor of this layout or :cpp:expr:`nullptr` if none exists.

   **Protected Functions**

   .. cpp:function:: void relayout()

      Call this on any change that changes (or might change) the position of the items in the layout.

      Calling this function will trigger a new layout cycle.

   **Reimplemented Functions**

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: QSize sizeHint() const

      Returns :cpp:expr:`QRect()`.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: SizePolicy sizePolicyH() const

      Returns :cpp:expr:`Tui::SizePolicy::Preferred`.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: SizePolicy sizePolicyV() const

      Returns :cpp:expr:`Tui::SizePolicy::Preferred`.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: bool isVisible() const

      Returns :cpp:expr:`true`.

   .. rst-class:: tw-static
   .. cpp:function:: bool isSpacer() const

      Returns :cpp:expr:`false`.


ZLayoutItem
-----------

.. cpp:class:: Tui::ZLayoutItem

   Abstract class to present a uniform interface for layouts, widgets and spacers for usage as items in a layout.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void setGeometry(QRect r)

      This method is called when the size allocation of the item changes.
      The implementation needs to adjust the geometry of the layouted item to the rect ``r``.
      If the contents of the item is itself layouted it should trigger relayout with the new geometry.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: QSize sizeHint() const

      Returns the composite size hint of this item.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: SizePolicy sizePolicyH() const

      Returns the composite horizontal size hint of this item.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: SizePolicy sizePolicyV() const

      Returns the composite vertical size hint of this item.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: bool isVisible() const

      Returns if the item is currently visible and thus should be allocated space.

   .. rst-class:: tw-static
   .. cpp:function:: bool isSpacer() const

      Returns :cpp:expr:`true` if the item counts as a spacer.

   .. rst-class:: tw-static
   .. cpp:function:: std::unique_ptr<ZLayoutItem> wrapWidget(ZWidget *widget)

      Wraps a widget into a newly allocated layout item.
