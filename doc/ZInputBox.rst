.. _ZInputBox:

ZInputBox
=========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/inputbox.tpi

     text input box with text "InputBox"

* .. figure:: tpi/inputbox-focus.tpi

     in focused state

* .. figure:: tpi/inputbox-disabled.tpi

     in disabled state

* .. figure:: tpi/inputbox-password.tpi

     as password input

ZInputBox is a single line text input widget.
If the input text is longer than the width of the widget it supports scrolling the text so the currently edited portion
of the text is visible.

Example
-------

.. literalinclude:: examples/widgets/inputbox.cpp
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
      - Result

   *  - (text input)
      - Insert text at cursor position.

   *  - :kbd:`Backspace`
      - Remove one letter/character before cursor position.

   *  - :kbd:`Del`
      - Remove one letter/character after cursor position.

   *  - :kbd:`←`
      - Move cursor position one letter/character to the left.

   *  - :kbd:`→`
      - Move cursor position one letter/character to the right.

   *  - :kbd:`Home`
      - Move cursor position to the start of the text.

   *  - :kbd:`End`
      - Move cursor position to the position just after the end of the text.

   *  - :kbd:`Insert`
      - Toggle overwrite mode.

   *  - | :kbd:`Ctrl+V`
        | :kbd:`Shift+Insert`
      - Inserts the current clipboard content at the cursor position.

Behavior
--------

Input boxes by default accept focus, are one cell high and have a expanding vertical layout policy.
The size request of a checkbox is fixed at 10 cells width to avoid the layout changing when text is entered.

The user can edit the text by using the keys described in the keyboard usage section.
Additionally the input box accepts text pasted into the terminal and inserts it at the cursor position.
On each change to the text the input box
emits a :cpp:func:`~void Tui::ZInputBox::textChanged(const QString &text)` signal.

When unfocused the text is always scrolled such that the start of the text is visible.
When focused the text is scrolled so that the cursor position is visible.

Newline (``\n``) characters in the text are displayed as in the following example (displaying ``one\ntwo``):

.. image:: tpi/inputbox-newline.tpi

Other non printable characters are displayed using similar visual conventions.

..
   TODO link to docs regarding this.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``lineedit.fg``, ``lineedit.bg``
      - Body of the |control| (active, **unfocused**)

   *  - ``lineedit.focused.fg``, ``lineedit.focused.bg``
      - Body of the |control| (active, **focused**)

   *  - ``lineedit.disabled.fg``, ``lineedit.disabled.bg``
      - Body of the |control| (**disabled**)


ZInputBox
---------

.. cpp:class:: Tui::ZInputBox : public Tui::ZWidget

   A single line text input widget.

   **Enums**

   .. cpp:enum:: EchoMode

      .. cpp:enumerator:: Normal

         Display text normally

      .. cpp:enumerator:: NoEcho

         Do not display any text and show cursor at start of the widget.
         This is the classic everything hidden password input.

      .. cpp:enumerator:: Password

         Display text masked by ``*``.
         This is the modern masked password input that leaks passwort length but provides better feedback and editing.

   **Constructors**

   .. cpp:function:: ZInputBox(const QString &text, Tui::ZWidget *parent=nullptr)

      Create the |control| with the given ``text``.

   **Functions**

   .. cpp:function:: QString text() const

      Get the text from the |control|.

   .. cpp:function:: void setEchoMode(Tui::ZInputBox::EchoMode echoMode)
   .. cpp:function:: Tui::ZInputBox::EchoMode echoMode() const

      The ``echoMode`` allows selecting how the input text is displayed.
      This allows selecting normal display or password input modes.

   .. cpp:function:: void setOverwriteMode(bool overwriteMode)
   .. cpp:function:: bool overwriteMode() const

      The ``overwriteMode`` allows for user input to overwrite existing text.

      If :cpp:expr:`true` user input letters will overwrite the existing letter the cursor is placed on.
      Otherwise it will be inserted after the cursor.

   .. cpp:function:: int cursorPosition() const
   .. cpp:function:: void setCursorPosition(int pos)

      The cursor position in code units.

      If the cursor position is out of range it will be adjusted to be in the range from 0 to the length of the current
      text (in code units).
      The cursor position is conceptually between letters/characters (clusters).

      If the given cursor position is not at the start of a cluster (or at the end of the text) in text,
      it will be adjusted to the preceding valid cursor position.

   .. cpp:function:: void insertAtCursorPosition(const QString &text)

      Insert the text given in ``text`` at the current cursor position and
      emits :cpp:func:`~void Tui::ZInputBox::textChanged(const QString &text)`.

   **Signals**

   .. cpp:function:: void textChanged(const QString &text)

      This signal is emitted when the user changes the text or the text is changed using functions on this widget.

   **Slots**

   .. cpp:function:: void setText(const QString &text)

      If ``text`` differs from the current text:
      Replace the text with ``text`` and emits :cpp:func:`~void Tui::ZInputBox::textChanged(const QString &text)`.
      Sets cursor position to the end of the text.

.. |control| replace:: input box
