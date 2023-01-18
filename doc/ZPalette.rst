.. _ZPalette:

ZPalette
========

Tui Widgets uses a palette system for colors.

The palette assignes colors to symbols.
It also has a rule system to change color assignments when the path on the widget tree to the root contains a widget
with specific palette class.

ZPalette has predefined palettes which an application should use as a base for custom palettes.
This ensures that new color symbols introduced in future version get some kind of default even when using custom
palettes.



.. cpp:class:: Tui::ZPalette

   ZPalette stores color assignments for use in widget painting.

   It is copyable and defines no comparision operators.

   It the following predefined default palettes:


   .. rst-class:: tw-static
   .. cpp:function:: Tui::ZPalette::ZPalette classic()

      "classic" colorful white on blue palette.

   .. rst-class:: tw-static
   .. cpp:function:: Tui::ZPalette::ZPalette black()

      Monochrome-ish palette using mostly grey levels.


   .. cpp:class:: ColorDef

      Defines a color in the palette

      .. cpp:member:: Tui::ZImplicitSymbol name

         Symbol for the color to define.

      .. cpp:member:: Tui::ZColor color

         Color defined for the symbol.


   .. cpp:enum:: Type

      .. cpp:enumerator:: Publish

         The changes in the rule apply to the widget with the given palette classes and all descendants.

      .. cpp:enumerator:: Local

         The change in the command apply only to the widget with the given palette classes, colors descendants are
         not changed.

   .. cpp:class:: RuleCmd

      Defines a part of a :cpp:class:`Tui::ZPalette::RuleDef`.

      .. cpp:member:: Tui::ZPalette::Type type

         Type of change, either locally or including descendants.

      .. cpp:member:: Tui::ZImplicitSymbol name

         Symbol of the color to change.

      .. cpp:member:: Tui::ZImplicitSymbol reference

         Symbol of the color to use as new color.

   .. cpp:class:: RuleDef

      Defines a rule matching palette class combinations to color changes commands.

      .. cpp:member:: QSet<QString> classes

         The rule matches when a widget has all ``classes``

      .. cpp:member:: QList<Tui::ZPalette::RuleCmd> cmds

   .. rst-class:: tw-midspacebefore
   .. cpp:function:: void setColors(QList<Tui::ZPalette::ColorDef> newColors)

      Define colors for symbols as specified in ``newColors``.
      The defined colors take effect for the widget given this palette and its descendants unless overridden later.

   .. cpp:function:: addRules(QList<Tui::ZPalette::RuleDef> newRules)

      Define color assignment rules as specified in ``newRules``.
      Existing rules are not removed.

      The rules take effect on the widget given this plaette and its descendants.

      Rules with more more classes to match override assignments by rules with less classes to match.

   .. cpp:function:: bool isNull() const

      Returns ``true`` if not colors or rules are set in this palette.

   .. rst-class:: tw-static
   .. cpp:function:: ZColor getColor(ZWidget *targetWidget, ZImplicitSymbol x)

      Return a named color for a target widget.
      If the color was not defined it returns red (``#ff0000``) as placeholder.

      This is the backend for :cpp:func:`ZColor Tui::ZWidget::getColor(const ZImplicitSymbol &x)`,
      which applications should use instead.


Palette Symbols
---------------

The predefined palettes assume most widgets are inside windows (or dialogs).

Symbols available without a window:

.. list-table::
   :header-rows: 1
   :align: left

   * - name
     - description

   * - root.bg
     - background color for the root widget / backdrop

   * - root.fg
     - foreground color for the root widget / backdrop

   * - menu.bg
     - background for the menubar and for popup menus

   * - menu.fg
     - foreground for the menubar and for popup menus

   * - menu.disabled.bg
     - background for disabled menu items

   * - menu.disabled.fg
     - foreground for disabled menu items

   * - menu.shortcut.bg
     - background for menu item mnemonic (shortcut) letter

   * - menu.shortcut.fg
     - foreground for menu item mnemonic (shortcut) letter

   * - menu.selected.bg
     - background for the selected menu item

   * - menu.selected.fg
     - foreground for the selected menu item

   * - menu.selected.disabled.bg
     - background for the selected menu item in case it is disabled

   * - menu.selected.disabled.fg
     - foreground for the selected menu item in case it is disabled

   * - menu.selected.shortcut.bg
     - background for the selected menu item's mnemonic (shortcut) letter

   * - menu.selected.shortcut.fg
     - foreground for the selected menu item's mnemonic (shortcut) letter

Additional symbols available inside a window:

.. list-table::
   :header-rows: 1
   :align: left

   * - name
     - description

   * - bg
     - generic background color

   * - window.bg
     - background color of the window

   * - window.frame.focused.bg
     - background color of the frame of a focused window

   * - window.frame.focused.fg
     - foreground color of the frame of a focused window

   * - window.frame.focused.control.bg
     - background color of the control in the frame of a focused window

   * - window.frame.focused.control.fg
     - foreground color of the control in the frame of a focused window

   * - window.frame.unfocused.bg
     - background color of the frame of an unfocused window

   * - window.frame.unfocused.fg
     - foreground color of the frame of an unfocused window

   * - scrollbar.bg
     - scrollbar background color

   * - scrollbar.fg
     - scrollbar foreground color

   * - scrollbar.control.bg
     - scrollbar dragger control backgroud color

   * - scrollbar.control.fg
     - scrollbar dragger control foregroud color

   * - text.bg
     -

   * - text.fg
     -

   * - text.selected.bg
     -

   * - text.selected.fg
     -

   * - control.bg
     - background color for unfocused controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.fg
     - foreground color for unfocused controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.focused.bg
     - background color for focused controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.focused.fg
     - foreground color for focused controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.disabled.bg
     - background color for disabled controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.disabled.fg
     - foreground color for disabled controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.shortcut.bg
     - background color for mnemonic (shortcut) letter for controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - control.shortcut.fg
     - foreground color for mnemonic (shortcut) letter for controls like lables, radioboxes, checkboxes and similar non boxed controls.

   * - dataview.bg
     - background color for unselected items in unfocused data controls like list views.

   * - dataview.fg
     - foreground color for unselected items in unfocused data controls like list views.

   * - dataview.selected.bg
     - background color for selected items in unfocused data controls like list views.

   * - dataview.selected.fg
     - foreground color for selected items in unfocused data controls like list views.

   * - dataview.selected.focused.bg
     - background color for selected items in focused data controls like list views.

   * - dataview.selected.focused.fg
     - foreground color for selected items in focused data controls like list views.

   * - dataview.disabled.bg
     - background color for unselected items in disabled data controls like list views.

   * - dataview.disabled.fg
     - foreground color for unselected items in disabled data controls like list views.

   * - dataview.disabled.selected.bg
     - background color for selected items in disabled data controls like list views.

   * - dataview.disabled.selected.fg
     - foreground color for selected items in disabled data controls like list views.

   * - button.bg
     - background color for unfocused buttons

   * - button.fg
     - foreground color for unfocused buttons

   * - button.default.bg
     - background color for unfocused default buttons

   * - button.default.fg
     - foreground color for unfocused default buttons

   * - button.focused.bg
     - background color for focused buttons

   * - button.focused.fg
     - foreground color for focused buttons

   * - button.disabled.bg
     - background color for disabled buttons

   * - button.disabled.fg
     - foreground color for disabled buttons

   * - button.shortcut.bg
     - background color for button mnemonic (shortcut) letter

   * - button.shortcut.fg
     - foreground color for button mnemonic (shortcut) letter

   * - lineedit.bg
     - background color for unfocused input boxes

   * - lineedit.fg
     - foreground color for unfocused input boxes

   * - lineedit.focused.bg
     - background color for focused input boxes

   * - lineedit.focused.fg
     - foreground color for focused input boxes

   * - lineedit.disabled.bg
     - background color for disabled input boxes

   * - lineedit.disabled.fg
     - foreground color for disabled input boxes

Predefined palette classes:

.. list-table::
   :header-rows: 1
   :align: left

   * - name
     - description

   * - window
     - The widget with this class triggers window rules.

   * - dialog
     - The widget with this class and the ``window`` class triggers dialog window rules.

   * - cyan
     - The widget with this class and the ``window`` class triggers cyan window rules.
