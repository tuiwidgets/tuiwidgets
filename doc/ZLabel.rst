.. _ZLabel:

ZLabel
======

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/label.tpi

     Checkbox with text "CheckBox"

* .. figure:: tpi/label-focus.tpi

     reflecting focus of "buddy" widget

* .. figure:: tpi/label-disabled.tpi

     in disabled state

* .. figure:: tpi/label-color.tpi

     with custom palette

* .. figure:: tpi/label-buddy.tpi

     with "buddy" input box


Labels display user interface text in a dialog.
They are often used to label other widgets and if setup with a "buddy" widget also serve to show focus for that widget.
For example :cpp:class:`Tui::ZInputBox` and :cpp:class:`Tui::ZListView` do not display focus indication themselves and can be
augmented with a label to show focus.

In contract to a label the :cpp:class:`Tui::ZTextLine` does not reserve a cell as focus indicator and doesn't have
support for a "buddy" widget or showing a keyboard shortcut.

See also: :cpp:class:`Tui::ZTextLine`

Example
-------

.. literalinclude:: examples/widgets/label.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end


Keyboard Usage
--------------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - :kbd:`Alt` + (setup mnemonic)
      - Focus the label's "buddy" widget.


Behavior
--------

Labels by default don't accept focus, are one cell high and have a preferred vertical layout policy.
The size request of a label is the length of the text plus 1 cells plus the contents margins.

The effective enabled/disabled status for its visual appearance is the logical-AND combination of both its own status and
the "buddy" widget's status (if a buddy is set).

For its visual appearance the relevant focus status is that of its "buddy" widget, if there is no "buddy" widget, the
label will always appear as unfocused.

The shortcut from the label's markup is used to focus the "buddy" widget.

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


ZLabel
------

.. cpp:class:: Tui::ZLabel : public Tui::ZWidget

   A label widget.

   **Constructors**

   .. cpp:function:: ZLabel(const QString &text, Tui::ZWidget *parent = nullptr)
   .. cpp:function:: ZLabel(WithMarkupTag, const QString &markup, Tui::ZWidget *parent = nullptr)

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

   .. cpp:function:: Tui::ZWidget *buddy() const
   .. cpp:function:: void setBuddy(Tui::ZWidget *buddy)

      The "buddy" of a label is a related widget for which the label displays the focus and which inputting the
      shortcut of the label will focus.

.. |control| replace:: label
