.. _ZTextOption:

ZTextOption
===========

.. cpp:class:: Tui::ZTextOption

   This class is copyable. It does define comparison operators for equality.

   This class contains options for use with :cpp:class:`Tui::ZTextLayout`.

   **Enums**

   .. rst-class:: tw-invisible
   .. cpp:type:: Flags = QFlags<Tui::ZTextOption::Flag>

   .. cpp:enum:: Flag

      .. cpp:enumerator:: ShowTabsAndSpaces

         Display tabs and spaces using characters ``→`` and ``·`` respectivly.

         Additionally the application can select a text style different from normal text to display these characters.

      .. cpp:enumerator:: ShowTabsAndSpacesWithColors

         Display tabs and trailing spaces using special colors.

         The choice of colors is done either by setting fixed colors in ZTextOption for these or by setting
         color mapping functions that return the used color based on the text style of the normal text.

   .. cpp:enum:: TabType

      .. cpp:enumerator:: LeftTab

         Left aligned tab.

         The text follows the tab stop position.

   .. cpp:enum:: WrapMode

      .. cpp:enumerator:: NoWrap

         Do not wrap lines.

      .. cpp:enumerator:: WordWrap

         Wrap lines between words.

      .. cpp:enumerator:: WrapAnywhere

         Wrap lines anywhere, even breaking words apart.

   **Types**

   .. cpp:class:: Tab

      Describes a tab stop.

      .. cpp:function:: Tab(int pos, Tui::ZTextOption::TabType tabType)

         Sets the position of the tab stop to ``pos`` and the type of the tab stop to ``tabType``.

      .. cpp:member:: int position = 0

         The position of the tab stop in cells.

      .. cpp:member:: Tui::ZTextOption::TabType type = LeftTab

         The type of the tab stop.

         Currently only left aligned tabs are supported.

   **Functions**

   .. cpp:function:: Tui::ZTextOption::Flags flags() const
   .. cpp:function:: void setFlags(Tui::ZTextOption::Flags flags)

      See :cpp:type:`Tui::ZTextOption::Flags` for possible flag values.

   .. cpp:function:: int tabStopDistance() const
   .. cpp:function:: void setTabStopDistance(int tabStopDistance)

      TODO

   .. cpp:function:: QList<Tui::ZTextOption::Tab> tabs() const
   .. cpp:function:: void setTabs(const QList<Tui::ZTextOption::Tab> &tabStops)

      TODO

   .. cpp:function:: QList<int> tabArray() const
   .. cpp:function:: void setTabArray(const QList<int> &tabStops)

      TODO

   .. cpp:function:: WrapMode wrapMode() const
   .. cpp:function:: void setWrapMode(WrapMode mode)

      TODO

   .. cpp:function:: void setTrailingWhitespaceColor(std::function<Tui::ZTextStyle(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)

      TODO

   .. cpp:function:: Tui::ZTextStyle mapTrailingWhitespaceColor(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const

      TODO

   .. cpp:function::  void setTabColor(std::function<Tui::ZTextStyle(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)

      TODO

   .. cpp:function:: Tui::ZTextStyle mapTabColor(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const

      TODO
