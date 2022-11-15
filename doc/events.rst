.. _events:

Events
======

Tui Widgets is an event driven framework. Events are deliviered primarily to instances of :cpp:class:`Tui::ZWidget` but
are also delivered to other classes derived from :cpp:class:`QObject`.

Classes either override virtual functions for specific events (like :cpp:func:`Tui::ZWidget::paintEvent()`) or
override :cpp:func:`QObject::event` and manually dispatching on the event type.

Tui Widgets defines its own event types and also reuses some event types from Qt.

.. rst-class:: tw-invisible
.. cpp:class:: Tui::ZEventType

Events are identified by :cpp:enum:`QEvent::Type` values.
Tui Widgets defines getter for the type values for its events. The Tui Widgets event types are:

.. cpp:function:: Tui::ZEventType::paint()

   Requests a widget to paint itself (:cpp:class:`Tui::ZPaintEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::paintEvent(Tui::ZPaintEvent *event)` to handle this event.

   The widgets can get the :cpp:class:`Tui::ZPainter` instance for painting by calling the
   :cpp:func:`Tui::ZPainter *Tui::ZPaintEvent::painter() const` function.

.. cpp:function:: Tui::ZEventType::key()

   Signals a key press that the widget may handle (:cpp:class:`Tui::ZKeyEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::keyEvent(Tui::ZKeyEvent *event)` to handle this event.

   A key event is either a character or a non character key.

   In case of a character the application can get the pressed character by reading
   :cpp:func:`QString Tui::ZKeyEvent::text() const`.
   The function might return multiple unicode code points in the future for languages with clusters of multiple
   codepoints.

   In case of a non character key the application can get the key by reading
   :cpp:func:`int Tui::ZKeyEvent::key() const`.

   Only on of these are meaningful at a given time. The other will be :cpp:enumerator:`Tui::Key_unknown` or
   the empty string.

   In both cases the application can get the modifier keys pressed when the key input happend by reading
   :cpp:func:`Tui::KeyboardModifiers Tui::ZKeyEvent::modifiers() const`.

   Tui Widgets sends the key events in the accepted state (see :cpp:func:`QEvent::isAccepted()`).
   If the application passes the handling of the key event to the
   default implementations of :cpp:func:`void Tui::ZWidget::keyEvent(Tui::ZKeyEvent *event)` and
   ``void Tui::ZWidget::event(QEvent *event)`` the accepted flag will be cleared.

   If a key event, that is dispatched to the focused widget, is not in the accepted state after dispatching to a ZWidget,
   it will bubble through all parents of the widget until reaching the root. It will be dispatched to each widget on
   the way until a widget accepts it.

   Widgets can also receive key events if they currently have the keyboard grab (see
   :cpp:func:`void Tui::ZWidget::grabKeyboard()`), in this case the event will not bubble toward the root.

.. cpp:function:: Tui::ZEventType::paste()

   Signals a clipboard paste that the widget may handle (:cpp:class:`Tui::ZPasteEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::pasteEvent(Tui::ZPasteEvent *event)` to handle this event.

   The widget can get the pasted text by calling :cpp:func:`QString Tui::ZPasteEvent::text() const`.

   Tui Widgets sends the paste events in the accepted state (see :cpp:func:`QEvent::isAccepted()`).
   If the application passes the handling of the paste event to the
   default implementations of :cpp:func:`void Tui::ZWidget::pasteEvent(Tui::ZPasteEvent *event)` and
   ``void Tui::ZWidget::event(QEvent *event)`` the accepted flag will be cleared.

   If a paste event, that is dispatched to the focused widget, is not in the accepted state after dispatching to a ZWidget,
   it will bubble through all parents of the widget until reaching the root. It will be dispatched to each widget on
   the way until a widget accepts it.

   Widgets can also receive paste events if they currently have the keyboard grab (see
   :cpp:func:`void Tui::ZWidget::grabKeyboard()`), in this case the event will not bubble toward the root.

.. cpp:function:: Tui::ZEventType::queryAcceptsEnter()

   Queries if a widget will handle the :kbd:`Enter` key (:cpp:class:`QEvent`).

   If it will handle the key it should accept the event.
   This is used to determine the visual state of default widgets (i.e. default buttons) that react to :kbd:`Enter`.

.. cpp:function:: Tui::ZEventType::focusIn()

   Informs a widget that it gained keyboard focus (:cpp:class:`Tui::ZFocusEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::focusInEvent(Tui::ZFocusEvent *event)` to handle this event.

   The widget can query the focus reason using :cpp:func:`Tui::FocusReason Tui::ZFocusEvent::reason() const`

.. cpp:function:: Tui::ZEventType::focusOut()

   Informs a widget that it lost keyboard focus (:cpp:class:`Tui::ZFocusEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::focusOutEvent(Tui::ZFocusEvent *event)` to handle this event.

   The widget can query the focus reason using :cpp:func:`Tui::FocusReason Tui::ZFocusEvent::reason() const`

.. cpp:function:: Tui::ZEventType::move()

   Informs a widget that its position relative to its parents has changed (:cpp:class:`Tui::ZMoveEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::resizeEvent(Tui::ZResizeEvent *event)` to handle this event.

   The widget can query the previous position using :cpp:func:`QPoint Tui::ZMoveEvent::oldPos() const` and the
   new position using :cpp:func:`QPoint Tui::ZMoveEvent::pos() const`.

.. cpp:function:: Tui::ZEventType::resize()

   Informs a widget that its size has changed (:cpp:class:`Tui::ZResizeEvent`).

   Widgets can override :cpp:func:`void Tui::ZWidget::resizeEvent(Tui::ZResizeEvent *event)` to handle this event.

   The widget can query the previous size using :cpp:func:`QSize Tui::ZResizeEvent::oldSize() const` and the
   new size using :cpp:func:`QSize Tui::ZResizeEvent::size() const`.

.. cpp:function:: Tui::ZEventType::otherChange()

   This event is sent to every QObject in widget tree on some changes (:cpp:class:`Tui::ZOtherChangeEvent`).

   The application can use :cpp:func:`bool Tui::ZOtherChangeEvent::match(const QEvent *event, Tui::ZSymbol changed)` to
   match the event against a type of change.

.. cpp:function:: Tui::ZEventType::show()

   Informs the widget that it has become visible (:cpp:class:`QEvent`).

   See also :ref:`QEvent::ShowToParent <qevent_showtoparent>`.

.. cpp:function:: Tui::ZEventType::hide()

   Informs the widget that it is no longer visible (:cpp:class:`QEvent`).

   See also :ref:`QEvent::HideToParent <qevent_hidetoparent>`.

.. cpp:function:: Tui::ZEventType::close()

   Used as query to decide if a window should be closed (:cpp:class:`Tui::ZCloseEvent`).

   Classes derived from :cpp:class:`Tui::ZWindow` can override
   :cpp:func:`void Tui::ZWindow::closeEvent(Tui::ZCloseEvent *event)` to handle this event.

   See the section on the :ref:`close request protcol <ZWindow_close_requests>` for details.

.. cpp:function:: Tui::ZEventType::updateRequest()

   This event is used internally to coordinate the emission of paint events (:cpp:class:`Tui::ZPaintEvent`).

.. cpp:function:: Tui::ZEventType::terminalNativeEvent()

   This event is dispatched on :cpp:class:`ZTerminal` when processing native events from the low level terminal
   abstraction library (:cpp:class:`Tui::ZTerminalNativeEvent`).

   Applications that need to customize low level terminal handling can intercept it by subclassing ZTerminal or using
   an event filter.

.. cpp:function:: Tui::ZEventType::rawSequence()

   This event is dispatched on :cpp:class:`ZTerminal` while processing terminal input
   sequences (:cpp:class:`Tui::ZRawSequenceEvent`).

   Applications that need to customize low level terminal handling can intercept it by subclassing ZTerminal or using
   an event filter.

.. cpp:function:: Tui::ZEventType::pendingRawSequence()

   This event is dispatched on :cpp:class:`ZTerminal` while processing terminal input
   sequences that are not completely received yet (:cpp:class:`Tui::ZRawSequenceEvent`).

   Applications that need to customize low level terminal handling can intercept it by subclassing ZTerminal or using
   an event filter.

The following Qt signals are used by Tui Widgets:

.. rst-class:: tw-spaceafter
.. adhoc-def:: QEvent::LayoutRequest

   Used in :cpp:class:`Tui::ZWidget` and :cpp:class:`Tui::ZTerminal` to trigger relayout (:cpp:class:`QEvent`).


.. rst-class:: tw-spaceafter
.. _qevent_showtoparent:
.. adhoc-def:: QEvent::ShowToParent

   Informs a widget that is is now locally visible (:cpp:class:`QEvent`).

   If the effective visibility changed the widget will have received a :cpp:func:`Tui::ZEventType::show()` event before
   this event.

.. rst-class:: tw-spaceafter
.. _qevent_hidetoparent:
.. adhoc-def:: QEvent::HideToParent

   Informs a widget that is is no longer locally visible (:cpp:class:`QEvent`).

   If the effective visibility changed the widget will have received a :cpp:func:`Tui::ZEventType::hide()` event before
   this event.

.. rst-class:: tw-spaceafter
.. _qevent_enablechanged:
.. adhoc-def:: QEvent::EnabledChange

   Informs a widget that its enabled state has changed.

   This event is sent whenever the the effective enabled state is changed.

..
  TOOD
  QEvent::ParentAboutToChange

ZEvent
------

.. cpp:class:: Tui::ZEvent : public QEvent

   ZEvent is the base class of all event classes defined by Tui Widgets.

   It is copyable and does not define comparison operators.

   It has no user accessable constructors or functions.


ZPaintEvent
-----------

.. cpp:class:: Tui::ZPaintEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::paint()` for usage details.

   .. cpp:function:: ZPaintEvent(Tui::ZPainter *painter)

      Creates a :cpp:func:`Tui::ZEventType::paint()` event using the painter ``painter``.

   .. cpp:function:: ZPaintEvent(Tui::ZPaintEvent::Update, Tui::ZPainter *painter)

      Creates a :cpp:func:`Tui::ZEventType::updateRequest()` event using the painter ``painter``.

   .. cpp:function:: Tui::ZPainter *painter() const

      Returns the painter associated with the event.

   .. cpp:class:: Update
   .. cpp:var:: static constexpr Update update {}

      Used as tag to select the constructor of this class.

ZKeyEvent
---------

.. cpp:class:: Tui::ZKeyEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::key()` for usage details.

   .. cpp:function:: ZKeyEvent(int key, Tui::KeyboardModifiers modifiers, const QString &text)

       Creates a :cpp:func:`Tui::ZEventType::key()` event using the key ``key``, text ``text`` and
       modifiers ``modifiers``.

   .. cpp:function:: int key() const

       Returns the key associated with the event.

   .. cpp:function:: QString text() const

      Returns the text associated with the event.

   .. cpp:function:: Tui::KeyboardModifiers modifiers() const

      Returns the modifiers associated with the event.

ZPasteEvent
-----------

.. cpp:class:: Tui::ZPasteEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::paste()` for usage details.

   .. cpp:function:: ZPasteEvent(const QString &text)

      Creates a :cpp:func:`Tui::ZEventType::key()` event using text ``text``.

   .. cpp:function:: QString text() const

      Returns the text associated with the event.


ZFocusEvent
-----------

.. cpp:class:: Tui::ZFocusEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::focusIn()` and :cpp:func:`Tui::ZEventType::focusOut()` for usage details.

   .. cpp:function:: ZFocusEvent(Tui::ZFocusEvent::FocusIn, Tui::FocusReason reason = Tui::OtherFocusReason)

      Creates a :cpp:func:`Tui::ZEventType::focusIn()` event using reason ``reason``.

   .. cpp:function:: ZFocusEvent(Tui::ZFocusEvent::FocusOut, Tui::FocusReason reason = Tui::OtherFocusReason)

      Creates a :cpp:func:`Tui::ZEventType::focusOut()` event using reason ``reason``.

   .. cpp:function:: Tui::FocusReason reason() const

      Returns the focus reason associated with the event.

   .. cpp:class:: FocusIn
   .. cpp:var:: static constexpr FocusIn focusIn {}
   .. cpp:class:: FocusOut
   .. cpp:var:: static constexpr FocusOut focusOut {}

      Used as tags to select the constructor of this class.

ZMoveEvent
----------

.. cpp:class:: Tui::ZMoveEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::move()` for usage details.

   .. cpp:function:: ZMoveEvent(QPoint pos, QPoint oldPos)

      Creates a :cpp:func:`Tui::ZEventType::move()` event using position ``pos`` and old position ``oldPos``.

   .. cpp:function:: QPoint pos() const

      Returns the position associated with the event.

   .. cpp:function:: QPoint oldPos() const

      Returns the old position associated with the event.

ZResizeEvent
------------

.. cpp:class:: Tui::ZResizeEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::resize()` for usage details.

   .. cpp:function:: ZResizeEvent(QSize size, QSize oldSize)

      Creates a :cpp:func:`Tui::ZEventType::resize()` event using size ``size`` and old size ``oldSize``.

   .. cpp:function:: QSize size() const

      Returns the size associated with the event.

   .. cpp:function:: QSize oldSize() const

      Returns the old size associated with the event.

ZOtherChangeEvent
-----------------

.. cpp:class:: Tui::ZOtherChangeEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::otherChange()` for usage details.

   .. cpp:function:: ZOtherChangeEvent(QSet<Tui::ZSymbol> unchanged)

      Creates a :cpp:func:`Tui::ZEventType::otherChange()` event using ``unchanged`` as set of unchanged symbols.

   .. cpp:function:: QSet<Tui::ZSymbol> unchanged() const

      Returns the set of unchanged symbols associated with the event.

   .. cpp:function:: static QSet<Tui::ZSymbol> all()

      Returns a list of all known change symbols.

   .. rst-class:: tw-static
   .. cpp:function:: bool match(const QEvent *event, Tui::ZSymbol changed)

      Matches the event ``event`` against the change symbol ``changed``. It is safe to use this with any valid QEvent.

ZCloseEvent
-----------

.. cpp:class:: Tui::ZCloseEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::close()` for usage details.

   .. cpp:function:: ZCloseEvent(QStringList skipChecks)

      Creates a :cpp:func:`Tui::ZEventType::close()` event using ``skipChecks`` as list of checks to skip.

   .. cpp:function:: QStringList skipChecks() const

      Returns the list of checks to skip associated with the event.

ZRawSequenceEvent
-----------------

.. cpp:class:: Tui::ZRawSequenceEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::rawSequence()` and :cpp:func:`Tui::ZEventType::pendingRawSequence()` for usage details.

   .. cpp:function:: ZRawSequenceEvent(QByteArray seq)

      Creates a :cpp:func:`Tui::ZEventType::rawSequence()` event using ``seq`` as sequence.

   .. cpp:function:: ZRawSequenceEvent(Pending, QByteArray seq)

      Creates a :cpp:func:`Tui::ZEventType::pendingRawSequence()` event using ``seq`` as sequence.

   .. cpp:function:: QByteArray sequence() const

      Returns the sequence associated with the event.

   .. cpp:class:: Pending
   .. cpp:var:: static constexpr Pending pending {}

      Used as tag to select the constructor of this class.


ZTerminalNativeEvent
--------------------

.. cpp:class:: Tui::ZTerminalNativeEvent : public Tui::ZEvent

   See :cpp:func:`Tui::ZEventType::terminalNativeEvent()` for usage details.

   .. cpp:function:: ZTerminalNativeEvent(void *native)

      Initializes native pointer to ``native``.

   .. cpp:function:: void *nativeEventPointer() const

      Returns a pointer to the native terminal event.

      Currently it returns a pointer to a :c:type:`termpaint_event`.
