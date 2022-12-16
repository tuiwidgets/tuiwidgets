.. _ZShortcut:

ZShortcut
=========

The ZShortcut class allows to bind a signal to a keyboard shortcut.

.. rst-class:: tw-invisible
.. cpp:class:: Tui::ZKeySequence

   .. cpp:function:: static ZKeySequence forMnemonic(const QString &c);
   .. cpp:function:: static ZKeySequence forKey(int key, Tui::KeyboardModifiers modifiers = {});
   .. cpp:function:: static ZKeySequence forShortcut(const QString &c, Tui::KeyboardModifiers modifiers = Tui::ControlModifier);
   .. cpp:function:: static ZKeySequence forShortcutSequence(const QString &c, Tui::KeyboardModifiers modifiers, const QString &c2, Tui::KeyboardModifiers modifiers2);
   .. cpp:function:: static ZKeySequence forShortcutSequence(const QString &c, Tui::KeyboardModifiers modifiers, int key2, Tui::KeyboardModifiers modifiers2);


.. cpp:class:: Tui::ZShortcut : public QObject

   .. cpp:function:: explicit ZShortcut(const ZKeySequence &key, ZWidget *parent, Tui::ShortcutContext context = Tui::WindowShortcut)

      Creates a ZShortcut instance for sequence ``key``.
      The shortcut will be active for keyboard input in the context ``context`` relative to ``parent``.

      ``parent`` must not be :cpp:expr:`nullptr`.

      Useful values for ``key`` are:

      * ``ZKeySequence::forKey(key[, modifiers])``
        Use this for shortcuts with keys. ``modifiers`` defaults to :kbd:`Ctrl`.
      * ``ZKeySequence::forShortcut(c[, modifiers])``
        Use this for shortcuts with printable characters. ``modifiers`` defaults to :kbd:`Ctrl`.
      * ``ZKeySequence::forShortcutSequence(c, modifiers, c2, modifiers2)``
        Use this for multi step shortcuts with printable characters.
      * ``ZKeySequence::forShortcutSequence(c, modifiers, key2, modifiers2)``
        Use this for multi step shortcuts beginning with a printable character followed by a key.

   .. cpp:function:: bool isEnabled() const
   .. cpp:function:: void setEnabled(bool enable)

      If a shortcut is not enabled it will not react to keyboard input.

   .. cpp:function:: void setEnabledDelegate(Tui::Private::ZMoFunc<bool()>&& delegate)

      In addition to the enabled property, this function allows adding a callback to check if the shortcut should
      react to keyboard input.
      Use this when the enabled condition depends on state that is hard to sync to calls to
      :cpp:func:`void Tui::ZShortcut::setEnabled(bool enable)`.

   .. cpp:function:: bool matches(ZWidget *focusWidget, const ZKeyEvent *event) const

      Returns true iff the the key event ``event`` matches the shortcut or its first key in multi key shortcuts
      and the context for the shortcut is satisfied.

   .. rst-class:: tw-signal
   .. cpp:function:: void activated()

      This signal is emitted when the shortcut was activated.

ZPendingKeySequenceCallbacks
----------------------------

.. cpp:class:: Tui::ZPendingKeySequenceCallbacks

   The ZPendingKeySequenceCallbacks allows monitoring in-progress multi-step shortcuts.

   Monitoring is activated using
   :cpp:func:`void Tui::ZTerminal::registerPendingKeySequenceCallbacks(const Tui::ZPendingKeySequenceCallbacks &callbacks)`.

   .. cpp:function:: void setPendingSequenceStarted(std::function<void()> callback)

      Set a callback to be called when a multi-step shortcut is started.

   .. cpp:function:: void setPendingSequenceFinished(std::function<void(bool matched)> callback)

      Set a callback to be called when a multi-step shortcut is finished.

      The ``matched`` parameter of the callback will be ``true`` if the input did match a active shortcut.

   .. cpp:function:: void setPendingSequenceUpdated(std::function<void()> callback)

      Set a callback to be called when a further keystroke for a multi-step shortcut is input.
