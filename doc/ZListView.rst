.. _ZListView:

ZListView
=========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/listview.tpi

     list view

* .. figure:: tpi/listview-focus.tpi

     in focused state

* .. figure:: tpi/listview-disabled.tpi

     in disabled state

* .. figure:: tpi/listview-decoration.tpi

     with decorated item

A list view allows the user to select an item from a list.
It supports scrolling if the amount of items does not fit into its geometry.

The items in the list view can come either from a internally managed model or from a externally managed
:cpp:class:`QAbstractItemModel` derived Qt item model.

Currently ``ZListView`` does not support selecting multiple items.

When used with a model the list view displays the :cpp:enumerator:`Qt::DisplayRole`.
Additionally it supports a (possibly colored) decoration on the left side of the item text.
This allows adding additional information to the displayed options.
To keep alignment of the items it is recommended to keep the width of the decoration plus the
amount of space after the decoration(:cpp:enumerator:`Tui::LeftDecorationSpaceRole`) consistent over all items.
The decoration is configured using the Qt item roles :cpp:enumerator:`Tui::LeftDecorationRole` for the text,
:cpp:enumerator:`Tui::LeftDecorationFgRole` and :cpp:enumerator:`Tui::LeftDecorationBgRole`
(both of type :cpp:class:`Tui::ZColor`) for overriding the color of the decoration, and
:cpp:enumerator:`Tui::LeftDecorationSpaceRole` (a number) for additional uncolored cells after the decoration.

Example
-------

Using internal item storage:

.. literalinclude:: examples/widgets/listview.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:

Using items from a external :cpp:class:`QAbstractItemModel`:

.. literalinclude:: examples/widgets/listview.cpp
    :start-after: // snippet-model-start
    :end-before:  // snippet-model-end
    :dedent:


Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :widths: 33 67
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - :kbd:`↑`
      - Move selection to previous item (does not cycle to bottom)

   *  - :kbd:`↓`
      - Move selection to next item (does not cycle to top)

   *  - :kbd:`Home`
      - Select first item

   *  - :kbd:`End`
      - Select last item

   *  - :kbd:`Page Up`
      - Select item one page up

   *  - :kbd:`Page Down`
      - Select item one page down

   *  - :kbd:`Enter`
      - emit :cpp:func:`~void Tui::ZListView::enterPressed(int selected)` signal

   *  - (any letter)
      - Search next item beginning with the typed letter

Behavior
--------

List views by default accept focus and have a expanding vertical and horizontal layout policy.
The size request of a list view is currently empty(i.e. to use in layouts use of
:cpp:func:`~void Tui::ZWidget::setMinimumSize(int w, int h)` is required).

List views have a size hint of :cpp:expr:`(10, 3)` as placeholder.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - | ``dataview.fg``,
        | ``dataview.bg``
      - Body of the |control| (active, **unfocused**)

   *  - | ``dataview.selected.fg``,
        | ``dataview.selected.bg``
      - selected items in list (active, **unfocused**)

   *  - | ``dataview.selected.focused.fg``,
        | ``dataview.selected.focused.bg``
      - selected items in list (active, **focused**)

   *  - | ``dataview.disabled.fg``,
        | ``dataview.disabled.bg``
      - Body of the |control| (**disabled**)

   *  - | ``dataview.disabled.selected.fg``,
        | ``dataview.disabled.selected.bg``
      - selected items in list (**disabled**)

ZListView
---------

.. cpp:class:: Tui::ZListView : public Tui::ZWidget

   A list view widget.

   **Enums**

   .. cpp:enum:: ScrollHint

      .. cpp:enumerator:: EnsureVisible

         Assure that the reference item is visible.

      .. cpp:enumerator:: PositionAtTop

         Assure that the reference item is at the top of the visible area.

      .. cpp:enumerator:: PositionAtBottom

         Assure that the reference item is at the bottom of the visible area.

      .. cpp:enumerator:: PositionAtCenter

         Assure that the reference item is at the center of the visible area.

   **Functions**

   .. cpp:function:: void setItems(const QStringList& newItems)

      Set the items to display and use the internal model.

      If an external model was in use detaches from it and switches to the internal model.

   .. cpp:function:: QStringList items() const

      Returns a list of the contents (Qt::DisplayRole) for all items in the list view's current model.

   .. cpp:function:: QString currentItem() const

      Returns the contents (Qt::DisplayRole) of the current item or an empty string if there is no current item.

   .. cpp:function:: void setModel(QAbstractItemModel *model)

      Set a new model for the list view.

      If the model is changed, this detaches current model and attaches to the new model.

   .. cpp:function:: QAbstractItemModel *model() const

      Returns the currently used model.

      This can be either the internal model or a application-set model.

   .. cpp:function:: void setCurrentIndex(QModelIndex index)
   .. cpp:function:: QModelIndex currentIndex() const

      The current selection as :cpp:class:`QModelIndex`.

   .. cpp:function:: QItemSelectionModel *selectionModel() const

      Returns the current selection model.

   .. cpp:function:: void scrollTo(const QModelIndex& index, Tui::ZListView::ScrollHint hint=EnsureVisible)

      Scrolls the list view so that the item at ``index`` is visible.

      The list view will try to scroll according to ``scrollHint``.

   **Signals**

   .. cpp:function:: void enterPressed(int selected)

      This signal is emitted when the user presses the :kbd:`Enter` key with the index of the currently selected item as
      ``selected``.

.. |control| replace:: list view
