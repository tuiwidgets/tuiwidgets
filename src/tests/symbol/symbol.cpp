// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZSymbol.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"

TEST_CASE("symbol-base") {

    SECTION("constructor") {
        delete new Tui::ZSymbol();
        delete new Tui::ZSymbol("c");
    }

    SECTION("default") {
        Tui::ZSymbol b;
        CHECK(b.toString() == "");
        CHECK(!b);
        Tui::ZSymbol c = Tui::ZSymbol("");
        CHECK(b == c);
        CHECK(b == TUISYM_LITERAL(""));
        CHECK(qHash(b) == 0);
        CHECK(std::hash<Tui::ZSymbol>()(b) == 0);
    }

    SECTION("same-text") {
        Tui::ZSymbol c1 = Tui::ZSymbol("c");
        Tui::ZSymbol c2 = Tui::ZSymbol("c");
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
        CHECK(qHash(c1) == qHash(c2));
        CHECK(std::hash<Tui::ZSymbol>()(c1) == std::hash<Tui::ZSymbol>()(c2));
    }

    SECTION("operators1") {
        Tui::ZSymbol c = Tui::ZSymbol("c");
        CHECK(c == c);
        CHECK(c <= c);
        CHECK(c >= c);
        Tui::ZSymbol a = Tui::ZSymbol("a");
        CHECK(c != a);
        CHECK(c < a);
        CHECK(c <= a);
        CHECK(a > c);
        CHECK(a >= c);
        CHECK(c);
        CHECK(!!c);

        CHECK(c.toString() == "c");
        CHECK(qHash(c) != qHash(a));
        CHECK(std::hash<Tui::ZSymbol>()(c) != std::hash<Tui::ZSymbol>()(a));
    }

    SECTION("operators2") {
        Tui::ZSymbol c = Tui::ZSymbol("あい");
        CHECK(c == c);
        CHECK(c <= c);
        CHECK(c >= c);
        Tui::ZSymbol a = Tui::ZSymbol("😇");
        CHECK(c != a);
        CHECK(c < a);
        CHECK(c <= a);
        CHECK(a > c);
        CHECK(a >= c);

        CHECK(c.toString() == "あい");
        CHECK(c == TUISYM_LITERAL("あい"));
        CHECK(a.toString() == "😇");
        CHECK(a == TUISYM_LITERAL("😇"));
    }

    SECTION("assignment") {
        Tui::ZSymbol c1 = Tui::ZSymbol("c");
        Tui::ZSymbol c2 = Tui::ZSymbol("del");
        c1 = c2;
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
    }
}

void implicitTest(Tui::ZImplicitSymbol s) {
}

TEST_CASE("implicit-symbol-base") {

    SECTION("constructor") {
        delete new Tui::ZImplicitSymbol();
        delete new Tui::ZImplicitSymbol("c");
    }

    SECTION("default") {
        Tui::ZImplicitSymbol b;
        CHECK(b.toString() == "");
        Tui::ZImplicitSymbol c = Tui::ZImplicitSymbol("");
        CHECK(b == c);
        CHECK(b == TUISYM_LITERAL(""));
        CHECK(qHash(b) == 0);
        CHECK(std::hash<Tui::ZSymbol>()(b) == 0);
    }

    SECTION("same-text") {
        Tui::ZImplicitSymbol c1 = Tui::ZImplicitSymbol("c");
        Tui::ZImplicitSymbol c2 = Tui::ZImplicitSymbol("c");
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
        CHECK(qHash(c1) == qHash(c2));
        CHECK(std::hash<Tui::ZSymbol>()(c1) == std::hash<Tui::ZSymbol>()(c2));
    }

    SECTION("operators1") {
        Tui::ZImplicitSymbol c = Tui::ZImplicitSymbol("c");
        CHECK(c == c);
        CHECK(c <= c);
        CHECK(c >= c);
        Tui::ZImplicitSymbol a = Tui::ZImplicitSymbol("a");
        CHECK(c != a);
        CHECK(c < a);
        CHECK(c <= a);
        CHECK(a > c);
        CHECK(a >= c);

        CHECK(c.toString() == "c");
        CHECK(qHash(c) != qHash(a));
        CHECK(std::hash<Tui::ZSymbol>()(c) != std::hash<Tui::ZSymbol>()(a));
    }

    SECTION("operators2") {
        Tui::ZImplicitSymbol c = Tui::ZImplicitSymbol("あい");
        CHECK(c == c);
        CHECK(c <= c);
        CHECK(c >= c);
        Tui::ZImplicitSymbol a = Tui::ZImplicitSymbol("😇");
        CHECK(c != a);
        CHECK(c < a);
        CHECK(c <= a);
        CHECK(a > c);
        CHECK(a >= c);

        CHECK(c.toString() == "あい");
        CHECK(c == TUISYM_LITERAL("あい"));
        CHECK(a.toString() == "😇");
        CHECK(a == TUISYM_LITERAL("😇"));
    }

    SECTION("assignment") {
        Tui::ZImplicitSymbol c1 = Tui::ZImplicitSymbol("c");
        Tui::ZImplicitSymbol c2 = Tui::ZImplicitSymbol("del");
        c1 = c2;
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
    }

    SECTION("assignment-im-to-sym") {
        Tui::ZImplicitSymbol c1 = Tui::ZImplicitSymbol("c");
        Tui::ZSymbol c2 = Tui::ZSymbol("del");
        c1 = c2;
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
    }

    SECTION("assignment-sym-to-im") {
        Tui::ZSymbol c1 = Tui::ZSymbol("c");
        Tui::ZImplicitSymbol c2 = Tui::ZImplicitSymbol("del");
        c1 = c2;
        CHECK(c1 == c2);
        CHECK(c1.toString() == c2.toString());
    }

    SECTION("implicit") {
        implicitTest("text");
        implicitTest(QString("text"));
    }
}
