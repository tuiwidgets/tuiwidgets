#include <Tui/ZFormatRange.h>

#include "../catchwrapper.h"

TEST_CASE("formatrange", "") {
    const Tui::ZTextStyle style1{Tui::Color::red, Tui::Color::yellow, Tui::ZTextAttribute::Bold};
    const Tui::ZTextStyle style2{Tui::Color::magenta, Tui::Color::darkGray, Tui::ZTextAttribute::Italic};
    const Tui::ZTextStyle style3{Tui::Color::green, Tui::Color::brightCyan, Tui::ZTextAttribute::Strike};

    SECTION("constructor") {
        Tui::ZFormatRange range;
        CHECK(range.start() == 0);
        CHECK(range.length() == 0);
        CHECK(range.format() == Tui::ZTextStyle{});
        CHECK(range.formattingChar() == Tui::ZTextStyle{});
        CHECK(range.userData() == 0);
    }

    SECTION("constructor-with-values") {
        Tui::ZFormatRange range{10, 23, style1, style2};
        CHECK(range.start() == 10);
        CHECK(range.length() == 23);
        CHECK(range.format() == style1);
        CHECK(range.formattingChar() == style2);
        CHECK(range.userData() == 0);
    }

    SECTION("constructor-with-all-values") {
        Tui::ZFormatRange range{10, 23, style1, style2, 2134};
        CHECK(range.start() == 10);
        CHECK(range.length() == 23);
        CHECK(range.format() == style1);
        CHECK(range.formattingChar() == style2);
        CHECK(range.userData() == 2134);
    }

    SECTION("setStart") {
        Tui::ZFormatRange range;
        CHECK(range.start() == 0);
        range.setStart(45);
        CHECK(range.start() == 45);
    }

    SECTION("setLength") {
        Tui::ZFormatRange range;
        CHECK(range.length() == 0);
        range.setLength(45);
        CHECK(range.length() == 45);
    }

    SECTION("setFormat") {
        Tui::ZFormatRange range;
        CHECK(range.format() == Tui::ZTextStyle{});
        range.setFormat(style3);
        CHECK(range.format() == style3);
    }

    SECTION("setFormattingChar") {
        Tui::ZFormatRange range;
        CHECK(range.formattingChar() == Tui::ZTextStyle{});
        range.setFormattingChar(style3);
        CHECK(range.formattingChar() == style3);
    }

    SECTION("setUserData") {
        Tui::ZFormatRange range;
        CHECK(range.userData() == 0);
        range.setUserData(45);
        CHECK(range.userData() == 45);
    }

    SECTION("operator=") {
        Tui::ZFormatRange range1{10, 23, style1, style2, 2134};
        Tui::ZFormatRange range2{30, 43, style2, style3, 6788};

        range1 = range2;
        CHECK(range1.start() == 30);
        CHECK(range1.length() == 43);
        CHECK(range1.format() == style2);
        CHECK(range1.formattingChar() == style3);
        CHECK(range1.userData() == 6788);

        CHECK(range2.start() == 30);
        CHECK(range2.length() == 43);
        CHECK(range2.format() == style2);
        CHECK(range2.formattingChar() == style3);
        CHECK(range2.userData() == 6788);
    }

    SECTION("operator= move") {
        Tui::ZFormatRange range1{10, 23, style1, style2, 2134};
        Tui::ZFormatRange range2{30, 43, style2, style3, 6788};

        range1 = std::move(range2);
        CHECK(range1.start() == 30);
        CHECK(range1.length() == 43);
        CHECK(range1.format() == style2);
        CHECK(range1.formattingChar() == style3);
        CHECK(range1.userData() == 6788);

        CHECK(range2.start() == 0);
        CHECK(range2.length() == 0);
        CHECK(range2.format() == Tui::ZTextStyle{});
        CHECK(range2.formattingChar() == Tui::ZTextStyle{});
        CHECK(range2.userData() == 0);
    }

    SECTION("copy constructor") {
        Tui::ZFormatRange range2{30, 43, style2, style3, 6788};

        Tui::ZFormatRange range1{range2};
        CHECK(range1.start() == 30);
        CHECK(range1.length() == 43);
        CHECK(range1.format() == style2);
        CHECK(range1.formattingChar() == style3);
        CHECK(range1.userData() == 6788);

        CHECK(range2.start() == 30);
        CHECK(range2.length() == 43);
        CHECK(range2.format() == style2);
        CHECK(range2.formattingChar() == style3);
        CHECK(range2.userData() == 6788);
    }

    SECTION("move constructor") {
        Tui::ZFormatRange range2{30, 43, style2, style3, 6788};

        Tui::ZFormatRange range1{std::move(range2)};
        CHECK(range1.start() == 30);
        CHECK(range1.length() == 43);
        CHECK(range1.format() == style2);
        CHECK(range1.formattingChar() == style3);
        CHECK(range1.userData() == 6788);

        CHECK(range2.start() == 0);
        CHECK(range2.length() == 0);
        CHECK(range2.format() == Tui::ZTextStyle{});
        CHECK(range2.formattingChar() == Tui::ZTextStyle{});
        CHECK(range2.userData() == 0);
    }

}
