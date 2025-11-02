// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocumentCursor.h>
#include <Tui/ZDocumentCursor_p.h>

#include <Tui/ZDocument_p.h>

#include <Tui/Utils_p.h>

TUIWIDGETS_NS_START

ZDocumentCursorPrivate::ZDocumentCursorPrivate(ZDocumentCursor *pub, ZDocumentPrivate *doc,
                                               std::function<ZTextLayout(int line, bool wrappingAllowed)> createTextLayout)
    : doc(doc), createTextLayout(createTextLayout), pub_ptr(pub)
{
    doc->registerTextCursor(this);
}

ZDocumentCursorPrivate::~ZDocumentCursorPrivate() {
    doc->unregisterTextCursor(this);
}


ZDocumentCursor::ZDocumentCursor(ZDocument *doc,
                                 std::function<ZTextLayout(int line, bool wrappingAllowed)> createTextLayout)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentCursorPrivate>(this, ZDocumentPrivate::get(doc), createTextLayout))
{
}

ZDocumentCursor::ZDocumentCursor(const ZDocumentCursor &other)
    : tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentCursorPrivate>(this, other.tuiwidgets_impl()->doc,
                                                                    other.tuiwidgets_impl()->createTextLayout))
{
    auto *const p = tuiwidgets_impl();
    auto *const otherP = other.tuiwidgets_impl();
    p->cursorCodeUnit = otherP->cursorCodeUnit;
    p->cursorLine = otherP->cursorLine;
    p->anchorCodeUnit = otherP->anchorCodeUnit;
    p->anchorLine = otherP->anchorLine;
    p->verticalMovementColumn = otherP->verticalMovementColumn;
}

ZDocumentCursor::~ZDocumentCursor() {
}

ZDocumentCursor &ZDocumentCursor::operator=(const ZDocumentCursor &other) {
    auto *const p = tuiwidgets_impl();
    auto *const otherP = other.tuiwidgets_impl();
    if (p->doc != otherP->doc) {
        p->doc->unregisterTextCursor(p);
        p->doc = otherP->doc;
        p->doc->registerTextCursor(p);
        p->scheduleChangeSignal();
    }

    p->createTextLayout = otherP->createTextLayout;

    if (p->cursorCodeUnit != otherP->cursorCodeUnit
        || p->cursorLine != otherP->cursorLine
        || p->anchorCodeUnit != otherP->anchorCodeUnit
        || p->anchorLine != otherP->anchorLine
        || p->verticalMovementColumn != otherP->verticalMovementColumn) {
        p->cursorCodeUnit = otherP->cursorCodeUnit;
        p->cursorLine = otherP->cursorLine;
        p->anchorCodeUnit = otherP->anchorCodeUnit;
        p->anchorLine = otherP->anchorLine;
        p->verticalMovementColumn = otherP->verticalMovementColumn;
        p->scheduleChangeSignal();
    }

    return *this;
}

void ZDocumentCursor::insertText(const QString &text) {
    auto *const p = tuiwidgets_impl();
    auto undoGroup = p->doc->startUndoGroup(this);

    p->doc->prepareModification(this->position());

    auto lines = text.split(QStringLiteral("\n"));

    removeSelectedText();

    if (text.size()) {
        if (p->doc->newlineAfterLastLineMissing && atEnd() && lines.size() > 1 && lines.last().size() == 0) {
            lines.removeLast();
            p->doc->newlineAfterLastLineMissing = false;
        }
        p->doc->insertIntoLine(this, p->cursorLine, p->cursorCodeUnit, lines.front());
        p->cursorCodeUnit += lines.front().size();
        for (int i = 1; i < lines.size(); i++) {
            p->doc->splitLine(this, {p->cursorCodeUnit, p->cursorLine});
            p->cursorLine++;
            p->cursorCodeUnit = 0;
            p->doc->insertIntoLine(this, p->cursorLine, p->cursorCodeUnit, lines.at(i));
            p->cursorCodeUnit = lines.at(i).size();
        }
        p->anchorCodeUnit = p->cursorCodeUnit;
        p->anchorLine = p->cursorLine;
        p->scheduleChangeSignal();
    }

    ZTextLayout lay = p->createTextLayout(p->cursorLine, true);
    p->updateVerticalMovementColumn(lay);

    if (text.size()) {
        p->doc->saveUndoStep(this->position(), true, !text.contains(QLatin1Char(' '))
                                                  && !text.contains(QLatin1Char('\n'))
                                                  && !text.contains(QLatin1Char('\t')));
    }

    p->doc->debugConsistencyCheck(nullptr);
}

void ZDocumentCursor::overwriteText(const QString &text, int clusterCount) {
    auto *const p = tuiwidgets_impl();
    auto undoGroup = p->doc->startUndoGroup(this);

    if (!text.size()) {
        return;
    } else if (!hasSelection()) {
        clusterCount -= text.count(QLatin1Char('\n'));
        if (clusterCount < 0) {
            clusterCount = 0;
        }
        const auto [currentCodeUnit, currentLine] = position();
        if (currentCodeUnit < p->doc->lines[currentLine].chars.size()) {
            ZTextLayout lay = p->createTextLayout(p->cursorLine, false);
            const Position start = {currentCodeUnit, currentLine};
            Position end = {currentCodeUnit, currentLine};
            for (int i = 0; i < clusterCount; i++) {
                end = {lay.nextCursorPosition(end.codeUnit, ZTextLayout::SkipCharacters), end.line};
            }
            p->doc->removeFromLine(this, start.line, start.codeUnit, end.codeUnit - start.codeUnit);
        }
    }

    insertText(text);
}

void ZDocumentCursor::removeSelectedText() {
    auto *const p = tuiwidgets_impl();

    if (!hasSelection()) {
        return;
    }

    p->doc->prepareModification(this->position());

    const Position start = selectionStartPos();
    const Position end = selectionEndPos();

    if (start.line == end.line) {
        // selection only on one line
        p->doc->removeFromLine(this, start.line, start.codeUnit, end.codeUnit - start.codeUnit);
    } else {
        p->doc->removeFromLine(this, start.line, start.codeUnit, p->doc->lines[start.line].chars.size() - start.codeUnit);
        const auto orignalTextLines = p->doc->lines.size();
        if (start.line + 1 < end.line) {
            p->doc->removeLines(this, start.line + 1, end.line - start.line - 1);
        }
        if (end.line == orignalTextLines) {
            // selected until the end of buffer, no last selection line to edit
        } else {
            p->doc->removeFromLine(this, start.line + 1, 0, end.codeUnit);
            p->doc->mergeLines(this, start.line);
        }
    }
    clearSelection();
    setPosition(start);

    p->doc->saveUndoStep(this->position());
    p->doc->debugConsistencyCheck(nullptr);
}

void ZDocumentCursor::clearSelection() {
    auto *const p = tuiwidgets_impl();
    if (p->anchorCodeUnit != p->cursorCodeUnit || p->anchorLine != p->cursorLine) {
        p->anchorCodeUnit = p->cursorCodeUnit;
        p->anchorLine = p->cursorLine;
        p->scheduleChangeSignal();
    }
}

QString ZDocumentCursor::selectedText() const {
    auto *const p = tuiwidgets_impl();

    if (!hasSelection()) {
        return QStringLiteral("");
    }

    const Position start = selectionStartPos();
    const Position end = selectionEndPos();

    if (start.line == end.line) {
        // selection only on one line
        return p->doc->lines[start.line].chars.mid(start.codeUnit, end.codeUnit - start.codeUnit);
    } else {
        QString res = p->doc->lines[start.line].chars.mid(start.codeUnit);
        for (int line = start.line + 1; line < end.line; line++) {
            res += QStringLiteral("\n");
            res += p->doc->lines[line].chars;
        }
        res += QStringLiteral("\n");
        res += p->doc->lines[end.line].chars.mid(0, end.codeUnit);
        return res;
    }
}

void ZDocumentCursor::deleteCharacter() {
    if (!hasSelection()) {
        moveCharacterRight(true);
    }

    removeSelectedText();
}

void ZDocumentCursor::deletePreviousCharacter() {
    if (!hasSelection()) {
        moveCharacterLeft(true);
    }

    removeSelectedText();
}

void ZDocumentCursor::deleteWord() {
    if (!hasSelection()) {
        moveWordRight(true);
    }

    removeSelectedText();
}

void ZDocumentCursor::deletePreviousWord() {
    if (!hasSelection()) {
        moveWordLeft(true);
    }

    removeSelectedText();
}

void ZDocumentCursor::deleteLine() {
    auto *const p = tuiwidgets_impl();
    auto undoGroup = p->doc->startUndoGroup(this);
    clearSelection();
    moveToStartOfLine();
    moveToEndOfLine(true);
    removeSelectedText();
    if (atEnd()) {
        deletePreviousCharacter();
    } else {
        deleteCharacter();
    }
}

void ZDocumentCursor::moveCharacterLeft(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();
    if (currentCodeUnit) {
        ZTextLayout lay = p->createTextLayout(p->cursorLine, false);
        setPosition({lay.previousCursorPosition(currentCodeUnit, ZTextLayout::SkipCharacters), currentLine},
                    extendSelection);
    } else if (currentLine > 0) {
        setPosition({size2int(p->doc->lines[currentLine - 1].chars.size()), currentLine - 1}, extendSelection);
    } else {
        // here we update the selection and the vertical movemend position
        setPosition({currentCodeUnit, currentLine}, extendSelection);
    }
}

void ZDocumentCursor::moveCharacterRight(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();
    if (currentCodeUnit < p->doc->lines[currentLine].chars.size()) {
        ZTextLayout lay = p->createTextLayout(p->cursorLine, false);
        setPosition({lay.nextCursorPosition(currentCodeUnit, ZTextLayout::SkipCharacters), currentLine},
                    extendSelection);
    } else if (currentLine + 1 < p->doc->lines.size()) {
        setPosition({0, currentLine + 1}, extendSelection);
    } else {
        // here we update the selection and the vertical movemend position
        setPosition({currentCodeUnit, currentLine}, extendSelection);
    }
}

void ZDocumentCursor::moveWordLeft(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();
    if (currentCodeUnit) {
        ZTextLayout lay = p->createTextLayout(p->cursorLine, false);
        setPosition({lay.previousCursorPosition(currentCodeUnit, ZTextLayout::SkipWords), currentLine},
                    extendSelection);
    } else if (currentLine > 0) {
        setPosition({size2int(p->doc->lines[currentLine - 1].chars.size()), currentLine - 1}, extendSelection);
    } else {
        // here we update the selection and the vertical movemend position
        setPosition({currentCodeUnit, currentLine}, extendSelection);
    }
}

void ZDocumentCursor::moveWordRight(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();
    if (currentCodeUnit < p->doc->lines[currentLine].chars.size()) {
        ZTextLayout lay = p->createTextLayout(p->cursorLine, false);
        setPosition({lay.nextCursorPosition(currentCodeUnit, ZTextLayout::SkipWords), currentLine},
                    extendSelection);
    } else if (currentLine + 1 < p->doc->lines.size()) {
        setPosition({0, currentLine + 1}, extendSelection);
    } else {
        // here we update the selection and the vertical movemend position
        setPosition({currentCodeUnit, currentLine}, extendSelection);
    }
}

void ZDocumentCursor::moveUp(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();

    ZTextLayout layStarting = p->createTextLayout(currentLine, true);
    ZTextLineRef lineStarting = layStarting.lineForTextPosition(currentCodeUnit);

    if (lineStarting.lineNumber() > 0) {
        int fineMoveCodeUnit = layStarting.lineAt(lineStarting.lineNumber() - 1).xToCursor(p->verticalMovementColumn);
        if (layStarting.lineForTextPosition(fineMoveCodeUnit).lineNumber() != lineStarting.lineNumber() - 1) {
            // When the line is shorter than _saveCursorPositionX the cursor ends up in the next line,
            // which is not intended, move once to the left in that case
            fineMoveCodeUnit = layStarting.previousCursorPosition(fineMoveCodeUnit, ZTextLayout::SkipCharacters);
        }

        setPositionPreservingVerticalMovementColumn({fineMoveCodeUnit, currentLine}, extendSelection);
        return;
    }

    if (currentLine > 0) {
        ZTextLayout lay = p->createTextLayout(currentLine - 1, true);
        ZTextLineRef la = lay.lineAt(lay.lineCount() - 1);
        setPositionPreservingVerticalMovementColumn({la.xToCursor(p->verticalMovementColumn), currentLine - 1},
                                                    extendSelection);
    } else {
        setPositionPreservingVerticalMovementColumn({0, 0}, extendSelection);
    }
}

void ZDocumentCursor::moveDown(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();

    ZTextLayout layStarting = p->createTextLayout(currentLine, true);
    ZTextLineRef lineStarting = layStarting.lineForTextPosition(currentCodeUnit);

    if (lineStarting.lineNumber() + 1 < layStarting.lineCount()) {
        int fineMoveCodeUnit = layStarting.lineAt(lineStarting.lineNumber() + 1).xToCursor(p->verticalMovementColumn);
        if (layStarting.lineForTextPosition(fineMoveCodeUnit).lineNumber() != lineStarting.lineNumber() + 1) {
            // When the line is shorter than _saveCursorPositionX the cursor ends up in the next line,
            // which is not intended, move once to the left in that case
            fineMoveCodeUnit = layStarting.previousCursorPosition(fineMoveCodeUnit, ZTextLayout::SkipCharacters);
        }

        setPositionPreservingVerticalMovementColumn({fineMoveCodeUnit, currentLine}, extendSelection);
        return;
    }

    if (currentLine < p->doc->lines.size() - 1) {
        ZTextLayout lay = p->createTextLayout(currentLine + 1, true);
        ZTextLineRef la = lay.lineAt(0);
        setPositionPreservingVerticalMovementColumn({la.xToCursor(p->verticalMovementColumn), currentLine + 1},
                                                    extendSelection);
    } else {
        int textLength = p->doc->lines[currentLine].chars.size();
        setPositionPreservingVerticalMovementColumn({textLength, currentLine}, extendSelection);
    }
}

void ZDocumentCursor::moveToStartOfLine(bool extendSelection) {
    const auto [currentCodeUnit, currentLine] = position();
    setPosition({0, currentLine}, extendSelection);
}

void ZDocumentCursor::moveToStartIndentedText(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();

    int i = 0;
    for (; i < p->doc->lines[currentLine].chars.size(); i++) {
        if (p->doc->lines[currentLine].chars[i] != QLatin1Char(' ')
                && p->doc->lines[currentLine].chars[i] != QLatin1Char('\t')) {
            break;
        }
    }

    setPosition({i, currentLine}, extendSelection);
}

void ZDocumentCursor::moveToEndOfLine(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    const auto [currentCodeUnit, currentLine] = position();
    setPosition({size2int(p->doc->lines[currentLine].chars.size()), currentLine}, extendSelection);
}

void ZDocumentCursor::moveToStartOfDocument(bool extendSelection) {
    setPosition({0, 0}, extendSelection);
}

void ZDocumentCursor::moveToEndOfDocument(bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    setPosition({size2int(p->doc->lines.last().chars.size()), size2int(p->doc->lines.size()) - 1}, extendSelection);
}

ZDocumentCursor::Position ZDocumentCursor::position() const {
    auto *const p = tuiwidgets_impl();
    return Position{p->cursorCodeUnit, p->cursorLine};
}

void ZDocumentCursor::setPosition(ZDocumentCursor::Position pos, bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    setPositionPreservingVerticalMovementColumn(pos, extendSelection);
    ZTextLayout lay = p->createTextLayout(p->cursorLine, true);
    p->updateVerticalMovementColumn(lay);
}

void ZDocumentCursor::setPositionPreservingVerticalMovementColumn(ZDocumentCursor::Position pos, bool extendSelection) {
    auto *const p = tuiwidgets_impl();
    auto cursorLine = std::max(std::min(pos.line, size2int(p->doc->lines.size() - 1)), 0);
    auto cursorCodeUnit = std::max(std::min(pos.codeUnit, size2int(p->doc->lines[cursorLine].chars.size())), 0);

    // We are not allowed to jump between characters. Therefore, we go once to the left and again to the right.
    if (cursorCodeUnit > 0) {
        ZTextLayout lay = p->createTextLayout(cursorLine, false);
        cursorCodeUnit = lay.previousCursorPosition(cursorCodeUnit, ZTextLayout::SkipCharacters);
        cursorCodeUnit = lay.nextCursorPosition(cursorCodeUnit, ZTextLayout::SkipCharacters);
    }

    if (!extendSelection && (p->anchorCodeUnit != cursorCodeUnit || p->anchorLine != cursorLine)) {
        p->anchorCodeUnit = cursorCodeUnit;
        p->anchorLine = cursorLine;
        p->scheduleChangeSignal();
    }

    if (p->cursorLine != cursorLine || p->cursorCodeUnit != cursorCodeUnit) {
        p->cursorLine = cursorLine;
        p->cursorCodeUnit = cursorCodeUnit;
        p->scheduleChangeSignal();
    }
}

ZDocumentCursor::Position ZDocumentCursor::anchor() const {
    auto *const p = tuiwidgets_impl();
    return Position{p->anchorCodeUnit, p->anchorLine};
}

void ZDocumentCursor::setAnchorPosition(ZDocumentCursor::Position pos) {
    auto *const p = tuiwidgets_impl();
    auto anchorLine = std::max(std::min(pos.line, size2int(p->doc->lines.size()) - 1), 0);
    auto anchorCodeUnit = std::max(std::min(pos.codeUnit, size2int(p->doc->lines[anchorLine].chars.size())), 0);

    // We are not allowed to jump between characters. Therefore, we go once to the left and again to the right.
    if (anchorCodeUnit > 0) {
        ZTextLayout lay = p->createTextLayout(anchorLine, false);
        anchorCodeUnit = lay.previousCursorPosition(anchorCodeUnit, ZTextLayout::SkipCharacters);
        anchorCodeUnit = lay.nextCursorPosition(anchorCodeUnit, ZTextLayout::SkipCharacters);
    }

    if (p->anchorLine != anchorLine || p->anchorCodeUnit != anchorCodeUnit) {
        p->anchorLine = anchorLine;
        p->anchorCodeUnit = anchorCodeUnit;
        p->scheduleChangeSignal();
    }
}

int ZDocumentCursor::verticalMovementColumn() const {
    auto *const p = tuiwidgets_impl();
    return p->verticalMovementColumn;
}

void ZDocumentCursor::setVerticalMovementColumn(int column) {
    auto *const p = tuiwidgets_impl();
    p->verticalMovementColumn = std::max(0, column);
}

ZDocumentCursor::Position ZDocumentCursor::selectionStartPos() const {
    auto *const p = tuiwidgets_impl();
    return std::min(Position{p->anchorCodeUnit, p->anchorLine},
                    Position{p->cursorCodeUnit, p->cursorLine});
}

ZDocumentCursor::Position ZDocumentCursor::selectionEndPos() const {
    auto *const p = tuiwidgets_impl();
    return std::max(Position{p->anchorCodeUnit, p->anchorLine},
                    Position{p->cursorCodeUnit, p->cursorLine});
}

void ZDocumentCursor::selectAll() {
    moveToStartOfDocument(false);
    moveToEndOfDocument(true);
}

bool ZDocumentCursor::hasSelection() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorCodeUnit != p->anchorCodeUnit
            || p->cursorLine != p->anchorLine;
}

bool ZDocumentCursor::atStart() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorLine == 0 && p->cursorCodeUnit == 0;
}

bool ZDocumentCursor::atEnd() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorLine == p->doc->lines.size() - 1
            && p->cursorCodeUnit == p->doc->lines[p->cursorLine].chars.size();
}

bool ZDocumentCursor::atLineStart() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorCodeUnit == 0;
}

bool ZDocumentCursor::atLineEnd() const {
    auto *const p = tuiwidgets_impl();
    return p->cursorCodeUnit == p->doc->lines[p->cursorLine].chars.size();
}

void ZDocumentCursorPrivate::updateVerticalMovementColumn(const ZTextLayout &layoutForCursorLine) {
    ZTextLineRef tlr = layoutForCursorLine.lineForTextPosition(cursorCodeUnit);
    verticalMovementColumn = tlr.cursorToX(cursorCodeUnit, ZTextLayout::Leading);
}

void ZDocumentCursorPrivate::scheduleChangeSignal() {
    changed = true;
    doc->scheduleChangeSignals();
}

void ZDocumentCursor::debugConsistencyCheck() const {
    auto *const p = tuiwidgets_impl();
    if (p->anchorLine >= p->doc->lines.size()) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _anchorLine beyond max");
        abort();
    } else if (p->anchorLine < 0) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _anchorLine negative");
        abort();
    }

    if (p->anchorCodeUnit < 0) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _anchorCodeUnit negative");
        abort();
    } else if (p->anchorCodeUnit > p->doc->lines[p->anchorLine].chars.size()) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _anchorCodeUnit beyond max");
        abort();
    }

    if (p->cursorLine >= p->doc->lines.size()) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _cursorLine beyond max");
        abort();
    } else if (p->cursorLine < 0) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _cursorLine negative");
        abort();
    }

    if (p->cursorCodeUnit < 0) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _cursorCodeUnit negative");
        abort();
    } else if (p->cursorCodeUnit > p->doc->lines[p->cursorLine].chars.size()) {
        qFatal("ZDocumentCursor::debugConsistencyCheck: _cursorCodeUnit beyond max");
        abort();
    }
}


bool operator<(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return std::tie(lhs.line, lhs.codeUnit) < std::tie(rhs.line, rhs.codeUnit);
}

bool operator<=(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return std::tie(lhs.line, lhs.codeUnit) <= std::tie(rhs.line, rhs.codeUnit);
}

bool operator>(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return std::tie(lhs.line, lhs.codeUnit) > std::tie(rhs.line, rhs.codeUnit);
}

bool operator>=(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return std::tie(lhs.line, lhs.codeUnit) >= std::tie(rhs.line, rhs.codeUnit);
}

bool operator==(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return std::tie(lhs.codeUnit, lhs.line) == std::tie(rhs.codeUnit, rhs.line);
}

bool operator!=(const ZDocumentCursor::Position &lhs, const ZDocumentCursor::Position &rhs) {
    return !(lhs == rhs);
}

TUIWIDGETS_NS_END
