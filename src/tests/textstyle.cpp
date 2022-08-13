#include <Tui/ZTextStyle.h>

#include "catchwrapper.h"

TEST_CASE("ZTextStyle") {

    auto setterTests = [] (Tui::ZTextStyle &style) {
        auto prevFg = style.foregroundColor();
        auto prevBg = style.backgroundColor();
        auto prevAttr = style.attributes();

        SECTION("setForeground") {
            style.setForegroundColor(Tui::ZColor(255, 34, 23));
            CHECK(style.foregroundColor() == Tui::ZColor(255, 34, 23));
            CHECK(style.backgroundColor() == prevBg);
            CHECK(style.attributes() == prevAttr);
        }

        SECTION("setBackground") {
            style.setBackgroundColor(Tui::ZColor(128, 15, 230));
            CHECK(style.foregroundColor() == prevFg);
            CHECK(style.backgroundColor() == Tui::ZColor(128, 15, 230));
            CHECK(style.attributes() == prevAttr);
        }

        SECTION("setAttributes") {
            Tui::ZTextAttributes expectedAttributes = Tui::ZTextAttribute::Strike | Tui::ZTextAttribute::Inverse;
            style.setAttributes(expectedAttributes);
            CHECK(style.foregroundColor() == prevFg);
            CHECK(style.backgroundColor() == prevBg);
            CHECK(style.attributes() == expectedAttributes);
        }
    };

    SECTION("defaults") {
        Tui::ZTextStyle style;
        CHECK(style.foregroundColor() == Tui::ZColor::defaultColor());
        CHECK(style.backgroundColor() == Tui::ZColor::defaultColor());
        CHECK(style.attributes() == Tui::ZTextAttributes{});

        setterTests(style);
    }

    SECTION("constructor fg, bg") {
        Tui::ZTextStyle style{Tui::Colors::red, Tui::Colors::blue};
        CHECK(style.foregroundColor() == Tui::Colors::red);
        CHECK(style.backgroundColor() == Tui::Colors::blue);
        CHECK(style.attributes() == Tui::ZTextAttributes{});

        setterTests(style);
    }

    SECTION("constructor fg, bg, attr") {
        Tui::ZTextAttributes expectedAttributes = Tui::ZTextAttribute::Bold;
        Tui::ZTextStyle style{Tui::Colors::red, Tui::Colors::blue, expectedAttributes};
        CHECK(style.foregroundColor() == Tui::Colors::red);
        CHECK(style.backgroundColor() == Tui::Colors::blue);
        CHECK(style.attributes() == expectedAttributes);

        setterTests(style);
    }

    SECTION("equals") {
        Tui::ZTextStyle style1{Tui::Colors::red, Tui::Colors::blue, Tui::ZTextAttribute::Bold};
        Tui::ZTextStyle style2{Tui::Colors::red, Tui::Colors::blue, Tui::ZTextAttribute::Bold};

        CHECK(style1 == style2);
        CHECK_FALSE(style1 != style2);

        SECTION("different fg") {
            style1.setForegroundColor(Tui::Colors::black);
            CHECK_FALSE(style1 == style2);
            CHECK(style1 != style2);
        }

        SECTION("different bg") {
            style1.setBackgroundColor(Tui::Colors::black);
            CHECK_FALSE(style1 == style2);
            CHECK(style1 != style2);
        }

        SECTION("different attr") {
            style1.setAttributes(Tui::ZTextAttribute::UnderlineDouble);
            CHECK_FALSE(style1 == style2);
            CHECK(style1 != style2);
        }

    }

}
