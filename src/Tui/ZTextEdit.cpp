// SPDX-License-Identifier: BSL-1.0

#include "ZTextEdit.h"
#include "ZTextEdit_p.h"

#include <QFutureWatcher>

#include <Tui/ZClipboard.h>
#include <Tui/ZPainter.h>
#include <Tui/ZSymbol.h>

TUIWIDGETS_NS_START

ZTextEditPrivate::ZTextEditPrivate(const ZTextMetrics &textMetrics, ZDocument *document, ZWidget *pub)
    : ZWidgetPrivate(pub), textMetrics(textMetrics),
      doc(document ? document : new ZDocument()),
      cursor(makeCursor()),
      scrollPositionLine(doc)
{
    if (!document) {
        autoDeleteDoc.reset(doc);
    }
}

ZTextEditPrivate::~ZTextEditPrivate() {
}


ZTextEdit::ZTextEdit(const ZTextMetrics &textMetrics, ZWidget *parent)
    : ZTextEdit(textMetrics, nullptr, parent)
{
}

ZTextEdit::ZTextEdit(const ZTextMetrics &textMetrics, ZDocument *document, ZWidget *parent)
    : ZWidget(parent, std::make_unique<ZTextEditPrivate>(textMetrics, document, this))
{
    auto *const p = tuiwidgets_impl();

    setSizePolicyH(SizePolicy::Expanding);
    setSizePolicyV(SizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setCursorStyle(CursorStyle::Bar);


    QObject::connect(p->doc, &ZDocument::modificationChanged, this, &ZTextEdit::modifiedChanged);

    QObject::connect(p->doc, &ZDocument::lineMarkerChanged, this, [this](const ZDocumentLineMarker *marker) {
        auto *const p = tuiwidgets_impl();
        if (marker == &p->scrollPositionLine) {
            // Recalculate the scroll position:
            //  * In case any editing from outside of this class moved our scroll position line marker to ensure
            //    that the line marker still keeps the cursor visible
            // Internal changes trigger these signals as well, that should be safe, as long as adjustScrollPosition
            // does not change the scroll position when called again as long as neither document nor the cursor is
            // changed inbetween.
            adjustScrollPosition();
        }
    });

    QObject::connect(p->doc, &ZDocument::cursorChanged, this, [this](const ZDocumentCursor *changedCursor) {
        auto *const p = tuiwidgets_impl();
        if (changedCursor == &p->cursor) {
            // Ensure that even if editing from outside of this class moved the cursor position it is still in the
            // visible portion of the widget.
            adjustScrollPosition();

            // this is our main way to notify other components of cursor position changes.
            emitCursorPostionChanged();
        }
    });
}

ZTextEdit::~ZTextEdit() {
}

void ZTextEdit::registerCommandNotifiers(Qt::ShortcutContext context) {
    auto *const p = tuiwidgets_impl();

    if (p->cmdCopy) {
        // exit if already done
        return;
    }

    p->cmdCopy = new ZCommandNotifier("Copy", this, context);
    QObject::connect(p->cmdCopy, &ZCommandNotifier::activated, this, &ZTextEdit::copy);
    p->cmdCopy->setEnabled(canCopy());

    p->cmdCut = new ZCommandNotifier("Cut", this, context);
    QObject::connect(p->cmdCut, &ZCommandNotifier::activated, this, &ZTextEdit::cut);
    p->cmdCut->setEnabled(canCut());

    p->cmdPaste = new ZCommandNotifier("Paste", this, context);
    QObject::connect(p->cmdPaste, &ZCommandNotifier::activated, this, &ZTextEdit::paste);
    p->updatePasteCommandEnabled();

    ZClipboard *clipboard = findFacet<ZClipboard>();
    if (clipboard) {
        QObject::connect(clipboard, &ZClipboard::contentsChanged, this, [this] {
            tuiwidgets_impl()->updatePasteCommandEnabled();
        });
    }

    p->cmdUndo = new ZCommandNotifier("Undo", this, context);
    QObject::connect(p->cmdUndo, &ZCommandNotifier::activated, this, &ZTextEdit::undo);
    p->cmdUndo->setEnabled(!p->readOnly && p->undoRedoEnabled && p->doc->isUndoAvailable());
    QObject::connect(p->doc, &ZDocument::undoAvailable, this, [this](bool available) {
        auto *const p = tuiwidgets_impl();
        p->cmdUndo->setEnabled(!p->readOnly && p->undoRedoEnabled && available);
    });

    p->cmdRedo = new ZCommandNotifier("Redo", this, context);
    QObject::connect(p->cmdRedo, &ZCommandNotifier::activated, this, &ZTextEdit::redo);
    p->cmdRedo->setEnabled(!p->readOnly && p->undoRedoEnabled && p->doc->isRedoAvailable());
    QObject::connect(p->doc, &ZDocument::redoAvailable, this, [this](bool available) {
        auto *const p = tuiwidgets_impl();
        p->cmdRedo->setEnabled(!p->readOnly && p->undoRedoEnabled && available);
    });
}

void ZTextEdit::emitCursorPostionChanged() {
    auto *const p = tuiwidgets_impl();

    const auto [cursorCodeUnit, cursorLine] = p->cursor.position();
    ZTextLayout layNoWrap = textLayoutForLineWithoutWrapping(cursorLine);
    int cursorColumn = layNoWrap.lineAt(0).cursorToX(cursorCodeUnit, ZTextLayout::Leading);
    int utf8CodeUnit = p->doc->line(cursorLine).leftRef(cursorCodeUnit).toUtf8().size();
    cursorPositionChanged(cursorColumn, cursorCodeUnit, utf8CodeUnit, cursorLine);
}

void ZTextEdit::setCursorPosition(Position position, bool extendSelection) {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.setPosition(position, extendSelection);

    updateCommands();
    adjustScrollPosition();
    update();
}

ZDocumentCursor::Position ZTextEdit::cursorPosition() const {
    auto *const p = tuiwidgets_impl();

    return p->cursor.position();
}

void ZTextEdit::setAnchorPosition(Position position) {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.setAnchorPosition(position);

    updateCommands();
    update();
}

ZDocumentCursor::Position ZTextEdit::anchorPosition() const {
    auto *const p = tuiwidgets_impl();

    return p->cursor.anchor();
}

void ZTextEdit::setSelection(Position anchor, Position position) {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.setAnchorPosition(anchor);
    p->cursor.setPosition(position, true);

    updateCommands();
    adjustScrollPosition();
    update();
}

void ZTextEdit::setTextCursor(const ZDocumentCursor &cursor) {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.setAnchorPosition(cursor.anchor());
    p->cursor.setPositionPreservingVerticalMovementColumn(cursor.position(), true);
    p->cursor.setVerticalMovementColumn(cursor.verticalMovementColumn());

    updateCommands();
    adjustScrollPosition();
    update();
}

ZDocumentCursor ZTextEdit::textCursor() const {
    auto *const p = tuiwidgets_impl();

    return p->cursor;
}

ZDocumentCursor ZTextEdit::makeCursor() {
    auto *const p = tuiwidgets_impl();

    return p->makeCursor();
}

ZDocumentCursor ZTextEditPrivate::makeCursor() {
    return ZDocumentCursor(doc, [this](int line, bool wrappingAllowed) {
        ZTextLayout lay(textMetrics, doc->line(line));
        ZTextOption option;
        option.setTabStopDistance(tabsize);
        if (wrappingAllowed) {
            option.setWrapMode(wrapMode);
            lay.setTextOption(option);
            lay.doLayout(std::max(pub()->rect().width() - pub()->allBordersWidth(), 0));
        } else {
            lay.setTextOption(option);
            lay.doLayout(std::numeric_limits<unsigned short>::max() - 1);
        }
        return lay;
    });
}

ZDocument *ZTextEdit::document() const {
    auto *const p = tuiwidgets_impl();

    return p->doc;
}

int ZTextEdit::lineNumberBorderWidth() const {
    auto *const p = tuiwidgets_impl();

    if (p->showLineNumbers) {
        return QString::number(p->doc->lineCount()).size() + 1;
    }
    return 0;
}

void ZTextEdit::setTabStopDistance(int tab) {
    auto *const p = tuiwidgets_impl();

    p->tabsize = std::max(1, tab);
    adjustScrollPosition();
    update();
}

int ZTextEdit::tabStopDistance() const {
    auto *const p = tuiwidgets_impl();

    return p->tabsize;
}

void ZTextEdit::setShowLineNumbers(bool show) {
    auto *const p = tuiwidgets_impl();

    p->showLineNumbers = show;
    adjustScrollPosition();
    update();
}

bool ZTextEdit::showLineNumbers() const {
    auto *const p = tuiwidgets_impl();

    return p->showLineNumbers;
}

void ZTextEdit::setUseTabChar(bool tab) {
    auto *const p = tuiwidgets_impl();

    p->useTabChar = tab;
    update();
}

bool ZTextEdit::useTabChar() const {
    auto *const p = tuiwidgets_impl();

    return p->useTabChar;
}

void ZTextEdit::setWordWrapMode(ZTextOption::WrapMode wrap) {
    auto *const p = tuiwidgets_impl();

    p->wrapMode = wrap;
    if (p->wrapMode != ZTextOption::WrapMode::NoWrap) {
        p->scrollPositionColumn = 0;
    }
    adjustScrollPosition();
    update();
}

ZTextOption::WrapMode ZTextEdit::wordWrapMode() const {
    auto *const p = tuiwidgets_impl();

    return p->wrapMode;
}

void ZTextEdit::setOverwriteMode(bool mode) {
    auto *const p = tuiwidgets_impl();

    if (p->overwriteMode != mode) {
        p->overwriteMode = mode;
        if (p->overwriteMode) {
            setCursorStyle(p->overwriteCursorStyle);
        } else {
            setCursorStyle(p->insertCursorStyle);
        }
        overwriteModeChanged(p->overwriteMode);
        update();
    }
}

void ZTextEdit::toggleOverwriteMode() {
    setOverwriteMode(!overwriteMode());
}

bool ZTextEdit::overwriteMode() const {
    auto *const p = tuiwidgets_impl();

    return p->overwriteMode;
}

void ZTextEdit::setSelectMode(bool mode) {
    auto *const p = tuiwidgets_impl();

    if (p->selectMode != mode) {
        p->selectMode = mode;
        selectModeChanged(p->selectMode);
    }
}

void ZTextEdit::toggleSelectMode() {
    auto *const p = tuiwidgets_impl();

    setSelectMode(!p->selectMode);
}

bool ZTextEdit::selectMode() const {
    auto *const p = tuiwidgets_impl();

    return p->selectMode;
}

void ZTextEdit::setInsertCursorStyle(CursorStyle style) {
    auto *const p = tuiwidgets_impl();

    p->insertCursorStyle = style;
    if (!p->overwriteMode) {
        setCursorStyle(p->insertCursorStyle);
    }
    update();
}

CursorStyle ZTextEdit::insertCursorStyle() const {
    auto *const p = tuiwidgets_impl();

    return p->insertCursorStyle;
}

void ZTextEdit::setOverwriteCursorStyle(CursorStyle style) {
    auto *const p = tuiwidgets_impl();

    p->overwriteCursorStyle = style;
    if (p->overwriteMode) {
        setCursorStyle(p->overwriteCursorStyle);
    }
    update();
}

CursorStyle ZTextEdit::overwriteCursorStyle() const {
    auto *const p = tuiwidgets_impl();

    return p->overwriteCursorStyle;
}

void ZTextEdit::setTabChangesFocus(bool enabled) {
    auto *const p = tuiwidgets_impl();

    p->tabChangesFocus = enabled;
}

bool ZTextEdit::tabChangesFocus() const {
    auto *const p = tuiwidgets_impl();

    return p->tabChangesFocus;
}

void ZTextEdit::setReadOnly(bool readOnly) {
    auto *const p = tuiwidgets_impl();

    p->readOnly = readOnly;
}

bool ZTextEdit::isReadOnly() const {
    auto *const p = tuiwidgets_impl();

    return p->readOnly;
}

void ZTextEdit::setUndoRedoEnabled(bool enabled) {
    auto *const p = tuiwidgets_impl();

    p->undoRedoEnabled = enabled;
}

bool ZTextEdit::isUndoRedoEnabled() const {
    auto *const p = tuiwidgets_impl();

    return p->undoRedoEnabled;
}

bool ZTextEdit::isModified() const {
    auto *const p = tuiwidgets_impl();

    return p->doc->isModified();
}

void ZTextEdit::insertText(const QString &str) {
    auto *const p = tuiwidgets_impl();

    p->cursor.insertText(str);

    updateCommands();
    adjustScrollPosition();
    update();
}

void ZTextEdit::insertTabAt(ZDocumentCursor &cur) {
    auto *const p = tuiwidgets_impl();

    auto undoGroup = p->doc->startUndoGroup(&p->cursor);

    if (useTabChar()) {
        cur.insertText(QStringLiteral("\t"));
    } else {
        ZTextLayout lay = textLayoutForLineWithoutWrapping(cur.position().line);
        ZTextLineRef tlr = lay.lineAt(0);
        const int colum = tlr.cursorToX(cur.position().codeUnit, ZTextLayout::Leading);
        const int remainingTabWidth = tabStopDistance() - colum % tabStopDistance();
        cur.insertText(QStringLiteral(" ").repeated(remainingTabWidth));
    }
    adjustScrollPosition();
    update();
}

void ZTextEdit::cut() {
    copy();
    removeSelectedText();
    updateCommands();
    adjustScrollPosition();
    update();
}

void ZTextEdit::copy() {
    auto *const p = tuiwidgets_impl();

    if (p->cursor.hasSelection()) {
        ZClipboard *clipboard = findFacet<ZClipboard>();
        if (clipboard) {
            clipboard->setContents(p->cursor.selectedText());
        }
    }
}

void ZTextEdit::pasteEvent(ZPasteEvent *event) {
    auto *const p = tuiwidgets_impl();

    QString text = event->text();

    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QString(QLatin1Char('\n')));

    // Inserting might adjust the scroll position, so save it here and restore it later.
    const int line = p->scrollPositionLine.line();
    p->cursor.insertText(text);
    p->scrollPositionLine.setLine(line);

    p->doc->clearCollapseUndoStep();
    adjustScrollPosition();
    update();
}

void ZTextEdit::paste() {
    auto *const p = tuiwidgets_impl();

    auto undoGroup = p->doc->startUndoGroup(&p->cursor);
    ZClipboard *clipboard = findFacet<ZClipboard>();
    if (clipboard && clipboard->contents().size()) {
        // Inserting might adjust the scroll position, so save it here and restore it later.
        const int line = p->scrollPositionLine.line();
        p->cursor.insertText(clipboard->contents());
        p->scrollPositionLine.setLine(line);
        adjustScrollPosition();
        updateCommands();
        p->doc->clearCollapseUndoStep();
        update();
    }
}

ZDocument::UndoGroup ZTextEdit::startUndoGroup() {
    auto *const p = tuiwidgets_impl();

    return p->doc->startUndoGroup(&p->cursor);
}

void ZTextEdit::removeSelectedText() {
    auto *const p = tuiwidgets_impl();

    if (p->cursor.hasSelection()) {
        auto undoGroup = p->doc->startUndoGroup(&p->cursor);
        p->cursor.removeSelectedText();
    }

    adjustScrollPosition();
    updateCommands();
    update();
}

void ZTextEdit::clearSelection() {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.clearSelection();
    setSelectMode(false);

    updateCommands();
    update();
}

void ZTextEdit::selectAll() {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    p->cursor.selectAll();
    updateCommands();
    adjustScrollPosition();
    update();
}

QString ZTextEdit::selectedText() const {
    auto *const p = tuiwidgets_impl();

    return p->cursor.selectedText();
}

bool ZTextEdit::hasSelection() const {
    auto *const p = tuiwidgets_impl();

    return p->cursor.hasSelection();
}

void ZTextEdit::undo() {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    setSelectMode(false);
    p->doc->undo(&p->cursor);
    adjustScrollPosition();
    update();
}

void ZTextEdit::redo() {
    auto *const p = tuiwidgets_impl();

    clearAdvancedSelection();

    setSelectMode(false);
    p->doc->redo(&p->cursor);
    adjustScrollPosition();
    update();
}

void ZTextEdit::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();


    ZColor fg;
    ZColor bg;

    if (focus()) {
        bg = getColor("textedit.focused.bg");
        fg = getColor("textedit.focused.fg");
    } else if (!isEnabled()) {
        bg = getColor("textedit.disabled.bg");
        fg = getColor("textedit.disabled.fg");
    } else {
        bg = getColor("textedit.bg");
        fg = getColor("textedit.fg");
    }
    const ZTextStyle base{fg, bg};

    ZColor lineNumberFg;
    ZColor lineNumberBg;

    if (focus()) {
        lineNumberFg = getColor("textedit.focused.linenumber.fg");
        lineNumberBg = getColor("textedit.focused.linenumber.bg");
    } else {
        lineNumberFg = getColor("textedit.linenumber.fg");
        lineNumberBg = getColor("textedit.linenumber.bg");
    }

    const ZTextStyle selected{getColor("textedit.selected.fg"),
                                   getColor("textedit.selected.bg"),
                                   ZTextAttribute::Bold};


    auto *painter = event->painter();
    painter->clear(fg, bg);

    setCursorColor(fg.redOrGuess(), fg.greenOrGuess(), fg.blueOrGuess());

    ZTextOption option = textOption();

    Position selectionStartPos(-1, -1);
    Position selectionEndPos(-1, -1);

    if (p->cursor.hasSelection()) {
        selectionStartPos = p->cursor.selectionStartPos();
        selectionEndPos = p->cursor.selectionEndPos();
    }

    const auto [cursorCodeUnit, cursorLine] = p->cursor.position();

    int y = -p->scrollPositionFineLine;
    for (int line = p->scrollPositionLine.line(); y < rect().height() && line < p->doc->lineCount(); line++) {
        QVector<ZFormatRange> highlights;

        ZTextLayout lay = textLayoutForLine(option, line);

        if (line > selectionStartPos.line && line < selectionEndPos.line) {
            // whole line
            highlights.append(ZFormatRange{0, p->doc->lineCodeUnits(line), selected, selected});
        } else if (line > selectionStartPos.line && line == selectionEndPos.line) {
            // selection ends on this line
            highlights.append(ZFormatRange{0, selectionEndPos.codeUnit, selected, selected});
        } else if (line == selectionStartPos.line && line < selectionEndPos.line) {
            // selection starts on this line
            highlights.append(ZFormatRange{selectionStartPos.codeUnit,
                                                p->doc->lineCodeUnits(line) - selectionStartPos.codeUnit, selected, selected});
        } else if (line == selectionStartPos.line && line == selectionEndPos.line) {
            // selection is contained in this line
            highlights.append(ZFormatRange{selectionStartPos.codeUnit,
                                                selectionEndPos.codeUnit - selectionStartPos.codeUnit, selected, selected});
        }

        const bool lineBreakSelected = selectionStartPos.line <= line && selectionEndPos.line > line;

        if (lineBreakSelected) {
            ZTextLineRef lastLine = lay.lineAt(lay.lineCount() - 1);
            const int lineEndX = -p->scrollPositionColumn + lastLine.width() + allBordersWidth();
            painter->clearRect(lineEndX, y + lastLine.y(),
                               rect().width() - lineEndX, 1,
                               selected.foregroundColor(), selected.backgroundColor());
        }

        lay.draw(*painter, {-p->scrollPositionColumn + allBordersWidth(), y}, base, &base, highlights);

        if (cursorLine == line) {
            if (focus()) {
                lay.showCursor(*painter, {-p->scrollPositionColumn + allBordersWidth(), y}, cursorCodeUnit);
            }
        }

        if (p->showLineNumbers) {
            for (int i = lay.lineCount() - 1; i > 0; i--) {
                painter->writeWithColors(0, y + i,
                                         QStringLiteral(" ").repeated(lineNumberBorderWidth()),
                                         lineNumberFg, lineNumberBg);
            }
            QString numberText = QString::number(line + 1)
                    + QStringLiteral(" ").repeated(allBordersWidth() - QString::number(line + 1).size());
            int lineNumberY = y;
            if (y < 0) {
                numberText.replace(QLatin1Char(' '), QLatin1Char('^'));
                lineNumberY = 0;
            }
            if (line == cursorLine) {
                painter->writeWithAttributes(0, lineNumberY, numberText, lineNumberFg, lineNumberBg, ZTextAttribute::Bold);
            } else {
                painter->writeWithColors(0, lineNumberY, numberText, lineNumberFg, lineNumberBg);
            }
        }
        y += lay.lineCount();
    }
}


void ZTextEdit::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();

    auto undoGroup = p->doc->startUndoGroup(&p->cursor);

    QString text = event->text();

    if(event->key() == Key_Space && event->modifiers() == 0) {
        text = QStringLiteral(" ");
    }

    const bool editable = !p->readOnly;
    const bool undoredo = editable && p->undoRedoEnabled;

    if (editable && event->key() == Key_Backspace && event->modifiers() == 0) {
        p->detachedScrolling = false;
        setSelectMode(false);
        p->cursor.deletePreviousCharacter();
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && event->key() == Key_Backspace && event->modifiers() == ControlModifier) {
        p->detachedScrolling = false;
        setSelectMode(false);
        p->cursor.deletePreviousWord();
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && event->key() == Key_Delete && event->modifiers() == 0) {
        p->detachedScrolling = false;
        setSelectMode(false);
        p->cursor.deleteCharacter();
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && event->key() == Key_Delete && event->modifiers() == ControlModifier) {
        p->detachedScrolling = false;
        setSelectMode(false);
        p->cursor.deleteWord();
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && text.size() && event->modifiers() == 0) {
        p->detachedScrolling = false;
        setSelectMode(false);
        // Inserting might adjust the scroll position, so save it here and restore it later.
        const int line = p->scrollPositionLine.line();
        if (overwriteMode()) {
            p->cursor.overwriteText(text);
        } else {
            p->cursor.insertText(text);
        }
        p->scrollPositionLine.setLine(line);
        adjustScrollPosition();
        updateCommands();
        update();
    } else if (event->key() == Qt::Key_Left && (event->modifiers() == 0 || event->modifiers() == ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveCharacterLeft(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Left
               && (event->modifiers() == ControlModifier || event->modifiers() == (ControlModifier | ShiftModifier))) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveWordLeft(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Right && (event->modifiers() == 0 || event->modifiers() == ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveCharacterRight(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Right
               && (event->modifiers() == ControlModifier || event->modifiers() == (ControlModifier | ShiftModifier))) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveWordRight(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Down && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveDown(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Up && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveUp(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Home && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;

        if (p->cursor.atLineStart()) {
            p->cursor.moveToStartIndentedText(extendSelection);
        } else {
            p->cursor.moveToStartOfLine(extendSelection);
        }
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_Home
               && (event->modifiers() == Qt::ControlModifier || event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveToStartOfDocument(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_End && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveToEndOfLine(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_End
               && (event->modifiers() == Qt::ControlModifier || (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)))) {
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        p->cursor.moveToEndOfDocument(extendSelection);
        updateCommands();
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_PageDown && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        // Note: Shift+PageUp/Down does not work with xterm's default settings.
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        const int amount = pageNavigationLineCount();
        for (int i = 0; i < amount; i++) {
            p->cursor.moveDown(extendSelection);
        }
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (event->key() == Qt::Key_PageUp && (event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier)) {
        // Note: Shift+PageUp/Down does not work with xterm's default settings.
        p->detachedScrolling = false;
        clearAdvancedSelection();

        const bool extendSelection = event->modifiers() & Qt::ShiftModifier || p->selectMode;
        const int amount = pageNavigationLineCount();
        for (int i = 0; i < amount; i++) {
            p->cursor.moveUp(extendSelection);
        }
        adjustScrollPosition();
        update();
        p->doc->clearCollapseUndoStep();
    } else if (editable && event->key() == Qt::Key_Enter && (event->modifiers() & ~Qt::KeypadModifier) == 0) {
        p->detachedScrolling = false;
        setSelectMode(false);
        clearAdvancedSelection();

        // Inserting might adjust the scroll position, so save it here and restore it later.
        const int line = p->scrollPositionLine.line();
        p->cursor.insertText(QStringLiteral("\n"));
        p->scrollPositionLine.setLine(line);
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && !p->tabChangesFocus && event->key() == Qt::Key_Tab && event->modifiers() == 0) {
        p->detachedScrolling = false;
        if (p->cursor.hasSelection()) {
            // Add one level of indent to the selected lines.

            const auto [firstLine, lastLine] = p->getSelectedLinesSort();
            const auto [startLine, endLine] = p->getSelectedLines();

            ZDocumentCursor cur = makeCursor();

            for (int line = firstLine; line <= lastLine; line++) {
                // Don't create new trailing whitespace only lines
                if (p->doc->lineCodeUnits(line) > 0) {
                    if (useTabChar()) {
                        cur.setPosition({0, line});
                        cur.insertText(QStringLiteral("\t"));
                    } else {
                        cur.setPosition({0, line});
                        cur.insertText(QStringLiteral(" ").repeated(tabStopDistance()));
                    }
                }
            }

            p->selectLines(startLine, endLine);
        } else {
            // a normal tab
            setSelectMode(false);
            insertTabAt(p->cursor);
        }
        updateCommands();
        adjustScrollPosition();
        update();
    } else if (editable && !p->tabChangesFocus && event->key() == Qt::Key_Tab && event->modifiers() == Qt::ShiftModifier) {
        p->detachedScrolling = false;
        // returns current line if no selection is active
        const auto [firstLine, lastLine] = p->getSelectedLinesSort();
        const auto [startLine, endLine] = p->getSelectedLines();
        const auto [cursorCodeUnit, cursorLine] = p->cursor.position();

        const bool reselect = p->cursor.hasSelection();

        int cursorAdjust = 0;

        for (int line = firstLine; line <= lastLine; line++) {
            int codeUnitsToRemove = 0;
            if (p->doc->lineCodeUnits(line) && p->doc->line(line)[0] == QLatin1Char('\t')) {
                codeUnitsToRemove = 1;
            } else {
                while (true) {
                    if (codeUnitsToRemove < p->doc->lineCodeUnits(line)
                        && codeUnitsToRemove < tabStopDistance()
                        && p->doc->line(line)[codeUnitsToRemove] == QLatin1Char(' ')) {
                        codeUnitsToRemove++;
                    } else {
                        break;
                    }
                }
            }

            p->cursor.setPosition({0, line});
            p->cursor.setPosition({codeUnitsToRemove, line}, true);
            p->cursor.removeSelectedText();
            if (line == cursorLine) {
                cursorAdjust = codeUnitsToRemove;
            }
        }

        // Update cursor / recreate selection
        if (!reselect) {
            setCursorPosition({cursorCodeUnit - cursorAdjust, cursorLine});
        } else {
            p->selectLines(startLine, endLine);
        }
        updateCommands();
        adjustScrollPosition();
        update();
    } else if ((event->text() == QStringLiteral("c") && event->modifiers() == Qt::ControlModifier) ||
               (event->key() == Qt::Key_Insert && event->modifiers() == Qt::ControlModifier) ) {
        // Ctrl + C and Ctrl + Insert
        p->detachedScrolling = false;
        copy();
    } else if (editable && ((event->text() == QStringLiteral("v") && event->modifiers() == Qt::ControlModifier) ||
               (event->key() == Qt::Key_Insert && event->modifiers() == Qt::ShiftModifier))) {
        // Ctrl + V and Shift + Insert
        p->detachedScrolling = false;
        setSelectMode(false);
        paste();
    } else if (editable && ((event->text() == QStringLiteral("x") && event->modifiers() == Qt::ControlModifier) ||
               (event->key() == Qt::Key_Delete && event->modifiers() == Qt::ShiftModifier))) {
        // Ctrl + X and Shift + Delete
        p->detachedScrolling = false;
        setSelectMode(false);
        cut();
    } else if (undoredo && event->text() == QStringLiteral("z") && event->modifiers() == Qt::ControlModifier) {
        p->detachedScrolling = false;
        undoGroup.closeGroup();
        undo();
    } else if (undoredo && event->text() == QStringLiteral("y") && event->modifiers() == Qt::ControlModifier) {
        p->detachedScrolling = false;
        undoGroup.closeGroup();
        redo();
    } else if (event->text() == QStringLiteral("a") && event->modifiers() == Qt::ControlModifier) {
        // Ctrl + a
        selectAll();
        p->doc->clearCollapseUndoStep();
    } else if (editable && event->key() == Qt::Key_Insert && event->modifiers() == 0) {
        p->detachedScrolling = false;
        toggleOverwriteMode();
    } else if (event->key() == Qt::Key_F4 && event->modifiers() == 0) {
        p->detachedScrolling = false;
        toggleSelectMode();
    } else {
        undoGroup.closeGroup();
        ZWidget::keyEvent(event);
    }
}

int ZTextEdit::scrollPositionLine() const {
    auto *const p = tuiwidgets_impl();

    return p->scrollPositionLine.line();
}

int ZTextEdit::scrollPositionColumn() const {
    auto *const p = tuiwidgets_impl();

    return p->scrollPositionColumn;
}

int ZTextEdit::scrollPositionFineLine() const {
    auto *const p = tuiwidgets_impl();

    return p->scrollPositionFineLine;
}

void ZTextEdit::setScrollPosition(int column, int line, int fineLine) {
    auto *const p = tuiwidgets_impl();

    if (column < 0 || line < 0 || fineLine < 0) {
        return;
    }

    if (p->scrollPositionColumn != column || p->scrollPositionLine.line() != line || p->scrollPositionFineLine != fineLine) {

        if (line >= p->doc->lineCount()) {
            return;
        }

        if (fineLine > 0) {
            ZTextLayout lay = textLayoutForLine(textOption(), line);
            if (fineLine >= lay.lineCount()) {
                return;
            }
        }

        p->scrollPositionColumn = column;
        p->scrollPositionLine.setLine(line);
        p->scrollPositionFineLine = fineLine;
        scrollPositionChanged(column, line, fineLine);
        update();
    }
}

int ZTextEdit::pageNavigationLineCount() const {
    return std::max(1, geometry().height() - 1);
}

ZDocumentCursor ZTextEdit::findSync(const QString &subString,
                                         ZDocument::FindFlags options)
{
    auto *const p = tuiwidgets_impl();

    ZDocumentCursor cur = p->doc->findSync(subString, textCursor(), options);
    if (cur.hasSelection()) {
        if (p->selectMode) {
            if (options & FindFlag::FindBackward) {
                setCursorPosition(cur.anchor(), true);
            } else {
                setCursorPosition(cur.position(), true);
            }
        } else {
            setTextCursor(cur);
        }
    } else {
        clearSelection();
    }
    return cur;
}

ZDocumentCursor ZTextEdit::findSync(const QRegularExpression &regex,
                                         ZDocument::FindFlags options)
{
    auto *const p = tuiwidgets_impl();

    ZDocumentCursor cur = p->doc->findSync(regex, textCursor(), options);
    if (cur.hasSelection()) {
        if (p->selectMode) {
            if (options & FindFlag::FindBackward) {
                setCursorPosition(cur.anchor(), true);
            } else {
                setCursorPosition(cur.position(), true);
            }
        } else {
            setTextCursor(cur);
        }
    } else {
        clearSelection();
    }
    return cur;
}

ZDocumentFindResult ZTextEdit::findSyncWithDetails(const QRegularExpression &regex, FindFlags options)
{
    auto *const p = tuiwidgets_impl();

    ZDocumentFindResult res = p->doc->findSyncWithDetails(regex, textCursor(), options);
    if (res.cursor().hasSelection()) {
        if (p->selectMode) {
            if (options & FindFlag::FindBackward) {
                setCursorPosition(res.cursor().anchor(), true);
            } else {
                setCursorPosition(res.cursor().position(), true);
            }
        } else {
            setTextCursor(res.cursor());
        }
    } else {
        clearSelection();
    }
    return res;
}


QFuture<ZDocumentFindAsyncResult> ZTextEditPrivate::connectAsyncFindCommon(QFuture<ZDocumentFindAsyncResult> res,
                                                                           ZTextEdit::FindFlags options) {
    // We wrap the future in another future here to ensure that the internal processing is done when the future
    // we return to the user is triggered.
    // If we would return the original future here, then waiting on it would not ensure that the cursor changes
    // are already done when the user code is signaled.
    // BUT: now the returned future will only be signaled when the search is done and the owning thread of this
    // widget is processing messages. But otherwise using a async search and then blocking the thread on that
    // future seems like a case that does not need to be supported.
    QFutureInterface<ZDocumentFindAsyncResult> wrappingPromise;
    QFuture<ZDocumentFindAsyncResult> wrappingFuture = wrappingPromise.future();
    wrappingPromise.reportStarted();

    auto cancelWatcher = new QFutureWatcher<ZDocumentFindAsyncResult>();
    cancelWatcher->setFuture(wrappingFuture);


    auto watcher = new QFutureWatcher<ZDocumentFindAsyncResult>();
    QObject::connect(cancelWatcher, &QFutureWatcher<ZDocumentFindAsyncResult>::canceled,
                     watcher, &QFutureWatcher<ZDocumentFindAsyncResult>::cancel);

    QObject::connect(watcher, &QFutureWatcher<ZDocumentFindAsyncResult>::finished, pub(),
                     [this, watcher, cancelWatcher, options, wrappingPromise]() mutable {
        if (!watcher->isCanceled()) {
            ZDocumentFindAsyncResult res = watcher->future().result();
            if (res.anchor() != res.cursor()) { // has a match?
                if (selectMode) {
                    if (options & ZTextEdit::FindFlag::FindBackward) {
                        pub()->setCursorPosition(res.anchor(), true);
                    } else {
                        pub()->setCursorPosition(res.cursor(), true);
                    }
                } else {
                    pub()->setSelection(res.anchor(), res.cursor());
                }

            } else {
                pub()->clearSelection();
            }
            pub()->updateCommands();
            pub()->adjustScrollPosition();
            wrappingPromise.reportResult(res);
            wrappingPromise.reportFinished();
        } else {
            wrappingPromise.reportCanceled();
            wrappingPromise.reportFinished();
        }
        watcher->deleteLater();
        cancelWatcher->deleteLater();
    });

    watcher->setFuture(res);
    return wrappingFuture;
}

QFuture<ZDocumentFindAsyncResult> ZTextEdit::findAsync(const QString &subString, ZDocument::FindFlags options)
{
    auto *const p = tuiwidgets_impl();

    QFuture<ZDocumentFindAsyncResult> res = p->doc->findAsync(subString, textCursor(), options);
    return p->connectAsyncFindCommon(res, options);
}

QFuture<ZDocumentFindAsyncResult> ZTextEdit::findAsync(const QRegularExpression &regex, FindFlags options) {
    auto *const p = tuiwidgets_impl();

    QFuture<ZDocumentFindAsyncResult> res = p->doc->findAsync(regex, textCursor(), options);
    return p->connectAsyncFindCommon(res, options);
}

QFuture<ZDocumentFindAsyncResult> ZTextEdit::findAsyncWithPool(QThreadPool *pool, int priority,
                                                                    const QString &subString, FindFlags options) {
    auto *const p = tuiwidgets_impl();

    QFuture<ZDocumentFindAsyncResult> res = p->doc->findAsyncWithPool(pool, priority, subString, textCursor(), options);
    return p->connectAsyncFindCommon(res, options);
}

QFuture<ZDocumentFindAsyncResult> ZTextEdit::findAsyncWithPool(QThreadPool *pool,
                                                                    int priority,
                                                                    const QRegularExpression &regex, FindFlags options) {
    auto *const p = tuiwidgets_impl();

    QFuture<ZDocumentFindAsyncResult> res = p->doc->findAsyncWithPool(pool, priority, regex, textCursor(), options);
    return p->connectAsyncFindCommon(res, options);
}

void ZTextEdit::clear() {
    auto *const p = tuiwidgets_impl();

    p->doc->reset();
    p->scrollPositionLine.setLine(0);
    p->scrollPositionColumn = 0;
    p->scrollPositionFineLine = 0;

    cursorPositionChanged(0, 0, 0, 0);
    scrollPositionChanged(0, 0, 0);
    setSelectMode(false);
    update();
}

void ZTextEdit::readFrom(QIODevice *file) {
    readFrom(file, {0, 0});
}

void ZTextEdit::readFrom(QIODevice *file, Position initialPosition) {
    auto *const p = tuiwidgets_impl();

    p->doc->readFrom(file, initialPosition, &p->cursor);
    adjustScrollPosition();
    updateCommands();
    update();
}

void ZTextEdit::writeTo(QIODevice *file) const {
    auto *const p = tuiwidgets_impl();

    p->doc->writeTo(file, p->doc->crLfMode());
    p->doc->markUndoStateAsSaved();
}


ZTextOption ZTextEdit::textOption() const {
    auto *const p = tuiwidgets_impl();

    ZTextOption option;
    option.setWrapMode(p->wrapMode);
    option.setTabStopDistance(p->tabsize);

    return option;
}

ZTextLayout ZTextEdit::textLayoutForLine(const ZTextOption &option, int line) const {
    auto *const p = tuiwidgets_impl();

    ZTextLayout lay(p->textMetrics, p->doc->line(line));
    lay.setTextOption(option);
    if (p->wrapMode != ZTextOption::WrapMode::NoWrap) {
        lay.doLayout(std::max(rect().width() - allBordersWidth(), 0));
    } else {
        lay.doLayout(std::numeric_limits<unsigned short>::max() - 1);
    }
    return lay;
}

ZTextLayout ZTextEdit::textLayoutForLineWithoutWrapping(int line) const {
    ZTextOption option = textOption();
    option.setWrapMode(ZTextOption::NoWrap);
    return textLayoutForLine(option, line);
}

const ZTextMetrics &ZTextEdit::textMetrics() const {
    auto *const p = tuiwidgets_impl();

    return p->textMetrics;
}

void ZTextEdit::resizeEvent(ZResizeEvent *event) {
    if (event->size().height() > 0 && event->size().width() > 0) {
        adjustScrollPosition();
    }
}

void ZTextEdit::adjustScrollPosition() {
    auto *const p = tuiwidgets_impl();


    if (geometry().width() <= 0 && geometry().height() <= 0) {
        return;
    }

    int newScrollPositionColumn = scrollPositionColumn();
    int newScrollPositionLine = scrollPositionLine();
    int newScrollPositionFineLine = scrollPositionFineLine();

    if (p->detachedScrolling) {
        if (newScrollPositionLine >= p->doc->lineCount()) {
            newScrollPositionLine = p->doc->lineCount() - 1;
        }

        setScrollPosition(newScrollPositionColumn, newScrollPositionLine, newScrollPositionFineLine);
        return;
    }

    const auto [cursorCodeUnit, cursorLine] = p->cursor.position();

    int viewWidth = geometry().width() - allBordersWidth();

    // horizontal scroll position
    if (p->wrapMode == ZTextOption::WrapMode::NoWrap) {
        ZTextLayout layNoWrap = textLayoutForLineWithoutWrapping(cursorLine);
        int cursorColumn = layNoWrap.lineAt(0).cursorToX(cursorCodeUnit, ZTextLayout::Leading);

        if (cursorColumn - newScrollPositionColumn >= viewWidth) {
             newScrollPositionColumn = cursorColumn - viewWidth + 1;
        }
        if (cursorColumn > 0) {
            if (cursorColumn - newScrollPositionColumn < 1) {
                newScrollPositionColumn = cursorColumn - 1;
            }
        } else {
            newScrollPositionColumn = 0;
        }
    } else {
        newScrollPositionColumn = 0;
    }

    // vertical scroll position
    if (p->wrapMode == ZTextOption::WrapMode::NoWrap) {
        if (cursorLine >= 0) {
            if (cursorLine - newScrollPositionLine < 1) {
                newScrollPositionLine = cursorLine;
                newScrollPositionFineLine = 0;
            }
        }

        if (cursorLine - newScrollPositionLine >= geometry().height() - 1) {
            newScrollPositionLine = cursorLine - geometry().height() + 2;
        }

        if (p->doc->lineCount() - newScrollPositionLine < geometry().height() - 1) {
            newScrollPositionLine = std::max(0, p->doc->lineCount() - geometry().height() + 1);
        }
    } else {
        ZTextOption option = textOption();

        const int availableLinesAbove = geometry().height() - 2;

        ZTextLayout layCursorLayout = textLayoutForLine(option, cursorLine);
        int linesAbove = layCursorLayout.lineForTextPosition(cursorCodeUnit).lineNumber();

        if (linesAbove >= availableLinesAbove) {
            if (newScrollPositionLine < cursorLine) {
                newScrollPositionLine = cursorLine;
                newScrollPositionFineLine = linesAbove - availableLinesAbove;
            }
            if (newScrollPositionLine == cursorLine) {
                if (newScrollPositionFineLine < linesAbove - availableLinesAbove) {
                    newScrollPositionFineLine = linesAbove - availableLinesAbove;
                }
            }
        } else {
            for (int line = cursorLine - 1; line >= 0; line--) {
                ZTextLayout lay = textLayoutForLine(option, line);
                if (linesAbove + lay.lineCount() >= availableLinesAbove) {
                    if (newScrollPositionLine < line) {
                        newScrollPositionLine = line;
                        newScrollPositionFineLine = (linesAbove + lay.lineCount()) - availableLinesAbove;
                    }
                    if (newScrollPositionLine == line) {
                        if (newScrollPositionFineLine < (linesAbove + lay.lineCount()) - availableLinesAbove) {
                            newScrollPositionFineLine = (linesAbove + lay.lineCount()) - availableLinesAbove;
                        }
                    }
                    break;
                }
                linesAbove += lay.lineCount();
            }
        }

        linesAbove = layCursorLayout.lineForTextPosition(cursorCodeUnit).lineNumber();

        if (newScrollPositionLine == cursorLine) {
            if (linesAbove < newScrollPositionFineLine) {
                newScrollPositionFineLine = linesAbove;
            }
        } else if (newScrollPositionLine > cursorLine) {
            newScrollPositionLine = cursorLine;
            newScrollPositionFineLine = linesAbove;
        }

        // scroll when window is larger than the document shown (unless scrolled to top)
        if (newScrollPositionLine && newScrollPositionLine + (geometry().height() - 1) > p->doc->lineCount()) {
            int linesCounted = 0;

            for (int line = p->doc->lineCount() - 1; line >= 0; line--) {
                ZTextLayout lay = textLayoutForLine(option, line);
                linesCounted += lay.lineCount();
                if (linesCounted >= geometry().height() - 1) {
                    if (newScrollPositionLine > line) {
                        newScrollPositionLine = line;
                        newScrollPositionFineLine = linesCounted - (geometry().height() - 1);
                    } else if (newScrollPositionLine == line &&
                               newScrollPositionFineLine > linesCounted - (geometry().height() - 1)) {
                        newScrollPositionFineLine = linesCounted - (geometry().height() - 1);
                    }
                    break;
                }
            }
        }
    }

    setScrollPosition(newScrollPositionColumn, newScrollPositionLine, newScrollPositionFineLine);

    int max = 0;
    for (int i = newScrollPositionLine; i < p->doc->lineCount() && i < newScrollPositionLine + geometry().height(); i++) {
        if (max < p->doc->lineCodeUnits(i)) {
            max = p->doc->lineCodeUnits(i);
        }
    }
    scrollRangeChanged(std::max(0, max - viewWidth), std::max(0, p->doc->lineCount() - geometry().height()));
}

int ZTextEdit::allBordersWidth() const {
    return lineNumberBorderWidth();
}

bool ZTextEdit::canPaste() {
    ZClipboard *clipboard = findFacet<ZClipboard>();
    if (clipboard) {
        QString _clipboard = clipboard->contents();
        return !_clipboard.isEmpty();
    } else {
        return false;
    }
}

bool ZTextEdit::canCopy() {
    auto *const p = tuiwidgets_impl();

    return p->cursor.hasSelection();
}

bool ZTextEdit::canCut() {
    auto *const p = tuiwidgets_impl();

    return p->cursor.hasSelection();
}

void ZTextEdit::updateCommands() {
    auto *const p = tuiwidgets_impl();

    if (p->cmdCopy) {
        p->cmdCopy->setEnabled(canCopy());
    }
    if (p->cmdCut) {
        p->cmdCut->setEnabled(!p->readOnly && canCut());
    }
}

void ZTextEditPrivate::updatePasteCommandEnabled() {
    if (cmdPaste) {
        cmdPaste->setEnabled(!readOnly && pub()->canPaste());
    }
}

void ZTextEdit::clearAdvancedSelection() {
    // derived classes can override this
}

void ZTextEdit::enableDetachedScrolling() {
    auto *const p = tuiwidgets_impl();

    p->detachedScrolling = true;
}

void ZTextEdit::disableDetachedScrolling() {
    auto *const p = tuiwidgets_impl();

    p->detachedScrolling = false;
}

bool ZTextEdit::isDetachedScrolling() const {
    auto *const p = tuiwidgets_impl();

    return p->detachedScrolling;
}

void ZTextEdit::detachedScrollUp() {
    auto *const p = tuiwidgets_impl();

    if (p->scrollPositionFineLine > 0) {
        enableDetachedScrolling();
        setScrollPosition(p->scrollPositionColumn, p->scrollPositionLine.line(), p->scrollPositionFineLine - 1);
    } else if (p->scrollPositionLine.line() > 0) {
        enableDetachedScrolling();

        int newScrollPositionLine = scrollPositionLine() - 1;
        int newScrollPositionFineLine = 0;

        if (wordWrapMode() != ZTextOption::WrapMode::NoWrap) {
            ZTextLayout lay = textLayoutForLine(textOption(), newScrollPositionLine);
            newScrollPositionFineLine = lay.lineCount() - 1;
        }
        setScrollPosition(p->scrollPositionColumn, newScrollPositionLine, newScrollPositionFineLine);
    }
}

void ZTextEdit::detachedScrollDown() {
    auto *const p = tuiwidgets_impl();

    if (wordWrapMode() != ZTextOption::WrapMode::NoWrap) {
        ZTextLayout lay = textLayoutForLine(textOption(), p->scrollPositionLine.line());
        if (lay.lineCount() - 1 > p->scrollPositionFineLine) {
            setScrollPosition(p->scrollPositionColumn, p->scrollPositionLine.line(), p->scrollPositionFineLine + 1);
            return;
        }
    }

    if (document()->lineCount() - 1 > p->scrollPositionLine.line()) {
        enableDetachedScrolling();
        setScrollPosition(p->scrollPositionColumn, p->scrollPositionLine.line() + 1, 0);
    }
}

QPair<int, int> ZTextEditPrivate::getSelectedLinesSort() {
    auto lines = getSelectedLines();
    return {std::min(lines.first, lines.second), std::max(lines.first, lines.second)};
}

QPair<int, int> ZTextEditPrivate::getSelectedLines() {
    int startY;
    int endY;

    const auto [startCodeUnit, startLine] = cursor.anchor();
    const auto [endCodeUnit, endLine] = cursor.position();
    startY = startLine;
    endY = endLine;

    if (startLine < endLine) {
        if (endCodeUnit == 0) {
            endY--;
        }
    } else if (endLine < startLine) {
        if (startCodeUnit == 0) {
            startY--;
        }
    }

    return {std::max(0, startY), std::max(0, endY)};
}

void ZTextEditPrivate::selectLines(int startLine, int endLine) {
    if (startLine > endLine) {
        cursor.setPosition({doc->lineCodeUnits(startLine), startLine});
        cursor.setPosition({0, endLine}, true);
    } else {
        cursor.setPosition({0, startLine});
        cursor.setPosition({doc->lineCodeUnits(endLine), endLine}, true);
    }
}

bool ZTextEdit::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZTextEdit::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZTextEdit::sizeHint() const {
    return ZWidget::sizeHint();
}

QSize ZTextEdit::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZTextEdit::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZTextEdit::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZTextEdit::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZTextEdit::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZTextEdit::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZTextEdit::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZTextEdit::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZTextEdit::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZTextEdit::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZTextEdit::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZTextEdit::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}

TUIWIDGETS_NS_END
