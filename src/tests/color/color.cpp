#include <Tui/ZColor.h>

#include <array>

#include <termpaint.h>

#include "../../third-party/catch.hpp"

TEST_CASE("zcolor - base") {
    SECTION("default constructor") {
        Tui::ZColor color;
        CHECK(color.colorType() == Tui::ZColor::Default);
        CHECK(color.nativeValue() == TERMPAINT_DEFAULT_COLOR);
    }

    SECTION("zcolor - predefined color") {
        Tui::ZColor color = Tui::Color::brown;
        CHECK(color.red() == 0xaa);
        CHECK(color.green() == 0x55);
        CHECK(color.blue() == 0);
    }

    SECTION("compare") {
        std::array<Tui::ZColor, 10> colors = {
            Tui::ZColor(),
            Tui::ZColor::fromRgb(12, 34, 56),
            Tui::ZColor::fromRgb(0, 34, 56),
            Tui::ZColor::fromRgb(12, 54, 56),
            Tui::ZColor::fromRgb(12, 34, 89),
            Tui::ZColor::fromTerminalColor(Tui::TerminalColor::green),
            Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightGreen),
            Tui::ZColor::fromTerminalColorIndexed(2),
            Tui::ZColor::fromTerminalColorIndexed(10),
            Tui::ZColor::fromTerminalColorIndexed(125)
        };

        int i = GENERATE(0, 9);
        int j = GENERATE(0, 9);

        if (i == j) {
            CHECK(colors[i] == colors[j]);
        } else {
            CHECK(colors[i] != colors[j]);
        }
    }

    SECTION("defaultColor") {
        Tui::ZColor color = Tui::ZColor::defaultColor();

        CHECK(color.colorType() == Tui::ZColor::Default);
        CHECK(color.nativeValue() == TERMPAINT_DEFAULT_COLOR);
        CHECK(color == Tui::ZColor());
    }

}

TEST_CASE("zcolor - color types") {
    Tui::ZColor color;

    SECTION("named") {
        int testCase = GENERATE(0, 15);
        color = Tui::ZColor::fromTerminalColor(testCase);
        CHECK(color.colorType() == Tui::ZColor::Terminal);
        CHECK(static_cast<int>(color.terminalColor()) == testCase);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_NAMED_COLOR + testCase));

        auto color2 = Tui::ZColor(static_cast<Tui::TerminalColor>(testCase));
        CHECK(color2.colorType() == Tui::ZColor::Terminal);
        CHECK(static_cast<int>(color2.terminalColor()) == testCase);
        CHECK(color2.nativeValue() == static_cast<uint32_t>(TERMPAINT_NAMED_COLOR + testCase));

        auto color3 = Tui::ZColor::fromTerminalColor(static_cast<Tui::TerminalColor>(testCase));
        CHECK(color3.colorType() == Tui::ZColor::Terminal);
        CHECK(static_cast<int>(color3.terminalColor()) == testCase);
        CHECK(color3.nativeValue() == static_cast<uint32_t>(TERMPAINT_NAMED_COLOR + testCase));
    }

    SECTION("indexed") {
        int testCase = GENERATE(0, 255);
        color = Tui::ZColor::fromTerminalColorIndexed(testCase);
        CHECK(color.colorType() == Tui::ZColor::TerminalIndexed);
        CHECK(color.terminalColorIndexed() == testCase);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_INDEXED_COLOR + testCase));
    }

    SECTION("rgb") {
        int r = GENERATE(0, 93, 97, 149, 189, 216, 232, 255);
        int g = GENERATE(0, 115, 117, 169, 189, 205, 241, 255);
        int b = GENERATE(0, 3, 60, 77, 99, 207, 216, 255);
        color = Tui::ZColor::fromRgb(r, g, b);
        CHECK(color.colorType() == Tui::ZColor::RGB);
        CHECK(color.red() == r);
        CHECK(color.green() == g);
        CHECK(color.blue() == b);
        CHECK(color.nativeValue() == TERMPAINT_RGB_COLOR(r, g, b));

        Tui::ZColor color2 = Tui::ZColor(r, g, b);
        CHECK(color.colorType() == Tui::ZColor::RGB);
        CHECK(color.red() == r);
        CHECK(color.green() == g);
        CHECK(color.blue() == b);

        CHECK(color == color2);
    }
}

