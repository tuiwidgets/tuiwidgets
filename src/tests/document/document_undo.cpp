// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocumentCursor.h>
#include <Tui/ZDocumentLineMarker.h>

#include <optional>

#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>

#include "../catchwrapper.h"
#include "../eventrecorder.h"
#include "../Testhelper.h"


static QVector<QString> docToVec(const Tui::ZDocument &doc) {
    QVector<QString> ret;

    for (int i = 0; i < doc.lineCount(); i++) {
        ret.append(doc.line(i));
    }

    return ret;
}

TEST_CASE("Document Undo Redo") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    enum ActionType {
        Collapsed,
        Invisible,
        NormalStp
    };

    struct Action {
        ActionType type;
        std::function<void()> step;
    };

    auto runChecks = [&](std::initializer_list<Action> steps) {

        const bool groupSteps = GENERATE(false, true);
        CAPTURE(groupSteps);

        struct DocumentState {
            QList<QString> lines;
            bool noNewline = false;
            Tui::ZDocumentCursor::Position startCursorPos;
            Tui::ZDocumentCursor::Position endCursorPos;
        };

        QList<DocumentState> states;
        Tui::ZDocumentCursor::Position beforePos = cursor1.position();

        auto captureState = [&] {
            QList<QString> lines;
            for (int i = 0; i < doc.lineCount(); i++) {
                lines.append(doc.line(i));
            }
            states.append(DocumentState{lines, doc.newlineAfterLastLineMissing(), beforePos, cursor1.position()});
        };

        auto compareState = [&] (const DocumentState &state) {
            QList<QString> lines;
            for (int i = 0; i < doc.lineCount(); i++) {
                lines.append(doc.line(i));
            }
            CHECK(state.lines == lines);
            CHECK(state.noNewline == doc.newlineAfterLastLineMissing());
        };

        captureState();
        compareState(states.last());
        for (const auto& step: steps) {
            std::optional<Tui::ZDocument::UndoGroup> group;
            if (groupSteps) {
                group.emplace(doc.startUndoGroup(&cursor1));
            }
            step.step();
            if (step.type == NormalStp) {
                captureState();
                compareState(states.last());
                beforePos = cursor1.position();
            } else if (step.type == Invisible) {
                beforePos = cursor1.position();
            }
        }

        int stateIndex = states.size() - 1;


        while (doc.isUndoAvailable()) {
            CAPTURE(stateIndex);
            doc.undo(&cursor1);
            REQUIRE(stateIndex > 0);
            CHECK(states[stateIndex].startCursorPos == cursor1.position());
            stateIndex -= 1;
            compareState(states[stateIndex]);
        }

        REQUIRE(stateIndex == 0);

        while (doc.isRedoAvailable()) {
            doc.redo(&cursor1);
            REQUIRE(stateIndex + 1 < states.size());
            stateIndex += 1;
            compareState(states[stateIndex]);
            CHECK(states[stateIndex].endCursorPos == cursor1.position());
        }

        REQUIRE(stateIndex + 1 == states.size());
    };

    SECTION("undo on start") {
        // Should silently do nothing
        doc.undo(&cursor1);
    }

    SECTION("undo and change") {
        cursor1.insertText("first");
        doc.undo(&cursor1);
        CHECK(docToVec(doc) == QVector<QString>{""});
        cursor1.insertText("second");
        CHECK(docToVec(doc) == QVector<QString>{"second"});
        doc.undo(&cursor1);
        CHECK(docToVec(doc) == QVector<QString>{""});
        doc.redo(&cursor1);
        CHECK(docToVec(doc) == QVector<QString>{"second"});
    }

    SECTION("redo on start") {
        // Should silently do nothing
        doc.redo(&cursor1);
    }

    SECTION("undoAvailable signal") {
        cursor1.insertText("test test");

        doc.undo(&cursor1);

        EventRecorder recorder;

        // signal emited sync
        auto undoAvailableSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::undoAvailable));

        doc.redo(&cursor1);

        recorder.waitForEvent(undoAvailableSignal);
        CHECK(recorder.consumeFirst(undoAvailableSignal, true));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("redoAvailable signal") {
        cursor1.insertText("test test");

        EventRecorder recorder;

        // signal emited sync
        auto redoAvailableSignal = recorder.watchSignal(&doc, RECORDER_SIGNAL(&Tui::ZDocument::redoAvailable));

        doc.undo(&cursor1);

        recorder.waitForEvent(redoAvailableSignal);
        CHECK(recorder.consumeFirst(redoAvailableSignal, true));
        CHECK(recorder.noMoreEvents());

        doc.redo(&cursor1);

        recorder.waitForEvent(redoAvailableSignal);
        CHECK(recorder.consumeFirst(redoAvailableSignal, false));
        CHECK(recorder.noMoreEvents());
    }

    SECTION("inserts") {
        runChecks({
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { Collapsed, [&] { cursor1.insertText("b"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } }
                  });
    }

    SECTION("inserts2") {
        runChecks({
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { Collapsed, [&] { cursor1.insertText("b"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { Collapsed, [&] { cursor1.insertText(" "); } },
                      { Collapsed, [&] { cursor1.insertText("d"); } },
                      { Collapsed, [&] { cursor1.insertText("e"); } },
                      { NormalStp, [&] { cursor1.insertText("f"); } }
                  });
    }

    SECTION("insert and move") {
        runChecks({
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { Collapsed, [&] { cursor1.insertText("b"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { Invisible, [&] { cursor1.moveCharacterLeft(); } },
                      { Collapsed, [&] { cursor1.insertText("d"); } },
                      { Collapsed, [&] { cursor1.insertText("e"); } },
                      { NormalStp, [&] { cursor1.insertText("f"); } }
                  });
    }

    SECTION("insert and cancle collapsing") {
        runChecks({
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { Collapsed, [&] { cursor1.insertText("b"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { Invisible, [&] { doc.clearCollapseUndoStep(); } },
                      { Collapsed, [&] { cursor1.insertText("d"); } },
                      { Collapsed, [&] { cursor1.insertText("e"); } },
                      { NormalStp, [&] { cursor1.insertText("f"); } }
                  });
    }

    SECTION("no newline") {
        runChecks({
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { Collapsed, [&] { cursor1.insertText("b"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { NormalStp, [&] { doc.setNewlineAfterLastLineMissing(true); } }
                  });
    }

    SECTION("group with cursor movement") {
        runChecks({
                      { NormalStp, [&] { cursor1.insertText("    "); } },
                      { NormalStp, [&] { cursor1.insertText("    "); } },
                      { NormalStp, [&] { cursor1.insertText("    "); } },
                      { NormalStp, [&] {
                            const auto [cursorCodeUnit, cursorLine] = cursor1.position();
                            auto group = doc.startUndoGroup(&cursor1);
                            cursor1.setPosition({0, 0});
                            cursor1.setPosition({4, 0}, true);
                            cursor1.removeSelectedText();
                            cursor1.setPosition({cursorCodeUnit - 4, cursorLine});
                        } }
                  });
    }

    SECTION("newline") {
        runChecks({
                      { NormalStp, [&] { cursor1.insertText("\n"); } },
                      { Collapsed, [&] { cursor1.insertText("\n"); } },
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { NormalStp, [&] { cursor1.insertText("b"); } },
                      { Collapsed, [&] { cursor1.insertText("\n"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { NormalStp, [&] { cursor1.insertText("\n"); } }
                  });
    }

    SECTION("space") {
        runChecks({
                      { NormalStp, [&] { cursor1.insertText(" "); } },
                      { Collapsed, [&] { cursor1.insertText(" "); } },
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { NormalStp, [&] { cursor1.insertText("b"); } },
                      { Collapsed, [&] { cursor1.insertText(" "); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { NormalStp, [&] { cursor1.insertText(" "); } }
                  });
    }


    SECTION("tab") {
        runChecks({
                      { NormalStp, [&] { cursor1.insertText("\t"); } },
                      { Collapsed, [&] { cursor1.insertText("\t"); } },
                      { Collapsed, [&] { cursor1.insertText("a"); } },
                      { NormalStp, [&] { cursor1.insertText("b"); } },
                      { Collapsed, [&] { cursor1.insertText("\t"); } },
                      { NormalStp, [&] { cursor1.insertText("c"); } },
                      { NormalStp, [&] { cursor1.insertText("\t"); } }
                  });
    }

}

TEST_CASE("Document additional cursor adjustments") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    // Check if modifiation, undo and redo correctly moves other cursors active on the document.

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    QStringList lines;

    auto positionAt = [&] (QChar ch) -> Tui::ZDocumentCursor::Position {
        int lineNo = 0;
        for (const QString &line: lines) {
            if (line.contains(ch)) {
                return {line.indexOf(ch), lineNo};
            }
            lineNo += 1;
        }
        // should never be reached
        REQUIRE(false);
        return {0, 0};
    };

    auto posAdd = [] (Tui::ZDocumentCursor::Position in, int codeUnit, int line) -> Tui::ZDocumentCursor::Position {
        return {in.codeUnit + codeUnit, in.line + line};
    };

    SECTION("Insert midline") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curDtoC = cursor1;
        curDtoC.setPosition(positionAt('D'));
        curDtoC.setPosition(positionAt('C'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        cursor1.setPosition(positionAt('<'));
        const QString insertedText = "XXXXXXXXXXX";
        const int insertedLen = insertedText.size();
        cursor1.insertText(insertedText);



        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), insertedLen, 0));
        CHECK(curDtoC.anchor() == posAdd(positionAt('D'), insertedLen, 0));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curD.position() == posAdd(positionAt('D'), insertedLen, 0));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), insertedLen - 1, 0));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), insertedLen - 2, 0));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curDtoC.anchor() == positionAt('D'));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        Tui::ZDocumentCursor curSelectionBefore = curBefore;
        curSelectionBefore.moveCharacterRight(true);
        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        Tui::ZDocumentCursor curSelectionBeforeReverse = curBefore;
        curSelectionBeforeReverse.moveCharacterRight();
        curSelectionBeforeReverse.moveCharacterLeft(true);
        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        Tui::ZDocumentCursor curSelectionAfter = curBefore;
        curSelectionAfter.moveCharacterRight();
        curSelectionAfter.moveCharacterRight(true);
        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('>'), 2, 0));

        Tui::ZDocumentCursor curSelectionAfterReverse = curBefore;
        curSelectionAfterReverse.moveCharacterRight(false);
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterLeft(true);
        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('>'), 2, 0));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('>'), 1, 0));


        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), insertedLen, 0));
        CHECK(curDtoC.anchor() == posAdd(positionAt('D'), insertedLen, 0));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curInside2.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curInside3.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curInside4.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curInside1ToInside3.position() == posAdd(positionAt('<'), insertedLen, 0));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('>'), 1 + insertedLen, 0));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('>'), 2 + insertedLen, 0));

        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('>'), 2 + insertedLen, 0));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('>'), 1 + insertedLen, 0));
    }

    SECTION("split line") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curDtoC = cursor1;
        curDtoC.setPosition(positionAt('D'));
        curDtoC.setPosition(positionAt('C'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        cursor1.setPosition(positionAt('<'));
        const int insertionOff = cursor1.position().codeUnit;
        const int insertedLen2 = 0;
        cursor1.insertText("\n");


        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, 1));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curDtoC.anchor() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -insertionOff + insertedLen2, 1));
        CHECK(curD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, 1));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, 1));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, 1));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curDtoC.anchor() == positionAt('D'));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curSelectionBefore = curBefore;
        curSelectionBefore.moveCharacterRight(true);
        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        Tui::ZDocumentCursor curSelectionBeforeReverse = curBefore;
        curSelectionBeforeReverse.moveCharacterRight();
        curSelectionBeforeReverse.moveCharacterLeft(true);
        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        Tui::ZDocumentCursor curSelectionAfter = curBefore;
        curSelectionAfter.moveCharacterRight();
        curSelectionAfter.moveCharacterRight(true);
        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('>'), 2, 0));

        Tui::ZDocumentCursor curSelectionAfterReverse = curBefore;
        curSelectionAfterReverse.moveCharacterRight(false);
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterLeft(true);
        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('>'), 2, 0));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('>'), 1, 0));


        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curDtoC.anchor() == posD2);
        CHECK(curDtoC.position() == posC2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('<'), -insertionOff, 1));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('<'), -insertionOff + 1, 1));

        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('<'), -insertionOff + 1, 1));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('<'), -insertionOff, 1));
    }

    SECTION("Insert midline two lines") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curDtoC = cursor1;
        curDtoC.setPosition(positionAt('D'));
        curDtoC.setPosition(positionAt('C'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        cursor1.setPosition(positionAt('<'));
        const int insertionOff = cursor1.position().codeUnit;
        const QString insertedTextLine1 = "XXX";
        const QString insertedTextLine2 = "XXXXXXXX";
        const QString insertedText = insertedTextLine1 + "\n" + insertedTextLine2;
        const int insertedLen2 = insertedTextLine2.size();
        cursor1.insertText(insertedText);



        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, 1));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curDtoC.anchor() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -insertionOff + insertedLen2, 1));
        CHECK(curD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, 1));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, 1));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, 1));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -insertionOff + insertedLen2 - 1, 1));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -insertionOff + insertedLen2 - 2, 1));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curDtoC.anchor() == positionAt('D'));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curDtoC.anchor() == posD2);
        CHECK(curDtoC.position() == posC2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);
    }


    SECTION("Insert midline three lines") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        cursor1.setPosition(positionAt('<'));
        const int insertionOff = cursor1.position().codeUnit;
        const QString insertedTextLine1 = "XXX";
        const QString insertedTextLine2 = "XXXXXX";
        const QString insertedTextLine3 = "XXXXXXXX";
        const QString insertedText = insertedTextLine1 + "\n" + insertedTextLine2 + "\n" + insertedTextLine3;
        const int insertedLen3 = insertedTextLine3.size();
        cursor1.insertText(insertedText);



        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, 2));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen3, 2));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -insertionOff + insertedLen3, 2));
        CHECK(curD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen3, 2));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, 2));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, 2));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, 2));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -insertionOff + insertedLen3 - 1, 2));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -insertionOff + insertedLen3 - 2, 2));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);
    }

    SECTION("Remove midline") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX< D",
            "E            F"
        };

        const int removedChars = lines[1].count("X");

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curDtoC = cursor1;
        curDtoC.setPosition(positionAt('D'));
        curDtoC.setPosition(positionAt('C'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -1, 0));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -2, 0));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -removedChars, 0));
        CHECK(curDtoC.anchor() == posAdd(positionAt('D'), -removedChars, 0));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -removedChars, 0));
        CHECK(curD.position() == posAdd(positionAt('D'), -removedChars, 0));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside2.hasSelection() == false);

        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside3.hasSelection() == false);

        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curSelectionBefore = curBefore;
        curSelectionBefore.moveCharacterRight(true);
        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        Tui::ZDocumentCursor curSelectionBeforeReverse = curBefore;
        curSelectionBeforeReverse.moveCharacterRight();
        curSelectionBeforeReverse.moveCharacterLeft(true);
        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        Tui::ZDocumentCursor curSelectionAfter = curBefore;
        curSelectionAfter.moveCharacterRight();
        curSelectionAfter.moveCharacterRight(true);
        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('>'), 2, 0));

        Tui::ZDocumentCursor curSelectionAfterReverse = curBefore;
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterLeft(true);
        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('>'), 2, 0));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('>'), 1, 0));

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curDtoC.anchor() == positionAt('D'));
        CHECK(curDtoC.position() == positionAt('C'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        CHECK(curSelectionAfter.anchor() == positionAt('<'));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('<'), 1, 0));

        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('<'), 1, 0));
        CHECK(curSelectionAfterReverse.position() == positionAt('<'));


        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == posB2);
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curEtoB.anchor() == posE2);
        CHECK(curEtoB.position() == posB2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curDtoC.anchor() == posD2);
        CHECK(curDtoC.position() == posC2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

    }

    SECTION("merge two lines") {
        lines = QStringList{
            "A            B",
            "C >",
            "< D",
            "E            F"
        };

        const int removedChars2 = lines[2].count("X");

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        const int deletionStartOff = cursor1.position().codeUnit;
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, -1));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -removedChars2 + deletionStartOff, -1));
        CHECK(curD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, -1));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, -1));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, -1));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curSelectionBefore = curBefore;
        curSelectionBefore.moveCharacterRight(true);
        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        Tui::ZDocumentCursor curSelectionBeforeReverse = curBefore;
        curSelectionBeforeReverse.moveCharacterRight();
        curSelectionBeforeReverse.moveCharacterLeft(true);
        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        Tui::ZDocumentCursor curSelectionAfter = curBefore;
        curSelectionAfter.moveCharacterRight();
        curSelectionAfter.moveCharacterRight(true);
        CHECK(curSelectionAfter.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('>'), 2, 0));

        Tui::ZDocumentCursor curSelectionAfterReverse = curBefore;
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterRight();
        curSelectionAfterReverse.moveCharacterLeft(true);
        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('>'), 2, 0));
        CHECK(curSelectionAfterReverse.position() == posAdd(positionAt('>'), 1, 0));


        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);

        CHECK(curSelectionBefore.anchor() == positionAt('>'));
        CHECK(curSelectionBefore.position() == posAdd(positionAt('>'), 1, 0));

        CHECK(curSelectionBeforeReverse.anchor() == posAdd(positionAt('>'), 1, 0));
        CHECK(curSelectionBeforeReverse.position() == positionAt('>'));

        CHECK(curSelectionAfter.anchor() == positionAt('<'));
        CHECK(curSelectionAfter.position() == posAdd(positionAt('<'), 1, 0));

        CHECK(curSelectionAfterReverse.anchor() == posAdd(positionAt('<'), 1, 0));
        CHECK(curSelectionAfterReverse.position() == positionAt('<'));

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == posB2);
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curEtoB.anchor() == posE2);
        CHECK(curEtoB.position() == posB2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
    }

    SECTION("Remove midline two lines") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX",
            "XXXXXX< D",
            "E            F"
        };

        const int removedChars2 = lines[2].count("X");

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -1, 0));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -2, 0));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        const int deletionStartOff = cursor1.position().codeUnit;
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, -1));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -removedChars2 + deletionStartOff, -1));
        CHECK(curD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, -1));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, -1));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, -1));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside2.hasSelection() == false);

        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside3.hasSelection() == false);

        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside4.hasSelection() == false);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == posB2);
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curEtoB.anchor() == posE2);
        CHECK(curEtoB.position() == posB2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);
    }

    SECTION("Remove midline two lines at end of document") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX",
            "XXXXXX< D"
        };

        const int removedChars2 = lines[2].count("X");

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoD = cursor1;
        curBtoD.setPosition(positionAt('B'));
        curBtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curDtoB = cursor1;
        curDtoB.setPosition(positionAt('D'));
        curDtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -1, 0));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -2, 0));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        const int deletionStartOff = cursor1.position().codeUnit;
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoD.anchor() == positionAt('B'));
        CHECK(curBtoD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -removedChars2 + deletionStartOff, -1));
        CHECK(curD.position() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curDtoB.anchor() == posAdd(positionAt('D'), -removedChars2 + deletionStartOff, -1));
        CHECK(curDtoB.position() == positionAt('B'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside2.hasSelection() == false);

        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside3.hasSelection() == false);

        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside4.hasSelection() == false);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoD.anchor() == positionAt('B'));
        CHECK(curBtoD.position() == positionAt('D'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curDtoB.anchor() == positionAt('D'));
        CHECK(curDtoB.position() == positionAt('B'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoD.anchor() == posB2);
        CHECK(curBtoD.position() == posD2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curDtoB.anchor() == posD2);
        CHECK(curDtoB.position() == posB2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);
    }

    SECTION("Remove midline three lines") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX",
            "XXXXXXXXXXXXX",
            "XXXXXX< D",
            "E            F"
        };

        const int removedChars3 = lines[3].count("X");

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curA = cursor1;
        curA.setPosition(positionAt('A'));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curC = cursor1;
        curC.setPosition(positionAt('C'));

        Tui::ZDocumentCursor curCtoD = cursor1;
        curCtoD.setPosition(positionAt('C'));
        curCtoD.setPosition(positionAt('D'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        Tui::ZDocumentCursor curF = cursor1;
        curF.setPosition(positionAt('F'));

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        Tui::ZDocumentCursor curInside2 = curInside;
        curInside2.moveCharacterRight();
        CHECK(curInside2.position() == posAdd(positionAt('>'), 2, 0));
        CHECK(curInside2.hasSelection() == false);

        Tui::ZDocumentCursor curInside3 = curAfter;
        curInside3.moveCharacterLeft();
        CHECK(curInside3.position() == posAdd(positionAt('<'), -1, 0));
        CHECK(curInside3.hasSelection() == false);

        Tui::ZDocumentCursor curInside4 = curInside3;
        curInside4.moveCharacterLeft();
        CHECK(curInside4.position() == posAdd(positionAt('<'), -2, 0));
        CHECK(curInside4.hasSelection() == false);

        Tui::ZDocumentCursor curInside1ToInside3 = curInside;
        curInside1ToInside3.setPosition(curInside3.position(), true);

        Tui::ZDocumentCursor curInside5 = curInside3;
        curInside5.setPosition({4, 2});

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        const int deletionStartOff = cursor1.position().codeUnit;
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const auto posA2 = curA.position();
        const auto posB2 = curB.position();
        const auto posC2 = curC.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();
        const auto posF2 = curF.position();

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, -2));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == posAdd(positionAt('D'), -removedChars3 + deletionStartOff, -2));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -removedChars3 + deletionStartOff, -2));
        CHECK(curD.position() == posAdd(positionAt('D'), -removedChars3 + deletionStartOff, -2));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, -2));
        CHECK(curEtoB.anchor() == posAdd(positionAt('E'), 0, -2));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == posAdd(positionAt('F'), 0, -2));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);

        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside2.hasSelection() == false);

        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside3.hasSelection() == false);

        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside4.hasSelection() == false);

        CHECK(curInside5.position() == posAfter2);
        CHECK(curInside5.hasSelection() == false);

        doc.undo(&cursor1);

        CHECK(curA.position() == positionAt('A'));
        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curC.position() == positionAt('C'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));
        CHECK(curEtoB.anchor() == positionAt('E'));
        CHECK(curEtoB.position() == positionAt('B'));
        CHECK(curF.position() == positionAt('F'));

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == positionAt('C'));
        CHECK(curCtoD.position() == positionAt('D'));
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside2.position() == positionAt('<'));
        CHECK(curInside3.position() == positionAt('<'));
        CHECK(curInside4.position() == positionAt('<'));
        CHECK(curInside1ToInside3.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);

        // inside the line that the redo will delete
        Tui::ZDocumentCursor curInside6 = curInside3;
        curInside6.setPosition({4, 2});

        doc.redo(&cursor1);

        CHECK(curA.position() == posA2);
        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == posB2);
        CHECK(curBtoE.position() == posE2);
        CHECK(curC.position() == posC2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);
        CHECK(curEtoB.anchor() == posE2);
        CHECK(curEtoB.position() == posB2);
        CHECK(curF.position() == posF2);

        CHECK(curA.hasSelection() == false);
        CHECK(curB.hasSelection() == false);
        CHECK(curC.hasSelection() == false);
        CHECK(curCtoD.anchor() == posC2);
        CHECK(curCtoD.position() == posD2);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);
        CHECK(curF.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside2.position() == posAfter2);
        CHECK(curInside3.position() == posAfter2);
        CHECK(curInside4.position() == posAfter2);
        CHECK(curInside5.position() == posAfter2);
        CHECK(curInside6.position() == posAfter2);
        CHECK(curInside1ToInside3.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
        CHECK(curInside2.hasSelection() == false);
        CHECK(curInside3.hasSelection() == false);
        CHECK(curInside4.hasSelection() == false);
        CHECK(curInside5.hasSelection() == false);
        CHECK(curInside6.hasSelection() == false);
        CHECK(curInside1ToInside3.hasSelection() == false);
    }

    SECTION("sort lines") {
        lines = QStringList{
            "dummy line 1",
            "dummy line 2",
            "ghello world",
            "asome test",
            "bcompletly different",
            "hthe little fox",
            "dummy line 3",
            "dummy line 4",
            "dummy line 5"
        };

        // sorted in lines 2-5 this is:
        // "dummy line 1",
        // "dummy line 2",
        // "asome test",
        // "bcompletly different",
        // "ghello world",
        // "hthe little fox",
        // "dummy line 3",
        // "dummy line 4",
        // "dummy line 5"

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor cur0 = cursor1;
        cur0.setPosition({5, 0});
        Tui::ZDocumentCursor cur1 = cursor1;
        cur1.setPosition({10, 1});
        Tui::ZDocumentCursor cur2 = cursor1;
        cur2.setPosition({9, 2});
        Tui::ZDocumentCursor cur3 = cursor1;
        cur3.setPosition({3, 3});
        Tui::ZDocumentCursor cur4 = cursor1;
        cur4.setPosition({7, 4});
        Tui::ZDocumentCursor cur5 = cursor1;
        cur5.setPosition({8, 5});
        Tui::ZDocumentCursor cur6 = cursor1;
        cur6.setPosition({4, 6});
        Tui::ZDocumentCursor cur7 = cursor1;
        cur7.setPosition({2, 7});
        Tui::ZDocumentCursor cur8 = cursor1;
        cur8.setPosition({1, 8});

        doc.sortLines(2, 5, &cursor1);

        const auto pos0After = cur0.position();
        const auto pos1After = cur1.position();
        const auto pos2After = cur2.position();
        const auto pos3After = cur3.position();
        const auto pos4After = cur4.position();
        const auto pos5After = cur5.position();
        const auto pos6After = cur6.position();
        const auto pos7After = cur7.position();
        const auto pos8After = cur8.position();

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{5, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{10, 1});

        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 4});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 2});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 3});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 5});

        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 6});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});

        doc.undo(&cursor1);

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{5, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{10, 1});
        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 2});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 3});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 4});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 5});
        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 6});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});

        doc.redo(&cursor1);

        CHECK(cur0.position() == pos0After);
        CHECK(cur1.position() == pos1After);
        CHECK(cur2.position() == pos2After);
        CHECK(cur3.position() == pos3After);
        CHECK(cur4.position() == pos4After);
        CHECK(cur5.position() == pos5After);
        CHECK(cur6.position() == pos6After);
        CHECK(cur7.position() == pos7After);
        CHECK(cur8.position() == pos8After);
    }

    SECTION("collapsed inserts") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor curB = cursor1;
        curB.setPosition(positionAt('B'));

        Tui::ZDocumentCursor curBtoE = cursor1;
        curBtoE.setPosition(positionAt('B'));
        curBtoE.setPosition(positionAt('E'), true);

        Tui::ZDocumentCursor curEtoB = cursor1;
        curEtoB.setPosition(positionAt('E'));
        curEtoB.setPosition(positionAt('B'), true);

        Tui::ZDocumentCursor curBefore = cursor1;
        curBefore.setPosition(positionAt('>'));

        Tui::ZDocumentCursor curAfter = cursor1;
        curAfter.setPosition(positionAt('<'));

        Tui::ZDocumentCursor curD = cursor1;
        curD.setPosition(positionAt('D'));

        Tui::ZDocumentCursor curE = cursor1;
        curE.setPosition(positionAt('E'));

        cursor1.setPosition(positionAt('<'));
        const int insertionOff = cursor1.position().codeUnit;
        cursor1.insertText("\n");
        cursor1.insertText("a");
        cursor1.insertText("b");
        cursor1.insertText("c");
        const int insertedLen2 = 3;

        const auto posB2 = curB.position();
        const auto posBefore2 = curBefore.position();
        const auto posAfter2 = curAfter.position();
        const auto posD2 = curD.position();
        const auto posE2 = curE.position();

        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posAdd(positionAt('E'), 0, 1));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == posAdd(positionAt('<'), -insertionOff + insertedLen2, 1));
        CHECK(curD.position() == posAdd(positionAt('D'), -insertionOff + insertedLen2, 1));
        CHECK(curE.position() == posAdd(positionAt('E'), 0, 1));

        CHECK(curB.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);

        Tui::ZDocumentCursor curInside = curBefore;
        curInside.moveCharacterRight();
        CHECK(curInside.position() == posAdd(positionAt('>'), 1, 0));
        CHECK(curInside.hasSelection() == false);

        doc.undo(&cursor1);

        CHECK(curB.position() == positionAt('B'));
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == positionAt('E'));
        CHECK(curBefore.position() == positionAt('>'));
        CHECK(curAfter.position() == positionAt('<'));
        CHECK(curD.position() == positionAt('D'));
        CHECK(curE.position() == positionAt('E'));

        CHECK(curB.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);

        CHECK(curInside.position() == positionAt('<'));
        CHECK(curInside.hasSelection() == false);

        doc.redo(&cursor1);

        CHECK(curB.position() == posB2);
        CHECK(curBtoE.anchor() == positionAt('B'));
        CHECK(curBtoE.position() == posE2);
        CHECK(curBefore.position() == posBefore2);
        CHECK(curAfter.position() == posAfter2);
        CHECK(curD.position() == posD2);
        CHECK(curE.position() == posE2);

        CHECK(curB.hasSelection() == false);
        CHECK(curBefore.hasSelection() == false);
        CHECK(curAfter.hasSelection() == false);
        CHECK(curD.hasSelection() == false);
        CHECK(curE.hasSelection() == false);

        CHECK(curInside.position() == posAfter2);
        CHECK(curInside.hasSelection() == false);
    }

    SECTION("move line down") {
        lines = QStringList{
            "line 1aaaaaa",
            "line 2b",
            "line 3ccc",
            "line 4dddddd",
            "line 5eeeee",
            "line 6ffffffff",
            "line 7gggg",
            "line 8",
            "line 9jjjjjjj"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor cur0 = cursor1;
        cur0.setPosition({9, 0});
        Tui::ZDocumentCursor cur1 = cursor1;
        cur1.setPosition({0, 1});
        Tui::ZDocumentCursor cur2 = cursor1;
        cur2.setPosition({9, 2});
        Tui::ZDocumentCursor cur3 = cursor1;
        cur3.setPosition({3, 3});
        Tui::ZDocumentCursor cur4 = cursor1;
        cur4.setPosition({7, 4});
        Tui::ZDocumentCursor cur5 = cursor1;
        cur5.setPosition({8, 5});
        Tui::ZDocumentCursor cur6 = cursor1;
        cur6.setPosition({4, 6});
        Tui::ZDocumentCursor cur7 = cursor1;
        cur7.setPosition({2, 7});
        Tui::ZDocumentCursor cur8 = cursor1;
        cur8.setPosition({1, 8});

        doc.moveLine(2, 4, &cursor1);

        const auto pos0After = cur0.position();
        const auto pos1After = cur1.position();
        const auto pos2After = cur2.position();
        const auto pos3After = cur3.position();
        const auto pos4After = cur4.position();
        const auto pos5After = cur5.position();
        const auto pos6After = cur6.position();
        const auto pos7After = cur7.position();
        const auto pos8After = cur8.position();

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{9, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 4});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 2});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 3});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 5});
        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 6});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});

        doc.undo(&cursor1);

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{9, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 2});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 3});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 4});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 5});
        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 6});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});

        doc.redo(&cursor1);

        CHECK(cur0.position() == pos0After);
        CHECK(cur1.position() == pos1After);
        CHECK(cur2.position() == pos2After);
        CHECK(cur3.position() == pos3After);
        CHECK(cur4.position() == pos4After);
        CHECK(cur5.position() == pos5After);
        CHECK(cur6.position() == pos6After);
        CHECK(cur7.position() == pos7After);
        CHECK(cur8.position() == pos8After);
    }


    SECTION("move line up") {
        lines = QStringList{
            "line 1aaaaaa",
            "line 2b",
            "line 3ccc",
            "line 4dddddd",
            "line 5eeeee",
            "line 6ffffffff",
            "line 7gggg",
            "line 8",
            "line 9jjjjjjj"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentCursor cur0 = cursor1;
        cur0.setPosition({9, 0});
        Tui::ZDocumentCursor cur1 = cursor1;
        cur1.setPosition({0, 1});
        Tui::ZDocumentCursor cur2 = cursor1;
        cur2.setPosition({9, 2});
        Tui::ZDocumentCursor cur3 = cursor1;
        cur3.setPosition({3, 3});
        Tui::ZDocumentCursor cur4 = cursor1;
        cur4.setPosition({7, 4});
        Tui::ZDocumentCursor cur5 = cursor1;
        cur5.setPosition({8, 5});
        Tui::ZDocumentCursor cur6 = cursor1;
        cur6.setPosition({4, 6});
        Tui::ZDocumentCursor cur7 = cursor1;
        cur7.setPosition({2, 7});
        Tui::ZDocumentCursor cur8 = cursor1;
        cur8.setPosition({1, 8});

        doc.moveLine(7, 4, &cursor1);

        const auto pos0After = cur0.position();
        const auto pos1After = cur1.position();
        const auto pos2After = cur2.position();
        const auto pos3After = cur3.position();
        const auto pos4After = cur4.position();
        const auto pos5After = cur5.position();
        const auto pos6After = cur6.position();
        const auto pos7After = cur7.position();
        const auto pos8After = cur8.position();

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{9, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 2});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 3});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 4});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 5});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 6});
        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});
        doc.undo(&cursor1);

        CHECK(cur0.position() == Tui::ZDocumentCursor::Position{9, 0});
        CHECK(cur1.position() == Tui::ZDocumentCursor::Position{0, 1});
        CHECK(cur2.position() == Tui::ZDocumentCursor::Position{9, 2});
        CHECK(cur3.position() == Tui::ZDocumentCursor::Position{3, 3});
        CHECK(cur4.position() == Tui::ZDocumentCursor::Position{7, 4});
        CHECK(cur5.position() == Tui::ZDocumentCursor::Position{8, 5});
        CHECK(cur6.position() == Tui::ZDocumentCursor::Position{4, 6});
        CHECK(cur7.position() == Tui::ZDocumentCursor::Position{2, 7});
        CHECK(cur8.position() == Tui::ZDocumentCursor::Position{1, 8});

        doc.redo(&cursor1);

        CHECK(cur0.position() == pos0After);
        CHECK(cur1.position() == pos1After);
        CHECK(cur2.position() == pos2After);
        CHECK(cur3.position() == pos3After);
        CHECK(cur4.position() == pos4After);
        CHECK(cur5.position() == pos5After);
        CHECK(cur6.position() == pos6After);
        CHECK(cur7.position() == pos7After);
        CHECK(cur8.position() == pos8After);
    }
}

TEST_CASE("Document line marker adjustments") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    // Check if modifiation, undo and redo correctly moves line markers on the document.

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    QStringList lines;

    auto positionAt = [&] (QChar ch) -> Tui::ZDocumentCursor::Position {
        int lineNo = 0;
        for (const QString &line: lines) {
            if (line.contains(ch)) {
                return {line.indexOf(ch), lineNo};
            }
            lineNo += 1;
        }
        // should never be reached
        REQUIRE(false);
        return {0, 0};
    };

    SECTION("Insert midline") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker marker0{&doc, 0};
        Tui::ZDocumentLineMarker marker1{&doc, 1};
        Tui::ZDocumentLineMarker marker2{&doc, 2};

        cursor1.setPosition(positionAt('<'));
        const QString insertedText = "XXXXXXXXXXX";
        cursor1.insertText(insertedText);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);

        doc.undo(&cursor1);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);

        doc.redo(&cursor1);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);
    }

    SECTION("Insert midline two lines") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker markerA{&doc, 0};
        Tui::ZDocumentLineMarker markerC{&doc, 1};
        Tui::ZDocumentLineMarker markerE{&doc, 2};

        cursor1.setPosition(positionAt('<'));
        const QString insertedTextLine1 = "XXX";
        const QString insertedTextLine2 = "XXXXXXXX";
        const QString insertedText = insertedTextLine1 + "\n" + insertedTextLine2;
        cursor1.insertText(insertedText);

        const int posA2 = markerA.line();
        const int posC2 = markerC.line();
        const int posE2 = markerE.line();

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerE.line() == 3);

        Tui::ZDocumentLineMarker markerD{&doc, 2};

        doc.undo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 2);


        doc.redo(&cursor1);

        CHECK(markerA.line() == posA2);
        CHECK(markerC.line() == posC2);
        CHECK(markerD.line() == posC2);
        CHECK(markerE.line() == posE2);
    }

    SECTION("Insert midline three lines") {
        lines = QStringList{
            "A            B",
            "C >< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker markerA{&doc, 0};
        Tui::ZDocumentLineMarker markerC{&doc, 1};
        Tui::ZDocumentLineMarker markerE{&doc, 2};

        cursor1.setPosition(positionAt('<'));
        const QString insertedTextLine1 = "XXX";
        const QString insertedTextLine2 = "XXXXXX";
        const QString insertedTextLine3 = "XXXXXXXX";
        const QString insertedText = insertedTextLine1 + "\n" + insertedTextLine2 + "\n" + insertedTextLine3;
        cursor1.insertText(insertedText);

        const int posA2 = markerA.line();
        const int posC2 = markerC.line();
        const int posE2 = markerE.line();

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerE.line() == 4);

        Tui::ZDocumentLineMarker markerX{&doc, 2};
        Tui::ZDocumentLineMarker markerD{&doc, 3};

        doc.undo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerX.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 2);

        doc.redo(&cursor1);

        CHECK(markerA.line() == posA2);
        CHECK(markerC.line() == posC2);
        CHECK(markerX.line() == posC2);
        CHECK(markerD.line() == posC2);
        CHECK(markerE.line() == posE2);

    }

    SECTION("Remove midline") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker markerA{&doc, 0};
        Tui::ZDocumentLineMarker markerC{&doc, 1};
        Tui::ZDocumentLineMarker markerE{&doc, 2};

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();


        cursor1.setPosition(positionAt('<'));
        const QString insertedText = "XXXXXXXXXXX";
        cursor1.insertText(insertedText);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerE.line() == 2);

        doc.undo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerE.line() == 2);

        doc.redo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerE.line() == 2);


    }

    SECTION("Remove midline two lines") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX",
            "XXXXXX< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker markerA{&doc, 0};
        Tui::ZDocumentLineMarker markerC{&doc, 1};
        Tui::ZDocumentLineMarker markerD{&doc, 2};
        Tui::ZDocumentLineMarker markerE{&doc, 3};

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const int posA2 = markerA.line();
        const int posC2 = markerC.line();
        const int posE2 = markerE.line();

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 2);

        doc.undo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 3);

        doc.redo(&cursor1);

        CHECK(markerA.line() == posA2);
        CHECK(markerC.line() == posC2);
        CHECK(markerD.line() == posC2);
        CHECK(markerE.line() == posE2);
    }

    SECTION("Remove midline three lines") {
        lines = QStringList{
            "A            B",
            "C >XXXXXXXXXXXXXXXX",
            "XXXXXXXXXXXXX",
            "XXXXXX< D",
            "E            F"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker markerA{&doc, 0};
        Tui::ZDocumentLineMarker markerC{&doc, 1};
        Tui::ZDocumentLineMarker markerX{&doc, 2};
        Tui::ZDocumentLineMarker markerD{&doc, 3};
        Tui::ZDocumentLineMarker markerE{&doc, 4};

        cursor1.setPosition(positionAt('>'));
        cursor1.moveCharacterRight();
        cursor1.setPosition(positionAt('<'), true);
        cursor1.removeSelectedText();

        const int posA2 = markerA.line();
        const int posC2 = markerC.line();
        const int posE2 = markerE.line();

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerX.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 2);

        doc.undo(&cursor1);

        CHECK(markerA.line() == 0);
        CHECK(markerC.line() == 1);
        CHECK(markerX.line() == 1);
        CHECK(markerD.line() == 1);
        CHECK(markerE.line() == 4);

        Tui::ZDocumentLineMarker markerX3{&doc, 3};

        doc.redo(&cursor1);

        CHECK(markerA.line() == posA2);
        CHECK(markerC.line() == posC2);
        CHECK(markerX.line() == posC2);
        CHECK(markerX3.line() == posC2);
        CHECK(markerD.line() == posC2);
        CHECK(markerE.line() == posE2);
    }

    SECTION("sort lines") {
        lines = QStringList{
            "dummy line 1",
            "dummy line 2",
            "ghello world",
            "asome test",
            "bcompletly different",
            "hthe little fox",
            "dummy line 3",
            "dummy line 4",
            "dummy line 5"
        };

        // sorted in lines 2-5 this is:
        // "dummy line 1",
        // "dummy line 2",
        // "asome test",
        // "bcompletly different",
        // "ghello world",
        // "hthe little fox",
        // "dummy line 3",
        // "dummy line 4",
        // "dummy line 5"

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker marker0{&doc, 0};
        Tui::ZDocumentLineMarker marker1 = marker0;
        marker1.setLine(1);
        Tui::ZDocumentLineMarker marker2 = marker0;
        marker2.setLine(2);
        Tui::ZDocumentLineMarker marker3 = marker0;
        marker3.setLine(3);
        Tui::ZDocumentLineMarker marker4 = marker0;
        marker4.setLine(4);
        Tui::ZDocumentLineMarker marker5 = marker0;
        marker5.setLine(5);
        Tui::ZDocumentLineMarker marker6 = marker0;
        marker6.setLine(6);
        Tui::ZDocumentLineMarker marker7 = marker0;
        marker7.setLine(7);
        Tui::ZDocumentLineMarker marker8 = marker0;
        marker8.setLine(8);

        doc.sortLines(2, 5, &cursor1);

        const int pos0After = marker0.line();
        const int pos1After = marker1.line();
        const int pos2After = marker2.line();
        const int pos3After = marker3.line();
        const int pos4After = marker4.line();
        const int pos5After = marker5.line();
        const int pos6After = marker6.line();
        const int pos7After = marker7.line();
        const int pos8After = marker8.line();

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);

        CHECK(marker2.line() == 4);
        CHECK(marker3.line() == 2);
        CHECK(marker4.line() == 3);
        CHECK(marker5.line() == 5);

        CHECK(marker6.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);

        doc.undo(&cursor1);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);
        CHECK(marker3.line() == 3);
        CHECK(marker4.line() == 4);
        CHECK(marker5.line() == 5);
        CHECK(marker6.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);

        doc.redo(&cursor1);

        CHECK(marker0.line() == pos0After);
        CHECK(marker1.line() == pos1After);
        CHECK(marker2.line() == pos2After);
        CHECK(marker3.line() == pos3After);
        CHECK(marker4.line() == pos4After);
        CHECK(marker5.line() == pos5After);
        CHECK(marker6.line() == pos6After);
        CHECK(marker7.line() == pos7After);
        CHECK(marker8.line() == pos8After);
    }

    SECTION("move line down") {
        lines = QStringList{
            "line 1aaaaaa",
            "line 2b",
            "line 3ccc",
            "line 4dddddd",
            "line 5eeeee",
            "line 6ffffffff",
            "line 7gggg",
            "line 8",
            "line 9jjjjjjj"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker marker0{&doc, 0};
        Tui::ZDocumentLineMarker marker1 = marker0;
        marker1.setLine(1);
        Tui::ZDocumentLineMarker marker2 = marker0;
        marker2.setLine(2);
        Tui::ZDocumentLineMarker marker3 = marker0;
        marker3.setLine(3);
        Tui::ZDocumentLineMarker marker4 = marker0;
        marker4.setLine(4);
        Tui::ZDocumentLineMarker marker5 = marker0;
        marker5.setLine(5);
        Tui::ZDocumentLineMarker marker6 = marker0;
        marker6.setLine(6);
        Tui::ZDocumentLineMarker marker7 = marker0;
        marker7.setLine(7);
        Tui::ZDocumentLineMarker marker8 = marker0;
        marker8.setLine(8);

        doc.moveLine(2, 4, &cursor1);

        const int pos0After = marker0.line();
        const int pos1After = marker1.line();
        const int pos2After = marker2.line();
        const int pos3After = marker3.line();
        const int pos4After = marker4.line();
        const int pos5After = marker5.line();
        const int pos6After = marker6.line();
        const int pos7After = marker7.line();
        const int pos8After = marker8.line();

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 4);
        CHECK(marker3.line() == 2);
        CHECK(marker4.line() == 3);
        CHECK(marker5.line() == 5);
        CHECK(marker6.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);

        doc.undo(&cursor1);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);
        CHECK(marker3.line() == 3);
        CHECK(marker4.line() == 4);
        CHECK(marker5.line() == 5);
        CHECK(marker6.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);

        doc.redo(&cursor1);

        CHECK(marker0.line() == pos0After);
        CHECK(marker1.line() == pos1After);
        CHECK(marker2.line() == pos2After);
        CHECK(marker3.line() == pos3After);
        CHECK(marker4.line() == pos4After);
        CHECK(marker5.line() == pos5After);
        CHECK(marker6.line() == pos6After);
        CHECK(marker7.line() == pos7After);
        CHECK(marker8.line() == pos8After);

    }

    SECTION("move line up") {
        lines = QStringList{
            "line 1aaaaaa",
            "line 2b",
            "line 3ccc",
            "line 4dddddd",
            "line 5eeeee",
            "line 6ffffffff",
            "line 7gggg",
            "line 8",
            "line 9jjjjjjj"
        };

        cursor1.insertText(lines.join("\n"));

        Tui::ZDocumentLineMarker marker0{&doc, 0};
        Tui::ZDocumentLineMarker marker1 = marker0;
        marker1.setLine(1);
        Tui::ZDocumentLineMarker marker2 = marker0;
        marker2.setLine(2);
        Tui::ZDocumentLineMarker marker3 = marker0;
        marker3.setLine(3);
        Tui::ZDocumentLineMarker marker4 = marker0;
        marker4.setLine(4);
        Tui::ZDocumentLineMarker marker5 = marker0;
        marker5.setLine(5);
        Tui::ZDocumentLineMarker marker6 = marker0;
        marker6.setLine(6);
        Tui::ZDocumentLineMarker marker7 = marker0;
        marker7.setLine(7);
        Tui::ZDocumentLineMarker marker8 = marker0;
        marker8.setLine(8);

        doc.moveLine(4, 6, &cursor1);

        const int pos0After = marker0.line();
        const int pos1After = marker1.line();
        const int pos2After = marker2.line();
        const int pos3After = marker3.line();
        const int pos4After = marker4.line();
        const int pos5After = marker5.line();
        const int pos6After = marker6.line();
        const int pos7After = marker7.line();
        const int pos8After = marker8.line();

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);
        CHECK(marker3.line() == 3);
        CHECK(marker5.line() == 4);
        CHECK(marker6.line() == 5);
        CHECK(marker4.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);
        doc.undo(&cursor1);

        CHECK(marker0.line() == 0);
        CHECK(marker1.line() == 1);
        CHECK(marker2.line() == 2);
        CHECK(marker3.line() == 3);
        CHECK(marker4.line() == 4);
        CHECK(marker5.line() == 5);
        CHECK(marker6.line() == 6);
        CHECK(marker7.line() == 7);
        CHECK(marker8.line() == 8);

        doc.redo(&cursor1);

        CHECK(marker0.line() == pos0After);
        CHECK(marker1.line() == pos1After);
        CHECK(marker2.line() == pos2After);
        CHECK(marker3.line() == pos3After);
        CHECK(marker4.line() == pos4After);
        CHECK(marker5.line() == pos5After);
        CHECK(marker6.line() == pos6After);
        CHECK(marker7.line() == pos7After);
        CHECK(marker8.line() == pos8After);
    }
}

