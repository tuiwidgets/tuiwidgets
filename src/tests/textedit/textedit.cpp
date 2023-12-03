// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTextEdit.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../vcheck_zwidget.h"
#include "../eventrecorder.h"

#include <QBuffer>
#include <QElapsedTimer>
#include <QFutureWatcher>

#include <Tui/ZClipboard.h>
#include <Tui/ZCommandManager.h>
#include <Tui/ZPalette.h>

static void loadText(Tui::ZTextEdit *textedit, const QString &text) {
    QByteArray x = text.toUtf8();
    QBuffer data(&x);
    data.open(QIODevice::ReadOnly);
    textedit->readFrom(&data);
}

static QString getText(Tui::ZTextEdit *textedit) {
    QByteArray x;
    QBuffer data(&x);
    data.open(QIODevice::WriteOnly);
    textedit->writeTo(&data);
    return QString::fromUtf8(x);
}

static QVector<QString> docToVec(const Tui::ZDocument *doc) {
    QVector<QString> ret;

    for (int i = 0; i < doc->lineCount(); i++) {
        ret.append(doc->line(i));
    }

    return ret;
}

TEST_CASE("textedit-base", "") {

    Testhelper t("unused", "unused", 9, 4);

    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    Tui::ZDocument doc;

    SECTION("no-parent") {
        // The initialisation must not crash.
        delete new Tui::ZTextEdit(t.terminal->textMetrics());
        delete new Tui::ZTextEdit(t.terminal->textMetrics(), (Tui::ZDocument*)nullptr);
        delete new Tui::ZTextEdit(t.terminal->textMetrics(), &doc);
    }

    SECTION("initialisation") {
        // The initialisation must not crash.
        delete new Tui::ZTextEdit(t.terminal->textMetrics(), w.get());
        delete new Tui::ZTextEdit(t.terminal->textMetrics(), nullptr, w.get());
        delete new Tui::ZTextEdit(t.terminal->textMetrics(), &doc, w.get());
    }

    auto checkDefaultState = [] (Tui::ZTextEdit *t) {
        CHECK(t->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(t->sizePolicyV() == Tui::SizePolicy::Expanding);
        CHECK(t->focusPolicy() == Tui::StrongFocus);
        CHECK(t->cursorStyle() == Tui::CursorStyle::Bar);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(t, DefaultException::SizePolicyV
                                                   | DefaultException::SizePolicyH
                                                   | DefaultException::FocusPolicy
                                                   | DefaultException::CursorStyle));

        CHECK(t->cursorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(t->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(t->document() != nullptr);
        CHECK(t->tabStopDistance() == 8);
        CHECK(t->showLineNumbers() == false);
        CHECK(t->useTabChar() == false);
        CHECK(t->wordWrapMode() == Tui::ZTextOption::WrapMode::NoWrap);
        CHECK(t->overwriteMode() == false);
        CHECK(t->selectMode() == false);
        CHECK(t->insertCursorStyle() == Tui::CursorStyle::Bar);
        CHECK(t->overwriteCursorStyle() == Tui::CursorStyle::Block);
        CHECK(t->tabChangesFocus() == true);
        CHECK(t->isReadOnly() == false);
        CHECK(t->isUndoRedoEnabled() == true);
        CHECK(t->isModified() == false);

        CHECK(t->canPaste() == false);
        CHECK(t->canCut() == false);
        CHECK(t->canCopy() == false);
        CHECK(t->selectedText() == QString(""));
        CHECK(t->isDetachedScrolling() == false);

        CHECK(t->scrollPositionLine() == 0);
        CHECK(t->scrollPositionColumn() == 0);
        CHECK(t->scrollPositionFineLine() == 0);
    };

    SECTION("constructor") {
        std::unique_ptr<Tui::ZTextEdit> te = std::make_unique<Tui::ZTextEdit>(t.terminal->textMetrics(), w.get());
        checkDefaultState(te.get());
    }

    SECTION("constructor-with-document") {
        std::unique_ptr<Tui::ZTextEdit> te = std::make_unique<Tui::ZTextEdit>(t.terminal->textMetrics(), &doc, w.get());
        CHECK(te->document() == &doc);
        checkDefaultState(te.get());

        // TODO make sure document works with tw
        te->insertText("blub");
        REQUIRE(doc.lineCount() == 1);
        CHECK(doc.line(0) == QString("blub"));
    }

    std::unique_ptr<Tui::ZTextEdit> te = std::make_unique<Tui::ZTextEdit>(t.terminal->textMetrics(), w.get());

    SECTION("abi-vcheck") {
        Tui::ZWidget base;
        checkZWidgetOverrides(&base, te.get());
    }

    SECTION("get-set-cursorPosition") {
        te->insertText("one\ntwo");
        te->setCursorPosition({1, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 0});
        te->setCursorPosition({1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
    }

    SECTION("get-set-anchorPosition") {
        te->insertText("one\ntwo");
        te->setAnchorPosition({1, 0});
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 0});
        te->setAnchorPosition({1, 1});
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
    }

    SECTION("setSelection") {
        te->insertText("one\ntwo\nthree");
        te->setSelection({1, 0}, {3, 2});
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 2});
    }

    SECTION("selection - extend selection") {
        te->insertText("one\ntwo\nthree");
        te->setCursorPosition({1, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 0});
        te->setCursorPosition({3, 2}, true);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 2});
    }

    SECTION("get-set-tabStopDistance") {
        CHECK(te->tabStopDistance() == 8);
        te->setTabStopDistance(3);
        CHECK(te->tabStopDistance() == 3);
    }

    SECTION("get-set-showLineNumbers") {
        CHECK(te->showLineNumbers() == false);
        te->setShowLineNumbers(true);
        CHECK(te->showLineNumbers() == true);
        te->setShowLineNumbers(false);
        CHECK(te->showLineNumbers() == false);
    }

    SECTION("get-set-useTabChar") {
        CHECK(te->useTabChar() == false);
        te->setUseTabChar(true);
        CHECK(te->useTabChar() == true);
        te->setUseTabChar(false);
        CHECK(te->useTabChar() == false);
    }

    SECTION("get-set-wordWrapMode") {
        CHECK(te->wordWrapMode() == Tui::ZTextOption::WrapMode::NoWrap);
        te->setWordWrapMode(Tui::ZTextOption::WrapMode::WordWrap);
        CHECK(te->wordWrapMode() == Tui::ZTextOption::WrapMode::WordWrap);
        te->setWordWrapMode(Tui::ZTextOption::WrapMode::WrapAnywhere);
        CHECK(te->wordWrapMode() == Tui::ZTextOption::WrapMode::WrapAnywhere);
    }

    SECTION("get-set-overwriteMode") {
        CHECK(te->overwriteMode() == false);
        te->setOverwriteMode(true);
        CHECK(te->overwriteMode() == true);
        te->setOverwriteMode(false);
        CHECK(te->overwriteMode() == false);
        te->toggleOverwriteMode();
        CHECK(te->overwriteMode() == true);
        te->toggleOverwriteMode();
        CHECK(te->overwriteMode() == false);
    }

    SECTION("get-set-selectMode") {
        CHECK(te->selectMode() == false);
        te->setSelectMode(true);
        CHECK(te->selectMode() == true);
        te->setSelectMode(false);
        CHECK(te->selectMode() == false);
        te->toggleSelectMode();
        CHECK(te->selectMode() == true);
        te->toggleSelectMode();
        CHECK(te->selectMode() == false);
    }

    SECTION("get-set-insertCursorStyle") {
        CHECK(te->insertCursorStyle() == Tui::CursorStyle::Bar);
        te->setInsertCursorStyle(Tui::CursorStyle::Underline);
        CHECK(te->insertCursorStyle() == Tui::CursorStyle::Underline);
        te->setInsertCursorStyle(Tui::CursorStyle::Block);
        CHECK(te->insertCursorStyle() == Tui::CursorStyle::Block);
    }

    SECTION("get-set-overwriteCursorStyle") {
        CHECK(te->overwriteCursorStyle() == Tui::CursorStyle::Block);
        te->setOverwriteCursorStyle(Tui::CursorStyle::Bar);
        CHECK(te->overwriteCursorStyle() == Tui::CursorStyle::Bar);
        te->setOverwriteCursorStyle(Tui::CursorStyle::Underline);
        CHECK(te->overwriteCursorStyle() == Tui::CursorStyle::Underline);
    }

    SECTION("get-set-tabChangesFocus") {
        CHECK(te->tabChangesFocus() == true);
        te->setTabChangesFocus(false);
        CHECK(te->tabChangesFocus() == false);
        te->setTabChangesFocus(true);
        CHECK(te->tabChangesFocus() == true);
    }

    SECTION("get-set-isReadOnly") {
        CHECK(te->isReadOnly() == false);
        te->setReadOnly(true);
        CHECK(te->isReadOnly() == true);
        te->setReadOnly(false);
        CHECK(te->isReadOnly() == false);
    }

    SECTION("get-set-isUndoRedoEnabled") {
        CHECK(te->isUndoRedoEnabled() == true);
        te->setUndoRedoEnabled(false);
        CHECK(te->isUndoRedoEnabled() == false);
        te->setUndoRedoEnabled(true);
        CHECK(te->isUndoRedoEnabled() == true);
    }

    SECTION("get-set-isDetachedScrolling") {
        CHECK(te->isDetachedScrolling() == false);
        te->enableDetachedScrolling();
        CHECK(te->isDetachedScrolling() == true);
        te->disableDetachedScrolling();
        CHECK(te->isDetachedScrolling() == false);
    }
}

TEST_CASE("textedit-read-write", "") {

    Testhelper t("textedit", "unused", 20, 10);

    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);

    QByteArray inData;
    QBuffer inFile(&inData);
    REQUIRE(inFile.open(QIODevice::ReadOnly));

    QByteArray outData;
    QBuffer outFile(&outData);
    REQUIRE(outFile.open(QIODevice::WriteOnly));


    SECTION("empty") {
        te->readFrom(&inFile);
        CHECK(docToVec(te->document()) == QVector<QString>{ "" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == true);
        CHECK(te->isModified() == false);

        te->writeTo(&outFile);
        CHECK(outData.size() == 0);
    }

    SECTION("simple") {
        inData = QByteArray("line1\nline2\n");
        te->readFrom(&inFile);
        CHECK(docToVec(te->document()) == QVector<QString>{ "line1", "line2" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == false);
        CHECK(te->isModified() == false);
        CHECK(te->document()->isModified() == false);
        CHECK(te->document()->isUndoAvailable() == false);
        CHECK(te->document()->isRedoAvailable() == false);

        te->writeTo(&outFile);
        CHECK(outData == inData);
    }

    SECTION("initial cursor position") {
        inData = QByteArray("line1\nline2\n");
        te->readFrom(&inFile, Tui::ZDocumentCursor::Position{1, 1});
        CHECK(docToVec(te->document()) == QVector<QString>{ "line1", "line2" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == false);
        CHECK(te->isModified() == false);
        CHECK(te->document()->isModified() == false);
        CHECK(te->document()->isUndoAvailable() == false);
        CHECK(te->document()->isRedoAvailable() == false);
        CHECK(te->textCursor().position() == Tui::ZDocumentCursor::Position{1, 1});
        CHECK(te->textCursor().hasSelection() == false);

        te->writeTo(&outFile);
        CHECK(outData == inData);
    }

}

TEST_CASE("textedit-text-setText", "") {

    Testhelper t("textedit", "unused", 20, 10);

    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);

    SECTION("empty") {
        te->setText(QString());
        CHECK(docToVec(te->document()) == QVector<QString>{ "" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == true);
        CHECK(te->isModified() == false);

        CHECK(te->text().size() == 0);
    }

    SECTION("simple") {
        QString inData = "line1\nline2\n";
        te->setText(inData);
        CHECK(docToVec(te->document()) == QVector<QString>{ "line1", "line2" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == false);
        CHECK(te->isModified() == false);
        CHECK(te->document()->isModified() == false);
        CHECK(te->document()->isUndoAvailable() == false);
        CHECK(te->document()->isRedoAvailable() == false);

        CHECK(te->text() == inData);
    }

    SECTION("initial cursor position") {
        QString inData = "line1\nline2\n";
        te->setText(inData, Tui::ZDocumentCursor::Position{1, 1});
        CHECK(docToVec(te->document()) == QVector<QString>{ "line1", "line2" });
        CHECK(te->document()->crLfMode() == false);
        CHECK(te->document()->newlineAfterLastLineMissing() == false);
        CHECK(te->isModified() == false);
        CHECK(te->document()->isModified() == false);
        CHECK(te->document()->isUndoAvailable() == false);
        CHECK(te->document()->isRedoAvailable() == false);
        CHECK(te->textCursor().position() == Tui::ZDocumentCursor::Position{1, 1});
        CHECK(te->textCursor().hasSelection() == false);

        CHECK(te->text() == inData);
    }

}

TEST_CASE("textedit-behavior", "") {

    Testhelper t("textedit", "unused", 20, 10);

    t.root->setGeometry({0, 0, 20, 10});
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({0, 0, 20, 10});

    // TODO: select mode

    SECTION("Key Up") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        t.sendKey(Tui::Key_Up);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 0});
    }

    SECTION("Key Shift+Up") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        t.sendKey(Tui::Key_Up, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 0});
    }

    SECTION("Key Up - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Up);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 0});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Down") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        t.sendKey(Tui::Key_Down);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 2});
    }

    SECTION("Key Shift+Down") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        t.sendKey(Tui::Key_Down, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 2});
    }

    SECTION("Key Down - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({1, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Down);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 2});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Left") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Left);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
    }

    SECTION("Key Shift+Left") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Left, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
    }

    SECTION("Key Left - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Left);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Right") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Right);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
    }

    SECTION("Key Shift+Right") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Right, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
    }

    SECTION("Key Right - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Right);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Ctrl+Left") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({7, 1});
        t.sendKey(Tui::Key_Left, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
    }

    SECTION("Key Shift+Ctrl+Left") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({7, 1});
        t.sendKey(Tui::Key_Left, Tui::ShiftModifier | Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
    }

    SECTION("Key Ctrl+Left - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({7, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Left, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Ctrl+Right") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Right, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
    }

    SECTION("Key Shift+Ctrl+Right") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Right, Tui::ShiftModifier | Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
    }

    SECTION("Key Ctrl+Right - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Right, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Home") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Home);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 1});
    }

    SECTION("Key Shift+Home") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Home, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 1});
    }

    SECTION("Key Home - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Home);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Ctrl+Home") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Home, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 0});
    }

    SECTION("Key Shift+Ctrl+Home") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Home, Tui::ControlModifier | Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 0});
    }

    SECTION("Key Ctrl+Home - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_Home, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key End") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_End);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{8, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{8, 1});
    }

    SECTION("Key Shift+End") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_End, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{8, 1});
    }

    SECTION("Key Ctrl+End") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_End, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{10, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
    }

    SECTION("Key Shift+Ctrl+End") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_End, Tui::ControlModifier | Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
    }

    SECTION("Key Ctrl+End - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_End, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Page up") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        t.sendKey(Tui::Key_PageUp);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 31});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 31});
    }

    SECTION("Key Shift+Page up") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        t.sendKey(Tui::Key_PageUp, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 40});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 31});
    }

    SECTION("Key Page up - select mode") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_PageUp);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 40});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 31});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Page down") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        t.sendKey(Tui::Key_PageDown);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 49});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 49});
    }

    SECTION("Key Shift+Page Down") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        t.sendKey(Tui::Key_PageDown, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 40});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 49});
    }

    SECTION("Key Page Down - select mode") {
        loadText(te, QString("line\n").repeated(100));
        te->setFocus();
        te->setCursorPosition({3, 40});
        te->setSelectMode(true);
        t.sendKey(Tui::Key_PageDown);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 40});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 49});
        CHECK(te->selectMode() == true);
    }

    SECTION("Key Ctrl+a") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendChar("a", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
    }

    SECTION("Key Ctrl+a - select mode") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        t.sendChar("a", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
        CHECK(te->selectMode() == true);
    }


    SECTION("Char A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendChar("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAne two"));
    }

    SECTION("preselected - Char A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        t.sendChar("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAe two"));
    }

    SECTION("overwrite - Char A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setOverwriteMode(true);
        te->setCursorPosition({2, 1});
        t.sendChar("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAe two"));
    }

    SECTION("overwrite - end of line - Char A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setOverwriteMode(true);
        te->setCursorPosition({8, 1});
        t.sendChar("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{9, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line twoA"));
    }

    SECTION("overwrite - preselected - Char A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setOverwriteMode(true);
        te->setSelection({2, 1}, {3, 1});
        t.sendChar("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAe two"));
    }

    SECTION("readonly - Char A") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("A"));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Char A") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("A"));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("insertText A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->insertText("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAne two"));
    }

    SECTION("preselected - insertText A") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        te->insertText("A");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liAe two"));
    }


    SECTION("insertText line break") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        te->insertText("\n");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 2});
        REQUIRE(te->document()->lineCount() == 4);
        CHECK(te->document()->line(1) == QString("li"));
        CHECK(te->document()->line(2) == QString("ne two"));
    }

    SECTION("preselected - insertText line break") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        te->insertText("\n");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 2});
        REQUIRE(te->document()->lineCount() == 4);
        CHECK(te->document()->line(1) == QString("li"));
        CHECK(te->document()->line(2) == QString("e two"));
    }

    SECTION("Key Enter") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Enter);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 2});
        REQUIRE(te->document()->lineCount() == 4);
        CHECK(te->document()->line(1) == QString("li"));
        CHECK(te->document()->line(2) == QString("ne two"));
    }

    SECTION("preselected - Key Enter") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        t.sendKey(Tui::Key_Enter);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 2});
        REQUIRE(te->document()->lineCount() == 4);
        CHECK(te->document()->line(1) == QString("li"));
        CHECK(te->document()->line(2) == QString("e two"));
    }

    SECTION("readonly - Key Enter") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Enter));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Enter") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Enter));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Tab - tabChangesFocus=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(true);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line two"));
    }

    SECTION("Key Tab - tabChangesFocus=0, useTabChar=0, tabStopDistance=8") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(8);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{8, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{8, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li      ne two"));
    }

    SECTION("Key Tab - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li   ne two"));
    }

    SECTION("readonly - Key Tab - tabChangesFocus=0, useTabChar=0") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        te->setReadOnly(true);
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Tab - start of line - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({0, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("     line two"));
    }

    SECTION("Key Tab - double wide char - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nあtwo\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({1, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("あ   two"));
    }

    SECTION("Key Tab - tabChangesFocus=0, useTabChar=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(true);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li\tne two"));
    }

    SECTION("insertTabAt - useTabChar=0, tabStopDistance=8") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setUseTabChar(false);
        te->setTabStopDistance(8);
        te->setCursorPosition({2, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        te->insertTabAt(cursor);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{8, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{8, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li      ne two"));
    }

    SECTION("insertTabAt - useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({2, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        te->insertTabAt(cursor);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li   ne two"));
    }

    SECTION("insertTabAt - start of line - useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({0, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        te->insertTabAt(cursor);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("     line two"));
    }

    SECTION("insertTabAt - double wide char - useTabChar=0, tabStopDistance=5") {
        loadText(te, "line one\nあtwo\nline three");
        te->setFocus();
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({1, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        te->insertTabAt(cursor);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("あ   two"));
    }

    SECTION("insertTabAt - tabChangesFocus=0, useTabChar=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setUseTabChar(true);
        te->setCursorPosition({2, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        te->insertTabAt(cursor);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li\tne two"));
    }


    SECTION("Key Tab - selection indent - tabChangesFocus=0, useTabChar=0") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setSelection({2, 1}, {1, 2});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{15, 2});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("     line two"));
        CHECK(te->document()->line(2) == QString("     line three"));
    }

    SECTION("Key Tab - reverse selection indent - tabChangesFocus=0, useTabChar=0") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setSelection({1, 2}, {2, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{15, 2});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("     line two"));
        CHECK(te->document()->line(2) == QString("     line three"));
    }

    SECTION("Key Tab - selection indent one line - tabChangesFocus=0, useTabChar=0") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setSelection({2, 1}, {3, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{13, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("     line two"));
        CHECK(te->document()->line(2) == QString("line three"));
    }

    SECTION("Key Tab - selection indent one line with linebreak - tabChangesFocus=0, useTabChar=0") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        // selecting the line break does not select the next line
        te->setSelection({2, 1}, {0, 2});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{13, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("line one"));
        CHECK(te->document()->line(1) == QString("     line two"));
        CHECK(te->document()->line(2) == QString("line three"));
    }

    SECTION("Key Tab - reverse selection indent one line with linebreak - tabChangesFocus=0, useTabChar=0") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        // selecting the line break does not select the next line
        te->setSelection({0, 2}, {2, 1});
        t.sendKey(Tui::Key_Tab);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{13, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("line one"));
        CHECK(te->document()->line(1) == QString("     line two"));
        CHECK(te->document()->line(2) == QString("line three"));
    }

    SECTION("readonly - Key Tab - selection indent - tabChangesFocus=0, useTabChar=0") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setSelection({2, 1}, {1, 2});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 2});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Shift+Tab - tabChangesFocus=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setTabChangesFocus(true);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab, Tui::ShiftModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line two"));
    }

    SECTION("Key Shift+Tab - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "     line one\n     line two\n     line three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("     line one"));
        CHECK(te->document()->line(1) == QString("line two"));
        CHECK(te->document()->line(2) == QString("     line three"));
    }

    SECTION("readonly - Key Shift+Tab - tabChangesFocus=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setReadOnly(true);
        te->setTabChangesFocus(false);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab, Tui::ShiftModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line two"));
    }

    SECTION("Key Shift+Tab - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "     line one\n     line two\n     line three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setCursorPosition({7, 1});
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("     line one"));
        CHECK(te->document()->line(1) == QString("line two"));
        CHECK(te->document()->line(2) == QString("     line three"));
    }

    SECTION("Key Shift+Tab - selection - tabChangesFocus=0, useTabChar=0, tabStopDistance=5") {
        loadText(te, "     line one\n     line two\n     line three");
        te->setFocus();
        te->setTabChangesFocus(false);
        te->setUseTabChar(false);
        te->setTabStopDistance(5);
        te->setSelection({7, 1}, {7, 2});
        t.sendKey(Tui::Key_Tab, Tui::ShiftModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{10, 2});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("     line one"));
        CHECK(te->document()->line(1) == QString("line two"));
        CHECK(te->document()->line(2) == QString("line three"));
    }

    SECTION("readonly - selection - Key Shift+Tab - tabChangesFocus=1") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setReadOnly(true);
        te->setTabChangesFocus(false);
        te->setSelection({7, 1}, {7, 2});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Tab, Tui::ShiftModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{7, 2});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line two"));
    }

    SECTION("Key Backspace") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Backspace);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("lne two"));
    }

    SECTION("Key Ctrl+Backspace") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({7, 1});
        t.sendKey(Tui::Key_Backspace, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{5, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("line o"));
    }

    SECTION("preselected - Key Backspace") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {4, 1});
        t.sendKey(Tui::Key_Backspace);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li two"));
    }

    SECTION("preselected - Key Ctrl+Backspace") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({2, 1}, {7, 1});
        t.sendKey(Tui::Key_Backspace, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("lio"));
    }

    SECTION("readonly - Key Backspace") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Backspace));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - Key Ctrl+Backspace") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({7, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Backspace, Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Backspace") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({2, 1}, {4, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Backspace));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Ctrl+Backspace") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({2, 1}, {7, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Backspace, Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{7, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Delete") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Delete);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("lie two"));
    }

    SECTION("Key Ctrl+Delete") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Delete, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li two"));
    }

    SECTION("preselected - Key Delete") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({1, 1}, {6, 1});
        t.sendKey(Tui::Key_Delete);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("lwo"));
    }

    SECTION("preselected - Key Ctrl+Delete") {
        loadText(te, "line one\nline two\nline three");
        te->setFocus();
        te->setSelection({1, 1}, {6, 1});
        t.sendKey(Tui::Key_Delete, Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{1, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("lwo"));
    }

    SECTION("readonly - Key Delete") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Delete));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - Key Ctrl+Delete") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Delete, Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Delete") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({1, 1}, {6, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Delete));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Ctrl+Delete") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({1, 1}, {6, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Delete, Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{1, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Ctrl+v") {
        // overwriteMode does not apply to clipboard operations.
        te->setOverwriteMode(GENERATE(false, true));
        loadText(te, "line one\nline two\nline three");
        t.root->findFacet<Tui::ZClipboard>()->setContents("C");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendChar("v", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liCne two"));
    }

    SECTION("preselected - Key Ctrl+v") {
        // overwriteMode does not apply to clipboard operations.
        te->setOverwriteMode(GENERATE(false, true));
        loadText(te, "line one\nline two\nline three");
        t.root->findFacet<Tui::ZClipboard>()->setContents("B");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        t.sendChar("v", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liBe two"));
    }

    SECTION("readonly - Key Ctrl+v") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        t.root->findFacet<Tui::ZClipboard>()->setContents("C");
        te->setReadOnly(true);
        te->setFocus();
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("v", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - preselected - Key Ctrl+v") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        t.root->findFacet<Tui::ZClipboard>()->setContents("C");
        te->setReadOnly(true);
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("v", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Paste Event") {
        // overwriteMode does not apply to clipboard operations.
        te->setOverwriteMode(GENERATE(false, true));
        loadText(te, "line one\nline two\nline three");
        t.root->findFacet<Tui::ZClipboard>()->setContents("C");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendPaste("X");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liXne two"));
    }

    SECTION("preselected - Paste Event") {
        // overwriteMode does not apply to clipboard operations.
        te->setOverwriteMode(GENERATE(false, true));
        loadText(te, "line one\nline two\nline three");
        t.root->findFacet<Tui::ZClipboard>()->setContents("B");
        te->setFocus();
        te->setSelection({2, 1}, {3, 1});
        t.sendPaste("X");
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{3, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{3, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liXe two"));
    }

    SECTION("Key Ctrl+c - no selection") {
        // readOnly does not apply to copy operations.
        te->setReadOnly(GENERATE(false, true));
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendChar("c", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
        CHECK(clipboard->contents() == "C");
    }

    SECTION("Key Ctrl+c - selection") {
        // readOnly does not apply to copy operations.
        te->setReadOnly(GENERATE(false, true));
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setSelection({2, 1}, {4, 1});
        t.sendChar("c", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(getText(te) == contents);
        CHECK(clipboard->contents() == "ne");
    }

    SECTION("Key Ctrl+x - no selection") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setCursorPosition({2, 1});
        t.sendChar("x", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
        CHECK(clipboard->contents() == "C");
    }

    SECTION("Key Ctrl+x - selection") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setSelection({2, 1}, {4, 1});
        t.sendChar("x", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("li two"));
        CHECK(clipboard->contents() == "ne");
    }

    SECTION("readonly - Key Ctrl+x - no selection") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setReadOnly(true);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("x", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
        CHECK(clipboard->contents() == "C");
    }

    SECTION("readonly - Key Ctrl+x - selection") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        auto *const clipboard = t.root->findFacet<Tui::ZClipboard>();
        clipboard->setContents("C");
        te->setFocus();
        te->setReadOnly(true);
        te->setSelection({2, 1}, {4, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("x", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 1});
        CHECK(getText(te) == contents);
        CHECK(clipboard->contents() == "C");
    }

    SECTION("Key Insert") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->overwriteMode() == false);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_Insert);
        CHECK(te->overwriteMode() == true);
        t.sendKey(Tui::Key_Insert);
        CHECK(te->overwriteMode() == false);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("readonly - Key Insert") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setReadOnly(true);
        te->setFocus();
        CHECK(te->overwriteMode() == false);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkKeyEventBubbles(Tui::Key_Insert));
        CHECK(te->overwriteMode() == false);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key F4") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        t.sendKey(Tui::Key_F4);
        CHECK(te->selectMode() == true);
        t.sendKey(Tui::Key_F4);
        CHECK(te->selectMode() == false);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Ctrl+z - initial undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        t.sendChar("z", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Ctrl+z - second undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        t.sendChar("z", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Key Ctrl+z then Ctrl-y - second undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        t.sendChar("z", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
        t.sendChar("y", Tui::ControlModifier);
        CHECK(te->document()->line(1) == QString("liblahne two"));
    }

    auto readOnlyOrUndoDisabled = [] {
        int testCase = GENERATE(0, 1, 2);
        if (testCase == 0) {
            return std::make_pair(false, true);
        } else if (testCase == 1) {
            return std::make_pair(true, false);
        } else {
            return std::make_pair(true, true);
        }
    };

    SECTION("noop - Key Ctrl+z - initial undo step") {
        auto [readOnly, undoDisabled] = readOnlyOrUndoDisabled();
        te->setReadOnly(true);
        te->setUndoRedoEnabled(!undoDisabled);
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        FAIL_CHECK_VEC(t.checkCharEventBubbles("z", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("noop - Key Ctrl+z - second undo step") {
        auto [readOnly, undoDisabled] = readOnlyOrUndoDisabled();
        te->setReadOnly(true);
        te->setUndoRedoEnabled(!undoDisabled);
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        FAIL_CHECK_VEC(t.checkCharEventBubbles("z", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{6, 1});
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
    }

    SECTION("noop - Key Ctrl+z then Ctrl-y - second undo step") {
        auto [readOnly, undoDisabled] = readOnlyOrUndoDisabled();
        te->setReadOnly(true);
        te->setUndoRedoEnabled(!undoDisabled);
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        FAIL_CHECK_VEC(t.checkCharEventBubbles("z", Tui::ControlModifier));
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(te->document()->line(1) == QString("liblahne two"));
        t.sendChar("y", Tui::ControlModifier);
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{6, 1});
        CHECK(te->document()->line(1) == QString("liblahne two"));
    }

    SECTION("Undo - initial undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->undo();
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Undo - second undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        te->undo();
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
    }

    SECTION("Undo then Redo - second undo step") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->insertText("blah");
        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(1) == QString("liblahne two"));
        te->undo();
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{2, 1});
        CHECK(getText(te) == contents);
        te->redo();
        CHECK(te->document()->line(1) == QString("liblahne two"));
    }

    SECTION("clear") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        CHECK(te->selectMode() == false);
        te->setCursorPosition({2, 1});
        te->setSelectMode(true);
        te->clear();
        CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);
        CHECK(te->selectMode() == false);
        CHECK(getText(te) == "");
    }

    SECTION("pageNavigationLineCount") {
        CHECK(te->pageNavigationLineCount() == 9);

        te->setGeometry({0, 0, 20, 15});
        CHECK(te->pageNavigationLineCount() == 14);
    }

    SECTION("isModified") {
        CHECK(te->isModified() == false);

        te->insertText("M");

        CHECK(te->isModified() == true);

        te->undo();

        CHECK(te->isModified() == false);
    }

    SECTION("makeCursor") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->makeCursor().insertText("Y");

        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("Yline one"));
    }

    SECTION("textCursor") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->textCursor().insertText("Y");

        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("Yline one"));

        te->setSelection({1, 0}, {4, 1});
        Tui::ZDocumentCursor cursor = te->textCursor();
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 1});
    }

    SECTION("setTextCursor") {
        const QString contents = "line one\nline two\nline three";
        loadText(te, contents);
        te->setFocus();
        te->textCursor().insertText("Y");

        REQUIRE(te->document()->lineCount() == 3);
        CHECK(te->document()->line(0) == QString("Yline one"));

        te->setSelection({1, 0}, {4, 1});
        Tui::ZDocumentCursor cursor = te->makeCursor();
        cursor.setAnchorPosition({5, 1});
        cursor.setPosition({2, 2}, true);
        cursor.setVerticalMovementColumn(8);

        te->setTextCursor(cursor);

        CHECK(te->anchorPosition() == Tui::ZDocumentCursor::Position{5, 1});
        CHECK(te->cursorPosition() == Tui::ZDocumentCursor::Position{2, 2});
        t.sendKey(Tui::Key_Up);
        CHECK(te->anchorPosition() == Tui::ZDocumentCursor::Position{8, 1});
        CHECK(te->cursorPosition() == Tui::ZDocumentCursor::Position{8, 1});
    }
}

TEST_CASE("textedit-commands", "") {

    Testhelper t("textedit", "unused", 20, 10);

    t.root->setGeometry({0, 0, 20, 10});
    Tui::ZCommandManager *const cmdManager = t.root->ensureCommandManager();
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({0, 0, 20, 10});

    loadText(te, "line one\nline two\nline three");

    te->registerCommandNotifiers(Qt::ShortcutContext::ApplicationShortcut);

    SECTION("copy") {
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Copy")) == false);
        te->setSelection({0, 0}, {1, 0});
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Copy")) == true);
        cmdManager->activateCommand(TUISYM_LITERAL("Copy"));
        CHECK(te->findFacet<Tui::ZClipboard>()->contents() == QString("l"));
    }

    SECTION("cut") {
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Cut")) == false);
        te->setSelection({0, 0}, {1, 0});
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Cut")) == true);
        cmdManager->activateCommand(TUISYM_LITERAL("Cut"));
        CHECK(te->findFacet<Tui::ZClipboard>()->contents() == QString("l"));
        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("ine one"));
    }

    SECTION("readonly - cut") {
        te->setReadOnly(true);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Cut")) == false);
        te->setSelection({0, 0}, {1, 0});
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Cut")) == false);
        cmdManager->activateCommand(TUISYM_LITERAL("Cut"));
        CHECK(te->findFacet<Tui::ZClipboard>()->contents() == QString(""));
        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("line one"));
    }

    SECTION("paste") {
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Paste")) == false);
        te->findFacet<Tui::ZClipboard>()->setContents("Z");
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Paste")) == true);
        cmdManager->activateCommand(TUISYM_LITERAL("Paste"));
        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("Zline one"));
    }

    SECTION("readonly - paste") {
        te->setReadOnly(true);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Paste")) == false);
        te->findFacet<Tui::ZClipboard>()->setContents("Z");
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Paste")) == false);
        cmdManager->activateCommand(TUISYM_LITERAL("Paste"));
        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("line one"));
    }

    SECTION("undo redo") {
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == false);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);

        te->insertText("M");

        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("Mline one"));

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == true);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);

        cmdManager->activateCommand(TUISYM_LITERAL("Undo"));

        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("line one"));

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == false);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == true);

        cmdManager->activateCommand(TUISYM_LITERAL("Redo"));

        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("Mline one"));

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == true);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);
    }

    SECTION("noop - undo") {
        int testCase = GENERATE(0, 1, 2);
        if (testCase == 0) {
            te->setReadOnly(true);
        } else if (testCase == 1) {
            te->setUndoRedoEnabled(false);
        } else {
            te->setReadOnly(true);
            te->setUndoRedoEnabled(false);
        }

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == false);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);

        te->insertText("M");

        REQUIRE(te->document()->lineCount() == 3);
        REQUIRE(te->document()->line(0) == QString("Mline one"));

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == false);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);

        te->undo();

        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Undo")) == false);
        CHECK(cmdManager->isCommandEnabled(TUISYM_LITERAL("Redo")) == false);
    }
}


TEST_CASE("textedit-signals", "") {

    Testhelper t("textedit", "unused", 20, 10);

    t.root->setGeometry({0, 0, 20, 10});
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({0, 0, 20, 10});

    loadText(te, "line one\nline two\nline three\n" + QString("line\n").repeated(30));

    SECTION("selectModeChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::selectModeChanged));

        te->setSelectMode(false);
        CHECK(recorder.noMoreEvents());

        te->setSelectMode(true);
        CHECK(recorder.consumeFirst(signal, true));
        CHECK(recorder.noMoreEvents());

        te->setSelectMode(true);
        CHECK(recorder.noMoreEvents());

        te->setSelectMode(false);
        CHECK(recorder.consumeFirst(signal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("overwriteModeChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::overwriteModeChanged));

        te->setOverwriteMode(false);
        CHECK(recorder.noMoreEvents());

        te->setOverwriteMode(true);
        CHECK(recorder.consumeFirst(signal, true));
        CHECK(recorder.noMoreEvents());

        te->setOverwriteMode(true);
        CHECK(recorder.noMoreEvents());

        te->setOverwriteMode(false);
        CHECK(recorder.consumeFirst(signal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("modifiedChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::modifiedChanged));

        te->insertText("x");

        CHECK(recorder.consumeFirst(signal, true));
        CHECK(recorder.noMoreEvents());

        getText(te);

        CHECK(recorder.consumeFirst(signal, false));
        CHECK(recorder.noMoreEvents());
    }


    SECTION("cursorPositionChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::cursorPositionChanged));

        te->insertText("あ");

        QCoreApplication::instance()->processEvents();

        CHECK(recorder.consumeFirst(signal, 2, 1, 3, 0));
        CHECK(recorder.noMoreEvents());
    }


    SECTION("scrollPositionChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::scrollPositionChanged));

        te->setCursorPosition({0, 25});

        CHECK(recorder.consumeFirst(signal, 0, 17, 0));
        CHECK(recorder.noMoreEvents());
    }


    SECTION("scrollRangeChanged") {
        EventRecorder recorder;

        auto signal = recorder.watchSignal(te, RECORDER_SIGNAL(&Tui::ZTextEdit::scrollRangeChanged));

        te->setSelection({0, 0}, {0, 20});
        te->textCursor().removeSelectedText();

        CHECK(recorder.consumeFirst(signal, 0, 23));
        CHECK(recorder.noMoreEvents());
    }
}


TEST_CASE("textedit-find", "") {

    Testhelper t("textedit", "unused", 20, 10);

    t.root->setGeometry({0, 0, 20, 10});
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({0, 0, 20, 10});

    auto testsSync = [&] (auto searchTerm1) {

        SECTION("forward case insensitive nowrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 2});

            cursor = te->findSync(searchTerm1);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{4, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 2});
        }

        SECTION("forward case insensitive wrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 2});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 0});
        }


        SECTION("forward case sensitive nowrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindCaseSensitively;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 2});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{9, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 2});
        }

        SECTION("forward case sensitive wrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap | Tui::ZTextEdit::FindFlag::FindCaseSensitively;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 2});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 0});
        }

        SECTION("backward case insensitive nowrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindBackward;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 0});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{4, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 0});
        }

        SECTION("backward case insensitive wrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap | Tui::ZTextEdit::FindFlag::FindBackward;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 0});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{4, 2});
        }

        SECTION("backward case sensitive nowrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindCaseSensitively | Tui::ZTextEdit::FindFlag::FindBackward;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 0});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{9, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 0});
        }

        SECTION("backward case sensitive wrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap
                    | Tui::ZTextEdit::FindFlag::FindCaseSensitively
                    | Tui::ZTextEdit::FindFlag::FindBackward;
            Tui::ZDocumentCursor cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 0});

            cursor = te->findSync(searchTerm1, flags);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(cursor.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(cursor.position() == Tui::ZTextEdit::Position{9, 2});
        }

    };


    auto waitFor = [](auto future) {
        QElapsedTimer timer;
        timer.start();
        while (!future.isFinished()) {
            QCoreApplication::instance()->processEvents(QEventLoop::AllEvents, 10);
            if (timer.hasExpired(10000)) {
                FAIL("Timeout in waiting for future");
            }
        }
    };

    auto testsAsync = [&] (auto searchTerm1) {

        SECTION("forward case insensitive nowrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1);

            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 2});

            future = te->findAsync(searchTerm1);

            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.anchor() == res.cursor());
        }

        SECTION("forward case insensitive wrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 2});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 0});
        }

        SECTION("forward case sensitive nowrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindCaseSensitively;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 2});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(res.anchor() == res.cursor());
        }

        SECTION("forward case sensitive wrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap | Tui::ZTextEdit::FindFlag::FindCaseSensitively;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 2});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 0});
        }

        SECTION("backward case insensitive nowrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindBackward;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 0});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(res.anchor() == res.cursor());
        }

        SECTION("backward case insensitive wrap") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap | Tui::ZTextEdit::FindFlag::FindBackward;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 0});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 2});
        }

        SECTION("backward case sensitive nowrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindCaseSensitively | Tui::ZTextEdit::FindFlag::FindBackward;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 0});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(res.anchor() == res.cursor());
        }

        SECTION("backward case sensitive wrap") {
            loadText(te, "line lIne\nline lIne\nline lIne");
            te->setFocus();
            te->setCursorPosition({6, 1});
            Tui::ZTextEdit::FindFlags flags = Tui::ZTextEdit::FindFlag::FindWrap
                    | Tui::ZTextEdit::FindFlag::FindCaseSensitively
                    | Tui::ZTextEdit::FindFlag::FindBackward;
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 0});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 0});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 0});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 0});

            future = te->findAsync(searchTerm1, flags);
            waitFor(future);
            res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{5, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{9, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{5, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{9, 2});
        }
    };

    SECTION("sync literal") {
        testsSync("lIne");
    };

    SECTION("async literal") {
        testsAsync("lIne");
    };

    SECTION("sync regex") {
        auto regex = QRegularExpression("(lI.e)");
        testsSync(regex);

        SECTION("captures") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            Tui::ZDocumentFindResult res = te->findSyncWithDetails(regex);
            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.cursor().anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(res.cursor().position() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.regexLastCapturedIndex() == 1);
            CHECK(res.regexCapture(0) == QString("line"));
            CHECK(res.regexCapture(1) == QString("line"));
        }
    };

    SECTION("async regex") {
        auto regex = QRegularExpression("(lI.e)");
        testsAsync(regex);

        SECTION("captures") {
            loadText(te, "line one\nline two\nline three");
            te->setFocus();
            te->setCursorPosition({1, 1});
            QFuture<Tui::ZDocumentFindAsyncResult> future = te->findAsync(regex);
            waitFor(future);
            Tui::ZDocumentFindAsyncResult res = future.result();

            CHECK(te->anchorPosition() == Tui::ZTextEdit::Position{0, 2});
            CHECK(te->cursorPosition() == Tui::ZTextEdit::Position{4, 2});
            CHECK(res.anchor() == Tui::ZTextEdit::Position{0, 2});
            CHECK(res.cursor() == Tui::ZTextEdit::Position{4, 2});

            CHECK(res.regexLastCapturedIndex() == 1);
            CHECK(res.regexCapture(0) == QString("line"));
            CHECK(res.regexCapture(1) == QString("line"));
        }
    };

    QCoreApplication::instance()->processEvents();
}


TEST_CASE("textedit-scroll", "") {

    Testhelper t("textedit", "unused", 20, 10);

    t.root->setGeometry({0, 0, 20, 10});
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({0, 0, 20, 10});

    SECTION("vertical") {
        loadText(te, QString("line\n").repeated(100));
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 11});

        CHECK(te->scrollPositionLine() == 3);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 25});

        CHECK(te->scrollPositionLine() == 17);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 99});

        CHECK(te->scrollPositionLine() == 91);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);
    }

    SECTION("wrapped") {
        loadText(te, QString("word ").repeated(100));
        te->setWordWrapMode(Tui::ZTextOption::WrapAnywhere);
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({20, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({8 * 20, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({8 * 20 + 19, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({9 * 20, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 1);

    }

    SECTION("horizontal") {
        loadText(te, QString("word ").repeated(100));
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({19, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({20, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 1);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({19, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 1);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({40, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 21);
        CHECK(te->scrollPositionFineLine() == 0);

    }

    SECTION("vertical resize") {
        loadText(te, QString("line\n").repeated(100));
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({0, 11});

        CHECK(te->scrollPositionLine() == 3);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setGeometry({0, 0, 20, 5});

        CHECK(te->scrollPositionLine() == 8);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);
    }

    SECTION("horizontal") {
        loadText(te, QString("word ").repeated(100));
        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);

        te->setCursorPosition({19, 0});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 0);
        CHECK(te->scrollPositionFineLine() == 0);


        te->setGeometry({0, 0, 11, 10});

        CHECK(te->scrollPositionLine() == 0);
        CHECK(te->scrollPositionColumn() == 9);
        CHECK(te->scrollPositionFineLine() == 0);
    }
}


TEST_CASE("textedit-visual", "") {

    Testhelper t("textedit", "visual", 20, 10);
    TestBackground *w = new TestBackground(t.root);
    w->setGeometry({0, 0, 20, 10});
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), w);
    te->setGeometry({1, 1, 18, 8});

    SECTION("nowrap") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");

        t.compare();
    }

    SECTION("nowrap - scroll line") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setCursorPosition({0, 1});
        te->setScrollPosition(0, 1, 0);
        t.compare();
    }

    SECTION("nowrap - scroll column") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setScrollPosition(2, 0, 0);
        t.compare();
    }

    SECTION("wrap anywhere") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setWordWrapMode(Tui::ZTextOption::WrapMode::WrapAnywhere);
        t.compare();
    }

    SECTION("wrap anywhere - fine scroll") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setWordWrapMode(Tui::ZTextOption::WrapMode::WrapAnywhere);
        te->setScrollPosition(0, 0, 1);
        t.compare();
    }

    SECTION("word wrap") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setWordWrapMode(Tui::ZTextOption::WrapMode::WordWrap);
        t.compare();
    }

    SECTION("selection") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        te->setSelection({5, 0}, {0, 3});
        t.compare();
    }

    SECTION("cursor attributes") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        t.render();
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Bar);

        te->setOverwriteMode(true);
        t.render();
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Block);

        te->setInsertCursorStyle(Tui::CursorStyle::Underline);
        t.render();
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Block);

        te->setOverwriteMode(false);
        t.render();
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Underline);

        te->setOverwriteMode(true);
        te->setOverwriteCursorStyle(Tui::CursorStyle::Unset);
        t.render();
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
    }

    SECTION("linenumber") {
        te->setFocus();
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        CHECK(te->allBordersWidth() == 0);
        te->setShowLineNumbers(true);
        CHECK(te->allBordersWidth() == 2);
        t.compare();
    }

    SECTION("linenumber-unfocused") {
        loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
        CHECK(te->allBordersWidth() == 0);
        te->setShowLineNumbers(true);
        CHECK(te->allBordersWidth() == 2);
        t.compare();
    }

}


TEST_CASE("textedit-palette", "") {

    Testhelper t("textedit", "palette", 20, 10);
    Tui::ZTextEdit *te = new Tui::ZTextEdit(t.terminal->textMetrics(), t.root);
    te->setGeometry({1, 1, 18, 8});

    auto tests = [&] {

        SECTION("non") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            t.compare();
        }

        SECTION("textedit.focused") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.focused.bg", Tui::Colors::magenta},
                            { "textedit.focused.fg", Tui::Colors::yellow}});
            te->setPalette(pal);
            t.compare();
        }

        SECTION("textedit.selected") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.selected.bg", Tui::Colors::brightWhite},
                            { "textedit.selected.fg", Tui::Colors::blue}});
            te->setPalette(pal);
            t.compare();
        }

        SECTION("textedit.linenumber") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.linenumber.bg", Tui::Colors::brightWhite},
                            { "textedit.linenumber.fg", Tui::Colors::blue}});
            te->setPalette(pal);
            t.compare();
        }

        SECTION("textedit.focused.linenumber") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.focused.linenumber.bg", Tui::Colors::brightGreen},
                            { "textedit.focused.linenumber.fg", Tui::Colors::brown}});
            te->setPalette(pal);
            t.compare();
        }

        SECTION("textedit.disabled") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.disabled.bg", Tui::Colors::brightWhite},
                            { "textedit.disabled.fg", Tui::Colors::blue}});
            te->setPalette(pal);
            t.compare();
        }


        SECTION("textedit") {
            loadText(te, "this is a really long line\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9");
            te->setSelection({5, 0}, {0, 3});
            te->setShowLineNumbers(true);

            Tui::ZPalette pal = te->palette();
            pal.setColors({ { "textedit.bg", Tui::Colors::brightWhite},
                            { "textedit.fg", Tui::Colors::blue}});
            te->setPalette(pal);
            t.compare();
        }
    };

    SECTION("focused status") {
        te->setFocus();

        tests();
    }

    SECTION("disabled status") {
        te->setEnabled(false);

        tests();
    }

    SECTION("unfocused") {
        tests();
    }
}
