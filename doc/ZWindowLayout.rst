.. _ZWindowLayout:

ZWindowLayout
=============

.. cpp:class:: Tui::ZWindowLayout : public Tui::ZLayout

   A special layout to place widgets (like scroll bars or status indicators) over the borders of a
   :cpp:class:`Tui::ZWindow` or derived classes.

   .. cpp:function:: void setCentralWidget(ZWidget *w)
   .. cpp:function:: void setCentral(ZLayoutItem *li)

      The central widget or layout is placed in the normal layout area.

   .. cpp:function:: void setRightBorderWidget(ZWidget *w, Tui::Alignment align = Tui::Alignment())
   .. cpp:function:: void setBottomBorderWidget(ZWidget *w, Tui::Alignment align = Tui::Alignment())
   .. cpp:function:: void setTopBorderWidget(ZWidget *w, Tui::Alignment align = Tui::AlignHCenter)

      The border widgets are placed over the window border.
      The alignment ``align`` allows to align the widget either centered, at the start or at the end.

   .. cpp:function:: int topBorderLeftAdjust() const
   .. cpp:function:: void setTopBorderLeftAdjust(int topBorderLeftAdjust)

      The ``topBorderLeftAdjust`` allows fine adjustment of the top widgets left side placement.

   .. cpp:function:: int topBorderRightAdjust() const
   .. cpp:function:: void setTopBorderRightAdjust(int topBorderRightAdjust)

      The ``topBorderRightAdjust`` allows fine adjustment of the top widgets right side placement.

   .. cpp:function:: int rightBorderTopAdjust() const
   .. cpp:function:: void setRightBorderTopAdjust(int rightBorderTopAdjust)

      The ``rightBorderTopAdjust`` allows fine adjustment of the right widgets top side placement.

   .. cpp:function:: int rightBorderBottomAdjust() const
   .. cpp:function:: void setRightBorderBottomAdjust(int rightBorderBottomAdjust)

      The ``rightBorderBottomAdjust`` allows fine adjustment of the right widgets bottom side placement.

   .. cpp:function:: int bottomBorderLeftAdjust() const
   .. cpp:function:: void setBottomBorderLeftAdjust(int bottomBorderLeftAdjust)

      The ``bottomBorderLeftAdjust`` allows fine adjustment of the bottom widgets left side placement.

   .. cpp:function:: int bottomBorderRightAdjust() const
   .. cpp:function:: void setBottomBorderRightAdjust(int bottomBorderRightAdjust)

      The ``bottomBorderRightAdjust`` allows fine adjustment of the bottom widgets right side placement.
