// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENTCURSOR_INCLUDED
#define TUIWIDGETS_ZDOCUMENTCURSOR_INCLUDED

#include <functional>

#include <Tui/ZWidget.h>
#include <Tui/ZTextLayout.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDocument;
class ZDocumentPrivate;

class ZDocumentCursorPrivate;

class TUIWIDGETS_EXPORT ZDocumentCursor {
public:
    class TUIWIDGETS_EXPORT Position {
    public:
        int codeUnit = 0; // in line
        int line = 0;

        Position() {}
        Position(int codeUnit, int line) : codeUnit(codeUnit), line(line) {}
        friend TUIWIDGETS_EXPORT bool operator<(const Position& lhs, const Position& rhs);
        friend TUIWIDGETS_EXPORT bool operator<=(const Position& lhs, const Position& rhs);
        friend TUIWIDGETS_EXPORT bool operator>(const Position& lhs, const Position& rhs);
        friend TUIWIDGETS_EXPORT bool operator>=(const Position& lhs, const Position& rhs);
        friend TUIWIDGETS_EXPORT bool operator==(const Position& lhs, const Position& rhs);
        friend TUIWIDGETS_EXPORT bool operator!=(const Position& lhs, const Position& rhs);
    };

public:
    explicit ZDocumentCursor(ZDocument *doc,
                             std::function<ZTextLayout(int line, bool wrappingAllowed)> createTextLayout);
    ZDocumentCursor(const ZDocumentCursor &other);
    ~ZDocumentCursor();

    ZDocumentCursor &operator=(const ZDocumentCursor &other);

public:
    void insertText(const QString &text);
    void overwriteText(const QString &text, int clusterCount = 1);
    void removeSelectedText();
    void clearSelection();
    QString selectedText() const;

    void deleteCharacter();
    void deletePreviousCharacter();
    void deleteWord();
    void deletePreviousWord();
    void deleteLine();

    void moveCharacterLeft(bool extendSelection = false);
    void moveCharacterRight(bool extendSelection = false);
    void moveWordLeft(bool extendSelection = false);
    void moveWordRight(bool extendSelection = false);

    void moveUp(bool extendSelection = false);
    void moveDown(bool extendSelection = false);

    void moveToStartOfLine(bool extendSelection = false);
    void moveToStartIndentedText(bool extendSelection = false);
    void moveToEndOfLine(bool extendSelection = false);

    void moveToStartOfDocument(bool extendSelection = false);
    void moveToEndOfDocument(bool extendSelection = false);

    Position position() const;
    void setPosition(Position pos, bool extendSelection = false);
    void setPositionPreservingVerticalMovementColumn(Position pos, bool extendSelection = false);

    Position anchor() const;
    void setAnchorPosition(Position pos);

    int verticalMovementColumn() const;
    void setVerticalMovementColumn(int column);

    // for hasSelection() == true
    Position selectionStartPos() const;
    Position selectionEndPos() const;

    void selectAll();

    bool hasSelection() const;

    bool atStart() const;
    bool atEnd() const;
    bool atLineStart() const;
    bool atLineEnd() const;

public:
    void debugConsistencyCheck() const;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocumentCursor)
    std::unique_ptr<ZDocumentCursorPrivate> tuiwidgets_pimpl_ptr;
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENTCURSOR_INCLUDED
