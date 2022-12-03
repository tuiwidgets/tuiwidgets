.. _ZButton:

ZButton
=======

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/button.tpi

     Button with text "Button"

* .. figure:: tpi/button-focus.tpi

     in focused state

* .. figure:: tpi/button-disabled.tpi

     in disabled state

* .. figure:: tpi/button-default.tpi

     as dialog default


A button allows users to trigger actions in the application and is a commonly used widget in dialog boxes.
A button can be activated by focusing it and pressing :kbd:`Enter` or if set by using a keyboard mnemonic.

In a dialog one button can be setup as default button that can also be activated by pressing :kbd:`Enter` in another
widget in the same dialog if that widget does not itself use the :kbd:`Enter` key.

The key to activate the button is usually shown highlighted when set using :ref:`markup <ControlMarkup>` to set its text.


Example
-------

.. literalinclude:: examples/widgets/button.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end


Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :widths: 33 67
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - :kbd:`Enter`
      - Activate the button

   *  - :kbd:`Space`
      - Activate the button

   *  - :kbd:`Alt` + (setup mnemonic)
      - Activate the button

   *  - (setup shortcut)
      - Activate the button


Also :kbd:`Enter` can be used from anywhere in a dialog if the button is a default button and the focused widget does
not suppress the usage of :kbd:`Enter` to activate the default widget.


Behavior
--------

Buttons by default accept focus, are one cell high and have a fixed vertical layout policy.
The size request of a button is the length of the text plus 6 cells plus the contents margins.

When a button has focus the default widget the dialog can not be activated using :kbd:`Enter`.


Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``button.fg``, ``button.bg``
      - Body of the button (active, **unfocused**, not default button)

   *  - ``button.default.fg``, ``button.default.bg``
      - Body of the button (active, unfocused, **default button**)

   *  - ``button.focused.fg``, ``button.focused.bg``
      - Body of the button (active, **focused**)

   *  - ``button.disabled.fg``, ``button.disabled.bg``
      - Body of the button (**disabled**)

   *  - ``button.shortcut.fg``, ``button.shortcut.bg``
      - Shortcut character in button text.

   *  -  ``control.fg``, ``control.bg``
      - Focus markers left and right of the button body.


ZButton
-------

.. cpp:class:: Tui::ZButton : public Tui::ZWidget

   A button widget.

   **Constructors**

   .. cpp:function:: explicit ZButton(const QString &text, Tui::ZWidget *parent=nullptr)
   .. cpp:function:: explicit ZButton(WithMarkupTag, const QString &markup, Tui::ZWidget *parent=nullptr)

      Create the |control| with the given ``text`` or ``markup``.

   **Functions**

   .. cpp:function:: QString text() const
   .. cpp:function:: void setText(const QString &t)

      Get or set the plain text content of the |control|.

      When set the shortcut is also reset.

      When the content of the |control| was most recently set using :cpp:func:`setMarkup` the returned text is empty.

   .. cpp:function:: QString markup() const
   .. cpp:function:: void setMarkup(const QString &m)

      Get or set the text content of the |control| using markup.

      When set the shortcut is also reset, if the markup contains a mnemonic it is setup as new shortcut.

      When the content of the |control| was most recently set using :cpp:func:`setText` the returned markup is empty.

   .. cpp:function:: void setShortcut(const Tui::ZKeySequence &key)

      Set the given ``key`` as shortcut for the |control|.

   .. cpp:function:: void setDefault(bool d)
   .. cpp:function:: bool isDefault() const

      If the button is the dialog default button is can be activated by :kbd:`Enter` even if another widget is focused.

      This is only available if the button is used in a dialog (or another widget that exposes the
      :cpp:class:`Tui::ZDefaultWidgetManager` facet.

   **Signals**

   .. cpp:function:: void clicked()

      This signal is emitted when the button is activated.

   **Slots**

   .. cpp:function:: void click()

      Focus and activate the |control|.

.. |control| replace:: button
