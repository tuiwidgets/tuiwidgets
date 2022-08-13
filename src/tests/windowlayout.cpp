// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWindowLayout.h>

#include "catchwrapper.h"

#include <math.h>

#include "Tui/ZTest.h"

#include "Testhelper.h"


#include <Tui/ZVBoxLayout.h>

TEST_CASE("windowlayout-base", "") {

    SECTION("defaults") {
        Tui::ZWindowLayout layout;
        CHECK(layout.sizeHint() == QSize(0, 0));
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(layout.isVisible() == true);
        CHECK(layout.isSpacer() == false);
        CHECK(layout.topBorderLeftAdjust() == 0);
        CHECK(layout.topBorderRightAdjust() == 0);
        CHECK(layout.rightBorderTopAdjust() == 0);
        CHECK(layout.rightBorderBottomAdjust() == 0);
        CHECK(layout.bottomBorderLeftAdjust() == 0);
        CHECK(layout.bottomBorderRightAdjust() == 0);
    }

    SECTION("topBorderLeftAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5, -6);
        layout.setTopBorderLeftAdjust(value);
        CHECK(layout.topBorderLeftAdjust() == value);
    }

    SECTION("topBorderRightAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5 , -6);
        layout.setTopBorderRightAdjust(value);
        CHECK(layout.topBorderRightAdjust() == value);
    }

    SECTION("rightBorderTopAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5 , -6);
        layout.setRightBorderTopAdjust(value);
        CHECK(layout.rightBorderTopAdjust() == value);
    }

    SECTION("rightBorderLeftAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5 , -6);
        layout.setRightBorderBottomAdjust(value);
        CHECK(layout.rightBorderBottomAdjust() == value);
    }

    SECTION("bottomBorderLeftAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5 , -6);
        layout.setBottomBorderLeftAdjust(value);
        CHECK(layout.bottomBorderLeftAdjust() == value);
    }

    SECTION("bottomBorderRightAdjust") {
        Tui::ZWindowLayout layout;
        int value = GENERATE(0, 1, -1, 5 , -6);
        layout.setBottomBorderRightAdjust(value);
        CHECK(layout.bottomBorderRightAdjust() == value);
    }

}

TEST_CASE("windowlayout-layout", "") {

    Tui::ZWindow win;
    Tui::ZWindowLayout layout;
    win.setLayout(&layout);

    // window layout has close interaction with ZWindow. Thus we check sizeHint() as modified by ZWindow not
    // directly as returned by the window layout.

    SECTION("default") {
        win.setOptions({});
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());

        CHECK(win.sizeHint() == QSize{2, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
    }

    SECTION("sizehint-noitems-closebtn") {
        auto edges = GENERATE(Qt::TopEdge | Qt::BottomEdge, Qt::TopEdge | Qt::BottomEdge | Qt::LeftEdge,
                              Qt::TopEdge | Qt::BottomEdge | Qt::RightEdge,
                              Qt::TopEdge | Qt::BottomEdge | Qt::LeftEdge | Qt::RightEdge);
        CAPTURE(edges);

        win.setOptions(Tui::ZWindow::Option::CloseButton);
        win.setBorderEdges(edges);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());

        CHECK(win.sizeHint() == QSize{6, 2});
    }

    SECTION("sizehint-noitems-closebtn-no-top-edge") {
        int leftEdge = GENERATE(0, 1);
        int rightEdge = GENERATE(0, 1);
        int bottomEdge = GENERATE(0, 1);
        Qt::Edges edges = {};
        if (leftEdge) edges |= Qt::LeftEdge;
        if (rightEdge) edges |= Qt::RightEdge;
        if (bottomEdge) edges |= Qt::BottomEdge;
        CAPTURE(edges);

        win.setOptions(Tui::ZWindow::Option::CloseButton);
        win.setBorderEdges(edges);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());

        CHECK(win.sizeHint() == QSize{leftEdge + rightEdge, bottomEdge});
    }


    SECTION("sizehint-noitems") {
        int leftEdge = GENERATE(0, 1);
        int rightEdge = GENERATE(0, 1);
        int topEdge = GENERATE(0, 1);
        int bottomEdge = GENERATE(0, 1);
        Qt::Edges edges = {};
        if (leftEdge) edges |= Qt::LeftEdge;
        if (rightEdge) edges |= Qt::RightEdge;
        if (topEdge) edges |= Qt::TopEdge;
        if (bottomEdge) edges |= Qt::BottomEdge;
        CAPTURE(edges);

        win.setOptions({});
        win.setBorderEdges(edges);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());

        CHECK(win.sizeHint() == QSize{leftEdge + rightEdge, topEdge + bottomEdge});
    }

    SECTION("central-widget-item") {
        int leftEdge = GENERATE(0, 1);
        int rightEdge = GENERATE(0, 1);
        int topEdge = GENERATE(0, 1);
        int bottomEdge = GENERATE(0, 1);
        Qt::Edges edges = {};
        if (leftEdge) edges |= Qt::LeftEdge;
        if (rightEdge) edges |= Qt::RightEdge;
        if (topEdge) edges |= Qt::TopEdge;
        if (bottomEdge) edges |= Qt::BottomEdge;
        CAPTURE(edges);
        win.setBorderEdges(edges);
        bool useItemSizeHint = GENERATE(false, true);
        CAPTURE(useItemSizeHint);

        win.setOptions({});
        StubLayout *item0 = new StubLayout();
        if (useItemSizeHint) {
            item0->stubSizeHint = {3, 5};
        }
        layout.setCentral(item0);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{(useItemSizeHint ? 3 : 0) + leftEdge + rightEdge,
                                         (useItemSizeHint ? 5 : 0) + topEdge + bottomEdge});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(item0->stubGeometry == QRect(leftEdge, topEdge, 16 - leftEdge - rightEdge,
                                           5 - topEdge - bottomEdge));
    }

    SECTION("central-widget-widget") {
        int leftEdge = GENERATE(0, 1);
        int rightEdge = GENERATE(0, 1);
        int topEdge = GENERATE(0, 1);
        int bottomEdge = GENERATE(0, 1);
        Qt::Edges edges = {};
        if (leftEdge) edges |= Qt::LeftEdge;
        if (rightEdge) edges |= Qt::RightEdge;
        if (topEdge) edges |= Qt::TopEdge;
        if (bottomEdge) edges |= Qt::BottomEdge;
        CAPTURE(edges);
        bool useItemSizeHint = GENERATE(false, true);
        CAPTURE(useItemSizeHint);

        win.setOptions({});
        win.setBorderEdges(edges);
        StubWidget widget;
        if (useItemSizeHint) {
            widget.stubSizeHint = {3, 5};
        }
        layout.setCentralWidget(&widget);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{(useItemSizeHint ? 3 : 0) + leftEdge + rightEdge,
                                         (useItemSizeHint ? 5 : 0) + topEdge + bottomEdge});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(leftEdge, topEdge, 16 - leftEdge - rightEdge,
                                         5 - topEdge - bottomEdge));
    }

    SECTION("top-widget-no-alignment") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        bool nonEmptySizeHint = GENERATE(false, true);
        CAPTURE(nonEmptySizeHint);

        win.setOptions({});
        StubWidget widget;
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        if (nonEmptySizeHint) {
            widget.stubSizeHint = {4, 1};
        }
        layout.setTopBorderWidget(&widget, {});
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        if (nonEmptySizeHint) {
            CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        } else {
            CHECK(win.sizeHint() == QSize{std::max(2 + leftAdjust + rightAdjust, 2), 2});
        }
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(1 + leftAdjust, 0, 14 - leftAdjust - rightAdjust, 1));
    }

    SECTION("top-widget-alignment-no-sizehint") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignHCenter, Qt::AlignLeft, Qt::AlignRight);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        layout.setTopBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{std::max(2 + leftAdjust + rightAdjust, 2), 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // Ignore left component of result, because it does not really make a difference with 0 width.
        CHECK(widget.geometry() == QRect(widget.geometry().left(), 0, 0, 1));
    }

    SECTION("top-widget-center") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignHCenter | Qt::AlignBottom, Qt::AlignHCenter, Qt::AlignCenter,
                                  Qt::AlignHCenter | Qt::AlignTop);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        layout.setTopBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // leftAdjust and rightAdjust move the center slightly
        int fudge = leftAdjust - ceil((leftAdjust + rightAdjust) / 2.);
        CAPTURE(fudge);
        CHECK(fudge <= 1);
        CHECK(fudge >= -1);
        CHECK(widget.geometry() == QRect(1 + 5 + fudge, 0, 4, 1));
    }

    SECTION("top-widget-closebtn-center") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignHCenter | Qt::AlignBottom, Qt::AlignHCenter, Qt::AlignCenter,
                                  Qt::AlignHCenter | Qt::AlignTop);
        CAPTURE(alignment);

        win.setOptions(Tui::ZWindow::Option::CloseButton);
        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        layout.setTopBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{6 + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // leftAdjust and rightAdjust move the center slightly
        int fudge = leftAdjust - ceil((leftAdjust + rightAdjust) / 2.);
        CAPTURE(fudge);
        CHECK(fudge <= 1);
        CHECK(fudge >= -1);
        CHECK(widget.geometry() == QRect(5 + 3 + fudge, 0, 4, 1));
    }

    SECTION("top-widget-left") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        bool useAlign = GENERATE(false, true);
        bool closeBtn = GENERATE(false, true);
        CAPTURE(closeBtn);
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        CAPTURE(useAlign);

        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        if (useAlign) {
            auto alignment = GENERATE(Qt::AlignLeft | Qt::AlignBottom, Qt::AlignLeft | Qt::AlignVCenter, Qt::AlignLeft,
                                      Qt::AlignLeft | Qt::AlignTop);
            layout.setTopBorderWidget(&widget, alignment);
        } else {
            layout.setTopBorderWidget(&widget, {});
            widget.setSizePolicyH(Tui::SizePolicy::Fixed);
        }
        if (closeBtn) {
            win.setOptions(Tui::ZWindow::Option::CloseButton);
        } else {
            win.setOptions({});
        }
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{(closeBtn ? 6 : 2) + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect((closeBtn ? 5 : 1) + leftAdjust, 0, 4, 1));
    }

    SECTION("top-widget-right") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignRight | Qt::AlignBottom, Qt::AlignRight | Qt::AlignVCenter, Qt::AlignRight,
                                  Qt::AlignRight | Qt::AlignTop);
        CAPTURE(alignment);
        bool closeBtn = GENERATE(false, true);
        CAPTURE(closeBtn);

        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        layout.setTopBorderWidget(&widget, alignment);
        if (closeBtn) {
            win.setOptions(Tui::ZWindow::Option::CloseButton);
        } else {
            win.setOptions({});
        }
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{(closeBtn ? 6 : 2) + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(1 + 10 - rightAdjust, 0, 4, 1));
    }

    SECTION("top-widget-closebtn-no-alignment") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);

        win.setOptions(Tui::ZWindow::Option::CloseButton);
        StubWidget widget;
        layout.setTopBorderLeftAdjust(leftAdjust);
        layout.setTopBorderRightAdjust(rightAdjust);
        layout.setTopBorderWidget(&widget, {});
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{std::max(6, 6 + leftAdjust + rightAdjust), 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(5 + leftAdjust, 0, 10 - leftAdjust - rightAdjust, 1));
    }


    SECTION("bottom-widget-no-alignment") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        bool nonEmptySizeHint = GENERATE(false, true);
        CAPTURE(nonEmptySizeHint);

        win.setOptions({});
        StubWidget widget;
        layout.setBottomBorderLeftAdjust(leftAdjust);
        layout.setBottomBorderRightAdjust(rightAdjust);
        if (nonEmptySizeHint) {
            widget.stubSizeHint = {4, 1};
        }
        layout.setBottomBorderWidget(&widget, {});
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        if (nonEmptySizeHint) {
            CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        } else {
            CHECK(win.sizeHint() == QSize{std::max(2 + leftAdjust + rightAdjust, 2), 2});
        }
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(1 + leftAdjust, 4, 14 - leftAdjust - rightAdjust, 1));
    }

    SECTION("bottom-widget-alignment-no-sizehint") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignHCenter, Qt::AlignLeft, Qt::AlignRight);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        layout.setBottomBorderLeftAdjust(leftAdjust);
        layout.setBottomBorderRightAdjust(rightAdjust);
        layout.setBottomBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{std::max(2 + leftAdjust + rightAdjust, 2), 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // Ignore left component of result, because it does not really make a difference with 0 width.
        CHECK(widget.geometry() == QRect(widget.geometry().left(), 4, 0, 1));
    }

    SECTION("bottom-widget-center") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignHCenter | Qt::AlignBottom, Qt::AlignHCenter, Qt::AlignCenter,
                                  Qt::AlignHCenter | Qt::AlignTop);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setBottomBorderLeftAdjust(leftAdjust);
        layout.setBottomBorderRightAdjust(rightAdjust);
        layout.setBottomBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // leftAdjust and rightAdjust move the center slightly
        int fudge = leftAdjust - ceil((leftAdjust + rightAdjust) / 2.);
        CAPTURE(fudge);
        CHECK(fudge <= 1);
        CHECK(fudge >= -1);
        CHECK(widget.geometry() == QRect(1 + 5 + fudge, 4, 4, 1));
    }

    SECTION("bottom-widget-left") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        bool useAlign = GENERATE(false, true);
        CAPTURE(useAlign);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setBottomBorderLeftAdjust(leftAdjust);
        layout.setBottomBorderRightAdjust(rightAdjust);
        if (useAlign) {
            auto alignment = GENERATE(Qt::AlignLeft | Qt::AlignBottom, Qt::AlignLeft | Qt::AlignVCenter, Qt::AlignLeft,
                                      Qt::AlignLeft | Qt::AlignTop);
            layout.setBottomBorderWidget(&widget, alignment);
        } else {
            layout.setBottomBorderWidget(&widget, {});
            widget.setSizePolicyH(Tui::SizePolicy::Fixed);
        }
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(1 + leftAdjust, 4, 4, 1));
    }

    SECTION("bottom-widget-right") {
        int leftAdjust = GENERATE(0, -1, 1);
        CAPTURE(leftAdjust);
        int rightAdjust = GENERATE(0, -1, 1);
        CAPTURE(rightAdjust);
        auto alignment = GENERATE(Qt::AlignRight | Qt::AlignBottom, Qt::AlignRight | Qt::AlignVCenter, Qt::AlignRight,
                                  Qt::AlignRight | Qt::AlignTop);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {4, 1};
        layout.setBottomBorderLeftAdjust(leftAdjust);
        layout.setBottomBorderRightAdjust(rightAdjust);
        layout.setBottomBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 16, 5});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2 + 4 + leftAdjust + rightAdjust, 2});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(1 + 10 - rightAdjust, 4, 4, 1));
    }


    SECTION("right-widget-no-alignment") {
        int topAdjust = GENERATE(0, -1, 1);
        CAPTURE(topAdjust);
        int bottomAdjust = GENERATE(0, -1, 1);
        CAPTURE(bottomAdjust);
        bool nonEmptySizeHint = GENERATE(false, true);
        CAPTURE(nonEmptySizeHint);

        win.setOptions({});
        StubWidget widget;
        layout.setRightBorderTopAdjust(topAdjust);
        layout.setRightBorderBottomAdjust(bottomAdjust);
        if (nonEmptySizeHint) {
            widget.stubSizeHint = {1, 4};
        }
        layout.setRightBorderWidget(&widget, {});
        win.setGeometry({0, 1, 10, 16});
        layout.setGeometry(win.layoutArea());
        if (nonEmptySizeHint) {
            CHECK(win.sizeHint() == QSize{2, 2 + 4 + topAdjust + bottomAdjust});
        } else {
            CHECK(win.sizeHint() == QSize{2, std::max(2 + topAdjust + bottomAdjust, 2)});
        }
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(9, 1 + topAdjust, 1, 14 - topAdjust - bottomAdjust));
    }

    SECTION("right-widget-alignment-no-sizehint") {
        int topAdjust = GENERATE(0, -1, 1);
        CAPTURE(topAdjust);
        int bottomAdjust = GENERATE(0, -1, 1);
        CAPTURE(bottomAdjust);
        auto alignment = GENERATE(Qt::AlignVCenter, Qt::AlignTop, Qt::AlignBottom);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        layout.setRightBorderTopAdjust(topAdjust);
        layout.setRightBorderBottomAdjust(bottomAdjust);
        layout.setRightBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 10, 16});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2, std::max(2 + topAdjust + bottomAdjust, 2)});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // Ignore left component of result, because it does not really make a difference with 0 width.
        CHECK(widget.geometry() == QRect(9, widget.geometry().top(), 1, 0));
    }

    SECTION("right-widget-center") {
        int topAdjust = GENERATE(0, -1, 1);
        CAPTURE(topAdjust);
        int bottomAdjust = GENERATE(0, -1, 1);
        CAPTURE(bottomAdjust);
        auto alignment = GENERATE(Qt::AlignVCenter | Qt::AlignRight, Qt::AlignVCenter, Qt::AlignCenter,
                                  Qt::AlignVCenter | Qt::AlignLeft);
        CAPTURE(alignment);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {1, 4};
        layout.setRightBorderTopAdjust(topAdjust);
        layout.setRightBorderBottomAdjust(bottomAdjust);
        layout.setRightBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 10, 16});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2, 2 + 4 + topAdjust + bottomAdjust});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        // topAdjust and bottomAdjust move the center slightly
        int fudge = topAdjust - ceil((topAdjust + bottomAdjust) / 2.);
        CAPTURE(fudge);
        CHECK(fudge <= 1);
        CHECK(fudge >= -1);
        CHECK(widget.geometry() == QRect(9, 1 + 5 + fudge, 1, 4));
    }

    SECTION("right-widget-top") {
        int topAdjust = GENERATE(0, -1, 1);
        CAPTURE(topAdjust);
        int bottomAdjust = GENERATE(0, -1, 1);
        CAPTURE(bottomAdjust);
        bool useAlign = GENERATE(false, true);
        CAPTURE(useAlign);

        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {1, 4};
        layout.setRightBorderTopAdjust(topAdjust);
        layout.setRightBorderBottomAdjust(bottomAdjust);
        if (useAlign) {
            auto alignment = GENERATE(Qt::AlignTop | Qt::AlignRight, Qt::AlignTop | Qt::AlignHCenter, Qt::AlignTop,
                                      Qt::AlignTop | Qt::AlignLeft);
            layout.setRightBorderWidget(&widget, alignment);
        } else {
            layout.setRightBorderWidget(&widget, {});
            widget.setSizePolicyV(Tui::SizePolicy::Fixed);
        }
        win.setGeometry({0, 1, 10, 16});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2, 2 + 4 + topAdjust + bottomAdjust});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(9, 1 + topAdjust, 1, 4));
    }

    SECTION("right-widget-bottom") {
        int topAdjust = GENERATE(0, -1, 1);
        CAPTURE(topAdjust);
        int bottomAdjust = GENERATE(0, -1, 1);
        CAPTURE(bottomAdjust);
        auto alignment = GENERATE(Qt::AlignBottom | Qt::AlignRight, Qt::AlignBottom | Qt::AlignHCenter, Qt::AlignBottom,
                                  Qt::AlignBottom | Qt::AlignLeft);
        CAPTURE(alignment);
        win.setOptions({});
        StubWidget widget;
        widget.stubSizeHint = {1, 4};
        layout.setRightBorderTopAdjust(topAdjust);
        layout.setRightBorderBottomAdjust(bottomAdjust);
        layout.setRightBorderWidget(&widget, alignment);
        win.setGeometry({0, 1, 10, 16});
        layout.setGeometry(win.layoutArea());
        CHECK(win.sizeHint() == QSize{2, 2 + 4 + topAdjust + bottomAdjust});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Preferred);
        CHECK(widget.geometry() == QRect(9, 1 + 10 - bottomAdjust, 1, 4));
    }

}

TEST_CASE("windowlayout-nested-layout", "") {

    Tui::ZWindow win;

    Tui::ZVBoxLayout box;
    Tui::ZWindowLayout *layout = new Tui::ZWindowLayout();
    win.setLayout(&box);
    box.add(layout);

    StubWidget widget;
    widget.stubSizeHint = {1, 4};
    layout->setCentralWidget(&widget);

    StubWidget widgetT;
    widgetT.stubSizeHint = {1, 4};
    layout->setTopBorderWidget(&widgetT);

    StubWidget widgetR;
    widgetR.stubSizeHint = {1, 4};
    layout->setTopBorderWidget(&widgetR);

    StubWidget widgetB;
    widgetB.stubSizeHint = {1, 4};
    layout->setTopBorderWidget(&widgetB);

    win.setGeometry({0, 1, 10, 16});
    layout->setGeometry(win.layoutArea());
    CHECK(widget.geometry() == QRect(1, 1, 8, 14));

    CHECK(widgetT.geometry() == QRect(0, 0, 0, 0));
    CHECK(widgetR.geometry() == QRect(0, 0, 0, 0));
    CHECK(widgetB.geometry() == QRect(0, 0, 0, 0));
}
