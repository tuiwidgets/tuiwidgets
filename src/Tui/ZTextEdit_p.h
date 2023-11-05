// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTEDIT_P_INCLUDED
#define TUIWIDGETS_ZTEXTEDIT_P_INCLUDED

#include <Tui/ZTextEdit.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextEditPrivate : public ZWidgetPrivate {
public:
    ZTextEditPrivate(const Tui::ZTextMetrics &textMetrics, Tui::ZDocument *document, ZWidget *pub);
    ~ZTextEditPrivate() override;

public:
    Tui::ZDocumentCursor makeCursor();

    QFuture<ZDocumentFindAsyncResult> connectAsyncFindCommon(QFuture<ZDocumentFindAsyncResult> res,
                                                             ZTextEdit::FindFlags options);

    QPair<int, int> getSelectedLinesSort();
    QPair<int, int> getSelectedLines();
    void selectLines(int startLine, int endLine);

    void updatePasteCommandEnabled();

public:
    Tui::ZTextMetrics textMetrics;
    Tui::ZDocument *doc = nullptr;
    std::unique_ptr<Tui::ZDocument> autoDeleteDoc;
    Tui::ZDocumentCursor cursor;
    bool selectMode = false;

    int tabsize = 8;
    bool useTabChar = false;
    Tui::ZTextOption::WrapMode wrapMode = Tui::ZTextOption::NoWrap;
    bool showLineNumbers = false;
    bool overwriteMode = false;
    bool tabChangesFocus = true;
    bool readOnly = false;
    bool undoRedoEnabled = true;
    Tui::CursorStyle insertCursorStyle = Tui::CursorStyle::Bar;
    Tui::CursorStyle overwriteCursorStyle = Tui::CursorStyle::Block;

    bool detachedScrolling = false;
    int scrollPositionColumn = 0;
    Tui::ZDocumentLineMarker scrollPositionLine;
    int scrollPositionFineLine = 0;

    Tui::ZCommandNotifier *cmdCopy = nullptr;
    Tui::ZCommandNotifier *cmdCut = nullptr;
    Tui::ZCommandNotifier *cmdPaste = nullptr;
    Tui::ZCommandNotifier *cmdUndo = nullptr;
    Tui::ZCommandNotifier *cmdRedo = nullptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZTextEdit)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTEDIT_P_INCLUDED
