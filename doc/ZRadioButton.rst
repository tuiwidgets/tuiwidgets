.. _ZRadioButton:

ZRadioButton
============

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/radiobutton.tpi

     A group of radiobuttons |br| with "Red" currently selected

* .. figure:: tpi/radiobutton-focus.tpi

     in focused state

* .. figure:: tpi/radiobutton-disabled.tpi

     in disabled state


A set of radiobuttons allows users to select from a set of options presented as multiple widgets.
In contrast to :ref:`checkboxes <ZCheckBox>` a group of radiobuttons together represents a choice of options.

The state can be changed by using :kbd:`Space` or if set by using a keyboard mnemonic.

The key to toggle the checkbox is usually shown highlighted when set using :ref:`markup <ControlMarkup>` to set its text.

In a group of radiobuttons only one button can be checked at a time.
If another button is checked the button currently checked will get unchecked automatically.

All radiobuttons sharing a common parent are one radiobutton group.


Example
-------

.. literalinclude:: examples/widgets/radiobuttons.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:

With two groups:

.. literalinclude:: examples/widgets/radiobuttons.cpp
    :start-after: // snippet-2group-start
    :end-before:  // snippet-2group-end
    :dedent:


Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :widths: 33 67
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - :kbd:`Space`
      - Activate the button

   *  - :kbd:`Alt` + (setup mnemonic)
      - Activate the button

   *  - (setup shortcut)
      - Activate the button


Behavior
--------

Radiobuttons by default accept focus, are one cell high and have a expanding vertical layout policy.
The size request of a radiobutton is the length of the text plus 5 cells plus the contents margins.

When the user checks a radiobutton (e.g. using the :kbd:`Space` key) all other radiobuttons in the group are unchecked
and it emits a :cpp:func:`~Tui::ZRadioButton::toggled` signal on all radiobuttons in the group.

It is not possible to uncheck all radiobuttons in a group using the keyboard, but it is possible for the application to
set all radiobuttons to unchecked.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``control.fg``, ``control.bg``
      - Body of the |control| (active, **unfocused**)

   *  - ``control.focused.fg``, ``control.focused.bg``
      - Body of the |control| (active, **focused**)

   *  - ``control.disabled.fg``, ``control.disabled.bg``
      - Body of the |control| (**disabled**)

   *  - ``control.shortcut.fg``, ``control.shortcut.bg``
      - Shortcut character in |control| text.


ZRadioButton
------------

.. cpp:class:: Tui::ZRadioButton : public Tui::ZWidget

   A radiobutton widget.

   **Constructors**

   .. cpp:function:: ZRadioButton(const QString &text, Tui::ZWidget *parent = nullptr)
   .. cpp:function:: ZRadioButton(WithMarkupTag, const QString &markup, Tui::ZWidget *parent = nullptr)

      Create the |control| with the given ``text`` or ``markup``.

   **Functions**

   .. cpp:function:: QString text() const
   .. cpp:function:: void setText(const QString &text)

      Get or set the plain text content of the |control|.

      When set the shortcut is also reset.

      When the content of the |control| was most recently set using :cpp:func:`setMarkup` the returned text is empty.

   .. cpp:function:: QString markup() const
   .. cpp:function:: void setMarkup(const QString &markup)

      Get or set the text content of the |control| using markup.

      When set the shortcut is also reset, if the markup contains a mnemonic it is setup as new shortcut.

      When the content of the |control| was most recently set using :cpp:func:`setText` the returned markup is empty.

   .. cpp:function:: bool checked() const
   .. cpp:function:: void setChecked(bool state)

      The ``checked`` state is the displayed state of the radiobutton.
      Setting a radiobutton using this function does not update other radiobuttons in the group.

      To automatically disable the other radiobuttons in a group use :cpp:func:`void Tui::ZRadioButton::toggle()`.

   .. cpp:function:: void setShortcut(const Tui::ZKeySequence &key)

      Set the given ``key`` as shortcut for the |control|.

   **Signals**

   .. cpp:function:: void toggled(bool state)

      This signal is emitted when the user changes the state or the state is changed through the
      :cpp:func:`void Tui::ZRadioButton::toggle()` function.

      The new state is passed in the ``state`` parameter.

   **Slots**

   .. cpp:function:: void click()

      Set the |control| to the checked state and emit :cpp:func:`~void Tui::ZRadioButton::toggled(bool state)`.
      All other radiobuttons in the group will be set to unchecked and emit :cpp:func:`~void Tui::ZRadioButton::toggled(bool state)`.

   .. cpp:function:: void toggle()

      If the checkbox is enabled, focus the checkbox set it to the checked state and
      emit :cpp:func:`~void Tui::ZRadioButton::toggled(bool state)`.
      All other radiobuttons in the group will be set to unchecked and emit :cpp:func:`~void Tui::ZRadioButton::toggled(bool state)`.

      If the checkbox is disabled, does nothing.

.. |control| replace:: radiobutton
.. |br| raw:: html

  <br/>
