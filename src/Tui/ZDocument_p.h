// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENT_P_INCLUDED
#define TUIWIDGETS_ZDOCUMENT_P_INCLUDED

#include <memory>
#include <optional>

#include <QString>
#include <QVector>

#include <Tui/ListNode_p.h>
#include <Tui/ZDocument.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDocumentLineMarker;
class ZDocumentLineMarkerPrivate;

struct LineMarkerToDocumentTag;
struct TextCursorToDocumentTag;

struct LineData {
    QString chars;
    unsigned revision = 0;
    std::shared_ptr<ZDocumentLineUserData> userData;
};

class ZDocumentFindAsyncResultPrivate {
public:
    ZDocumentFindAsyncResultPrivate();
    ZDocumentFindAsyncResultPrivate(ZDocumentCursor::Position anchor, ZDocumentCursor::Position cursor,
                                    unsigned revision, QRegularExpressionMatch match);

public:
    ZDocumentCursor::Position anchor;
    ZDocumentCursor::Position cursor;
    unsigned revision = -1;

    QRegularExpressionMatch match;
};

class ZDocumentFindResultPrivate {
public:
    ZDocumentFindResultPrivate(const ZDocumentCursor &cursor) : cursor(cursor) {}

    ZDocumentCursor cursor;

    QRegularExpressionMatch _match;
};

class ZDocumentUndoGroupPrivate {
public:
    ZDocumentUndoGroupPrivate(ZDocumentPrivate *doc, ZDocumentCursor *cursor);

public:
    ZDocumentPrivate *doc = nullptr;
    ZDocumentCursor *cursor = nullptr;
    bool _closed = false;
};

class ZDocumentPrivate {
public:
    struct UndoCursor {
        ZDocumentCursor *cursor;
        bool anchorUpdated = false;
        ZDocumentCursor::Position anchor = {0, 0};
        bool positionUpdated = false;
        ZDocumentCursor::Position position = {0, 0};

        bool hasSelection() const {
            return anchor != position;
        }
    };

    struct UndoLineMarker {
        ZDocumentLineMarker *marker;
        bool updated = false;
        int line = 0;
    };

    struct UndoStep {
        QVector<LineData> lines;
        int startCursorCodeUnit;
        int startCursorLine;
        int endCursorCodeUnit;
        int endCursorLine;
        bool noNewlineAtEnd = false;
        QVector<std::function<void(QVector<UndoCursor>&, QVector<UndoLineMarker>&)>> undoCursorAdjustments;
        QVector<std::function<void(QVector<UndoCursor>&, QVector<UndoLineMarker>&)>> redoCursorAdjustments;
        bool collapsable = false;
    };

public:
    ZDocumentPrivate(ZDocument *pub);

public:
    static ZDocumentPrivate *get(ZDocument *doc);
    static const ZDocumentPrivate *get(const ZDocument *doc);

public: // used internally and by TextCursor
    ZDocument::UndoGroup startUndoGroup(ZDocumentCursor *cursor);
    void debugConsistencyCheck(const ZDocumentCursor *exclude) const;

public: // TextCursor interface
    void removeFromLine(ZDocumentCursor *cursor, int line, int codeUnitStart, int codeUnits);
    void insertIntoLine(ZDocumentCursor *cursor, int line, int codeUnitStart, const QString &data);
    void removeLines(ZDocumentCursor *cursor, int start, int count);
    void splitLine(ZDocumentCursor *cursor, ZDocumentCursor::Position pos);
    void mergeLines(ZDocumentCursor *cursor, int line);
    void saveUndoStep(ZDocumentCursor::Position cursorPosition, bool collapsable=false, bool collapse=false);
    void prepareModification(ZDocumentCursor::Position cursorPosition);
    void registerTextCursor(ZDocumentCursorPrivate *cursor);
    void unregisterTextCursor(ZDocumentCursorPrivate *cursor);

public: // UndoGroup interface
    void closeUndoGroup(ZDocumentCursor *cursor);

public: // LineMarker interface
    void registerLineMarker(ZDocumentLineMarkerPrivate *marker);
    void unregisterLineMarker(ZDocumentLineMarkerPrivate *marker);

public: // TextCursor + LineMarker interface
    void scheduleChangeSignals();

public:
    void applyCursorAdjustments(ZDocumentCursor *cursor,
                                const QVector<std::function<void(QVector<ZDocumentPrivate::UndoCursor>&, QVector<ZDocumentPrivate::UndoLineMarker>&)>> &cursorAdjustments);
    void initalUndoStep(int endCodeUnit, int endLine);
    void noteContentsChange();
    void emitModifedSignals();

public:
    QString filename;
    QVector<LineData> lines;
    bool newlineAfterLastLineMissing = false;
    bool crLfMode = false;

    QVector<UndoStep> undoSteps;
    int currentUndoStep = -1;
    int savedUndoStep = -1;

    bool collapseUndoStep = false;
    int groupUndo = 0;
    bool undoStepCreationDeferred = false;
    bool undoGroupCollapsable = false;
    bool undoGroupCollapse = false;
    struct PendingUndoStep {
        ZDocumentCursor::Position preModificationCursorPosition;
        QVector<std::function<void(QVector<UndoCursor>&, QVector<UndoLineMarker>&)>> redoCursorAdjustments;
        QVector<std::function<void(QVector<UndoCursor>&, QVector<UndoLineMarker>&)>> undoCursorAdjustments;
    };
    std::optional<PendingUndoStep> pendingUpdateStep;

    ListHead<ZDocumentLineMarkerPrivate, LineMarkerToDocumentTag> lineMarkerList;
    ListHead<ZDocumentCursorPrivate, TextCursorToDocumentTag> cursorList;
    bool changeScheduled = false;
    bool contentsChangedSignalToBeEmitted = false;
    std::shared_ptr<std::atomic<unsigned>> revision = std::make_shared<std::atomic<unsigned>>(0);
    int lineRevisionCounter = 0;

    ZDocument *pub_ptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZDocument)
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDOCUMENT_P_INCLUDED
