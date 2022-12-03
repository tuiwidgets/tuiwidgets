.. _ZWidget:

ZWidget
=======

ZWidget is the base class of all widgets in Tui Widgets.

It defines the common state and behavior of widgets.
Applications typically use derived classes as visual elements.
Applications can define custom widgets be defining classes that inherit from ZWidget.

Size, Position and Visibility
-----------------------------

The root widget of a widget tree needs to be connected to a :cpp:class:`Tui::ZTerminal` instance using the
:cpp:func:`~void Tui::ZTerminal::setMainWidget(ZWidget *w)` function.
Its position is fixed to cover the terminal.

All other widgets have a parent widget and have a position relative to the parent.
The position specifies the  top-left cell the widget starts in.
The widget then expands to the right and down as specified by its width and height.

If a widget is placed outside of its parent the parts that don't overlap with the parent are not visible.
They are clipped.

The widget has a visible property (:cpp:func:`~bool Tui::ZWidget::isLocallyVisible() const`,
:cpp:func:`~void Tui::ZWidget::setVisible(bool v)`)
that determines if it is rendered and is considered for input.
If the parent of a widget is not visible the widget itself is not rendered too.
The visible property describes the state if the parent is visible.
If determine the state that includes the visibility of all parents a widget can be queried for its effective visibility
using :cpp:func:`~bool Tui::ZWidget::isVisible() const`.

All child widgets that share a parent are organised into a stacking order.
When Widgets overlap the widget lower in this order is covered by the widget higher in the stacking order.

Widgets that are newly added to a parent at placed on top of other widgets with the same stacking layer.
The stacking layer(:cpp:func:`~void Tui::ZWidget::setStackingLayer(int layer)`,
:cpp:func:`~int Tui::ZWidget::stackingLayer() const`) allows defining bands in the stacking that can be managed
independently.
For example to place popup menus or dialogs above normal user interface elements.

Points can be mapped from relative to a given widget to relative to the terminal and back using
:cpp:func:`~QPoint Tui::ZWidget::mapFromTerminal(const QPoint &pos)` and
:cpp:func:`~QPoint Tui::ZWidget::mapToTerminal(const QPoint &pos)`.

Layout and Margins
------------------

Widgets can have margins.
The margin of a widget defined how much whitespace should be placed around the contents of if.
Not all widget classes support marging.

Often it is convinient to have the placement of widgets in a parent automated.
This allows exact placement to automatically adapt to changes to the size of the widget and avoids doing position
calculations manually or adhoc in the code using the widgets.
This automated placement is done by so called "layouts".
Layouts like :cpp:class:`Tui::ZHBoxLayout` and
:cpp:class:`Tui::ZVBoxLayout` are classes derived from :cpp:class:`Tui::ZLayout`.
Each widget can have one top level layout set using :cpp:func:`~void Tui::ZWidget::setLayout(ZLayout *l)`.
Then child widgets can be added to that layout (or one of its nested layouts) using layout specific functions.

To customized the layout the functions :cpp:func:`~void Tui::ZWidget::setMaximumSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setMinimumSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setFixedSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setSizePolicyH(Tui::SizePolicy policy)` and
:cpp:func:`~void Tui::ZWidget::setSizePolicyV(Tui::SizePolicy policy)` can be used.
They allow specifing how the available space is allocated to the widgets.

Focus
-----

Keyboard input and paste events are processed by the widget that currently has focus.
Focus is a per terminal property.
The focus can be placed on a widget by calling
:cpp:func:`setFocus() <void Tui::ZWidget::setFocus(Tui::FocusReason reason)>`.

In a :ref:`window <ZWindow>`, focus can be switched by the user using :kbd:`Tab` and :kbd:`Shift+Tab`.
This cycles through visible and enabled widgets that have a
:cpp:func:`focus policy <void Tui::ZWidget::setFocusPolicy(Tui::FocusPolicy policy)>` that allows for keyboard focusing.

:cpp:class:`Tui::ZRoot` implements switching focus between windows using :kbd:`F6` and :kbd:`Shift+F6`.

Focus order can be setup using :cpp:func:`~void Tui::ZWidget::setFocusOrder(int order)`.
For widgets with the same focus order value, focus follows the stacking order from bottom to top.

In addition to using the keyboard to move in the focus among the focus order the functions
:cpp:func:`ZWidget *Tui::ZWidget::placeFocus(bool last)`,
:cpp:func:`ZWidget *Tui::ZWidget::nextFocusable()` and
:cpp:func:`ZWidget *Tui::ZWidget::prevFocusable()`
can be used for focus management.

Widgets optionally can act as focus containers.
This modifies how focus inside such widgets is handled.

For widgets that act as windows the focus container mode :cpp:enumerator:`Tui::FocusContainerMode::Cycle` can be used
to restrict focus changes by :kbd:`Tab`/:kbd:`Shift+Tab` and via
:cpp:func:`ZWidget *Tui::ZWidget::nextFocusable()`/:cpp:func:`ZWidget *Tui::ZWidget::prevFocusable()` from moving the
focus outside of the widget.
If the last focusable widget inside the widget marked with the mode was focused last the next widget to focus will
be the first focusable widget in the container.
Reverse focus movement works respectivly.

To restrict the scope of the effects of the focus order property the focus container mode
:cpp:enumerator:`Tui::FocusContainerMode::SubOrdering` can be used.

..
  TODO: say more about sub ordering mode?

Widgets can be enabled, that is ready for user interaction, or disabled.
Similarily to how visibility works, enabled is a local setting but only is effective when the parent is also effecivly
enabled.

Sometimes a widget needs to override focus handling and temporarily route all keyboard input to itself.
This can be achieved by initiation of a keyboard grab using :cpp:func:`~void Tui::ZWidget::grabKeyboard()`.

Terminal
--------

Widgets only fully work in conjunction with a terminal represented by a :cpp:class:`Tui::ZTerminal` instance.
A widget is connected to a terminal if itself is the main widget of a terminal or one of its ancestors is the main
widget of a terminal.

Fully functional focus handling depends on the terminal, although unconnected widgets keep a note on calling
:cpp:func:`setFocus() <void Tui::ZWidget::setFocus(Tui::FocusReason reason = Tui::OtherFocusReason)>`, as a fallback
for the special case that a widget tree is constructed before setting the terminal's main widget.
Apart from this minimal support, focus, keyboard grabs, cursor configuration and even text measuring are not available
without a terminal.


Usage of widgets
----------------

Generally a widget is created with a parent as a constructor parameter and then either manually placed using
:cpp:func:`~void Tui::ZWidget::setGeometry(const QRect &geometry)` or placed using a layout.


TODO example


The colors used in the widgets that are part of Tui Widgets are taken from a palette associated with the widget or one
of its parents.
In some situations the colors can also be influenced by the palette class set on a widget or one of its parents.

When using widgets in a window focus order if determined by :cpp:func:`void Tui::ZWidget::setFocusOrder(int order)`,
where widgets with the same focus order are reached in order of their z-order from bottom (smaller values) to
top (larger values).

Defining custom widgets
-----------------------

While Tui Widgets provides a set of useful widgets for common tasks, there are often situations where a custom
widget is a good way to realize a part of the user interface.

Custom widgets are created by subclassing ZWidget.
The widget behavior can then be customized by overriding virtual functions.

To customize rendering of the widget override :cpp:func:`~void Tui::ZWidget::paintEvent(Tui::ZPaintEvent *event)`.
To customize input handling override :cpp:func:`~void Tui::ZWidget::keyEvent(Tui::ZKeyEvent *event)` and
:cpp:func:`~void Tui::ZWidget::pasteEvent(Tui::ZPasteEvent *event)`.
Also set the :cpp:func:`focus policy <void Tui::ZWidget::setFocusPolicy(Tui::FocusPolicy policy)>` to receive input.

To customize reactions to changes in widget state use
:cpp:func:`~void Tui::ZWidget::focusInEvent(Tui::ZFocusEvent *event)`,
:cpp:func:`~void Tui::ZWidget::focusOutEvent(Tui::ZFocusEvent *event)`,
:cpp:func:`~void Tui::ZWidget::moveEvent(Tui::ZMoveEvent *event)` and
:cpp:func:`~void Tui::ZWidget::resizeEvent(Tui::ZResizeEvent *event)` .

Further customization is available using
:cpp:func:`~virtual QObject *Tui::ZWidget::facet(const QMetaObject &metaObject) const`,
:cpp:func:`~virtual QSize Tui::ZWidget::sizeHint() const` and
:cpp:func:`~virtual QSize Tui::ZWidget::minimumSizeHint() const`.

It is often useful to setup
:cpp:func:`~void Tui::ZWidget::setPaletteClass(QStringList classes)`,
:cpp:func:`~void Tui::ZWidget::setContentsMargins(QMargins m)`,
:cpp:func:`~void Tui::ZWidget::setMinimumSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setMaximumSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setFixedSize(QSize s)`,
:cpp:func:`~void Tui::ZWidget::setFocusMode(FocusContainerMode mode)`,
:cpp:func:`~void Tui::ZWidget::setFocusPolicy(Tui::FocusPolicy policy)`,
:cpp:func:`~void Tui::ZWidget::setSizePolicyH(Tui::SizePolicy policy)`,
:cpp:func:`~void Tui::ZWidget::setSizePolicyV(Tui::SizePolicy policy)` and
:cpp:func:`~void Tui::ZWidget::setStackingLayer(int layer)`
to a widget specific default in the widget constructor, but these are genereally reserved for customization by the
widget user after construction.

ZWidget
-------

.. cpp:class:: Tui::ZWidget : public QObject

   This class is neither copyable nor movable. It does not define comparison operators.

   **Constructors**

   :cpp:func:`Tui::ZWidget::ZWidget(ZWidget *parent = nullptr) <void Tui::ZWidget::ZWidget(ZWidget *parent = nullptr)>`

   **Functions**

   | :cpp:func:`void addPaletteClass(const QString &clazz)`
   | :cpp:func:`ZCommandManager *commandManager() const`
   | :cpp:func:`QMargins contentsMargins() const`
   | :cpp:func:`QRect contentsRect() const`
   | :cpp:func:`CursorStyle cursorStyle() const`
   | :cpp:func:`QSize effectiveMinimumSize() const`
   | :cpp:func:`QSize effectiveSizeHint() const`
   | :cpp:func:`ZCommandManager *ensureCommandManager()`
   | :cpp:func:`virtual QObject *facet(const QMetaObject &metaObject) const`
   | :cpp:func:`template<typename T> T *findFacet() const`
   | :cpp:func:`bool focus() const`
   | :cpp:func:`FocusContainerMode focusMode() const`
   | :cpp:func:`int focusOrder() const`
   | :cpp:func:`Tui::FocusPolicy focusPolicy() const`
   | :cpp:func:`QRect geometry() const`
   | :cpp:func:`ZColor getColor(const ZImplicitSymbol &x)`
   | :cpp:func:`void grabKeyboard()`
   | :cpp:func:`void grabKeyboard(Private::ZMoFunc<void(QEvent*)> handler)`
   | :cpp:func:`bool isAncestorOf(const ZWidget *child) const`
   | :cpp:func:`bool isEnabled() const`
   | :cpp:func:`bool isEnabledTo(const ZWidget *ancestor) const`
   | :cpp:func:`bool isInFocusPath() const`
   | :cpp:func:`bool isLocallyEnabled() const`
   | :cpp:func:`bool isLocallyVisible() const`
   | :cpp:func:`bool isVisible() const`
   | :cpp:func:`bool isVisibleTo(const ZWidget *ancestor) const`
   | :cpp:func:`ZLayout *layout() const`
   | :cpp:func:`virtual QRect layoutArea() const`
   | :cpp:func:`void lower()`
   | :cpp:func:`QPoint mapFromTerminal(const QPoint &pos)`
   | :cpp:func:`QPoint mapToTerminal(const QPoint &pos)`
   | :cpp:func:`QSize maximumSize() const`
   | :cpp:func:`QSize minimumSize() const`
   | :cpp:func:`virtual QSize minimumSizeHint() const`
   | :cpp:func:`ZWidget const *nextFocusable() const`
   | :cpp:func:`ZWidget *nextFocusable()`
   | :cpp:func:`const ZPalette &palette() const`
   | :cpp:func:`QStringList paletteClass() const`
   | :cpp:func:`ZWidget *parentWidget() const`
   | :cpp:func:`const ZWidget *placeFocus(bool last = false) const`
   | :cpp:func:`ZWidget *placeFocus(bool last = false)`
   | :cpp:func:`ZWidget const *prevFocusable() const`
   | :cpp:func:`ZWidget *prevFocusable()`
   | :cpp:func:`void raise()`
   | :cpp:func:`QRect rect() const`
   | :cpp:func:`void releaseKeyboard()`
   | :cpp:func:`void removePaletteClass(const QString &clazz)`
   | :cpp:func:`void resetCursorColor()`
   | :cpp:func:`virtual ZWidget *resolveSizeHintChain()`
   | :cpp:func:`int stackingLayer() const`
   | :cpp:func:`void stackUnder(ZWidget *w)`
   | :cpp:func:`void setCommandManager(ZCommandManager *cmd)`
   | :cpp:func:`void setContentsMargins(QMargins m)`
   | :cpp:func:`void setCursorColor(int r, int b, int g)`
   | :cpp:func:`void setCursorStyle(CursorStyle style)`
   | :cpp:func:`void setEnabled(bool e)`
   | :cpp:func:`void setFixedSize(QSize s)`
   | :cpp:func:`void setFixedSize(int w, int h)`
   | :cpp:func:`void setFocus(Tui::FocusReason reason = Tui::OtherFocusReason)`
   | :cpp:func:`void setFocusMode(FocusContainerMode mode)`
   | :cpp:func:`void setFocusOrder(int order)`
   | :cpp:func:`void setFocusPolicy(Tui::FocusPolicy policy)`
   | :cpp:func:`void setGeometry(const QRect &geometry)`
   | :cpp:func:`void setLayout(ZLayout *l)`
   | :cpp:func:`void setMaximumSize(QSize s)`
   | :cpp:func:`void setMaximumSize(int w, int h)`
   | :cpp:func:`void setMinimumSize(QSize s)`
   | :cpp:func:`void setMinimumSize(int w, int h)`
   | :cpp:func:`void setPalette(const ZPalette &pal)`
   | :cpp:func:`void setPaletteClass(QStringList classes)`
   | :cpp:func:`void setParent(ZWidget *newParent)`
   | :cpp:func:`void setSizePolicyH(Tui::SizePolicy policy)`
   | :cpp:func:`void setSizePolicyV(Tui::SizePolicy policy)`
   | :cpp:func:`void setStackingLayer(int layer)`
   | :cpp:func:`void setVisible(bool v)`
   | :cpp:func:`void showCursor(QPoint position)`
   | :cpp:func:`virtual QSize sizeHint() const`
   | :cpp:func:`Tui::SizePolicy sizePolicyH() const`
   | :cpp:func:`Tui::SizePolicy sizePolicyV() const`
   | :cpp:func:`ZTerminal *terminal() const`
   | :cpp:func:`void update()`
   | :cpp:func:`void updateGeometry()`

   **Protected Functions**

   | :cpp:func:`void focusInEvent(Tui::ZFocusEvent *event)`
   | :cpp:func:`void focusOutEvent(Tui::ZFocusEvent *event)`
   | :cpp:func:`void keyEvent(Tui::ZKeyEvent *event)`
   | :cpp:func:`void moveEvent(Tui::ZMoveEvent *event)`
   | :cpp:func:`void paintEvent(Tui::ZPaintEvent *event)`
   | :cpp:func:`void pasteEvent(Tui::ZPasteEvent *event)`
   | :cpp:func:`void resizeEvent(Tui::ZResizeEvent *event)`


Members
-------

.. cpp:namespace:: Tui::ZWidget

.. cpp:function:: explicit ZWidget(ZWidget *parent = nullptr)

   The constructed widget uses ``parent`` as its parent.

.. cpp:function:: ZWidget *parentWidget() const

   Returns the QObject parent if it is a widget or :cpp:expr:`nullptr` otherwise.

.. cpp:function:: void setParent(ZWidget *newParent)

   Sets the QObject parent to ``parent``, updating Tui Widgets bookkeeping as well.

   sends QEvent::ParentAboutToChange
   adjusts focus


   Always use ZWidget::setParent instead of QObject::setParent. Otherwise events are not generated and various
   widget and terminal state is not correctly updated.

.. cpp:function:: QRect geometry() const
.. cpp:function:: void setGeometry(const QRect &geometry)

   The geometry of a widget represents the position relative to its parent widget and its size.

.. cpp:function:: QRect rect() const

   Returns a QRect representing the whole widget in local coordinates.
   That is it starts at (0, 0) and has the same width and height as :cpp:func:`~QRect Tui::ZWidget::geometry() const`.

.. cpp:function:: QRect contentsRect() const

   Returns a QRect representing the part of the widget inside the margins in local coordinates.

.. cpp:function:: bool isEnabled() const
.. cpp:function:: bool isLocallyEnabled() const
.. cpp:function:: void setEnabled(bool e)

   The enabled state describes if a widget is accepting user interaction.
   Commonly widgets that are disabled (i.e. not enabled) have a visible difference to their enabled state.
   Tui Widgets does not send either :cpp:func:`Tui::ZEventType::key()` or :cpp:func:`Tui::ZEventType::paste()`
   events to disabled widgets.

   A widget is enabled if itself and all its parents are enabled.
   The local enabled state of the widget is availabe by calling ``isLocallyEnabled`` and can be changed by
   ``setEnabled``.

   ``setEnabled`` only directly effects the state returned by ``isEnabled`` if the parent widget's effectivly enabled
   state (the return value of ``isEnabled``) was already :cpp:expr:`true`.

   Only changes to the effective enabled state trigger an event.
   The event sent is :ref:`QEvent::EnabledChange <qevent_enablechanged>`.

   If a change in the effective enabled state of a focused widget results in it beeing disabled, it looses its focus
   and the focus is either moved to the next focusable widget or if no such widget exists the focus is removed.

.. cpp:function:: bool isVisible() const
.. cpp:function:: bool isLocallyVisible() const
.. cpp:function:: void setVisible(bool v)

   The visiblity of a widget describes if the widget is rendered.
   Even if a widget is visible according to this property it can still be occluded by a widget higher in the stacking
   order or be in a position that is not visible to the user.

   A widget is visible if itself and all its parents are visible.
   The local visibility state of the widget is availabe by calling ``isLocallyVisible`` and can be changed by
   ``setVisibile``.

   ``setVisible`` only directly effects the state returned by ``isVisible`` if the parent widget was already visible.
   Changing the local visibility state can trigger two kinds of events.
   Changes in the local visibility state trigger the events :ref:`QEvent::ShowToParent <qevent_showtoparent>` and
   :ref:`QEvent::HideToParent <qevent_hidetoparent>`.
   Changes to the effective visibility state (the return value of ``isVisible``) result in delivery of the events
   :cpp:func:`Tui::ZEventType::show()` and :cpp:func:`Tui::ZEventType::hide()`.

   If a change in the effective visibility state of a focused widget results in it beeing no longer visible, it looses
   its focus and the focus is either moved to the next focusable widget or if no such widget exists the focus is removed.

.. cpp:function:: void setStackingLayer(int layer)
.. cpp:function:: int stackingLayer() const

   The z-order of widgets is organized into stacking layers.
   Stacking layers with higher number are higher.
   In each stacking layer widgets can be moved using :cpp:func:`~void Tui::ZWidget::raise()`,
   :cpp:func:`~void Tui::ZWidget::lower()` and :cpp:func:`~void Tui::ZWidget::stackUnder(ZWidget *w)`.
   But these functions can not move a widget outside of its stacking layer.

   When moving a widget to a different stacking layer it is always placed as the top most widget of the new
   stacking layer.

.. cpp:function:: void raise()

   Move the widget to the top of its stacking layer.

.. cpp:function:: void lower()

   Move the widget to the bottom of its stacking layer.

.. cpp:function:: void stackUnder(ZWidget *w)

   Adjust z-order of the widget to be just below ``w``.
   Does not move the widget from its stacking layer.

.. cpp:function:: QSize minimumSize() const
.. cpp:function:: void setMinimumSize(QSize s)
.. cpp:function:: void setMinimumSize(int w, int h)
.. cpp:function:: QSize maximumSize() const
.. cpp:function:: void setMaximumSize(QSize s)
.. cpp:function:: void setMaximumSize(int w, int h)
.. cpp:function:: void setFixedSize(QSize s)
.. cpp:function:: void setFixedSize(int w, int h)

   The minimum and maximum size represent contraints for layouting.
   ``setFixedSize`` is just a short form for setting maximum and minimum size to the same value.
   Direct usage of :cpp:func:`~void Tui::ZWidget::setGeometry(const QRect &geometry)` is not constrained by these
   properties.

.. cpp:function:: Tui::SizePolicy sizePolicyH() const
.. cpp:function:: void setSizePolicyH(Tui::SizePolicy policy)
.. cpp:function:: Tui::SizePolicy sizePolicyV() const
.. cpp:function:: void setSizePolicyV(Tui::SizePolicy policy)

   The size policy of an widget is used by layouts to determine how space is allocated to the widget.
   See :cpp:enum:`Tui::SizePolicy` for details.

.. rst-class:: tw-virtual
.. cpp:function:: QSize sizeHint() const

   Returns the calculated size of the widget.

   This is used by the layout system when placing the widget.

   When implementing widgets, override this to return the preferred size of the widget.
   The meaning depends on the set :cpp:enum:`size policy <Tui::SizePolicy>`.

.. rst-class:: tw-virtual
.. cpp:function:: QSize minimumSizeHint() const

   Returns the calculated minimum size of the widget.

.. cpp:function:: QSize effectiveSizeHint() const
.. cpp:function:: QSize effectiveMinimumSize() const
.. rst-class:: tw-virtual
.. cpp:function:: QRect layoutArea() const

   Returns a QRect describing the area in the widget that should be used for layouts to place the child widgets.

.. cpp:function:: ZLayout *layout() const
.. cpp:function:: void setLayout(ZLayout *l)

   The layout of a widget allows automatically placing child widgets in the widget's layout area.

   If a layout is set it will control the size request of the widget based on the size requests of the widgets in
   the layout.

   See also: :ref:`ZLayout`

.. cpp:function:: void showCursor(QPoint position)

   Place the terminal cursor to widget relative position ``position`` if possible.

   Placing the cursor is possible if:

   * If this widget has focus and the keyboard focus is not grabbed, or the widget is the grabbing widget
   * The cursor is inside the clipping region of the widget and all its parent widgets.

.. cpp:function:: CursorStyle cursorStyle() const
.. cpp:function:: void setCursorStyle(CursorStyle style)

   The cursor style used to display the cursor in this widget.
   Support of this depends on the terminal's capabilities.

.. cpp:function:: void resetCursorColor()
.. cpp:function:: void setCursorColor(int r, int b, int g)

   The color used to display the cursor in this widget.
   Support of this depends on the terminal's capabilities.

.. cpp:function:: ZTerminal *terminal() const

   Returns the terminal this widget is connected to.

.. cpp:function:: void update()

   Requests the widget to be redrawn.

   When creating a custom widget the implementation must call this function whenever the the visible contents of the
   widget changes.
   It should never be needed to call this when just using a widget.

.. cpp:function:: void updateGeometry()

   Requests the layouts containing the widgets to be updated.

   When creating a custom widget the implementation must call this function whenever properties that influence the
   layout of this widget or its direct children change.
   It should never be needed to call this when just using a widget.

.. cpp:function:: void setFocus(Tui::FocusReason reason = Tui::OtherFocusReason)

   Requests focus for the widget.

.. cpp:function:: void setFocusPolicy(Tui::FocusPolicy policy)
.. cpp:function:: Tui::FocusPolicy focusPolicy() const

   The focus policy determines how this widget can gain focus.
   If the focus policy contains
   :cpp:enumerator:`Tui::TabFocus` the widget is focusable by keyboard navigation (i.e. using :kbd:`Tab` etc).

   :cpp:enumerator:`Tui::StrongFocus` includes :cpp:enumerator:`Tui::TabFocus`.

.. cpp:function:: void setFocusMode(FocusContainerMode mode)
.. cpp:function:: FocusContainerMode focusMode() const

   See :cpp:enum:`Tui::FocusContainerMode` for details.

.. cpp:function:: void setFocusOrder(int order)
.. cpp:function:: int focusOrder() const

   Defines the ordering of the widget in the focus order.
   Higher values yield a later position in the focus order.
   If two widgets in the same focus container have the same focus order, their relative z-order is used to determine
   focus order.

.. cpp:function:: QMargins contentsMargins() const
.. cpp:function:: void setContentsMargins(QMargins m)

   Margins allow to add empty space around the contents of a widget.

.. cpp:function:: const ZPalette &palette() const
.. cpp:function:: void setPalette(const ZPalette &pal)

   A widgets palette sets or modifies the palette colors for a widget and its decendents.

   Usually the root of the widget tree needs a palette that sets up all the standard colors for an application.
   This is usually done by using :cpp:class:`Tui::ZRoot` as root widget.

   Other widgets don't need to have a palette set, but it can be useful to set a palette for changing colors of
   specific widgets. In that case the usual way is to retrieve the palette, set some overriding color definitions and
   set the resulting palette on the widget.

   See :ref:`ZPalette` for details.

.. cpp:function:: ZColor getColor(const ZImplicitSymbol &x)

   Get a specific palette color named by ``x``.

   If the color is not properly defined this function will just return red as an error indicator.
   Many colors are only defined for widgets that are contained in windows.

   This internally uses :cpp:func:`ZColor Tui::ZPalette::getColor(ZWidget *targetWidget, ZImplicitSymbol x)`.

.. cpp:function:: QStringList paletteClass() const
.. cpp:function:: void setPaletteClass(QStringList classes)
.. cpp:function:: void addPaletteClass(const QString &clazz)
.. cpp:function:: void removePaletteClass(const QString &clazz)

   The list of palette classes modifies which colors are active from the palette.

.. cpp:function:: void grabKeyboard()
.. cpp:function:: void grabKeyboard(Private::ZMoFunc<void(QEvent*)> handler)
.. cpp:function:: void releaseKeyboard()

   The keyboard grab allows to temporarily override keyboard focus and redirect input to this widget or to a
   dedicated event handling closure.

.. cpp:function:: bool isAncestorOf(const ZWidget *child) const

   Returns ``true`` if ``child`` is an ancestor of this widget.
   If the widget is passed as ``child`` it returns :cpp:expr:`true` too.

.. cpp:function:: bool isEnabledTo(const ZWidget *ancestor) const

   Returns :cpp:expr:`true` if all widgets on the path from the widget to ``ancestor`` (excluding ``ancestor``, but
   including the widget) are locally enabled.

.. cpp:function:: bool isVisibleTo(const ZWidget *ancestor) const

   Returns :cpp:expr:`true` if all widgets on the path from the widget to ``ancestor`` (excluding ``ancestor``, but
   including the widget) are locally visible.

.. cpp:function:: bool focus() const

   Returns :cpp:expr:`true` if the widget has focus.

.. cpp:function:: bool isInFocusPath() const

   Returns :cpp:expr:`true` if the widget or any of its descendants has focus.

.. cpp:function:: QPoint mapFromTerminal(const QPoint &pos)
.. cpp:function:: QPoint mapToTerminal(const QPoint &pos)

   Map the point ``pos`` between local coordinates and terminal coordinates.

.. cpp:function:: ZWidget const *prevFocusable() const
.. cpp:function:: ZWidget *prevFocusable()
.. cpp:function:: ZWidget const *nextFocusable() const
.. cpp:function:: ZWidget *nextFocusable()

   Returns the next/previous widget in the focus order or :cpp:expr:`nullptr` if no such widget exists.

.. cpp:function:: const ZWidget *placeFocus(bool last = false) const
.. cpp:function:: ZWidget *placeFocus(bool last = false)

   Returns the first/last widget in the widget and its decendents that can take focus or :cpp:expr:`nullptr` of no such widget exists.

.. rst-class:: tw-virtual
.. cpp:function:: ZWidget *resolveSizeHintChain()

   Applications can override this function to customize which widgets are considered linked by chained layouts for
   layout cycles.

   The base implementation considers widgets to be linked to their parent if the parent has a layout set.

   Application should only need to override this if they implement automatic widget layouting without using layouts.

.. cpp:function:: void setCommandManager(ZCommandManager *cmd)
.. cpp:function:: ZCommandManager *commandManager() const

   The command manager associated with the widget is responsible for connection command and command notifiers.

.. cpp:function:: ZCommandManager *ensureCommandManager()

   If no command manager is associated with the widget, sets a new :cpp:class:`ZCommandManager` instance.

   It then returns the existing or newly created command manager.

.. rst-class:: tw-virtual
.. cpp:function:: QObject *facet(const QMetaObject &metaObject) const

   A facet is a way to add additionals interfaces to a widget without coupeling it to the widget's implementation class.
   This is used for example for window related behavior using the ZWindowFacet and
   for default controls using ZDefaultWidgetManager.

   It returns a reference to a class implementing the interface of ``metaObject``.

.. cpp:function:: template<typename T> T *findFacet() const

   Returns a facet from this widget or the nearest parent implementing it.

.. rst-class:: tw-virtual
.. cpp:function:: void paintEvent(Tui::ZPaintEvent *event)

   This event handler is used in widget implementations to handle the paint event.

   See :cpp:func:`Tui::ZEventType::paint()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void keyEvent(Tui::ZKeyEvent *event)

   This event handler is used in widget implementations to handle the key event.

   See :cpp:func:`Tui::ZEventType::key()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void pasteEvent(Tui::ZPasteEvent *event)

   This event handler is used in widget implementations to handle the paste event.

   See :cpp:func:`Tui::ZEventType::paste()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void focusInEvent(Tui::ZFocusEvent *event)

   This event handler is used in widget implementations to handle the focus in event.

   See :cpp:func:`Tui::ZEventType::focusIn()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void focusOutEvent(Tui::ZFocusEvent *event)

   This event handler is used in widget implementations to handle the focus out event.

   See :cpp:func:`Tui::ZEventType::focusOut()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void resizeEvent(Tui::ZResizeEvent *event)

   This event handler is used in widget implementations to handle the resize event.

   See :cpp:func:`Tui::ZEventType::resize()` for details.

.. rst-class:: tw-virtual
.. cpp:function:: void moveEvent(Tui::ZMoveEvent *event)

   This event handler is used in widget implementations to handle the move event.

   See :cpp:func:`Tui::ZEventType::move()` for details.

.. cpp:namespace:: NULL


Related Types
-------------


.. cpp:enum:: Tui::SizePolicy

   The enum describes how layout should allocate space to an item.

   .. cpp:enumerator:: Fixed

      The item should be be kept at the size indicated by the size hint.

   .. cpp:enumerator:: Minimum

      The size hint is the minimum acceptable size.

   .. cpp:enumerator:: Maximum

      The size hint is the maximum acceptable size.

   .. cpp:enumerator:: Preferred

      The item can shrink and expand.

   .. cpp:enumerator:: Expanding

      The item can shrink and expand.
      Items with this policy should be preferred over items with other policies when expanding.

.. cpp:enum:: Tui::FocusContainerMode

   If not ``None`` the widget with this mode is a focus container.

   .. cpp:enumerator:: None

      The widget is not a focus container.

   .. cpp:enumerator:: Cycle

      A widget with this mode will not pass focus to the parent widget or to its siblings.
      Use this for windows and dialogs.

   .. cpp:enumerator:: SubOrdering

      A widget with this mode will determine focus order locally using the focus order property.
      If all widgets inside this widgets are passed throught it will pass focus to the parent widget or to its siblings.
