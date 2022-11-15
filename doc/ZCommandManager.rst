.. _ZCommandManager:

Commands
========

Tui Widgets uses commands to decouple user interface elements from application logic.

A command is identifed by a :cpp:class:`Tui::ZSymbol`.

The two main classes of the command system are :cpp:class:`Tui::ZCommandManager` and :cpp:class:`Tui::ZCommandNotifier`.

User interface elements get the command manager instance from a suitable :cpp:class:`Tui::ZWidget` and use it to
get information about the command (e.g. if it is currently enabled) and to trigger the command.

Application code creates a :cpp:class:`Tui::ZCommandNotifier` instance with a parent in the widget hierarchie to
interact with the command system. It can then set the enabled state of the command and bind to the activated signal.

ZCommandNotifier
----------------

.. cpp:class:: Tui::ZCommandNotifier : public QObject

   ZCommandNotifier is not copyable or movable. It does not define comparision operators.

   ZCommandNotifier creates a command (potentially local to a widget or window).
   The :cpp:func:`void activated()` signal must be connected to define what the command does.

   **Constructors**

   .. cpp:function:: explicit ZCommandNotifier(ZImplicitSymbol command, QObject *parent, Tui::ShortcutContext context = Tui::ApplicationShortcut)

      Creates a ZCommandNotifier instance for command ``command``.
      The command notifier will be active for commands triggered in the context ``context`` relative to ``parent``.

   **Functions**

   .. cpp:function:: ZSymbol command() const

      Returns the command this notifier is bound to.

   .. cpp:function:: Tui::ShortcutContext context() const

      Returns the context in from which this command can be triggered.

   .. cpp:function:: bool isEnabled() const
   .. cpp:function:: void setEnabled(bool s)

      If a command notifier is not enabled the command will not be activated and the corrosponding user interface
      elements might be shown in disabled state.

   .. cpp:function:: bool isContextSatisfied() const

      Returns true if the currently focused widget satisfies the context of this command notifier.

   **Signals**

   .. rst-class:: tw-signal
   .. cpp:function:: void activated()

      This signal is emitted when the command was activated by some user interface element.

   .. rst-class:: tw-signal
   .. cpp:function:: void enabledChanged(bool s)

      This signal is emitted when the compound value of the enabled status combined (by logical and) the value of
      :cpp:func:`~bool Tui::ZCommandNotifier::isContextSatisfied() const` changes.

ZCommandManager
---------------

.. cpp:class:: Tui::ZCommandManager : public QObject

   ZCommandManager is not copyable or movable. It does not define comparision operators.

   ZCommandManager is used to implement widgets that display and trigger commands.
   It is also used in the implementation of :cpp:class:`Tui::ZCommandNotifier`.

   .. cpp:function:: void registerCommandNotifier(ZCommandNotifier *notifier)

      Used internally in :cpp:class:`Tui::ZCommandNotifier`.

   .. cpp:function:: bool isCommandEnabled(ZSymbol command) const

      Queries the effective enabled state of a command.

      The state depends on the currently focused widget and on the state of the :cpp:class:`Tui::ZCommandNotifier`
      instances for the command ``command``.

   .. cpp:function:: void activateCommand(ZSymbol command)

      Activates the command ``command`` if possible.

      Which (if any) :cpp:class:`Tui::ZCommandNotifier` instance is triggered depends on the currently focused widget
      and on the state of the applicable instance.

   .. rst-class:: tw-signal
   .. cpp:function:: commandStateChanged(ZSymbol command)

      This signal is emitted when the effective state of command ``command`` changes.
