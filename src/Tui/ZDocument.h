// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZDOCUMENT_INCLUDED
#define TUIWIDGETS_ZDOCUMENT_INCLUDED

#include <QFuture>
#include <QIODevice>
#include <QObject>
#include <QRegularExpression>
#include <QString>

#include <Tui/ZDocumentCursor.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZDocumentLineMarker;
class ZDocumentSnapshot;

class TUIWIDGETS_EXPORT ZDocumentLineUserData {
public:
    ZDocumentLineUserData();
    ZDocumentLineUserData(const ZDocumentLineUserData&);
    ZDocumentLineUserData(ZDocumentLineUserData&&);
    virtual ~ZDocumentLineUserData();

    ZDocumentLineUserData &operator=(const ZDocumentLineUserData &other);
    ZDocumentLineUserData &operator=(ZDocumentLineUserData &&other);
};

class ZDocumentFindAsyncResultPrivate;

class TUIWIDGETS_EXPORT ZDocumentFindAsyncResult {
public:
    ZDocumentFindAsyncResult();
    ZDocumentFindAsyncResult(const ZDocumentFindAsyncResult &other);
    ~ZDocumentFindAsyncResult();

    ZDocumentFindAsyncResult &operator=(const ZDocumentFindAsyncResult &other);

public:
    ZDocumentCursor::Position anchor() const;
    ZDocumentCursor::Position cursor() const;
    unsigned revision() const;

    int regexLastCapturedIndex() const;
    QString regexCapture(int index) const;
    QString regexCapture(const QString &name) const;

public: // internal
    ZDocumentFindAsyncResult(std::unique_ptr<ZDocumentFindAsyncResultPrivate> impl);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocumentFindAsyncResult)
    std::unique_ptr<ZDocumentFindAsyncResultPrivate> tuiwidgets_pimpl_ptr;
    friend class ZDocument;
};

class ZDocumentFindResultPrivate;

class TUIWIDGETS_EXPORT ZDocumentFindResult {
public:
    ZDocumentFindResult(const ZDocumentFindResult &other);
    ~ZDocumentFindResult();

    ZDocumentFindResult &operator=(const ZDocumentFindResult &other);

public:
    ZDocumentCursor cursor() const;

    int regexLastCapturedIndex() const;
    QString regexCapture(int index) const;
    QString regexCapture(const QString &name) const;

private:
    friend class ZDocument;
    ZDocumentFindResult(ZDocumentCursor cursor, QRegularExpressionMatch match);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocumentFindResult)
    std::unique_ptr<ZDocumentFindResultPrivate> tuiwidgets_pimpl_ptr;
};

class ZDocumentUndoGroupPrivate;

class TUIWIDGETS_EXPORT ZDocument : public QObject {
    Q_OBJECT
public:
    class UndoGroup {
    public:
        UndoGroup(const UndoGroup&) = delete;
        UndoGroup(UndoGroup &&);
        UndoGroup &operator=(const UndoGroup&) = delete;

        ~UndoGroup();

    public:
        void closeGroup();

    public: // internal
        UndoGroup(std::unique_ptr<ZDocumentUndoGroupPrivate> impl);

    private:
        TUIWIDGETS_DECLARE_PRIVATE(ZDocumentUndoGroup)
        std::unique_ptr<ZDocumentUndoGroupPrivate> tuiwidgets_pimpl_ptr;
    };

    enum FindFlag : int {
        FindBackward = 1 << 0,
        FindCaseSensitively = 1 << 1,
        FindWrap = 1 << 2,
    };

    using FindFlags = QFlags<FindFlag>;

public:
    explicit ZDocument(QObject *parent=nullptr);
    ~ZDocument() override;

public:
    void reset();
    void writeTo(QIODevice *file, bool crLfMode = false) const;
    void readFrom(QIODevice *file);
    void readFrom(QIODevice *file, ZDocumentCursor::Position initialPosition, ZDocumentCursor *initialPositionCursor);

    void setCrLfMode(bool crLf);
    bool crLfMode() const;

    int lineCount() const;
    QString line(int line) const;
    int lineCodeUnits(int line) const;
    unsigned lineRevision(int line) const;
    void setLineUserData(int line, std::shared_ptr<ZDocumentLineUserData> userData);
    std::shared_ptr<ZDocumentLineUserData> lineUserData(int line) const;
    ZDocumentSnapshot snapshot() const;

    unsigned revision() const;
    bool isModified() const;

    void setNewlineAfterLastLineMissing(bool value);
    bool newlineAfterLastLineMissing() const;

    QString filename() const;
    void setFilename(const QString &filename);

    void clearCollapseUndoStep();

    void sortLines(int first, int last, ZDocumentCursor *cursorForUndoStep);
    void moveLine(int from, int to, ZDocumentCursor *cursorForUndoStep);
    void debugConsistencyCheck(const ZDocumentCursor *exclude=nullptr) const;

    void undo(ZDocumentCursor *cursor);
    void redo(ZDocumentCursor *cursor);
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    UndoGroup startUndoGroup(ZDocumentCursor *cursor);

    void markUndoStateAsSaved();

    ZDocumentCursor findSync(const QString &subString, const ZDocumentCursor &start,
                             FindFlags options = FindFlags{}) const;
    ZDocumentCursor findSync(const QRegularExpression &regex, const ZDocumentCursor &start,
                             FindFlags options = FindFlags{}) const;
    ZDocumentFindResult findSyncWithDetails(const QRegularExpression &regex, const ZDocumentCursor &start,
                                            FindFlags options = FindFlags{}) const;
    QFuture<ZDocumentFindAsyncResult> findAsync(const QString &subString, const ZDocumentCursor &start,
                                                FindFlags options = FindFlags{}) const;
    QFuture<ZDocumentFindAsyncResult> findAsync(const QRegularExpression &regex, const ZDocumentCursor &start,
                                                FindFlags options = FindFlags{}) const;
    QFuture<ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority,
                                                        const QString &subString, const ZDocumentCursor &start,
                                                        FindFlags options = FindFlags{}) const;
    QFuture<ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority,
                                                        const QRegularExpression &regex, const ZDocumentCursor &start,
                                                        FindFlags options = FindFlags{}) const;

Q_SIGNALS:
    void modificationChanged(bool changed);
    void redoAvailable(bool available);
    void undoAvailable(bool available);

    void contentsChanged();

    void cursorChanged(const ZDocumentCursor *cursor);
    void lineMarkerChanged(const ZDocumentLineMarker *marker);

    void crLfModeChanged(bool crLf);

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZDocument)
    std::unique_ptr<ZDocumentPrivate> tuiwidgets_pimpl_ptr;
};

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZDocument::FindFlags)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZDocument::FindFlags)

#endif // TUIWIDGETS_ZDOCUMENT_INCLUDED
