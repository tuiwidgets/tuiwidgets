// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZDocument.h>
#include <Tui/ZDocumentCursor.h>

#include <QElapsedTimer>
#include <QThreadPool>

#include <Tui/ZTerminal.h>
#include <Tui/ZTextMetrics.h>

#include "../catchwrapper.h"
#include "../eventrecorder.h"
#include "../Testhelper.h"

#include <Tui/Utils_p.h>


namespace {

    struct SearchTestCase {
        QString documentContents;
        bool hasMatch;
        Tui::ZDocumentCursor::Position start;
        Tui::ZDocumentCursor::Position end;
        Tui::ZDocumentCursor::Position foundStart;
        Tui::ZDocumentCursor::Position foundEnd;
        QString marker;
    };

    struct MatchPosition {
        Tui::ZDocumentCursor::Position foundStart{0, 0};
        Tui::ZDocumentCursor::Position foundEnd{0, 0};
        QString marker;
    };

}


// Generate a document contents and a list of expected matches from a specially formatted string.
// Lines that don't contain a '|' or '>' character are ignored.
// Lines should be visibly aligned on their '|' and '>' characters.
// Lines with a '|' character denote a line in the document to be used in the search test.
//     Although is it not checked the text before the '|' should be the line index (starting with 0).
//     Everything after the '|' is the line contents. Don't end lines with spaces or tabs are IDEs are commonly
//     configured to remove them at the end of the line.
// Line with a '>' character denote a search pattern matches in the preceeding '|' line.
//     Each search result uses a unique (for the whole input) character.
//     Single-character matches are marked with a character below the match in the '|' line.
//     Multi-character matches are marked with a run of characters covering the whole match blow the match in the '|' line.
//     Matches may span multiple document lines. If a match only consists of the line break in the document align the
//     match character one position right of the line end in the '|' line.
//     If overlapping matches exist multiple '>' lines per '|' line can be use.
static auto parseSearchInfo(const QString &input) {
    QStringList lines;

    int line = -1;
    //int lineCodeUnits = -1;

    QMap<QChar, MatchPosition> matchesMap;

    auto extendOrCreateMatch = [&] (int codeUnit, QChar id) {
        if (!matchesMap.contains(id)) {
            if (codeUnit == lines.back().size()) {
                matchesMap[id] = MatchPosition{{codeUnit, line}, {0, line + 1}, id};
            } else {
                matchesMap[id] = MatchPosition{{codeUnit, line}, {codeUnit + 1, line}, id};
            }
        } else {
            if (matchesMap[id].foundEnd.line == line) {
                REQUIRE(matchesMap[id].foundEnd.codeUnit == codeUnit);
                if (codeUnit == lines.back().size()) {
                    matchesMap[id].foundEnd.line = line + 1;
                    matchesMap[id].foundEnd.codeUnit = 0;
                } else {
                    matchesMap[id].foundEnd.codeUnit = codeUnit + 1;
                }
            } else {
                REQUIRE(matchesMap[id].foundEnd.line + 1 == line);
                REQUIRE(codeUnit == 0);
                matchesMap[id].foundEnd.codeUnit = 1;
                matchesMap[id].foundEnd.line = line;
            }
        }
    };

    for (QString inputLine: input.split("\n")) {
        if (inputLine.contains('|')) {
            line += 1;
            //lineCodeUnits = inputLine.section('|', 1).size();
            lines.append(inputLine.section('|', 1));
        } else if (inputLine.contains('>')) {
            QString part = inputLine.section('>', 1);
            for (int i = 0; i < part.size(); i++) {
                if (part[i] != ' ') {
                    extendOrCreateMatch(i, part[i]);
                }
            }
        }
    }

    QString documentContents = lines.join("\n");

    return std::make_tuple(matchesMap, documentContents, lines);
}

// See parseSearchInfo for description of input format
static std::vector<SearchTestCase> generateTestCases(const QString &input) {

    auto [matchesMap, documentContents, lines] = parseSearchInfo(input);

    if (matchesMap.isEmpty()) {
        return { {documentContents, false, {0, 0}, {size2int(lines.last().size()), size2int(lines.size()) - 1}, {0, 0}, {0, 0}, {}} };
    }

    QList<MatchPosition> matches = matchesMap.values();
    std::sort(matches.begin(), matches.end(), [](auto &a, auto &b) {
        return a.foundStart < b.foundStart;
    });

    std::vector<SearchTestCase> ret;

    ret.push_back(SearchTestCase{documentContents, true, {0, 0}, matches[0].foundStart, matches[0].foundStart, matches[0].foundEnd, matches[0].marker});
    Tui::ZDocumentCursor::Position nextStart = matches[0].foundStart;

    auto advanceNextStart = [&, lines=lines] {
        if (nextStart.codeUnit == lines[nextStart.line].size()) {
            nextStart.codeUnit = 0;
            nextStart.line += 1;
        } else {
            nextStart.codeUnit += 1;
        }
    };

    advanceNextStart();

    for (int i = 1; i < matches.size(); i++) {
        ret.push_back(SearchTestCase{documentContents, true, nextStart, matches[i].foundStart, matches[i].foundStart, matches[i].foundEnd, matches[i].marker});
        nextStart = matches[i].foundStart;
        advanceNextStart();
    }
    ret.push_back(SearchTestCase{documentContents, true, nextStart, {size2int(lines.last().size()), size2int(lines.size()) - 1}, matches[0].foundStart, matches[0].foundEnd, matches[0].marker});

    return ret;
}

// See parseSearchInfo for description of input format
static std::vector<SearchTestCase> generateTestCasesBackward(const QString &input) {

    auto [matchesMap, documentContents, lines] = parseSearchInfo(input);

    if (matchesMap.isEmpty()) {
        return { {documentContents, false, {0, 0}, {size2int(lines.last().size()), size2int(lines.size()) - 1}, {0, 0}, {0, 0}, {}} };
    }

    QList<MatchPosition> matches = matchesMap.values();
    std::sort(matches.begin(), matches.end(), [](auto &a, auto &b) {
        return a.foundStart < b.foundStart;
    });

    std::vector<SearchTestCase> ret;

    ret.push_back(SearchTestCase{documentContents, true, matches.last().foundEnd, {size2int(lines.last().size()), size2int(lines.size()) - 1},
                                 matches.last().foundStart, matches.last().foundEnd, matches.last().marker});

    Tui::ZDocumentCursor::Position nextEnd = matches.last().foundEnd;

    auto moveBackNextEnd = [&, lines=lines] {
        if (nextEnd.codeUnit == 0) {
            nextEnd.line -= 1;
            nextEnd.codeUnit = lines[nextEnd.line].size();
        } else {
            nextEnd.codeUnit--;
        }
    };

    moveBackNextEnd();

    for (int i = matches.size() - 2; i >= 0; i--) {
        ret.push_back(SearchTestCase{documentContents, true, matches[i].foundEnd, nextEnd, matches[i].foundStart, matches[i].foundEnd, matches[i].marker});
        nextEnd = matches[i].foundEnd;
        moveBackNextEnd();
    }

    ret.push_back(SearchTestCase{documentContents, true, {0, 0}, nextEnd, matches.last().foundStart, matches.last().foundEnd, matches.last().marker});

    return ret;
}


TEST_CASE("Search") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    auto runChecks = [&](const SearchTestCase &testCase, const QString &needle, Qt::CaseSensitivity caseMatching) {
        cursor1.insertText(testCase.documentContents);

        const bool wrapAround = GENERATE(false, true);

        CAPTURE(testCase.start);
        CAPTURE(testCase.end);
        CAPTURE(testCase.foundStart);
        CAPTURE(testCase.foundEnd);
        CAPTURE(wrapAround);
        Tui::ZDocument::FindFlags options = wrapAround ? Tui::ZDocument::FindFlag::FindWrap : Tui::ZDocument::FindFlags{};
        if (caseMatching == Qt::CaseSensitive) {
            options |= Tui::ZDocument::FindFlag::FindCaseSensitively;
        }

        cursor1.setPosition(testCase.start, true);
        cursor1.setAnchorPosition({0, 0});

        if (testCase.hasMatch) {
            auto result = doc.findSync(needle, cursor1, options);
            if (!wrapAround && testCase.start > testCase.foundStart) {
                CAPTURE(result.anchor());
                CAPTURE(result.position());
                CHECK(!result.hasSelection());
            } else {
                CHECK(result.hasSelection());
                CHECK(result.anchor() == testCase.foundStart);
                CHECK(result.position() == testCase.foundEnd);
            }

            while (cursor1.position() < testCase.end) {
                CAPTURE(cursor1.position());
                REQUIRE(!cursor1.atEnd());
                cursor1.moveCharacterRight();
                // check if we overstepped
                if (cursor1.position() > testCase.end) break;

                cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                if (!wrapAround && testCase.start > testCase.foundStart) {
                    CAPTURE(result.anchor());
                    CAPTURE(result.position());
                    CHECK(!result.hasSelection());
                } else {
                    CHECK(result.hasSelection());
                    CHECK(result.anchor() == testCase.foundStart);
                    CHECK(result.position() == testCase.foundEnd);
                }
            }
        } else {
            auto result = doc.findSync(needle, cursor1, options);
            CHECK(!result.hasSelection());

            while (!cursor1.atEnd()) {
                CAPTURE(cursor1.position());
                cursor1.moveCharacterRight();

                cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                CHECK(!result.hasSelection());
            }
        }
    };

    SECTION("no char") {
        cursor1.insertText("");
        cursor1 = doc.findSync(" ", cursor1, Tui::ZDocument::FindFlag::FindWrap);
        CHECK(cursor1.hasSelection() == false);
        CHECK(cursor1.selectionStartPos().codeUnit == 0);
        CHECK(cursor1.selectionEndPos().codeUnit == 0);
        CHECK(cursor1.selectionStartPos().line == 0);
        CHECK(cursor1.selectionEndPos().line == 0);
    }

    SECTION("empty search string") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "", Qt::CaseSensitive);
    }

    SECTION("one char") {
        cursor1.insertText("m");
        cursor1 = doc.findSync("m", cursor1, Tui::ZDocument::FindFlag::FindWrap);
        CHECK(cursor1.hasSelection() == true);
        CHECK(cursor1.selectionStartPos().codeUnit == 0);
        CHECK(cursor1.selectionEndPos().codeUnit == 1);
        CHECK(cursor1.selectionStartPos().line == 0);
        CHECK(cursor1.selectionEndPos().line == 0);
    }

    SECTION("one char t") {
        static auto testCases = generateTestCases(R"(
                                                  0|test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t", Qt::CaseSensitive);
    }

    SECTION("one char t - mismatched case") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t", Qt::CaseSensitive);
    }

    SECTION("one char t - case insensitive") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t", Qt::CaseInsensitive);
    }

    SECTION("one char repeated") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                   >12
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t", Qt::CaseSensitive);
    }

    SECTION("two char") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                   >11
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "tt", Qt::CaseSensitive);
    }

    SECTION("two char, two lines") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                   >11
                                                  1|tt
                                                   >22
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "tt", Qt::CaseSensitive);
    }

    SECTION("two char multiline") {
        static auto testCases = generateTestCases(R"(
                                                  0|at
                                                   > 1
                                                  1|ba
                                                   >1
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t\nb", Qt::CaseSensitive);
    }

    SECTION("multiline case mismatch first line") {
        static auto testCases = generateTestCases(R"(
                                                  0|heLlo
                                                  1|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nworld", Qt::CaseSensitive);
    }

    SECTION("multiline case mismatch second line") {
        static auto testCases = generateTestCases(R"(
                                                  0|hello
                                                  1|woRld
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nworld", Qt::CaseSensitive);
    }

    SECTION("multiline case insensitive") {
        static auto testCases = generateTestCases(R"(
                                                  0|helLo
                                                   >11111
                                                  1|woRld
                                                   >11111
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nworld", Qt::CaseInsensitive);
    }

    SECTION("two char multiline2") {
        static auto testCases = generateTestCases(R"(
                                                  0|at
                                                   > 1
                                                  1|t
                                                   >1
                                                   >2
                                                  2|ta
                                                   >2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t\nt", Qt::CaseSensitive);
    }

    SECTION("three multiline") {
        static auto testCases = generateTestCases(R"(
                                                  0|bat
                                                   >  1
                                                  1|zy
                                                   >11
                                                  2|ga
                                                   >1
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "t\nzy\ng", Qt::CaseSensitive);
    }

    SECTION("three multiline case mismatch first line") {
        static auto testCases = generateTestCases(R"(
                                                  0|hEllo
                                                  1|whole
                                                  2|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("three multiline case mismatch middle line") {
        static auto testCases = generateTestCases(R"(
                                                  0|hello
                                                  1|whOle
                                                  2|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("three multiline case mismatch last line") {
        static auto testCases = generateTestCases(R"(
                                                  0|hello
                                                  1|whole
                                                  2|worlD
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("three multiline case insensitive") {
        static auto testCases = generateTestCases(R"(
                                                  0|heLlo
                                                   >11111
                                                  1|wHole
                                                   >11111
                                                  2|worlD
                                                   >11111
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "hello\nwhole\nworld", Qt::CaseInsensitive);
    }

    SECTION("four multiline") {
        static auto testCases = generateTestCases(R"(
                                                  0|bae
                                                   >  1
                                                  1|rt
                                                   >11
                                                  2|zu
                                                   >11
                                                  3|ia
                                                   >1
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "e\nrt\nzu\ni", Qt::CaseSensitive);
    }

    SECTION("four multiline double") {
        static auto testCases = generateTestCases(R"(
                                                  0|ab
                                                   >11
                                                  1|ab
                                                   >11
                                                  2|ab
                                                   >11
                                                  3| ab
                                                   > 22
                                                  4|ab
                                                   >22
                                                  5|ab
                                                   >22
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "ab\nab\nab", Qt::CaseSensitive);
    }

    SECTION("first not match") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                  1|aa
                                                  2|tt
                                                   >11
                                                  3|tt
                                                   >11
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "tt\ntt", Qt::CaseSensitive);
    }

    SECTION("first not match of three") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                  1|aa
                                                  2|tt
                                                  3|tt
                                                   >11
                                                  4|bb
                                                   >11
                                                  5|tt
                                                   >11
                                                  6|tt
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "tt\nbb\ntt", Qt::CaseSensitive);
    }

    SECTION("line break") {
        static auto testCases = generateTestCases(R"(
                                                  0|tt
                                                   >  1
                                                  1|aa
                                                   >  2
                                                  2|ttt
                                                   >   3
                                                  3|tt
                                                   >  4
                                                  4|bb
                                                   >  5
                                                  5|tt
                                                   >  6
                                                  6|tt
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "\n", Qt::CaseSensitive);
    }

    SECTION("cursor in search string") {
        static auto testCases = generateTestCases(R"(
                                                  0|blah
                                                   > 111
                                                  1|blub
                                                   >1111
                                                  2|blah
                                                   > 222
                                                  3|blub
                                                   >2222
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "lah\nblub", Qt::CaseSensitive);
    }

    SECTION("cursor in search string with wraparound") {
        static auto testCases = generateTestCases(R"(
                                                  0|blah
                                                   > 111
                                                  1|blub
                                                   >1111
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, "lah\nblub", Qt::CaseSensitive);
    }


    auto runChecksBackward = [&](const SearchTestCase &testCase, const QString &needle, Qt::CaseSensitivity caseMatching) {
        cursor1.insertText(testCase.documentContents);

        const bool wrapAround = GENERATE(false, true);
        const bool useSelection = GENERATE(false, true);

        CAPTURE(testCase.start);
        CAPTURE(testCase.end);
        CAPTURE(testCase.foundStart);
        CAPTURE(testCase.foundEnd);
        CAPTURE(wrapAround);
        CAPTURE(useSelection);
        Tui::ZDocument::FindFlags options = wrapAround ? Tui::ZDocument::FindFlag::FindWrap : Tui::ZDocument::FindFlags{};
        options |= Tui::ZDocument::FindFlag::FindBackward;
        if (caseMatching == Qt::CaseSensitive) {
            options |= Tui::ZDocument::FindFlag::FindCaseSensitively;
        }

        cursor1.setPosition(testCase.start);

        if (useSelection && !cursor1.atEnd() && !cursor1.atStart()) {
            cursor1.moveCharacterRight();
            cursor1.setAnchorPosition({0, 0});
        }

        if (testCase.hasMatch) {
            auto result = doc.findSync(needle, cursor1, options);
            if (!wrapAround && testCase.start <= testCase.foundStart) {
                CAPTURE(result.anchor());
                CAPTURE(result.position());
                CHECK_FALSE(result.hasSelection());
            } else {
                CHECK(result.hasSelection());
                CHECK(result.anchor() == testCase.foundStart);
                CHECK(result.position() == testCase.foundEnd);
            }

            while (cursor1.position() < testCase.end) {
                REQUIRE(!cursor1.atEnd());
                cursor1.moveCharacterRight();
                // check if we overstepped
                if (cursor1.position() > testCase.end) break;
                CAPTURE(cursor1.position());

                //cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                if (!wrapAround && testCase.start <= testCase.foundStart) {
                    CAPTURE(result.anchor());
                    CAPTURE(result.position());
                    CHECK_FALSE(result.hasSelection());
                } else {
                    CHECK(result.hasSelection());
                    CHECK(result.anchor() == testCase.foundStart);
                    CHECK(result.position() == testCase.foundEnd);
                }
            }
        } else {
            auto result = doc.findSync(needle, cursor1, options);
            CHECK(!result.hasSelection());

            while (!cursor1.atEnd()) {
                CAPTURE(cursor1.position());
                cursor1.moveCharacterRight();

                cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                CHECK(!result.hasSelection());
            }
        }
    };

    SECTION("backward one char t") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "t", Qt::CaseSensitive);
    }

    SECTION("backward empty search string") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "", Qt::CaseSensitive);
    }

    SECTION("backward one char t - mismatched case") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "t", Qt::CaseSensitive);
    }

    SECTION("backward one char t - case insensitive") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "t", Qt::CaseInsensitive);
    }


    SECTION("backward hb") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|blah
                                                   >   1
                                                  1|blub
                                                   >1
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "h\nb", Qt::CaseSensitive);
    }

    SECTION("backward hbl") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|blah
                                                   >   1
                                                  1|blub
                                                   >11
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "h\nbl", Qt::CaseSensitive);
    }

    SECTION("backward ahb") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|blah
                                                   >  11
                                                  1|blub
                                                   >1
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "ah\nb", Qt::CaseSensitive);
    }

    SECTION("backward multiline case mismatch first line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|heLlo
                                                  1|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nworld", Qt::CaseSensitive);
    }

    SECTION("backward multiline case mismatch second line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|hello
                                                  1|woRld
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nworld", Qt::CaseSensitive);
    }

    SECTION("backward multiline case insensitive") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|heLlo
                                                   >11111
                                                  1|woRld
                                                   >11111
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nworld", Qt::CaseInsensitive);
    }

    SECTION("backward three multiline case mismatch first line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|hEllo
                                                  1|whole
                                                  2|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("backward three multiline case mismatch middle line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|hello
                                                  1|whOle
                                                  2|world
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("backward three multiline case mismatch last line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|hello
                                                  1|whole
                                                  2|worlD
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nwhole\nworld", Qt::CaseSensitive);
    }

    SECTION("backward three multiline case insensitive") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|heLlo
                                                   >11111
                                                  1|wHole
                                                   >11111
                                                  2|worlD
                                                   >11111
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "hello\nwhole\nworld", Qt::CaseInsensitive);
    }

    SECTION("backward 123") {

        static auto testCases = generateTestCasesBackward(R"(
                                                  0|123
                                                   >  a
                                                  1|123
                                                   >a b
                                                  2|123
                                                   >b c
                                                  3|123
                                                   >c
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "3\n1", Qt::CaseSensitive);
    }

    SECTION("backward one char in line") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|t
                                                   >1
                                                  1|t
                                                   >1
                                                   >2
                                                  2|t
                                                   >2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, "t\nt", Qt::CaseSensitive);
    }
}


TEST_CASE("regex search") {
    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    struct MatchCaptures {
        QStringList captures;
        QMap<QString, QString> named;
    };

    auto runChecks = [&](const SearchTestCase &testCase, const QRegularExpression &needle, Qt::CaseSensitivity caseMatching,
            const QMap<QString, MatchCaptures> expectedCapturesMap) {
        cursor1.insertText(testCase.documentContents);

        const bool wrapAround = GENERATE(false, true);

        auto checkCaptures = [&] (const Tui::ZDocumentFindResult &res) {
            REQUIRE(expectedCapturesMap.contains(testCase.marker));
            const auto &expectedCaptures = expectedCapturesMap[testCase.marker];
            CHECK(res.regexLastCapturedIndex() == expectedCaptures.captures.size() - 1);
            if (res.regexLastCapturedIndex() == expectedCaptures.captures.size() - 1) {
                for (int i = 0; i < expectedCaptures.captures.size(); i++) {
                    CHECK(res.regexCapture(i) == expectedCaptures.captures[i]);
                }
            }
            for (QString key: expectedCaptures.named.keys()) {
                CHECK(res.regexCapture(key) == expectedCaptures.named.value(key));
            }
        };

        CAPTURE(testCase.marker);
        CAPTURE(testCase.start);
        CAPTURE(testCase.end);
        CAPTURE(testCase.foundStart);
        CAPTURE(testCase.foundEnd);
        CAPTURE(wrapAround);
        Tui::ZDocument::FindFlags options = wrapAround ? Tui::ZDocument::FindFlag::FindWrap : Tui::ZDocument::FindFlags{};
        if (caseMatching == Qt::CaseSensitive) {
            options |= Tui::ZDocument::FindFlag::FindCaseSensitively;
        }

        cursor1.setPosition(testCase.start, true);
        cursor1.setAnchorPosition({0, 0});

        if (testCase.hasMatch) {
            Tui::ZDocumentFindResult result = doc.findSyncWithDetails(needle, cursor1, options);
            if (!wrapAround && testCase.start > testCase.foundStart) {
                CAPTURE(result.cursor().anchor());
                CAPTURE(result.cursor().position());
                CHECK(!result.cursor().hasSelection());
            } else {
                CHECK(result.cursor().hasSelection());
                CHECK(result.cursor().anchor() == testCase.foundStart);
                CHECK(result.cursor().position() == testCase.foundEnd);
                checkCaptures(result);
            }

            while (cursor1.position() < testCase.end) {
                CAPTURE(cursor1.position());
                REQUIRE(!cursor1.atEnd());
                cursor1.moveCharacterRight();
                // check if we overstepped
                if (cursor1.position() > testCase.end) break;

                cursor1.setAnchorPosition({0, 0});
                Tui::ZDocumentFindResult result = doc.findSyncWithDetails(needle, cursor1, options);
                if (!wrapAround && testCase.start > testCase.foundStart) {
                    CAPTURE(result.cursor().anchor());
                    CAPTURE(result.cursor().position());
                    CHECK(!result.cursor().hasSelection());
                } else {
                    CHECK(result.cursor().hasSelection());
                    CHECK(result.cursor().anchor() == testCase.foundStart);
                    CHECK(result.cursor().position() == testCase.foundEnd);
                    checkCaptures(result);
                }
            }
        } else {
            auto result = doc.findSync(needle, cursor1, options);
            CHECK(!result.hasSelection());

            while (!cursor1.atEnd()) {
                CAPTURE(cursor1.position());
                cursor1.moveCharacterRight();

                cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                CHECK(!result.hasSelection());
            }
        }
    };

    SECTION("invalid regex") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("["), Qt::CaseSensitive, {});
    }

    SECTION("literal-a") {
        static auto testCases = generateTestCases(R"(
                                                  0|some Text
                                                  1|same Thing
                                                   > 1
                                                  2|aaaa bbbb
                                                   >2345
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("a"), Qt::CaseSensitive,
                {
                      {"1", MatchCaptures{ {"a"}, {}}},
                      {"2", MatchCaptures{ {"a"}, {}}},
                      {"3", MatchCaptures{ {"a"}, {}}},
                      {"4", MatchCaptures{ {"a"}, {}}},
                      {"5", MatchCaptures{ {"a"}, {}}}
                });

    }

    SECTION("literal-linebreak") {
        static auto testCases = generateTestCases(R"(
                                                  0|some Text
                                                   >         1
                                                  1|same Thing
                                                   >          2
                                                  2|aaaa bbbb
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("\\n"), Qt::CaseSensitive,
                {
                      {"1", MatchCaptures{ {"\n"}, {}}},
                      {"2", MatchCaptures{ {"\n"}, {}}},
                      {"3", MatchCaptures{ {"\n"}, {}}}
                });

    }

    SECTION("multi-linebreak") {
        static auto testCases = generateTestCases(R"(
                                                  0|some Text
                                                   >         1
                                                  1|same Thing
                                                   >11111111111
                                                  2|aaaa bbbb
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("\\n.*\\n"), Qt::CaseSensitive,
                {
                      {"1", MatchCaptures{ {"\nsame Thing\n"}, {}}},
                });

    }

    SECTION("literal astral") {
        static auto testCases = generateTestCases(R"(
                                                  0|some Text
                                                  1|s😁me Thing
                                                   > 11
                                                  2|😁aa bbbb
                                                   >22
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("😁"), Qt::CaseSensitive,
                {
                      {"1", MatchCaptures{ {"😁"}, {}}},
                      {"2", MatchCaptures{ {"😁"}, {}}}
                });

    }

    SECTION("one char t - mismatched case") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t"), Qt::CaseSensitive, {
              {"2", MatchCaptures{ {"t"}, {}}},
        });
    }

    SECTION("one char t - case insensitive") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t"), Qt::CaseInsensitive, {
              {"1", MatchCaptures{ {"T"}, {}}},
              {"2", MatchCaptures{ {"t"}, {}}},
        });
    }

    SECTION("one char t - mismatched case with pattern option") {
        static auto testCases = generateTestCases(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t", QRegularExpression::PatternOption::CaseInsensitiveOption),
                  Qt::CaseSensitive, {
              {"2", MatchCaptures{ {"t"}, {}}},
        });
    }

    SECTION("literal-abc") {
        static auto testCases = generateTestCases(R"(
            0|some Test
            1|abc Thing
             >111
            2|xabcabc bbbb
             > 222333
        )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("abc"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("literal-abc-nonutf16-in-line") {
        static auto testCases = generateTestCases(QString(R"(
                                  0|some Test
                                  1|abc Xhing
                                   >111
                                  2|xabcabc bbbb
                                   > 222333
                              )").replace('X', QChar(0xdc00)));

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression{"abc"}, Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("literal-abc-nonutf16-at-end") {
        static auto testCases = generateTestCases(QString(R"(
                                  0|some Test
                                  1|abc thingX
                                   >111
                                  2|xabcabc bbbb
                                   > 222333
                              )").replace('X', QChar(0xd800)));

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression{"abc"}, Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("multiline line literal") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >1
                                  2|xabcabc bbbb
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t\na"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\na"}, {}}},
        });
    }

    SECTION("three multiline line literal") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                                   >1111
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t\n.*\nxabc"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\nabc Thing\nxabc"}, {}}},
        });
    }

    SECTION("multiline line dotdefault") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                  1|abc Thing
                                  2|xabcabc bbbb
                              )");
        REQUIRE(testCases[0].hasMatch == false);
        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("t.*xabc"), Qt::CaseSensitive, {});
    }

    SECTION("multiline dotall") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                                   >1111
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression{"t.*xabc", QRegularExpression::PatternOption::DotMatchesEverythingOption},
                  Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\nabc Thing\nxabc"}, {}}},
        });
    }

    SECTION("multiline dotall prefix") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                  1|abc Thing
                                   >    11111
                                  2|xabcabc bbbb
                                   >1111
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression{"T[^T]*xabc", QRegularExpression::PatternOption::DotMatchesEverythingOption},
                  Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"Thing\nxabc"}, {}}},
        });
    }

    SECTION("anchors") {
        static auto testCases = generateTestCases(R"(
                                  0|some Test
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("^abc Thing$"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc Thing"}, {}}},
        });
    }

    SECTION("numbers") {
        static auto testCases = generateTestCases(R"(
                                  0|This is 1 test
                                   >        1
                                  1|2 test
                                   >2
                                  2|and the last 3
                                   >             3
                                  3|or 123 456tests
                                   >   456 789
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("[0-9]"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"1"}, {}}},
              {"2", MatchCaptures{ {"2"}, {}}},
              {"3", MatchCaptures{ {"3"}, {}}},
              {"4", MatchCaptures{ {"1"}, {}}},
              {"5", MatchCaptures{ {"2"}, {}}},
              {"6", MatchCaptures{ {"3"}, {}}},
              {"7", MatchCaptures{ {"4"}, {}}},
              {"8", MatchCaptures{ {"5"}, {}}},
              {"9", MatchCaptures{ {"6"}, {}}},
        });
    }

    SECTION("numbers+") {
        static auto testCases = generateTestCases(R"(
                                  0|This is 1 test
                                   >        1
                                  1|2 test
                                   >2
                                  2|and the last 3
                                   >             3
                                  3|or 123 456tests
                                   >   444 555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("[0-9]+"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"1"}, {}}},
              {"2", MatchCaptures{ {"2"}, {}}},
              {"3", MatchCaptures{ {"3"}, {}}},
              {"4", MatchCaptures{ {"123"}, {}}},
              {"5", MatchCaptures{ {"456"}, {}}},
        });
    }

    SECTION("captures") {
        static auto testCases = generateTestCases(R"(
                                  0|This is <1> test
                                   >        111
                                  1|<2> test
                                   >222
                                  2|and the last <3>
                                   >             333
                                  3|or <123> <456>tests
                                   >   44444 55555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("<([0-9]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1>", "1"}, {}}},
              {"2", MatchCaptures{ {"<2>", "2"}, {}}},
              {"3", MatchCaptures{ {"<3>", "3"}, {}}},
              {"4", MatchCaptures{ {"<123>", "123"}, {}}},
              {"5", MatchCaptures{ {"<456>", "456"}, {}}},
        });
    }

    SECTION("named captures") {
        static auto testCases = generateTestCases(R"(
                                  0|This is <1> test
                                   >        111
                                  1|<2> test
                                   >222
                                  2|and the last <3>
                                   >             333
                                  3|or <123> <456>tests
                                   >   44444 55555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("<(?<zahl>[0-9]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1>", "1"}, {{"zahl", "1"}}}},
              {"2", MatchCaptures{ {"<2>", "2"}, {{"zahl", "2"}}}},
              {"3", MatchCaptures{ {"<3>", "3"}, {{"zahl", "3"}}}},
              {"4", MatchCaptures{ {"<123>", "123"}, {{"zahl", "123"}}}},
              {"5", MatchCaptures{ {"<456>", "456"}, {{"zahl", "456"}}}},
        });
    }

    SECTION("captures 2x") {
        static auto testCases = generateTestCases(R"(
                                  0|This is <1=x> test
                                   >        11111
                                  1|<2=u> test
                                   >22222
                                  2|and the last <3=t>
                                   >             33333
                                  3|or <123=xut> <456=L>tests
                                   >   444444444 5555555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("<([0-9]+)=([a-zA-Z]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1=x>", "1", "x"}, {}}},
              {"2", MatchCaptures{ {"<2=u>", "2", "u"}, {}}},
              {"3", MatchCaptures{ {"<3=t>", "3", "t"}, {}}},
              {"4", MatchCaptures{ {"<123=xut>", "123", "xut"}, {}}},
              {"5", MatchCaptures{ {"<456=L>", "456", "L"}, {}}},
        });
    }

    SECTION("named captures 2x") {
        static auto testCases = generateTestCases(R"(
                                  0|This is <1=x> test
                                   >        11111
                                  1|<2=u> test
                                   >22222
                                  2|and the last <3=t>
                                   >             33333
                                  3|or <123=xut> <456=L>tests
                                   >   444444444 5555555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecks(testCase, QRegularExpression("<(?<zahl>[0-9]+)=(?<letters>[a-zA-Z]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1=x>", "1", "x"}, {{"zahl", "1"}, {"letters", "x"}}}},
              {"2", MatchCaptures{ {"<2=u>", "2", "u"}, {{"zahl", "2"}, {"letters", "u"}}}},
              {"3", MatchCaptures{ {"<3=t>", "3", "t"}, {{"zahl", "3"}, {"letters", "t"}}}},
              {"4", MatchCaptures{ {"<123=xut>", "123", "xut"}, {{"zahl", "123"}, {"letters", "xut"}}}},
              {"5", MatchCaptures{ {"<456=L>", "456", "L"}, {{"zahl", "456"}, {"letters", "L"}}}},
        });
    }

    auto runChecksBackward = [&](const SearchTestCase &testCase, const QRegularExpression &needle, Qt::CaseSensitivity caseMatching,
            const QMap<QString, MatchCaptures> expectedCapturesMap) {
        cursor1.insertText(testCase.documentContents);

        const bool wrapAround = GENERATE(false, true);
        const bool useSelection = GENERATE(false, true);

        auto checkCaptures = [&] (const Tui::ZDocumentFindResult &res) {
            REQUIRE(expectedCapturesMap.contains(testCase.marker));
            const auto &expectedCaptures = expectedCapturesMap[testCase.marker];
            CHECK(res.regexLastCapturedIndex() == expectedCaptures.captures.size() - 1);
            if (res.regexLastCapturedIndex() == expectedCaptures.captures.size() - 1) {
                for (int i = 0; i < expectedCaptures.captures.size(); i++) {
                    CHECK(res.regexCapture(i) == expectedCaptures.captures[i]);
                }
            }
            for (QString key: expectedCaptures.named.keys()) {
                CHECK(res.regexCapture(key) == expectedCaptures.named.value(key));
            }
        };

        CAPTURE(testCase.marker);
        CAPTURE(testCase.start);
        CAPTURE(testCase.end);
        CAPTURE(testCase.foundStart);
        CAPTURE(testCase.foundEnd);
        CAPTURE(wrapAround);
        CAPTURE(useSelection);
        Tui::ZDocument::FindFlags options = wrapAround ? Tui::ZDocument::FindFlag::FindWrap : Tui::ZDocument::FindFlags{};
        options |= Tui::ZDocument::FindFlag::FindBackward;
        if (caseMatching == Qt::CaseSensitive) {
            options |= Tui::ZDocument::FindFlag::FindCaseSensitively;
        }

        cursor1.setPosition(testCase.start);

        if (useSelection && !cursor1.atEnd() && !cursor1.atStart()) {
            cursor1.moveCharacterRight();
            cursor1.setAnchorPosition({0, 0});
        }

        if (testCase.hasMatch) {
            Tui::ZDocumentFindResult result = doc.findSyncWithDetails(needle, cursor1, options);
            if (!wrapAround && testCase.start <= testCase.foundStart) {
                CAPTURE(result.cursor().anchor());
                CAPTURE(result.cursor().position());
                CHECK_FALSE(result.cursor().hasSelection());
            } else {
                CHECK(result.cursor().hasSelection());
                CHECK(result.cursor().anchor() == testCase.foundStart);
                CHECK(result.cursor().position() == testCase.foundEnd);
                checkCaptures(result);
            }

            while (cursor1.position() < testCase.end) {
                REQUIRE(!cursor1.atEnd());
                cursor1.moveCharacterRight();
                // check if we overstepped
                if (cursor1.position() > testCase.end) break;
                CAPTURE(cursor1.position());

                //cursor1.setAnchorPosition({0, 0});
                Tui::ZDocumentFindResult result = doc.findSyncWithDetails(needle, cursor1, options);
                if (!wrapAround && testCase.start <= testCase.foundStart) {
                    CAPTURE(result.cursor().anchor());
                    CAPTURE(result.cursor().position());
                    CHECK_FALSE(result.cursor().hasSelection());
                } else {
                    CHECK(result.cursor().hasSelection());
                    CHECK(result.cursor().anchor() == testCase.foundStart);
                    CHECK(result.cursor().position() == testCase.foundEnd);
                    checkCaptures(result);
                }
            }
        } else {
            auto result = doc.findSync(needle, cursor1, options);
            CHECK(!result.hasSelection());

            while (!cursor1.atEnd()) {
                CAPTURE(cursor1.position());
                cursor1.moveCharacterRight();

                cursor1.setAnchorPosition({0, 0});
                auto result = doc.findSync(needle, cursor1, options);
                CHECK(!result.hasSelection());
            }
        }
    };

    SECTION("backward invalid regex") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("["), Qt::CaseSensitive, {});
    }

    SECTION("backward literal-a") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|some Text
                                                  1|same Thing
                                                   > 1
                                                  2|aaaa bbbb
                                                   >2345
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("a"), Qt::CaseSensitive,
        {
              {"1", MatchCaptures{ {"a"}, {}}},
              {"2", MatchCaptures{ {"a"}, {}}},
              {"3", MatchCaptures{ {"a"}, {}}},
              {"4", MatchCaptures{ {"a"}, {}}},
              {"5", MatchCaptures{ {"a"}, {}}}
        });

    }


    SECTION("backward literal-newline") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|some Text
                                                   >         1
                                                  1|same Thing
                                                   >          2
                                                  2|aaaa bbbb
                                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("\\n"), Qt::CaseSensitive,
        {
              {"1", MatchCaptures{ {"\n"}, {}}},
              {"2", MatchCaptures{ {"\n"}, {}}},
        });

    }

    SECTION("backward one char t - mismatched case") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t"), Qt::CaseSensitive, {
                              {"2", MatchCaptures{ {"t"}, {}}},
                        });
    }

    SECTION("backward one char t - case insensitive") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                                   >1  2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t"), Qt::CaseInsensitive, {
              {"1", MatchCaptures{ {"T"}, {}}},
              {"2", MatchCaptures{ {"t"}, {}}},
        });
    }

    SECTION("backward one char t - mismatched case with pattern option") {
        static auto testCases = generateTestCasesBackward(R"(
                                                  0|Test
                                                   >   2
                                              )");
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t", QRegularExpression::PatternOption::CaseInsensitiveOption),
                  Qt::CaseSensitive, {
              {"2", MatchCaptures{ {"t"}, {}}},
        });
    }

    SECTION("backward literal-abc") {
        static auto testCases = generateTestCasesBackward(R"(
            0|some Test
            1|abc Thing
             >111
            2|xabcabc bbbb
             > 222333
        )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("abc"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("backward literal-abc with mismatched multi line option") {
        static auto testCases = generateTestCasesBackward(R"(
            0|some Test
            1|abc Thing
             >111
            2|xabcabc bbbb
             > 222333
        )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("abc", QRegularExpression::PatternOption::MultilineOption),
                          Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("backward literal-abc-nonutf16-in-line") {
        static auto testCases = generateTestCasesBackward(QString(R"(
                                  0|some Test
                                  1|abc Xhing
                                   >111
                                  2|xabcabc bbbb
                                   > 222333
                              )").replace('X', QChar(0xdc00)));

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression{"abc"}, Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("backward literal-abc-nonutf16-at-end") {
        static auto testCases = generateTestCasesBackward(QString(R"(
                                  0|some Test
                                  1|abc thingX
                                   >111
                                  2|xabcabc bbbb
                                   > 222333
                              )").replace('X', QChar(0xd800)));

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression{"abc"}, Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc"}, {}}},
              {"2", MatchCaptures{ {"abc"}, {}}},
              {"3", MatchCaptures{ {"abc"}, {}}},
        });
    }

    SECTION("backward multiline line literal") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >1
                                  2|xabcabc bbbb
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t\na"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\na"}, {}}},
        });
    }

    SECTION("backward multiline line literal with regex option already set.") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >1
                                  2|xabcabc bbbb
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t\na", QRegularExpression::PatternOption::MultilineOption),
                          Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\na"}, {}}},
        });
    }

    SECTION("backward three multiline line literal") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                                   >1111
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t\n.*\nxabc"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\nabc Thing\nxabc"}, {}}},
        });
    }

    SECTION("backward multiline line dotdefault") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                  1|abc Thing
                                  2|xabcabc bbbb
                              )");
        REQUIRE(testCases[0].hasMatch == false);
        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("t.*xabc"), Qt::CaseSensitive, {});
    }

    SECTION("backward multiline dotall") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                   >        1
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                                   >1111
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression{"t.*xabc", QRegularExpression::PatternOption::DotMatchesEverythingOption},
                  Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"t\nabc Thing\nxabc"}, {}}},
        });
    }

    SECTION("backward anchors") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|some Test
                                  1|abc Thing
                                   >111111111
                                  2|xabcabc bbbb
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("^abc Thing$"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"abc Thing"}, {}}},
        });
    }

    SECTION("backward numbers") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is 1 test
                                   >        1
                                  1|2 test
                                   >2
                                  2|and the last 3
                                   >             3
                                  3|or 123 456tests
                                   >   456 789
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("[0-9]"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"1"}, {}}},
              {"2", MatchCaptures{ {"2"}, {}}},
              {"3", MatchCaptures{ {"3"}, {}}},
              {"4", MatchCaptures{ {"1"}, {}}},
              {"5", MatchCaptures{ {"2"}, {}}},
              {"6", MatchCaptures{ {"3"}, {}}},
              {"7", MatchCaptures{ {"4"}, {}}},
              {"8", MatchCaptures{ {"5"}, {}}},
              {"9", MatchCaptures{ {"6"}, {}}},
        });
    }

    SECTION("backward numbers+") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is 1 test
                                   >        1
                                  1|2 test
                                   >2
                                  2|and the last 3
                                   >             3
                                  3|or 123 456tests
                                   >   444 555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("[0-9]+"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"1"}, {}}},
              {"2", MatchCaptures{ {"2"}, {}}},
              {"3", MatchCaptures{ {"3"}, {}}},
              {"4", MatchCaptures{ {"123"}, {}}},
              {"5", MatchCaptures{ {"456"}, {}}},
        });
    }

    SECTION("backward captures") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is <1> test
                                   >        111
                                  1|<2> test
                                   >222
                                  2|and the last <3>
                                   >             333
                                  3|or <123> <456>tests
                                   >   44444 55555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("<([0-9]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1>", "1"}, {}}},
              {"2", MatchCaptures{ {"<2>", "2"}, {}}},
              {"3", MatchCaptures{ {"<3>", "3"}, {}}},
              {"4", MatchCaptures{ {"<123>", "123"}, {}}},
              {"5", MatchCaptures{ {"<456>", "456"}, {}}},
        });
    }

    SECTION("backward named captures") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is <1> test
                                   >        111
                                  1|<2> test
                                   >222
                                  2|and the last <3>
                                   >             333
                                  3|or <123> <456>tests
                                   >   44444 55555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("<(?<zahl>[0-9]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1>", "1"}, {{"zahl", "1"}}}},
              {"2", MatchCaptures{ {"<2>", "2"}, {{"zahl", "2"}}}},
              {"3", MatchCaptures{ {"<3>", "3"}, {{"zahl", "3"}}}},
              {"4", MatchCaptures{ {"<123>", "123"}, {{"zahl", "123"}}}},
              {"5", MatchCaptures{ {"<456>", "456"}, {{"zahl", "456"}}}},
        });
    }

    SECTION("backward captures 2x") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is <1=x> test
                                   >        11111
                                  1|<2=u> test
                                   >22222
                                  2|and the last <3=t>
                                   >             33333
                                  3|or <123=xut> <456=L>tests
                                   >   444444444 5555555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("<([0-9]+)=([a-zA-Z]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1=x>", "1", "x"}, {}}},
              {"2", MatchCaptures{ {"<2=u>", "2", "u"}, {}}},
              {"3", MatchCaptures{ {"<3=t>", "3", "t"}, {}}},
              {"4", MatchCaptures{ {"<123=xut>", "123", "xut"}, {}}},
              {"5", MatchCaptures{ {"<456=L>", "456", "L"}, {}}},
        });
    }

    SECTION("backward named captures 2x") {
        static auto testCases = generateTestCasesBackward(R"(
                                  0|This is <1=x> test
                                   >        11111
                                  1|<2=u> test
                                   >22222
                                  2|and the last <3=t>
                                   >             33333
                                  3|or <123=xut> <456=L>tests
                                   >   444444444 5555555
                              )");

        auto testCase = GENERATE(from_range(testCases));

        runChecksBackward(testCase, QRegularExpression("<(?<zahl>[0-9]+)=(?<letters>[a-zA-Z]+)>"), Qt::CaseSensitive, {
              {"1", MatchCaptures{ {"<1=x>", "1", "x"}, {{"zahl", "1"}, {"letters", "x"}}}},
              {"2", MatchCaptures{ {"<2=u>", "2", "u"}, {{"zahl", "2"}, {"letters", "u"}}}},
              {"3", MatchCaptures{ {"<3=t>", "3", "t"}, {{"zahl", "3"}, {"letters", "t"}}}},
              {"4", MatchCaptures{ {"<123=xut>", "123", "xut"}, {{"zahl", "123"}, {"letters", "xut"}}}},
              {"5", MatchCaptures{ {"<456=L>", "456", "L"}, {{"zahl", "456"}, {"letters", "L"}}}},
        });
    }

}

TEST_CASE("async search") {
    // We basically assume that async search uses the same internal search functions as the sync search,
    // so this only tests searching lightly and focuses on the async interface.

    Testhelper t("unused", "unused", 2, 4);
    auto textMetrics = t.terminal->textMetrics();

    Tui::ZDocument doc;

    Tui::ZDocumentCursor cursor1{&doc, [&textMetrics, &doc](int line, bool /* wrappingAllowed */) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            lay.doLayout(65000);
            return lay;
        }
    };

    SECTION("search substring") {
        cursor1.insertText("test a, which is a test that tests a.");
        cursor1.setPosition({0, 0});
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync("a", cursor1);

        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{5, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{6, 0});
        CHECK(result.revision() == doc.revision());
        CHECK(result.regexLastCapturedIndex() == -1);

        // repeat with same cursor should give same result
        future = doc.findAsync("a", cursor1);
        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{5, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{6, 0});
        CHECK(result.revision() == doc.revision());

        auto cursor2 = cursor1;

        std::vector<int> positions = {17, 26, 35};
        for (int expected: positions) {
            CAPTURE(expected);
            cursor2.setPosition(result.anchor());
            cursor2.setPosition(result.cursor(), true);
            // repeat with cursor build from search result should find next result;
            future = doc.findAsync("a", cursor2);
            future.waitForFinished();
            REQUIRE(future.isFinished());
            REQUIRE(future.isResultReadyAt(0) == true);
            result = future.result();

            CHECK(result.anchor() == Tui::ZDocumentCursor::Position{expected, 0});
            CHECK(result.cursor() == Tui::ZDocumentCursor::Position{expected + 1, 0});
            CHECK(result.revision() == doc.revision());
        }
    }

    SECTION("empty search string") {
        cursor1.insertText("test a, which is a test that tests a.");
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync("", cursor1);
        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == result.cursor());
        CHECK(result.revision() == doc.revision());
    }

    SECTION("search substring reverse") {
        cursor1.insertText("test a, which is a test that tests a.");
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync("a", cursor1, Tui::ZDocument::FindFlag::FindBackward);
        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{35, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{36, 0});
        CHECK(result.revision() == doc.revision());
    }

    SECTION("search regex") {
        cursor1.insertText("test a, which is a test that tests a.");
        cursor1.setPosition({0, 0});
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync(QRegularExpression(" ([^ ]+h)[^ ]+ "), cursor1);

        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{7, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{14, 0});
        CHECK(result.revision() == doc.revision());
        CHECK(result.regexLastCapturedIndex() == 1);
        CHECK(result.regexCapture(0) == QString(" which "));
        CHECK(result.regexCapture(1) == QString("wh"));
    }

    SECTION("invalid regex") {
        cursor1.insertText("test a, which is a test that tests a.");
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync(QRegularExpression("["), cursor1);
        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == result.cursor());
        CHECK(result.revision() == doc.revision());
    }

    SECTION("search regex reverse") {
        cursor1.insertText("test a, which is a test that tests a.");
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync(QRegularExpression(" ([^ ]+h)[^ ]+ "), cursor1,
                                                                Tui::ZDocument::FindFlag::FindBackward);

        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{23, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{29, 0});
        CHECK(result.revision() == doc.revision());
        CHECK(result.regexLastCapturedIndex() == 1);
        CHECK(result.regexCapture(0) == QString(" that "));
        CHECK(result.regexCapture(1) == QString("th"));
    }

    SECTION("search regex named capture") {
        cursor1.insertText("test a, which is a test that tests a.");
        cursor1.setPosition({0, 0});
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync(QRegularExpression(" (?<capture>[^ ]+h)[^ ]+ "), cursor1);

        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{7, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{14, 0});
        CHECK(result.revision() == doc.revision());
        CHECK(result.regexLastCapturedIndex() == 1);
        CHECK(result.regexCapture(0) == QString(" which "));
        CHECK(result.regexCapture("capture") == QString("wh"));
    }

    SECTION("search regex with changes") {
        cursor1.insertText("test a, which is a test that tests a.");
        cursor1.setPosition({0, 0});
        QFuture<Tui::ZDocumentFindAsyncResult> future = doc.findAsync(QRegularExpression(" ([^ ]+h)[^ ]+ "), cursor1);

        future.waitForFinished();
        REQUIRE(future.isFinished());
        REQUIRE(future.isResultReadyAt(0) == true);
        Tui::ZDocumentFindAsyncResult result = future.result();
        cursor1.insertText("dummy");

        CHECK(result.anchor() == Tui::ZDocumentCursor::Position{7, 0});
        CHECK(result.cursor() == Tui::ZDocumentCursor::Position{14, 0});
        CHECK(result.revision() != doc.revision());
        CHECK(result.regexLastCapturedIndex() == 1);
        CHECK(result.regexCapture(0) == QString(" which "));
        CHECK(result.regexCapture(1) == QString("wh"));
    }

    SECTION("search regex with cancel") {
        // We can only check if canceling the future really stops the search thread by timing the usage of the
        // threads in the thread pool.
        // We increase the searched text until we reach a decent time the search takes and then check if
        // canceling reduces the time to almost nothing.

        // There is a possibility that timing differences make this test success even when cancelation
        // does not work.

        cursor1.insertText("test a, which is a test that tests a.X");

        QFuture<Tui::ZDocumentFindAsyncResult> future;
        QElapsedTimer timer;

        int sizeIncrement = 100000;

        int uncanceledInMs = 0;

        bool testSuccessful = false;

        while (uncanceledInMs < 30000) {
            cursor1.setPosition({0, 0});
            cursor1.insertText(QString(' ').repeated(sizeIncrement) + "\n");
            timer.start();
            cursor1.setPosition({0, 0});
            future = doc.findAsync(QRegularExpression(".*X"), cursor1);
            future.waitForFinished();
            sizeIncrement *= 2;

            uncanceledInMs = timer.elapsed();

            if (uncanceledInMs > 300) {
                INFO("ms for uncanceled run: " << timer.elapsed());

                cursor1.setPosition({0, 0});
                timer.start();
                future = doc.findAsync(QRegularExpression(".*X"), cursor1);

                while (QThreadPool::globalInstance()->activeThreadCount() == 0) {
                    // wait
                }

                future.cancel();

                future.waitForFinished();
                REQUIRE(future.isFinished());
                REQUIRE(future.isCanceled());

                while (QThreadPool::globalInstance()->activeThreadCount() != 0) {
                    // wait
                }
                const int elapsedInMs = timer.elapsed();
                INFO("ms for canceled run: " << elapsedInMs);
                if (elapsedInMs < uncanceledInMs / 10) {
                    testSuccessful = true;
                    break;
                }
            }
        }
        CHECK(testSuccessful);
    }
}

