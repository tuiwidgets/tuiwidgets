.. _ZClipboard:

ZClipboard
==========

ZClipboard is a simple text clipboard used for copy and paste.

It is typically used as a facet on the root widget.
:cpp:class:`Tui::ZRoot` returns an instance of this class in its 
:cpp:func:`facet(…) <virtual QObject *Tui::ZWidget::facet(const QMetaObject &metaObject) const>` implementation
when queried using this type.

Widgets can obtain an instance using ``findFacet<Tui::ZClipboard>()``.

ZClipboard
----------

.. cpp:class:: Tui::ZClipboard : public QObject

   A simple text clipboard.

   **Functions**

   .. cpp:function:: contents() const
   .. cpp:function:: setContents(const QString &contents)

      The contents of the clipboard is a string.

   .. cpp:function:: clear()

      Clear the contents of the clipboard.

   **Signals**

   .. cpp:function:: contentsChanged()

      This signal is emitted when the contents of the clipboard changes.
