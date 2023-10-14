.. _ZDocument:

ZDocument
=========

ZDocument is a model class to store multi-line text for usage with multi-line text edit widgets.

The document class itself mostly serves read access.
Most modifications are done using the :cpp:class:`Tui::ZDocumentCursor` class.

The current contents of the document can be accessed line by line.
Also this class serves to enable functions on the document as a whole, like load/save and undo/redo.

The document may only be accessed from its associated thread, but
a :cpp:class:`Tui::ZDocumentSnapshot` can be created from a document that allows access, to a
frozen state of the document, from any thread.
This can be useful to implement time intensive operations on the document (like syntax highlighting)
without blocking the main thread.

When loading a document that consistently uses CRLF (\\r\\n) line endings (traditional on MSDOS and
Windows) the CR (\\r) will be removed from all lines and can later be added back on saving.

Text documents are expected to normally end in a line break.
If the document does not, then the
:cpp:func:`~bool Tui::ZDocument::newlineAfterLastLineMissing() const` property is set and no
final newline will be generated when saving the document.

Cursors and line markers
------------------------

It is often needed to track a certain position in the document, even if the contents before that
position changes in length.
For this :cpp:class:`Tui::ZDocumentCursor` and :cpp:class:`Tui::ZDocumentLineMarker` can be used.
Instances of these classes are updated accordingly to the edits happening before them in the
document.

In contrast :cpp:class:`Tui::ZDocumentCursor::Position` is a simple value of a fixed position that
is not updated on changes.

The cursor is also the primary means of modification to the document.
See :ref:`using_document_cursors` for details.

Cursors and line markers are also updated on undo and redo.

Change tracking
---------------

Each change of the document's contents changes
:cpp:func:`~unsigned Tui::ZDocument::revision() const`.
The revision of the document does not change back to an old value on undo.
The revision is not changed when `line user-data <Line user-data>`_ is set or modified.

Each line also has a revision that is changed whenever the line is changed.
The line revision returns to a previous value on undo and redo.
It is not changed when line user-data is set or modified.

For changes to the document the signal :cpp:func:`~void Tui::ZDocument::contentsChanged()` is
emitted asynchronously. That means the signal is emitted after changes have happened in a future
event loop iteration.
Thus the part of the application that performs edits can be sure the receivers of the signal do not
interfere with the editing.

Additional signals are emitted when the modification status (i.e. if the current undo step is
the initial one or was explicitly marked as saved) changes.

Changes to cursor
(:cpp:func:`cursorChanged(const Tui::ZDocumentCursor *cursor) <void Tui::ZDocument::cursorChanged(const Tui::ZDocumentCursor *cursor)>`)
and line marker
(:cpp:func:`lineMarkerChanged(const Tui::ZDocumentLineMarker *marker) <void Tui::ZDocument::lineMarkerChanged(const Tui::ZDocumentLineMarker *marker)>`)
positions also trigger signals.

Line user-data
--------------

It is often useful for an application to store additional information associated with a line.
This could for example be cached syntax highlighting information or generally additional information.

To facilitate this, the document allows saving an additional object with each line, by using
:cpp:func:`setLineUserData <void Tui::ZDocument::setLineUserData(int line, std::shared_ptr<Tui::ZDocumentLineUserData> userData)>`.

Setting it or changes to it do not trigger change signals or increment the document or line revisions.
The user is responsible for threading save usage of this object, when using snapshots with other
threads than the document's thread.

Currently the user-data is restored when using undo and redo, but this might change in the future.

Undo and redo
-------------

The document class supports undo and redo operations.

The document's modified state is tracked according to the current undo step.
When the document is saved the application is responsible to call
:cpp:func:`~void Tui::ZDocument::markUndoStateAsSaved()` to update the undo step that is considered
to be unmodified.

When inserting text in the document, multiple insert operations are merged to avoid too fine grained
undo steps. If the inserted text contains a space, tab or line break or if the insert is performed
at a different position the merging is prevented.
The application can suppress merging by
calling :cpp:func:`~void Tui::ZDocument::clearCollapseUndoStep()`.

If one logical action of the user is composed of multiple changes to the document, an undo group
can merge all changes into one undo step.
An undo group is started with
:cpp:func:`startUndoGroup <UndoGroup Tui::ZDocument::startUndoGroup(Tui::ZDocumentCursor *cursor)>` and is
active until :cpp:func:`explicitly closed <void Tui::ZDocument::UndoGroup::closeGroup()>` or
until the returned group object is destroyed.

An application can keep itself informed, if currently an undo or redo operation is available, by
connecting to the :cpp:func:`~void Tui::ZDocument::undoAvailable(bool available)`
and :cpp:func:`~void Tui::ZDocument::redoAvailable(bool available)` signals.

Finding
-------

The document offers functions to search for literal text or regular expression matches in the
document.

These functions are offered in a synchronous and asynchronous variants.

The start position of a search is passed in by using a cursor.

For forward search, the next match for a given starting cursor is a match which starting point is
greater than the maximum of the starting cursor's "anchor" and "position" point.

For backward search, the next match for a given starting cursor is a match that ends before the
maximum of the starting cursor's "anchor" and "position" point and does not intersect with the
last character of the selection (if a selection is active).

By using :cpp:type:`Tui::ZDocument::FindFlags` the application can specify how the search is
performed.
By default the search is case insensitive, runs forward and does not wrap.
If wrapping is enabled by :cpp:enumerator:`FindFlags::FindWrap <Tui::ZDocument::FindFlag::FindWrap>`,
if no result is found by searching in the specified direction the search will be repeated from the
other end of the document till to the starting position of the search. |br|
If case senstive search is requested by
:cpp:enumerator:`FindFlags::FindCaseSensitively <Tui::ZDocument::FindFlag::FindCaseSensitively>`
a match will be required to be exact, otherwise a match using 
:cpp:enum:`Qt::CaseInSensitive <Qt::CaseInSensitive>` or
:cpp:enum:`QRegularExpression::PatternOption::CaseInsensitiveOption <QRegularExpression::PatternOption::CaseInsensitiveOption>`
will be performed. |br|
A backward search is requested by
:cpp:enumerator:`FindFlags::FindBackward <Tui::ZDocument::FindFlag::FindBackward>`.

.. _using_document_cursors:

Using cursors
-------------

A cursor represents a position in the document or a selection between two positions.
It has functions to navigate in the document and alter its contents.

Points in the document are between characters (or more precisely between clusters of code points)
in the document.
The position is expressed in the number of code-units before the character/cluster and the line
number.
Zero code-units into a line designates the position before the first character/cluster in the line
and the position after the last character/cluster is designated by the total count of code-units
in the line (i.e. the result of :cpp:func:`Tui::ZDocument::lineCodeUnits(line) <int Tui::ZDocument::lineCodeUnits(int) const>`)
Movement and the vertical position of the layouted text are calculated using the
:cpp:class:`Tui::ZTextLayout` class.
When creating the cursor, the application has to supply a function to create a
:cpp:class:`Tui::ZTextLayout` for a given line with the configuration applied that the cursor
should use.

.. literalinclude:: examples/widgets/document.cpp
    :start-after: // snippet-start
    :end-before:  // snippet-end
    :dedent:


The movement is then determined by
:cpp:func:`Tui::ZTextLayout::nextCursorPosition(…) <int Tui::ZTextLayout::nextCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const>` and
:cpp:func:`Tui::ZTextLayout::previousCursorPosition(…) <int Tui::ZTextLayout::previousCursorPosition(int oldPos, Tui::ZTextLayout::CursorMode mode = SkipCharacters) const>`.

The selection spans between two points called "anchor" and "position".
If both points are equal the cursor has no selection.
If they differ the text between the two positions is selected.
The order of the two points is significant when the navigation functions of the cursor are used.
The navigation always moves the "position".
If the navigation functions are used to extend the selection, the "anchor" point is not changed.
Otherwise the "anchor" point is reset to the "position" point after the navigation.
If, after a navigation while extending the selection, the "anchor" point and the "position" point
are equal, the cursor does not have a active selection.

If the cursor has a active selection, then inserting will replace the selected range in the document.
With an active selection, the delete functions (except
:cpp:func:`deleteLine() <void Tui::ZDocumentCursor::deleteLine()>`) will remove the selected text
instead of their specific function.

In addition to the "anchor" and "position" points that determine the position and selection of the
cursor, there is a additional vertical movement position.
The vertical movement position is not in code-units as the points, but in columns and is used when
the "position" point of the cursor is moved up or down using 
:cpp:func:`moveUp <void Tui::ZDocumentCursor::moveUp(bool extendSelection = false)>` or
:cpp:func:`moveDown <void Tui::ZDocumentCursor::moveDown(bool extendSelection = false)>`.
This allows a user to navigate across lines or within wrapped lines visually and preserves the
vertical position when navigating over short lines.

The "position" point will always stay inside the bounds of the line in the document,
while the vertical movement position can be right of the end of the line.

The vertical movement position is updated when setting a new position with
:cpp:func:`setPosition(…) <void Tui::ZDocumentCursor::setPosition(Position pos, bool extendSelection = false)>`
and when using horizontal navigation functions.
It can also be set explicitly.

ZDocument
---------

.. cpp:class:: Tui::ZDocument : public QObject


   **Constructors**

   .. cpp:function:: ZDocument(QObject *parent = nullptr)

      Constructs an empty document, with ``parent`` as its parent in the QObject system.

   **Functions**

   .. cpp:function:: void reset()

      Reset the document back to the empty state.

      This function triggers change signals.

   .. cpp:function:: void writeTo(QIODevice *file, bool crLfMode = false) const

      Write the document's contents to the QIODevice ``file``.

      If ``crLfMode`` is :cpp:expr:`true`, line breaks will use the CRLF (\\r\\n) line ending
      convention.
      To keep the same line ending convention as the originally loaded file, pass the return value
      from :cpp:func:`bool Tui::ZDocument::crLfMode() const` here.

      If :cpp:func:`~bool newlineAfterLastLineMissing() const` is not set, an trailing line break
      will be added to the file.

   .. cpp:function:: void readFrom(QIODevice *file)
   .. cpp:function:: void readFrom(QIODevice *file, Tui::ZDocumentCursor::Position initialPosition, Tui::ZDocumentCursor *initialPositionCursor)

      Reset the document and read new data from the QIODevice ``file``.

      Sets the :cpp:func:`crLfMode property <bool Tui::ZDocument::crLfMode() const>` to  :cpp:expr:`true`,
      if the file consistently uses the CRLF (\\r\\n) line ending convention.
      Pass the this value as ``crLfMode`` into ``writeTo`` to keep the same line ending
      convention as the loaded file when saving.

      The variant taking a ``initialPosition`` sets the initial cursor position to ``initialPosition``
      in the initial undo step.

      The variant taking a ``initialPositionCursor`` sets the initial cursor position also in the
      cursor passed as ``initialPositionCursor``.
      The passed pointer may be :cpp:expr:`nullptr`.

   .. cpp:function:: void setCrLfMode(bool crLf)
   .. cpp:function:: bool crLfMode() const

      This property contains the line ending convention used when loading this documents.
      If it is :cpp:expr:`false`, lines are terminated using LF (\\n).
      If it is :cpp:expr:`true`, lines are terminated using CRLF (\\r\\n).

   .. cpp:function:: int lineCount() const

      Returns the number of lines in the document.

   .. cpp:function:: QString line(int line) const

      Returns the text contents of the line with index ``line`` in the document.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: int lineCodeUnits(int line) const

      Returns the length in code-units of the line with index ``line`` in the document.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: unsigned lineRevision(int line) const

      Return the revision of the line with index ``line`` in the document.

      See `Change tracking`_ for more details.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: void setLineUserData(int line, std::shared_ptr<Tui::ZDocumentLineUserData> userData)

      Set the user-data pointer of the line with index ``line`` in the document to ``userData``.

      This function does not change the line's or document's revision and does not trigger change
      signals.
      It also does not create a new undo step.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: std::shared_ptr<Tui::ZDocumentLineUserData> lineUserData(int line) const

      Return the user-data pointer of the line with index ``line`` in the document.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: Tui::ZDocumentSnapshot snapshot() const

      Returns a snapshot of the document.

      This snapshot can be safely accessed from other threads and captures the current contents of
      the document.
      It will not change when further edits are applied to the document.

      The application is responsible to ensure that multi-threaded access to the user-data pointer
      contained in the snapshot is safe.

   .. cpp:function:: unsigned revision() const

      Returns the current document revision.

      See `Change tracking`_ for more details.

   .. cpp:function:: bool isModified() const

      Returns if the document is currently modified.

      That is, it returns :cpp:expr:`true` if the current undo step is not marked as the currently
      saved undo step.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void setNewlineAfterLastLineMissing(bool value)
   .. cpp:function:: bool newlineAfterLastLineMissing() const

      This property describes if the document is missing the conventional final line break of a
      text document.

      If it is :cpp:expr:`false` the final line break is implicitly added when saving the document.

   .. cpp:function:: QString filename() const
   .. cpp:function:: void setFilename(const QString &filename)

      This property contains the file name of the document.
      It is for informational purposes only and is not used in implementation of this class.
      It is up to the application, if and how this property is maintained.

   .. cpp:function:: void clearCollapseUndoStep()

      Prevent the next operation to be merged with the current undo step.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void sortLines(int first, int last, Tui::ZDocumentCursor *cursorForUndoStep)

      Sort the lines from ``first`` to ``last`` (exclusive) of the document.

   .. cpp:function:: void moveLine(int from, int to, Tui::ZDocumentCursor *cursorForUndoStep)

      Move the line with index ``from`` to the position ``to``.

      The line in index ``from`` will end up on index ``to`` after the operation.

      Example:

      Given a document with the following lines:

      | red
      | green
      | blue
      | white

      If we now call ``moveLine(1, 2, …)``, the result will be:

      | red
      | blue
      | green
      | white

      The value of ``from`` and ``to`` must both be ``0 <= · < lineCount()`` to avoid
      undefined behavior.

   .. cpp:function:: void debugConsistencyCheck(const Tui::ZDocumentCursor *exclude=nullptr) const

      Perform a consistency check for debugging.
      Excludes the cursor ``exclude`` from the consistency check if set to a non-null value.

      This function aborts the program is a problem is found.

   .. cpp:function:: void undo(Tui::ZDocumentCursor *cursor)

      Preforms an undo step.

      The cursor ``cursor`` is moved to the position of the cursor, that created the undo step,
      before the modification.
      The passed pointer must not be :cpp:expr:`nullptr`.

      If :cpp:func:`~bool Tui::ZDocument::isUndoAvailable() const` is :cpp:expr:`false` this does
      nothing.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void redo(Tui::ZDocumentCursor *cursor)

      Performs a redo step.
      That is, it counteracts a previous undo if no editing has happened in-between.

      The cursor ``cursor`` is moved to the position of the cursor, that created the undo step,
      after the modification.
      The passed pointer must not be :cpp:expr:`nullptr`.

      If :cpp:func:`~bool Tui::ZDocument::isRedoAvailable() const` is :cpp:expr:`false` this does
      nothing.

      See `Undo and redo`_ for more details.

   .. cpp:function:: bool isUndoAvailable() const

      Returns :cpp:expr:`true` if :cpp:func:`undo <void Tui::ZDocument::undo(Tui::ZDocumentCursor *cursor)>`
      can be called.

   .. cpp:function:: bool isRedoAvailable() const

      Returns :cpp:expr:`true` if :cpp:func:`redo <void Tui::ZDocument::redo(Tui::ZDocumentCursor *cursor)>`
      can be called.

   .. cpp:function:: UndoGroup startUndoGroup(Tui::ZDocumentCursor *cursor)

      Starts an undo group and returns the undo group's activation object.

      The position of cursor ``cursor`` before and after the modification is saved in the undo
      step.
      The passed pointer must not be :cpp:expr:`nullptr`.

      Undo groups can not be nested and are closed when the activation object (or its moved to
      objects) are destroyed or the undo group is explicitly closed.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void markUndoStateAsSaved()

      Marks the current undo step as saved.

      After calling this function :cpp:func:`bool isModified() const` will return :cpp:expr:`false`
      until the next modification is made (or undo/redo is called).

      It also prevents merging of the current undo step with a newly created undo step on future
      changes.

      See `Undo and redo`_ for more details.

   .. cpp:function:: Tui::ZDocumentCursor findSync(const QString &subString, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

      Find the next occurrence of literal string ``subString`` in the document starting with ``start``.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns a cursor with the match selected if found, otherwise a cursor without a
      selection.

      See `Finding`_ for more details.

   .. cpp:function:: Tui::ZDocumentCursor findSync(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

      Find the next occurrence of regular expression ``regex`` in the document starting with ``start``.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns a cursor with the match selected if found, otherwise a cursor without a
      selection.
      If access to details about capture groups is required use
      :cpp:func:`findSyncWithDetails <Tui::ZDocumentFindResult Tui::ZDocument::findSyncWithDetails(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const>`.

      See `Finding`_ for more details.

   .. cpp:function:: Tui::ZDocumentFindResult findSyncWithDetails(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

      Find the next occurrence of regular expression ``regex`` in the document starting with ``start``.

      This function runs synchronously, so be careful not to block the main thread for too long,
      by searching in long documents.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The function returns an instance of :cpp:class:`Tui::ZDocumentFindResult` with the find
      result. It contains a cursor with the match selected if found, otherwise a cursor without a
      selection. Additionally it contains the contents of the regular expression's capture groups.

      See `Finding`_ for more details.

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsync(const QString &subString, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority, const QString &subString, const Tui::ZDocumentCursor &start, FindFlags options = Tui::ZDocument::FindFlags{}) const

      Find the next occurrence of literal string ``subString`` in the document starting with ``start``.

      This function runs asynchronously and returns a future resolving to a
      :cpp:class:`Tui::ZDocumentFindAsyncResult` instance with the find
      result. It contains positions for a cursor with the match selected if found, otherwise
      both positions are equal to signal no match was found.

      The parameter ``options`` allows selecting different behaviors for case-sensitivity,
      wrap around and search direction.

      The variant taking ``pool`` and ``priority``, runs the search operation on the
      thread pool ``pool`` with the priority ``priority``.
      The variant without runs the search operation on the default thread pool with default priority.

      See `Finding`_ for more details.

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsync(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

   .. cpp:function:: QFuture<Tui::ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority, const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const

      Find the next occurrence of regular expression ``regex`` in the document starting with ``start``.

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

      See `Finding`_ for more details.

   **Signals**


   .. cpp:function:: void modificationChanged(bool changed)

      This signal informs the application of the current modification state of the document.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void redoAvailable(bool available)

      This signal informs the application if the redo operation is currently available.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void undoAvailable(bool available)

      This signal informs the application if the undo operation is currently available.

      See `Undo and redo`_ for more details.

   .. cpp:function:: void contentsChanged()

      This signal is emitted asynchronously when the contents of the document was changed.

      See `Change tracking`_ for more details.

   .. cpp:function:: void cursorChanged(const Tui::ZDocumentCursor *cursor)

      This signal is emitted asynchronously when the position of the cursor ``cursor`` has changed.

      This signal is emitted for direct position changes as well as for changes caused by
      modifications before the cursor position or caused by undo or redo.

      See `Change tracking`_ for more details.

   .. cpp:function:: void lineMarkerChanged(const Tui::ZDocumentLineMarker *marker)

      This signal is emitted asynchronously when the position of the line marker ``marker`` has
      changed.

      This signal is emitted for direct position changes as well as for changes caused by
      modifications before the line marker position or caused by undo or redo.

      See `Change tracking`_ for more details.

   .. cpp:function:: void crLfModeChanged(bool crLf)

      This signal is emitted when the :cpp:func:`crLfMode property <bool Tui::ZDocument::crLfMode() const>` changes.
      ``crLf`` contains the new value of the property.

   **Nested Types**

   .. cpp:type:: FindFlags = QFlags<Tui::ZDocument::FindFlag>

   .. cpp:enum:: FindFlag

      Flags used to select find behavior in :cpp:class:`Tui::ZDocument`.

      .. cpp:enumerator:: FindBackward

         Search backwords from the starting cursor.


      .. cpp:enumerator:: FindCaseSensitively

         Search case sensitive.

         If this flag is used, a match will be required to be exact, otherwise a match using
         :cpp:enum:`Qt::CaseInSensitive <Qt::CaseInSensitive>` or
         :cpp:enum:`QRegularExpression::PatternOption::CaseInsensitiveOption <QRegularExpression::PatternOption::CaseInsensitiveOption>`
         will performed.


      .. cpp:enumerator:: FindWrap

         If no match was found, wrap around once and continue searching.

   .. cpp:class:: UndoGroup

      This class is only move constructable and move assignable.
      It is not directly user constructable.

      Activate an undo group by calling :cpp:func:`Tui::ZDocument::startUndoGroup <UndoGroup Tui::ZDocument::startUndoGroup(Tui::ZDocumentCursor *cursor)>`

      The undo group is closed as soon as the instance (or the moved to instance) is destructed,
      or it is manually closed.


     .. cpp:function:: void closeGroup()

        Manually close the undo group.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentCursor

   A cursor is the primary means of modification for a :cpp:class:`Tui::ZDocument`.

   This class is copy constructable and copy assignable.

   See `Using cursors`_ for more details.

   **Constructors**

   .. cpp:function:: explicit ZDocumentCursor(Tui::ZDocument *doc, std::function<Tui::ZTextLayout(int line, bool wrappingAllowed)> createTextLayout)

      Construct a new cursor.

      The cursor needs a layouting function for navigating the document.
      This function is passed as ``createTextLayout`` and must return a layouted
      :cpp:class:`Tui::ZTextLayout` instance for the line passed as ``line``.
      If the application supports line wrapping of the document, it should use the current line
      wrapping setting when the function is called with ``wrappingAllowed`` set to :cpp:expr:`true`.

   **Functions**

   .. cpp:function:: void insertText(const QString &text)

      Insert the text ``text`` at the current cursor location.

      If the cursor has a selection, it will be replaced by the text ``text``.

      Both cursor points will be set to the end of the inserted text.

   .. cpp:function:: void removeSelectedText()

      If the cursor has a selection, it will be removed.

      The cursor will be placed before the start of the selection, after removing the selection.

   .. cpp:function:: void clearSelection()

      Clears the selection.
      That is moves the "anchor" point to the "position" point without changing the contents of
      the document.

   .. cpp:function:: QString selectedText() const

      If the cursor has a selection, returns the currently selected text, otherwise returns an
      empty string.

   .. cpp:function:: void deleteCharacter()

      If the cursor does not have a selection, removes the character/cluster to the right of the
      cursor.
      Otherwise it removes the selection.

      If the cursor started on the end of a line, it removes the line break and joins the current
      line with the next line.

   .. cpp:function:: void deletePreviousCharacter()

      If the cursor does not have a selection, removes the character/cluster to the left of the
      cursor.
      Otherwise it removes the selection.

      If the cursor started on the start of a line, it removes the line break and joins the current
      line with the previous line.

   .. cpp:function:: void deleteWord()

      If the cursor does not have a selection, removes the word to the right of the
      cursor.
      Otherwise it removes the selection.

      If the cursor started on the end of a line, it removes the line break and joins the current
      line with the next line.

      The precise amount of removed text is determined by the :cpp:func:`Tui::ZTextLayout::nextCursorPosition`
      function using :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>`.

   .. cpp:function:: void deletePreviousWord()

      If the cursor does not have a selection, removes the word to the left of the
      cursor.
      Otherwise it removes the selection.

      If the cursor started on the start of a line, it removes the line break and joins the current
      line with the previous line.

      The precise amount of removed text is determined by the :cpp:func:`Tui::ZTextLayout::previousCursorPosition`
      function using :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>`.

   .. cpp:function:: void deleteLine()

      Deletes the whole line that the "position" point is on.

      The cursor will no longer have a selection active after this function is called.

   .. cpp:function:: void moveCharacterLeft(bool extendSelection = false)

      Moves the "position" point of the cursor one character/cluster to the left.
      If the cursor started on the start of a line, it moves the cursor to the end of the previous
      line.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveCharacterRight(bool extendSelection = false)

      Moves the "position" point of the cursor one character/cluster to the right.
      If the cursor started on the end of a line, it moves the cursor to the start of the next
      line.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveWordLeft(bool extendSelection = false)

      Moves the "position" point of the cursor one word to the left.
      If the cursor started on the start of a line, it moves the cursor to the end of the previous
      line.

      The precise movement is determined by the :cpp:func:`Tui::ZTextLayout::previousCursorPosition`
      function using :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>`.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveWordRight(bool extendSelection = false)

      Moves the "position" point of the cursor one word to the right.
      If the cursor started on the end of a line, it moves the cursor to the start of the next
      line.

      The precise movement is determined by the :cpp:func:`Tui::ZTextLayout::nextCursorPosition`
      function using :cpp:enumerator:`CursorMode::SkipWords <Tui::ZTextLayout::CursorMode::SkipWords>`.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveUp(bool extendSelection = false)

      Moves the cursor up.

      When word wrapping is active the cursor navigates based on the wrapped lines.
      Thus moving up on a wrapped line moves the "position" point of the cursor closer to the
      start of the line.

      If the cursor is already on the first line, it moves to the start of the first line.

      The position in the line the moved cursor is placed on is determined based on the layouted
      positions, using the current value of :cpp:func:`int verticalMovementColumn() const`.
      If that vertical position is inside the line the cursor will be placed there, otherwise it
      is placed at the right most position in the line.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveDown(bool extendSelection = false)

      Moves the cursor down.

      When word wrapping is active the cursor navigates based on the wrapped lines.
      Thus moving down on a wrapped line moves the "position" point of the cursor closer to the
      end of the line.

      If the cursor is already on the last line, it moves to the end of the last line.

      The position in the line the moved cursor is placed on is determined based on the layouted
      positions, using the current value of :cpp:func:`int verticalMovementColumn() const`.
      If that vertical position is inside the line the cursor will be placed there, otherwise it
      is placed at the right most position in the line.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveToStartOfLine(bool extendSelection = false)

      Moves the "position" point of the cursor to the start of the current line.
      This function ignores line wrapping.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveToStartIndentedText(bool extendSelection = false)

      Moves the "position" point of the cursor to the start of the non whitespace text on the
      current line.
      The cursor is placed before the first non space and not tab character, or if no such character
      exists at the end of the line.
      This function ignores line wrapping.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveToEndOfLine(bool extendSelection = false)

      Moves the "position" point of the cursor to the end of the current line.
      This function ignores line wrapping.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveToStartOfDocument(bool extendSelection = false)

      Moves the "position" point of the cursor to the start of the first line of the document.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: void moveToEndOfDocument(bool extendSelection = false)

      Moves the "position" point of the cursor to the end of the last line of the document.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

   .. cpp:function:: Position position() const

      Returns the current "position" point of the cursor.

   .. cpp:function:: void setPosition(Position pos, bool extendSelection = false)

      Sets the "position" point of the cursor to ``pos``.

      If ``pos`` points into a cluster of characters or inside a multi-code-unit code-point, the
      position at the nearest cluster boundary after ``pos`` will be used.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

      Updates the vertical movement position of the cursor.

   .. cpp:function:: void setPositionPreservingVerticalMovementColumn(Position pos, bool extendSelection = false)

      Sets the "position" point of the cursor to ``pos``.

      If ``pos`` points into a cluster of characters or inside a multi-code-unit code-point, the
      position at the nearest cluster boundary after ``pos`` will be used.

      If ``extendSelection`` is :cpp:expr:`true` then the "anchor" point of the cursor is not moved,
      otherwise the "anchor" point is set to the new "position" point of the cursor.
      Even if the cursor does not move, if ``extendSelection`` is :cpp:expr:`false` the cursor will
      no longer have a selection active after this function is called.

      Furthermore if the "anchor" and "position" points are equal after this call the cursor will
      no longer have a selection active after this function is called.

      Does not update the vertical movement position of the cursor.

   .. cpp:function:: Position anchor() const

      Returns the current "anchor" point of the cursor.

   .. cpp:function:: void setAnchorPosition(Position pos)

      Sets the "anchor" point of the cursor to ``pos``.

      If ``pos`` points into a cluster of characters or inside a multi-code-unit code-point, the
      position at the nearest cluster boundary after ``pos`` will be used.

      If the "anchor" and "position" points are equal after this call, the cursor will
      no longer have a selection active after this function is called.
      Otherwise the characters in the document from the "anchor" point to the "position" point
      will be selected.

   .. cpp:function:: int verticalMovementColumn() const
   .. cpp:function:: void setVerticalMovementColumn(int column)

      The vertical movement position is used by :cpp:func:`void moveUp(bool extendSelection = false)`
      and :cpp:func:`void moveDown(bool extendSelection = false)` to determine the new position
      of the cursor in a (possibly wrapped) line.

      The vertical movement position of the cursor, is not in code-units as the "anchor" or
      "position" points of the cursor, but in visual columns.

      Vertical movement uses an independent visual position to allow moving the cursor across
      shorter lines without loosing the vertical position, so that moving to a longer line remembers
      the starting point of the up or down movement.

      The vertical movement position is reset to the position of the cursor when calling
      :cpp:func:`void setPosition(Position pos, bool extendSelection = false)` and when the cursor
      is moved with function that is not moveUp or moveDown.

   .. cpp:function:: Position selectionStartPos() const

      If the cursor has an active selection, returns the minimum of the "anchor" and "position"
      points of the cursor. Otherwise return the current cursor position.

   .. cpp:function:: Position selectionEndPos() const

      If the cursor has an active selection, returns the maximum of the "anchor" and "position"
      points of the cursor. Otherwise return the current cursor position.

   .. cpp:function:: void selectAll()

      Selects the whole content of the document.

      It sets the "anchor" point of the cursor to the beginning of the document and the
      "position" point of the cursor after the last character of the document.

   .. cpp:function:: bool hasSelection() const

      Returns true if the document has an active selection.

      That is, if the "anchor" and "position" points are different.

   .. cpp:function:: bool atStart() const

      Returns true of the cursor position is at the start of the document.

   .. cpp:function:: bool atEnd() const

      Returns true if the cursor position is at the end of the document, that is after the
      last character of the last line.

   .. cpp:function:: bool atLineStart() const

      Returns true if the cursor position is at the start of a line.

   .. cpp:function:: bool atLineEnd() const

      Returns true if the cursor position is at the end of a line, that is after the last
      character of the line.

   .. cpp:function:: void debugConsistencyCheck() const

      Perform a consistency check for debugging.

      This function aborts the program is a problem is found.

   **Nested Types**

   .. cpp:class:: Position

      Holds a point in a document as line index and code unit index inside that line.

      The position is before the character specified by the line and code unit index.

      This class is copy and move constructable and assignable.
      It has a default constructor that sets the line and code unit indices to zero.
      The position is comparable and implements operators for equality and comparison.

      .. cpp:member:: int codeUnit

         The code unit before which this position instance points.

      .. cpp:member:: int line

         The line the position instance points to.


.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentLineUserData

   ZDocumentLineUserData is used as base class for line user-data stored by
   :cpp:class:`Tui::ZDocument`.

   See `Line user-data`_ for more details.


   This class is copy and move constructable and assignable.
   It has a default constructor.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentFindAsyncResult

   Contains the results of an asynchronous find operation on :cpp:class:`Tui::ZDocument`.

   This class is copy constructable and copy assignable.
   It has a default constructor but is usually obtained by calling one of the async find methods
   on :cpp:class:`Tui::ZDocument`.

   .. cpp:function:: Tui::ZDocumentCursor::Position anchor() const

      Returns the start position of the match.

      If the position is equal to the position returned by
      :cpp:func:`~Tui::ZDocumentCursor::Position cursor() const` then the search did not match,
      otherwise this position is the start of the match.

      Setting a cursor with both positions will yield a cursor with the match selected.

      If :cpp:func:`unsigned revision() const` does not match the document's revision the search
      result might be outdated.

   .. cpp:function:: Tui::ZDocumentCursor::Position cursor() const

      Returns the position after the end of the match.

      If the position is equal to the position returned by
      :cpp:func:`~Tui::ZDocumentCursor::Position anchor() const` then the search did not match,
      otherwise this position is after the end of the match.

      Setting a cursor with both positions will yield a cursor with the match selected.

      If :cpp:func:`unsigned revision() const` does not match the document's revision the search
      result might be outdated.

   .. cpp:function:: unsigned revision() const

      Returns the revision of the snapshot of the document used for the find operation.

   .. cpp:function:: int regexLastCapturedIndex() const

      Returns the last (highest) index valid to pass in to
      :cpp:func:`~QString regexCapture(int index) const`.

      This is only meaningful when this is a result of a regular expression match,
      otherwise it will be :cpp:expr:`-1`.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#lastCapturedIndex

   .. cpp:function:: QString regexCapture(int index) const

      Returns the captured string of capture group ``index``.

      This is only meaningful when this is a result of a regular expression match.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#captured

   .. cpp:function:: QString regexCapture(const QString &name) const

      Returns the captured string of named capture group ``name``.

      This is only meaningful when this is a result of a regular expression match.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#captured-1

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentFindResult

   Contains the results of a synchronous regular expression find operation
   on :cpp:class:`Tui::ZDocument`.

   This class is copy constructable and copy assignable.
   An instance is usually obtained by calling 
   :cpp:func:`Tui::ZDocument::findSyncWithDetails(…) <Tui::ZDocumentFindResult Tui::ZDocument::findSyncWithDetails(const QRegularExpression &regex, const Tui::ZDocumentCursor &start, Tui::ZDocument::FindFlags options = FindFlags{}) const>`.

   .. cpp:function:: Tui::ZDocumentCursor cursor() const

      Returns the cursor for the match.

      The cursor has the match selected if found, otherwise the cursor does not have a selection.

   .. cpp:function:: int regexLastCapturedIndex() const

      Returns the last (highest) index valid to pass in to
      :cpp:func:`QString regexCapture(int index) const`.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#lastCapturedIndex

   .. cpp:function:: QString regexCapture(int index) const

      Returns the captured string of capture group ``index``.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#captured

   .. cpp:function:: QString regexCapture(const QString &name) const

      Returns the captured string of named capture group ``name``.

      See https://doc.qt.io/qt-5/qregularexpressionmatch.html#captured-1


.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentLineMarker

   A line marker marks a line in a :cpp:class:`Tui::ZDocument`.

   When lines before it are added or removed, it is automatically adjusted to point to the
   original line.

   This class is copy constructable and copy assignable.

   **Constructors**

   .. cpp:function:: explicit ZDocumentLineMarker(Tui::ZDocument *doc)

      Construct an instance initialized to the first line of document ``doc``.

   .. cpp:function:: explicit ZDocumentLineMarker(Tui::ZDocument *doc, int line)

      Construct an instance initialized to the line with index ``line`` of document ``doc``.

   **Functions**

   .. cpp:function:: int line() const

      Return the line index of the line tracked by this line marker.

   .. cpp:function:: void setLine(int line)

      Sets the line tracked by this line marker to the line with index ``line``.


.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZDocumentSnapshot

   A thread-safe snapshot of a :cpp:class:`Tui::ZDocument`.

   This class is copy constructable and copy assignable.
   It is default constructable but instances are usually obtained from
   :cpp:func:`Tui::ZDocumentSnapshot Tui::ZDocument::snapshot() const`.

   .. cpp:function:: int lineCount() const

      Returns the number of lines in the snapshot.

   .. cpp:function:: QString line(int line) const

      Returns the text contents of the line with index ``line`` in the snapshot.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: int lineCodeUnits(int line) const

      Returns the length in code-units of the line with index ``line`` in the snapshot.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: unsigned lineRevision(int line) const

      Return the revision of the line with index ``line`` in the snapshot.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: std::shared_ptr<Tui::ZDocumentLineUserData> lineUserData(int line) const

      Return the user-data pointer of the line with index ``line`` in the snapshot.

      The value of ``line`` must be :cpp:expr:`0 <= line < lineCount()` to avoid undefined behavior.

   .. cpp:function:: unsigned revision() const

      Returns the revision of the document that was used to create this snapshot.

   .. cpp:function:: bool isUpToDate() const

      Returns :cpp:expr:`false` if the revision of the document, this snapshot was created from, has
      changed since the snapshot was created.

.. |br| raw:: html

  <br/>
