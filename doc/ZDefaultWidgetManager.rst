.. _ZDefaultWidgetManager:

ZDefaultWidgetManager
=====================

In dialogs there is often a default widget that can be activated even if focus is on another widget using :kbd:`Enter`.
This facet of the dialog window widget is used to coordinate this.

Implementations of this facet stores a pointer to the default widget and implement logic to determine if the
default widget is active and will receive :kbd:`Enter` events or if the focused widget will handle the :kbd:`Enter` key.

See also: :cpp:class:`Tui::ZBasicDefaultWindowManager`

.. rst-class:: tw-facet
.. cpp:class:: Tui::ZDefaultWidgetManager : public QObject

   Policy facet for default widget handling.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: void setDefaultWidget(ZWidget *w)

      Saves the given widget ``w`` as the default widget for the dialog.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: ZWidget *defaultWidget() const

      Returns the current default widget of the dialog if any, or :cpp:expr:`nullptr`.

   .. rst-class:: tw-pure-virtual
   .. cpp:function:: bool isDefaultWidgetActive() const

      Determines if given the currently focused widget and possibly other state if the default widget will handle the
      :kbd:`Enter` key.
