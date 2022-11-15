.. _ZMenuItem:

ZMenuItem
=========

Menu items are used by :cpp:class:`Tui::ZMenubar` and :cpp:class:`Tui::ZMenu`.

A common way to create a menu structure using c++ initializer lists is:

.. literalinclude:: examples/widgets/menubar.cpp
    :start-after: // snippet-initlist-items-start
    :end-before:  // snippet-initlist-items-end


.. cpp:class:: Tui::ZMenuItem

   This class is copyable and movable. It does not define comparison operators.

   A menu item for use with :cpp:class:`Tui::ZMenubar` and :cpp:class:`Tui::ZMenu`.

   **Constructors**

   .. cpp:function:: ZMenuItem()

      Constructs a menu item without text and command.

      If left in this state the item represents a separator.
      :cpp:class:`Tui::ZMenu` and sub-menus in :cpp:class:`Tui::ZMenubar` display separators as a horizontal line
      dividing the menu in multiple parts.

   .. cpp:function:: ZMenuItem(const QString &markup, const QString &fakeShortcut, Tui::ZImplicitSymbol command, QVector<Tui::ZMenuItem> subitems)

      Constructs a menu item and sets its data.

      This is a short form that has the same result as calling :cpp:func:`void setMarkup(const QString &markup)`,
      :cpp:func:`void setFakeShortcut(const QString &fakeShortcut)`,
      :cpp:func:`void setCommand(const Tui::ZImplicitSymbol &command)` and
      :cpp:func:`void setSubitems(const QVector<Tui::ZMenuItem> &subitems)` with the passed parameters.

   .. cpp:function:: ZMenuItem(const QString &markup, QObject *context, std::function<QVector<Tui::ZMenuItem>()> subitemsGenerator)

      Constructs a menu item and sets its data.

      This is a short form that has the same result as calling :cpp:func:`void setMarkup(const QString &markup)` and
      :cpp:func:`void setSubitemsGenerator(QObject *context, std::function<QVector<Tui::ZMenuItem>()> subitemsGenerator)`
      with the passed parameters.

   **Functions**

   .. cpp:function:: const QString &markup() const
   .. cpp:function:: void setMarkup(const QString &markup)

      The markup represents the visible text of a menu item.

   .. cpp:function:: const QString &fakeShortcut() const
   .. cpp:function:: void setFakeShortcut(const QString &fakeShortcut)

      This property represents the text that is displayed as shortcut in the menu item.

      The menu system does not by itself arrange for the shortcut to be bound to the command of the item.
      Use a :cpp:class:`Tui::ZShortcut` to actually make the shortcut work.

   .. cpp:function:: const Tui::ZImplicitSymbol &command() const
   .. cpp:function:: void setCommand(const Tui::ZImplicitSymbol &command)

      The :ref:`command <ZCommandManager>` this menu item binds to.

      The disabled state of the menu item will be the same as that of the command.
      If the menu item is activated this command will be activated.

   .. cpp:function:: const QVector<Tui::ZMenuItem> subitems() const
   .. cpp:function:: void setSubitems(const QVector<Tui::ZMenuItem> &subitems)
   .. cpp:function:: void setSubitemsGenerator(QObject *context, std::function<QVector<Tui::ZMenuItem>()> subitemsGenerator)
   .. cpp:function:: bool hasSubitems() const

      If a menu item has a non empty list of sub items it is a sub menu.

      Currently sub-menus are only supported in :cpp:class:`Tui::ZMenubar` and may not nest.

      For some menu items the available sub items are highly dynamic, thus using a sub item generator function allows
      the application to generate the list of sub items on demand.

      ``hasSubitems`` returns if sub items defined either as static list or generator are currently available.
