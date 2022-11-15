.. _ZBasicDefaultWindowManager:

ZBasicDefaultWindowManager
==========================

.. rst-class:: tw-facet-impl
.. cpp:class:: Tui::ZBasicDefaultWindowManager : public Tui::ZDefaultWidgetManager

   This class is a basic implementation of :cpp:class:`Tui::ZDefaultWidgetManager`.

   For a given :cpp:class:`Tui::ZWidget` -- the dialog -- passed in the constructor,
   it stores one decendent of the dialog as default widget.

   :cpp:func:`bool Tui::ZDefaultWidgetManager::isDefaultWidgetActive() const` is reimplemented to
   return true iff there is a default widget and the widgets in the path of the currently focused
   widget to the dialog don't accept :kbd:`Enter`.

   If a widget accepts :kbd:`Enter` is determined by sending a ZEventType::queryAcceptsEnter event
   to it and it the event is accepted the widget is assumed to accept :kbd:`Enter` and thus the default
   window is not active.

