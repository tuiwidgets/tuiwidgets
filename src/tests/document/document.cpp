// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocumentCursor.h>
#include <Tui/ZDocumentLineMarker.h>
#include <Tui/ZDocumentSnapshot.h>

#include <random>
#include <array>

#include <QBuffer>
#include <QCoreApplication>

#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextOption.h>

#include "../catchwrapper.h"
#include "../eventrecorder.h"
#include "../Testhelper.h"

#include "../vcheck_qobject.h"

static QVector<QString> docToVec(const Tui::ZDocument &doc) {
    QVector<QString> ret;

    for (int i = 0; i < doc.lineCount(); i++) {
        ret.append(doc.line(i));
    }

    return ret;
}

static QVector<QString> snapToVec(const Tui::ZDocumentSnapshot &snap) {
    QVector<QString> ret;

    for (int i = 0; i < snap.lineCount(); i++) {
        ret.append(snap.line(i));
    }

    return ret;
}

namespace {
    class TestUserData : public Tui::ZDocumentLineUserData {
    };
}

TEST_CASE("ZDocument") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    Tui::ZDocumentCursor wrappedCursor{&doc, [&textMetrics, &doc](int line, bool wrappingAllowed) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            if (wrappingAllowed) {
                Tui::ZTextOption option;
                option.setWrapMode(Tui::ZTextOption::WrapMode::WordWrap);
                lay.setTextOption(option);
            }
            lay.doLayout(40);
            return lay;
        }
    };


    SECTION("abi-vcheck") {
        QObject base;
        checkQObjectOverrides(&base, &doc);
    }

    SECTION("setCrLfMode") {
        CHECK(doc.crLfMode() == false);

        EventRecorder recorder;
        auto changedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::crLfModeChanged));
        doc.setCrLfMode(true);
        CHECK(doc.crLfMode() == true);

        recorder.waitForEvent(changedSignal);
        CHECK(recorder.consumeFirst(changedSignal, true));
        CHECK(recorder.noMoreEvents());

        doc.setCrLfMode(false);
        CHECK(doc.crLfMode() == false);

        recorder.waitForEvent(changedSignal);
        CHECK(recorder.consumeFirst(changedSignal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("insert nonewline") {
        doc.setNewlineAfterLastLineMissing(true);

        cursor.insertText("test\ntest");

        REQUIRE(doc.lineCount() == 2);
        CHECK(doc.line(0) == "test");
        CHECK(doc.line(1) == "test");
    }

    SECTION("insert and remove nonewline") {
        doc.setNewlineAfterLastLineMissing(true);
        cursor.insertText("\n");
        REQUIRE(doc.lineCount() == 1);
        CHECK(doc.newlineAfterLastLineMissing() == false);
    }

    SECTION("insert-empty") {
        cursor.insertText("");
        REQUIRE(doc.lineCount() == 1);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
    }

    SECTION("insert-empty-line-and-text") {
        cursor.insertText("\ntest test");
        REQUIRE(doc.lineCount() == 2);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
    }

    SECTION("insert-lines") {
        cursor.insertText("test test");
        REQUIRE(doc.lineCount() == 1);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 0});
        cursor.selectAll();

        cursor.insertText("test test\ntest test");
        REQUIRE(doc.lineCount() == 2);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
        cursor.selectAll();

        cursor.insertText("test test\ntest test\ntest test\n");
        REQUIRE(doc.lineCount() == 4);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 3});
        cursor.selectAll();

        cursor.insertText("test test\ntest test\n");
        cursor.insertText("test test\ntest test");
        REQUIRE(doc.lineCount() == 4);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 3});
        cursor.selectAll();
    }

    SECTION("insert-and-selection") {
        cursor.insertText("test test");
        REQUIRE(doc.lineCount() == 1);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 0});
        cursor.moveWordLeft(true);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 0});
        cursor.insertText("new new");

        QVector<QString> test;
        test.append("test new new");
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{12, 0});
        CHECK(docToVec(doc) == test);

        cursor.moveWordLeft(true);
        cursor.moveWordLeft(true);
        cursor.moveWordLeft(true);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.insertText("\nold");
    }

    SECTION("insert contentsChanged signal") {
        cursor.insertText("test test");

        // signal emitted async

        EventRecorder recorder;
        auto changedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::contentsChanged));

        recorder.waitForEvent(changedSignal);
        CHECK(recorder.consumeFirst(changedSignal));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("insert modificationChanged signal") {
        EventRecorder recorder;

        // signal emited sync
        auto modificationChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::modificationChanged));

        const char *const textToInsert = GENERATE("test test", "a", " " , "\t" , "\n"); // insert as special handling for whitespace
        cursor.insertText(textToInsert);

        recorder.waitForEvent(modificationChangedSignal);
        CHECK(recorder.consumeFirst(modificationChangedSignal, true));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("insert undoAvailable signal") {
        EventRecorder recorder;

        // signal emited sync
        auto undoAvailableSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::undoAvailable));

        cursor.insertText("test test");

        recorder.waitForEvent(undoAvailableSignal);
        CHECK(recorder.consumeFirst(undoAvailableSignal, true));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("modificationChanged signal") {
        cursor.insertText("some\nlines");

        int testCase = GENERATE(0, 1, 2, 3, 4, 5);

        cursor.setPosition({2, 2});
        if (testCase == 5) {
            cursor.setAnchorPosition({1, 2});
        }

        // flush events
        QCoreApplication::instance()->processEvents();

        EventRecorder recorder;

        // signal emited sync
        auto modificationChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::modificationChanged));

        CHECK(recorder.noMoreEvents());

        if (testCase == 0) {
            cursor.deleteLine();
        } else if (testCase == 1) {
            cursor.deleteWord();
        } else if (testCase == 2) {
            cursor.deletePreviousWord();
        } else if (testCase == 3) {
            cursor.deleteCharacter();
        } else if (testCase == 4) {
            cursor.deletePreviousCharacter();
        } else if (testCase == 5) {
            cursor.removeSelectedText();
        } else {
            FAIL("Test case not provided");
        }

        recorder.waitForEvent(modificationChangedSignal);
        CHECK(recorder.consumeFirst(modificationChangedSignal, true));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("position and anchor") {
        cursor.insertText("1234\nabcdefgh\nABCDE");

        SECTION("basic") {
            cursor.setPosition({3, 0});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{3, 0});
        }

        SECTION("out of range code unit") {
            cursor.setPosition({5, 0});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{4, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 0});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 4);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{4, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{4, 0});
        }

        SECTION("out of range line") {
            cursor.setPosition({5, 3});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{5, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 2});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{5, 2});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{5, 2});
        }

        SECTION("vertical movement column") {
            cursor.setPosition({3, 0});
            cursor.setVerticalMovementColumn(5);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 5);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{3, 0});
        }

        SECTION("selection by extend") {
            cursor.setPosition({3, 0});
            cursor.setPosition({1, 1}, true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.hasSelection() == true);
            CHECK(cursor.verticalMovementColumn() == 1);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 1});
        }

        SECTION("selection by extend - reversed") {
            cursor.setPosition({1, 1});
            cursor.setPosition({3, 0}, true);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.hasSelection() == true);
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 1});
        }

        SECTION("selection by setAnchor") {
            cursor.setPosition({1, 1});
            cursor.setAnchorPosition({3, 0});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.hasSelection() == true);
            CHECK(cursor.verticalMovementColumn() == 1);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 1});
        }

        SECTION("selection by setAnchor - reversed") {
            cursor.setPosition({3, 0});
            cursor.setAnchorPosition({1, 1});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.hasSelection() == true);
            CHECK(cursor.verticalMovementColumn() == 3);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 0});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 1});
        }

        SECTION("setVerticalMovementColumn") {
            cursor.setPosition({1, 1});
            cursor.setVerticalMovementColumn(100);
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 100);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 1});
        }

        SECTION("setPositionPreservingVerticalMovementColumn") {
            cursor.setPosition({1, 1});
            cursor.setPositionPreservingVerticalMovementColumn({3, 1});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 1});
            CHECK(cursor.hasSelection() == false);
            CHECK(cursor.verticalMovementColumn() == 1);
            CHECK(cursor.selectionStartPos() == Tui::ZDocumentCursor::Position{3, 1});
            CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{3, 1});
        }

        SECTION("inside code point") {
            cursor.insertText("\na😁b");

            cursor.setPosition({2, 3});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});

            cursor.setPositionPreservingVerticalMovementColumn({2, 3});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});

            cursor.setAnchorPosition({2, 3});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 3});
        }

        SECTION("inside cluster") {
            cursor.insertText("\nao\xCD\x9A" "b"); // U+035A COMBINING DOUBLE RING BELOW

            cursor.setPosition({2, 3});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});

            cursor.setPositionPreservingVerticalMovementColumn({2, 3});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 3});

            cursor.setAnchorPosition({2, 3});
            CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 3});
        }
    }


    SECTION("reset") {
        cursor.insertText("\ntest test");
        REQUIRE(doc.lineCount() == 2);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
        Tui::ZDocumentLineMarker marker{&doc, 1};

        doc.reset();

        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 0);
        CHECK(doc.isModified() == false);
        CHECK(doc.isUndoAvailable() == false);
        CHECK(doc.isRedoAvailable() == false);

        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(marker.line() == 0);
    }

    SECTION("reset contentsChanged signal") {
        cursor.insertText("\ntest test");

        // signal emitted async, so needs to be consumed
        EventRecorder recorder;
        auto changedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::contentsChanged));

        recorder.waitForEvent(changedSignal);
        CHECK(recorder.consumeFirst(changedSignal));
        CHECK(recorder.noMoreEvents());

        doc.reset();
        recorder.waitForEvent(changedSignal);
        CHECK(recorder.consumeFirst(changedSignal));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("reset cursorChanged signal") {
        // brittle: The order of signals emitted for changed cursors in not specified
        cursor.insertText("\ntest test");
        REQUIRE(doc.lineCount() == 2);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});

        // signal emitted async, so needs to be consumed
        EventRecorder recorder;
        auto cursorChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::cursorChanged));

        recorder.waitForEvent(cursorChangedSignal);
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
        CHECK(recorder.noMoreEvents());

        doc.reset();
        recorder.waitForEvent(cursorChangedSignal);
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("reset lineMarkerChanged signal") {
        cursor.insertText("\ntest test");
        Tui::ZDocumentLineMarker marker{&doc, 1};

        // signal emitted async
        EventRecorder recorder;
        auto lineMarkerChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::lineMarkerChanged));

        doc.reset();
        recorder.waitForEvent(lineMarkerChangedSignal);
        CHECK(recorder.consumeFirst(lineMarkerChangedSignal, (const Tui::ZDocumentLineMarker*)&marker));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("reset undoAvailable signal") {
        cursor.insertText("\ntest test");

        EventRecorder recorder;
        auto undoAvailableSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::undoAvailable));

        doc.reset();
        recorder.waitForEvent(undoAvailableSignal);
        CHECK(recorder.consumeFirst(undoAvailableSignal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("reset redoAvailable signal") {
        cursor.insertText("\ntest test");
        cursor.insertText("\ntest test");
        doc.undo(&cursor);

        EventRecorder recorder;
        auto redoAvailableSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::redoAvailable));

        doc.reset();
        recorder.waitForEvent(redoAvailableSignal);
        CHECK(recorder.consumeFirst(redoAvailableSignal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("reset modificationChanged signal") {
        cursor.insertText("\ntest test");

        EventRecorder recorder;
        auto modificationChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::modificationChanged));

        doc.reset();
        recorder.waitForEvent(modificationChangedSignal);
        CHECK(recorder.consumeFirst(modificationChangedSignal, false));
        CHECK(recorder.noMoreEvents());
    }


    SECTION("readFrom - writeTo") {
        QByteArray inData;
        QBuffer inFile(&inData);
        REQUIRE(inFile.open(QIODevice::ReadOnly));

        QByteArray outData;
        QBuffer outFile(&outData);
        REQUIRE(outFile.open(QIODevice::WriteOnly));

        SECTION("empty") {
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == true);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData.size() == 0);
        }

        SECTION("simple") {
            inData = QByteArray("line1\nline2\n");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("initial cursor position") {
            inData = QByteArray("line1\nline2\n");
            doc.readFrom(&inFile, Tui::ZDocumentCursor::Position{1, 1}, &cursor);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 1});
            CHECK(cursor.hasSelection() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("initial cursor position with out of range code unit") {
            inData = QByteArray("line1\nline2\n");
            doc.readFrom(&inFile, Tui::ZDocumentCursor::Position{10, 1}, &cursor);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 1});
            CHECK(cursor.hasSelection() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("initial cursor position with out of range line") {
            inData = QByteArray("line1\nline2\n");
            doc.readFrom(&inFile, Tui::ZDocumentCursor::Position{2, 10}, &cursor);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 1});
            CHECK(cursor.hasSelection() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("resets cursors and line markers") {
            cursor.insertText("\ntest test");
            REQUIRE(doc.lineCount() == 2);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            Tui::ZDocumentLineMarker marker{&doc, 1};

            inData = QByteArray("line1\nline2\n");
            doc.readFrom(&inFile);

            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
            CHECK(marker.line() == 0);
        }

        SECTION("simple - missing last linebreak") {
            inData = QByteArray("line1\nline2");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == true);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("mixed line endings") {
            inData = QByteArray("line1\r\nline2\nline3\n");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1\r", "line2", "line3" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("mixed line endings - last line missing CR") {
            inData = QByteArray("line1\r\nline2\r\nline3\n");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1\r", "line2\r", "line3" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("mixed line endings - missing last linebreak") {
            inData = QByteArray("line1\r\nline2\nline3");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1\r", "line2", "line3" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == true);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("crlf line endings") {
            inData = QByteArray("line1\r\nline2\r\n");

            EventRecorder recorder;
            auto changedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::crLfModeChanged));

            doc.readFrom(&inFile);

            recorder.waitForEvent(changedSignal);
            CHECK(recorder.consumeFirst(changedSignal, true));
            CHECK(recorder.noMoreEvents());

            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == true);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("crlf line endings - missing last linebreak") {
            inData = QByteArray("line1\r\nline2");
            doc.readFrom(&inFile);
            CHECK(docToVec(doc) == QVector<QString>{ "line1", "line2" });
            CHECK(doc.crLfMode() == true);
            CHECK(doc.newlineAfterLastLineMissing() == true);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("with NUL bytes") {
            inData = QByteArray("li\0e1\nline2\n", 12);
            doc.readFrom(&inFile);
            std::array<char16_t, 5> expectedLine1 = {'l', 'i', 0x00, 'e', '1'};
            CHECK(docToVec(doc) == QVector<QString>{ QString::fromUtf16(expectedLine1.data(), expectedLine1.size()), "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("not valid utf8") {
            // \x80\x89\xa0\xff is not valid utf8, so expect surrogate escape encoding.
            // 'e' as '\145' to work around c++ string literal parsing
            inData = QByteArray("li\x80\x89\xa0\xff\1451\nline2\n", 15);
            doc.readFrom(&inFile);
            std::array<char16_t, 8> expectedLine1 = {'l', 'i', 0xdc80, 0xdc89, 0xdca0, 0xdcff, 'e', '1'};
            CHECK(docToVec(doc) == QVector<QString>{ QString::fromUtf16(expectedLine1.data(), expectedLine1.size()), "line2" });
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("with very long binary line") {
            // internal buffer is 16384 bytes

            // unseeded is ok
            std::mt19937 gen;
            std::independent_bits_engine<std::mt19937, 32, unsigned int> eng(gen);
            QByteArray random;
            random.fill('X', 16384 * 2 + 256);
            char *buf = random.data();
            for (int i = 0; i < random.size(); i += 4) {
                unsigned int item = eng();
                memcpy(&item, buf + i, 4);
            }

            inData = random + "\nsecond line\n";
            doc.readFrom(&inFile);
            CHECK(doc.lineCount() == 2);
            CHECK(doc.crLfMode() == false);
            CHECK(doc.newlineAfterLastLineMissing() == false);
            CHECK(doc.isModified() == false);
            CHECK(doc.isUndoAvailable() == false);
            CHECK(doc.isRedoAvailable() == false);

            doc.writeTo(&outFile, doc.crLfMode());
            CHECK(outData == inData);
        }

        SECTION("signals") {
            inData = QByteArray("line1\nline2\n");

            EventRecorder recorderModificationChanged;
            auto modificationChangedSignal = recorderModificationChanged.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::modificationChanged));

            EventRecorder recorderRedoAvailable;
            auto redoAvailableSignal = recorderRedoAvailable.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::redoAvailable));

            EventRecorder recorderUndoAvailable;
            auto undoAvailableSignal = recorderUndoAvailable.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::undoAvailable));

            EventRecorder recorderContentsChanged;
            auto contentsChangedSignal = recorderContentsChanged.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::contentsChanged));

            doc.readFrom(&inFile);

            recorderModificationChanged.waitForEvent(modificationChangedSignal);
            CHECK(recorderModificationChanged.consumeFirst(modificationChangedSignal, false));
            CHECK(recorderModificationChanged.noMoreEvents());

            recorderRedoAvailable.waitForEvent(redoAvailableSignal);
            CHECK(recorderRedoAvailable.consumeFirst(redoAvailableSignal, false));
            CHECK(recorderRedoAvailable.noMoreEvents());

            recorderUndoAvailable.waitForEvent(undoAvailableSignal);
            CHECK(recorderUndoAvailable.consumeFirst(undoAvailableSignal, false));
            CHECK(recorderUndoAvailable.noMoreEvents());

            recorderContentsChanged.waitForEvent(contentsChangedSignal);
            CHECK(recorderContentsChanged.consumeFirst(contentsChangedSignal));
            CHECK(recorderContentsChanged.noMoreEvents());
        }

        SECTION("cursors and line marker change signals") {
            cursor.insertText("\ntest test");
            REQUIRE(doc.lineCount() == 2);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
            Tui::ZDocumentLineMarker marker{&doc, 1};

            inData = QByteArray("line1\nline2\n");

            EventRecorder recorderLineMarkerChanged;
            auto lineMarkerChangedSignal = recorderLineMarkerChanged.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::lineMarkerChanged));

            EventRecorder recorderCursorChanged;
            auto cursorChangedSignal = recorderCursorChanged.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::cursorChanged));

            recorderCursorChanged.waitForEvent(cursorChangedSignal);
            CHECK(recorderCursorChanged.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
            CHECK(recorderCursorChanged.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
            CHECK(recorderCursorChanged.noMoreEvents());

            doc.readFrom(&inFile);

            recorderCursorChanged.waitForEvent(cursorChangedSignal);
            CHECK(recorderCursorChanged.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
            CHECK(recorderCursorChanged.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
            CHECK(recorderCursorChanged.noMoreEvents());

            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

            recorderLineMarkerChanged.waitForEvent(lineMarkerChangedSignal);
            CHECK(recorderLineMarkerChanged.consumeFirst(lineMarkerChangedSignal, (const Tui::ZDocumentLineMarker*)&marker));
            CHECK(recorderLineMarkerChanged.noMoreEvents());

            CHECK(marker.line() == 0);
        }
    }


    SECTION("markUndoStateAsSaved") {
        CHECK(doc.isModified() == false);
        CHECK(doc.isUndoAvailable() == false);
        CHECK(doc.isRedoAvailable() == false);

        cursor.insertText("abc");

        CHECK(doc.isModified() == true);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == false);

        doc.markUndoStateAsSaved();

        CHECK(doc.isModified() == false);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == false);

        cursor.insertText("def");

        CHECK(doc.isModified() == true);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == false);

        doc.undo(&cursor);

        // normally insertText "abc" followed by "def" would collapse and we would have no undo available here
        // with a markUndoStateAsSaved inbetween the intermediate undo step is preserved.
        CHECK(doc.isModified() == false);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == true);

        doc.undo(&cursor);
        // undo to a state before last markUndoStateAsSaved also reads as modified
        CHECK(doc.isModified() == true);
        CHECK(doc.isUndoAvailable() == false);
        CHECK(doc.isRedoAvailable() == true);

        doc.redo(&cursor);
        // now back at the marked as saved state
        CHECK(doc.isModified() == false);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == true);

        doc.redo(&cursor);
        CHECK(doc.isModified() == true);
        CHECK(doc.isUndoAvailable() == true);
        CHECK(doc.isRedoAvailable() == false);
    }

    SECTION("markUndoStateAsSaved signals") {
        cursor.insertText("abc");

        EventRecorder recorder;
        auto modificationChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::modificationChanged));

        doc.markUndoStateAsSaved();

        recorder.waitForEvent(modificationChangedSignal);
        CHECK(recorder.consumeFirst(modificationChangedSignal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("filename") {
        CHECK(doc.filename() == QString(""));
        doc.setFilename("some.txt");
        CHECK(doc.filename() == QString("some.txt"));
        doc.setFilename("other.txt");
        CHECK(doc.filename() == QString("other.txt"));
    }

    SECTION("ZDocumentCursor-Position-without-text") {

        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        //check the original after delete any text
        bool afterDeletedText = GENERATE(false, true);
        CAPTURE(afterDeletedText);
        if (afterDeletedText) {
            cursor.insertText("test test\ntest test\ntest test\n");
            REQUIRE(doc.lineCount() == 4);
            cursor.selectAll();
            cursor.removeSelectedText();
            REQUIRE(doc.lineCount() == 1);
        }

        cursor.moveCharacterLeft(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveWordLeft(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveToStartOfLine(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        cursor.moveCharacterRight(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveWordRight(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveToEndOfLine(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        cursor.moveToStartOfDocument(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveToStartOfDocument(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        cursor.moveUp();
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveDown();
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        cursor.setPosition({3, 2}, selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
    }

    SECTION("ZDocumentCursor-Position-with-text") {

        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        cursor.insertText("test test\ntest test\ntest test");
        REQUIRE(doc.lineCount() == 3);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});


        cursor.moveCharacterLeft(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{8, 2});
        cursor.moveWordLeft(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 2});
        cursor.moveToStartOfLine(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});

        cursor.moveCharacterRight(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 2});
        cursor.moveWordRight(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
        cursor.moveToEndOfLine(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});

        cursor.moveToStartOfDocument(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        cursor.moveToEndOfDocument(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});

        cursor.moveUp();
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
        cursor.moveDown();
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});

        cursor.setPosition({3, 2}, selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 2});
    }




    SECTION("move-up") {
        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        cursor.insertText("test test\ntest test\ntest test");
        cursor.setPosition({0, 1});
        cursor.moveUp(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
    }

    SECTION("move-up-position1-1") {
        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        cursor.insertText("test test\ntest test\ntest test");
        cursor.setPosition({1, 1});
        cursor.moveUp(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
    }

    SECTION("move-down") {
        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        cursor.insertText("test test\ntest test\ntest test");
        cursor.setPosition({0, 1});
        cursor.moveDown(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
    }

    SECTION("move-down-notext") {
        bool selection = GENERATE(false, true);
        CAPTURE(selection);

        cursor.insertText("test test\ntest test\ntest");
        cursor.setPosition({8, 1});
        cursor.moveDown(selection);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{4, 2});
    }

    SECTION("delete") {
        cursor.insertText("test test\ntest test\ntest test");
        REQUIRE(doc.lineCount() == 3);
        REQUIRE(doc.lineCodeUnits(0) == 9);
        REQUIRE(doc.lineCodeUnits(1) == 9);
        REQUIRE(doc.lineCodeUnits(2) == 9);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 2});

        cursor.deletePreviousCharacter();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test", "test tes"});
        REQUIRE(doc.lineCount() == 3);
        REQUIRE(doc.lineCodeUnits(0) == 9);
        REQUIRE(doc.lineCodeUnits(1) == 9);
        REQUIRE(doc.lineCodeUnits(2) == 8);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{8, 2});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test", "test "});
        REQUIRE(doc.lineCount() == 3);
        REQUIRE(doc.lineCodeUnits(0) == 9);
        REQUIRE(doc.lineCodeUnits(1) == 9);
        REQUIRE(doc.lineCodeUnits(2) == 5);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{5, 2});

        cursor.setPosition({0, 1});

        cursor.deleteCharacter();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "est test", "test "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(doc.lineCount() == 3);
        CHECK(doc.lineCodeUnits(0) == 9);
        CHECK(doc.lineCodeUnits(1) == 8);
        CHECK(doc.lineCodeUnits(2) == 5);

        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{"test test", " test", "test "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(doc.lineCount() == 3);
        CHECK(doc.lineCodeUnits(0) == 9);
        CHECK(doc.lineCodeUnits(1) == 5);
        CHECK(doc.lineCodeUnits(2) == 5);
    }

    SECTION("delete-newline") {
        cursor.insertText("\n\n\n\n\n\n");
        for (int i = 6; i > 0; i--) {
            CAPTURE(i);
            cursor.deletePreviousCharacter();
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, i - 1});
        }
        CHECK(docToVec(doc) == QVector<QString>{""});

        cursor.insertText("\n\n\n\n\n\n");
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 6});
        CHECK(doc.lineCount() == 7);

        cursor.deletePreviousCharacter();
        CHECK(docToVec(doc) == QVector<QString>{"", "", "", "", "", ""});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 5});
        CHECK(doc.lineCount() == 6);

        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"", "", "", "", ""});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 4});
        CHECK(doc.lineCount() == 5);

        cursor.setPosition({0, 2});

        cursor.deleteCharacter();
        CHECK(docToVec(doc) == QVector<QString>{"", "", "", ""});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
        CHECK(doc.lineCount() == 4);

        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{"", "", ""});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
        CHECK(doc.lineCount() == 3);
    }

    SECTION("delete-word") {
        cursor.insertText("a bb  ccc   dddd     eeeee");
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a bb  ccc   dddd     "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{21, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a bb  ccc   "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{12, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a bb  "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a "});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{""});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});

        cursor.insertText("a bb  ccc   dddd     eeeee");
        cursor.setPosition({7, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a bb  cc   dddd     eeeee"});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 0});

        cursor.selectAll();
        cursor.insertText("a bb  ccc   dddd     eeeee");
        cursor.setPosition({8, 0});
        cursor.deletePreviousWord();
        CHECK(docToVec(doc) == QVector<QString>{"a bb  c   dddd     eeeee"});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 0});

        cursor.selectAll();
        cursor.insertText("a bb  ccc   dddd     eeeee");
        cursor.setPosition({0, 0});
        CHECK(doc.lineCodeUnits(0) == 26);
        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{" bb  ccc   dddd     eeeee"});
        CHECK(doc.lineCodeUnits(0) == 25);
        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{"  ccc   dddd     eeeee"});
        CHECK(doc.lineCodeUnits(0) == 22);
        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{"   dddd     eeeee"});
        CHECK(doc.lineCodeUnits(0) == 17);
        cursor.deleteWord();
        CHECK(docToVec(doc) == QVector<QString>{"     eeeee"});
        CHECK(doc.lineCodeUnits(0) == 10);
        cursor.deleteWord();
        CHECK(doc.lineCodeUnits(0) == 0);
    }

    SECTION("delete-line") {
        bool emptyline = GENERATE(true, false);
        if (emptyline) {
            cursor.insertText("test test\n\ntest test");
            cursor.setPosition({0, 1});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
        } else {
            cursor.insertText("test test\nremove remove\ntest test");
            cursor.setPosition({4, 0});
            cursor.moveDown(true);
            cursor.moveWordRight(true);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{6, 1});
        }
        REQUIRE(doc.lineCount() == 3);

        cursor.deleteLine();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test"});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(doc.lineCount() == 2);
    }

    SECTION("delete-first-line") {
        bool emptyline = GENERATE(true, false);
        CAPTURE(emptyline);
        if (emptyline) {
            cursor.insertText("\ntest test\ntest test");
            cursor.setPosition({0, 0});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        } else {
            cursor.insertText("remove remove\ntest test\ntest test");
            cursor.setPosition({0, 1});
            cursor.moveUp(true);
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        }
        REQUIRE(doc.lineCount() == 3);
        cursor.deleteLine();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test"});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(doc.lineCount() == 2);
        CHECK(cursor.hasSelection() == false);
    }

    SECTION("delete-last-line") {
        bool emptyline = GENERATE(true, false);
        CAPTURE(emptyline);
        if (emptyline) {
            cursor.insertText("test test\ntest test\n");
            cursor.setPosition({0, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
        } else {
            cursor.insertText("test test\ntest test\nremove remove");
            cursor.setPosition({0, 2});
            CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 2});
        }
        REQUIRE(doc.lineCount() == 3);
        cursor.deleteLine();
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test"});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{9, 1});
        CHECK(doc.lineCount() == 2);
        CHECK(cursor.hasSelection() == false);
    }

    SECTION("hasSelection-and-pos") {
        cursor.insertText("");
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());

        cursor.moveCharacterLeft(true);
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());

        cursor.moveCharacterRight(true);
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());

        cursor.insertText(" ");
        cursor.moveCharacterLeft(true);
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.moveCharacterRight(true);
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.clearSelection();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.moveCharacterLeft(true);
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.moveCharacterRight(true);
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.selectAll();
        CHECK(cursor.hasSelection() == true);
        cursor.insertText(" ");
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.selectionStartPos() == cursor.position());
        CHECK(cursor.selectionEndPos() == cursor.position());
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
    }

    SECTION("no-selection") {
        cursor.insertText("abc");
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{3, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{3, 0});

        cursor.moveCharacterLeft();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});

        cursor.moveCharacterLeft(true);
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.moveCharacterRight(true);
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{2, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{2, 0});

        cursor.deletePreviousCharacter();
        CHECK(doc.line(0) == "ac");
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
    }

    SECTION("selectAll") {
        cursor.selectAll();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.selectedText() == "");

        cursor.clearSelection();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.selectedText() == "");

        cursor.clearSelection();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.selectedText() == "");

        cursor.insertText(" ");
        CHECK(cursor.selectedText() == "");
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.selectAll();
        CHECK(cursor.selectedText() == " ");
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.clearSelection();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});
        CHECK(cursor.selectedText() == "");

        cursor.moveCharacterLeft();

        cursor.clearSelection();
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.selectedText() == "");

        cursor.selectAll();
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{1, 0});

        cursor.insertText("\n");
        CHECK(cursor.selectedText() == "");
        CHECK(cursor.hasSelection() == false);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});

        cursor.selectAll();
        CHECK(cursor.selectedText() == "\n");
        CHECK(cursor.hasSelection() == true);
        CHECK(cursor.anchor() == Tui::ZDocumentCursor::Position{0, 0});
        CHECK(cursor.position() == Tui::ZDocumentCursor::Position{0, 1});
    }

    SECTION("removeSelectedText") {
        CHECK(cursor.hasSelection() == false);
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 0);

        cursor.selectAll();
        CHECK(cursor.hasSelection() == false);
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 0);

        cursor.insertText(" ");
        cursor.selectAll();
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 0);

        cursor.insertText("\n");
        CHECK(doc.lineCount() == 2);
        cursor.selectAll();
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 0);

        cursor.insertText("aRemovEb");
        cursor.moveCharacterLeft();
        for (int i = 0; i < 6; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "RemovE");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 2);
        CHECK(doc.line(0) == "ab");

        //clear
        cursor.selectAll();
        cursor.removeSelectedText();

        cursor.insertText("RemovEb");
        cursor.moveCharacterLeft();
        for (int i = 0; i < 6; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "RemovE");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 1);
        CHECK(doc.line(0) == "b");

        //clear
        cursor.selectAll();
        cursor.removeSelectedText();

        cursor.insertText("aRemovE");
        for (int i = 0; i < 6; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "RemovE");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 1);
        CHECK(doc.line(0) == "a");

        //clear
        cursor.selectAll();
        cursor.removeSelectedText();

        cursor.insertText("aRem\novEb");
        cursor.moveCharacterLeft();
        for (int i = 0; i < 7; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "Rem\novE");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 2);
        CHECK(doc.line(0) == "ab");

        //clear
        cursor.selectAll();
        cursor.removeSelectedText();

        cursor.insertText("aRem\novE\n");
        for (int i = 0; i < 8; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "Rem\novE\n");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 1);
        CHECK(doc.line(0) == "a");

        //clear
        cursor.selectAll();
        cursor.removeSelectedText();

        cursor.insertText("\nRem\novEb");
        cursor.moveCharacterLeft();
        for (int i = 0; i < 8; i++) {
            cursor.moveCharacterLeft(true);
        }
        CHECK(cursor.selectedText() == "\nRem\novE");
        cursor.removeSelectedText();
        CHECK(doc.lineCount() == 1);
        CHECK(doc.lineCodeUnits(0) == 1);
        CHECK(doc.line(0) == "b");
    }

    SECTION("removeSelectedText cursorChanged signal") {
        cursor.insertText("some test text");
        wrappedCursor.setPosition({4, 0}, true);

        // signal emitted async, so needs to be consumed
        EventRecorder recorder;
        auto cursorChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::cursorChanged));

        recorder.waitForEvent(cursorChangedSignal);
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
        CHECK(recorder.noMoreEvents());

        wrappedCursor.removeSelectedText();

        recorder.waitForEvent(cursorChangedSignal);
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&cursor));
        CHECK(recorder.consumeFirst(cursorChangedSignal, (const Tui::ZDocumentCursor*)&wrappedCursor));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("removeSelectedText lineMarkerChanged signal") {
        cursor.insertText("\ntext");
        wrappedCursor.setPosition({0, 0});
        wrappedCursor.setPosition({0, 1}, true);

        EventRecorder recorder;
        auto lineMarkerChangedSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::lineMarkerChanged));

        Tui::ZDocumentLineMarker marker{&doc, 1};
        wrappedCursor.removeSelectedText();

        recorder.waitForEvent(lineMarkerChangedSignal);
        CAPTURE(docToVec(doc));
        CHECK(recorder.consumeFirst(lineMarkerChangedSignal, (const Tui::ZDocumentLineMarker*)&marker));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("moveLines") {
        cursor.insertText("one\ntwo\nthree\nfour\nfive");

        SECTION("same") {
            doc.moveLine(3, 3, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "two", "three", "four", "five"
                  });
        }

        SECTION("up") {
            doc.moveLine(3, 1, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "four", "two", "three", "five"
                  });
        }

        SECTION("up prev") {
            doc.moveLine(3, 2, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "two", "four", "three", "five"
                  });
        }

        SECTION("up 4-3") {
            doc.moveLine(4, 3, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "two", "three", "five", "four"
                  });
        }

        SECTION("up 4-2") {
            doc.moveLine(4, 2, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "two", "five", "three", "four"
                  });
        }

        SECTION("up 4-1") {
            doc.moveLine(4, 1, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "five", "two", "three", "four"
                  });
        }

        SECTION("up 4-0") {
            doc.moveLine(4, 0, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "five", "one", "two", "three", "four"
                  });
        }

        SECTION("down") {
            doc.moveLine(1, 3, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                    "one", "three", "four", "two", "five"
                  });

        }

        SECTION("down next") {
            doc.moveLine(1, 2, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                      "one", "three", "two", "four", "five"
                  });

        }

        SECTION("down 0-1") {
            doc.moveLine(0, 1, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                      "two", "one", "three", "four", "five"
                  });

        }

        SECTION("down 0-2") {
            doc.moveLine(0, 2, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                      "two", "three", "one", "four", "five"
                  });

        }

        SECTION("down 0-3") {
            doc.moveLine(0, 3, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                      "two", "three", "four", "one", "five"
                  });

        }

        SECTION("down 0-4") {
            doc.moveLine(0, 4, &cursor);

            CHECK(docToVec(doc) == QVector<QString>{
                      "two", "three", "four", "five", "one"
                  });

        }

    }


    SECTION("at") {
        CHECK(cursor.atStart() == true);
        CHECK(cursor.atEnd() == true);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == true);

        cursor.insertText(" ");
        CHECK(cursor.atStart() == false);
        CHECK(cursor.atEnd() == true);
        CHECK(cursor.atLineStart() == false);
        CHECK(cursor.atLineEnd() == true);

        cursor.insertText("\n");
        CHECK(cursor.atStart() == false);
        CHECK(cursor.atEnd() == true);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == true);

        cursor.insertText(" ");
        CHECK(cursor.atStart() == false);
        CHECK(cursor.atEnd() == true);
        CHECK(cursor.atLineStart() == false);
        CHECK(cursor.atLineEnd() == true);

        cursor.moveCharacterLeft();
        CHECK(cursor.atStart() == false);
        CHECK(cursor.atEnd() == false);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == false);

        cursor.moveToStartOfDocument();
        CHECK(cursor.atStart() == true);
        CHECK(cursor.atEnd() == false);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == false);

        //clear
        cursor.selectAll();
        cursor.insertText("\n");
        CHECK(cursor.atStart() == false);
        CHECK(cursor.atEnd() == true);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == true);

        cursor.moveUp();
        CHECK(cursor.atStart() == true);
        CHECK(cursor.atEnd() == false);
        CHECK(cursor.atLineStart() == true);
        CHECK(cursor.atLineEnd() == true);

    }


    SECTION("userdata") {
        cursor.insertText("test\ntest");
        REQUIRE(doc.lineCount() == 2);
        const unsigned int revision1 = doc.revision();
        const unsigned int line0revision1 = doc.lineRevision(0);
        const auto userdata1 = std::make_shared<TestUserData>();
        doc.setLineUserData(0, userdata1);

        // doc and line revision do not change on user data update
        CHECK(doc.revision() == revision1);
        CHECK(doc.lineRevision(0) == line0revision1);
        CHECK(doc.lineUserData(0) == userdata1);
        CHECK(doc.lineUserData(1) == nullptr);

        cursor.setPosition({0, 0});
        cursor.insertText("new");

        const unsigned int revision2 = doc.revision();
        const unsigned int line0revision2 = doc.lineRevision(0);

        CHECK(revision1 != revision2);
        CHECK(line0revision1 != line0revision2);
        // User data stays on line on modification
        CHECK(doc.lineUserData(0) == userdata1);
        CHECK(doc.lineUserData(1) == nullptr);

        const auto userdata2 = std::make_shared<TestUserData>();
        doc.setLineUserData(0, userdata2);

        CHECK(revision1 != revision2);
        CHECK(line0revision1 != line0revision2);

        // The new snapshot does have the newly set user data.
        CHECK(doc.lineUserData(0) == userdata2);
        CHECK(doc.lineUserData(1) == nullptr);

        // line data stays with left part of split line
        cursor.insertText("\nnova");
        CHECK(doc.lineUserData(0) == userdata2);
        CHECK(doc.lineUserData(1) == nullptr);
        CHECK(doc.lineUserData(2) == nullptr);

        const auto userdata3 = std::make_shared<TestUserData>();
        doc.setLineUserData(1, userdata3);
        CHECK(doc.lineUserData(1) == userdata3);

        cursor.setPosition({0, 0});
        cursor.moveToEndOfLine();

        // When removing line break, user data from top line is preserved.
        cursor.deleteCharacter();
        CHECK(doc.lineUserData(0) == userdata2);
        CHECK(doc.lineUserData(1) == nullptr);
    }


    SECTION("snapshot") {
        cursor.insertText("test\ntest");
        const auto userdata1 = std::make_shared<TestUserData>();
        doc.setLineUserData(0, userdata1);

        CHECK(docToVec(doc) == QVector<QString>{"test", "test"});

        Tui::ZDocumentSnapshot snap = doc.snapshot();

        CHECK(snap.isUpToDate() == true);
        CHECK(snap.revision() == doc.revision());
        CHECK(snapToVec(snap) == QVector<QString>{"test", "test"});
        REQUIRE(snap.lineCount() == 2);
        CHECK(snap.lineCodeUnits(0) == 4);
        CHECK(snap.lineCodeUnits(1) == 4);
        REQUIRE(doc.lineCount() == 2);
        CHECK(snap.lineUserData(0) == userdata1);
        CHECK(snap.lineUserData(1) == nullptr);
        CHECK(snap.lineRevision(0) == doc.lineRevision(0));
        CHECK(snap.lineRevision(1) == doc.lineRevision(1));

        cursor.insertText("new");

        CHECK(snap.isUpToDate() == false);
        CHECK(snap.revision() != doc.revision());
        CHECK(snapToVec(snap) == QVector<QString>{"test", "test"});
        CHECK(snap.lineUserData(0) == userdata1);
        CHECK(snap.lineUserData(1) == nullptr);
        CHECK(snap.lineRevision(0) == doc.lineRevision(0));
        CHECK(snap.lineRevision(1) != doc.lineRevision(1));

        Tui::ZDocumentSnapshot snap2 = doc.snapshot();
        CHECK(snap2.isUpToDate() == true);
        CHECK(snap2.revision() == doc.revision());
        CHECK(snapToVec(snap2) == QVector<QString>{"test", "testnew"});
        REQUIRE(snap2.lineCount() == 2);
        CHECK(snap2.lineCodeUnits(0) == 4);
        CHECK(snap2.lineCodeUnits(1) == 7);
        REQUIRE(doc.lineCount() == 2);
        CHECK(snap2.lineUserData(0) == userdata1);
        CHECK(snap2.lineUserData(1) == nullptr);
        CHECK(snap2.lineRevision(0) == doc.lineRevision(0));
        CHECK(snap2.lineRevision(1) == doc.lineRevision(1));

        const auto userdata2 = std::make_shared<TestUserData>();
        doc.setLineUserData(0, userdata2);

        // Changing line user data does not invalidate snapshots
        CHECK(snap2.isUpToDate() == true);

        // But the snapshot still has the old data
        CHECK(snap2.lineUserData(0) == userdata1);
        CHECK(snap2.lineUserData(1) == nullptr);

        Tui::ZDocumentSnapshot snap3 = doc.snapshot();
        // The new snapshot does have the newly set user data.
        CHECK(snap3.lineUserData(0) == userdata2);
        CHECK(snap3.lineUserData(1) == nullptr);
    }

    SECTION("overwriteText") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "TEST";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"TEST test", "test test"});
    }

    SECTION("overwriteText-long-line") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "TEST TEST TEST";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"TEST TEST TEST", "test test"});
    }

    SECTION("overwriteText-empty-with-replace-count") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "";
        cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test"});
    }

    SECTION("overwriteText-empty") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "";
        cursor.overwriteText(s);
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test test"});
    }

    SECTION("overwriteText-new-line") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "TEST\nTEST";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"TEST", "TESTt", "test test"});
    }

    SECTION("overwriteText-U+1F603") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({0, 0});
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            const QString s = "😃😃";
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            // manually split here, because both parts are 2 UTF-16 code units and the for loop doesn't get that right.
            cursor.overwriteText("😃");
            cursor.overwriteText("😃");
        }
        CHECK(docToVec(doc) == QVector<QString>{"😃😃st test", "test test"});
    }

    SECTION("overwriteText-U+1F603-at-end-of-line") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({8, 0});
        const QString s = "😃";
        cursor.overwriteText(s);
        CHECK(docToVec(doc) == QVector<QString>{"test tes😃", "test test"});
    }

    SECTION("overwriteText-overwrite-U+1F603") {
        cursor.insertText("😃😃😃 test\ntest test");
        cursor.setPosition({0, 0});
        const QString s = "TS";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"TS😃 test", "test test"});
    }

    SECTION("overwriteText-almost-at-and") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({8, 1});
        const QString s = "TEST";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test tesTEST"});
    }

    SECTION("overwriteText-at-and") {
        cursor.insertText("test test\ntest test");
        const QString s = "TEST\nTEST";
        bool inOneStep = GENERATE(true, false);
        CAPTURE(inOneStep);
        if (inOneStep) {
            cursor.overwriteText(s, textMetrics.sizeInClusters(s));
        } else {
            for (QString c: s) {
                cursor.overwriteText(c);
            }
        }
        CHECK(docToVec(doc) == QVector<QString>{"test test", "test testTEST", "TEST"});
    }

    SECTION("overwriteText-seletion") {
        cursor.insertText("test test\ntest test");
        cursor.setPosition({1, 0});
        cursor.setAnchorPosition({3, 0});
        CHECK(cursor.selectedText() == "es");
        const QString s = "EST";
        bool inOneStep = GENERATE(true, false);
        if (inOneStep) {
            int ignoredCodeUnitsValue = GENERATE(0, 3, 12);
            cursor.overwriteText(s, ignoredCodeUnitsValue);
            CHECK(docToVec(doc) == QVector<QString>{"tESTt test", "test test"});
        } else {
            foreach (QString c, s) {
                cursor.overwriteText(c);
            }
            CHECK(docToVec(doc) == QVector<QString>{"tESTtest", "test test"});
        }
    }

}


TEST_CASE("Tui::ZDocumentCursor::Position") {

    SECTION("default constructor") {
        Tui::ZDocumentCursor::Position pos;
        CHECK(pos.codeUnit == 0);
        CHECK(pos.line == 0);
    }

    SECTION("value constructor 5 7") {
        Tui::ZDocumentCursor::Position pos{5, 7};
        CHECK(pos.codeUnit == 5);
        CHECK(pos.line == 7);
    }

    SECTION("value constructor 7 5") {
        Tui::ZDocumentCursor::Position pos{7, 5};
        CHECK(pos.codeUnit == 7);
        CHECK(pos.line == 5);
    }


    SECTION("value constructor -2 -7") {
        Tui::ZDocumentCursor::Position pos{-2, -7};
        CHECK(pos.codeUnit == -2);
        CHECK(pos.line == -7);
    }

    SECTION("equals") {
        CHECK(Tui::ZDocumentCursor::Position{-2, -7} == Tui::ZDocumentCursor::Position{-2, -7});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{-2, -7} != Tui::ZDocumentCursor::Position{-2, -7});

        CHECK(Tui::ZDocumentCursor::Position{56, 12} == Tui::ZDocumentCursor::Position{56, 12});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{56, 12} != Tui::ZDocumentCursor::Position{56, 12});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{-2, -7} == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(Tui::ZDocumentCursor::Position{-2, -7} != Tui::ZDocumentCursor::Position{2, 7});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{15, 3} == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(Tui::ZDocumentCursor::Position{15, 3} != Tui::ZDocumentCursor::Position{2, 7});
    }

    SECTION("less") {
        CHECK(Tui::ZDocumentCursor::Position{2, 6} < Tui::ZDocumentCursor::Position{2, 7});
        CHECK(Tui::ZDocumentCursor::Position{2, 7} < Tui::ZDocumentCursor::Position{3, 7});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{-2, -7} < Tui::ZDocumentCursor::Position{-2, -7});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 7} < Tui::ZDocumentCursor::Position{2, 6});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{3, 7} < Tui::ZDocumentCursor::Position{2, 7});
    }

    SECTION("less or equal") {
        CHECK(Tui::ZDocumentCursor::Position{-2, -7} <= Tui::ZDocumentCursor::Position{-2, -7});
        CHECK(Tui::ZDocumentCursor::Position{2, 6} <= Tui::ZDocumentCursor::Position{2, 7});
        CHECK(Tui::ZDocumentCursor::Position{2, 7} <= Tui::ZDocumentCursor::Position{3, 7});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 7} <= Tui::ZDocumentCursor::Position{2, 6});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{3, 7} <= Tui::ZDocumentCursor::Position{2, 7});
    }

    SECTION("greater") {
        CHECK(Tui::ZDocumentCursor::Position{2, 7} > Tui::ZDocumentCursor::Position{2, 6});
        CHECK(Tui::ZDocumentCursor::Position{3, 7} > Tui::ZDocumentCursor::Position{2, 7});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{-2, -7} > Tui::ZDocumentCursor::Position{-2, -7});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 6} > Tui::ZDocumentCursor::Position{2, 7});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 7} > Tui::ZDocumentCursor::Position{3, 7});
    }

    SECTION("greater or equal") {
        CHECK(Tui::ZDocumentCursor::Position{-2, -7} >= Tui::ZDocumentCursor::Position{-2, -7});
        CHECK(Tui::ZDocumentCursor::Position{2, 7} >= Tui::ZDocumentCursor::Position{2, 6});
        CHECK(Tui::ZDocumentCursor::Position{3, 7} >= Tui::ZDocumentCursor::Position{2, 7});

        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 6} >= Tui::ZDocumentCursor::Position{2, 7});
        CHECK_FALSE(Tui::ZDocumentCursor::Position{2, 7} >= Tui::ZDocumentCursor::Position{3, 7});
    }

}

TEST_CASE("document cross doc") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc1;

    Tui::ZDocumentCursor cursor1{&doc1, [&textMetrics, &doc1](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc1.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    Tui::ZDocument doc2;

    Tui::ZDocumentCursor cursor2{&doc2, [&textMetrics, &doc2](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc2.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    SECTION("simple") {
        cursor1.insertText("doc1");
        cursor2.insertText("doc2");

        Tui::ZDocumentCursor curX = cursor1;
        curX.setPosition({0, 0});
        curX.setPosition({4, 0}, true);
        CHECK(curX.selectedText() == "doc1");

        curX = cursor2;

        curX.setPosition({0, 0});
        curX.setPosition({4, 0}, true);
        CHECK(curX.selectedText() == "doc2");
    }

    SECTION("cursorChanged signal") {
        cursor1.insertText("doc1");
        cursor2.insertText("doc2");

        Tui::ZDocumentCursor curX = cursor1;
        curX.setPosition({0, 0});
        curX.setPosition({4, 0}, true);
        CHECK(curX.selectedText() == "doc1");

        EventRecorder recorder1;
        auto cursorChangedSignal1 = recorder1.watchSignal(&doc1, RECORDER_SIGNAL(&Tui::ZDocument::cursorChanged));

        EventRecorder recorder2;
        auto cursorChangedSignal2 = recorder2.watchSignal(&doc2, RECORDER_SIGNAL(&Tui::ZDocument::cursorChanged));

        recorder1.waitForEvent(cursorChangedSignal1);
        CHECK(recorder1.consumeFirst(cursorChangedSignal1, (const Tui::ZDocumentCursor*)&cursor1));
        CHECK(recorder1.consumeFirst(cursorChangedSignal1, (const Tui::ZDocumentCursor*)&curX));
        CHECK(recorder1.noMoreEvents());

        recorder2.waitForEvent(cursorChangedSignal2);
        CHECK(recorder2.consumeFirst(cursorChangedSignal2, (const Tui::ZDocumentCursor*)&cursor2));
        CHECK(recorder2.noMoreEvents());

        curX = cursor2;

        CAPTURE(&cursor1);
        CAPTURE(&cursor2);
        CAPTURE(&curX);

        recorder2.waitForEvent(cursorChangedSignal2);
        CHECK(recorder2.consumeFirst(cursorChangedSignal2, (const Tui::ZDocumentCursor*)&curX));
        CHECK(recorder2.noMoreEvents());
        CHECK(recorder1.noMoreEvents());

        curX.setPosition({0, 0});
        curX.setPosition({4, 0}, true);
        CHECK(curX.selectedText() == "doc2");

        recorder2.waitForEvent(cursorChangedSignal2);
        CHECK(recorder2.consumeFirst(cursorChangedSignal2, (const Tui::ZDocumentCursor*)&curX));
        CHECK(recorder2.noMoreEvents());
        CHECK(recorder1.noMoreEvents());

        curX.removeSelectedText();
        recorder2.waitForEvent(cursorChangedSignal2);
        CHECK(recorder2.consumeFirst(cursorChangedSignal2, (const Tui::ZDocumentCursor*)&cursor2));
        CHECK(recorder2.consumeFirst(cursorChangedSignal2, (const Tui::ZDocumentCursor*)&curX));
        CHECK(recorder2.noMoreEvents());
        CHECK(recorder1.noMoreEvents());
    }


    SECTION("lineMarkerChanged signal") {
        cursor1.insertText("doc1\nline");
        cursor2.insertText("doc2\nline");

        Tui::ZDocumentLineMarker marker1{&doc1, 1};
        Tui::ZDocumentLineMarker marker2{&doc2, 1};
        Tui::ZDocumentLineMarker marker = marker1;

        EventRecorder recorder1;
        auto lineMarkerChangedSignal1 = recorder1.watchSignal(&doc1, RECORDER_SIGNAL(&Tui::ZDocument::lineMarkerChanged));

        EventRecorder recorder2;
        auto lineMarkerChangedSignal2 = recorder2.watchSignal(&doc2, RECORDER_SIGNAL(&Tui::ZDocument::lineMarkerChanged));

        QCoreApplication::processEvents(QEventLoop::AllEvents);
        CHECK(recorder1.noMoreEvents());
        CHECK(recorder2.noMoreEvents());

        marker = marker2;

        QCoreApplication::processEvents(QEventLoop::AllEvents);

        CHECK(recorder1.noMoreEvents());
        CHECK(recorder2.consumeFirst(lineMarkerChangedSignal2, (const Tui::ZDocumentLineMarker*)&marker));
        CHECK(recorder2.noMoreEvents());

        cursor2.setPosition({0, 0});
        cursor2.insertText("\n");

        QCoreApplication::processEvents(QEventLoop::AllEvents);

        CHECK(recorder2.consumeFirst(lineMarkerChangedSignal2, (const Tui::ZDocumentLineMarker*)&marker2));
        CHECK(recorder2.consumeFirst(lineMarkerChangedSignal2, (const Tui::ZDocumentLineMarker*)&marker));
        CHECK(recorder2.noMoreEvents());
        CHECK(recorder1.noMoreEvents());
    }
}
