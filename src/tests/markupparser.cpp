// SPDX-License-Identifier: BSL-1.0

#include "catchwrapper.h"

#include "Tui/MarkupParser.h"

TEST_CASE("markup parser entities") {
    struct TestCase { const QString text; std::vector<char32_t> result; };
    const auto testCase = GENERATE(
        TestCase{ "&gt;", { '>' } },
        TestCase{ "&lt;", { '<' } },
        TestCase{ "&quot;", { '"' } },
        TestCase{ "&apos;", { '\'' } },
        TestCase{ "&amp;", { '&' } },
        TestCase{ "&larrlp;", { 0x21ab } }, // chain with length 6
        TestCase{ "&NotEqualTilde;", { 8770, 824 } }, // two code points
        TestCase{ "&Xopf;", { 0x1d54f } }, // non BMP
        TestCase{ "&#60;", { 60 } },
        TestCase{ "&#x60;", { 0x60 } }
    );
    CAPTURE(testCase.text);
    Tui::Private::MarkupTokenizer tok = testCase.text;
    for (size_t i = 0; i < testCase.result.size(); i++) {
        tok.getToken();
        CHECK(tok._isError == false);
        CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_Char);
        CHECK(tok.currentChar == testCase.result[i]);
    }
    tok.getToken();
    CHECK(tok._isError == false);
    CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_EOF);
}

TEST_CASE("astral plain") {
    Tui::Private::MarkupTokenizer tok{"😀"};
    tok.getToken();
    CHECK(tok._isError == false);
    CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_Char);
    CHECK(tok.currentChar == 0x1F600);
    CHECK(tok.currentChar == U'😀');
    tok.getToken();
    CHECK(tok._isError == false);
    CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_EOF);
    // TODO this needs checks for unpaired surrogates and end of string just after a high surrogate.
}
