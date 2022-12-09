.. _ZWindow:

ZWindow
=======

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/window-title-focus.tpi

     Window with title "WindowTitle"

* .. figure:: tpi/window-title.tpi

     without focus

* .. figure:: tpi/window-options.tpi

     with open system menu

* .. figure:: tpi/window-move.tpi

     during interactive move

The window is a basic user interface element that groups widgets together for the user.

Windows generally have a title that describes what the window contains.
The title could be a file name or similar location or a title for an action, etc.

Currently windows are the only parts of Tui Widgets that have a predefinied palette class that assigns palette colors
for the widgets typically used in dialogs and other windows (classes "window" and "dialog").
Thus make sure to place widgets into a window that are not explicitly designed to work outside windows.

If the default look of the window does not suit your application, not setting a title, disabling all border edges and
possibly overriding the palette colors for the window will yield a window like widget that does not enforce any
particular look.

Windows enable keyboard navigation among the widgets inside the window using :kbd:`Tab` and :kbd:`Shift+Tab`.
This navigation is based first on the value of :cpp:func:`int Tui::ZWidget::focusOrder() const` (higher values come later in tab order)
and second on the stacking layer of the widget (higher stacked/later added widgets come later in the tab order).

When enabled using :cpp:func:`setOptions(Tui::ZWindow::Options options) <void Tui::ZWindow::setOptions(Tui::ZWindow::Options options)>`
or by overriding :cpp:func:`~QVector<ZMenuItem> Tui::ZWindow::systemMenu()` the window has a associated menu
-- called the system menu -- that can be shown using :kbd:`Alt+-`.

Borders
-------

It is configurable which edges show border decorations using
:cpp:func:`setBorderEdges(Tui::Edges borders) <void Tui::ZWindow::setBorderEdges(Tui::Edges borders)>`.
The close indicator and the title are only visible if the top edge border is enabled.


.. _ZWindow_close_requests:

Close Requests
--------------

Often applications offer the user the ability to close a window, but in certain states need to confirm the close
operation with some interaction with the user (e.g. ask if the modified contents should be saved).

For this the window defines a protocol, so that generic code in Tui Widgets and libraries can fullfil these requirements.
A request to close a window is started using :cpp:func:`~void Tui::ZWindow::close()` which then sends an
:cpp:func:`Tui::ZEventType::close()` event to the window widget.
This event can be handled in a derived class by overriding
:cpp:func:`void Tui::ZWindow::closeEvent(Tui::ZCloseEvent *event)` or by other components in the application by
installing an event filter.

Each reason that requires user interaction to decide if the window should be closed is called a "check".

If, while handling the close event, any component detects a situation where the window close should be rejected or a
interaction with the user is needed, it can reject the event and display suitable prompts.

If the user answers the prompt in a way that indicates that the windows should be closed even if the check triggers,
the application starts a new close request now using :cpp:func:`~void Tui::ZWindow::closeSkipCheck(QStringList skipChecks)`
and passes the check that the user just answered as check to skip.

When processing the close event for the new request the component can now skip the already finished user prompt by no
longer rejecting the close request.
In applications that have more than one check for any given window, the code implementing the prompt must arrange to
preserve skipped checks from the event to the newly started close request.

Placement and Containers
------------------------

Windows support automatic placement (e.g. centered).
The automatic placement is mostly controlled by the :ref:`window facet <ZWindowFacet>`.
Windows start with manual placement enabled which disables automatic placement.

When enabled, automatic placement does simple automatic placement of the window based on the geometry of the parent
widget.
The automatic placement uses alignment like ``AlignLeft`` or ``AlignVCenter``, etc for placement.
Further the position can be adjusted by a displacement.
For example using ``AlignVCenter | AlignBottom`` with a displacement of :cpp:expr:`(0, -2)` will place the window so
that is is placed on the center of the bottom of the parent widget with 2 cells between the bottom of the window and
the bottom of the parent widget.

If more complex placement is needed, the window have a :ref:`window container <ZWindowContainer>` that will then be
responsible for placement of the window.
This allows e.g. tiling window layouts in a specific part of the parent widget to be realized.


Example
-------

.. literalinclude:: examples/widgets/window.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:

.. _ZWindow_keys:

Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - State
      - Result

   *  - :kbd:`Tab`
      - normal
      - Activate next widget in the window in tab order.

   *  - :kbd:`Shift+Tab`
      - normal
      - Activate previous widget in the window in tab order.

   *  - :kbd:`Alt+-`
      - normal
      - Pop up system menu.

   *  - :kbd:`←`
      - interactive **move**
      - Move window one cell to the left.

   *  - :kbd:`→`
      - interactive **move**
      - Move window one cell to the right.

   *  - :kbd:`↑`
      - interactive **move**
      - Move window one cell up.

   *  - :kbd:`↓`
      - interactive **move**
      - Move window one cell down.

   *  - :kbd:`←`
      - interactive **resize**
      - Reduce width of the window by one cell.
        (minimum 3 cells width)

   *  - :kbd:`→`
      - interactive **resize**
      - Extend width of the window by one cell.

   *  - :kbd:`↑`
      - interactive **resize**
      - Reduce height of the window by one cell.
        (minimum 3 cells height)

   *  - :kbd:`↓`
      - interactive **resize**
      - Extend height of the window by one cell.

   *  - :kbd:`Enter`
      - | interactive **move**
        | interactive **resize**
      - Confirm current position/size and leave interactive mode.

   *  - :kbd:`Esc`
      - | interactive **move**
        | interactive **resize**
      - Cancel interactive mode and reset to previous size/postion.


Additional related keys are :kbd:`F6` and :kbd:`Shift+F6`, to switch between windows, which are handled by :ref:`ZRoot`.


Behavior
--------

The window by default is not focusable and defines a focus mode of ``Cycle``. It has a palette class of ``window`` and
expanding size policies in both directions.

The size hint of the window is based on the size hint of its layout plus its contents margins and its enabled borders.
The layout area excludes the borders, if placement inside the borders (e.g. scrollbars or status indicators) is needed
the :cpp:class:`Tui::ZWindowLayout` offers special handling to enable that.

The window registers window local commands related to possible system menu options.
The commands are always registered even if the system menu options are not enabled using
:cpp:func:`~void Tui::ZWindow::setOptions(Tui::ZWindow::Options options)`.

The window implements a :cpp:class:`Tui::ZWindowFacet` which defines its characteristics as window.
The returned facet is derived from :cpp:class:`Tui::ZBasicWindowFacet`.
The window starts in manually placed mode.

The window also offers interactive move and resize modes.
These modes grab the keyboard and offer additional keyboard bindings to the user while active.


.. rst-class:: tw-float-right
.. image:: tpi/window-reducedcharset.tpi

The used characters in the rendering of the window differs based on the detected capabilities of the terminal.
With terminals that only support a very limited font repertoire a simplified drawing is used.

.. raw:: html

   <div style="clear: right;">

If the window is resized, reparented or shown and it is currently automatically placed it will refresh its placement.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - | ``window.frame.unfocused.fg``,
        | ``window.frame.unfocused.bg``
      - Body of the window and most of the frame (**unfocused**)

   *  - | ``window.frame.focused.fg``,
        | ``window.frame.focused.bg``
      - Body of the window and most of the frame (**focused**)

   *  - | ``window.frame.focused.control.fg``,
        | ``window.frame.focused.control.bg``
      - | Body of the close indicator (**focused**)
        | Also this color is used for the frame in interactive move and resize modes.


Comands
-------

The window registers the following window local commands:

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Command
      - Usage

   *  - ``ZWindowInteractiveMove``
      - Start interactive move mode.

   *  - ``ZWindowInteractiveResize``
      - Start interactive resize mode.

   *  - ``ZWindowAutomaticPlacement``
      - Set window to automatic placement.

   *  - ``ZWindowClose``
      - Request window close.


ZWindow
-------

.. cpp:class:: Tui::ZWindow : public Tui::ZWidget

   A window widget.

   **Enums**

   .. rst-class:: tw-invisible
   .. cpp:type:: Options = QFlags<Tui::ZWindow::Option>


   .. cpp:enum:: Option

      .. cpp:enumerator:: CloseOption

         Show close indicator and offer close option in system menu.

      .. cpp:enumerator:: MoveOption

         Show move option in system menu.

      .. cpp:enumerator:: ResizeOption

         Show resize option in system menu.

      .. cpp:enumerator:: AutomaticOption

         Show option "Automatic" to position to automatic placement in system menu.

      .. cpp:enumerator:: ContainerOptions

         Show menu options from associated :cpp:class:`Tui::ZWindowContainer` in system menu.

      .. cpp:enumerator:: DeleteOnClose

         Delete the window widget using :cpp:func:`QObject::deleteLater` when window is closed.

   **Constructors**

   .. cpp:function:: ZWindow(const QString &title, Tui::ZWidget *parent=nullptr)

      Construct the window with the title ``title``.

   **Functions**

   .. cpp:function:: QString windowTitle() const

      Return the current title of the window.

   .. cpp:function:: Tui::ZWindow::Options options() const
   .. cpp:function:: void setOptions(Tui::ZWindow::Options options)

      Options influence the appearance and functionality of the window.

      See :cpp:enum:`Tui::ZWindow::Option` for the possible options.

   .. cpp:function:: Tui::Edges borderEdges() const
   .. cpp:function:: void setBorderEdges(Tui::Edges borders)

      Select which of the window's edges should have a frame.

      Defaults to all edges.

   .. cpp:function:: void setDefaultPlacement(Tui::Alignment align, QPoint displace = {0, 0})

      Set the placement for automatic positioning.

      Alignment options supported are combinations of one of either Tui::AlignLeft, Tui::AlignRight or Tui::AlignHCenter
      and one of either Tui::AlignTop, Tui::AlignBottom or Tui::AlignVCenter.

      Additionally add a displacement from the alignment by ``displace`` cells to the bottom-right.

      This is only supported when the window facet is not overridden otherwise it will produce a warning.

   .. cpp:function:: void setAutomaticPlacement()

      Set the placement to automatic and ensure automatic placement was done.

   **Signals**

   .. cpp:function:: void windowTitleChanged(const QString &title)

      This signal is emitted when the window title is changed.

   **Slots**

   .. cpp:function:: void setWindowTitle(const QString &title)

      Set the window's title to ``title``.

   .. cpp:function:: bool showSystemMenu()

      Show the system menu.

      Shows the system menu if it contains any item.

   .. cpp:function:: void startInteractiveMove()

      Start interactive move mode.

      In the interactive move mode the user can move the window using the arrow keys and confirm with :kbd:`Enter` and
      cancel using :kbd:`Esc`.

   .. cpp:function:: void startInteractiveResize()

      Start interactive resize mode.

      In the interactive resize mode the user can resize the window using the arrow keys and confirm with :kbd:`Enter`
      and cancel using :kbd:`Esc`.

   .. cpp:function:: void close()

      Request the window to close.

      The close request will send a :cpp:func:`Tui::ZEventType::close()` event to itself and, if no event handler
      rejects this event, will close the window.
      The close event is sent without any checks to skip set.

      If the window is closed the window is set to invisible.
      If the :cpp:enumerator:`Tui::ZWindow::Option::DeleteOnClose` option is set, the windows is also deleted using
      :cpp:func:`QObject::deleteLater`.

   .. cpp:function:: void closeSkipCheck(QStringList skipChecks)

      Request the window to close, skipping the checks in ``skipChecks``.

      The close request will send a :cpp:func:`Tui::ZEventType::close()` event to itself and, if no event handler
      rejects this event, will close the window.
      The close event that is sent uses ``skipChecks`` as checks to skip.

      If the window is closed the window is set to invisible.
      If the :cpp:enumerator:`Tui::ZWindow::Option::DeleteOnClose` option is set, the window is also deleted using
      :cpp:func:`QObject::deleteLater`.

   **Protected Functions**

   .. rst-class:: tw-virtual
   .. cpp:function:: void closeEvent(Tui::ZCloseEvent *event)

      Applications can override this function to (temporarily) reject close requests.

      The :cpp:func:`Tui::ZEventType::close()` event is generated with :cpp:func:`QEvent::isAccepted()` set
      to :cpp:expr:`true` and the event handler can reject the close request by rejecting the event using
      :cpp:expr:`event->setAccepted(false)`.
      The code must not reset a already rejected event to accepted.

      The base implementation does not reject any close requests.

   .. rst-class:: tw-virtual
   .. cpp:function:: QVector<ZMenuItem> systemMenu()

      Applications can override this function to modify the items show on the system menu.

      The base implementation adds menu items based on the enabled options.
