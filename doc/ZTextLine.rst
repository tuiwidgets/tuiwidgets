.. _ZTextLine:

ZTextLine
=========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/textline.tpi

     text line with text "TextLine"

* .. figure:: tpi/textline-disabled.tpi

     in disabled state

* .. figure:: tpi/textline-color.tpi

     with custom palette


ZTextLine is a widget that simply displays a single line of text.

In contrast to :ref:`labels <ZLabel>` it does not reserve a cell as focus indicator and does not support forwarding
focus to a "buddy" widget nor does it support showing a keyboard shortcut.

See also: :cpp:class:`Tui::ZLabel`, :cpp:class:`Tui::ZTextLayout`

Example
-------

.. literalinclude:: examples/widgets/textline.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:


Keyboard Usage
--------------

This widget does not react to any keyboard input.


Behavior
--------

Labels by default don't accept focus, are one cell high and have a expanding vertical layout policy.
The size request of a label is the length of the text plus the contents margins.

The widget does not have visual distinct disabled or focused states.

Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``control.fg``, ``control.bg``
      - Body of the |control|


ZTextLine
---------

.. cpp:class:: Tui::ZTextLine : public Tui::ZWidget

   A simple text label widget without "buddy" widget support.

   **Constructors**

   .. cpp:function:: ZTextLine(const QString &text, Tui::ZWidget *parent = nullptr)
   .. cpp:function:: ZTextLine(WithMarkupTag, const QString &markup, Tui::ZWidget *parent = nullptr)

      Create the |control| with the given ``text`` or ``markup``.

   **Functions**

   .. cpp:function:: QString text() const
   .. cpp:function:: void setText(const QString &text)

      Get or set the plain text content of the |control|.

      When the content of the |control| was most recently set using :cpp:func:`setMarkup` the returned text is empty.

   .. cpp:function:: QString markup() const
   .. cpp:function:: void setMarkup(const QString &markup)

      Get or set the text content of the |control| using markup.

      When the content of the |control| was most recently set using :cpp:func:`setText` the returned markup is empty.


.. |control| replace:: text line widget
