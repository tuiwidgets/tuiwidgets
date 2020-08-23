#include "../third-party/catch.hpp"

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
    for (int i = 0; i < testCase.result.size(); i++) {
        tok.getToken();
        CHECK(tok._isError == false);
        CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_Char);
        CHECK(tok.currentChar == testCase.result[i]);
    }
    tok.getToken();
    CHECK(tok._isError == false);
    CHECK(tok.currentToken == Tui::Private::MarkupTokenizer::TT_EOF);
}
