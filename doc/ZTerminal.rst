.. _ZTerminal:

ZTerminal
=========

ZTerminal represents a connection to a terminal and the terminal scoped state for Tui Widgets.

The terminal connection can be to the terminal the application was run from, an internal offscreen terminal
or a terminal specified by the application.

.. _term_startup:

Startup
-------

As soon as the message loop runs after creating an instance of ``ZTerminal``, terminal auto detection is started.
Also the terminal is setup for operations with Tui Widgets.
If a :ref:`main widget <term_mainwidget>` is set when the terminal detection finishes, a terminal change event
is delivered to all widgets in the main widgets' widget tree.

If the terminal detection yields that the terminal is likely not compatible with Tui Widgets the program is terminated
by a call to :cpp:func:`QCoreApplication::quit()` unless
:cpp:enumerator:`~Tui::ZTerminal::Option::ForceIncompatibleTerminals`
or :cpp:func:`~Tui::ZTerminal::incompatibleTerminalDetected()` was used.

If terminal auto detection takes too long a message is printed to the terminal.
The application can override this message using :cpp:func:`~Tui::ZTerminal::setAutoDetectTimeoutMessage`
or disable it using :cpp:enumerator:`~Tui::ZTerminal::Option::DisableAutoDetectTimeoutMessage`.

.. _term_options:

Options
-------

Most constructors of ``ZTerminal`` take an ``options`` parameter that allows customizing the terminal's behavior.

Some of these options only apply to terminals connected directly through a kernel device.

.. rst-class:: tw-invisible
.. cpp:type:: Tui::ZTerminal::Options = QFlags<Tui::ZTerminal::Option>

.. cpp:enum:: Tui::ZTerminal::Option

   .. cpp:enumerator:: AllowInterrupt

      Don't disable kernel interpretation of interrupt character for SIGINT (often :kbd:`Ctrl+C`).

      Normally interpretation of the interrupt character is disabled so it is usable as normal key combination in
      the application.
      When this flag is included in a terminal's options the handling will be left at the setting that was active
      when starting the terminal connection and likely a SIGINT is delivered when pressing that key.

      By default SIGINT will trigger an unclean termination of the application process.

      This option is only available for terminals connected directly through a kernel device that is acting as
      the controlling terminal of the process

   .. cpp:enumerator:: AllowSuspend

      Don't disable kernel interpretation of suspend character for SIGTSTP (often :kbd:`Ctrl+Z`).

      Normally interpretation of the suspend character is disabled so it is usable as normal key combination in
      the application.
      When this flag is included in a terminal's options the handling will be left at the setting that was active
      when starting the terminal connection and likely a SIGTSTP is delivered when pressing that key.

      By default SIGTSTP will trigger the application suspending into the background.
      The terminal will be restored to the state before the terminal was connected and the application stopped.

      When the application is resumed back into the foreground (e.g. by using ``fg`` in the shell) the terminal
      will be reconfigured for usage with ``ZTerminal`` and the application resumes when passing through the event loop
      the next time.

      This option is only available for terminals connected directly through a kernel device that is acting as
      the controlling terminal of the process

   .. cpp:enumerator:: AllowQuit

      Don't disable kernel interpretation of quit character for SIGQUIT (often :kbd:`Ctrl+\\`).

      Normally interpretation of the quit character is disabled so it is usable as normal key combination in
      the application.
      When this flag is included in a terminal's options the handling will be left at the setting that was active
      when starting the terminal connection and likely a SIGQUIT is delivered when pressing that key.

      By default SIGQUIT will trigger a unclean termination of the application process.

      This option is only available for terminals connected directly through a kernel device that is acting as
      the controlling terminal of the process

   .. cpp:enumerator:: DisableAutoResize

      By default ``ZTerminal`` reacts to the signal SIGWINCH from the kernel by adjusting the internal representation of
      the terminal to the new size.
      If this option is included in the terminal's options the internal state will not be adjusted.

      This option is only available for terminals connected directly through a kernel device and acting as
      the controlling terminal of the process.

   .. cpp:enumerator:: DisableAlternativeScreen

      By default ``ZTerminal`` switches the terminal to the alternate screen if supported.
      This means that the application will not overwrite the contents of the terminal and the terminal view will be
      restored after the instance is destroyed.

      If this option is included in the terminal's options then the terminal will not switch to the alternate screen
      and the last output of the application will be preserved in the terminal.

   .. cpp:enumerator:: DisableAutoDetectTimeoutMessage

      By default ``ZTerminal`` displays a message if the terminal auto detection takes longer than 10 seconds.
      See :cpp:func:`~void Tui::ZTerminal::setAutoDetectTimeoutMessage(const QString &message)` for details about setting that
      message.

      If this option is included in the terminal's options then that message will not be generated.

   .. cpp:enumerator:: ForceIncompatibleTerminals

      If the terminal autodetection result is that the terminal is not compatible with ``ZTerminal`` then by default, it
      will not proceed to initialize the terminal any further.
      Further by default the application will be terminated with a message.
      The termination can be prevented by connecting to the signal :cpp:func:`~Tui::ZTerminal::incompatibleTerminalDetected()`.

      If this option is included in the terminal's options then this check will be skipped and ``ZTerminal`` will try to
      use the terminal anyway.

   .. cpp:enumerator:: DisableTaggedPaste

      By default ``ZTerminal`` will setup supported terminals to generate :cpp:class:`Tui::ZPasteEvent` events when the
      user pastes text from the clipboard instead of generating key events.

      If this option is included in the terminal's options then the terminal is left for paste events at the setting
      it was when the connection was started. Usually this means that pasted text will be received as key events.

   .. cpp:enumerator:: DebugDisableBufferedIo

      By default ``ZTerminal`` uses internal buffering for terminal output and flushes the output buffer after doing a
      refresh. This option allows disabling this buffering when debugging problems in the low level output handling.

   .. cpp:enumerator:: ConservativeTrueColorOutput

      By default ``ZTerminal`` will send RGB terminal colors used by the application to the terminal as RGB colors when
      the terminal is likely to support those colors.

      If this option is included in the terminal's options then RGB colors are converted to indexed colors for some
      terminals where the auto detection did not yield a certain result for RGB color support.

If none of the :cpp:enumerator:`~Tui::ZTerminal::Option::AllowInterrupt`,
:cpp:enumerator:`~Tui::ZTerminal::Option::AllowSuspend` and :cpp:enumerator:`~Tui::ZTerminal::Option::AllowQuit`
options are active the terminal might be switched into an advanced keyboard mode that supports additional key
combinations or keys.

Terminal connection options
---------------------------

Default terminal
................

The default constructor and the constructor that only takes an ``options`` parameter connect to the terminal the
application was run from. It uses the first file descriptor that is a terminal searching stdin, stdout, stderr
and the process' controlling terminal.
If no such terminal is found the application is terminated by calling :cpp:func:`QCoreApplication::quit()`.
The application can avoid termination by using :cpp:func:`~Tui::ZTerminal::isDefaultTerminalAvailable()` to avoid
constructing a ``ZTerminal`` instance when no terminal is available.

Terminal from file descriptor
.............................

Using the constructor that takes :cpp:struct:`~Tui::ZTerminal::FileDescriptor` as parameter the application can
connect the ``ZTerminal`` instance to a terminal represented by the given file descriptor.
The application has to ensure that the file descriptor actually is a terminal (:manpage:`isatty(3)`) and is both
readable and writable.

Offscreen terminal
..................

Using the constructor that takes :cpp:struct:`~Tui::ZTerminal::OffScreen` as parameter the application can create
a ``ZTerminal`` instance that is not connected to an actual terminal, but instead does not display anything.

This can be useful for example for testing or as null instance when an application that should be runnable without a
terminal does not have access to an actual terminal.

Custom terminal connection
..........................

Using the constructor that takes :cpp:struct:`~Tui::ZTerminal::TerminalConnection` as parameter the application
can create a ``ZTerminal`` instance that uses methods of :cpp:struct:`~Tui::ZTerminal::TerminalConnection` and
an instance of an application supplied class derived from
:cpp:struct:`~Tui::ZTerminal::TerminalConnectionDelegate` for all terminal communication.

This can be useful to connect to terminals that are not directly reachable via a kernel terminal device, such as
internally implemented ssh connections or other custom transports.

Shutdown
--------

The destructor of the terminal instance will restore the terminal to its configuration before ``ZTerminal`` started using
the terminal. Additionally ``ZTerminal`` installs signal handlers to restore the terminal if the application crashes.

If on the other hand the application is using the default terminal connection and the terminal gets disconnected, the
application is terminated by calling :cpp:func:`QCoreApplication::quit()`, unless the application connects to the
:cpp:func:`~Tui::ZTerminal::terminalConnectionLost()` signal.

.. _term_mainwidget:

Main Widget
-----------

When used with widgets, a terminal instance has one main widget. It can be set by using
:cpp:func:`~void Tui::ZTerminal::setMainWidget(ZWidget *w)`.
This main widget is the root of the widget tree that will be connected to the terminal.

When the main widget is initially connected to the terminal (after terminal auto detection and setup) it and all
widgets in its widget tree will receive a terminal changed event.
After receiving that event the widgets can be used through the terminal connection.

The keyboard focus is a terminal scoped property.
But to ease preparing the widget tree before actually connecting it to the terminal when the main widget is set,
the newly attached widget tree is scanned for the widget with the most recent call to
:cpp:func:`~void Tui::ZWidget::setFocus(Tui::FocusReason reason = Tui::OtherFocusReason)` which is both enabled and
visible to take focus.

If a widget is set as main widget of the terminal, its size will be set to the size of the terminal
(or its minimum size if that is larger) when the terminal size changes.
The main widget will always be placed in the top left corner of the terminal and the position part of the widget's
geometry is ignored.


.. _term_viewport:

Viewport
--------

If the terminal size is smaller than the minimum size indicated by the main widget, ``ZTerminal`` will switch into
viewport mode.
In viewport mode the application is rendered to a buffer that is larger than the actual terminal size and the terminal
will view a selectable part of that buffer.

The minimum size used for this is determined by taking the larger value (in each dimension) of the sizes returned by
:cpp:func:`QSize Tui::ZWidget::minimumSize() const` and :cpp:func:`virtual QSize Tui::ZWidget::minimumSizeHint() const`.
Some root widgets (like :cpp:class:`Tui::ZRoot`) allow dynamically expanding the minimum size to be large enough to
display specially marked windows.

If the viewport mode is active, ``ZTerminal`` will display the text "F6 Scroll" in the lower left corner of the terminal.
Pressing :kbd:`F6` will switch to viewport scroll mode with the text "←↑→↓ ESC" in the lower left corner.
In viewport scroll mode, the part of the application that is visible in the terminal can be adjusted using the arrow
keys.
Pressing :kbd:`F6` again will send an :kbd:`F6` event to the application and pressing :kbd:`ESC` will return to the
view port mode.
Any other key will be ignored.


Terminal information
--------------------

Certain information about the terminal connected to a ``ZTerminal`` instance is available with calling functions on it.

The size of the connected terminal is available from :cpp:func:`~int Tui::ZTerminal::width() const`
and :cpp:func:`~int Tui::ZTerminal::height() const`.

When working with text it is often required to calculate the space used by a string when displayed.
Calculations of the used space use a :cpp:class:`Tui::ZTextMetrics` instance. Such an instance is either available
from a :cpp:class:`Tui::ZPainter` instance or from the terminal instance using
:cpp:func:`~ZTextMetrics Tui::ZTerminal::textMetrics() const`.

In addition sometimes it is useful to query if the connected terminal has certain capabilities using
:cpp:func:`~bool Tui::ZTerminal::hasCapability(ZSymbol cap) const`.

Configuration
-------------

Some configuration of the terminal can be changed by methods on the terminal instance.

Terminals usually have a title that is displayed somewhere in the terminal user interface.
The functions :cpp:func:`~void Tui::ZTerminal::setTitle(const QString &title)`
and :cpp:func:`~void Tui::ZTerminal::setIconTitle(const QString &title)`
can be used to set this text when supported.

.. _term_capabilites:

Capabilities
------------

Terminals can have different capabilities.
Some capabilities can be queried after auto detection is completed.

Currently the only queryable capability is ``extendedCharset``.
This capability is active for all terminals that are not known to have a severely restricted character set.
An example is the linux system terminal that is restricted to 256 or at most 512 characters in the active font.

.. _term_introspection:


Behavior
--------

If widget tree does not handle :kbd:`Ctrl+L` the terminal will handle it by calling :cpp:func:`~void Tui::ZTerminal::forceRepaint()`
as most TUI applications allow using :kbd:`Ctrl+L` to force refresh the terminal if another application or some bug has left
the terminal in a garbled state.

On terminal resize (when :cpp:enumerator:`Tui::ZTerminal::Option::DisableAutoResize` is not in effect) the terminal
calls :cpp:func:`~void Tui::ZTerminal::forceRepaint()` to make sure the resize operation does not result in garbled output.

If the :ref:`viewport mode <term_viewport>` is active, :kbd:`F6` followed by arrow keys can be used to move the viewport.
See :ref:`term_viewport` for behavior details of this mode.

..
  TODO signals?
  TODO more?

.. _term_instrospection:

Introspection
-------------

Sometimes it is desired to read back the actually produced output from the ``ZTerminal`` instance.
For example for testing of widget or infrastructure code.

For this purpose the functions :cpp:func:`~ZImage Tui::ZTerminal::grabCurrentImage() const`,
:cpp:func:`~std::tuple<int, int, int> Tui::ZTerminal::grabCursorColor() const`,
:cpp:func:`~QPoint Tui::ZTerminal::grabCursorPosition() const`,
:cpp:func:`~CursorStyle Tui::ZTerminal::grabCursorStyle() const`
and :cpp:func:`~bool Tui::ZTerminal::grabCursorVisibility() const`
can be used.

It is possible to get notified after each render cycle by connecting to the
:cpp:func:`~Tui::ZTerminal::afterRendering()` signal.

Observing application state
---------------------------

Through the ``ZTerminal`` instance it is possible to observe events in the running application.

As keyboard focus is a terminal scoped state, the signal :cpp:func:`~Tui::ZTerminal::focusChanged()` can be used
to observe the focus moving from one widget to another widget.

The state of resolution if multi key shortcuts can be observed via callbacks registered using
:cpp:func:`~void Tui::ZTerminal::registerPendingKeySequenceCallbacks(const Tui::ZPendingKeySequenceCallbacks &callbacks)`.

And the progression through rendering cycles of the application can be monitored using the signals
:cpp:func:`~Tui::ZTerminal::afterRendering()` and :cpp:func:`~Tui::ZTerminal::beforeRendering()`.

.. _term_standalone:

Standalone usage
----------------

It is also possible (but unusual) to use a ``ZTerminal`` instance without widgets.
In this case :cpp:func:`~ZPainter Tui::ZTerminal::painter()` can be used to directly paint on the buffer of the
``ZTerminal`` instance and flush this buffer to the connected terminal by using
:cpp:func:`~void Tui::ZTerminal::updateOutput()` or
:cpp:func:`~void Tui::ZTerminal::updateOutputForceFullRepaint()`.

When not using widgets the cursor attributes, that are usually configured by functions in ZWidget, can be set
using
:cpp:func:`~void Tui::ZTerminal::setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB)`,
:cpp:func:`~void Tui::ZTerminal::setCursorPosition(QPoint cursorPosition)` and
:cpp:func:`~void Tui::ZTerminal::setCursorStyle(CursorStyle style)`.

.. _term_pause:

Pause / Unpause operation
-------------------------

It is possible to pause usage of the terminal by a ``ZTerminal`` instance by calling :cpp:func:`~void Tui::ZTerminal::pauseOperation()`.
This restores the terminal mode to the state before ``ZTerminal`` started using the terminal and enables using the terminal
for running other applications that access the terminal.

If the application wants to return to using the terminal with ``ZTerminal`` it should call
:cpp:func:`~void Tui::ZTerminal::unpauseOperation()`.


Layout infrastructure
---------------------

The ``ZTerminal`` instance also acts as a central point for coordination of layout updates.
Usually applications use the layout infrastructure through interfaces in :cpp:class:`Tui::ZWidget` and
:cpp:class:`Tui::ZLayout` (and for testing in :cpp:class:`Tui::ZTest`).

By default, layout work is deferred using the event loop to reduce costly relayout cycles.
:cpp:class:`Tui::ZWidget` and :cpp:class:`Tui::ZLayout` register widgets to be relayouted in the next cycle by calling
:cpp:func:`~void Tui::ZTerminal::maybeRequestLayout(ZWidget *w)`
or :cpp:func:`~void Tui::ZTerminal::requestLayout(ZWidget *w)`.

The function :cpp:func:`~bool Tui::ZTerminal::isLayoutPending() const` can be used to observe if a layout cycle
is pending.

While normally it is sufficient to let ``ZTerminal`` schedule the next layout cycle, an application can force running a
pending layout cycle immediately by calling :cpp:func:`~void Tui::ZTerminal::doLayout()`.
When calling this function care should be taken to call it only from code that cannot be itself be triggered by
a layout cycle to avoid recursion.

Components that need to cache information scoped to a layout cycle can use
:cpp:func:`~int Tui::ZTerminal::currentLayoutGeneration()` to get a value that changes for each layout cycle to
manage cache invalidation.

..
  TODO document ZRawSequenceEvent usage.
  TODO document ZTerminalNativeEvent usage.
  TODO maybe document event sending stuff?

ZTerminal
---------

.. cpp:class:: Tui::ZTerminal : public QObject

   This class is neither copyable nor movable. It does not define comparison operators.

   **Constructors**

   | :cpp:func:`ZTerminal(QObject *parent = nullptr) <ZTerminal::ZTerminal()>`
   | :cpp:func:`ZTerminal(Options options, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(Tui::ZTerminal::Options, QObject *parent = nullptr)>`
   | :cpp:func:`ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr)>`
   | :cpp:func:`ZTerminal(const OffScreen& offscreen, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(const OffScreen& offscreen, QObject *parent = nullptr)>`
   | :cpp:func:`ZTerminal(TerminalConnection *connection, Options options, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(TerminalConnection *connection, Options options, QObject *parent = nullptr)>`

   **Functions**

   | :cpp:func:`QString ZTerminal::autoDetectTimeoutMessage() const`
   | :cpp:func:`int currentLayoutGeneration()`
   | :cpp:func:`void dispatchKeyboardEvent(ZKeyEvent &translated)`
   | :cpp:func:`void dispatchPasteEvent(ZPasteEvent &translated)`
   | :cpp:func:`void doLayout()`
   | :cpp:func:`ZWidget *focusWidget() const`
   | :cpp:func:`void forceRepaint()`
   | :cpp:func:`ZImage grabCurrentImage() const`
   | :cpp:func:`std::tuple<int, int, int> grabCursorColor() const`
   | :cpp:func:`QPoint grabCursorPosition() const`
   | :cpp:func:`CursorStyle grabCursorStyle() const`
   | :cpp:func:`bool grabCursorVisibility() const`
   | :cpp:func:`bool hasCapability(ZSymbol cap) const`
   | :cpp:func:`int height() const`
   | :cpp:func:`QString iconTitle() const`
   | :cpp:func:`ZWidget *keyboardGrabber() const`
   | :cpp:func:`bool isLayoutPending() const`
   | :cpp:func:`ZWidget *mainWidget() const`
   | :cpp:func:`void maybeRequestLayout(ZWidget *w)`
   | :cpp:func:`ZPainter painter()`
   | :cpp:func:`bool isPaused() const`
   | :cpp:func:`void pauseOperation()`
   | :cpp:func:`void registerPendingKeySequenceCallbacks(const Tui::ZPendingKeySequenceCallbacks &callbacks)`
   | :cpp:func:`void requestLayout(ZWidget *w)`
   | :cpp:func:`void resize(int width, int height)`
   | :cpp:func:`QString terminalDetectionResultText() const`
   | :cpp:func:`QString terminalSelfReportedNameAndVersion() const`
   | :cpp:func:`void setAutoDetectTimeoutMessage(const QString &message)`
   | :cpp:func:`void setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB)`
   | :cpp:func:`void setCursorPosition(QPoint cursorPosition)`
   | :cpp:func:`void setCursorStyle(CursorStyle style)`
   | :cpp:func:`void setIconTitle(const QString &title)`
   | :cpp:func:`void setMainWidget(ZWidget *w)`
   | :cpp:func:`void setTitle(const QString &title)`
   | :cpp:func:`ZTextMetrics textMetrics() const`
   | :cpp:func:`QString title() const`
   | :cpp:func:`void unpauseOperation()`
   | :cpp:func:`void update()`
   | :cpp:func:`void updateOutput()`
   | :cpp:func:`void updateOutputForceFullRepaint()`
   | :cpp:func:`int width() const`

   **Protected Functions**

   | :cpp:func:`std::unique_ptr<Tui::ZKeyEvent> translateKeyEvent(const Tui::ZTerminalNativeEvent &nativeEvent)`

   **Static Functions**

   | :cpp:func:`bool isDefaultTerminalAvailable()`

   **Signals**

   | :cpp:func:`afterRendering()`
   | :cpp:func:`beforeRendering()`
   | :cpp:func:`focusChanged()`
   | :cpp:func:`incompatibleTerminalDetected()`
   | :cpp:func:`terminalConnectionLost()`



Members
-------

.. cpp:namespace:: Tui::ZTerminal

.. cpp:function:: ZTerminal(QObject *parent = nullptr)

   Construct an instance connected to the terminal the application was started from.

   This forwards to :cpp:func:`ZTerminal(Options options, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(Tui::ZTerminal::Options, QObject *parent = nullptr)>`
   with options set to empty.

.. cpp:function:: ZTerminal(Tui::ZTerminal::Options options, QObject *parent = nullptr)

   Construct an instance connected to the terminal the application was started from.

   The instance uses ``parent`` as parent. The parent is only used for Qt style automatic deletion and may be :cpp:expr:`nullptr`.

   See :ref:`term_startup` for details on terminal instance startup and :ref:`term_options` for details on the supported
   options.

.. cpp:function:: ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr)

   Construct an instance connected to the terminal specifed in ``fd``. The file descriptor must be opened for reading
   and writing and must be a kernel terminal device (see :manpage:`isatty(3)`, e.g. tty or pty).

   The class does not take ownership of the file descriptor contained in ``fd``.
   The application has to ensure that the file descriptor contained in ``fd`` is kept open for at least as long as
   the instance using it exists.

   The instance uses ``parent`` as parent. The parent is only used for Qt style automatic deletion and may be :cpp:expr:`nullptr`.

   See :ref:`term_startup` for details on terminal instance startup and :ref:`term_options` for details on the supported
   options.

.. cpp:function:: ZTerminal(const OffScreen& offscreen, QObject *parent = nullptr)

   Construct an instance not connected to a external terminal at all.

   Terminal auto detection is skipped. Terminal capabilities will be taken from ``offscreen``.

   The instance uses ``parent`` as parent. The parent is only used for Qt style automatic deletion and may be :cpp:expr:`nullptr`.

.. cpp:function:: ZTerminal(TerminalConnection *connection, Options options, QObject *parent = nullptr)

   Construct an instance that communicates with the terminal using virtual functions in the delegate passed in using
   ``connection`` and the :cpp:func:`void Tui::ZTerminal::TerminalConnection::terminalInput(const char *data, int length)`
   function.

   Parts of the configuration are taken from the initial settings in ``connection`` and from terminal auto detection
   running over the connection.

   ZTerminal does not take ownership of the ``connection`` instance.
   The application has to ensure that the ``connection`` instance is kept valid for at least as long as the instance
   using it exists.

   The instance uses ``parent`` as parent. The parent is only used for Qt style automatic deletion and may be :cpp:expr:`nullptr`.

   See :ref:`term_startup` for details on terminal instance startup and :ref:`term_options` for details on the supported
   options.

.. rst-class:: tw-signal
.. cpp:function:: void afterRendering()

   This signal is emitted after each repaint cycle but before sending the rendered output to the terminal.

   This signal is suitable to :ref:`introspect <term_introspection>` the rendering result or do last minute
   modifiations.

.. cpp:function:: void setAutoDetectTimeoutMessage(const QString &message)
.. cpp:function:: QString autoDetectTimeoutMessage() const

   The auto detect timeout message is send to the terminal when autodetection takes a long time.

   Auto detection is not automatically aborted after this message is send. Instead the user needs to press a key
   to terminate auto detection.

.. rst-class:: tw-signal
.. cpp:function:: void beforeRendering()

   This signal is emitted just before rendering and layout.

   This signal can be used to update display state right before a render cycle is done, when there is not suitable
   signal or event to trigger updateing.

.. rst-class:: tw-static
.. cpp:function:: static bool isDefaultTerminalAvailable()

   Returns ``true`` if connecting to the default terminal is possible.
   Returns ``false`` if the application does not have a terminal connected as standard I/O and does not have a
   controlling terminal.

.. cpp:function:: bool hasCapability(ZSymbol cap) const

   Queries if the capability ``cap`` is set.

   See :ref:`term_capabilites` for details and possible values for ``cap``.

.. cpp:function:: QString terminalDetectionResultText() const

   Returns a string describing the result of terminal auto detection.

   Don't parse the result of this function, it is only intended to be displayed as diagnostic information.
   An easy way to display this information is by using :ref:`ZTerminalDiagnosticsDialog`.

.. cpp:function:: QString terminalSelfReportedNameAndVersion() const

   Returns the the name and version the terminal reports about itself.

.. cpp:function:: int currentLayoutGeneration()

   The returned value can be used for cache invalidation when caching layout state during a layout cycle.

   ..
      TODO more details

.. cpp:function:: void setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB)

   Prefer using :cpp:func:`void Tui::ZWidget::setCursorColor(int r, int b, int g)`.

   Set the cursor color if the terminal supports cursor color.

   |standalone-or-ar|

.. cpp:function:: void setCursorPosition(QPoint cursorPosition)

   Prefer using :cpp:func:`void Tui::ZWidget::showCursor(QPoint position)`
   or :cpp:func:`void Tui::ZPainter::setCursor(int x, int y)`.

   Set the cursor position.

   The special position :cpp:expr:`QPoint{-1, -1}` hides the cursor.

   |standalone-or-ar|

.. cpp:function:: void setCursorStyle(CursorStyle style)

   Prefer using :cpp:func:`void Tui::ZWidget::setCursorStyle(CursorStyle style)`.

   Set the shape/style of the cursor.
   See :cpp:enum:`Tui::CursorStyle` for possible values.

   |standalone-or-ar|

.. cpp:function:: void dispatchKeyboardEvent(ZKeyEvent &translated)

   For testing prefer using
   :cpp:func:`Tui::ZTest::sendText <void Tui::ZTest::sendText(Tui::ZTerminal *terminal, const QString &text, Tui::KeyboardModifiers modifiers)>`
   or :cpp:func:`Tui::ZTest::sendKey <void Tui::ZTest::sendKey(Tui::ZTerminal *terminal, Tui::Key key, Tui::KeyboardModifiers modifiers)>`
   instead.

   The function allows injecting a artifical :cpp:class:`Tui::ZKeyEvent` into the application as if it had been send
   by the terminal.

.. cpp:function:: void dispatchPasteEvent(ZPasteEvent &translated)

   For testing prefer using
   :cpp:func:`Tui::ZTest::sendPaste <void Tui::ZTest::sendPaste(Tui::ZTerminal *terminal, const QString &text)>`
   instead.

   The function allows injecting a artifical :cpp:class:`Tui::ZPasteEvent` into the application as if it had been send
   by the terminal.

.. cpp:function:: void doLayout()

   Force immediate execution of a pending layout cycle.
   If no layout cycle is pending this function has no effect.

.. rst-class:: tw-signal
.. cpp:function:: void focusChanged()

   This signal is emitted whenever the keyboard focus changes or is lost.
   The newly focused widget can be retrieved by calling :cpp:func:`~ZWidget *focusWidget() const`

.. cpp:function:: ZWidget *focusWidget() const

   Returns the widget which currently has the keyboard focus or :cpp:expr:`nullptr` if no widget has focus.

.. cpp:function:: void forceRepaint()

   Force a immediate repaint cycle and do a full (non incremental) update of the terminal with rendered result.

.. cpp:function:: ZImage grabCurrentImage() const

   This :ref:`introspection <term_instrospection>` function returns the currently buffered ZTerminal side terminal
   contents.

   The application should not call this function while paint events are processed because the returned image might change
   in future versions.

   If the result of the rendering cycle is needed this function can be called from code connected to the
   :cpp:func:`~Tui::ZTerminal::afterRendering()` signal.

.. cpp:function:: std::tuple<int, int, int> grabCursorColor() const

   This :ref:`introspection <term_instrospection>` function returns the current cursor color in the order
   ``{ red, green, blue}``.

.. cpp:function:: QPoint grabCursorPosition() const

   This :ref:`introspection <term_instrospection>` function returns the current cursor position.

.. cpp:function:: CursorStyle grabCursorStyle() const

   This :ref:`introspection <term_instrospection>` function returns the current cursor style.

.. cpp:function:: bool grabCursorVisibility() const

   This :ref:`introspection <term_instrospection>` function returns the current visibility of the cursor.

.. cpp:function:: int height() const

   This function return the current height of the terminal as seen by ``ZTerminal``.

   If :cpp:enumerator:`Tui::ZTerminal::Option::DisableAutoResize` is not in effect the height should match the
   actual terminal height unless the application has overridden the height using
   :cpp:func:`~void Tui::ZTerminal::resize(int width, int height)`. If the terminal connection does not propagate
   size changes the height might be outdated.

   If :cpp:enumerator:`Tui::ZTerminal::Option::DisableAutoResize` is in effect the height returned is either the
   initial size of the terminal or the height of the last call to
   :cpp:func:`~void Tui::ZTerminal::resize(int width, int height)`.

   ..
     TODO: describe view port stuff. It's not influenced by that. Also in width.

.. cpp:function:: void setTitle(const QString &title)
.. cpp:function:: QString title() const
.. cpp:function:: void setIconTitle(const QString &title)
.. cpp:function:: QString iconTitle() const

   Most terminals display a title in some place.
   These functions allow managing the title.
   The title will only be used on terminals that are capable to restore the previous title on clean up.

   Some terminals actually manage 2 titles.
   The icon title is usually shown for iconified form of the terminal window and in window choosers / taskbars.
   The normal title is usually shown for the currently active terminal in the window title.

.. rst-class:: tw-signal
.. cpp:function:: void incompatibleTerminalDetected()

   This signal is emitted when a incompatible terminal is detected.

   If :cpp:enumerator:`Tui::ZTerminal::Option::ForceIncompatibleTerminals` is not in effect and this signal is not
   connected ``ZTerminal`` will output an error message and the application will be terminated
   using :cpp:func:`QCoreApplication::quit()`.
   If the signal is connected the terminal will still not be initialized but the application is free to handle the
   problem in another way like displaying a custom error message or proceeding without using Tui Widgets.

   If :cpp:enumerator:`Tui::ZTerminal::Option::ForceIncompatibleTerminals` is in effect even incomaptible terminals are
   initialized on a best effort basis and the signal will be emitted after setup of the terminal completes.

.. cpp:function:: ZWidget *keyboardGrabber() const

   Returns the current widget that has grabbed the keyboard if any.

   Widgets can grab the keyboard using :cpp:func:`void Tui::ZWidget::grabKeyboard()`.

.. cpp:function:: void setMainWidget(ZWidget *w)
.. cpp:function:: ZWidget *mainWidget() const

   The main window is the root of the widget tree connected to this ``ZTerminal`` instance.

   For details see :ref:`term_mainwidget`.

   If a new main widget is set, various widget related state in the ``ZTerminal`` instance is cleared (e.g. focus,
   keyboard grab, pending layouting).

   The geometry of the main widget overwritten by the terminal size in most cases.
   The size of the main widget can be influenced by setting the widget's minimum size.
   The position of the widget's geometry is ignored and the widget is always placed in the top-left corner of the
   terminal.

   The new main widget must not have a parent widget, otherwise the call will do nothing.

.. cpp:function:: ZPainter painter()

   Returns a painter that allows modifying the the ``ZTerminal`` side terminal buffer directly.

   |standalone-or-ar|

.. cpp:function:: bool isPaused() const
.. cpp:function:: void pauseOperation()
.. cpp:function:: void unpauseOperation()

   These functions manage pausing terminal operation.
   It is safe to call these functions repeatedly, but calls are not counted so for example one call to ``unpauseOperation``
   will undo the effect of multiple calls to ``pauseOperation``.

   See :ref:`term_pause` for details.

.. cpp:function:: void registerPendingKeySequenceCallbacks(const Tui::ZPendingKeySequenceCallbacks &callbacks)

   The ZPendingKeySequenceCallbacks allows monitoring in progress multi step shortcuts.

   See :cpp:class:`Tui::ZPendingKeySequenceCallbacks` for details.

.. cpp:function:: bool isLayoutPending() const
.. cpp:function:: void requestLayout(ZWidget *w)
.. cpp:function:: void maybeRequestLayout(ZWidget *w)

   These functions can be used to manage layout cycles.
   They are mostly used by :cpp:class:`Tui::ZLayout`, but could be used to in implementations of other layout systems too.

   ..
      TODO more details

.. cpp:function:: void resize(int width, int height)

   Clears and resizes the ``ZTerminal`` side terminal buffer.
   The ``ZTerminal`` side terminal buffer should be sized to match the size of the actual terminal and many parts of
   Tui Widgets assume that this holds true.

.. rst-class:: tw-signal
.. cpp:function:: void terminalConnectionLost()

   This signal is emitted if the connection to the default terminal or a terminal connected through
   :cpp:func:`ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr) <void Tui::ZTerminal::ZTerminal(FileDescriptor fd, Options options, QObject *parent = nullptr)>`
   is lost.

   After this signal is emitted ``ZTerminal`` will no longer be able to do any updates or receive input from the formerly
   connected terminal.

   If the signal is not connected ``ZTerminal`` will terminate the application using :cpp:func:`QCoreApplication::quit()`
   instead.

.. cpp:function:: ZTextMetrics textMetrics() const

   Returns a :cpp:class:`ZTextMetrics` instance that describes how text is assigned to cells in the connected terminal.

   The result should only be used after auto detection is completed.

.. cpp:function:: void update()

   Request an rendering cycle to be done soon.
   The update will be processed using an event with low priority or a timer.

   See also: :cpp:func:`void Tui::ZWidget::update()`

.. cpp:function:: void updateOutput()

   Send the current contents of the ``ZTerminal`` side terminal buffer to the terminal with an incremental update.

   |standalone|

.. cpp:function:: void updateOutputForceFullRepaint()

   Send the current contents of the ``ZTerminal`` side terminal buffer to the terminal with an full (non incremental) update.

   |standalone|

.. cpp:function:: int width() const

   This function return the current width of the terminal as seen by ``ZTerminal``.

   If :cpp:enumerator:`Tui::ZTerminal::Option::DisableAutoResize` is not in effect the width should match the
   actual terminal width unless the application has overridden the width using
   :cpp:func:`~void Tui::ZTerminal::resize(int width, int height)`. If the terminal connection does not propagate
   size changes the width might be outdated.

   If :cpp:enumerator:`Tui::ZTerminal::Option::DisableAutoResize` is in effect the width returned is either the
   initial size of the terminal or the width of the last call to
   :cpp:func:`~void Tui::ZTerminal::resize(int width, int height)`.

.. cpp:function:: std::unique_ptr<Tui::ZKeyEvent> translateKeyEvent(const Tui::ZTerminalNativeEvent &nativeEvent)

   Translates a native key event into a :cpp:class:`Tui::ZKeyEvent`.

   Applications that need to customize low level terminal handling can override this function.

Nested Types
------------

.. cpp:namespace:: NULL

.. rst-class:: tw-midspacebefore
.. cpp:struct:: Tui::ZTerminal::FileDescriptor

  This struct is a wrapper for type safe passing of a file descriptor.

  .. cpp:function:: explicit FileDescriptor(int fd)

     Construct an instance with ``fd`` as file descriptor.
     The class does not take ownership of ``fd``.
     The application has to ensure that ``fd`` is kept open for at least as long as a :cpp:class:`Tui::ZTerminal` instance
     using it exists.


.. rst-class:: tw-midspacebefore
.. cpp:struct:: Tui::ZTerminal::OffScreen

   This class is copyable and assignable.

   This class is used to configure details of the offscreen mode of :cpp:class:`Tui::ZTerminal`.

   .. cpp:function:: OffScreen(int width, int height)

      Construct offscreen configuration with requested width ``width`` and requested height ``height``.

   .. cpp:function:: OffScreen withCapability(ZSymbol capability)

      Returns a modified OffScreen object that requests that ``capability`` be enabled in the terminal instance.

      Unknown capabilities are ignored.

      See :ref:`term_capabilites` for possible capabilities.

   .. cpp:function:: OffScreen withoutCapability(ZSymbol capability)

      Returns a modified OffScreen object that requests that ``capability`` be disabled in the terminal instance.

      Unknown capabilities are ignored.

      See :ref:`term_capabilites` for possible capabilities.

.. rst-class:: tw-midspacebefore
.. cpp:struct:: Tui::ZTerminal::TerminalConnection

   This class represents an application defined connection to a terminal.

   The application need to setup the class using :cpp:func:`void setDelegate(TerminalConnectionDelegate *delegate)`,
   :cpp:func:`void setBackspaceIsX08(bool val)` and :cpp:func:`void setSize(int width, int height)` before passing
   the instance to the :cpp:class:`Tui::ZTerminal` constructor.

   After connnecting a :cpp:class:`Tui::ZTerminal` instance the application must supply terminal input by calling
   :cpp:func:`void terminalInput(const char *data, int length)` and may change the terminal size by calling
   :cpp:func:`void setSize(int width, int height)`.

   The application has to ensure that the instance is kept valid for at least as long as the terminal instance
   using it exists.
   One instance of this class can not be used with multiple terminal instances at the same time.

   .. cpp:function:: void terminalInput(const char *data, int length)

      The application uses this function to transmit terminal input to the terminal instance connected to this instance.

      It is an error to call this method while this instance is not connected to a terminal instance.

      The input of length ``length`` is passed in the ``data`` parameter.

   .. cpp:function:: void setDelegate(TerminalConnectionDelegate *delegate)

      The application must use this function to set an delegate object ``delegate`` for the terminal connection before
      connecting the instance to a terminal instance.

      It is an error to connect this instance to a terminal instance without having set a valid delegate.

   .. cpp:function:: void setBackspaceIsX08(bool val)

      Set how the backspace key is send by the terminal.
      if ``val`` is :cpp:expr:`true`, the terminal is expected to send a 0x08 character for backspace.
      if ``val`` is :cpp:expr:`false`, the terminal is expected to send a 0x7f character for backspace.

      If the terminal connection originates from a kernel terminal device ``val`` should be the ``true`` if the
      terminal is setup with ``termios.c_cc[VERASE] == 0x08``.

      The application should call this function before connecting the instance to a terminal instance, changes after
      connecting the terminal are ignored.

   .. cpp:function:: void setSize(int width, int height)

      Set the terminal size to width ``width`` and height ``height``.

      The application should call this function before connecting the instance to a terminal instance and for each
      terminal size change after the initial call.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZTerminal::TerminalConnectionDelegate

   This class acts as a delegate class for an application definied connection to a terminal.

   The application has to implement a derived class that implements all pure virtual functions for communication to the
   terminal.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void write(const char *data, int length)

      The terminal instance calls this function to write ``length`` bytes starting at ``data`` to the terminal.

      The delegate must either send these bytes to the terminal or buffer them for later delivery.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void flush()

      The terminal instance calls this function when the bytes send via :cpp:func:`void write(const char *data, int length)`
      have to be delivered to the terminal.

      It will be called when all terminal output for a given refresh or action is completed.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void restoreSequenceUpdated(const char *data, int len)

      The terminal instance calls this function whenever the sequence of bytes that need to be output to the
      terminal on unclean connection shutdown are changed.

      The delegate should save the ``length`` bytes starting at ``data`` for this purpose.

      If possible the delegate should arrange for these bytes to be send to the terminal even if the process shuts down
      uncleanly. For example in a client/server setup where a client forwards a terminal connection and the server is
      running the Tui Widgets application is would be good to move the restore sequence to the client.

      If the terminal instance is destroyed cleanly (i.e. the destructor of ``ZTerminal`` is run) this sequence should not
      be sent to the terminal, as the clean shutdown already handles this.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void deinit(bool awaitingResponse)

      The terminal instance calls this function on clean shutdown.

      if ``awaitingResponse`` is set the terminal will likely still send replies to query sequences sent by the
      ``ZTerminal`` instance. The delegate might try catch these replies by reading and discarding all terminal input
      for a limited time.

   .. rst-class:: tw-virtual
   .. cpp:function:: void pause()

      The terminal instance calls this function whenever :cpp:func:`void Tui::ZTerminal::pauseOperation()` is called.

   .. rst-class:: tw-virtual
   .. cpp:function:: void unpause()

      The terminal instance calls this function whenever :cpp:func:`void Tui::ZTerminal::unpauseOperation()` is called.
..
   Common text fragements:

.. |standalone| replace:: Usage of this function and a main widget will interfere. Use this only for
   :ref:`standalone <term_standalone>` usage.

.. |standalone-or-ar| replace:: Usage of this function and a main widget will interfere. Use this for
   :ref:`standalone <term_standalone>` usage or possibly in code connected
   to the :cpp:func:`~Tui::ZTerminal::afterRendering()` signal.
