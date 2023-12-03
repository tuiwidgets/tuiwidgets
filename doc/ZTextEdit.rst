.. _ZTextEdit:

ZTextEdit
=========

.. rst-class:: tw-flex-imgs

* .. figure:: tpi/textedit.tpi

     multi line text edit

* .. figure:: tpi/textedit-focus.tpi

     in focused state

* .. figure:: tpi/textedit-focus-selection.tpi

     in focused state with selection

* .. figure:: tpi/textedit-disabled.tpi

     in disabled state

* .. figure:: tpi/textedit-selection.tpi

     in un-focused state with selection

ZInputBox is a multi-line text input widget.
If the input text is longer than the height of the widget or wider,
it supports scrolling the text so the currently edited portion of the text is visible.

The text in the text edit can come either from a internally managed instance of :cpp:class:`Tui::ZDocument` or from
a externally managed :cpp:class:`Tui::ZDocument` instance.
An externally managed document can be shared by multiple text edit widgets, in that case each widget has an independent
cursor, selection and scroll position.

For details about how positions and selections work see :ref:`using_document_cursors`.

Example
-------

.. literalinclude:: examples/widgets/textedit.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:


Keyboard Usage
--------------


.. list-table::
   :class: noborder firstcolnobreakkbd
   :align: left
   :header-rows: 1

   *  - Key
      - Result

   *  - (text input) :sup:`(r)`
      - Insert text at cursor position.
        If there is a current selection, the selected text is deleted before inserting the new text.

   *  - :kbd:`Enter` :sup:`(r)`
      - Insert a line break.
        If there is a current selection, the selected text is deleted before the line break is added.

   *  - :kbd:`Backspace` :sup:`(r)`
      - Remove one letter/character before the cursor position.
        If there is a current selection, the selected text is deleted instead.

   *  - :kbd:`Delete` :sup:`(r)`
      - Remove one letter/character after the cursor position.
        If there is a current selection, the selected text is deleted instead.

   *  - :kbd:`Ctrl+Backspace` :sup:`(r)`
      - Remove one word before the cursor position.
        If there is a current selection, the selected text is deleted instead.

   *  - :kbd:`Ctrl+Delete` :sup:`(r)`
      - Remove one word after the cursor position.
        If there is a current selection, the selected text is deleted instead.

   *  - :kbd:`Tab` :sup:`(r)`
      - Without selection inserts a tab at the current cursor position.
        The width and type (using \\t or spaces) is configurable.
        With a selection indents the selected lines.

   *  - :kbd:`Shift+Tab` :sup:`(r)`
      - With a selection dedents the selected lines.

   *  - :kbd:`Insert` :sup:`(r)`
      - Toggle overwrite mode.

   *  - | :kbd:`Ctrl+C`
        | :kbd:`Ctrl+Insert`
      - Copies the current selection to the clipboard.

   *  - | :kbd:`Ctrl+V` :sup:`(r)`
        | :kbd:`Shift+Insert` :sup:`(r)`
      - Inserts the current clipboard content at the cursor position.
        If there is a current selection, the selected text is deleted before inserting the new text.

   *  - | :kbd:`Ctrl+X` :sup:`(r)`
        | :kbd:`Shift+Delete` :sup:`(r)`
      - Copies the selected text to the clipboard and removes it.

   *  - :kbd:`←`
      - Move cursor position one letter/character to the left. :sup:`(s)`

   *  - :kbd:`→`
      - Move cursor position one letter/character to the right. :sup:`(s)`

   *  - :kbd:`↓`
      - Move the cursor one visual line down. :sup:`(s)`

   *  - :kbd:`↑`
      - Move the cursor one visual line up. :sup:`(s)`

   *  - :kbd:`Home`
      - Move cursor position to the start of the current line. :sup:`(s)`

   *  - :kbd:`End`
      - Move cursor position to the position just after the end of the current line. :sup:`(s)`

   *  - :kbd:`Ctrl+←`
      - Move cursor position one word to the left. :sup:`(s)`

   *  - :kbd:`Ctrl+→`
      - Move cursor position one word to the right. :sup:`(s)`

   *  - :kbd:`Ctrl+Home`
      - Move cursor position to the start of the text. :sup:`(s)`

   *  - :kbd:`Ctrl+End`
      - Move cursor position to the end of the text. :sup:`(s)`

   *  - :kbd:`Page Up`
      - Move cursor position one page up. :sup:`(s)`

   *  - :kbd:`Page Down`
      - Move cursor position one page down. :sup:`(s)`

   *  - :kbd:`Ctrl+A`
      - Selects the whole text.

   *  - :kbd:`F4`
      - Toggles select mode.
        In select mode the selection is extended without the need to hold down :kbd:`Shift`.
        This can be helpful with terminals that don't report some key combinations with :kbd:`Shift`.

   *  - :kbd:`Ctrl+Z` :sup:`(r)(u)`
      - Undo the previous edit step.

   *  - :kbd:`Ctrl+Y` :sup:`(r)(u)`
      - Redo the previous edit step.


| (r) Read-only mode will disable this key.
| (s) With :kbd:`Shift` or when select mode is active, extends the selection.
| (u) If isUndoRedoEnabled is set to :cpp:expr:`false` this key will be disabled.

Behavior
--------

Text edit widgets by default accept focus, don't have a minimum size and have a expanding vertical and horizontal
layout policy.
The size request of the widget is 0x0.

The user can edit the text by using the keys described in the keyboard usage section.
Additionally the input box accepts text pasted into the terminal and inserts it at the cursor position.
It emits signals when the cursor or scroll position changes.

Unless in detached scrolling mode, the text is always scrolled so that the cursor position is visible.


Palette
-------

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Palette Color
      - Usage

   *  - ``textedit.fg``, ``textedit.bg``
      - Body of the text edit widget (active, **unfocused**)

   *  - ``textedit.disabled.fg``, ``textedit.disabled.bg``
      - Body of the text edit widget (**disabled**)

   *  - ``textedit.focused.fg``, ``textedit.focused.bg``
      - Body of the text edit widget (active, **focused**)

   *  - ``textedit.selected.fg``, ``textedit.selected.bg``
      - Selected text

   *  - ``textedit.linenumber.fg``, ``textedit.linenumber.bg``
      - Line number border (active, **unfocused**)

   *  - ``textedit.focused.linenumber.fg``, ``textedit.focused.linenumber.bg``
      - Line number border (active, **focused**)


Commands
--------

This widget registers the following commands if the applications calls
:cpp:func:`void Tui::ZTextEdit::registerCommandNotifiers(Qt::ShortcutContext context)` using the passed ``context``.

.. list-table::
   :class: noborder
   :align: left
   :header-rows: 1

   *  - Command
      - Usage

   *  - ``Copy``
      - Copies the current selection to the clipboard.

   *  - ``Cut`` :sup:`(r)`
      - Copies the selected text to the clipboard and removes it.

   *  - ``Paste`` :sup:`(r)`
      - Inserts the current clipboard content at the cursor position.
        If there is a current selection, the selected text is deleted before inserting the new text.

   *  - ``Undo`` :sup:`(r)(u)`
      - Undo the previous edit step.

   *  - ``Redo`` :sup:`(r)(u)`
      - Redo the previous edit step.

| (r) Read-only mode will disable this command.
| (u) If isUndoRedoEnabled is set to :cpp:expr:`false` this command will be disabled.

ZTextEdit
---------

.. cpp:class:: Tui::ZTextEdit : public Tui::ZWidget

   A multi line text edit widget.

   **Constructors**

   .. cpp:function:: ZTextLine(const Tui::ZTextMetrics &textMetrics, Tui::ZWidget *parent=nullptr)
   .. cpp:function:: ZTextLine(const Tui::ZTextMetrics &textMetrics, Tui::ZDocument *document, Tui::ZWidget *parent=nullptr)

      Create the ZTextEdit with the text metrics ``textMetrics`` and the given ``parent``.

      The variant taking ``document`` sets the document used to ``document`` otherwise it creates a fresh document
      as backing that is automatically deleted when this widget is destructed.

   **Functions**

   .. cpp:function:: void setCursorPosition(Tui::ZDocumentCursor::Position position, bool extendSelection=false)

      Sets the "position" point of the text cursor to ``position``.
      If ``extendSelection`` the "anchor" point is kept, otherwise it is set to the same position.

   .. cpp:function:: Tui::ZDocumentCursor::Position cursorPosition() const

      Returns the "position" point of the text cursor.

   .. cpp:function:: void setAnchorPosition(Tui::ZDocumentCursor::Position position)

      Sets the "anchor" point of the text cursor to ``position``.

   .. cpp:function:: Tui::ZDocumentCursor::Position anchorPosition() const

      Returns the "anchor" point of the text cursor.

   .. cpp:function:: void setTextCursor(const Tui::ZDocumentCursor &cursor)

      Sets the text cursor "anchor" and "position" point to the values from ``cursor`` and updates the vertical
      movement position to that of ``cursor``.

   .. cpp:function:: Tui::ZDocumentCursor textCursor() const

      Returns a copy of the editing cursor of the text edit widget.

   .. cpp:function:: void setSelection(Tui::ZDocumentCursor::Position anchor, Tui::ZDocumentCursor::Position position)

      Sets the text cursor selection to start at ``anchor`` and end at ``position``.

   .. cpp:function:: Tui::ZDocument *document() const

      Returns the model :cpp:class:`Tui::ZDocument` instance.
      Use this for more control over the document or to access additional apis to observe or edit the text in the widget.

   .. cpp:function:: int lineNumberBorderWidth() const

      Returns the width in columns of the line number border.
      If the line number border is disabled, it returns the width it would have if enabled.

   .. rst-class:: tw-virtual
   .. cpp:function:: virtual int allBordersWidth() const

      Returns the width of all enabled borders.
      The base class only has one border, the line number border.

      Derived classes can override this function to influence the text width used for cursor movement and scrolling
      calculations.

   .. cpp:function:: void setTabStopDistance(int tab)
   .. cpp:function:: int tabStopDistance() const

      The tab stop distance determines in what interval stops for tabs are set.
      These stops are used for hard tabs (\\t characters) as well as for the stops when emulating tabs with spaces and
      for indent and dedent.

   .. cpp:function:: void setShowLineNumbers(bool show)
   .. cpp:function:: bool showLineNumbers() const

      Selects if a border with line numbers is shown.

   .. cpp:function:: void setUseTabChar(bool tab)
   .. cpp:function:: bool useTabChar() const

      Selects if hard tabs (\\t characters) are used as tabs and for indention.

   .. cpp:function:: void setWordWrapMode(Tui::ZTextOption::WrapMode wrap)
   .. cpp:function:: Tui::ZTextOption::WrapMode wordWrapMode() const

      Selects the word/line wrapping mode.
      If wrapping is enabled horizontal scrolling is disabled.

   .. cpp:function:: void setOverwriteMode(bool mode)
   .. cpp:function:: void toggleOverwriteMode()
   .. cpp:function:: bool overwriteMode() const

      If set to :cpp:expr:`true`, typed text overwrites existing text.
      Text never overwrites line breaks.
      This option does not apply to text inserted by calling functions on this class or inserted by pasting.

   .. cpp:function:: void setSelectMode(bool mode)
   .. cpp:function:: void toggleSelectMode()
   .. cpp:function:: bool selectMode() const

      If the select mode is active, cursor movement keys extend the selection without holding down :kbd:`Shift`.
      This can be helpful with terminals that don’t report some key combinations with :kbd:`Shift`.

   .. cpp:function:: void setInsertCursorStyle(Tui::CursorStyle style)
   .. cpp:function:: Tui::CursorStyle insertCursorStyle() const

      Selects the cursor style to use when overwrite mode is not set.

   .. cpp:function:: void setOverwriteCursorStyle(Tui::CursorStyle style)
   .. cpp:function:: Tui::CursorStyle overwriteCursorStyle() const

      Selects the cursor style to use when overwrite mode is set.

   .. cpp:function:: void setTabChangesFocus(bool enabled)
   .. cpp:function:: bool tabChangesFocus() const

      Selects if :kbd:`Tab` can be used to change focus (i.e. is ignored by the text edit widget) or if it is used
      as an edit operation.

   .. cpp:function:: void setReadOnly(bool readOnly)
   .. cpp:function:: bool isReadOnly() const

      Selects if this widget is read-only.
      Read-only mode only applies to edits by keyboard and to commands.

   .. cpp:function:: void setUndoRedoEnabled(bool enabled)
   .. cpp:function:: bool isUndoRedoEnabled() const

      Selects if undo and redo keyboard shortcuts and commands are enabled.

   .. cpp:function:: bool isModified() const

      Returns :cpp:expr:`true` if the document displayed in this text edit widget has been modified.

      See :cpp:func:`Tui::ZDocument::isModified()`.

   .. cpp:function:: void insertText(const QString &str)

      Inserts the text ``str`` at the current cursor position.
      If there is a current selection, the selected text is deleted before inserting the new text.

   .. cpp:function:: void insertTabAt(Tui::ZDocumentCursor &cur)

      Inserts a tab at the given cursor ``cursor``.
      The width and type (using \\t or spaces) is configurable using :cpp:func:`~bool Tui::ZTextEdit::useTabChar() const`
      and :cpp:func:`~int Tui::ZTextEdit::tabStopDistance() const`.

   .. rst-class:: tw-virtual
   .. cpp:function:: void cut()

      Copies the selected text to the clipboard and removes it.

   .. rst-class:: tw-virtual
   .. cpp:function:: void copy()

      Copies the current selection to the clipboard.

   .. rst-class:: tw-virtual
   .. cpp:function:: void paste()

      Inserts the current clipboard content at the cursor position.
      If there is a current selection, the selected text is deleted before inserting the new text.

   .. rst-class:: tw-virtual
   .. cpp:function:: bool canPaste()

      Returns :cpp:expr:`true` if :cpp:func:`~bool Tui::ZTextEdit::paste()` is current available.

   .. rst-class:: tw-virtual
   .. cpp:function:: bool canCut()

      Returns :cpp:expr:`true` if :cpp:func:`~bool Tui::ZTextEdit::cut()` is current available.

   .. rst-class:: tw-virtual
   .. cpp:function:: bool canCopy()

      Returns :cpp:expr:`true` if :cpp:func:`~bool Tui::ZTextEdit::copy()` is current available.

   .. cpp:function:: Tui::ZDocument::UndoGroup startUndoGroup()

      Starts an undo group.

      See :ref:`zdocument_undoredo` for details.

   .. cpp:function:: void removeSelectedText()

      Removes the currently selected text.

   .. cpp:function:: void clearSelection()

      Clears the selection without changing the contents of the document.

   .. cpp:function:: void selectAll()

      Selects all text in the document.

   .. cpp:function:: QString selectedText() const

      Return the selected text, if any.
      Otherwise returns the empty string.

   .. cpp:function:: bool hasSelection() const

      Returns :cpp:expr:`true` if there is currently a text selection.

   .. cpp:function:: void undo()

      Preforms an undo step.

   .. cpp:function:: void redo()

      Performs a redo step.

   .. cpp:function:: void enableDetachedScrolling()
   .. cpp:function:: void disableDetachedScrolling()
   .. cpp:function:: bool isDetachedScrolling() const

      In detached scrolling mode, the scroll position is not automatically adjusted to keep the cursor visible.
      Use :cpp:func:`~void Tui::ZTextEdit::detachedScrollUp()`, :cpp:func:`~void Tui::ZTextEdit::detachedScrollUp()`
      or :cpp:func:`~void Tui::ZTextEdit::setScrollPosition(int column, int line, int fineLine)`
      to adjust the scroll position manually.

   .. cpp:function:: void detachedScrollUp()
   .. cpp:function:: void detachedScrollDown()

      When detached scrolling is active these function allow scrolling the text up and down.

   .. cpp:function:: int scrollPositionLine() const

      Returns the index of the topmost visible line.

   .. cpp:function:: int scrollPositionFineLine() const

      When line wrapping is enabled, returns the number of wrapped visual lines of the topmost visible logical line that
      are scrolled out.

      Otherwise always returns 0.

   .. cpp:function:: int scrollPositionColumn() const

      Returns the number of columns the text is scrolled to the right.

   .. cpp:function:: void setScrollPosition(int column, int line, int fineLine)

      Sets the current scroll position to ``column`` columns scrolled to the right, ``line`` lines scrolled down and
      ``fineLine`` wrapped visual lines of the topmost visible logical lines that are scrolled out.

   .. rst-class:: tw-virtual
   .. cpp:function:: int pageNavigationLineCount() const

      Returns the number of visual lines that :kbd:`Page Up` and :kbd:`Page Down` keys move up or down.

      Derived classes can override this method to change the number of lines the cursor is moved.

   .. cpp:function:: Tui::ZDocumentCursor findSync(const QString &subString, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})

      Find the next occurrence of literal string ``subString`` in the document starting at the current cursor position.
      If the search succeeds the found text will be selected, otherwise the current selection is cleared.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns a cursor with the match selected if found, otherwise a cursor without a
      selection.

      See :ref:`zdocument_finding` for more details.

   .. cpp:function:: Tui::ZDocumentCursor findSync(const QRegularExpression &regex, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})

      Find the next occurrence of regular expression ``regex`` in the document starting at the current cursor position.
      If the search succeeds the found text will be selected, otherwise the current selection is cleared.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns a cursor with the match selected if found, otherwise a cursor without a
      selection.
      If access to details about capture groups is required use
      :cpp:func:`findSyncWithDetails <Tui::ZDocumentFindResult Tui::ZDocument::findSyncWithDetails(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const>`.

      See :ref:`zdocument_finding` for more details.

   .. cpp:function:: Tui::ZDocumentFindResult findSyncWithDetails(const QRegularExpression &regex, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})

      Find the next occurrence of regular expression ``regex`` in the document starting at the current cursor position.
      If the search succeeds the found text will be selected, otherwise the current selection is cleared.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns an instance of :cpp:class:`Tui::ZDocumentFindResult` with the find
      result. It contains a cursor with the match selected if found, otherwise a cursor without a
      selection. Additionally it contains the contents of the regular expression's capture groups.

      See :ref:`zdocument_finding` for more details.

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsync(const QString &subString, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})
   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority, const QString &subString, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})

      Find the next occurrence of literal string ``subString`` in the document starting at the current cursor position.
      If the search succeeds the found text will be selected, otherwise the current selection is cleared.

      This function runs asynchronously and returns a future resolving to a
      :cpp:class:`Tui::ZDocumentFindAsyncResult` instance with the find
      result. It contains positions for a cursor with the match selected if found, otherwise
      both positions are equal to signal no match was found.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The variant taking ``pool`` and ``priority``, runs the search operation on the
      thread pool ``pool`` with the priority ``priority``.
      The variant without runs the search operation on the default thread pool with default priority.

      See :ref:`zdocument_finding` for more details.

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsync(const QRegularExpression &regex, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})
   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority, const QRegularExpression &regex, Tui::ZDocument::FindFlags options = Tui::ZDocument::FindFlags{})

      Find the next occurrence of regular expression ``regex`` in the document starting at the current cursor position.
      If the search succeeds the found text will be selected, otherwise the current selection is cleared.

      This function runs asynchronously and returns a future resolving to a
      :cpp:class:`Tui::ZDocumentFindAsyncResult` instance with the find
      result. It contains positions for a cursor with the match selected if found, otherwise
      both positions are equal to signal no match was found.
      Additionally it contains the contents of the regular expression's capture groups.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The variant taking ``pool`` and ``priority``, runs the search operation on the
      thread pool ``pool`` with the priority ``priority``.
      The variant without runs the search operation on the default thread pool with default priority.

      See :ref:`zdocument_finding` for more details.

   .. cpp:function:: void clear()

      Reset the document used by this widget back to the empty state.

   .. cpp:function:: void readFrom(QIODevice *file)

      Replaces the contents of the document used by this widget with the contents of :cpp:class:`QIODevice` ``file``.

   .. cpp:function:: void readFrom(QIODevice *file, Tui::ZDocumentCursor::Position initialPosition)

      Replaces the contents of the document used by this widget with the contents of :cpp:class:`QIODevice` ``file``.
      It sets the initial cursor position to ``initialPosition``.

   .. cpp:function:: void writeTo(QIODevice *file) const

      Saves the contents of the document used by this widget in to the :cpp:class:`QIODevice` ``file``.

      Also marks the current undo state as saved.

   .. cpp:function:: void setText(const QString &text)

      Replaces the contents of the document used by this widget with the contents of the string ``text``.

   .. cpp:function:: void setText(const QString &text, Tui::ZDocumentCursor::Position initialPosition)

      Replaces the contents of the document used by this widget with the contents of the string ``text``.
      It sets the initial cursor position to ``initialPosition``.

   .. cpp:function:: void text() const

      Returns the contents of the document used by this widget as a string.

      In constrast to :cpp:func:`~void writeTo(QIODevice *file) const` this function does not mark the current undo
      state as saved.

   .. cpp:function:: void registerCommandNotifiers(Qt::ShortcutContext context)

      Registers the commands listed in the command section using the shortcut context ``context``.

   .. cpp:function:: Tui::ZDocumentCursor makeCursor()

      Returns a newly created cursor for the backing document for this widget.

   **Signals**

   .. cpp:function:: void cursorPositionChanged(int x, int utf16CodeUnit, int utf8CodeUnit, int line)

      This signal is emitted with the current cursor position.
      The ``x`` value is the visual position in the line in columns, the ``utf16CodeUnit`` is position in the line
      in normal (utf16) code units, the ``utf8CodeUnit`` has the same information in utf8 code units as used in files
      and ``line`` is the line in the document.

   .. cpp:function:: void scrollPositionChanged(int x, int line, int fineLine)

      This signal is emitted with the current scroll position.
      The value ``x`` is the amount of columns the text is scrolled to the right, ``line`` is the number of (unwrapped)
      lines scrolled down and ``fineLine`` is the amount of visual lines scrolled in a wrapped logical line.

   .. cpp:function:: void scrollRangeChanged(int x, int y)

      This signal is emitted with the current scrolling range.
      The value ``x`` specifies that needed range ``0 <= · < x`` for the x direction and the ``y`` value specifies
      the allowed range of lines to scroll ``0 <= · < y``.

   .. cpp:function:: void overwriteModeChanged(bool overwrite)

      This signal is emitted when the value of overwriteMode is changed.

   .. cpp:function:: void modifiedChanged(bool modified)

      This signal is emitted when the value of isModified is changed.

   .. cpp:function:: void selectModeChanged(bool mode)

      This signal is emitted when the value of selectMode is changed.


   **Protected Functions**

   .. rst-class:: tw-virtual
   .. cpp:function:: Tui::ZTextOption textOption() const

      Returns the :cpp:class:`Tui::ZTextOption` used for painting the text and for various calculations while editing.
      A derived class can override this to change the used text options.
      This does not influence the options used for creating the cursor.

   .. cpp:function:: Tui::ZTextLayout textLayoutForLine(const Tui::ZTextOption &option, int line) const

      Returns the text layout for the line with index ``line`` and using the text option ``option``.

   .. cpp:function:: Tui::ZTextLayout textLayoutForLineWithoutWrapping(int line) const

      Returns the text layout for the line with index ``line`` and the text option returned by
      :cpp:func:`~Tui::ZTextOption Tui::ZTextEdit::textOption() const` but with the text wrap disabled.

   .. cpp:function:: const Tui::ZTextMetrics &textMetrics() const

      Returns the text metrics used by this widget.

   .. rst-class:: tw-virtual
   .. cpp:function:: void adjustScrollPosition()

      Recalculates the scroll position to ensure the cursor is visible (unless detached scrolling is active).
      A derived class can override this to function to change how the scrolling of the widget works.

   .. rst-class:: tw-virtual
   .. cpp:function:: void emitCursorPostionChanged()

      Emits the current cursor position.
      A derived class can override this to change what position is reported in the cursor position changed signal.

   .. rst-class:: tw-virtual
   .. cpp:function:: void updateCommands()

      Updates the active status of the registered commands of this widget.
      A derived class can override this to update additional commands, but it should always invoke the function in the
      base class.

   .. rst-class:: tw-virtual
   .. cpp:function:: void clearAdvancedSelection()

      This function does nothing in the base class, but is called in various places where a selection in created.
      A derived class can override this, if it implements its own selection mode that need to be cleared before
      a selection in the base class is created.
      The derived class likely needs to override the keyEvent handler and re-implement movement and related key
      combinations to get consistent behavior.

   **Type aliases**

   This class has some convenience type aliases.

   .. cpp:type:: Position = Tui::ZDocumentCursor::Position

   .. cpp:type:: FindFlag = Tui::ZDocument::FindFlag

   .. cpp:type:: FindFlags = Tui::ZDocument::FindFlags

