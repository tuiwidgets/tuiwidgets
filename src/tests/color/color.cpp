#include <Tui/ZColor.h>

#include <array>

#include <termpaint.h>

#include "../catchwrapper.h"

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

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::black);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BLACK));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::darkGray);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_DARK_GREY));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::lightGray);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_LIGHT_GREY));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightWhite);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_WHITE));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::blue);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BLUE));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::green);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_GREEN));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::cyan);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_CYAN));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::red);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_RED));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::magenta);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_MAGENTA));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::yellow);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_YELLOW));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightBlue);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_BLUE));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightGreen);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_GREEN));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightCyan);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_CYAN));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightRed);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_RED));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightMagenta);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_MAGENTA));

        color = Tui::ZColor::fromTerminalColor(Tui::TerminalColor::brightYellow);
        CHECK(color.nativeValue() == static_cast<uint32_t>(TERMPAINT_COLOR_BRIGHT_YELLOW));
    }

    SECTION("named-number") {
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
        CHECK(color.redOrGuess() == r);
        CHECK(color.green() == g);
        CHECK(color.greenOrGuess() == g);
        CHECK(color.blue() == b);
        CHECK(color.blueOrGuess() == b);
        CHECK(color.nativeValue() == TERMPAINT_RGB_COLOR(r, g, b));

        Tui::ZColor color2 = Tui::ZColor(r, g, b);
        CHECK(color.colorType() == Tui::ZColor::RGB);
        CHECK(color.red() == r);
        CHECK(color.redOrGuess() == r);
        CHECK(color.green() == g);
        CHECK(color.greenOrGuess() == g);
        CHECK(color.blue() == b);
        CHECK(color.blueOrGuess() == b);

        CHECK(color == color2);
    }
}

TEST_CASE("zcolor - guess rgb") {
    struct TestCase { int nr; int r, g, b; };
    auto testCase = GENERATE(
        TestCase{ 0, 0, 0, 0 }, TestCase{ 1, 0xaa, 0, 0 }, TestCase{ 2, 0, 0xaa, 0 }, TestCase{ 3, 0xaa, 0xaa, 0 },
        TestCase{ 4, 0, 0, 0xaa }, TestCase{ 5, 0xaa, 0, 0xaa }, TestCase{ 6, 0, 0xaa, 0xaa }, TestCase{ 7, 0xaa, 0xaa, 0xaa },

        TestCase{ 8, 0x55, 0x55, 0x55 }, TestCase{ 9, 0xff, 0x55, 0x55 }, TestCase{ 10, 0x55, 0xff, 0x55 }, TestCase{ 11, 0xff, 0xff, 0x55 },
        TestCase{ 12, 0x55, 0x55, 0xff }, TestCase{ 13, 0xff, 0x55, 0xff }, TestCase{ 14, 0x55, 0xff, 0xff }, TestCase{ 15, 0xff, 0xff, 0xff },

        TestCase{ 16, 0, 0, 0 }, TestCase{ 17, 0, 0, 95 }, TestCase{ 18, 0, 0, 135 }, TestCase{ 19, 0, 0, 175 },
        TestCase{ 20, 0, 0, 215 }, TestCase{ 21, 0, 0, 255 }, TestCase{ 22, 0, 95, 0 }, TestCase{ 23, 0, 95, 95 },
        TestCase{ 24, 0, 95, 135 }, TestCase{ 25, 0, 95, 175 }, TestCase{ 26, 0, 95, 215 }, TestCase{ 27, 0, 95, 255 },
        TestCase{ 28, 0, 135, 0 }, TestCase{ 29, 0, 135, 95 }, TestCase{ 30, 0, 135, 135 }, TestCase{ 31, 0, 135, 175 },
        TestCase{ 32, 0, 135, 215 }, TestCase{ 33, 0, 135, 255 }, TestCase{ 34, 0, 175, 0 }, TestCase{ 35, 0, 175, 95 },
        TestCase{ 36, 0, 175, 135 }, TestCase{ 37, 0, 175, 175 }, TestCase{ 38, 0, 175, 215 },
        TestCase{ 39, 0, 175, 255 }, TestCase{ 40, 0, 215, 0 }, TestCase{ 41, 0, 215, 95 }, TestCase{ 42, 0, 215, 135 },
        TestCase{ 43, 0, 215, 175 }, TestCase{ 44, 0, 215, 215 }, TestCase{ 45, 0, 215, 255 }, TestCase{ 46, 0, 255, 0 },
        TestCase{ 47, 0, 255, 95 }, TestCase{ 48, 0, 255, 135 }, TestCase{ 49, 0, 255, 175 },
        TestCase{ 50, 0, 255, 215 }, TestCase{ 51, 0, 255, 255 }, TestCase{ 52, 95, 0, 0 }, TestCase{ 53, 95, 0, 95 },
        TestCase{ 54, 95, 0, 135 }, TestCase{ 55, 95, 0, 175 }, TestCase{ 56, 95, 0, 215 }, TestCase{ 57, 95, 0, 255 },
        TestCase{ 58, 95, 95, 0 }, TestCase{ 59, 95, 95, 95 }, TestCase{ 60, 95, 95, 135 }, TestCase{ 61, 95, 95, 175 },
        TestCase{ 62, 95, 95, 215 }, TestCase{ 63, 95, 95, 255 }, TestCase{ 64, 95, 135, 0 },
        TestCase{ 65, 95, 135, 95 }, TestCase{ 66, 95, 135, 135 }, TestCase{ 67, 95, 135, 175 },
        TestCase{ 68, 95, 135, 215 }, TestCase{ 69, 95, 135, 255 }, TestCase{ 70, 95, 175, 0 },
        TestCase{ 71, 95, 175, 95 }, TestCase{ 72, 95, 175, 135 }, TestCase{ 73, 95, 175, 175 },
        TestCase{ 74, 95, 175, 215 }, TestCase{ 75, 95, 175, 255 }, TestCase{ 76, 95, 215, 0 },
        TestCase{ 77, 95, 215, 95 }, TestCase{ 78, 95, 215, 135 }, TestCase{ 79, 95, 215, 175 },
        TestCase{ 80, 95, 215, 215 }, TestCase{ 81, 95, 215, 255 }, TestCase{ 82, 95, 255, 0 },
        TestCase{ 83, 95, 255, 95 }, TestCase{ 84, 95, 255, 135 }, TestCase{ 85, 95, 255, 175 },
        TestCase{ 86, 95, 255, 215 }, TestCase{ 87, 95, 255, 255 }, TestCase{ 88, 135, 0, 0 },
        TestCase{ 89, 135, 0, 95 }, TestCase{ 90, 135, 0, 135 }, TestCase{ 91, 135, 0, 175 },
        TestCase{ 92, 135, 0, 215 }, TestCase{ 93, 135, 0, 255 }, TestCase{ 94, 135, 95, 0 },
        TestCase{ 95, 135, 95, 95 }, TestCase{ 96, 135, 95, 135 }, TestCase{ 97, 135, 95, 175 },
        TestCase{ 98, 135, 95, 215 }, TestCase{ 99, 135, 95, 255 }, TestCase{ 100, 135, 135, 0 },
        TestCase{ 101, 135, 135, 95 }, TestCase{ 102, 135, 135, 135 }, TestCase{ 103, 135, 135, 175 },
        TestCase{ 104, 135, 135, 215 }, TestCase{ 105, 135, 135, 255 }, TestCase{ 106, 135, 175, 0 },
        TestCase{ 107, 135, 175, 95 }, TestCase{ 108, 135, 175, 135 }, TestCase{ 109, 135, 175, 175 },
        TestCase{ 110, 135, 175, 215 }, TestCase{ 111, 135, 175, 255 }, TestCase{ 112, 135, 215, 0 },
        TestCase{ 113, 135, 215, 95 }, TestCase{ 114, 135, 215, 135 }, TestCase{ 115, 135, 215, 175 },
        TestCase{ 116, 135, 215, 215 }, TestCase{ 117, 135, 215, 255 }, TestCase{ 118, 135, 255, 0 },
        TestCase{ 119, 135, 255, 95 }, TestCase{ 120, 135, 255, 135 }, TestCase{ 121, 135, 255, 175 },
        TestCase{ 122, 135, 255, 215 }, TestCase{ 123, 135, 255, 255 }, TestCase{ 124, 175, 0, 0 },
        TestCase{ 125, 175, 0, 95 }, TestCase{ 126, 175, 0, 135 }, TestCase{ 127, 175, 0, 175 },
        TestCase{ 128, 175, 0, 215 }, TestCase{ 129, 175, 0, 255 }, TestCase{ 130, 175, 95, 0 },
        TestCase{ 131, 175, 95, 95 }, TestCase{ 132, 175, 95, 135 }, TestCase{ 133, 175, 95, 175 },
        TestCase{ 134, 175, 95, 215 }, TestCase{ 135, 175, 95, 255 }, TestCase{ 136, 175, 135, 0 },
        TestCase{ 137, 175, 135, 95 }, TestCase{ 138, 175, 135, 135 }, TestCase{ 139, 175, 135, 175 },
        TestCase{ 140, 175, 135, 215 }, TestCase{ 141, 175, 135, 255 }, TestCase{ 142, 175, 175, 0 },
        TestCase{ 143, 175, 175, 95 }, TestCase{ 144, 175, 175, 135 }, TestCase{ 145, 175, 175, 175 },
        TestCase{ 146, 175, 175, 215 }, TestCase{ 147, 175, 175, 255 }, TestCase{ 148, 175, 215, 0 },
        TestCase{ 149, 175, 215, 95 }, TestCase{ 150, 175, 215, 135 }, TestCase{ 151, 175, 215, 175 },
        TestCase{ 152, 175, 215, 215 }, TestCase{ 153, 175, 215, 255 }, TestCase{ 154, 175, 255, 0 },
        TestCase{ 155, 175, 255, 95 }, TestCase{ 156, 175, 255, 135 }, TestCase{ 157, 175, 255, 175 },
        TestCase{ 158, 175, 255, 215 }, TestCase{ 159, 175, 255, 255 }, TestCase{ 160, 215, 0, 0 },
        TestCase{ 161, 215, 0, 95 }, TestCase{ 162, 215, 0, 135 }, TestCase{ 163, 215, 0, 175 },
        TestCase{ 164, 215, 0, 215 }, TestCase{ 165, 215, 0, 255 }, TestCase{ 166, 215, 95, 0 },
        TestCase{ 167, 215, 95, 95 }, TestCase{ 168, 215, 95, 135 }, TestCase{ 169, 215, 95, 175 },
        TestCase{ 170, 215, 95, 215 }, TestCase{ 171, 215, 95, 255 }, TestCase{ 172, 215, 135, 0 },
        TestCase{ 173, 215, 135, 95 }, TestCase{ 174, 215, 135, 135 }, TestCase{ 175, 215, 135, 175 },
        TestCase{ 176, 215, 135, 215 }, TestCase{ 177, 215, 135, 255 }, TestCase{ 178, 215, 175, 0 },
        TestCase{ 179, 215, 175, 95 }, TestCase{ 180, 215, 175, 135 }, TestCase{ 181, 215, 175, 175 },
        TestCase{ 182, 215, 175, 215 }, TestCase{ 183, 215, 175, 255 }, TestCase{ 184, 215, 215, 0 },
        TestCase{ 185, 215, 215, 95 }, TestCase{ 186, 215, 215, 135 }, TestCase{ 187, 215, 215, 175 },
        TestCase{ 188, 215, 215, 215 }, TestCase{ 189, 215, 215, 255 }, TestCase{ 190, 215, 255, 0 },
        TestCase{ 191, 215, 255, 95 }, TestCase{ 192, 215, 255, 135 }, TestCase{ 193, 215, 255, 175 },
        TestCase{ 194, 215, 255, 215 }, TestCase{ 195, 215, 255, 255 }, TestCase{ 196, 255, 0, 0 },
        TestCase{ 197, 255, 0, 95 }, TestCase{ 198, 255, 0, 135 }, TestCase{ 199, 255, 0, 175 },
        TestCase{ 200, 255, 0, 215 }, TestCase{ 201, 255, 0, 255 }, TestCase{ 202, 255, 95, 0 },
        TestCase{ 203, 255, 95, 95 }, TestCase{ 204, 255, 95, 135 }, TestCase{ 205, 255, 95, 175 },
        TestCase{ 206, 255, 95, 215 }, TestCase{ 207, 255, 95, 255 }, TestCase{ 208, 255, 135, 0 },
        TestCase{ 209, 255, 135, 95 }, TestCase{ 210, 255, 135, 135 }, TestCase{ 211, 255, 135, 175 },
        TestCase{ 212, 255, 135, 215 }, TestCase{ 213, 255, 135, 255 }, TestCase{ 214, 255, 175, 0 },
        TestCase{ 215, 255, 175, 95 }, TestCase{ 216, 255, 175, 135 }, TestCase{ 217, 255, 175, 175 },
        TestCase{ 218, 255, 175, 215 }, TestCase{ 219, 255, 175, 255 }, TestCase{ 220, 255, 215, 0 },
        TestCase{ 221, 255, 215, 95 }, TestCase{ 222, 255, 215, 135 }, TestCase{ 223, 255, 215, 175 },
        TestCase{ 224, 255, 215, 215 }, TestCase{ 225, 255, 215, 255 }, TestCase{ 226, 255, 255, 0 },
        TestCase{ 227, 255, 255, 95 }, TestCase{ 228, 255, 255, 135 }, TestCase{ 229, 255, 255, 175 },
        TestCase{ 230, 255, 255, 215 }, TestCase{ 231, 255, 255, 255 },

        TestCase{ 232, 8, 8, 8 }, TestCase{ 233, 18, 18, 18 }, TestCase{ 234, 28, 28, 28 },
        TestCase{ 235, 38, 38, 38 }, TestCase{ 236, 48, 48, 48 }, TestCase{ 237, 58, 58, 58 },
        TestCase{ 238, 68, 68, 68 }, TestCase{ 239, 78, 78, 78 }, TestCase{ 240, 88, 88, 88 },
        TestCase{ 241, 98, 98, 98 }, TestCase{ 242, 108, 108, 108 }, TestCase{ 243, 118, 118, 118 },
        TestCase{ 244, 128, 128, 128 }, TestCase{ 245, 138, 138, 138 }, TestCase{ 246, 148, 148, 148 },
        TestCase{ 247, 158, 158, 158 }, TestCase{ 248, 168, 168, 168 }, TestCase{ 249, 178, 178, 178 },
        TestCase{ 250, 188, 188, 188 }, TestCase{ 251, 198, 198, 198 }, TestCase{ 252, 208, 208, 208 },
        TestCase{ 253, 218, 218, 218 }, TestCase{ 254, 228, 228, 228 }, TestCase{ 255, 238, 238, 238 }
     );

    Tui::ZColor color = Tui::ZColor::fromTerminalColorIndexed(testCase.nr);
    CHECK(color.redOrGuess() == testCase.r);
    CHECK(color.greenOrGuess() == testCase.g);
    CHECK(color.blueOrGuess() == testCase.b);

    if (testCase.nr < 16) {
        Tui::ZColor color = Tui::ZColor::fromTerminalColor(testCase.nr);
        CHECK(color.redOrGuess() == testCase.r);
        CHECK(color.greenOrGuess() == testCase.g);
        CHECK(color.blueOrGuess() == testCase.b);
    }
}


static double applyGamma(uint8_t val) {
    return pow(val / 255., 2.2);
}

TEST_CASE("zcolor - hsv") {

    SECTION("listed test cases") {
        struct TestCase {
            Tui::ZColor color;
            Tui::ZColorHSV hsv;
        };

        const int linear50 = pow(0.5, 1/2.2) * 255 + 0.5;
        CAPTURE(linear50);

        auto testCase = GENERATE_COPY(
                    TestCase{Tui::ZColor::fromRgb(0, 0, 0), Tui::ZColorHSV(0, 0, 0)},
                    TestCase{Tui::ZColor::fromRgb(255, 0, 0), Tui::ZColorHSV(0, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(255, 255, 0), Tui::ZColorHSV(60, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(0, 255, 0), Tui::ZColorHSV(120, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(0, 255, 255), Tui::ZColorHSV(180, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(0, 0, 255), Tui::ZColorHSV(240, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(255, 0, 255), Tui::ZColorHSV(300, 1, 1)},
                    TestCase{Tui::ZColor::fromRgb(255, 255, 255), Tui::ZColorHSV(0, 0, 1)},
                    TestCase{Tui::ZColor::fromRgb(127, 127, 127), Tui::ZColorHSV(0, 0, applyGamma(127))},
                    TestCase{Tui::ZColor::fromRgb(0xbf, 0xbf, 0), Tui::ZColorHSV(60, 1, applyGamma(0xbf))},
                    TestCase{Tui::ZColor::fromRgb(0x00, 0x80, 0x00), Tui::ZColorHSV(120, 1, applyGamma(0x80))},
                    TestCase{Tui::ZColor::fromRgb(linear50, 0xff, 0xff), Tui::ZColorHSV(180, 0.50, 1)},
                    TestCase{Tui::ZColor::fromRgb(linear50, linear50, 0xff), Tui::ZColorHSV(240, 0.50, 1)}
        );
        Tui::ZColor color = testCase.color;
        CAPTURE(color.red(), color.green(), color.blue());
        auto hsv = color.toHsv();
        REQUIRE(hsv.value() == Approx(testCase.hsv.value()).margin(5e-6));
        REQUIRE(hsv.saturation() == Approx(testCase.hsv.saturation()).margin(0.00296));
        CHECK(hsv.hue() == Approx(testCase.hsv.hue()).margin(0.258));

        auto color2 = Tui::ZColor::fromHsvStrict(hsv);
        CHECK(color2.red() == color.red());
        CHECK(color2.green() == color.green());
        CHECK(color2.blue() == color.blue());
    }

    SECTION("zcolor - grey value") {
        int testCase = GENERATE(0, 255);

        Tui::ZColor color = Tui::ZColor::fromRgb(testCase, testCase, testCase);
        CAPTURE(color.red(), color.green(), color.blue());
        auto hsv = color.toHsv();
        REQUIRE(hsv.saturation() == 0);
        CHECK(hsv.hue() == 0);
        REQUIRE(hsv.value() == pow(testCase / 255., 2.2));
    }
}

TEST_CASE("zcolor - hsv brute force", "[!hide]") {
    for (int r = 0; r < 256; r++) {
        for (int g = 0; g < 256; g++) {
            for (int b = 0; b < 256; b++) {
                Tui::ZColor color = Tui::ZColor::fromRgb(r, g, b);
                auto hsv = color.toHsv();
                auto color2 = Tui::ZColor::fromHsvStrict(hsv);
                if (std::max(abs(color2.red() - color.red()), std::max(abs(color2.blue() - color.blue()), abs(color2.green() - color.green()))) > 0) {
                    CAPTURE(color.red());
                    CAPTURE(color.green());
                    CAPTURE(color.blue());
                    CAPTURE(hsv.hue());
                    CAPTURE(hsv.saturation());
                    CAPTURE(hsv.value());
                    CAPTURE(color2.red());
                    CAPTURE(color2.green());
                    CAPTURE(color2.blue());
                    FAIL("ZColor->ZColorHSV->ZColor not identical");
                }
            }
        }
    }
}

/*
TEST_CASE("zcolor - hsv brute bench", "[!hide]") {

    BENCHMARK("fromHSV") {
        double sink = 0;
        for (int r = 0; r < 256; r++) {
            for (int g = 0; g < 256; g++) {
                for (int b = 0; b < 256; b++) {
                    Tui::ZColor color = Tui::ZColor::fromRgb(r, g, b);
                    auto hsv = color.toHsv();
                    //sink += hsv.hue();
                    auto color2 = Tui::ZColor::fromHsv(hsv);
                    sink += color2.red() + color2.green() + color2.blue();
                }
            }
        }
        return sink;
    };
    BENCHMARK("fromHSVStrict") {
        double sink = 0;
        for (int r = 0; r < 256; r++) {
            for (int g = 0; g < 256; g++) {
                for (int b = 0; b < 256; b++) {
                    Tui::ZColor color = Tui::ZColor::fromRgb(r, g, b);
                    auto hsv = color.toHsv();
                    //sink += hsv.hue();
                    auto color2 = Tui::ZColor::fromHsvStrict(hsv);
                    sink += color2.red() + color2.green() + color2.blue();
                }
            }
        }
        return sink;
    };

}
*/
