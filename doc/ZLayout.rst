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

The stored pointers to widgets and sub layouts should removed when the referenced items are deleted or reparented.
For widgets, implementing
:cpp:func:`removeWidgetRecursively <void Tui::ZLayout::removeWidgetRecursively(Tui::ZWidget *widget)>` is enough, as the
layout infrastructure calls it when a widget needs to be removed.
For sub-layouts the layout has to override :cpp:func:`QObject::childEvent` and filter removal
events for :cpp:class:`Tui::ZLayout` children and implement removal itself.

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

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void removeWidgetRecursively(Tui::ZWidget *widget)

      Remove the widget ``widget`` from the layout and all its sub-layouts.

      This is needed to properly handle widgets that get destroyed or reparented out of the widget that contains the
      layout.

   .. cpp:function:: ZWidget *parentWidget() const

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


   **Protected Functions**

   .. cpp:function:: bool removeWidgetRecursivelyHelper(ZLayoutItem *layoutItem, ZWidget *widget)

      Helper for implementing :cpp:func:`removeWidgetRecursively`.

      If the layout internally uses :cpp:class:`Tui::ZLayoutItem` to abstract storage of items, it can use
      this helper to implement :cpp:func:`removeWidgetRecursively`.
      In most cases calling this function for each stored layout item and forgetting that item if the function returns
      :cpp:expr:`true` should be sufficient.

      If the function returns :cpp:expr:`true`, the item ``layoutItem`` was already deleted.

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

   .. rst-class:: tw-virtual
   .. cpp:function:: bool isSpacer() const

      Returns :cpp:expr:`true` if the item counts as a spacer.

   .. rst-class:: tw-virtual
   .. cpp:function:: ZWidget *widget()

      Returns the wrapped widget of the layout item or :cpp:expr:`nullptr` if no widget is wrapped.

      Should only be implemented by the class used in the implementation of :cpp:func:`wrapWidget` or classes that
      can be threated exactly the same.

      The base implementation always returns :cpp:expr:`nullptr`.

   .. rst-class:: tw-virtual
   .. cpp:function:: ZLayout *layout()

      Returns :cpp:expr:`this` cast to :cpp:expr:`ZLayout*` iff this instance is in fact a :cpp:class:`Tui::ZLayout`.

      The base implementation always returns :cpp:expr:`nullptr`.

   .. rst-class:: tw-static
   .. cpp:function:: std::unique_ptr<ZLayoutItem> wrapWidget(ZWidget *widget)

      Wraps a widget into a newly allocated layout item.
