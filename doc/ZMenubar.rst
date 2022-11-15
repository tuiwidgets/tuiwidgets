.. _ZMenubar:

ZMenubar
========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/menu.tpi

     A menubar

* .. figure:: tpi/menu-disabled.tpi

     in disabled state

* .. figure:: tpi/menu-submenu.tpi

     with submenu open


A menu bar allows users to browse and select from a fairly big number of commands.
It is a classic user interface element to enable users to discover and use functionality of an application without having to
read the manual.
For faster access on repeated use, often menu items additionally display a keyboard shortcut that also allows accessing
the command.

Usually applications have a global menu bar that is almost always reachable.
The menu bar can be activated by pressing :kbd:`F10` or by pressing :kbd:`Alt` in combination with one of the the
highlighted characters on the menu bar.
The menu can then be navigated using the cursor keys and a command selected using :kbd:`Enter` or by pressing the
highlighted character (this time without using :kbd:`Alt` as modifier) or the menu can be left using :kbd:`Esc`.

The menu system uses :ref:`commands <ZCommandManager>` to determine if a menu item is currently available and to trigger
an action if the user activates a menu item.

The application must setup the menu with a collection of :ref:`menu items <ZMenuItem>` to gain a functional menu using
:cpp:func:`void Tui::ZMenubar::setItems(QVector<Tui::ZMenuItem> items)`.
Currently only menu bar entries and one level of sub-menus are supported.

See also: :ref:`ZMenuItem`, :ref:`ZMenu`

Example
-------

.. literalinclude:: examples/widgets/menubar.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end

Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - Context
      - Result

   *  - :kbd:`F10`
      - application shortcut
      - Activate the menu bar

   *  - :kbd:`Alt` + (setup mnemonic)
      - | application shortcut
        | active menu bar
        | sub-menu
      - Activate/Open the menu item

   *  - :kbd:`←`
      - | active menu bar
        | sub-menu
      - Cycle through menu items / sub-menus left

   *  - :kbd:`→`
      - | active menu bar
        | sub-menu
      - Cycle through menu items / sub-menus right

   *  - :kbd:`Esc`
      - | active menu bar
        | sub-menu
      - Close the menu

   *  - :kbd:`F10`
      - | active menu bar
        | sub-menu
      - Close the menu

   *  - :kbd:`Enter`
      - | active menu bar
        | sub-menu
      - Active the current menu item

   *  - :kbd:`↑`
      - | active sub-menu
      - Cycle through menu items up

   *  - :kbd:`↓`
      - | active sub-menu
      - Cycle through menu items down

   *  - (highlighted letter)
      - | active sub-menu
      - Active the menu item with the corresponding letter


Behavior
--------

The menu bar does not accept focus and is one cell high and has a expanding vertical layout policy.
The size request of a menu bar is calculated from the width of the top level menu items.

The menu bar ensures that its parent widget has a :ref:`command manager <ZCommandManager>` by calling
:cpp:func:`~ZCommandManager *Tui::ZWidget::ensureCommandManager()` on its parent and then bind to it to track
command status changes and reflect them to the top-level menu item's shortcuts.

It setups :kbd:`F10` as an application context shortcut to activate the menu.
For the mnemonic letters of the top level menu items it similarly creates application context shortcuts to activate
them (possibly showing their sub items as sub-menu).

When activated the menu bar grabs the keyboard.
It then allows the user to browse the menu and activate the menu items.
If a top level menu item has sub items it will create a browsable sub-menu (using :cpp:class:`Tui::ZMenu`).
The keyboard grab is transfered between the menu bar and sub-menus as needed.


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


ZMenubar
--------

.. cpp:class:: Tui::ZMenubar : public Tui::ZWidget

   A menu bar widget.

   **Functions**

   .. cpp:function:: void setItems(QVector<Tui::ZMenuItem> items)
   .. cpp:function:: QVector<Tui::ZMenuItem> items() const

      The menu items to display in the menu.

      The items can contain nested items lists to support sub-menus.
      Currently only menu bar entries and one level of sub-menus are supported.

   .. cpp:function:: void keyActivate(Tui::ZKeyEvent *event)

      Used by sub-menus to implement :kbd:`Alt` + mnemonic handling.

   **Slots**

   .. cpp:function:: void left()

      Used by sub-menus to implement :kbd:`←`.

   .. cpp:function:: void right()

      Used by sub-menus to implement :kbd:`→`.

   .. cpp:function:: void close()

      Close any open sub-menu and deactivate the menu bar.
