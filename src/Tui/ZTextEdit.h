// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTEDIT_INCLUDED
#define TUIWIDGETS_ZTEXTEDIT_INCLUDED

#include <Tui/ZCommandNotifier.h>
#include <Tui/ZCommon.h>
#include <Tui/ZDocument.h>
#include <Tui/ZDocumentLineMarker.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextOption.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextEditPrivate;

class TUIWIDGETS_EXPORT ZTextEdit : public ZWidget {
    Q_OBJECT

public:
    using Position = ZDocumentCursor::Position;
    using FindFlag = ZDocument::FindFlag;
    using FindFlags = ZDocument::FindFlags;

public:
    explicit ZTextEdit(const ZTextMetrics &textMetrics, ZWidget *parent=nullptr);
    explicit ZTextEdit(const ZTextMetrics &textMetrics, ZDocument *document, ZWidget *parent=nullptr);
    ~ZTextEdit() override;

public:
    void setCursorPosition(Position position, bool extendSelection=false);
    Position cursorPosition() const;
    void setAnchorPosition(Position position);
    Position anchorPosition() const;
    void setTextCursor(const ZDocumentCursor &cursor);
    ZDocumentCursor textCursor() const;
    void setSelection(Position anchor, Position position);

    ZDocument *document() const;

    int lineNumberBorderWidth() const;
    virtual int allBordersWidth() const;

    void setTabStopDistance(int tab);
    int tabStopDistance() const;
    void setShowLineNumbers(bool show);
    bool showLineNumbers() const;
    void setUseTabChar(bool tab);
    bool useTabChar() const;
    void setWordWrapMode(ZTextOption::WrapMode wrap);
    ZTextOption::WrapMode wordWrapMode() const;
    void setOverwriteMode(bool mode);
    void toggleOverwriteMode();
    bool overwriteMode() const;
    void setSelectMode(bool mode);
    void toggleSelectMode();
    bool selectMode() const;
    void setInsertCursorStyle(CursorStyle style);
    CursorStyle insertCursorStyle() const;
    void setOverwriteCursorStyle(CursorStyle style);
    CursorStyle overwriteCursorStyle() const;
    void setTabChangesFocus(bool enabled);
    bool tabChangesFocus() const;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    void setUndoRedoEnabled(bool enabled);
    bool isUndoRedoEnabled() const;

    bool isModified() const;

    void insertText(const QString &str);
    void insertTabAt(ZDocumentCursor &cur);

    virtual void cut();
    virtual void copy();
    virtual void paste();

    virtual bool canPaste();
    virtual bool canCut();
    virtual bool canCopy();

    ZDocument::UndoGroup startUndoGroup();

    void removeSelectedText();
    void clearSelection();
    void selectAll();
    QString selectedText() const;
    bool hasSelection() const;

    void undo();
    void redo();

    void enableDetachedScrolling();
    void disableDetachedScrolling();
    bool isDetachedScrolling() const;
    void detachedScrollUp();
    void detachedScrollDown();

    int scrollPositionLine() const;
    int scrollPositionColumn() const;
    int scrollPositionFineLine() const;
    void setScrollPosition(int column, int line, int fineLine);

    virtual int pageNavigationLineCount() const;

    ZDocumentCursor findSync(const QString &subString, FindFlags options = FindFlags{});
    ZDocumentCursor findSync(const QRegularExpression &regex, FindFlags options = FindFlags{});
    ZDocumentFindResult findSyncWithDetails(const QRegularExpression &regex, FindFlags options = FindFlags{});
    QFuture<ZDocumentFindAsyncResult> findAsync(const QString &subString, FindFlags options = FindFlags{});
    QFuture<ZDocumentFindAsyncResult> findAsync(const QRegularExpression &regex, FindFlags options = FindFlags{});
    QFuture<ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority,
                                                             const QString &subString, FindFlags options = FindFlags{});
    QFuture<ZDocumentFindAsyncResult> findAsyncWithPool(QThreadPool *pool, int priority,
                                                             const QRegularExpression &regex,
                                                             FindFlags options = FindFlags{});

    void clear();

    void readFrom(QIODevice *file);
    void readFrom(QIODevice *file, Position initialPosition);
    void writeTo(QIODevice *file) const;

    void registerCommandNotifiers(Qt::ShortcutContext context);

    ZDocumentCursor makeCursor();

Q_SIGNALS:
    void cursorPositionChanged(int x, int utf16CodeUnit, int utf8CodeUnit, int line);
    void scrollPositionChanged(int x, int line, int fineLine);
    void scrollRangeChanged(int x, int y);
    void overwriteModeChanged(bool overwrite);
    void modifiedChanged(bool modified);
    void selectModeChanged(bool mode);

protected:
    void paintEvent(ZPaintEvent *event) override;
    void keyEvent(ZKeyEvent *event) override;
    void pasteEvent(ZPasteEvent *event) override;
    void resizeEvent(ZResizeEvent *event) override;

protected:
    virtual ZTextOption textOption() const;
    ZTextLayout textLayoutForLine(const ZTextOption &option, int line) const;
    ZTextLayout textLayoutForLineWithoutWrapping(int line) const;
    const ZTextMetrics &textMetrics() const;

    virtual void adjustScrollPosition();
    virtual void emitCursorPostionChanged();

    virtual void updateCommands();

    virtual void clearAdvancedSelection();

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QRect layoutArea() const override;
    QObject *facet(const QMetaObject &metaObject) const override;
    ZWidget *resolveSizeHintChain() override;

protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void focusInEvent(ZFocusEvent *event) override;
    void focusOutEvent(ZFocusEvent *event) override;
    void moveEvent(ZMoveEvent *event) override;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTextEdit)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTEDIT_INCLUDED
