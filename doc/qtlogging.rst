.. _qtlogging:

Qt Logging
==========

Qt logs warnings using the qt logging infrastructure.
This can even happen in production code.
As the default logging setup in Qt logs to stderr, it will interact poorly with Tui Widgets.

Qt offers logging output customization, so the logging output can be fixed. As this is commonly needed,
Tui Widgets offers helpers to redirect logging to a file or to a memory buffer.


Redirecting logging to a file
-----------------------------

.. rst-class:: tw-invisible
.. cpp:class:: Tui::ZSimpleFileLogger

   Actually a namespace.

.. cpp:function:: void Tui::ZSimpleFileLogger::install(const QString &path)

   Divert Qt Logging output to the file named in ``path``.


Redirecting logging to buffer
-----------------------------

.. rst-class:: tw-invisible
.. cpp:class:: Tui::ZSimpleStringLogger

   Actually a namespace.

.. cpp:function:: void Tui::ZSimpleStringLogger::install()

   Divert Qt logging output to a internal string buffer.

   See :cpp:func:`QString Tui::ZSimpleStringLogger::getMessages()` for retrieving the logged messages.

.. cpp:function:: QString Tui::ZSimpleStringLogger::getMessages()

   Get logged messages.

   This only returns messages logged after calling :cpp:func:`void Tui::ZSimpleStringLogger::install()`

   This does not clear the messages from the buffer.
   To clear the messages from the buffer, use :cpp:func:`void Tui::ZSimpleStringLogger::clearMessages()`

.. cpp:function:: void Tui::ZSimpleStringLogger::clearMessages()

   Clears all messages from the internal buffer.
