// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocument_p.h>

#include <QTimer>

#include <Tui/Misc/SurrogateEscape.h>
//#include <Tui/ZWidget.h>

#include <Tui/ZDocumentCursor_p.h>
#include <Tui/ZDocumentLineMarker.h>
#include <Tui/ZDocumentLineMarker_p.h>
#include <Tui/ZDocumentSnapshot.h>
#include <Tui/ZDocumentSnapshot_p.h>

TUIWIDGETS_NS_START

ZDocumentPrivate::ZDocumentPrivate(ZDocument *pub) : pub_ptr(pub) {
}

ZDocumentPrivate *ZDocumentPrivate::get(ZDocument *doc) {
    return doc->tuiwidgets_impl();
}

const ZDocumentPrivate *ZDocumentPrivate::get(const ZDocument *doc) {
    return doc->tuiwidgets_impl();
}

ZDocument::ZDocument(QObject *parent) : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZDocumentPrivate>(this)) {
    auto *const p = tuiwidgets_impl();

    p->lines.append(LineData());
    p->initalUndoStep(0, 0);
}

ZDocument::~ZDocument() {
    auto *const p = tuiwidgets_impl();
    // Invalidate all snapshots out there, if snapshots are used in threads that should signal them to stop their work
    p->revision->store(p->revision->load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

    bool hasMarker = false;
    bool hasCursor = false;

    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        hasMarker = true;
    }
    for (ZDocumentCursorPrivate *cursor = p->cursorList.first; cursor; cursor = cursor->markersList.next) {
        hasCursor = true;
    }

    if (hasMarker && hasCursor) {
        qFatal("~ZDocument: Has still line markers and cursors active on destruction, this is not supported");
    } else if (hasMarker) {
        qFatal("~ZDocument: Has still line markers active on destruction, this is not supported");
    } else if (hasCursor) {
        qFatal("~ZDocument: Has still cursors active on destruction, this is not supported");
    }
}

void ZDocument::reset() {
    auto *const p = tuiwidgets_impl();
    p->lines.clear();
    p->lines.append(LineData());

    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        marker->pub()->setLine(0);
    }
    for (ZDocumentCursorPrivate *cursor = p->cursorList.first; cursor; cursor = cursor->markersList.next) {
        cursor->pub()->setPosition({0, 0});
    }

    setCrLfMode(false);

    p->initalUndoStep(0, 0);

    debugConsistencyCheck(nullptr);
    p->noteContentsChange();
}

bool ZDocument::writeTo(QIODevice *file, bool crLfMode) const {
    auto *const p = tuiwidgets_impl();
    for (int i = 0; i < lineCount(); i++) {
        const QByteArray converted = Misc::SurrogateEscape::encode(p->lines[i].chars);
        if (file->write(converted) != converted.size()) {
            return false;
        };
        if (i + 1 == lineCount() && p->newlineAfterLastLineMissing) {
            // omit newline
        } else {
            if (crLfMode) {
                if (file->write("\r\n", 2) != 2) {
                    return false;
                }
            } else {
                if (file->write("\n", 1) != 1) {
                    return false;
                }
            }
        }
    }
    return true;
}

QString ZDocument::text(bool crLfMode) const {
    QString result;
    auto *const p = tuiwidgets_impl();
    for (int i = 0; i < lineCount(); i++) {
        result += p->lines[i].chars;
        if (i + 1 == lineCount() && p->newlineAfterLastLineMissing) {
            // omit newline
        } else {
            if (crLfMode) {
                result += QStringLiteral("\r\n");
            } else {
                result += QStringLiteral("\n");
            }
        }
    }
    return result;
}


bool ZDocument::readFrom(QIODevice *file) {
    return readFrom(file, {0, 0}, nullptr);
}

bool ZDocument::readFrom(QIODevice *file, ZDocumentCursor::Position initialPosition, ZDocumentCursor *initialPositionCursor) {
    auto *const p = tuiwidgets_impl();
    // Clear line markers and cursors while _lines still has contents.
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        marker->pub()->setLine(0);
    }
    for (ZDocumentCursorPrivate *cursor = p->cursorList.first; cursor; cursor = cursor->markersList.next) {
        cursor->pub()->setPosition({0, 0});
    }

    p->lines.clear();
    QByteArray lineBuf;
    lineBuf.resize(16384);
    while (!file->atEnd()) { // each line
        int lineBytes = 0;
        p->newlineAfterLastLineMissing = true;
        while (!file->atEnd()) { // chunks of the line
            int res = file->readLine(lineBuf.data() + lineBytes, lineBuf.size() - 1 - lineBytes);
            if (res < 0) {
                // Some kind of error
                return false;
            } else if (res > 0) {
                lineBytes += res;
                if (lineBuf[lineBytes - 1] == '\n') {
                    --lineBytes; // remove \n
                    p->newlineAfterLastLineMissing = false;
                    break;
                } else if (lineBytes == lineBuf.size() - 2) {
                    lineBuf.resize(lineBuf.size() * 2);
                } else {
                    break;
                }
            }
        }

        QString text = Misc::SurrogateEscape::decode(lineBuf.constData(), lineBytes);
        p->lines.append({text, 0, nullptr});
    }

    if (p->lines.isEmpty()) {
        p->lines.append({QStringLiteral(""), 0, nullptr});
        p->newlineAfterLastLineMissing = true;
    }

    bool allLinesCrLf = false;

    for (int i = 0; i < lineCount() - (p->newlineAfterLastLineMissing ? 1 : 0); i++) {
        if (p->lines[i].chars.size() >= 1 && p->lines[i].chars.at(p->lines[i].chars.size() - 1) == QLatin1Char('\r')) {
            allLinesCrLf = true;
        } else {
            allLinesCrLf = false;
            break;
        }
    }
    if (allLinesCrLf) {
        for (int i = 0; i < lineCount() - (p->newlineAfterLastLineMissing ? 1 : 0); i++) {
            p->lines[i].chars.remove(p->lines[i].chars.size() - 1, 1);
        }
    }

    if (initialPosition.line >= p->lines.size()) {
        initialPosition.line = p->lines.size() - 1;
    }
    if (initialPosition.codeUnit > p->lines[initialPosition.line].chars.size()) {
        initialPosition.codeUnit = p->lines[initialPosition.line].chars.size();
    }

    if (initialPositionCursor) {
        initialPositionCursor->setPosition(initialPosition);
        initialPosition = initialPositionCursor->position();
    }
    p->initalUndoStep(initialPosition.codeUnit, initialPosition.line);

    debugConsistencyCheck(nullptr);

    p->noteContentsChange();

    setCrLfMode(allLinesCrLf);
    return true;
}

void ZDocument::setText(const QString &text) {
    setText(text, {0, 0}, nullptr);
}

void ZDocument::setText(const QString &text, ZDocumentCursor::Position initialPosition, ZDocumentCursor *initialPositionCursor) {
    auto *const p = tuiwidgets_impl();
    // Clear line markers and cursors while _lines still has contents.
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        marker->pub()->setLine(0);
    }
    for (ZDocumentCursorPrivate *cursor = p->cursorList.first; cursor; cursor = cursor->markersList.next) {
        cursor->pub()->setPosition({0, 0});
    }

    p->lines.clear();

    if (text.isEmpty()) {
        p->lines.append({QStringLiteral(""), 0, nullptr});
        p->newlineAfterLastLineMissing = true;
    } else {
        for (QString line: text.split(QLatin1Char('\n'))) {
            p->lines.append({line, 0, nullptr});
        }

        p->newlineAfterLastLineMissing = true;
        if (p->lines.back().chars.isEmpty()) {
            p->lines.removeLast();
            p->newlineAfterLastLineMissing = false;
        }
    }

    bool allLinesCrLf = false;

    for (int i = 0; i < lineCount() - (p->newlineAfterLastLineMissing ? 1 : 0); i++) {
        if (p->lines[i].chars.size() >= 1 && p->lines[i].chars.at(p->lines[i].chars.size() - 1) == QLatin1Char('\r')) {
            allLinesCrLf = true;
        } else {
            allLinesCrLf = false;
            break;
        }
    }
    if (allLinesCrLf) {
        for (int i = 0; i < lineCount() - (p->newlineAfterLastLineMissing ? 1 : 0); i++) {
            p->lines[i].chars.remove(p->lines[i].chars.size() - 1, 1);
        }
    }

    if (initialPosition.line >= p->lines.size()) {
        initialPosition.line = p->lines.size() - 1;
    }
    if (initialPosition.codeUnit > p->lines[initialPosition.line].chars.size()) {
        initialPosition.codeUnit = p->lines[initialPosition.line].chars.size();
    }

    if (initialPositionCursor) {
        initialPositionCursor->setPosition(initialPosition);
        initialPosition = initialPositionCursor->position();
    }
    p->initalUndoStep(initialPosition.codeUnit, initialPosition.line);

    debugConsistencyCheck(nullptr);

    p->noteContentsChange();

    setCrLfMode(allLinesCrLf);
}

void ZDocument::setCrLfMode(bool crLf) {
    auto *const p = tuiwidgets_impl();
    if (p->crLfMode != crLf) {
        p->crLfMode = crLf;
        crLfModeChanged(p->crLfMode);
    }
}

bool ZDocument::crLfMode() const {
    auto *const p = tuiwidgets_impl();
    return p->crLfMode;
}

int ZDocument::lineCount() const {
    auto *const p = tuiwidgets_impl();
    return p->lines.size();
}

QString ZDocument::line(int line) const {
    auto *const p = tuiwidgets_impl();
    return p->lines[line].chars;
}

int ZDocument::lineCodeUnits(int line) const {
    auto *const p = tuiwidgets_impl();
    return p->lines[line].chars.size();
}

unsigned ZDocument::lineRevision(int line) const {
    auto *const p = tuiwidgets_impl();
    return p->lines[line].revision;
}

void ZDocument::setLineUserData(int line, std::shared_ptr<ZDocumentLineUserData> userData) {
    auto *const p = tuiwidgets_impl();
    p->lines[line].userData = userData;
}

std::shared_ptr<ZDocumentLineUserData> ZDocument::lineUserData(int line) const {
    auto *const p = tuiwidgets_impl();
    return p->lines[line].userData;
}

ZDocumentSnapshot ZDocument::snapshot() const {
    auto *const p = tuiwidgets_impl();
    ZDocumentSnapshot ret;
    auto *const retP = ZDocumentSnapshotPrivate::get(&ret);
    retP->lines = p->lines;
    retP->revision = *p->revision;
    retP->revisionShared = p->revision;
    return ret;
}

unsigned ZDocument::revision() const {
    auto *const p = tuiwidgets_impl();
    return *p->revision;
}

bool ZDocument::isModified() const {
    auto *const p = tuiwidgets_impl();
    return p->currentUndoStep != p->savedUndoStep;
}

void ZDocument::setNewlineAfterLastLineMissing(bool value) {
    auto *const p = tuiwidgets_impl();
    const auto position = ZDocumentCursor::Position{p->lines[p->lines.size() - 1].chars.size(), p->lines.size() - 1};
    p->prepareModification(position);
    p->newlineAfterLastLineMissing = value;
    p->saveUndoStep(position);
}

bool ZDocument::newlineAfterLastLineMissing() const {
    auto *const p = tuiwidgets_impl();
    return p->newlineAfterLastLineMissing;
}

QString ZDocument::filename() const {
    auto *const p = tuiwidgets_impl();
    return p->filename;
}

void ZDocument::setFilename(const QString &filename) {
    auto *const p = tuiwidgets_impl();
    p->filename = filename;
}

void ZDocument::clearCollapseUndoStep() {
    auto *const p = tuiwidgets_impl();
    p->collapseUndoStep = false;
}

void ZDocument::sortLines(int first, int last, ZDocumentCursor *cursorForUndoStep) {
    auto *const p = tuiwidgets_impl();
    p->prepareModification(cursorForUndoStep->position());

    // We need to capture how lines got reordered to also adjust the cursor and line markers in the same pattern.
    // Basically this is std::stable_sort(_lines.begin() + first, _lines.begin() + last) but also capturing the reordering

    std::vector<int> reorderBuffer;
    reorderBuffer.resize(last - first);
    for (int i = 0; i < last - first; i++) {
        reorderBuffer[i] = first + i;
    }

    std::stable_sort(reorderBuffer.begin(), reorderBuffer.end(), [&](int lhs, int rhs) {
        return p->lines[lhs].chars < p->lines[rhs].chars;
    });

    std::vector<LineData> tmp;
    tmp.resize(last - first);
    for (int i = 0; i < last - first; i++) {
        tmp[i] = p->lines[first + i];
    }

    // Apply reorderBuffer to _lines
    for (int i = 0; i < last - first; i++) {
        p->lines[first + i] = tmp[reorderBuffer[i] - first];
    }

    std::vector<int> reorderBufferInverted;
    reorderBufferInverted.resize(last - first);
    for (int i = 0; i < last - first; i++) {
        reorderBufferInverted[reorderBuffer[i] - first] = first + i;
    }

    // And apply reorderBuffer to cursors
    for (ZDocumentCursorPrivate *curP = p->cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cursor = curP->pub();
        const auto [anchorCodeUnit, anchorLine] = cursor->anchor();
        const auto [cursorCodeUnit, cursorLine] = cursor->position();

        bool positionMustBeSet = false;

        if (first <= anchorLine && anchorLine < last) {
            cursor->setAnchorPosition({anchorCodeUnit, reorderBufferInverted[anchorLine - first]});
            positionMustBeSet = true;
        }

        if (first <= cursorLine && cursorLine < last) {
            cursor->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, reorderBufferInverted[cursorLine - first]}, true);
            positionMustBeSet = false;
        }

        if (positionMustBeSet) {
            cursor->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
        }
    }
    // Also line markers
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (first <= marker->pub()->line() && marker->pub()->line() < last) {
            marker->pub()->setLine(reorderBufferInverted[marker->pub()->line() - first]);
        }
    }

    debugConsistencyCheck(nullptr);

    auto redoTransform = [first, last, reorderBufferInverted] (QVector<ZDocumentPrivate::UndoCursor> &cursors, QVector<ZDocumentPrivate::UndoLineMarker> &markers) {
        for (ZDocumentPrivate::UndoCursor &cursor: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cursor.anchor;
            const auto [cursorCodeUnit, cursorLine] = cursor.position;

            if (first <= anchorLine && anchorLine < last) {
                cursor.anchor = {anchorCodeUnit, reorderBufferInverted[anchorLine - first]};
                cursor.anchorUpdated = true;
            }

            if (first <= cursorLine && cursorLine < last) {
                cursor.position = {cursorCodeUnit, reorderBufferInverted[cursorLine - first]};
                cursor.positionUpdated = true;
            }
        }

        for (ZDocumentPrivate::UndoLineMarker &marker: markers) {
            if (first <= marker.line && marker.line < last) {
                marker.line = reorderBufferInverted[marker.line - first];
                marker.updated = true;
            }
        }
    };

    auto undoTransform = [first, last, reorderBuffer](QVector<ZDocumentPrivate::UndoCursor> &cursors, QVector<ZDocumentPrivate::UndoLineMarker> &markers) {
        for (ZDocumentPrivate::UndoCursor &cursor: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cursor.anchor;
            const auto [cursorCodeUnit, cursorLine] = cursor.position;

            if (first <= anchorLine && anchorLine < last) {
                cursor.anchor = {anchorCodeUnit, reorderBuffer[anchorLine - first]};
                cursor.anchorUpdated = true;
            }

            if (first <= cursorLine && cursorLine < last) {
                cursor.position = {cursorCodeUnit, reorderBuffer[cursorLine - first]};
                cursor.positionUpdated = true;
            }
        }

        for (ZDocumentPrivate::UndoLineMarker &marker: markers) {
            if (first <= marker.line && marker.line < last) {
                marker.line = reorderBuffer[marker.line - first];
                marker.updated = true;
            }
        }
    };

    p->pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    p->pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    p->noteContentsChange();

    p->saveUndoStep(cursorForUndoStep->position());
}

void ZDocument::moveLine(int from, int to, ZDocumentCursor *cursorForUndoStep) {

    if (from == to) {
        return;
    }

    auto *const p = tuiwidgets_impl();
    p->prepareModification(cursorForUndoStep->position());

    if (from < to) {
        to += 1;
    }

    p->lines.insert(to, p->lines[from]);
    int undoFrom;
    int undoTo;
    if (from < to) {
        p->lines.remove(from);
        undoTo = from;
        undoFrom = to - 1;
    } else {
        p->lines.remove(from + 1);
        undoTo = from + 1;
        undoFrom = to;
    }

    auto transform = [](int from, int to, int line, int data, auto fn) {
        if (from < to) {
            // from         -> mid
            // mid          -> from
            // to           -> to
            if (line > from && line < to) {
                fn(line - 1, data);
            } else if (line == from) {
                fn(to - 1, data);
            }
        } else {
            // to           -> from
            // mid          -> to
            // from         -> mid
            if (line >= to && line < from) {
                fn(line + 1, data);
            } else if (line == from) {
                fn(to, data);
            }
        }
    };

    for (ZDocumentCursorPrivate *curP = p->cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cursor = curP->pub();
        const auto [anchorCodeUnit, anchorLine] = cursor->anchor();
        const auto [cursorCodeUnit, cursorLine] = cursor->position();

        bool positionMustBeSet = false;

        // anchor
        transform(from, to, anchorLine, anchorCodeUnit, [&](int line, int codeUnit) {
            cursor->setAnchorPosition({codeUnit, line});
            positionMustBeSet = true;
        });

        // position
        transform(from, to, cursorLine, cursorCodeUnit, [&](int line, int codeUnit) {
            cursor->setPositionPreservingVerticalMovementColumn({codeUnit, line}, true);
            positionMustBeSet = false;
        });

        if (positionMustBeSet) {
            cursor->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
        }
    }
    // similar for line markers
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        transform(from, to, marker->pub()->line(), 0, [&](int line, int) {
            marker->pub()->setLine(line);
        });
    }

    debugConsistencyCheck(nullptr);

    auto redoTransform = [from, to, transform] (QVector<ZDocumentPrivate::UndoCursor> &cursors, QVector<ZDocumentPrivate::UndoLineMarker> &markers) {
        for (ZDocumentPrivate::UndoCursor &cursor: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cursor.anchor;
            const auto [cursorCodeUnit, cursorLine] = cursor.position;

            // anchor
            transform(from, to, anchorLine, anchorCodeUnit, [&](int line, int codeUnit) {
                cursor.anchor = {codeUnit, line};
                cursor.anchorUpdated = true;
            });

            // position
            transform(from, to, cursorLine, cursorCodeUnit, [&](int line, int codeUnit) {
                cursor.position = {codeUnit, line};
                cursor.positionUpdated = true;
            });
        }

        for (ZDocumentPrivate::UndoLineMarker &marker: markers) {
            transform(from, to, marker.line, 0, [&](int line, int) {
                marker.line = line;
                marker.updated = true;
            });
        }
    };

    auto undoTransform = [undoFrom, undoTo, transform](QVector<ZDocumentPrivate::UndoCursor> &cursors, QVector<ZDocumentPrivate::UndoLineMarker> &markers) {
        for (ZDocumentPrivate::UndoCursor &cursor: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cursor.anchor;
            const auto [cursorCodeUnit, cursorLine] = cursor.position;

            // anchor
            transform(undoFrom, undoTo, anchorLine, anchorCodeUnit, [&](int line, int codeUnit) {
                cursor.anchor = {codeUnit, line};
                cursor.anchorUpdated = true;
            });

            // position
            transform(undoFrom, undoTo, cursorLine, cursorCodeUnit, [&](int line, int codeUnit) {
                cursor.position = {codeUnit, line};
                cursor.positionUpdated = true;
            });
        }

        for (ZDocumentPrivate::UndoLineMarker &marker: markers) {
            transform(undoFrom, undoTo, marker.line, 0, [&](int line, int) {
                marker.line = line;
                marker.updated = true;
            });
        }
    };

    p->pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    p->pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    p->noteContentsChange();

    p->saveUndoStep(cursorForUndoStep->position());
}

void ZDocument::debugConsistencyCheck(const ZDocumentCursor *exclude) const {
    auto *const p = tuiwidgets_impl();
    p->debugConsistencyCheck(exclude);
}

void ZDocumentPrivate::debugConsistencyCheck(const ZDocumentCursor *exclude) const {
    for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() < 0) {
            qFatal("ZDocument::debugConsistencyCheck: A line marker has a negative position");
            abort();
        } else if (marker->pub()->line() >= lines.size()) {
            qFatal("ZDocument::debugConsistencyCheck: A line marker is beyond the maximum line");
            abort();
        }
    }
    for (ZDocumentCursorPrivate *cursor = cursorList.first; cursor; cursor = cursor->markersList.next) {
        if (cursor->pub() == exclude) continue;
        cursor->pub()->debugConsistencyCheck();
    }
}


void ZDocumentPrivate::applyCursorAdjustments(ZDocumentCursor *cursor,
                                      const QVector<std::function<void(QVector<UndoCursor>&, QVector<UndoLineMarker>&)>> &cursorAdjustments) {
    // The adjustments don't necessarily have valid intermediate positions, so they can't replayed directly using the
    // cursors. Instead work on a list of positions and update the cursors after all adjustments are applied.
    QVector<UndoCursor> cursorPositions;
    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        if (cursor == curP->pub()) continue;
        cursorPositions.append({curP->pub(), false, curP->pub()->anchor(), false, curP->pub()->position()});
    }

    // Same for line markers
    QVector<UndoLineMarker> markerPositions;
    for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
        markerPositions.append({marker->pub(), false, marker->pub()->line()});
    }

    for (const auto &adjustment: cursorAdjustments) {
        adjustment(cursorPositions, markerPositions);
    }

    for (const UndoCursor &cur: cursorPositions) {
        bool positionMustBeSet = false;

        if (cur.anchorUpdated) {
            cur.cursor->setAnchorPosition(cur.anchor);
            positionMustBeSet = true;
        }

        if (cur.positionUpdated) {
            cur.cursor->setPosition(cur.position, true);
            positionMustBeSet = false;
        }

        if (positionMustBeSet) {
            cur.cursor->setPositionPreservingVerticalMovementColumn(cur.position, true);
        }
    }

    for (UndoLineMarker &marker: markerPositions) {
        if (marker.updated) {
            marker.marker->setLine(marker.line);
        }
    }
}

void ZDocument::undo(ZDocumentCursor *cursor) {
    auto *const p = tuiwidgets_impl();

    if(p->undoSteps.isEmpty()) {
        return;
    }

    if (p->currentUndoStep == 0) {
        return;
    }

    const auto startCursorCodeUnit = p->undoSteps[p->currentUndoStep].startCursorCodeUnit;
    const auto startCursorLine = p->undoSteps[p->currentUndoStep].startCursorLine;
    auto cursorAdjustments = p->undoSteps[p->currentUndoStep].undoCursorAdjustments;

    --p->currentUndoStep;

    p->undoSteps[p->currentUndoStep].collapsable = false;

    p->lines = p->undoSteps[p->currentUndoStep].lines;
    cursor->setPosition({startCursorCodeUnit, startCursorLine});
    p->newlineAfterLastLineMissing = p->undoSteps[p->currentUndoStep].noNewlineAtEnd;

    p->applyCursorAdjustments(cursor, cursorAdjustments);

    // ensure all cursors have valid positions, kept as a fallback for now.
    // Cursor positions after undo should be correctly adjusted now. But as bugs in this could could
    // make the application crash in hard to debug ways at least all cursors should be on valid positions after this
    for (ZDocumentCursorPrivate *cPriv = p->cursorList.first; cPriv; cPriv = cPriv->markersList.next) {
        ZDocumentCursor *cur = cPriv->pub();
        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        const auto [cursorCodeUnit, cursorLine] = cur->position();

        cur->setAnchorPosition({anchorCodeUnit, anchorLine});
        cur->setPosition({cursorCodeUnit, cursorLine}, true);
    }
    // similar for line markers
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() >= p->lines.size()) {
            marker->pub()->setLine(p->lines.size() - 1);
        }
    }

    debugConsistencyCheck(nullptr);

    p->noteContentsChange();

    p->emitModifedSignals();
}

void ZDocument::redo(ZDocumentCursor *cursor) {
    auto *const p = tuiwidgets_impl();

    if(p->undoSteps.isEmpty()) {
        return;
    }

    if (p->currentUndoStep + 1 >= p->undoSteps.size()) {
        return;
    }

    ++p->currentUndoStep;

    p->lines = p->undoSteps[p->currentUndoStep].lines;
    cursor->setPosition({p->undoSteps[p->currentUndoStep].endCursorCodeUnit,
                         p->undoSteps[p->currentUndoStep].endCursorLine});
    p->newlineAfterLastLineMissing = p->undoSteps[p->currentUndoStep].noNewlineAtEnd;
    auto cursorAdjustments = p->undoSteps[p->currentUndoStep].redoCursorAdjustments;

    p->applyCursorAdjustments(cursor, cursorAdjustments);

    // ensure all cursors have valid positions, kept as a fallback for now.
    // Cursor positions after undo should be correctly adjusted now. But as bugs in this could could
    // make the application crash in hard to debug ways at least all cursors should be on valid positions after this
    for (ZDocumentCursorPrivate *cPriv = p->cursorList.first; cPriv; cPriv = cPriv->markersList.next) {
        ZDocumentCursor *cur = cPriv->pub();
        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        const auto [cursorCodeUnit, cursorLine] = cur->position();

        cur->setAnchorPosition({anchorCodeUnit, anchorLine});
        cur->setPosition({cursorCodeUnit, cursorLine}, true);
    }
    // similar for line markers
    for (ZDocumentLineMarkerPrivate *marker = p->lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() >= p->lines.size()) {
            marker->pub()->setLine(p->lines.size() - 1);
        }
    }

    debugConsistencyCheck(nullptr);

    p->noteContentsChange();

    p->emitModifedSignals();
}

bool ZDocument::isUndoAvailable() const {
    auto *const p = tuiwidgets_impl();
    return p->currentUndoStep != 0;
}

bool ZDocument::isRedoAvailable() const {
    auto *const p = tuiwidgets_impl();
    return p->currentUndoStep + 1 < p->undoSteps.size();
}

ZDocument::UndoGroup ZDocument::startUndoGroup(ZDocumentCursor *cursor) {
    auto *const p = tuiwidgets_impl();
    return p->startUndoGroup(cursor);
}

ZDocument::UndoGroup ZDocumentPrivate::startUndoGroup(ZDocumentCursor *cursor) {
    if (groupUndo == 0) {
        undoStepCreationDeferred = false;
        undoGroupCollapsable = true;
        undoGroupCollapse = true;
        groupUndo = 1;
        prepareModification(cursor->position());
    } else {
        groupUndo++;
    }
    return ZDocument::UndoGroup{std::make_unique<ZDocumentUndoGroupPrivate>(this, cursor)};
}

void ZDocumentPrivate::closeUndoGroup(ZDocumentCursor *cursor) {
    if (groupUndo <= 1) {
        groupUndo = 0;
        if (undoStepCreationDeferred) {
            saveUndoStep(cursor->position(), undoGroupCollapsable, undoGroupCollapse);
            undoStepCreationDeferred = false;
        } else {
            if (pendingUpdateStep.has_value()) {
                if (pendingUpdateStep.value().redoCursorAdjustments.size()
                    || pendingUpdateStep.value().undoCursorAdjustments.size()) {

                    qFatal("ZDocument: Closing last undo group _pendingUpdateStep still containing changes.");
                    abort();
                }
            } else {
                qFatal("ZDocument: Closing last undo group without _pendingUpdateStep set.");
                abort();
            }
            pendingUpdateStep.reset();
        }
    } else {
        groupUndo--;
    }
}

void ZDocumentPrivate::registerLineMarker(ZDocumentLineMarkerPrivate *marker) {
    lineMarkerList.appendOrMoveToLast(marker);
}

void ZDocumentPrivate::unregisterLineMarker(ZDocumentLineMarkerPrivate *marker) {
    lineMarkerList.remove(marker);
}

void ZDocumentPrivate::scheduleChangeSignals() {
    if (changeScheduled) return;

    changeScheduled = true;

    QTimer::singleShot(0, pub(), [this] {
        changeScheduled = false;

        if (contentsChangedSignalToBeEmitted) {
            contentsChangedSignalToBeEmitted = false;
            Q_EMIT pub()->contentsChanged();
        }

        for (ZDocumentCursorPrivate *cursor = cursorList.first; cursor; cursor = cursor->markersList.next) {
            if (cursor->changed) {
                cursor->changed = false;
                Q_EMIT pub()->cursorChanged(cursor->pub());
            }
        }
        for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
            if (marker->changed) {
                marker->changed = false;
                Q_EMIT pub()->lineMarkerChanged(marker->pub());
            }
        }
    });
}

void ZDocumentPrivate::noteContentsChange() {
    contentsChangedSignalToBeEmitted = true;
    revision->store(revision->load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
    scheduleChangeSignals();
}

ZDocumentUndoGroupPrivate::ZDocumentUndoGroupPrivate(ZDocumentPrivate *doc, ZDocumentCursor *cursor)
    : doc(doc), cursor(cursor)
{
}


ZDocument::UndoGroup::UndoGroup(std::unique_ptr<ZDocumentUndoGroupPrivate> impl)
    : tuiwidgets_pimpl_ptr(std::move(impl))
{

}

ZDocument::UndoGroup::UndoGroup(ZDocument::UndoGroup &&other) : tuiwidgets_pimpl_ptr(std::move(other.tuiwidgets_pimpl_ptr)) {
}

ZDocument::UndoGroup::~UndoGroup() {
    if (tuiwidgets_impl()) {
        closeGroup();
    }
}

void ZDocument::UndoGroup::closeGroup() {
    auto *const p = tuiwidgets_impl();

    if (!p) {
        return;
    }

    p->doc->closeUndoGroup(p->cursor);
    tuiwidgets_pimpl_ptr = nullptr;
}

void ZDocument::markUndoStateAsSaved() {
    auto *const p = tuiwidgets_impl();
    p->undoSteps[p->currentUndoStep].collapsable = false;
    p->savedUndoStep = p->currentUndoStep;
    p->emitModifedSignals();
}

void ZDocumentPrivate::initalUndoStep(int endCodeUnit, int endLine) {
    collapseUndoStep = true;
    groupUndo = 0;
    undoSteps.clear();
    undoSteps.append({ lines, endCodeUnit, endLine, endCodeUnit, endLine, newlineAfterLastLineMissing, {}, {}, false});
    currentUndoStep = 0;
    savedUndoStep = currentUndoStep;
    emitModifedSignals();
}

void ZDocumentPrivate::prepareModification(ZDocumentCursor::Position cursorPosition) {
    if (groupUndo == 0) {
        if (pendingUpdateStep.has_value()) {
            qFatal("ZDocument: Internal error, prepareModification called with already pending modification");
            abort();
        }
    }
    if (!pendingUpdateStep.has_value()) {
        pendingUpdateStep = PendingUndoStep{cursorPosition, {}, {}};
    }
}

void ZDocumentPrivate::saveUndoStep(ZDocumentCursor::Position cursorPosition, bool collapsable, bool collapse) {
    if (groupUndo == 0) {
        const auto [startCodeUnit, startLine] = pendingUpdateStep.value().preModificationCursorPosition;
        const auto [endCodeUnit, endLine] = cursorPosition;
        if (currentUndoStep + 1 != undoSteps.size()) {
            undoSteps.resize(currentUndoStep + 1);
        }

        if (collapseUndoStep && undoSteps[currentUndoStep].collapsable
                   && undoSteps[currentUndoStep].endCursorCodeUnit == startCodeUnit
                   && undoSteps[currentUndoStep].endCursorLine == startLine
                   && collapse) {
            undoSteps[currentUndoStep].lines = lines;
            undoSteps[currentUndoStep].endCursorCodeUnit = endCodeUnit;
            undoSteps[currentUndoStep].endCursorLine = endLine;
            undoSteps[currentUndoStep].noNewlineAtEnd = newlineAfterLastLineMissing;
            undoSteps[currentUndoStep].undoCursorAdjustments
                    = pendingUpdateStep.value().undoCursorAdjustments + undoSteps[currentUndoStep].undoCursorAdjustments;
            undoSteps[currentUndoStep].redoCursorAdjustments += pendingUpdateStep.value().redoCursorAdjustments;
        } else {
            undoSteps.append({ lines, startCodeUnit, startLine, endCodeUnit, endLine,
                               newlineAfterLastLineMissing,
                               pendingUpdateStep.value().undoCursorAdjustments,
                               pendingUpdateStep.value().redoCursorAdjustments,
                               collapsable});
            currentUndoStep = undoSteps.size() - 1;
        }
        collapseUndoStep = true;
        pendingUpdateStep.reset();
        emitModifedSignals();
    } else {
        undoGroupCollapsable &= collapsable;
        undoGroupCollapse &= collapse;
        undoStepCreationDeferred = true;
    }
}

void ZDocumentPrivate::registerTextCursor(ZDocumentCursorPrivate *cursor) {
    cursorList.appendOrMoveToLast(cursor);
}

void ZDocumentPrivate::unregisterTextCursor(ZDocumentCursorPrivate *cursor) {
    cursorList.remove(cursor);
}

void ZDocumentPrivate::removeFromLine(ZDocumentCursor *cursor, int line, int codeUnitStart, int codeUnits) {
    lines[line].revision = lineRevisionCounter++;
    lines[line].chars.remove(codeUnitStart, codeUnits);

    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cur = curP->pub();
        if (cursor == cur) continue;

        bool positionMustBeSet = false;

        // anchor
        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        if (anchorLine == line) {
            if (anchorCodeUnit >= codeUnitStart + codeUnits) {
                cur->setAnchorPosition({anchorCodeUnit - codeUnits, anchorLine});
                positionMustBeSet = true;
            } else if (anchorCodeUnit >= codeUnitStart) {
                cur->setAnchorPosition({codeUnitStart, anchorLine});
                positionMustBeSet = true;
            }
        }

        // position
        const auto [cursorCodeUnit, cursorLine] = cur->position();
        if (cursorLine == line) {
            if (cursorCodeUnit >= codeUnitStart + codeUnits) {
                cur->setPosition({cursorCodeUnit - codeUnits, cursorLine}, true);
                positionMustBeSet = false;
            } else if (cursorCodeUnit >= codeUnitStart) {
                cur->setPosition({codeUnitStart, cursorLine}, true);
                positionMustBeSet = false;
            }
        }

        if (positionMustBeSet) {
            cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
        }
    }

    debugConsistencyCheck(cursor);

    auto redoTransform = [line, codeUnitStart, codeUnits](QVector<UndoCursor> &cursors, QVector<UndoLineMarker>&) {
        for (UndoCursor &cur: cursors) {
            // anchor
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            if (anchorLine == line) {
                if (anchorCodeUnit >= codeUnitStart + codeUnits) {
                    cur.anchor = {anchorCodeUnit - codeUnits, anchorLine};
                    cur.anchorUpdated = true;
                } else if (anchorCodeUnit >= codeUnitStart) {
                    cur.anchor = {codeUnitStart, anchorLine};
                    cur.anchorUpdated = true;
                }
            }

            // position
            const auto [cursorCodeUnit, cursorLine] = cur.position;
            if (cursorLine == line) {
                if (cursorCodeUnit >= codeUnitStart + codeUnits) {
                    cur.position = {cursorCodeUnit - codeUnits, cursorLine};
                    cur.positionUpdated = true;
                } else if (cursorCodeUnit >= codeUnitStart) {
                    cur.position = {codeUnitStart, cursorLine};
                    cur.positionUpdated = true;
                }
            }
        }
    };

    auto undoTransform = [line, codeUnitStart, codeUnits](QVector<UndoCursor> &cursors, QVector<UndoLineMarker>&) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            if (cur.hasSelection()) {
                const bool anchorBefore = anchorLine < cursorLine
                        || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

                // anchor
                if (anchorLine == line) {
                    const int anchorAdj = anchorBefore ? 0 : -1;
                    if (anchorCodeUnit + anchorAdj >= codeUnitStart) {
                        cur.anchor = {anchorCodeUnit + codeUnits, anchorLine};
                        cur.anchorUpdated = true;
                    }
                }

                // position
                if (cursorLine == line) {
                    const int cursorAdj = anchorBefore ? -1 : 0;
                    if (cursorCodeUnit + cursorAdj >= codeUnitStart) {
                        cur.position = {cursorCodeUnit + codeUnits, cursorLine};
                        cur.positionUpdated = true;
                    }
                }
            } else {
                if (cursorLine == line && cursorCodeUnit >= codeUnitStart) {
                    cur.position = cur.anchor = {cursorCodeUnit + codeUnits, cursorLine};
                    cur.positionUpdated = true;
                    cur.anchorUpdated = true;
                }
            }

        }
    };

    pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    noteContentsChange();
}

void ZDocumentPrivate::insertIntoLine(ZDocumentCursor *cursor, int line, int codeUnitStart, const QString &data) {
    lines[line].revision = lineRevisionCounter++;
    lines[line].chars.insert(codeUnitStart, data);

    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cur = curP->pub();
        if (cursor == cur) continue;

        bool positionMustBeSet = false;

        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        const auto [cursorCodeUnit, cursorLine] = cur->position();

        if (cur->hasSelection()) {
            const bool anchorBefore = anchorLine < cursorLine
                    || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

            // anchor
            if (anchorLine == line) {
                const int anchorAdj = anchorBefore ? 0 : -1;
                if (anchorCodeUnit + anchorAdj >= codeUnitStart) {
                    cur->setAnchorPosition({anchorCodeUnit + data.size(), anchorLine});
                    positionMustBeSet = true;
                }
            }

            // position
            if (cursorLine == line) {
                const int cursorAdj = anchorBefore ? -1 : 0;
                if (cursorCodeUnit + cursorAdj >= codeUnitStart) {
                    cur->setPosition({cursorCodeUnit + data.size(), cursorLine}, true);
                    positionMustBeSet = false;
                }
            }

            if (positionMustBeSet) {
                cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
            }
        } else {
            if (cursorLine == line && cursorCodeUnit >= codeUnitStart) {
                cur->setPosition({cursorCodeUnit + data.size(), cursorLine}, false);
            }
        }

    }

    debugConsistencyCheck(cursor);

    auto redoTransform = [line, codeUnitStart, codeUnits=data.size()](QVector<UndoCursor> &cursors, QVector<UndoLineMarker>&) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            if (cur.hasSelection()) {
                const bool anchorBefore = anchorLine < cursorLine
                        || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

                // anchor
                if (anchorLine == line) {
                    const int anchorAdj = anchorBefore ? 0 : -1;
                    if (anchorCodeUnit + anchorAdj >= codeUnitStart) {
                        cur.anchor = {anchorCodeUnit + codeUnits, anchorLine};
                        cur.anchorUpdated = true;
                    }
                }

                // position
                if (cursorLine == line) {
                    const int cursorAdj = anchorBefore ? -1 : 0;
                    if (cursorCodeUnit + cursorAdj >= codeUnitStart) {
                        cur.position = {cursorCodeUnit + codeUnits, cursorLine};
                        cur.positionUpdated = true;
                    }
                }
            } else {
                if (cursorLine == line && cursorCodeUnit >= codeUnitStart) {
                    cur.position = cur.anchor = {cursorCodeUnit + codeUnits, cursorLine};
                    cur.positionUpdated = true;
                    cur.anchorUpdated = true;
                }
            }
        }
    };

    auto undoTransform = [line, codeUnitStart, codeUnits=data.size()](QVector<UndoCursor> &cursors, QVector<UndoLineMarker>&) {
        for (UndoCursor &cur: cursors) {
            // anchor
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            if (anchorLine == line) {
                if (anchorCodeUnit >= codeUnitStart + codeUnits) {
                    cur.anchor = {anchorCodeUnit - codeUnits, anchorLine};
                    cur.anchorUpdated = true;
                } else if (anchorCodeUnit >= codeUnitStart) {
                    cur.anchor = {codeUnitStart, anchorLine};
                    cur.anchorUpdated = true;
                }
            }

            // position
            const auto [cursorCodeUnit, cursorLine] = cur.position;
            if (cursorLine == line) {
                if (cursorCodeUnit >= codeUnitStart + codeUnits) {
                    cur.position = {cursorCodeUnit - codeUnits, cursorLine};
                    cur.positionUpdated = true;
                } else if (cursorCodeUnit >= codeUnitStart) {
                    cur.position = {codeUnitStart, cursorLine};
                    cur.positionUpdated = true;
                }
            }
        }
    };

    pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    noteContentsChange();
}

void ZDocumentPrivate::removeLines(ZDocumentCursor *cursor, int start, int count) {
    lines.remove(start, count);

    for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() > start + count) {
            marker->pub()->setLine(marker->pub()->line() - count);
        } else if (marker->pub()->line() > start) {
            marker->pub()->setLine(start);
        }
    }
    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cur = curP->pub();
        if (cursor == cur) continue;

        bool positionMustBeSet = false;

        // anchor
        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        if (anchorLine >= start + count) {
            cur->setAnchorPosition({anchorCodeUnit, anchorLine - count});
            positionMustBeSet = true;
        } else if (anchorLine >= lines.size()) {
            cur->setAnchorPosition({lines[lines.size() - 1].chars.size(), lines.size() - 1});
            positionMustBeSet = true;
        } else if (anchorLine >= start) {
            cur->setAnchorPosition({0, start});
            positionMustBeSet = true;
        }

        // position
        const auto [cursorCodeUnit, cursorLine] = cur->position();
        if (cursorLine >= start + count) {
            cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine - count}, true);
            positionMustBeSet = false;
        } else if (cursorLine >= lines.size()) {
            cur->setPositionPreservingVerticalMovementColumn({lines[lines.size() - 1].chars.size(), lines.size() - 1},
                                                             true);
            positionMustBeSet = false;
        } else if (cursorLine >= start) {
            cur->setPositionPreservingVerticalMovementColumn({0, start}, true);
            positionMustBeSet = false;
        }

        if (positionMustBeSet) {
            cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
        }
    }

    debugConsistencyCheck(cursor);

    auto redoTransform = [start, count, lineCount=lines.size(), lastLineCodeUnits=lines[lines.size() - 1].chars.size()]
            (QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            // anchor
            if (anchorLine >= start + count) {
                cur.anchor = {anchorCodeUnit, anchorLine - count};
                cur.anchorUpdated = true;
            } else if (anchorLine >= lineCount) {
                cur.anchor = {lastLineCodeUnits, lineCount - 1};
                cur.anchorUpdated = true;
            } else if (anchorLine >= start) {
                cur.anchor = {0, start};
                cur.anchorUpdated = true;
            }

            // position
            if (cursorLine >= start + count) {
                cur.position = {cursorCodeUnit, cursorLine - count};
                cur.positionUpdated = true;
            } else if (cursorLine >= lineCount) {
                cur.position = {lastLineCodeUnits, lineCount - 1};
                cur.positionUpdated = true;
            } else if (cursorLine >= start) {
                cur.position = {0, start};
                cur.positionUpdated = true;
            }
        }

        for (UndoLineMarker &marker: markers) {
            if (marker.line > start + count) {
                marker.line = marker.line - count;
                marker.updated = true;
            } else if (marker.line > start) {
                marker.line = start;
                marker.updated = true;
            }
        }
    };

    auto undoTransform = [start, count](QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            // anchor
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            if (anchorLine >= start) {
                cur.anchor = {anchorCodeUnit, anchorLine + count};
                cur.anchorUpdated = true;
            }

            // position
            if (cursorLine >= start) {
                cur.position = {cursorCodeUnit, cursorLine + count};
                cur.positionUpdated = true;
            }
        }

        for (UndoLineMarker &marker: markers) {
            if (marker.line >= start) {
                marker.line = marker.line + count;
                marker.updated = true;
            }
        }
    };

    pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    noteContentsChange();
}

void ZDocumentPrivate::splitLine(ZDocumentCursor *cursor, ZDocumentCursor::Position pos) {
    lines[pos.line].revision = lineRevisionCounter++;
    lines.insert(pos.line + 1, {lines[pos.line].chars.mid(pos.codeUnit), 0, nullptr});
    lines[pos.line].chars.resize(pos.codeUnit);

    for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() > pos.line || (marker->pub()->line() == pos.line && pos.codeUnit == 0)) {
            marker->pub()->setLine(marker->pub()->line() + 1);
        }
    }
    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cur = curP->pub();
        if (cursor == cur) continue;

        bool positionMustBeSet = false;

        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        const auto [cursorCodeUnit, cursorLine] = cur->position();

        // anchor

        if (cur->hasSelection()) {
            const bool anchorBefore = anchorLine < cursorLine
                    || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

            if (anchorLine > pos.line) {
                cur->setAnchorPosition({anchorCodeUnit, anchorLine + 1});
                positionMustBeSet = true;
            } else if (anchorLine == pos.line) {
                const int anchorAdj = anchorBefore ? 0 : -1;
                if (anchorCodeUnit + anchorAdj >= pos.codeUnit) {
                    cur->setAnchorPosition({anchorCodeUnit - pos.codeUnit, anchorLine + 1});
                    positionMustBeSet = true;
                }
            }

            // position
            if (cursorLine > pos.line) {
                cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine + 1}, true);
                positionMustBeSet = false;
            } else if (cursorLine == pos.line) {
                const int cursorAdj = anchorBefore ? -1 : 0;
                if (cursorCodeUnit + cursorAdj >= pos.codeUnit) {
                    cur->setPosition({cursorCodeUnit - pos.codeUnit, cursorLine + 1}, true);
                    positionMustBeSet = false;
                }
            }

            if (positionMustBeSet) {
                cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
            }
        } else {
            if (cursorLine > pos.line) {
                cur->setPosition({cursorCodeUnit, cursorLine + 1}, false);
            } else if (cursorLine == pos.line && cursorCodeUnit >= pos.codeUnit) {
                cur->setPosition({cursorCodeUnit - pos.codeUnit, cursorLine + 1}, false);
            }
        }
    }

    debugConsistencyCheck(cursor);

    auto redoTransform = [pos](QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            if (cur.hasSelection()) {
                // anchor
                const bool anchorBefore = anchorLine < cursorLine
                        || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

                if (anchorLine > pos.line) {
                    cur.anchor = {anchorCodeUnit, anchorLine + 1};
                    cur.anchorUpdated = true;
                } else if (anchorLine == pos.line) {
                    const int anchorAdj = anchorBefore ? 0 : -1;
                    if (anchorCodeUnit + anchorAdj >= pos.codeUnit) {
                        cur.anchor = {anchorCodeUnit - pos.codeUnit, anchorLine + 1};
                        cur.anchorUpdated = true;
                    }
                }

                // position
                if (cursorLine > pos.line) {
                    cur.position = {cursorCodeUnit, cursorLine + 1};
                    cur.positionUpdated = true;
                } else if (cursorLine == pos.line) {
                    const int cursorAdj = anchorBefore ? -1 : 0;
                    if (cursorCodeUnit + cursorAdj >= pos.codeUnit) {
                        cur.position = {cursorCodeUnit - pos.codeUnit, cursorLine + 1};
                        cur.positionUpdated = true;
                    }
                }
            } else {
                if (cursorLine > pos.line) {
                    cur.position = cur.anchor = {cursorCodeUnit, cursorLine + 1};
                    cur.anchorUpdated = true;
                    cur.positionUpdated = true;
                } else if (cursorLine == pos.line && cursorCodeUnit >= pos.codeUnit) {
                    cur.position = cur.anchor = {cursorCodeUnit - pos.codeUnit, cursorLine + 1};
                    cur.anchorUpdated = true;
                    cur.positionUpdated = true;
                }
            }
        }

        for (UndoLineMarker &marker: markers) {
            if (marker.line > pos.line || (marker.line == pos.line && pos.codeUnit == 0)) {
                marker.line = marker.line + 1;
                marker.updated = true;
            }
        }
    };

    auto undoTransform = [pos](QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            // anchor
            if (anchorLine > pos.line + 1) {
                cur.anchor = {anchorCodeUnit, anchorLine - 1};
                cur.anchorUpdated = true;
            } else if (anchorLine == pos.line + 1) {
                cur.anchor = {pos.codeUnit + anchorCodeUnit, pos.line};
                cur.anchorUpdated = true;
            }

            // position
            if (cursorLine > pos.line + 1) {
                cur.position = {cursorCodeUnit, cursorLine - 1};
                cur.positionUpdated = true;
            } else if (cursorLine == pos.line + 1) {
                cur.position = {pos.codeUnit + cursorCodeUnit, pos.line};
                cur.positionUpdated = true;
            }
        }

        for (UndoLineMarker &marker: markers) {
            if (marker.line > pos.line || (marker.line - 1 == pos.line && pos.codeUnit == 0)) {
                marker.line = marker.line - 1;
                marker.updated = true;
            }
        }
    };

    pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    noteContentsChange();
}

void ZDocumentPrivate::mergeLines(ZDocumentCursor *cursor, int line) {
    const int originalLineCodeUnits = lines[line].chars.size();
    lines[line].revision = lineRevisionCounter++;
    lines[line].chars.append(lines[line + 1].chars);
    if (line + 1 < lines.size()) {
        lines.remove(line + 1, 1);
    } else {
        lines[line + 1].chars.clear();
        lines[line + 1].revision = lineRevisionCounter++;
    }

    for (ZDocumentLineMarkerPrivate *marker = lineMarkerList.first; marker; marker = marker->markersList.next) {
        if (marker->pub()->line() > line) {
            marker->pub()->setLine(marker->pub()->line() - 1);
        }
    }
    for (ZDocumentCursorPrivate *curP = cursorList.first; curP; curP = curP->markersList.next) {
        ZDocumentCursor *cur = curP->pub();
        if (cursor == cur) continue;

        bool positionMustBeSet = false;

        // anchor
        const auto [anchorCodeUnit, anchorLine] = cur->anchor();
        if (anchorLine > line + 1) {
            cur->setAnchorPosition({anchorCodeUnit, anchorLine - 1});
            positionMustBeSet = true;
        } else if (anchorLine == line + 1) {
            cur->setAnchorPosition({originalLineCodeUnits + anchorCodeUnit, line});
            positionMustBeSet = true;
        }

        // position
        const auto [cursorCodeUnit, cursorLine] = cur->position();
        if (cursorLine > line + 1) {
            cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine - 1}, true);
            positionMustBeSet = false;
        } else if (cursorLine == line + 1) {
            cur->setPosition({originalLineCodeUnits + cursorCodeUnit, line}, true);
            positionMustBeSet = false;
        }

        if (positionMustBeSet) {
            cur->setPositionPreservingVerticalMovementColumn({cursorCodeUnit, cursorLine}, true);
        }
    }

    debugConsistencyCheck(cursor);

    auto redoTransform = [line, originalLineCodeUnits](QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            // anchor
            if (anchorLine > line + 1) {
                cur.anchor = {anchorCodeUnit, anchorLine - 1};
                cur.anchorUpdated = true;
            } else if (anchorLine == line + 1) {
                cur.anchor = {originalLineCodeUnits + anchorCodeUnit, line};
                cur.anchorUpdated = true;
            }

            // position
            if (cursorLine > line + 1) {
                cur.position = {cursorCodeUnit, cursorLine - 1};
                cur.positionUpdated = true;
            } else if (cursorLine == line + 1) {
                cur.position = {originalLineCodeUnits + cursorCodeUnit, line};
                cur.positionUpdated = true;
            }
        }
        for (UndoLineMarker &marker: markers) {
            if (marker.line > line) {
                marker.line = marker.line - 1;
                marker.updated = true;
            }
        }
    };

    auto undoTransform = [line, originalLineCodeUnits](QVector<UndoCursor> &cursors, QVector<UndoLineMarker> &markers) {
        for (UndoCursor &cur: cursors) {
            const auto [anchorCodeUnit, anchorLine] = cur.anchor;
            const auto [cursorCodeUnit, cursorLine] = cur.position;

            if (cur.hasSelection()) {
                // anchor
                const bool anchorBefore = anchorLine < cursorLine
                        || (anchorLine == cursorLine && anchorCodeUnit < cursorCodeUnit);

                if (anchorLine > line) {
                    cur.anchor = {anchorCodeUnit, anchorLine + 1};
                    cur.anchorUpdated = true;
                } else if (anchorLine == line) {
                    const int anchorAdj = anchorBefore ? 0 : -1;
                    if (anchorCodeUnit + anchorAdj >= originalLineCodeUnits) {
                        cur.anchor = {anchorCodeUnit - originalLineCodeUnits, anchorLine + 1};
                        cur.anchorUpdated = true;
                    }
                }

                // position
                if (cursorLine > line) {
                    cur.position = {cursorCodeUnit, cursorLine + 1};
                    cur.positionUpdated = true;
                } else if (cursorLine == line) {
                    const int cursorAdj = anchorBefore ? -1 : 0;
                    if (cursorCodeUnit + cursorAdj >= originalLineCodeUnits) {
                        cur.position = {cursorCodeUnit - originalLineCodeUnits, cursorLine + 1};
                        cur.positionUpdated = true;
                    }
                }
            } else {
                if (cursorLine > line) {
                    cur.position = cur.anchor = {cursorCodeUnit, cursorLine + 1};
                    cur.anchorUpdated = true;
                    cur.positionUpdated = true;
                } else if (cursorLine == line && cursorCodeUnit >= originalLineCodeUnits) {
                    cur.position = cur.anchor = {cursorCodeUnit - originalLineCodeUnits, cursorLine + 1};
                    cur.anchorUpdated = true;
                    cur.positionUpdated = true;
                }
            }
        }
        for (UndoLineMarker &marker: markers) {
            if (marker.line > line || (marker.line == line && originalLineCodeUnits == 0)) {
                marker.line = marker.line + 1;
                marker.updated = true;
            }
        }
    };

    pendingUpdateStep.value().redoCursorAdjustments.push_back(redoTransform);
    pendingUpdateStep.value().undoCursorAdjustments.prepend(undoTransform);

    noteContentsChange();
}

void ZDocumentPrivate::emitModifedSignals() {
    // TODO: Ideally emit these only when changed
    Q_EMIT pub()->undoAvailable(pub()->isUndoAvailable());
    Q_EMIT pub()->redoAvailable(pub()->isRedoAvailable());
    Q_EMIT pub()->modificationChanged(pub()->isModified());
}

bool ZDocument::event(QEvent *event) {
    return QObject::event(event);
}

bool ZDocument::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}


void ZDocument::timerEvent(QTimerEvent *event) {
    return QObject::timerEvent(event);
}

void ZDocument::childEvent(QChildEvent *event) {
    return QObject::childEvent(event);
}

void ZDocument::customEvent(QEvent *event) {
    return QObject::customEvent(event);
}

void ZDocument::connectNotify(const QMetaMethod &signal) {
    return QObject::connectNotify(signal);
}

void ZDocument::disconnectNotify(const QMetaMethod &signal) {
    return QObject::disconnectNotify(signal);
}

ZDocumentLineUserData::ZDocumentLineUserData() {
}

ZDocumentLineUserData::ZDocumentLineUserData(const ZDocumentLineUserData &) = default;

ZDocumentLineUserData::ZDocumentLineUserData(ZDocumentLineUserData &&) = default;

ZDocumentLineUserData::~ZDocumentLineUserData() {
}

ZDocumentLineUserData &ZDocumentLineUserData::operator=(const ZDocumentLineUserData &other) = default;

ZDocumentLineUserData &ZDocumentLineUserData::operator=(ZDocumentLineUserData &&other) = default;

TUIWIDGETS_NS_END
