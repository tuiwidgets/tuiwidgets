.. _ZRoot:

ZRoot
=====

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/root.tpi

     ZRoot

* .. figure:: tpi/root-fillchar.tpi

     with fill character


ZRoot is designed as the root widget in the widget tree.

It sets a default palette, clears the widget and handles switching between windows using :kbd:`F6` and :kbd:`Shift+F6`.

Commonly an application has a class derived from ZRoot to setup the user interface widget tree.
It is recommended to create the widgets in an overridden :cpp:func:`~void Tui::ZRoot::terminalChanged()` function, so
that all terminal auto detection is already finished when the widget tree is build.

Example
-------

.. literalinclude:: examples/widgets/root.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:

See :ref:`Getting_Started` for an example overriding :cpp:func:`~void Tui::ZRoot::terminalChanged()` to setup the
user interface.


.. _ZRoot_keys:

Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - State
      - Result

   *  - :kbd:`F6`
      - normal
      - Activate next widget in tab order

   *  - :kbd:`Shift+F6`
      - normal
      - Activate previous widget in tab order

Behavior
--------

ZRoot has by default a minimum size of :cpp:expr:`(40, 7)` to raise the chance that applications not tested with very
small terminals are still somewhat usable.
If an application needs a bigger size to be usable or wants to support smaller sizes it can override this to a suitable
value.

If the terminal is smaller than the minimum size of the root widget (as set in the terminal) the terminal will enable
a minimal viewport scrolling interface so that the root widget still renders at the minimum size and the user can view
parts of the root widget by using the keyboard to scroll the viewport.

See :ref:`term_viewport` for details.

If any child widget of ZRoot implements the :cpp:class:`Tui::ZWindowFacet` and its
:cpp:func:`~bool Tui::ZWindowFacet::isExtendViewport() const` returns :cpp:expr:`true` the minimal size for the root widget is
temporarily extended to include the whole child widget.
This enables selected dialogs and menus to be usable even if the terminal is smaller than the widget.

ZRoot by default has the :cpp:func:`Tui::ZPalette::classic()` palette set as palette.
This palette defines the colors used by the widgets in Tui Widgets.
The predefined palettes only enable palette colors for widgets that are typically used in windows for widgets that are
contained in a widget that has ``window`` as the widget's palette class such as :cpp:class:`Tui::ZWindow` or
:cpp:class:`Tui::ZDialog`.

If ZRoot is resized windows that are neither manually placed nor in a window container according to their
:cpp:class:`Tui::ZWindowFacet` facet implementation are placed using their window facet's auto place implementation.

The layout area of ZRoot is usually its geometry with the contents margins removed.
If the root widgets size is currently expanded due to a window using ``isExtendViewport`` the layout area will not
reflect the temporarily larger geometry due to this window.

ZRoot exposes :cpp:class:`Tui::ZClipboard` as a facet.
This facet is used as clipboard for copy and paste operations by other widgets in the application.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - | ``root.fg``,
        | ``root.bg``
      - The body of the widget.


ZRoot
-----

.. cpp:class:: Tui::ZRoot : public Tui::ZWidget

   A widget designed to be used as root of the widget tree.

   **Functions**

   .. cpp:function:: void setFillChar(int fillChar)
   .. cpp:function:: int fillChar() const

      The fill character is what character is used to clear the widgets background.

      It defaults to :cpp:var:`Tui::Erased`.

      This allows for more faithful emulation of the classic look.

   .. cpp:function:: void activateNextWindow()
   .. cpp:function:: void activatePreviousWindow()

      Activate the next/previous child window in the widget.

      Eligible child widgets to activate are widgets that are visible, implement the
      :cpp:class:`Tui::ZWindowFacet` facet and where `ZWidget *placeFocus(bool last)` returns a widget
      (not :cpp:expr:`nullptr`).

      After activation it will be raised using :cpp:func:`~void Tui::ZRoot::raiseOnActivate(ZWidget *w)`.

   .. rst-class:: tw-virtual
   .. cpp:function:: void raiseOnActivate(ZWidget *w)

      Override this function to customize if and how a window is raised when activated.

      The base implementation raises the window using :cpp:func:`void Tui::ZWidget::raise()`.

   **Protected Functions**

   .. rst-class:: tw-virtual
   .. cpp:function:: void terminalChanged()

      This function is called when the terminal changes.

      In applications that don't move the root widget between terminals, it will be called once after the terminal
      auto detection is finished.

      Applications should override this method to build their widget tree.

      The base implementation does nothing.
