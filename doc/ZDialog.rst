.. _ZDialog:

ZDialog
=======

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/dialog-windowtitle.tpi

     Dialog with title "WindowTitle"

* .. figure:: tpi/dialog-options.tpi

     with system menu


ZDialog is a widget derived from :cpp:class:`Tui::ZWindow` specialized for dialogs.

It adds the concept of rejecting the dialog and of a default widget.
It also uses the palette class ``dialog`` which yields a visually distinct appearance in the default palettes.

Default widget
--------------

In dialogs a widget can be the default widget.
This means that if the focused widget does not itself use the :kbd:`Enter` key the default widget will handle the
:kbd:`Enter` key.

An example of a widget that can act as a default widget is :cpp:class:`Tui::ZButton`.

For details on the exact rules see :ref:`ZBasicDefaultWindowManager`.


Example
-------

.. literalinclude:: examples/widgets/dialog.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:


Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - State
      - Result

   *  - :kbd:`Esc`
      - normal
      - Reject the dialog.

   *  - :kbd:`Enter`
      - normal
      - Trigger the default widget, if set and active.

For additional related keys see :ref:`Tui::ZWindow <ZWindow_keys>` and :ref:`Tui::ZRoot <ZRoot_keys>`.

Behavior
--------

Most behavior is inherited from ZWindow.

In addition ZDialog has a palette class of (``window``, ``dialog``) and defaults to stacking layer 1000.

The dialog will automatically resize itself and enable auto placement to center on certain conditions.
It does this only once per instance.
It checks after creation in the next main loop iteration and when the dialog is shown, if the dialog is visible and has
a parent widget the automatic size and positioning code triggers.

If the dialog already has a geometry nothing further is done and the auto sizing will not happen for the instance,
otherwise the dialog will be resized according to the value of :cpp:func:`QSize Tui::ZWidget::effectiveSizeHint() const`
and automatic placement in the center of the parent widget is enabled.

The dialog implements the :ref:`ZDefaultWidgetManager` facet using :ref:`ZBasicDefaultWindowManager` or a derived class.


ZDialog
-------

.. cpp:class:: Tui::ZDialog : public Tui::ZWindow

   A dialog window class.

   **Signals**

   .. cpp:function:: void rejected()

      This dialog is emitted when the dialog is rejected either using the :cpp:func:`~void Tui::ZDialog::reject()`
      function or by pressing :kbd:`Esc`.

   **Slots**

   .. cpp:function:: void reject()

      Hides the dialog and triggers emission of the :cpp:func:`~void Tui::ZDialog::rejected()` signal.
