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

      Definies tab positions beyond tabs explicitly specified using :cpp:func:`setTabs <void setTabs(const QList<Tui::ZTextOption::Tab> &tabStops)>`
      or :cpp:func:`setTabArray <void setTabArray(const QList<int> &tabStops)>`.

      Tab stops definied by this are only active after the last individually placed tab stop and are in columns
      ``i * tabStopDistance`` (for i > 0).

      Values of ``tabStopDistance`` less than 1 are invalid and replaced by the value 1.

      Defaults to 8.

   .. cpp:function:: QList<Tui::ZTextOption::Tab> tabs() const
   .. cpp:function:: void setTabs(const QList<Tui::ZTextOption::Tab> &tabStops)

      Set explicit tab stops ``tabStops``.

      On return the tab stops are always sorted.

   .. cpp:function:: QList<int> tabArray() const
   .. cpp:function:: void setTabArray(const QList<int> &tabStops)

      Set explicit left aligned tab stops at the columns in ``tabStops``.

      On return the tab stops are always sorted.

   .. cpp:function:: WrapMode wrapMode() const
   .. cpp:function:: void setWrapMode(WrapMode mode)

      Selects which line wrapping mode is used.

      See :cpp:type:`Tui::ZTextOption::WrapMode` for possible values.

   .. cpp:function:: void setTrailingWhitespaceColor(std::function<Tui::ZTextStyle(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)

      Sets the function used in :cpp:func:`mapTrailingWhitespaceColor <Tui::ZTextStyle mapTrailingWhitespaceColor(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const>`

   .. cpp:function:: Tui::ZTextStyle mapTrailingWhitespaceColor(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const

      Apply the mapping function set by :cpp:func:`setTrailingWhitespaceColor <void setTrailingWhitespaceColor(std::function<Tui::ZTextStyle(const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)>`.

      If the mapping function was not yet set returns :cpp:expr:`range ? range->format() : formatingChars`.

   .. cpp:function::  void setTabColor(std::function<Tui::ZTextStyle(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)

      Sets the function used in :cpp:func:`mapTabColor <Tui::ZTextStyle mapTabColor(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const>`

   .. cpp:function:: Tui::ZTextStyle mapTabColor(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange *range) const

      Apply the mapping function set by :cpp:func:`setTabColor <void setTabColor(std::function<Tui::ZTextStyle(int pos, int size, int hidden, const Tui::ZTextStyle &baseStyle, const Tui::ZTextStyle &formatingChars, const Tui::ZFormatRange*)> colorMapper)>`.

      If the mapping function was not yet set returns :cpp:expr:`range ? range->format() : formatingChars`.
