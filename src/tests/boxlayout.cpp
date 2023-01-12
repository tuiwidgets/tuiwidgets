// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZHBoxLayout.h>
#include <Tui/ZVBoxLayout.h>

#include "catchwrapper.h"

#include <QPointer>
#include <QSet>

#include "Tui/ZTest.h"

#include "Testhelper.h"
#include "vcheck_zlayout.h"

namespace {

    struct RotV {
        QRect rect(int left, int top, int width, int height) {
            return {left, top, width, height};
        }

        QRect rect(QRect r) {
            return rect(r.left(), r.top(), r.width(), r.height());
        }

        QSize size(int width, int height) {
            return {width, height};
        }

        void setPolicyLayout(StubLayout *l, Tui::SizePolicy p) {
            l->stubSizePolicyV = p;
        }

        void setPolicyAcross(StubLayout *l, Tui::SizePolicy p) {
            l->stubSizePolicyH = p;
        }

        void setPolicyLayout(Tui::ZWidget &w, Tui::SizePolicy p) {
            w.setSizePolicyV(p);
        }

        void setPolicyAcross(Tui::ZWidget &w, Tui::SizePolicy p) {
            w.setSizePolicyH(p);
        }

        template <typename T>
        Tui::SizePolicy getPolicyLayout(T &layout) {
            return layout.sizePolicyV();
        }

        template <typename T>
        Tui::SizePolicy getPolicyAcross(T &layout) {
            return layout.sizePolicyH();
        }
    };

    struct RotH {
        QRect rect(int left, int top, int width, int height) {
            return {top, left, height, width};
        }

        QRect rect(QRect r) {
            return rect(r.left(), r.top(), r.width(), r.height());
        }

        QSize size(int width, int height) {
            return {height, width};
        }

        void setPolicyLayout(StubLayout *l, Tui::SizePolicy p) {
            l->stubSizePolicyH = p;
        }

        void setPolicyAcross(StubLayout *l, Tui::SizePolicy p) {
            l->stubSizePolicyV = p;
        }

        void setPolicyLayout(Tui::ZWidget &w, Tui::SizePolicy p) {
            w.setSizePolicyH(p);
        }

        void setPolicyAcross(Tui::ZWidget &w, Tui::SizePolicy p) {
            w.setSizePolicyV(p);
        }

        template <typename T>
        Tui::SizePolicy getPolicyLayout(T &layout) {
            return layout.sizePolicyH();
        }

        template <typename T>
        Tui::SizePolicy getPolicyAcross(T &layout) {
            return layout.sizePolicyV();
        }
    };
}

TEST_CASE("boxlayout-default", "") {

    auto tests = [] (auto &layout) {
        CHECK(layout.spacing() == 0);
        CHECK(layout.sizeHint() == QSize{0, 0});
        CHECK(layout.sizePolicyH() == Tui::SizePolicy::Fixed);
        CHECK(layout.sizePolicyV() == Tui::SizePolicy::Fixed);
        // empty layouts are invisible
        CHECK(layout.isVisible() == false);
        CHECK(layout.isSpacer() == false);

        SECTION("abi-vcheck") {
            ZLayoutBaseWrapper base;
            checkZLayoutOverrides(&base, &layout);
        }
    };

    SECTION("vbox") {
        Tui::ZVBoxLayout layout;
        tests(layout);
    }
    SECTION("hbox") {
        Tui::ZHBoxLayout layout;
        tests(layout);
    }

}

TEST_CASE("boxlayout-base", "") {
    // NOTE: To cover HBoxLayout and VBoxLayout with one set of tests, these tests are run once with rotated
    //       coordinates via `rotate` parameter which is either a RotH or a RotV instance.

    auto tests = [] (auto &layout, auto &rotate) {
        SECTION("empty") {
            layout.setGeometry(rotate.rect(0, 1, 10, 3));
        }

        SECTION("empty-stretch") {
            layout.addStretch();
            layout.setGeometry(rotate.rect(0, 1, 10, 3));
        }

        SECTION("empty-stretch4") {
            layout.addStretch();
            layout.addStretch();
            layout.addStretch();
            layout.addStretch();
            layout.setGeometry(rotate.rect(0, 1, 10, 3));
        }

        SECTION("empty-spacing") {
            layout.setSpacing(2);
            layout.setGeometry(rotate.rect(0, 1, 10, 3));
        }

        SECTION("empty-spacing4") {
            layout.setSpacing(4);
            layout.setGeometry(rotate.rect(0, 1, 10, 3));
        }

        SECTION("single") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            if (p1 == Tui::SizePolicy::Fixed || p1 == Tui::SizePolicy::Maximum) {
                item0->stubSizeHint = rotate.size(1, 6);
            }
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 6));
        }

        SECTION("single-nonexpanding0") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 0, 0));
        }

        SECTION("single-nonexpanding2") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubSizeHint = rotate.size(1, 2);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 2));
        }

        SECTION("single-expanding") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Minimum, Tui::SizePolicy::Expanding,
                                          Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            bool withSizeHint = GENERATE(true, false);
            CAPTURE(withSizeHint);
            if (withSizeHint) {
                item0->stubSizeHint = rotate.size(1, 2);
            }
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 6));
        }

        SECTION("single-shrink") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Maximum, Tui::SizePolicy::Expanding,
                                          Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubSizeHint = rotate.size(1, 12);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 6));
        }

        SECTION("single-noshrink") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Minimum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubSizeHint = rotate.size(1, 12);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 12));
        }

        SECTION("single-relayout") {
            layout.setGeometry(rotate.rect(0, 1, 1, 12));
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            CHECK(item0->stubGeometry == rotate.rect(0, 0, 0, 0));
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 12));
        }

        SECTION("single-relayout-10") {
            layout.setGeometry(rotate.rect(0, 1, 1, 12));
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            CHECK(item0->stubGeometry == rotate.rect(0, 0, 0, 0));
            layout.setGeometry(rotate.rect(0, 1, 3, 10));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 3, 10));
        }

        SECTION("single-stretch-before") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubSizeHint = rotate.size(1, 2);
            layout.addStretch();
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            if (p1 == Tui::SizePolicy::Expanding) {
                CHECK(item0->stubGeometry == rotate.rect(0, 3, 1, 4));
            } else {
                CHECK(item0->stubGeometry == rotate.rect(0, 5, 1, 2));
            }
        }

        SECTION("single-stretch-after") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubSizeHint = rotate.size(1, 2);
            layout.add(item0);
            layout.addStretch();
            layout.setGeometry(rotate.rect(0, 1, 1, 6));
            if (p1 == Tui::SizePolicy::Expanding) {
                CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 4));
            } else {
                CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 2));
            }
        }

        SECTION("single-spacing") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            int spacing = GENERATE(-1, 0, 1);
            CAPTURE(spacing);
            if (spacing >= 0) {
                layout.setSpacing(spacing);
            }
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(1, 6);
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 1, 10));
            if (p1 == Tui::SizePolicy::Fixed || p1 == Tui::SizePolicy::Maximum) {
                CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 6));
            } else {
                CHECK(item0->stubGeometry == rotate.rect(0, 1, 1, 10));
            }
        }

        SECTION("single-across-fixed") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyAcross(item0, p1);
            item0->stubSizeHint = rotate.size(2, 6);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 7, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 2, 6));
        }

        SECTION("single-across-expanding") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Minimum, Tui::SizePolicy::Expanding,
                                          Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyAcross(item0, p1);
            item0->stubSizeHint = rotate.size(2, 6);
            layout.add(item0);
            layout.setGeometry(rotate.rect(0, 1, 7, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 7, 6));
        }


        SECTION("double") {
            struct TestCase { int sourceLine; int size; Tui::SizePolicy p1; int sizeHint1; Tui::SizePolicy p2; int sizeHint2; QRect item0; QRect item1; };
            auto testCase = GENERATE(
                        TestCase{__LINE__, 11, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 5}, QRect{0, 6, 1, 6}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},
                        TestCase{__LINE__, 11, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 5}},

                        TestCase{__LINE__, 12, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 12, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},

                        TestCase{__LINE__, 13, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Fixed,     6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Maximum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Minimum,   6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Expanding, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Maximum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Minimum,   6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 6}, QRect{0, 7, 1, 7}},
                        TestCase{__LINE__, 13, Tui::SizePolicy::Preferred, 6, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 7}, QRect{0, 8, 1, 6}},

                        TestCase{__LINE__, 20, Tui::SizePolicy::Expanding, 6,  Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 10}, QRect{0, 11, 1, 10}},
                        TestCase{__LINE__, 20, Tui::SizePolicy::Expanding, 10, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 12}, QRect{0, 13, 1, 8}},
                        TestCase{__LINE__, 20, Tui::SizePolicy::Preferred, 10, Tui::SizePolicy::Expanding, 6, QRect{0, 1, 1, 10}, QRect{0, 11, 1, 10}},
                        TestCase{__LINE__, 20, Tui::SizePolicy::Preferred, 6,  Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 10}, QRect{0, 11, 1, 10}},
                        TestCase{__LINE__, 20, Tui::SizePolicy::Preferred, 10, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 12}, QRect{0, 13, 1, 8}},
                        TestCase{__LINE__, 10, Tui::SizePolicy::Preferred, 10, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 6},  QRect{0, 7, 1, 4}},
                        TestCase{__LINE__, 20, Tui::SizePolicy::Minimum,   10, Tui::SizePolicy::Preferred, 6, QRect{0, 1, 1, 12}, QRect{0, 13, 1, 8}},
                        TestCase{__LINE__, 5, Tui::SizePolicy::Minimum,    6,  Tui::SizePolicy::Fixed,     6, QRect{0, 1, 1, 6},  QRect{0, 7, 1, 6}}
                        );

            CAPTURE(testCase.sourceLine);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(1, testCase.sizeHint1);
            rotate.setPolicyLayout(item0, testCase.p1);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(1, testCase.sizeHint2);
            rotate.setPolicyLayout(item1, testCase.p2);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 1, testCase.size));

            CHECK(item0->stubGeometry == rotate.rect(testCase.item0));
            CHECK(item1->stubGeometry == rotate.rect(testCase.item1));
        }

        SECTION("double-visible") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);
            layout.setSpacing(2);
            std::unique_ptr<StubWidget> item0 = std::make_unique<StubWidget>();
            item0->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyAcross(*item0, p1);
            rotate.setPolicyLayout(*item0, p1);
            item0->setVisible(false);
            layout.addWidget(item0.get());
            std::unique_ptr<StubWidget> item1 = std::make_unique<StubWidget>();
            item1->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyAcross(*item1, p2);
            rotate.setPolicyLayout(*item1, p2);
            layout.addWidget(item1.get());
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->geometry() == rotate.rect(0, 0, 0, 0));
            if (p2 == Tui::SizePolicy::Fixed || p2 == Tui::SizePolicy::Maximum) {
                CHECK(item1->geometry() == rotate.rect(0, 1, 10, 5));
            } else {
                CHECK(item1->geometry() == rotate.rect(0, 1, 10, 12));
            }
        }

        SECTION("double-widget") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);
            layout.setSpacing(2);
            std::unique_ptr<StubWidget> item0 = std::make_unique<StubWidget>();
            item0->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyAcross(*item0, p1);
            rotate.setPolicyLayout(*item0, p1);

            layout.addWidget(item0.get());
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyLayout(item1, p2);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->geometry() == rotate.rect(0, 1, 10, 5));
            CHECK(item1->stubGeometry == rotate.rect(0, 8, 10, 5));
        }

        SECTION("double-setspacing") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);
            layout.setSpacing(2);
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyLayout(item1, p2);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 5));
            CHECK(item1->stubGeometry == rotate.rect(0, 8, 10, 5));
        }

        SECTION("double-addspacing") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);
            layout.addSpacing(2);
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(10, 5);
            rotate.setPolicyLayout(item1, p2);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 3, 10, 5));
            CHECK(item1->stubGeometry == rotate.rect(0, 8, 10, 5));
        }

        SECTION("double-stretch") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(10, 2);
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            layout.addStretch();
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            item1->stubSizeHint = rotate.size(10, 2);
            rotate.setPolicyLayout(item1, p2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            if (p1 != Tui::SizePolicy::Expanding) {
                if (p2 != Tui::SizePolicy::Expanding) {
                    // only stretch grows
                    CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 2));
                    CHECK(item1->stubGeometry == rotate.rect(0, 11, 10, 2));
                } else {
                    // p2 and stretch grow
                    CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 2));
                    CHECK(item1->stubGeometry == rotate.rect(0, 7, 10, 6));
                }
            } else {
                if (p2 == Tui::SizePolicy::Fixed || p2 == Tui::SizePolicy::Maximum
                    || p2 == Tui::SizePolicy::Minimum || p2 == Tui::SizePolicy::Preferred) {
                    // p1 and stretch grow
                    CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 6));
                    CHECK(item1->stubGeometry == rotate.rect(0, 11, 10, 2));
                } else {
                    // p1, p2 and stretch grow
                    CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 5));
                    CHECK(item1->stubGeometry == rotate.rect(0, 9, 10, 4));
                }
            }
        }

        SECTION("double-expanding") {
            // test that expanding takes available space and minium and preferred do not grow.
            int size = GENERATE(0, 1, 2, 3);
            Tui::SizePolicy sizePolicyItem1 = GENERATE(Tui::SizePolicy::Minimum, Tui::SizePolicy::Preferred);
            CAPTURE(size);
            CAPTURE(sizePolicyItem1);
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(3, 5);
            rotate.setPolicyLayout(item0, Tui::SizePolicy::Expanding);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(2, 5);
            rotate.setPolicyLayout(item1, sizePolicyItem1);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 6, 10 + size));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 6, 5 + size));
            CHECK(item1->stubGeometry == rotate.rect(0, 6 + size, 6, 5));
        }

        SECTION("double-across-fixed") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyAcross(item0, p1);
            item0->stubSizeHint = rotate.size(2, 3);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyAcross(item1, p1);
            item1->stubSizeHint = rotate.size(2, 3);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 2, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 4, 2, 3));
        }

        SECTION("double-across-expanding") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Minimum, Tui::SizePolicy::Expanding,
                                          Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyAcross(item0, p1);
            item0->stubSizeHint = rotate.size(2, 3);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyAcross(item1, p1);
            item1->stubSizeHint = rotate.size(2, 3);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 6));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 4, 10, 3));
        }

        SECTION("double-growing-space-distribution") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            struct TestCase { int sourceLine; int size; int spacing; QRect item0; QRect item1; };
            auto testCase = GENERATE(
                                    TestCase{__LINE__, -1, 0, QRect{0, 1, 10, 5}, QRect{0, 6, 10, 4}},
                                    TestCase{__LINE__,  0, 0, QRect{0, 1, 10, 5}, QRect{0, 6, 10, 5}},
                                    TestCase{__LINE__,  1, 0, QRect{0, 1, 10, 6}, QRect{0, 7, 10, 5}},
                                    TestCase{__LINE__,  2, 0, QRect{0, 1, 10, 6}, QRect{0, 7, 10, 6}},
                                    TestCase{__LINE__,  3, 0, QRect{0, 1, 10, 7}, QRect{0, 8, 10, 6}},

                                    TestCase{__LINE__,  5, 2, QRect{0, 1, 10, 7}, QRect{0, 10, 10, 6}},
                                    TestCase{__LINE__,  4, 2, QRect{0, 1, 10, 6}, QRect{0, 9, 10, 6}},
                                    TestCase{__LINE__,  3, 2, QRect{0, 1, 10, 6}, QRect{0, 9, 10, 5}},
                                    TestCase{__LINE__,  2, 2, QRect{0, 1, 10, 5}, QRect{0, 8, 10, 5}},
                                    TestCase{__LINE__,  1, 2, QRect{0, 1, 10, 5}, QRect{0, 7, 10, 5}},
                                    TestCase{__LINE__,  0, 2, QRect{0, 1, 10, 5}, QRect{0, 6, 10, 5}},
                                    TestCase{__LINE__,  -1, 2, QRect{0, 1, 10, 5}, QRect{0, 6, 10, 4}},
                                    TestCase{__LINE__,  -2, 2, QRect{0, 1, 10, 4}, QRect{0, 5, 10, 4}}
                        );

            CAPTURE(testCase.sourceLine);
            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(5, 5);
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            layout.setSpacing(testCase.spacing);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(5, 5);
            rotate.setPolicyLayout(item1, p1);
            layout.add(item1);
            layout.setGeometry(rotate.rect(0, 1, 10, 10 + testCase.size));
            CHECK(item0->stubGeometry == rotate.rect(testCase.item0));
            CHECK(item1->stubGeometry == rotate.rect(testCase.item1));
        }


        SECTION("triple") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 4));
            CHECK(item1->stubGeometry == rotate.rect(0, 5, 10, 4));
            CHECK(item2->stubGeometry == rotate.rect(0, 9, 10, 4));
        }

        SECTION("triple-spacing") {
            layout.setSpacing(2);
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 6, 10, 3));
            CHECK(item2->stubGeometry == rotate.rect(0, 11, 10, 2));
        }

        SECTION("triple-stretch1") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            layout.addStretch();
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 7, 10, 3));
            CHECK(item2->stubGeometry == rotate.rect(0, 10, 10, 3));
        }

        SECTION("triple-stretch1-1") {
            layout.addStretch(); //3
            layout.addStretch(); //3
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 7, 10, 2));
            CHECK(item1->stubGeometry == rotate.rect(0, 9, 10, 2));
            CHECK(item2->stubGeometry == rotate.rect(0, 11, 10, 2));
        }

        SECTION("triple-stretch1-2") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            layout.addStretch(); // 3
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            layout.addStretch(); // 2
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 7, 10, 2));
            CHECK(item2->stubGeometry == rotate.rect(0, 11, 10, 2));
        }

        SECTION("triple-stretch1-3") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            layout.addStretch(); // 2
            StubLayout *item2 = new StubLayout();
            layout.add(item2);
            layout.addStretch(); // 2
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 3));
            CHECK(item1->stubGeometry == rotate.rect(0, 4, 10, 3));
            CHECK(item2->stubGeometry == rotate.rect(0, 9, 10, 2));
        }


        SECTION("stretch-vs-spacing") {
            int i = GENERATE(-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
            CAPTURE(i);
            layout.setSpacing(i);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, Tui::SizePolicy::Fixed);
            layout.add(item0);
            layout.addStretch();
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, Tui::SizePolicy::Fixed);
            layout.add(item1);
            item0->stubSizeHint = rotate.size(2, 5);
            item1->stubSizeHint = rotate.size(7, 5);
            layout.setGeometry(rotate.rect(0, 1, 10, 12));
            CHECK(item0->stubGeometry == rotate.rect(0, 1, 10, 5));
            CHECK(item1->stubGeometry == rotate.rect(0, 8, 10, 5));
        }
    };

    SECTION("vbox") {
        Tui::ZVBoxLayout layout;
        RotV rotate;
        tests(layout, rotate);
    }
    SECTION("hbox") {
        Tui::ZHBoxLayout layout;
        RotH rotate;
        tests(layout, rotate);
    }
}

TEST_CASE("boxlayout does-not-crash", "[.slow]") {
    // NOTE: To cover HBoxLayout and VBoxLayout with one set of tests, these tests are run once with rotated
    //       coordinates via `rotate` parameter which is either a RotH or a RotV instance.

    auto tests = [] (auto &layout, auto &rotate) {
        SECTION("does-not-crash") {
            int b = GENERATE(true, false);
            CAPTURE(b);
            int b2 = GENERATE(true, false);
            CAPTURE(b2);
            int b3 = GENERATE(true, false);
            CAPTURE(b3);
            int i = GENERATE(-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
            CAPTURE(i);
            int i2 = GENERATE(-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
            CAPTURE(i2);
            int i3 = GENERATE(-1, 0, 1, 2, 3, 12);
            CAPTURE(i3);
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            Tui::SizePolicy p2 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p2);

            layout.setSpacing(i2);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            item0->stubIsSpacer = b2;
            item0->stubIsVisible = b3;
            layout.add(item0);
            layout.addStretch();
            item0->stubIsVisible = b;
            std::unique_ptr<StubWidget> item1 = std::make_unique<StubWidget>();
            rotate.setPolicyLayout(*item1, p2);
            layout.addWidget(item1.get());
            item0->stubSizeHint = rotate.size(i, i);
            item1->stubSizeHint = rotate.size(i3, i2);
            layout.addSpacing(i);
            layout.setGeometry(rotate.rect(0, 1, i2, i3));
        }
    };

    SECTION("vbox") {
        Tui::ZVBoxLayout layout;
        RotV rotate;
        tests(layout, rotate);
    }
    SECTION("hbox") {
        Tui::ZHBoxLayout layout;
        RotH rotate;
        tests(layout, rotate);
    }
}

TEST_CASE("boxlayout-terminal-interaction", "") {
    Testhelper t("unsued", "unused", 16, 5);
    Tui::ZWidget root;
    t.terminal->setMainWidget(&root);

    Tui::ZTest::withLayoutRequestTracking(t.terminal.get(), [&](QSet<Tui::ZWidget*> *layoutRequested) {
        CHECK(layoutRequested->size() == 0);

        auto checkAndConsumeRelayout = [&] {
            bool ok = (layoutRequested->size() == 1);
            ok &= (*layoutRequested->begin() == &root);
            if (ok) {
                layoutRequested->clear();
            }
            return ok;
        };

        auto tests = [&](auto &layout) {
            SECTION("add layout") {
                // Also checks that setGeometry does not request relayouts. It's used while doing layouts.
                CHECK(layoutRequested->size() == 0);
                root.setLayout(&layout);
                CHECK(checkAndConsumeRelayout());
                layout.setGeometry({0, 1, 10, 12});
                StubLayout *item0 = new StubLayout();
                CHECK(layoutRequested->size() == 0);
                layout.add(item0);
                CHECK(item0->stubGeometry == QRect{0, 0, 0, 0});
                CHECK(checkAndConsumeRelayout());
                layout.setGeometry({0, 1, 10, 12});
                CHECK(item0->stubGeometry == QRect{0, 1, 10, 12});
                CHECK(layoutRequested->size() == 0);
            }

            SECTION("add widget") {
                CHECK(layoutRequested->size() == 0);
                root.setLayout(&layout);
                CHECK(checkAndConsumeRelayout());
                StubWidget *item0 = new StubWidget(&root);
                CHECK(layoutRequested->size() == 0);
                layout.addWidget(item0);
                CHECK(checkAndConsumeRelayout());
            }

            SECTION("setSpacing") {
                CHECK(layoutRequested->size() == 0);
                root.setLayout(&layout);
                CHECK(checkAndConsumeRelayout());
                CHECK(layoutRequested->size() == 0);
                layout.setSpacing(2);
                CHECK(checkAndConsumeRelayout());
            }

            SECTION("addSpacing") {
                CHECK(layoutRequested->size() == 0);
                root.setLayout(&layout);
                CHECK(checkAndConsumeRelayout());
                CHECK(layoutRequested->size() == 0);
                layout.addSpacing(2);
                CHECK(checkAndConsumeRelayout());
            }

            SECTION("addStretch") {
                CHECK(layoutRequested->size() == 0);
                root.setLayout(&layout);
                CHECK(checkAndConsumeRelayout());
                CHECK(layoutRequested->size() == 0);
                layout.addStretch();
                CHECK(checkAndConsumeRelayout());
            }

        };

        SECTION("vbox") {
            Tui::ZVBoxLayout layout;
            tests(layout);
        }
        SECTION("hbox") {
            Tui::ZHBoxLayout layout;
            tests(layout);
        }
    });
}

TEST_CASE("boxlayout-policy", "") {
    // NOTE: To cover HBoxLayout and VBoxLayout with one set of tests, these tests are run once with rotated
    //       coordinates via `rotate` parameter which is either a RotH or a RotV instance.

    auto tests = [&](auto &layout, auto &rotate) {

        SECTION("one-1") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            CHECK(rotate.getPolicyLayout(layout) == p1);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Expanding);
        }

        SECTION("one-2") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, p1);
            layout.add(item0);
            CHECK(rotate.getPolicyLayout(layout) == Tui::SizePolicy::Fixed);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Expanding);
        }

        SECTION("one-spacer") {
            layout.addSpacing(10);
            CHECK(rotate.getPolicyLayout(layout) == Tui::SizePolicy::Fixed);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Fixed);
        }

        SECTION("one-plus-invisible") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed,
                                          Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred);
            CAPTURE(p1);
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, Tui::SizePolicy::Expanding);
            layout.add(item0);
            item0->stubIsVisible = false;

            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, p1);
            layout.add(item1);

            CHECK(rotate.getPolicyLayout(layout) == p1);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Expanding);
        }

        SECTION("two") {
            struct TestCase { int sourceLine; Tui::SizePolicy p1; Tui::SizePolicy p2; Tui::SizePolicy result; };
            auto testCase = GENERATE(
                                    TestCase{__LINE__, Tui::SizePolicy::Fixed, Tui::SizePolicy::Fixed, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Fixed, Tui::SizePolicy::Minimum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Fixed, Tui::SizePolicy::Maximum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Fixed, Tui::SizePolicy::Preferred, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Fixed, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding },

                                    TestCase{__LINE__, Tui::SizePolicy::Minimum, Tui::SizePolicy::Fixed, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Minimum, Tui::SizePolicy::Minimum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Minimum, Tui::SizePolicy::Maximum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Minimum, Tui::SizePolicy::Preferred, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Minimum, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding },

                                    TestCase{__LINE__, Tui::SizePolicy::Maximum, Tui::SizePolicy::Fixed, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Maximum, Tui::SizePolicy::Minimum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Maximum, Tui::SizePolicy::Maximum, Tui::SizePolicy::Fixed },
                                    TestCase{__LINE__, Tui::SizePolicy::Maximum, Tui::SizePolicy::Preferred, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Maximum, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding },

                                    TestCase{__LINE__, Tui::SizePolicy::Preferred, Tui::SizePolicy::Fixed, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Preferred, Tui::SizePolicy::Minimum, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Preferred, Tui::SizePolicy::Maximum, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Preferred, Tui::SizePolicy::Preferred, Tui::SizePolicy::Preferred },
                                    TestCase{__LINE__, Tui::SizePolicy::Preferred, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding },

                                    TestCase{__LINE__, Tui::SizePolicy::Expanding, Tui::SizePolicy::Fixed, Tui::SizePolicy::Expanding },
                                    TestCase{__LINE__, Tui::SizePolicy::Expanding, Tui::SizePolicy::Minimum, Tui::SizePolicy::Expanding },
                                    TestCase{__LINE__, Tui::SizePolicy::Expanding, Tui::SizePolicy::Maximum, Tui::SizePolicy::Expanding },
                                    TestCase{__LINE__, Tui::SizePolicy::Expanding, Tui::SizePolicy::Preferred, Tui::SizePolicy::Expanding },
                                    TestCase{__LINE__, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding, Tui::SizePolicy::Expanding }
                        );
            StubLayout *item0 = new StubLayout();
            rotate.setPolicyLayout(item0, testCase.p1);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, testCase.p2);
            layout.add(item1);
            CHECK(rotate.getPolicyLayout(layout) == testCase.result);
        }

        SECTION("two-spacer-1") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed,
                                          Tui::SizePolicy::Minimum, Tui::SizePolicy::Maximum);
            CAPTURE(p1);
            layout.addSpacing(10);
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, p1);
            layout.add(item1);
            CHECK(rotate.getPolicyLayout(layout) == Tui::SizePolicy::Fixed);
        }

        SECTION("two-spacer-2") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Preferred, Tui::SizePolicy::Expanding);
            layout.addSpacing(10);
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, p1);
            layout.add(item1);
            CHECK(rotate.getPolicyLayout(layout) == p1);
        }

    };

    auto testsOnlySameRotation = [&](auto &layout, auto &rotate) {

        SECTION("one-stretch") {
            layout.addStretch();
            CHECK(rotate.getPolicyLayout(layout) == Tui::SizePolicy::Expanding);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Fixed);
        }

        SECTION("two-stretch") {
            Tui::SizePolicy p1 = GENERATE(Tui::SizePolicy::Fixed, Tui::SizePolicy::Minimum, Tui::SizePolicy::Maximum,
                                          Tui::SizePolicy::Preferred, Tui::SizePolicy::Expanding);

            layout.addStretch();
            StubLayout *item1 = new StubLayout();
            rotate.setPolicyLayout(item1, p1);
            rotate.setPolicyAcross(item1, Tui::SizePolicy::Fixed);
            layout.add(item1);

            CHECK(rotate.getPolicyLayout(layout) == Tui::SizePolicy::Expanding);
            CHECK(rotate.getPolicyAcross(layout) == Tui::SizePolicy::Fixed);
        }

    };

    SECTION("vbox-v") {
        Tui::ZVBoxLayout layout;
        RotV rotate;
        tests(layout, rotate);
        testsOnlySameRotation(layout, rotate);
    }

    SECTION("vbox-h") {
        Tui::ZVBoxLayout layout;
        RotH rotate;
        tests(layout, rotate);
    }

    SECTION("hbox-h") {
        Tui::ZHBoxLayout layout;
        RotH rotate;
        tests(layout, rotate);
        testsOnlySameRotation(layout, rotate);
    }

    SECTION("hbox-v") {
        Tui::ZHBoxLayout layout;
        RotV rotate;
        tests(layout, rotate);
    }
}

TEST_CASE("boxlayout-sizehint", "") {
    // NOTE: To cover HBoxLayout and VBoxLayout with one set of tests, these tests are run once with rotated
    //       coordinates via `rotate` parameter which is either a RotH or a RotV instance.

    auto tests = [&](auto &layout, auto &rotate) {

        SECTION("one") {
            int p1 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p1);
            int p2 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p2);
            // spacing does not change anything for layout with just one item
            int spacing = GENERATE(0, 1, 5);
            CAPTURE(spacing);

            layout.setSpacing(spacing);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(p1, p2);
            layout.add(item0);

            CHECK(layout.sizeHint() == rotate.size(std::max(p1, 0), std::max(p2, 0)));
        }

        SECTION("two") {
            int p1 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p1);
            int p2 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p2);

            int p3 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p3);
            int p4 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p4);

            int spacing = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(spacing);

            layout.setSpacing(spacing);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(p1, p2);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(p3, p4);
            layout.add(item1);
            CHECK(layout.sizeHint() == rotate.size(std::max(std::max(p1, 0), std::max(p3, 0)),
                                                   std::max(p2, 0) +  std::max(spacing, 0) + std::max(p4, 0)));
        }

        SECTION("two-spacer") {
            int p1 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p1);
            int p2 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p2);

            int p3 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p3);
            int p4 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p4);

            // Spacing does not matter adjacent to an item with isSpacer() == true
            int spacing = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(spacing);
            layout.setSpacing(spacing);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(p1, p2);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(p3, p4);
            item1->stubIsSpacer = true;
            layout.add(item1);
            CHECK(layout.sizeHint() == rotate.size(std::max(std::max(p1, 0), std::max(p3, 0)),
                                                   std::max(p2, 0) + std::max(p4, 0)));
        }

        SECTION("three-one-invisible") {
            int p1 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p1);
            int p2 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p2);

            int p3 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p3);
            int p4 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p4);

            int spacing = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(spacing);
            layout.setSpacing(spacing);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(p1, p2);
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(45, 99);
            item1->stubIsVisible = false;
            layout.add(item1);
            StubLayout *item2 = new StubLayout();
            item2->stubSizeHint = rotate.size(p3, p4);
            layout.add(item2);
            CHECK(layout.sizeHint() == rotate.size(std::max(std::max(p1, 0), std::max(p3, 0)),
                                                   std::max(p2, 0) +  std::max(spacing, 0) + std::max(p4, 0)));
        }

        SECTION("two-stretch") {
            int p1 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p1);
            int p2 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p2);

            int p3 = GENERATE(-1, 0, 1, 4, 7);
            CAPTURE(p3);
            int p4 = GENERATE(-1, 0, 1, 3, 5);
            CAPTURE(p4);

            StubLayout *item0 = new StubLayout();
            item0->stubSizeHint = rotate.size(p1, p2);
            layout.add(item0);
            layout.addStretch();
            StubLayout *item1 = new StubLayout();
            item1->stubSizeHint = rotate.size(p3, p4);
            layout.add(item1);
            CHECK(layout.sizeHint() == rotate.size(std::max(std::max(p1, 0), std::max(p3, 0)),
                                                   std::max(p2, 0) + std::max(p4, 0)));
        }

    };

    SECTION("vbox") {
        Tui::ZVBoxLayout layout;
        RotV rotate;
        tests(layout, rotate);
    }

    SECTION("hbox") {
        Tui::ZHBoxLayout layout;
        RotH rotate;
        tests(layout, rotate);
    }
}

TEST_CASE("boxlayout-visible", "") {

    auto tests = [&](auto &layout) {

        SECTION("empty") {
            CHECK(layout.isVisible() == false);
        }

        SECTION("one-v") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            CHECK(layout.isVisible() == true);
        }

        SECTION("one-i") {
            StubLayout *item0 = new StubLayout();
            item0->stubIsVisible = false;
            layout.add(item0);
            CHECK(layout.isVisible() == false);
        }

        SECTION("two-vv") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            CHECK(layout.isVisible() == true);
        }

        SECTION("two-vi") {
            StubLayout *item0 = new StubLayout();
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubIsVisible = false;
            layout.add(item1);
            CHECK(layout.isVisible() == true);
        }

        SECTION("two-iv") {
            StubLayout *item0 = new StubLayout();
            item0->stubIsVisible = false;
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            layout.add(item1);
            CHECK(layout.isVisible() == true);
        }

        SECTION("two-ii") {
            StubLayout *item0 = new StubLayout();
            item0->stubIsVisible = false;
            layout.add(item0);
            StubLayout *item1 = new StubLayout();
            item1->stubIsVisible = false;
            layout.add(item1);
            CHECK(layout.isVisible() == false);
        }
    };

    SECTION("vbox") {
        Tui::ZVBoxLayout layout;
        tests(layout);
    }
    SECTION("hbox") {
        Tui::ZHBoxLayout layout;
        tests(layout);
    }
}

TEST_CASE("boxlayout-lifetime", "") {

    Testhelper t("unsued", "unused", 16, 5);

    auto tests = [&](auto *layout) {

        Tui::ZWidget outer;
        outer.setLayout(layout);

        SECTION("child-widget-reparented") {
            auto *w1 = new Tui::ZWidget(&outer);
            w1->setMinimumSize(10, 10);
            layout->addWidget(w1);
            CHECK(layout->sizeHint() == QSize{10, 10});
            w1->setParent(nullptr);
            CHECK(layout->sizeHint() == QSize{0, 0});
            delete w1;
        }

        SECTION("child-layout-reparented") {
            auto *innerLayout = new Tui::ZVBoxLayout();
            auto *w1 = new Tui::ZWidget(&outer);

            QPointer<Tui::ZVBoxLayout> innerLayoutWatch = innerLayout;
            QPointer<Tui::ZWidget> w1Watch = w1;

            w1->setMinimumSize(10, 10);
            innerLayout->addWidget(w1);
            layout->add(innerLayout);
            CHECK(layout->sizeHint() == QSize{10, 10});
            innerLayout->setParent(nullptr);
            CHECK(layout->sizeHint() == QSize{0, 0});
            CHECK(!innerLayoutWatch.isNull());
            CHECK(!w1Watch.isNull());
            delete innerLayout;
            CHECK(!w1Watch.isNull());
        }

        SECTION("child-widget-in-sublayout-reparented") {
            auto *innerLayout = new Tui::ZVBoxLayout();
            auto *w1 = new Tui::ZWidget(&outer);
            w1->setMinimumSize(10, 10);
            innerLayout->addWidget(w1);
            layout->add(innerLayout);
            CHECK(layout->sizeHint() == QSize{10, 10});
            w1->setParent(nullptr);
            CHECK(layout->sizeHint() == QSize{0, 0});
            delete w1;
        }
    };

    SECTION("vbox") {
        auto *layout = new Tui::ZVBoxLayout();
        tests(layout);
    }
    SECTION("hbox") {
        auto *layout = new Tui::ZHBoxLayout();
        tests(layout);
    }
}
