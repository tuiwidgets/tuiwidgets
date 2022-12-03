.. _ZTest:

.. rst-class:: tw-invisible
.. cpp:class:: Tui::ZTest

   Actually a namespace.

ZTest
-----

Support code for testing widgets.

.. cpp:function:: void Tui::ZTest::sendText(Tui::ZTerminal *terminal, const QString &text, Tui::KeyboardModifiers modifiers)

   Send input text to the focused widget of ``terminal`` as if input via keyboard.

.. cpp:function:: void Tui::ZTest::sendKey(Tui::ZTerminal *terminal, Tui::Key key, Tui::KeyboardModifiers modifiers)

   Send a key to the focused widget of ``terminal`` as if input via keyboard.

.. cpp:function:: void Tui::ZTest::sendKeyToWidget(Tui::ZWidget *w, Tui::Key key, Tui::KeyboardModifiers modifiers)

   Send a key directly to the widget ``w`` as if input via keyboard, bypasses focus handling.

.. cpp:function:: void Tui::ZTest::sendPaste(Tui::ZTerminal *terminal, const QString &text)

   Send pasted text to the focused widget of ``terminal`` as if pasted into the terminal.

.. cpp:function:: Tui::ZImage Tui::ZTest::waitForNextRenderAndGetContents(Tui::ZTerminal *terminal)

   Waits (busy looping on QCoreApplication::processEvents) until the next render cycle finished and returns the
   terminal contents.

.. cpp:function:: void Tui::ZTest::withLayoutRequestTracking(Tui::ZTerminal *terminal, std::function<void (QSet<Tui::ZWidget*>*)> closure)

   Calls closure ``closure`` with a :cpp:class:`QSet` that gets all widgets added which mark themselves as pending
   for relayout.
