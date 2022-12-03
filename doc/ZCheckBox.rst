.. _ZCheckBox:

ZCheckBox
=========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/checkbox.tpi

     Checkbox with text "CheckBox"

* .. figure:: tpi/checkbox-focus.tpi

     in focused state

* .. figure:: tpi/checkbox-disabled.tpi

     in disabled state


A checkbox allows users to select or deselect an option.
In contrast to :ref:`radiobuttons <ZRadioButton>` each checkbox has an independent state.
The state can be toggled by using :kbd:`Space` or if set by using a keyboard mnemonic.

The key to toggle the checkbox is usually shown highlighted when set using :ref:`markup <ControlMarkup>` to set its text.

A checkbox is either checked or unchecked.
If the tristate attribute is enabled it additionally supports a third partially checked state.
The partially checked state is commonly used when a checkbox represents a summary of multiple states and it's
represented states contain both checked and unchecked values.
A checkbox shown as partially checked by convention indicates that none of the represented substates will be altered.

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/checkbox-checked.tpi

     checked state

* .. figure:: tpi/checkbox-partially-checked.tpi

     partially checked state


Example
-------

.. literalinclude:: examples/widgets/checkbox.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
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

Checkboxes by default accept focus, are one cell high and have a expanding vertical layout policy.
The size request of a checkbox is the length of the text plus 5 cells plus the contents margins.

When the user toggles the state of the checkbox (e.g. using the :kbd:`Space` key) the state advances to the next state
(depending on the tristate mode either between :cpp:enumerator:`Tui::Unchecked` and :cpp:enumerator:`Tui::Checked` or
cycling through the states :cpp:enumerator:`Tui::Unchecked`, :cpp:enumerator:`Tui::Checked` and :cpp:enumerator:`Tui::PartiallyChecked`)
and emits a :cpp:func:`~Tui::ZCheckBox::stateChanged` signal.

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

ZCheckBox
---------

.. cpp:class:: Tui::ZCheckBox : public Tui::ZWidget

   A checkbox widget.

   **Constructors**

   .. cpp:function:: ZCheckBox(const QString &text, Tui::ZWidget *parent=nullptr);
   .. cpp:function:: ZCheckBox(WithMarkupTag, const QString &markup, Tui::ZWidget *parent=nullptr);

      Create the |control| with the given ``text`` or ``markup``.

   **Functions**

   .. cpp:function:: QString text() const;
   .. cpp:function:: void setText(const QString &text);

      Get or set the plain text content of the |control|.

      When set the shortcut is also reset.

      When the content of the |control| was most recently set using :cpp:func:`setMarkup` the returned text is empty.

   .. cpp:function:: QString markup() const;
   .. cpp:function:: void setMarkup(const QString &markup);

      Get or set the text content of the |control| using markup.

      When set the shortcut is also reset, if the markup contains a mnemonic it is setup as new shortcut.

      When the content of the |control| was most recently set using :cpp:func:`setText` the returned markup is empty.

   .. cpp:function:: Tui::CheckState checkState() const;
   .. cpp:function:: void setCheckState(Tui::CheckState state);

      The ``checkState`` is the displayed state of the checkbox.
      It can be :cpp:enumerator:`Tui::Unchecked`, :cpp:enumerator:`Tui::Checked` or :cpp:enumerator:`Tui::PartiallyChecked`.

      Using the setter will not cause :cpp:func:`Tui::ZCheckBox::stateChanged` to be emitted.

   .. cpp:function:: bool isTristate() const;
   .. cpp:function:: void setTristate(bool tristate = true);

      If the tristate attribute is :cpp:expr:`false` the checkbox alternates between the states :cpp:enumerator:`Tui::Unchecked` and
      :cpp:enumerator:`Tui::Checked`.

      If tristate is enabled it cycles through the states :cpp:enumerator:`Tui::Unchecked`,
      :cpp:enumerator:`Tui::Checked` and :cpp:enumerator:`Tui::PartiallyChecked`.

   .. cpp:function:: void setShortcut(const Tui::ZKeySequence &key);

      Set the given ``key`` as shortcut for the |control|.

   **Signals**

   .. cpp:function:: void stateChanged(Tui::CheckState state);

      This signal is emitted when the user changes the state or the state is changed through the
      :cpp:func:`void Tui::ZCheckBox::toggle()` function.

      The new state is passed in the ``state`` parameter.

   **Slots**

   .. cpp:function:: void toggle()

      Toggle through the states of the checkbox and emit :cpp:func:`Tui::ZCheckBox::stateChanged`.

   .. cpp:function:: void click()

      If the checkbox is enabled, focus the checkbox and toggle through its states
      and emit :cpp:func:`Tui::ZCheckBox::stateChanged`.

      If the checkbox is disabled, does nothing.


.. |control| replace:: checkbox
