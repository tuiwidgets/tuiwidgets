.. _ZBasicWindowFacet:

ZBasicWindowFacet
=================

.. rst-class:: tw-facet-impl
.. cpp:class:: Tui::ZBasicWindowFacet : public Tui::ZWindowFacet

   A basic implementation of :cpp:class:`Tui::ZWindowFacet`.

   This class implements more features for the :cpp:class:`Tui::ZWindowFacet` facet.

   It reimplements :cpp:func:`void Tui::ZWindowFacet::autoPlace(const QSize &available, ZWidget *self)` to allow
   more options for auto placement.
   The placement is controlled using :cpp:func:`void setDefaultPlacement(Tui::Alignment align, QPoint displace)`.

   .. cpp:function:: void setDefaultPlacement(Tui::Alignment align, QPoint displace)

      Sets the placement to use when not in manually placed mode.

      Aligenment options supported are combinations of one of either Tui::AlignLeft, Tui::AlignRight or Tui::AlignHCenter
      and one of either Tui::AlignTop, Tui::AlignBottom or Tui::AlignVCenter.

      Additionally add a displacement from the alignment by ``displace`` cells to the bottom-right.

   .. cpp:function:: void setExtendViewport(bool extend)

      Sets the return value of :cpp:func:`bool Tui::ZWindowFacet::isExtendViewport() const`.
