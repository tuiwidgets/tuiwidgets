.. _ZMenu:

ZMenu
=====

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/menue-popupmenu.tpi

     A popup menu


``ZMenu`` can be used to create popup menus.
A popup (or context) menu can be used to offer multiple action related to the currently selected item.

After the menu is opened, it can then be navigated using the cursor keys and a command selected using :kbd:`Enter`
or by pressing the highlighted character or the menu can be left using :kbd:`Esc`.

The menu system uses :ref:`commands <ZCommandManager>` to determine if a menu item is currently available and to trigger
an action if the user activates a menu item.

The application must setup the menu with a collection of :ref:`menu items <ZMenuItem>` to gain a functional menu using
:cpp:func:`void Tui::ZMenu::setItems(QVector<Tui::ZMenuItem> items)`.
Currenty sub-menus are not supported.

The menu is usually created with the root widget as parent and opened by using the
:cpp:func:`void Tui::ZMenu::popup(const QPoint &p)` function.

See also: :ref:`ZMenuItem`, :ref:`ZMenubar`

Example
-------

.. literalinclude:: examples/widgets/menu.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end

Keyboard Usage
--------------

..
  REMARK: The keys that only work when `parentMenu` is set are not documented here, they are only documented in ZMenubar

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - :kbd:`Esc`
      - Close the menu

   *  - :kbd:`F10`
      - Close the menu

   *  - :kbd:`Enter`
      - Active the current menu item

   *  - :kbd:`↑`
      - Cycle through menu items up

   *  - :kbd:`↓`
      - Cycle through menu items down

   *  - (highlighted letter)
      - Active the menu item with the corresponding letter


Behavior
--------

The menu uses stacking layer 20000 by default and acts like a window with
:cpp:func:`extend viewport <bool Tui::ZWindowFacet::isExtendViewport() const>` enabled.

The menu ensures that its parent widget has a :ref:`command manager <ZCommandManager>` by calling
:cpp:func:`~ZCommandManager *Tui::ZWidget::ensureCommandManager()` on its parent.

When activated the menu bar grabs the keyboard.
It then allows the user to browse the menu and activate the menu items.

(ZMenu does not support use as popup menu when disabled)

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``menu.fg``, ``menu.bg``
      - Main color for the menu

   *  - ``menu.shortcut.fg``, ``menu.shortcut.bg``
      - Shortcut character in menu items

   *  - ``menu.selected.bg``, ``menu.selected.fg``
      - Selected menu items

   *  - ``menu.selected.shortcut.fg``, ``menu.selected.shortcut.bg``
      - Shortcut character in selected menu items

   *  - ``menu.disabled.fg``, ``menu.disabled.bg``
      - Menu items with disabled command or disabled menu bar

   *  - ``menu.selected.disabled.fg``, ``menu.selected.disabled.bg``
      - Selected menu items with disabled command


ZMenu
-----

.. cpp:class:: Tui::ZMenu : public Tui::ZWidget

   A popup menu widget.

   **Functions**

   .. cpp:function:: void setItems(QVector<Tui::ZMenuItem> items)
   .. cpp:function:: QVector<Tui::ZMenuItem> items() const

      The menu items to display in the menu.

      Currently sub-menus are not supported.

   .. cpp:function:: void setParentMenu(Tui::ZMenubar *menu)

      Used by :cpp:class:`Tui::ZMenubar` to implement its sub-menus.

   .. cpp:function:: void popup(const QPoint &p)

      Open this popup menu at the given point.

      This grabs the keyboard and displays the menu with an automatic size and a position based on ``p``.

   **Signals**

   .. cpp:function:: void aboutToHide()

      This signal is emitted just before the popup menu is hidden on close.

   .. cpp:function:: void aboutToShow()

      This signal is emitted just before the popup menu is shown.
