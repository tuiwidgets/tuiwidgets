// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZWidget.h>

#include <limits>
#include <optional>

#include <Tui/ZCommandManager.h>
#include <Tui/ZLayout.h>
#include <Tui/ZPalette.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../eventrecorder.h"

namespace {
    class TestWidget : public Tui::ZWidget {
    public:
        explicit TestWidget(ZWidget *parent = 0) : Tui::ZWidget(parent) {};

    public:
        void paintEvent(Tui::ZPaintEvent *event) override {
            paint(event);
            Tui::ZWidget::paintEvent(event);
        }

        std::function<void(Tui::ZPaintEvent *event)> paint;
    };

    class TestWidgetEvents : public Tui::ZWidget {
    public:
        explicit TestWidgetEvents(EventRecorder &recorder) : recorder(recorder) {
            paintCalledEvent = recorder.createEvent("paint event");
            keyCalledEvent = recorder.createEvent("key event");
            pasteCalledEvent = recorder.createEvent("paste event");
            focusInCalledEvent = recorder.createEvent("focus in event");
            focusOutCalledEvent = recorder.createEvent("focus out event");
            resizeCalledEvent = recorder.createEvent("resize event");
            moveCalledEvent = recorder.createEvent("move event");
        };

    public:
        void paintEvent(Tui::ZPaintEvent *event) override {
            recorder.recordEvent(paintCalledEvent, event);
        }

        void keyEvent(Tui::ZKeyEvent *event) override {
            recorder.recordEvent(keyCalledEvent, event);
        }

        void pasteEvent(Tui::ZPasteEvent *event) override {
            recorder.recordEvent(pasteCalledEvent, event);
        }

        void focusInEvent(Tui::ZFocusEvent *event) override {
            recorder.recordEvent(focusInCalledEvent, event);
        }

        void focusOutEvent(Tui::ZFocusEvent *event) override {
            recorder.recordEvent(focusOutCalledEvent, event);
        }

        void resizeEvent(Tui::ZResizeEvent *event) override {
            recorder.recordEvent(resizeCalledEvent, event);
        }

        void moveEvent(Tui::ZMoveEvent *event) override {
            recorder.recordEvent(moveCalledEvent, event);
        }

        EventRecorder &recorder;

        RecorderEvent paintCalledEvent;
        RecorderEvent keyCalledEvent;
        RecorderEvent pasteCalledEvent;
        RecorderEvent focusInCalledEvent;
        RecorderEvent focusOutCalledEvent;
        RecorderEvent resizeCalledEvent;
        RecorderEvent moveCalledEvent;
    };

    class TestWidgetHints : public Tui::ZWidget {
    public:
        QSize sizeHint() const override {
            if (sizeHintOverride) {
                return *sizeHintOverride;
            }
            return ZWidget::sizeHint();
        }

        QSize minimumSizeHint() const override {
            if (minimumSizeHintOverride) {
                return *minimumSizeHintOverride;
            }
            return ZWidget::minimumSizeHint();
        }

        std::optional<QSize> sizeHintOverride;
        std::optional<QSize> minimumSizeHintOverride;
    };
}

TEST_CASE("widget-base") {
    {
        bool parent = GENERATE(false, true);
        CAPTURE(parent);

        std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

        SECTION("constructor") {
            delete new Tui::ZWidget();
            delete new Tui::ZWidget(w.get());
        }
    }

    std::unique_ptr<Tui::ZWidget> w = std::make_unique<Tui::ZWidget>();
    SECTION("defaults") {
        CHECK(w->geometry() == QRect{0, 0, 0, 0});
        CHECK(w->rect() == QRect{0, 0, 0, 0});
        CHECK(w->contentsRect() == QRect{0, 0, 0, 0});
        CHECK(w->isEnabled() == true);
        CHECK(w->isLocallyEnabled() == true);
        CHECK(w->isVisible() == true);
        CHECK(w->isLocallyVisible() == true);
        CHECK(w->stackingLayer() == 0);

        CHECK(w->minimumSize() == QSize{-1, -1});
        CHECK(w->maximumSize() == QSize{16777215, 16777215});
        CHECK(w->sizePolicyH() == Tui::SizePolicy::Preferred);
        CHECK(w->sizePolicyV() == Tui::SizePolicy::Preferred);

        CHECK(w->sizeHint() == QSize{-1, -1});
        CHECK(w->minimumSizeHint() == QSize{-1, -1});

        CHECK(w->effectiveSizeHint() == QSize{-1, -1});
        CHECK(w->effectiveMinimumSize() == QSize{-1, -1});

        CHECK(w->layoutArea() == QRect{0, 0, 0, 0});
        Tui::ZLayout *l = w->layout();
        CHECK(l == nullptr);

        CHECK(w->focusPolicy() == Tui::FocusPolicy::NoFocus);
        CHECK(w->focusMode() == Tui::FocusContainerMode::None);
        CHECK(w->focusOrder() == 0);
        CHECK(w->contentsMargins() == QMargins{0, 0, 0, 0});
        CHECK(w->paletteClass() == QStringList({}));
        CHECK(w->palette().isNull());
        CHECK(w->cursorStyle() == Tui::CursorStyle::Unset);

        CHECK(w->focus() == false);
        CHECK(w->isInFocusPath() == false);
    }

    SECTION("no terminal") {
        CHECK(w->terminal() == nullptr);

        CHECK(w->mapFromTerminal({0, 0}) == QPoint(0, 0));
        CHECK(w->mapToTerminal({0, 0}) == QPoint(0, 0));

        CHECK(qAsConst(w)->placeFocus() == nullptr);
        CHECK(w->placeFocus() == nullptr);

        CHECK(w->resolveSizeHintChain() == w.get());

        CHECK(w->commandManager() == nullptr);
    }
}

TEST_CASE("widget-getter-and-setter") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget = Tui::ZWidget(t.root);

    SECTION("geometry") {
        CHECK(widget.geometry() == QRect{0, 0, 0, 0});
        widget.setGeometry({});
        CHECK(widget.geometry() == QRect{0, 0, 0, 0});
        widget.setGeometry({-1, -1, -1, -1});
        CHECK(widget.geometry() == QRect{-1, -1, 0, 0});
        widget.setGeometry({65999, 65999, 65999, 65999});
        CHECK(widget.geometry() == QRect{65999, 65999, 65999, 65999});
    }
    SECTION("setVisible") {
        CHECK(widget.isVisible() == true);
        widget.setVisible(false);
        CHECK(widget.isVisible() == false);
    }
    SECTION("setStackingLayer") {
        widget.setStackingLayer(-1);
        CHECK(widget.stackingLayer() == -1);
    }
    SECTION("setMinimumSize") {
        widget.setMinimumSize(QSize{-1, 99});
        CHECK(widget.minimumSize() == QSize{-1, 99});
        widget.setMinimumSize(99, -1);
        CHECK(widget.minimumSize() == QSize{99, -1});
    }
    SECTION("maximumSize") {
        widget.setMaximumSize(QSize{-1, 99});
        CHECK(widget.maximumSize() == QSize{-1, 99});
        widget.setMaximumSize(99, -1);
        CHECK(widget.maximumSize() == QSize{99, -1});
    }
    SECTION("setFixedSize") {
        widget.setFixedSize({16777216, 16777217});
        CHECK(widget.minimumSize() == QSize{16777216, 16777217});
        CHECK(widget.maximumSize() == QSize{16777216, 16777217});

        widget.setFixedSize(16777217, 16777216);
        CHECK(widget.minimumSize() == QSize{16777217, 16777216});
        CHECK(widget.maximumSize() == QSize{16777217, 16777216});
    }
    SECTION("setSizePolicy") {
        Tui::SizePolicy sp = GENERATE(Tui::SizePolicy::Fixed,
                                      Tui::SizePolicy::Minimum,
                                      Tui::SizePolicy::Maximum,
                                      Tui::SizePolicy::Preferred,
                                      Tui::SizePolicy::Expanding);
        SECTION("H") {
            widget.setSizePolicyH(sp);
            CHECK(widget.sizePolicyH() == sp);
        }
        SECTION("V") {
            widget.setSizePolicyV(sp);
            CHECK(widget.sizePolicyV() == sp);
        }
    }
    SECTION("setLayout") {
        StubLayout layout;
        widget.setLayout(&layout);
        CHECK(widget.layout() == &layout);
    }
    SECTION("setFocusPolicy") {
        Tui::FocusPolicy fp = GENERATE(Tui::FocusPolicy::NoFocus,
                                       Tui::FocusPolicy::TabFocus,
                                       Tui::FocusPolicy::ClickFocus,
                                       Tui::FocusPolicy::StrongFocus,
                                       Tui::FocusPolicy::WheelFocus);
        widget.setFocusPolicy(fp);
        CHECK(widget.focusPolicy() == fp);
    }
    SECTION("setFocusMode") {
        Tui::FocusContainerMode fcm = GENERATE(Tui::FocusContainerMode::None,
                                               Tui::FocusContainerMode::SubOrdering,
                                               Tui::FocusContainerMode::Cycle);
        widget.setFocusMode(fcm);
        CHECK(widget.focusMode() == fcm);
    }
    SECTION("setFocusOrder") {
        widget.setFocusOrder(-1);
        CHECK(widget.focusOrder() == -1);
        widget.setFocusOrder(99);
        CHECK(widget.focusOrder() == 99);
    }
    SECTION("setContentsMargins") {
        widget.setContentsMargins({2, 1, 4, 6});
        CHECK(widget.contentsMargins() == QMargins{2, 1, 4, 6});
    }
    SECTION("setPaletteClass") {
        widget.setPaletteClass({"default"});
        CHECK(widget.paletteClass() == QStringList({"default"}));

        // trigger no changes code path
        widget.setPaletteClass({"default"});
        CHECK(widget.paletteClass() == QStringList({"default"}));
    }
    SECTION("addPaletteClass") {
        widget.setPaletteClass({"default"});
        CHECK(widget.paletteClass() == QStringList({"default"}));
        widget.addPaletteClass({"new"});
        CHECK(widget.paletteClass() == QStringList({"default", "new"}));
        widget.removePaletteClass({"default"});
        CHECK(widget.paletteClass() == QStringList({"new"}));
    }
    SECTION("setCursorStyle") {
        Tui::CursorStyle cs = GENERATE( Tui::CursorStyle::Bar,
                                        Tui::CursorStyle::Block,
                                        Tui::CursorStyle::Underline,
                                        Tui::CursorStyle::Unset);
        widget.setCursorStyle(cs);
        CHECK(widget.cursorStyle() == cs);
    }
    SECTION("setCommandManager") {
        Tui::ZCommandManager cm;
        widget.setCommandManager(&cm);
        CHECK(widget.commandManager() == &cm);
    }
}

TEST_CASE("widget-misc") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget = Tui::ZWidget(t.root);

    SECTION("setFocus") {
        CHECK(widget.focus() == false);
        widget.setFocus();
        CHECK(widget.focus() == true);
    }
    SECTION("grabKeyboard") {
        widget.grabKeyboard();
        CHECK(t.terminal->keyboardGrabber() == &widget);
    }
    SECTION("isAncestorOf") {
        CHECK(widget.isAncestorOf(nullptr) == false);
    }
    SECTION("isEnabledTo") {
        CHECK(widget.isEnabledTo(nullptr) == false);
    }
    SECTION("isVisibleTo") {
        CHECK(widget.isVisibleTo(nullptr) == false);
    }
    SECTION("placeFocus") {
        CHECK(qAsConst(widget).placeFocus() == nullptr);
        CHECK(widget.placeFocus() == nullptr);
    }
}

TEST_CASE("widget-geometry") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget(t.root);

    EventRecorder recorder;
    auto moveEvent = recorder.watchMoveEvent(&widget, "move");
    auto resizeEvent = recorder.watchResizeEvent(&widget, "resize");

    QPoint oldPos = {0, 0};
    QSize oldSize = {0, 0};

    for (int x = -2; x < 5; x++) {
        for (int y = -2; y <5 ; y++) {
            for (int w = -2; w < 5; w++) {
                for (int h = -2; h < 5; h++) {
                    widget.setGeometry({x, y, w, h});

                    QPoint expectedPos = QPoint{x, y};
                    if (expectedPos != oldPos) {
                        CHECK(recorder.consumeFirst(moveEvent, expectedPos, oldPos));
                        oldPos = expectedPos;
                    }

                    QSize expectedSize = QSize{std::max(0, w), std::max(0, h)};
                    if (expectedSize != oldSize) {
                        CHECK(recorder.consumeFirst(resizeEvent, expectedSize, oldSize));
                        oldSize = expectedSize;
                    }
                    CHECK(recorder.noMoreEvents());

                    CHECK(widget.geometry() == QRect{x, y, std::max(0, w), std::max(0, h)});
                    CHECK(widget.rect() == QRect{0, 0, std::max(0, w), std::max(0, h)});
                    CHECK(widget.mapToTerminal(QPoint{0, 0}) == QPoint{x, y});
                    CHECK(widget.mapFromTerminal(QPoint{x, y}) == QPoint{0, 0});
                }
            }
        }
    }
    CHECK(recorder.noMoreEvents());
}

TEST_CASE("widget-mapToAndFromTerminal") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    Tui::ZWidget widget(&parent);
    Tui::ZWidget child(&widget);

    int parentX = GENERATE(5, 10);
    int parentY = GENERATE(8, 13);

    int fromX = GENERATE(-1, 3);
    int fromY = GENERATE(-2, 5);

    int widgetX = GENERATE(-4, 6);
    int widgetY = GENERATE(-5, 8);

    int childX = GENERATE(-6, 9);
    int childY = GENERATE(-8, 7);

    int hw = GENERATE(0, 15);

    CAPTURE(QRect{parentX, parentY, fromX, fromY});
    CAPTURE(QRect{widgetX, widgetY, childX, childY});

    parent.setGeometry({parentX, parentY, hw * 4, hw * 4});
    CHECK(parent.mapToTerminal(QPoint{fromX, fromY}) == QPoint{parentX + fromX, parentY + fromY});
    CHECK(parent.mapFromTerminal(QPoint{parentX + fromX, parentY + fromY}) == QPoint{fromX, fromY});

    widget.setGeometry({widgetX, widgetY, hw * 2, hw * 2});
    CHECK(widget.mapToTerminal(QPoint{fromX - widgetX, fromY - widgetY}) == QPoint{parentX + fromX, parentY + fromY});
    CHECK(widget.mapFromTerminal(QPoint{parentX + fromX + widgetX, parentY + fromY + widgetY}) == QPoint{fromX, fromY});

    child.setGeometry({childX, childY, hw, hw});
    CHECK(child.mapToTerminal(QPoint{fromX - widgetX - childX, fromY - widgetY - childY}) == QPoint{parentX + fromX, parentY + fromY});
    CHECK(child.mapFromTerminal(QPoint{parentX + fromX + widgetX + childX, parentY + fromY + widgetY + childY}) == QPoint{fromX, fromY});
}

TEST_CASE("widget-enable") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget(t.root);
    Tui::ZWidget child(&widget);

    CHECK(widget.isEnabled() == true);
    CHECK(child.isLocallyEnabled() == true);
    CHECK(child.isEnabled() == true);
    CHECK(child.isLocallyEnabled() == true);

    EventRecorder recorder;
    auto childEnabledChange = recorder.watchEnabledChangeEvent(&child, "child enabled change");
    auto widgetEnabledChange = recorder.watchEnabledChangeEvent(&widget, "widget enabled change");

    // this test should not trigger visibility events
    recorder.watchShowToParentEvent(&widget, "widget show to parent");
    recorder.watchHideToParentEvent(&widget, "widget hide to parent");
    recorder.watchShowEvent(&widget, "widget show");
    recorder.watchHideEvent(&widget, "widget hide");
    recorder.watchShowToParentEvent(&child, "child show to parent");
    recorder.watchHideToParentEvent(&child, "child hide to parent");
    recorder.watchShowEvent(&child, "child show");
    recorder.watchHideEvent(&child, "child hide");

    SECTION("child-disable") {
        child.setEnabled(false);
        CHECK(recorder.consumeFirst(childEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == true);
        CHECK(widget.isLocallyEnabled() == true);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
    }

    SECTION("disable-enable") {
        child.setEnabled(false);
        CHECK(recorder.consumeFirst(childEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == true);
        CHECK(widget.isLocallyEnabled() == true);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
        widget.setEnabled(false);
        CHECK(recorder.consumeFirst(widgetEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == false);
        CHECK(widget.isLocallyEnabled() == false);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
        widget.setEnabled(true);
        CHECK(recorder.consumeFirst(widgetEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == true);
        CHECK(widget.isLocallyEnabled() == true);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
    }
    SECTION("disable-enable-both") {
        child.setEnabled(false);
        CHECK(recorder.consumeFirst(childEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == true);
        CHECK(widget.isLocallyEnabled() == true);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
        widget.setEnabled(false);
        CHECK(recorder.consumeFirst(widgetEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == false);
        CHECK(widget.isLocallyEnabled() == false);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == false);
        child.setEnabled(true);
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == false);
        CHECK(widget.isLocallyEnabled() == false);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == true);
        widget.setEnabled(true);
        CHECK(recorder.consumeFirst(childEnabledChange));
        CHECK(recorder.consumeFirst(widgetEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == true);
        CHECK(widget.isLocallyEnabled() == true);
        CHECK(child.isEnabled() == true);
        CHECK(child.isLocallyEnabled() == true);
    }

    SECTION("widget-disable") {
        widget.setEnabled(false);
        CHECK(recorder.consumeFirst(childEnabledChange));
        CHECK(recorder.consumeFirst(widgetEnabledChange));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isEnabled() == false);
        CHECK(widget.isLocallyEnabled() == false);
        CHECK(child.isEnabled() == false);
        CHECK(child.isLocallyEnabled() == true);

        SECTION("child-enable") {
            child.setEnabled(true);
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isEnabled() == false);
            CHECK(widget.isLocallyEnabled() == false);
            CHECK(child.isEnabled() == false);
            CHECK(child.isLocallyEnabled() == true);
        }

        SECTION("widget-enable") {
            widget.setEnabled(true);
            CHECK(recorder.consumeFirst(childEnabledChange));
            CHECK(recorder.consumeFirst(widgetEnabledChange));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isEnabled() == true);
            CHECK(widget.isLocallyEnabled() == true);
            CHECK(child.isEnabled() == true);
            CHECK(child.isLocallyEnabled() == true);
        }

        SECTION("child-disable") {
            child.setEnabled(false);
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isEnabled() == false);
            CHECK(widget.isLocallyEnabled() == false);
            CHECK(child.isEnabled() == false);
            CHECK(child.isLocallyEnabled() == false);
        }

        SECTION("parent-enable-child-disable") {
            widget.setEnabled(true);
            CHECK(recorder.consumeFirst(childEnabledChange));
            CHECK(recorder.consumeFirst(widgetEnabledChange));
            CHECK(recorder.noMoreEvents());
            CHECK(child.isEnabled() == true);
            child.setEnabled(false);
            CHECK(recorder.consumeFirst(childEnabledChange));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isEnabled() == true);
            CHECK(widget.isLocallyEnabled() == true);
            CHECK(child.isEnabled() == false);
            CHECK(child.isLocallyEnabled() == false);
        }
    }
    CHECK(recorder.noMoreEvents());
}

TEST_CASE("widget-visible") {
    // same tests as for enable, but events differ for visibility
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget(t.root);
    Tui::ZWidget child(&widget);

    EventRecorder recorder;
    auto widgetShowToParent = recorder.watchShowToParentEvent(&widget, "widget show to parent");
    auto widgetHideToParent = recorder.watchHideToParentEvent(&widget, "widget hide to parent");
    auto widgetShow = recorder.watchShowEvent(&widget, "widget show");
    auto widgetHide = recorder.watchHideEvent(&widget, "widget hide");
    auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
    auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
    auto childShow = recorder.watchShowEvent(&child, "child show");
    auto childHide = recorder.watchHideEvent(&child, "child hide");

    // this test should not trigger enabled events
    recorder.watchEnabledChangeEvent(&child, "child enabled change");
    recorder.watchEnabledChangeEvent(&widget, "widget enabled change");


    CHECK(widget.isVisible() == true);
    CHECK(child.isLocallyVisible() == true);
    CHECK(child.isVisible() == true);
    CHECK(child.isLocallyVisible() == true);

    SECTION("child-not-visible") {
        child.setVisible(false);
        CHECK(recorder.consumeFirst(childHide));
        CHECK(recorder.consumeFirst(childHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == true);
        CHECK(widget.isLocallyVisible() == true);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
    }

    SECTION("widget-change-visibility") {
        child.setVisible(false);
        CHECK(recorder.consumeFirst(childHide));
        CHECK(recorder.consumeFirst(childHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == true);
        CHECK(widget.isLocallyVisible() == true);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
        widget.setVisible(false);
        CHECK(recorder.consumeFirst(widgetHide));
        CHECK(recorder.consumeFirst(widgetHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == false);
        CHECK(widget.isLocallyVisible() == false);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
        widget.setVisible(true);
        CHECK(recorder.consumeFirst(widgetShow));
        CHECK(recorder.consumeFirst(widgetShowToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == true);
        CHECK(widget.isLocallyVisible() == true);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
    }
    SECTION("child-change-visibility") {
        child.setVisible(false);
        CHECK(recorder.consumeFirst(childHide));
        CHECK(recorder.consumeFirst(childHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == true);
        CHECK(widget.isLocallyVisible() == true);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
        widget.setVisible(false);
        CHECK(recorder.consumeFirst(widgetHide));
        CHECK(recorder.consumeFirst(widgetHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == false);
        CHECK(widget.isLocallyVisible() == false);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == false);
        child.setVisible(true);
        CHECK(recorder.consumeFirst(childShowToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == false);
        CHECK(widget.isLocallyVisible() == false);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == true);
        widget.setVisible(true);
        CHECK(recorder.consumeFirst(childShow));
        CHECK(recorder.consumeFirst(widgetShow));
        CHECK(recorder.consumeFirst(widgetShowToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == true);
        CHECK(widget.isLocallyVisible() == true);
        CHECK(child.isVisible() == true);
        CHECK(child.isLocallyVisible() == true);
    }


    SECTION("widget-not-visible") {
        widget.setVisible(false);
        CHECK(recorder.consumeFirst(widgetHide));
        CHECK(recorder.consumeFirst(childHide));
        CHECK(recorder.consumeFirst(widgetHideToParent));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.isVisible() == false);
        CHECK(widget.isLocallyVisible() == false);
        CHECK(child.isVisible() == false);
        CHECK(child.isLocallyVisible() == true);

        SECTION("child-set-visible") {
            child.setVisible(true);
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isVisible() == false);
            CHECK(widget.isLocallyVisible() == false);
            CHECK(child.isVisible() == false);
            CHECK(child.isLocallyVisible() == true);
        }

        SECTION("widget-set-visible") {
            widget.setVisible(true);
            CHECK(recorder.consumeFirst(childShow));
            CHECK(recorder.consumeFirst(widgetShow));
            CHECK(recorder.consumeFirst(widgetShowToParent));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isVisible() == true);
            CHECK(widget.isLocallyVisible() == true);
            CHECK(child.isVisible() == true);
            CHECK(child.isLocallyVisible() == true);
        }

        SECTION("child-not-visible") {
            child.setVisible(false);
            CHECK(recorder.consumeFirst(childHideToParent));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isVisible() == false);
            CHECK(widget.isLocallyVisible() == false);
            CHECK(child.isVisible() == false);
            CHECK(child.isLocallyVisible() == false);
        }

        SECTION("parent-visible-child-not-visible") {
            widget.setVisible(true);
            CHECK(recorder.consumeFirst(childShow));
            CHECK(recorder.consumeFirst(widgetShow));
            CHECK(recorder.consumeFirst(widgetShowToParent));
            CHECK(recorder.noMoreEvents());
            CHECK(child.isVisible() == true);
            child.setVisible(false);
            CHECK(recorder.consumeFirst(childHide));
            CHECK(recorder.consumeFirst(childHideToParent));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isVisible() == true);
            CHECK(widget.isLocallyVisible() == true);
            CHECK(child.isVisible() == false);
            CHECK(child.isLocallyVisible() == false);
        }
    }
    CHECK(recorder.noMoreEvents());
}

static void layerConistencyCheck(const Tui::ZWidget *widget) {
    int currentLayer = std::numeric_limits<int>::min();
    for (QObject *o: widget->children()) {
        auto *w = qobject_cast<Tui::ZWidget*>(o);
        if (w) {
            if (w->stackingLayer() < currentLayer) {
                FAIL_CHECK("stacking layers inconsistent");
            }
            currentLayer = w->stackingLayer();
        }
    }
};

TEST_CASE("widget-stackingLayer") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget(t.root);
    CHECK(widget.stackingLayer() == 0);

    Tui::ZWidget child1(&widget);
    CHECK(widget.stackingLayer() == 0);
    CHECK(child1.stackingLayer() == 0);
    CHECK(widget.children().indexOf(&child1) == 0);

    Tui::ZWidget child2(&widget);
    CHECK(widget.stackingLayer() == 0);
    CHECK(child1.stackingLayer() == 0);
    CHECK(child2.stackingLayer() == 0);
    CHECK(widget.children().indexOf(&child2) == 1);

    Tui::ZWidget child3(&widget);
    CHECK(widget.stackingLayer() == 0);
    CHECK(child1.stackingLayer() == 0);
    CHECK(child2.stackingLayer() == 0);
    CHECK(widget.children().indexOf(&child3) == 2);

    SECTION("lower") {
        child2.lower();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 0);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("lower-1") {
        child1.lower();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("raise-2") {
        child2.raise();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 1);
    }
    SECTION("raise-3") {
        child3.raise();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }

    SECTION("changeStackingLayer") {
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.stackingLayer() == 0);
        CHECK(child1.stackingLayer() == 0);
        CHECK(child2.stackingLayer() == 1);
        CHECK(child3.stackingLayer() == 0);

        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 1);

        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 0);

        child3.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 0);
        CHECK(widget.children().indexOf(&child3) == 2);

        CHECK(widget.stackingLayer() == 0);
        CHECK(child1.stackingLayer() == 1);
        CHECK(child2.stackingLayer() == 1);
        CHECK(child3.stackingLayer() == 1);
    }

    SECTION("noChangeStackingLayer") {
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.stackingLayer() == 0);
        CHECK(child1.stackingLayer() == 0);
        CHECK(child2.stackingLayer() == 1);
        CHECK(child3.stackingLayer() == 0);

        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 1);

        // setting to same stacking layer must not raise the widget.
        child1.setStackingLayer(0);

        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 1);
    }

    SECTION("StackingLayer-order12") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);
    }
    SECTION("StackingLayer-order21") {
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

    SECTION("StackingLayer-order12-in-1") {
        child1.setStackingLayer(-1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(-1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("StackingLayer-order21-in-1") {
        child2.setStackingLayer(-1);
        layerConistencyCheck(&widget);
        child1.setStackingLayer(-1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 0);
        CHECK(widget.children().indexOf(&child3) == 2);
    }

    SECTION("StackingLayer-raise1") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child1.raise();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
    }
    SECTION("StackingLayer-raise2") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.raise();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
    }
    SECTION("StackingLayer-raise3") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child3.raise();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

    SECTION("StackingLayer-lower1") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child1.lower();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
    }
    SECTION("StackingLayer-lower2") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.lower();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
    }
    SECTION("StackingLayer-lower3") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child3.lower();
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

    SECTION("stackUnder1-3") {
        child1.stackUnder(&child3);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 0);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("stackUnder2-3") {
        child2.stackUnder(&child3);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("stackUnder3-3") {
        child3.stackUnder(&child3);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("stackUnder3-1") {
        child3.stackUnder(&child1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);
    }
    SECTION("stackUnder3-2") {
        child3.stackUnder(&child2);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 1);
    }

    SECTION("stackUnder1-3-setStackingLayer") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);

        child1.stackUnder(&child3);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

    SECTION("stackUnder2-3-setStackingLayer") {
        child1.setStackingLayer(1);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);

        child2.stackUnder(&child3);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

    SECTION("stackUnder1-4-setStackingLayer") {
        Tui::ZWidget child4(&widget);
        child3.setStackingLayer(1);
        child4.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
        CHECK(widget.children().indexOf(&child4) == 3);

        child1.stackUnder(&child4);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 0);
        CHECK(widget.children().indexOf(&child3) == 2);
        CHECK(widget.children().indexOf(&child4) == 3);
    }

    SECTION("stackUnder2-4-setStackingLayer") {
        Tui::ZWidget child4(&widget);
        child3.setStackingLayer(1);
        child4.setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
        CHECK(widget.children().indexOf(&child4) == 3);

        child2.stackUnder(&child4);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
        CHECK(widget.children().indexOf(&child4) == 3);
    }

    SECTION("stackUnder-unrelated") {
        Tui::ZWidget w;
        child1.stackUnder(&w);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }

    SECTION("stackUnder-nullptr") {
        widget.stackUnder(nullptr);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }
    SECTION("stackUnder-self") {
        widget.stackUnder(&widget);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);
    }

    SECTION("setStackingLayer-from5to4") {
        child1.setStackingLayer(5);
        layerConistencyCheck(&widget);
        child2.setStackingLayer(5);
        layerConistencyCheck(&widget);
        child3.setStackingLayer(5);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 0);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 2);

        child3.setStackingLayer(4);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 1);
        CHECK(widget.children().indexOf(&child2) == 2);
        CHECK(widget.children().indexOf(&child3) == 0);

        child2.setStackingLayer(4);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(&child1) == 2);
        CHECK(widget.children().indexOf(&child2) == 1);
        CHECK(widget.children().indexOf(&child3) == 0);
    }

}

TEST_CASE("widget-stackingLayer-destruct") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget widget(t.root);
    CHECK(widget.stackingLayer() == 0);

    Tui::ZWidget *child1 = new Tui::ZWidget(&widget);
    Tui::ZWidget *child2 = new Tui::ZWidget(&widget);
    Tui::ZWidget *child3 = new Tui::ZWidget(&widget);

    SECTION("delete2") {
        CHECK(widget.children().indexOf(child1) == 0);
        CHECK(widget.children().indexOf(child3) == 2);
        delete child2;
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(child1) == 0);
        CHECK(widget.children().indexOf(child3) == 1);
    }

    SECTION("delete2-setStackingLayer") {
        child1->setStackingLayer(1);
        layerConistencyCheck(&widget);
        child3->setStackingLayer(1);
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(child2) == 0);
        CHECK(widget.children().indexOf(child1) == 1);
        CHECK(widget.children().indexOf(child3) == 2);
        delete child2;
        layerConistencyCheck(&widget);
        CHECK(widget.children().indexOf(child1) == 0);
        CHECK(widget.children().indexOf(child3) == 1);
    }
}

TEST_CASE("widget-focus") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    Tui::ZWidget widget(&parent);
    Tui::ZWidget child(&widget);

    EventRecorder recorder;
    auto parentFocusIn = recorder.watchFocusInEvent(&parent, "parent focus in");
    auto parentFocusOut = recorder.watchFocusOutEvent(&parent, "parent focus out");
    auto widgetFocusIn = recorder.watchFocusInEvent(&widget, "widget focus in");
    auto widgetFocusOut = recorder.watchFocusOutEvent(&widget, "widget focus out");
    auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
    auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

    auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

    CHECK(widget.focusPolicy() == Tui::FocusPolicy::NoFocus);
    CHECK(widget.focusMode() == Tui::FocusContainerMode::None);
    CHECK(widget.focusOrder() == 0);
    CHECK(child.focusPolicy() == Tui::FocusPolicy::NoFocus);
    CHECK(child.focusMode() == Tui::FocusContainerMode::None);
    CHECK(child.focusOrder() == 0);

    SECTION("child") {
        Tui::FocusPolicy focusMode = GENERATE(Tui::FocusPolicy::NoFocus,
                                              Tui::FocusPolicy::TabFocus,
                                              Tui::FocusPolicy::ClickFocus,
                                              Tui::FocusPolicy::WheelFocus,
                                              Tui::FocusPolicy::StrongFocus);
        CAPTURE(focusMode);

        bool parentOrChild = GENERATE(true, false);
        CAPTURE(parentOrChild);

        widget.setFocusPolicy(focusMode);

        if (parentOrChild) {
            parent.setFocus();
            CHECK(recorder.consumeFirst(terminalFocusChange));
            CHECK(recorder.consumeFirst(parentFocusIn, Tui::OtherFocusReason));
            CHECK(recorder.noMoreEvents());

            CHECK(widget.isInFocusPath() == false);
            CHECK(child.isInFocusPath() == false);
            CHECK(t.terminal->focusWidget() == &parent);
        } else {
            child.setFocus();
            CHECK(recorder.consumeFirst(terminalFocusChange));
            CHECK(recorder.consumeFirst(childFocusIn, Tui::OtherFocusReason));
            CHECK(recorder.noMoreEvents());
            CHECK(widget.isInFocusPath() == true);
            CHECK(child.isInFocusPath() == true);
            CHECK(t.terminal->focusWidget() == &child);
        }

        CHECK(parent.isInFocusPath() == true);
        CHECK(parent.focusPolicy() == Tui::FocusPolicy::NoFocus);
        CHECK(parent.focusMode() == Tui::FocusContainerMode::None);
        CHECK(parent.focusOrder() == 0);

        CHECK(child.focusPolicy() == Tui::FocusPolicy::NoFocus);
        CHECK(child.focusMode() == Tui::FocusContainerMode::None);
        CHECK(child.focusOrder() == 0);

        CHECK(widget.focusPolicy() == focusMode);
    }

    SECTION("reasons") {
        auto focusReason = GENERATE(Tui::TabFocusReason, Tui::BacktabFocusReason, Tui::ActiveWindowFocusReason,
                                    Tui::ShortcutFocusReason, Tui::OtherFocusReason);
        widget.setFocus(focusReason);
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(widgetFocusIn, focusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(t.terminal->focusWidget() == &widget);
    }

    SECTION("widget-disable") {
        widget.setFocus();
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(widgetFocusIn, Tui::OtherFocusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(widget.focus() == true);
        CHECK(t.terminal->focusWidget() == &widget);
        widget.setEnabled(false);
        CHECK(recorder.consumeFirst(widgetFocusOut, Tui::OtherFocusReason));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.noMoreEvents());
        CHECK(child.focus() == false);
        CHECK(widget.focus() == false);
        CHECK(parent.focus() == false);
        CHECK(t.terminal->focusWidget() == nullptr);
    }
    SECTION("widget-disable-child-focus") {
        child.setFocus();
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(childFocusIn, Tui::OtherFocusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(child.focus() == true);
        CHECK(t.terminal->focusWidget() == &child);
        widget.setEnabled(false);
        CHECK(recorder.consumeFirst(childFocusOut, Tui::OtherFocusReason));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.noMoreEvents());
        CHECK(child.focus() == false);
        CHECK(widget.focus() == false);
        CHECK(parent.focus() == false);
        CHECK(t.terminal->focusWidget() == nullptr);
        widget.setEnabled(true);
        CHECK(recorder.noMoreEvents());
        CHECK(child.focus() == false);
        CHECK(widget.focus() == false);
        CHECK(parent.focus() == false);
        CHECK(t.terminal->focusWidget() == nullptr);
    }
    CHECK(recorder.noMoreEvents());
}

TEST_CASE("widget-focus-siblings") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget brother(t.root);
    Tui::ZWidget sister(t.root);

    EventRecorder recorder;
    auto brotherFocusIn = recorder.watchFocusInEvent(&brother, "brother focus in");
    auto brotherFocusOut = recorder.watchFocusOutEvent(&brother, "brother focus out");
    auto sisterFocusIn = recorder.watchFocusInEvent(&sister, "sister focus in");
    auto sisterFocusOut = recorder.watchFocusOutEvent(&sister, "sister focus out");

    auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

    CHECK(brother.focus() == false);
    CHECK(sister.focus() == false);
    CHECK(brother.isInFocusPath() == false);
    CHECK(sister.isInFocusPath() == false);

    SECTION("change-focus") {
        brother.setFocus();
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(brotherFocusIn, Tui::OtherFocusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(brother.focus() == true);
        CHECK(sister.focus() == false);
        CHECK(brother.isInFocusPath() == true);
        CHECK(sister.isInFocusPath() == false);
        CHECK(t.terminal->focusWidget() == &brother);

        sister.setFocus();
        CHECK(recorder.consumeFirst(brotherFocusOut, Tui::OtherFocusReason));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(sisterFocusIn, Tui::OtherFocusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(brother.focus() == false);
        CHECK(sister.focus() == true);
        CHECK(brother.isInFocusPath() == false);
        CHECK(sister.isInFocusPath() == true);
        CHECK(t.terminal->focusWidget() == &sister);
    }
    SECTION("change-enable") {
        Tui::FocusPolicy focusMode = GENERATE(Tui::FocusPolicy::NoFocus,
                                              Tui::FocusPolicy::TabFocus,
                                              Tui::FocusPolicy::ClickFocus,
                                              Tui::FocusPolicy::WheelFocus,
                                              Tui::FocusPolicy::StrongFocus);
        CAPTURE(focusMode);
        sister.setFocusPolicy(focusMode);

        brother.setFocus();
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(brotherFocusIn, Tui::OtherFocusReason));
        CHECK(recorder.noMoreEvents());
        CHECK(brother.focus() == true);
        CHECK(sister.focus() == false);
        CHECK(t.terminal->focusWidget() == &brother);

        brother.setEnabled(false);
        CHECK(recorder.consumeFirst(brotherFocusOut, Tui::OtherFocusReason));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(brother.focus() == false);
        if (focusMode == Tui::FocusPolicy::NoFocus ||
            focusMode == Tui::FocusPolicy::ClickFocus) {

            CHECK(sister.focus() == false);
            CHECK(sister.isInFocusPath() == false);
            CHECK(t.terminal->focusWidget() == nullptr);
        } else {
            CHECK(recorder.consumeFirst(sisterFocusIn, Tui::OtherFocusReason));
            CHECK(sister.focus() == true);
            CHECK(sister.isInFocusPath() == true);
            CHECK(t.terminal->focusWidget() == &sister);
        }
        CHECK(recorder.noMoreEvents());

        CHECK(brother.isInFocusPath() == false);
    }
    CHECK(recorder.noMoreEvents());
}

TEST_CASE("widget-cursor") {
    Testhelper t("unused", "unused", 8, 8);
    CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
    CHECK(t.terminal->grabCursorVisibility() == true);

    Tui::ZWidget parent(t.root);
    TestWidget widget(&parent);
    Tui::ZWidget child(&widget);

    SECTION("CursorStyle") {
        parent.setGeometry({0, 0, 2, 3});
        widget.setGeometry({0, 0, 2, 3});
        widget.setFocus();
        Tui::CursorStyle cs = GENERATE( Tui::CursorStyle::Bar,
                                        Tui::CursorStyle::Block,
                                        Tui::CursorStyle::Underline,
                                        Tui::CursorStyle::Unset);
        widget.setCursorStyle(cs);
        CHECK(parent.cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(child.cursorStyle() == Tui::CursorStyle::Unset);
        CHECK(widget.cursorStyle() == cs);

        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
        t.render();
        CHECK(t.terminal->grabCursorStyle() == cs);
        CHECK(t.terminal->grabCursorVisibility() == true);

        widget.resetCursorColor();
        CHECK(widget.cursorStyle() == cs);

        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
        t.render();
        CHECK(t.terminal->grabCursorStyle() == cs);
        CHECK(t.terminal->grabCursorVisibility() == true);

        SECTION("disable") {
            widget.setEnabled(false);
            CHECK(widget.cursorStyle() == cs);
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == cs);
            CHECK(t.terminal->grabCursorVisibility() == false);
        }
        SECTION("invisible") {
            widget.setVisible(false);
            CHECK(widget.cursorStyle() == cs);
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == cs);
            CHECK(t.terminal->grabCursorVisibility() == false);
        }
        SECTION("reset") {
            widget.setCursorStyle(Tui::CursorStyle::Unset);
            CHECK(widget.cursorStyle() == Tui::CursorStyle::Unset);
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
            CHECK(t.terminal->grabCursorVisibility() == true);
        }
        SECTION("child2") {
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };

            TestWidget child2(&widget);
            child2.setGeometry({0, 0, 3, 4});
            child2.setFocus();
            CHECK(widget.cursorStyle() == cs);
            CHECK(child2.cursorStyle() == Tui::CursorStyle::Unset);

            child2.paint = [&](Tui::ZPaintEvent*) { child2.showCursor(QPoint{0, 0}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
            CHECK(t.terminal->grabCursorVisibility() == true);
        }
        SECTION("root-child") {
            TestWidget child2(t.root);
            child2.setGeometry({0, 0, 3, 4});
            child2.setFocus();
            CHECK(widget.cursorStyle() == cs);
            CHECK(child2.cursorStyle() == Tui::CursorStyle::Unset);

            child2.paint = [&](Tui::ZPaintEvent*) { child2.showCursor(QPoint{1, 1}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
            CHECK(t.terminal->grabCursorVisibility() == true);
        }
    }

    SECTION("CursorColor") {
        parent.setGeometry({0, 0, 2, 4});
        widget.setGeometry({0, 0, 2, 4});
        child.setGeometry({0, 0, 2, 4});
        widget.setFocus();

        std::tuple<int, int, int> gen, check,  color, default_color = {-1, -1, -1};
        gen = GENERATE(std::make_tuple(0, 0, 0),
                         std::make_tuple(255, 255, 255),
                         std::make_tuple(-2, 4096, 65999),
                         std::make_tuple(-1, 0, 255),
                         std::make_tuple(4096, 0, 255),
                         std::make_tuple(0, -1, 255),
                         std::make_tuple(0, 4096, 255),
                         std::make_tuple(255, 0, -1),
                         std::make_tuple(255, 0, 2048)
                         );
        CAPTURE(gen);
        if (std::get<0>(gen) < 0 || std::get<1>(gen) < 0 || std::get<2>(gen) < 0) {
            check = default_color;
        } else {
            check = {std::min(std::get<0>(gen), 255),
                     std::min(std::get<1>(gen), 255),
                     std::min(std::get<2>(gen), 255)};
        }

        widget.setCursorColor(std::get<0>(gen), std::get<1>(gen), std::get<2>(gen));
        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
        t.render();
        color = t.terminal->grabCursorColor();
        CHECK(check == color);
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
        CHECK(t.terminal->grabCursorVisibility() == true);

        widget.resetCursorColor();
        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
        t.render();
        color = t.terminal->grabCursorColor();
        CHECK(default_color == color);
        CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);
        CHECK(t.terminal->grabCursorVisibility() == true);
    }

    SECTION("cursorColor-supressed") {
        widget.setCursorColor(8, 16, 32);
        std::tuple<int, int, int> check = std::make_tuple(-1, -1, -1);
        QPoint qp = QPoint{1, 1};

        QString checkwithout = GENERATE("focus",
                                        "parent.setGeometry",
                                        "widget.setGeometry",
                                        "qpoint");
        CAPTURE(checkwithout);

        if (checkwithout != "parent.setGeometry") {
            parent.setGeometry({0, 0, 2, 4});
        }
        if (checkwithout != "widget.setGeometry") {
            widget.setGeometry({0, 0, 2, 4});
        }
        if (checkwithout != "focus") {
            widget.setFocus();
        }
        if (checkwithout == "qpoint") {
            qp = QPoint{1, 5};
        }

        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(qp); };
        t.render();

        std::tuple<int, int, int> color = t.terminal->grabCursorColor();

        CHECK(check == color);
    }

    SECTION("cursorColor-overlapping-child") {
        widget.setCursorColor(8, 16, 32);
        parent.setGeometry({0, 0, 4, 8});
        widget.setGeometry({0, 0, 4, 8});
        child.setGeometry({0, 0, 2, 4});
        widget.setFocus();

        SECTION("1-1") {
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 1}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);

            // At position (1, 1) the widget is behind the child, the cursor is currently still displayed in that case.
            std::tuple<int, int, int> color = t.terminal->grabCursorColor();
            CHECK(std::make_tuple(8, 16, 32) == color);
            CHECK(t.terminal->grabCursorVisibility() == true);
        }
        SECTION("3-7") {
            widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{3, 7}); };
            t.render();
            CHECK(t.terminal->grabCursorStyle() == Tui::CursorStyle::Unset);

            // At position (3, 7) the cursor is next to the child, it is displayed here.
            std::tuple<int, int, int> color = t.terminal->grabCursorColor();
            CHECK(std::make_tuple(8, 16, 32) == color);
            CHECK(t.terminal->grabCursorVisibility() == true);
        }
    }

    SECTION("showCursor") {
        parent.setGeometry({0, 0, 2, 4});
        widget.setGeometry({0, 0, 2, 4});
        widget.setFocus();
        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(QPoint{1, 2}); };
        t.render();
        CHECK(t.terminal->grabCursorPosition() == QPoint{1, 2});
    }

    SECTION("showCursor-suppressed") {
        QPoint qp = QPoint{1, 2};

        QString checkwithout = GENERATE("focus",
                                        "parent.setGeometry",
                                        "widget.setGeometry",
                                        "qpoint");
        CAPTURE(checkwithout);

        if (checkwithout != "parent.setGeometry") {
            parent.setGeometry({0, 0, 2, 4});
        }
        if (checkwithout != "widget.setGeometry") {
            widget.setGeometry({0, 0, 2, 4});
        }
        if (checkwithout != "focus") {
            widget.setFocus();
        }
        if (checkwithout == "qpoint") {
            qp = QPoint{1, 5};
        }

        widget.paint = [&](Tui::ZPaintEvent*) { widget.showCursor(qp); };
        t.render();

        CHECK(t.terminal->grabCursorPosition() == QPoint{-1, -1});
    }

}

TEST_CASE("widget-min-maximumSize-base") {
    Testhelper t("unused", "unused", 2, 4);
    TestWidget widget(t.root);

    widget.setMinimumSize(QSize{-1, 99});
    CHECK(widget.minimumSize() == QSize{-1, 99});
    widget.setMinimumSize(QSize{99, -1});
    CHECK(widget.minimumSize() == QSize{99, -1});
    widget.setMinimumSize(QSize{77, 99});
    CHECK(widget.minimumSize() == QSize{77, 99});
    widget.setMinimumSize(70, 90);
    CHECK(widget.minimumSize() == QSize{70, 90});

    widget.setMaximumSize(QSize{-1, 99});
    CHECK(widget.maximumSize() == QSize{-1, 99});
    widget.setMaximumSize(QSize{99, -1});
    CHECK(widget.maximumSize() == QSize{99, -1});
    widget.setMaximumSize(QSize{77, 99});
    CHECK(widget.maximumSize() == QSize{77, 99});
    widget.setMaximumSize(70, 90);
    CHECK(widget.maximumSize() == QSize{70, 90});
}

TEST_CASE("widget-setFixedSize") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    TestWidget widget(&parent);
    Tui::ZWidget child(&widget);

    CHECK(parent.minimumSize() == QSize{-1, -1});
    CHECK(parent.maximumSize() == QSize{16777215, 16777215});
    CHECK(widget.minimumSize() == QSize{-1, -1});
    CHECK(widget.maximumSize() == QSize{16777215, 16777215});
    CHECK(child.minimumSize() == QSize{-1, -1});
    CHECK(child.maximumSize() == QSize{16777215, 16777215});

    SECTION("qsize") {
        widget.setFixedSize({16777216, 16777217});
        CHECK(parent.minimumSize() == QSize{-1, -1});
        CHECK(parent.maximumSize() == QSize{16777215, 16777215});
        CHECK(widget.minimumSize() == QSize{16777216, 16777217});
        CHECK(widget.maximumSize() == QSize{16777216, 16777217});
        CHECK(child.minimumSize() == QSize{-1, -1});
        CHECK(child.maximumSize() == QSize{16777215, 16777215});
    }
    SECTION("ints") {
        widget.setFixedSize(16777216, 16777217);
        CHECK(parent.minimumSize() == QSize{-1, -1});
        CHECK(parent.maximumSize() == QSize{16777215, 16777215});
        CHECK(widget.minimumSize() == QSize{16777216, 16777217});
        CHECK(widget.maximumSize() == QSize{16777216, 16777217});
        CHECK(child.minimumSize() == QSize{-1, -1});
        CHECK(child.maximumSize() == QSize{16777215, 16777215});
    }
}

TEST_CASE("widget-isEnabledTo") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    TestWidget widget(&parent);
    Tui::ZWidget child(&widget);

    CHECK(widget.isEnabledTo(&parent) == true);
    CHECK(widget.isEnabledTo(&child) == false);

    SECTION("widget-is-disabled") {
        widget.setEnabled(false);
        CHECK(widget.isEnabledTo(&parent) == false);
        CHECK(widget.isEnabledTo(&child) == false);
    }
    SECTION("parent-is-disabled") {
        parent.setEnabled(false);
        CHECK(widget.isEnabledTo(&parent) == true);
        CHECK(widget.isEnabledTo(&child) == false);
    }
    SECTION("child-is-disabled") {
        child.setEnabled(false);
        CHECK(widget.isEnabledTo(&parent) == true);
        CHECK(widget.isEnabledTo(&child) == false);
    }
}

TEST_CASE("widget-isVisibleTo") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    TestWidget widget(&parent);
    Tui::ZWidget child(&widget);

    CHECK(widget.isVisibleTo(&parent) == true);
    CHECK(widget.isVisibleTo(&child) == false);

    SECTION("widget-is-invisible") {
        widget.setVisible(false);
        CHECK(widget.isVisibleTo(&parent) == false);
        CHECK(widget.isVisibleTo(&child) == false);
    }
    SECTION("parent-is-invisible") {
        parent.setVisible(false);
        CHECK(widget.isVisibleTo(&parent) == true);
        CHECK(widget.isVisibleTo(&child) == false);
    }
    SECTION("child-is-invisible") {
        child.setVisible(false);
        CHECK(widget.isVisibleTo(&parent) == true);
        CHECK(widget.isVisibleTo(&child) == false);
    }

    widget.setVisible(false);

    SECTION("widget-is-visible") {
        widget.setVisible(true);
        CHECK(widget.isVisibleTo(&parent) == true);
        CHECK(widget.isVisibleTo(&child) == false);
    }
    SECTION("parent-set-visible") {
        parent.setVisible(true);
        CHECK(widget.isVisibleTo(&parent) == false);
        CHECK(widget.isVisibleTo(&child) == false);
    }
    SECTION("child-set-visible") {
        child.setVisible(true);
        CHECK(widget.isVisibleTo(&parent) == false);
        CHECK(widget.isVisibleTo(&child) == false);
    }
}

TEST_CASE("widget-isAncestorOf") {
    Testhelper t("unused", "unused", 2, 4);
    Tui::ZWidget parent(t.root);
    TestWidget widget(&parent);
    Tui::ZWidget child(&widget);
    Tui::ZWidget other(t.root);

    CHECK(child.isAncestorOf(&other) == false);
    CHECK(child.isAncestorOf(&widget) == false);
    CHECK(child.isAncestorOf(&parent) == false);
    CHECK(child.isAncestorOf(&child) == true);

    CHECK(other.isAncestorOf(&child) == false);
    CHECK(other.isAncestorOf(&other) == true);
    CHECK(other.isAncestorOf(&widget) == false);
    CHECK(other.isAncestorOf(&parent) == false);

    CHECK(parent.isAncestorOf(&child) == true);
    CHECK(parent.isAncestorOf(&other) == false);
    CHECK(parent.isAncestorOf(&widget) == true);
    CHECK(parent.isAncestorOf(&parent) == true);

    CHECK(widget.isAncestorOf(&child) == true);
    CHECK(widget.isAncestorOf(&other) == false);
    CHECK(widget.isAncestorOf(&widget) == true);
    CHECK(widget.isAncestorOf(&parent) == false);

    CHECK(t.root->isAncestorOf(&child) == true);
    CHECK(t.root->isAncestorOf(&other) == true);
    CHECK(t.root->isAncestorOf(&widget) == true);
    CHECK(t.root->isAncestorOf(&parent) == true);
}

TEST_CASE("widget-declare-layout-before-widget") {
    Testhelper t("unsued", "unused", 16, 5);

    // Must not crash or trigger sanitizers
    StubLayout layout;
    Tui::ZWidget outer;
    outer.setLayout(&layout);
    layout.setParent(nullptr);
}

TEST_CASE("widget-palette") {
    Testhelper t("unsued", "unused", 16, 5);

    Tui::ZWidget outer;
    Tui::ZWidget inner{&outer};
    Tui::ZWidget inner2{&inner};

    const Tui::ZColor errorColor = Tui::ZColor::fromRgb(0xff, 0, 0);
    const Tui::ZColor blue = Tui::Colors::blue;
    const Tui::ZColor green = Tui::Colors::green;
    const Tui::ZColor cyan = Tui::Colors::cyan;

    SECTION("default") {
        // check a few symbols to be undefined

        Tui::ZSymbol symbol = GENERATE(TUISYM_LITERAL("root.bg"),
                                       TUISYM_LITERAL("root.fg"),
                                       TUISYM_LITERAL("menu.bg"),
                                       TUISYM_LITERAL("menu.fg"));

        CHECK(outer.getColor(symbol) == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, symbol) == errorColor);
    }

    SECTION("base") {
        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == errorColor);

        Tui::ZPalette pal;
        outer.setPalette(pal);

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == errorColor);

        // pal is not implicitly shared or stored as reference
        pal.setColors({{"dummy", blue}});

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == errorColor);

        outer.setPalette(pal);
        CHECK(outer.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == blue);
        CHECK(inner.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == blue);
    }

    SECTION("rules") {
        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == errorColor);
        CHECK(inner2.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == errorColor);

        Tui::ZPalette pal;
        pal.setColors({
                          {"torefblue", blue},
                          {"torefgreen", green},
                      });
        pal.addRules({
                         {{"someclass", "additionalclass"}, {{ Tui::ZPalette::Publish, "dummy", "torefgreen"} }},
                         {{"someclass"}, {{ Tui::ZPalette::Publish, "dummy", "torefblue"} }},
                         {{"localclass"}, {{ Tui::ZPalette::Local, "dummy", "torefblue"} }},
                         {{"localclass", "additionalclass"}, {{ Tui::ZPalette::Local, "dummy", "torefgreen"} }}
                     });
        outer.setPalette(pal);

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == errorColor);
        CHECK(inner2.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == errorColor);

        inner.setPaletteClass({"someclass"});

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == blue);
        CHECK(inner2.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == blue);

        inner.setPaletteClass({"someclass", "additionalclass"});

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == green);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == green);
        CHECK(inner2.getColor("dummy") == green);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == green);

        inner.setPaletteClass({"localclass"});

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == blue);
        CHECK(inner2.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == errorColor);

        inner.setPaletteClass({"localclass", "additionalclass"});

        CHECK(outer.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == errorColor);
        CHECK(inner.getColor("dummy") == green);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == green);
        CHECK(inner2.getColor("dummy") == errorColor);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == errorColor);

        pal.setColors({
                          {"torefblue", blue},
                          {"torefgreen", green},
                          {"dummy", cyan}
                      });

        outer.setPalette(pal);

        inner.setPaletteClass({"localclass"});

        CHECK(outer.getColor("dummy") == cyan);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == cyan);
        CHECK(inner.getColor("dummy") == blue);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == blue);
        CHECK(inner2.getColor("dummy") == cyan);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == cyan);

        inner.setPaletteClass({"localclass", "additionalclass"});

        CHECK(outer.getColor("dummy") == cyan);
        CHECK(Tui::ZPalette::getColor(&outer, "dummy") == cyan);
        CHECK(inner.getColor("dummy") == green);
        CHECK(Tui::ZPalette::getColor(&inner, "dummy") == green);
        CHECK(inner2.getColor("dummy") == cyan);
        CHECK(Tui::ZPalette::getColor(&inner2, "dummy") == cyan);
    }

}

TEST_CASE("widget-event-methods") {
    Testhelper t("unused", "unused", 2, 4);

    EventRecorder recorder;

    TestWidgetEvents widget{recorder};

    CHECK(recorder.noMoreEvents());

    SECTION("paint event") {
        Tui::ZPainter painter = t.terminal->painter();
        Tui::ZPaintEvent event{&painter};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.paintCalledEvent, &event));
    }

    SECTION("key event") {
        Tui::ZKeyEvent event{Tui::Key_F1, Tui::ControlModifier, ""};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.keyCalledEvent, &event));
    }

    SECTION("paste event") {
        Tui::ZPasteEvent event{"some text"};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.pasteCalledEvent, &event));
    }

    SECTION("focus in event") {
        Tui::ZFocusEvent event{Tui::ZFocusEvent::focusIn};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.focusInCalledEvent, &event));
    }

    SECTION("focus out event") {
        Tui::ZFocusEvent event{Tui::ZFocusEvent::focusOut};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.focusOutCalledEvent, &event));
    }

    SECTION("resize event") {
        Tui::ZResizeEvent event{{12, 12}, {4, 4}};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.resizeCalledEvent, &event));
    }

    SECTION("move event") {
        Tui::ZMoveEvent event{{12, 12}, {4, 4}};
        QCoreApplication::sendEvent(&widget, &event);
        CHECK(recorder.consumeFirst(widget.moveCalledEvent, &event));
    }

}

TEST_CASE("widget-setParent") {
    Testhelper t("unused", "unused", 2, 4);

    EventRecorder recorder;

    auto translatorParentAboutToChange = [&](std::shared_ptr<EventRecorder::RecorderEvent> eventRef, const QEvent *event) {
        if (event->type() == QEvent::ParentAboutToChange) {
            recorder.recordEvent(eventRef);
        }
    };

    auto translatorParentChange = [&](std::shared_ptr<EventRecorder::RecorderEvent> eventRef, const QEvent *event) {
        if (event->type() == QEvent::ParentChange) {
            recorder.recordEvent(eventRef);
        }
    };

    auto translatorChildRemoved = [&](std::shared_ptr<EventRecorder::RecorderEvent> eventRef, const QEvent *ev) {
        if (ev->type() == QEvent::ChildRemoved) {
            auto &event = dynamic_cast<const QChildEvent&>(*ev);
            recorder.recordEvent(eventRef, event.child());
        }
    };

    auto translatorChildAdded = [&](std::shared_ptr<EventRecorder::RecorderEvent> eventRef, const QEvent *ev) {
        if (ev->type() == QEvent::ChildAdded) {
            auto &event = dynamic_cast<const QChildEvent&>(*ev);
            recorder.recordEvent(eventRef, event.child());
        }
    };

    SECTION("qt tree events") {
        Tui::ZWidget parent;
        Tui::ZWidget child;

        auto parentAboutToChange = recorder.watchEvent(&child, "parent about to change", translatorParentAboutToChange);
        auto parentChange = recorder.watchEvent(&child, "parent change", translatorParentChange);
        auto childRemoved = recorder.watchEvent(&parent, "child removed", translatorChildRemoved);
        auto childAdded = recorder.watchEvent(&parent, "child added", translatorChildAdded);

        child.setParent(&parent);

        CHECK(recorder.consumeFirst(parentAboutToChange));
        CHECK(recorder.consumeFirst(childAdded));
        CHECK(recorder.consumeFirst(parentChange));
        CHECK(recorder.noMoreEvents());

        CHECK(child.parent() == &parent);
        CHECK(child.parentWidget() == &parent);

        child.setParent(&parent);

        CHECK(recorder.noMoreEvents());

        CHECK(child.parent() == &parent);
        CHECK(child.parentWidget() == &parent);

        child.setParent(nullptr);

        CHECK(recorder.consumeFirst(parentAboutToChange));
        CHECK(recorder.consumeFirst(childRemoved));
        CHECK(recorder.consumeFirst(parentChange));
        CHECK(recorder.noMoreEvents());
    }


    SECTION("stacking same layer") {
        Tui::ZWidget parent;
        Tui::ZWidget w1{&parent};
        Tui::ZWidget w2{&parent};

        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 1);

        Tui::ZWidget child;

        child.setParent(&parent);
        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 1);
        CHECK(parent.children().indexOf(&child) == 2);
    }

    SECTION("stacking different layer") {
        Tui::ZWidget parent;
        Tui::ZWidget w1{&parent};
        Tui::ZWidget w2{&parent};

        w1.setStackingLayer(-1);
        w2.setStackingLayer(1);

        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 1);

        Tui::ZWidget child;

        child.setParent(&parent);
        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 2);
        CHECK(parent.children().indexOf(&child) == 1);
    }

    SECTION("stacking same parent") {
        // if parent does not change, stacking does not change either
        Tui::ZWidget parent;
        Tui::ZWidget w1{&parent};
        Tui::ZWidget child{&parent};
        Tui::ZWidget w2{&parent};

        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 2);
        CHECK(parent.children().indexOf(&child) == 1);


        child.setParent(&parent);
        CHECK(parent.children().indexOf(&w1) == 0);
        CHECK(parent.children().indexOf(&w2) == 2);
        CHECK(parent.children().indexOf(&child) == 1);
    }

    SECTION("visibility") {

        SECTION("old parent invisible - parent visible - child visible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            oldParent.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.consumeFirst(childShow));
            CHECK(recorder.noMoreEvents());
            CHECK(child.isVisible());
            CHECK(child.isLocallyVisible());
        }

        SECTION("old parent invisible - parent invisible - child visible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setVisible(false);
            oldParent.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(child.isLocallyVisible());
        }

        SECTION("old parent visible - parent visible - child visible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(child.isVisible());
            CHECK(child.isLocallyVisible());
        }

        SECTION("old parent visible - parent invisible - child visible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.consumeFirst(childHide));
            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(child.isLocallyVisible());
        }

        SECTION("old parent invisible - parent visible - child invisible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            oldParent.setVisible(false);
            child.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(!child.isLocallyVisible());
        }

        SECTION("old parent invisible - parent invisible - child invisible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setVisible(false);
            oldParent.setVisible(false);
            child.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(!child.isLocallyVisible());
        }

        SECTION("old parent visible - parent visible - child invisible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            child.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(!child.isLocallyVisible());
        }

        SECTION("old parent visible - parent invisible - child invisible") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setVisible(false);
            child.setVisible(false);

            auto childShowToParent = recorder.watchShowToParentEvent(&child, "child show to parent");
            auto childHideToParent = recorder.watchHideToParentEvent(&child, "child hide to parent");
            auto childShow = recorder.watchShowEvent(&child, "child show");
            auto childHide = recorder.watchHideEvent(&child, "child hide");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isVisible());
            CHECK(!child.isLocallyVisible());
        }

    }

    SECTION("enabled") {

        SECTION("old parent disabled - parent enabled - child enabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            oldParent.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.consumeFirst(childEnableChanged));
            CHECK(recorder.noMoreEvents());
            CHECK(child.isEnabled());
            CHECK(child.isLocallyEnabled());
        }

        SECTION("old parent disabled - parent disabled - child enabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setEnabled(false);
            oldParent.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(child.isLocallyEnabled());
        }

        SECTION("old parent enabled - parent enabled - child enabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(child.isEnabled());
            CHECK(child.isLocallyEnabled());
        }

        SECTION("old parent enabled - parent disabled - child enabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.consumeFirst(childEnableChanged));
            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(child.isLocallyEnabled());
        }

        SECTION("old parent disabled - parent enabled - child disabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            oldParent.setEnabled(false);
            child.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(!child.isLocallyEnabled());
        }

        SECTION("old parent disabled - parent disabled - child disabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setEnabled(false);
            oldParent.setEnabled(false);
            child.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(!child.isLocallyEnabled());
        }

        SECTION("old parent enabled - parent enabled - child disabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            child.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(!child.isLocallyEnabled());
        }

        SECTION("old parent enabled - parent disabled - child disabled") {
            Tui::ZWidget oldParent;
            Tui::ZWidget parent;
            Tui::ZWidget child{&oldParent};

            parent.setEnabled(false);
            child.setEnabled(false);

            auto childEnableChanged = recorder.watchEnabledChangeEvent(&child, "child enable changed");

            child.setParent(&parent);

            CHECK(recorder.noMoreEvents());
            CHECK(!child.isEnabled());
            CHECK(!child.isLocallyEnabled());
        }

    }


    SECTION("focus with attached widget - no previous focus") {
        Tui::ZWidget parent{t.root};
        Tui::ZWidget child;

        child.setFocus();
        CHECK(!child.focus());

        auto parentFocusIn = recorder.watchFocusInEvent(&parent, "parent focus in");
        auto parentFocusOut = recorder.watchFocusOutEvent(&parent, "parent focus out");
        auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
        auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

        auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        child.setParent(&parent);
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(childFocusIn));
        CHECK(recorder.noMoreEvents());

        CHECK(child.focus());
    }

    SECTION("focus with attached widget - priority focus on parent") {
        Tui::ZWidget parent{t.root};
        Tui::ZWidget child;

        child.setFocus();
        CHECK(!child.focus());

        parent.setFocus();
        CHECK(parent.focus());

        auto parentFocusIn = recorder.watchFocusInEvent(&parent, "parent focus in");
        auto parentFocusOut = recorder.watchFocusOutEvent(&parent, "parent focus out");
        auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
        auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

        auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        child.setParent(&parent);
        CHECK(recorder.noMoreEvents());

        CHECK(parent.focus());
        CHECK(!child.focus());
    }

    SECTION("focus with attached widget - priority focus on child") {
        Tui::ZWidget parent{t.root};
        Tui::ZWidget child;

        parent.setFocus();
        child.setFocus();
        CHECK(!child.focus());

        CHECK(parent.focus());

        auto parentFocusIn = recorder.watchFocusInEvent(&parent, "parent focus in");
        auto parentFocusOut = recorder.watchFocusOutEvent(&parent, "parent focus out");
        auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
        auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

        auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        child.setParent(&parent);
        CHECK(recorder.consumeFirst(parentFocusOut));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(childFocusIn));
        CHECK(recorder.noMoreEvents());

        CHECK(!parent.focus());
        CHECK(child.focus());
    }

    SECTION("reparent focused into invisible") {
        Tui::ZWidget w1{t.root};
        Tui::ZWidget w2{t.root};
        Tui::ZWidget child{t.root};

        w1.setVisible(false);
        w2.setFocusPolicy(Tui::StrongFocus);

        child.setFocus();

        auto w2FocusIn = recorder.watchFocusInEvent(&w2, "w2 focus in");
        auto w2FocusOut = recorder.watchFocusOutEvent(&w2, "w2 focus out");
        auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
        auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

        auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        child.setParent(&w1);

        CHECK(recorder.consumeFirst(childFocusOut));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(w2FocusIn));
        CHECK(w2.focus());
    }

    SECTION("reparent focused into disabled") {
        Tui::ZWidget w1{t.root};
        Tui::ZWidget w2{t.root};
        Tui::ZWidget child{t.root};

        w1.setEnabled(false);
        w2.setFocusPolicy(Tui::StrongFocus);

        child.setFocus();

        auto w2FocusIn = recorder.watchFocusInEvent(&w2, "w2 focus in");
        auto w2FocusOut = recorder.watchFocusOutEvent(&w2, "w2 focus out");
        auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
        auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

        auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

        child.setParent(&w1);

        CHECK(recorder.consumeFirst(childFocusOut));
        CHECK(recorder.consumeFirst(terminalFocusChange));
        CHECK(recorder.consumeFirst(w2FocusIn));
        CHECK(w2.focus());
    }

    SECTION("cross terminal") {
        t.terminal->setObjectName("terminal1");
        Tui::ZTerminal terminal2{Tui::ZTerminal::OffScreen{12, 12}};
        terminal2.setObjectName("terminal2");

        Tui::ZWidget oldParent;
        terminal2.setMainWidget(&oldParent);

        SECTION("disperse and transfer focus") {
            Tui::ZWidget w1{&oldParent};
            w1.setFocusPolicy(Tui::StrongFocus);

            Tui::ZWidget child{&oldParent};
            child.setFocusPolicy(Tui::StrongFocus);
            child.setFocus();

            auto w1FocusIn = recorder.watchFocusInEvent(&w1, "w1 focus in");
            auto w1FocusOut = recorder.watchFocusOutEvent(&w1, "w1 focus out");
            auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
            auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

            auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
            auto terminal2FocusChange = recorder.watchSignal(&terminal2, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));


            child.setParent(t.root);
            CHECK(recorder.consumeFirst(childFocusOut));
            CHECK(recorder.consumeFirst(terminal2FocusChange));
            CHECK(recorder.consumeFirst(w1FocusIn));
            CHECK(recorder.consumeFirst(terminalFocusChange));
            CHECK(recorder.consumeFirst(childFocusIn));
            CHECK(recorder.noMoreEvents());

            CHECK(w1.focus());
            CHECK(terminal2.focusWidget() == &w1);
            CHECK(child.focus());
            CHECK(t.terminal->focusWidget() == &child);
        }

        SECTION("disperse focus when new parent has priority focus on other widget") {
            Tui::ZWidget w1{&oldParent};
            w1.setFocusPolicy(Tui::StrongFocus);

            Tui::ZWidget child{&oldParent};
            child.setFocusPolicy(Tui::StrongFocus);
            child.setFocus();

            t.root->setFocus();

            auto w1FocusIn = recorder.watchFocusInEvent(&w1, "w1 focus in");
            auto w1FocusOut = recorder.watchFocusOutEvent(&w1, "w1 focus out");
            auto childFocusIn = recorder.watchFocusInEvent(&child, "child focus in");
            auto childFocusOut = recorder.watchFocusOutEvent(&child, "child focus out");

            auto terminalFocusChange = recorder.watchSignal(t.terminal.get(), RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));
            auto terminal2FocusChange = recorder.watchSignal(&terminal2, RECORDER_SIGNAL(&Tui::ZTerminal::focusChanged));

            child.setParent(t.root);
            CHECK(recorder.consumeFirst(childFocusOut));
            CHECK(recorder.consumeFirst(terminal2FocusChange));
            CHECK(recorder.consumeFirst(w1FocusIn));
            CHECK(recorder.noMoreEvents());

            CHECK(w1.focus());
            CHECK(terminal2.focusWidget() == &w1);
            CHECK(!child.focus());
            CHECK(t.terminal->focusWidget() == t.root);
        }

        SECTION("keyboard grab") {
            Tui::ZWidget child{&oldParent};
            child.grabKeyboard();
            CHECK(terminal2.keyboardGrabber() == &child);

            child.setParent(t.root);
            CHECK(terminal2.keyboardGrabber() == nullptr);
            CHECK(t.terminal->keyboardGrabber() == nullptr);
        }

        SECTION("terminal change event") {
            Tui::ZWidget child{&oldParent};

            auto terminalChangeEvent = recorder.watchTerminalChangeEvent(&child, "terminal change");

            child.setParent(t.root);

            CHECK(recorder.consumeFirst(terminalChangeEvent));
            CHECK(recorder.noMoreEvents());
        }

    }
}

TEST_CASE("widget-painting") {
    Testhelper t("unused", "unused", 80, 25);
    TestWidget root;

    EventRecorder recorder;

    t.terminal->setMainWidget(&root);

    TestWidget w1{&root};
    w1.setGeometry({2, 3, 4, 2});
    TestWidget w2{&root};
    w2.setGeometry({2, 7, 4, 2});
    TestWidget w3{&root};
    w3.setGeometry({2, 11, 4, 2});

    TestWidget childOfW2{&w2};
    childOfW2.setGeometry({0, 0, 1, 1});


    TestWidget w4{&root};
    w4.setGeometry({2, 13, 70, 12});
    TestWidget child1{&w4};
    child1.setGeometry({1, 1, 68, 10});
    TestWidget child2{&child1};
    child2.setGeometry({1, 1, 66, 8});
    TestWidget child3{&child2};
    child3.setGeometry({1, 1, 64, 6});

    RecorderEvent rootPaint = recorder.createEvent("root paint");
    root.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.clearWithChar(Tui::Colors::brown, Tui::Colors::green, 'x');
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(50));
        painter.setForeground(root.rect().width() - 1, root.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(51));
        recorder.recordEvent(rootPaint);
    };

    RecorderEvent w1Paint = recorder.createEvent("w1 paint");
    w1.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(52));
        painter.setForeground(w1.rect().width() - 1, w1.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(53));
        recorder.recordEvent(w1Paint);
    };

    RecorderEvent w2Paint = recorder.createEvent("w2 paint");
    w2.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(54));
        painter.setForeground(w2.rect().width() - 1, w2.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(55));
        recorder.recordEvent(w2Paint);
    };

    RecorderEvent childOfW2Paint = recorder.createEvent("childOfW2 paint");
    childOfW2.paint = [&](Tui::ZPaintEvent *event) {
        (void)event;
        recorder.recordEvent(childOfW2Paint);
    };

    RecorderEvent w3Paint = recorder.createEvent("w3 paint");
    w3.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(56));
        painter.setForeground(w3.rect().width() - 1, w3.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(57));
        recorder.recordEvent(w3Paint);
    };

    RecorderEvent w4Paint = recorder.createEvent("w4 paint");
    w4.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(58));
        // this will be over painted by child
        painter.setForeground(1, 1, Tui::ZColor::fromTerminalColorIndexed(255));
        painter.setForeground(w4.rect().width() - 1, w4.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(59));
        recorder.recordEvent(w4Paint);
    };

    RecorderEvent child1Paint = recorder.createEvent("child1 paint");
    child1.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(60));
        // this will be over painted by child
        painter.setForeground(1, 1, Tui::ZColor::fromTerminalColorIndexed(255));
        painter.setForeground(child1.rect().width() - 1, child1.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(61));
        recorder.recordEvent(child1Paint);
    };

    RecorderEvent child2Paint = recorder.createEvent("child2 paint");
    child2.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(62));
        // this will be over painted by child
        painter.setForeground(1, 1, Tui::ZColor::fromTerminalColorIndexed(255));
        painter.setForeground(child2.rect().width() - 1, child2.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(63));
        recorder.recordEvent(child2Paint);
    };

    RecorderEvent child3Paint = recorder.createEvent("child3 paint");
    child3.paint = [&](Tui::ZPaintEvent *event) {
        Tui::ZPainter painter = *event->painter();
        painter.setForeground(0, 0, Tui::ZColor::fromTerminalColorIndexed(64));
        painter.setForeground(child3.rect().width() - 1, child3.rect().height() - 1, Tui::ZColor::fromTerminalColorIndexed(65));
        recorder.recordEvent(child3Paint);
    };


    t.render();
    CHECK(recorder.consumeFirst(rootPaint));
    CHECK(recorder.consumeFirst(w1Paint));
    CHECK(recorder.consumeFirst(w2Paint));
    CHECK(recorder.consumeFirst(childOfW2Paint));
    CHECK(recorder.consumeFirst(w3Paint));
    CHECK(recorder.consumeFirst(w4Paint));
    CHECK(recorder.consumeFirst(child1Paint));
    CHECK(recorder.consumeFirst(child2Paint));
    CHECK(recorder.consumeFirst(child3Paint));
    CHECK(recorder.noMoreEvents());

    Tui::ZImage img = t.terminal->grabCurrentImage();
    CHECK(img.peekForground(0, 0) == Tui::ZColor::fromTerminalColorIndexed(50));
    CHECK(img.peekForground(79, 24) == Tui::ZColor::fromTerminalColorIndexed(51));

    CHECK(img.peekForground(2, 3) == Tui::ZColor::fromTerminalColorIndexed(52));
    CHECK(img.peekForground(2 + 3, 3 + 1) == Tui::ZColor::fromTerminalColorIndexed(53));

    CHECK(img.peekForground(2, 7) == Tui::ZColor::fromTerminalColorIndexed(54));
    CHECK(img.peekForground(2 + 3, 7 + 1) == Tui::ZColor::fromTerminalColorIndexed(55));

    CHECK(img.peekForground(2, 11) == Tui::ZColor::fromTerminalColorIndexed(56));
    CHECK(img.peekForground(2 + 3, 11 + 1) == Tui::ZColor::fromTerminalColorIndexed(57));

    CHECK(img.peekForground(2, 13) == Tui::ZColor::fromTerminalColorIndexed(58));
    CHECK(img.peekForground(71, 24) == Tui::ZColor::fromTerminalColorIndexed(59));

    CHECK(img.peekForground(3, 14) == Tui::ZColor::fromTerminalColorIndexed(60));
    CHECK(img.peekForground(70, 23) == Tui::ZColor::fromTerminalColorIndexed(61));

    CHECK(img.peekForground(4, 15) == Tui::ZColor::fromTerminalColorIndexed(62));
    CHECK(img.peekForground(69, 22) == Tui::ZColor::fromTerminalColorIndexed(63));

    CHECK(img.peekForground(5, 16) == Tui::ZColor::fromTerminalColorIndexed(64));
    CHECK(img.peekForground(68, 21) == Tui::ZColor::fromTerminalColorIndexed(65));

}

TEST_CASE("widget-sizes") {
    TestWidgetHints widget;

    widget.setGeometry({3, 5, 7, 13});

    CHECK(widget.minimumSize() == QSize{});
    CHECK(widget.maximumSize() == QSize{16777215, 16777215});
    CHECK(widget.effectiveSizeHint() == QSize{});
    CHECK(widget.effectiveMinimumSize() == QSize{});

    SECTION("layoutArea") {
        CHECK(widget.contentsMargins() == QMargins{});
        CHECK(widget.contentsRect() == QRect{0, 0, 7, 13});
        CHECK(widget.layoutArea() == QRect{0, 0, 7, 13});

        widget.setContentsMargins({2, 1, 4, 6});

        CHECK(widget.contentsMargins() == QMargins{2, 1, 4, 6});
        CHECK(widget.contentsRect() == QRect{2, 1, 1, 6});
        CHECK(widget.layoutArea() == QRect{2, 1, 1, 6});
    }

    SECTION("without hints") {
        CHECK(widget.effectiveSizeHint() == QSize{});
        widget.setMinimumSize(3, 5);
        CHECK(widget.effectiveSizeHint() == QSize{3, 5});
        CHECK(widget.effectiveMinimumSize() == QSize{3, 5});

        widget.setMaximumSize(2, 1);
        CHECK(widget.effectiveSizeHint() == QSize{2, 1});
        CHECK(widget.effectiveMinimumSize() == QSize{2, 1});
    }

    SECTION("with minimumSizeHint") {
        CHECK(widget.effectiveSizeHint() == QSize{});
        widget.setMinimumSize(3, 5);

        widget.minimumSizeHintOverride = QSize{4, 6};

        CHECK(widget.effectiveSizeHint() == QSize{3, 5});
        CHECK(widget.effectiveMinimumSize() == QSize{3, 5});

        widget.setMinimumSize(QSize{});
        CHECK(widget.effectiveSizeHint() == QSize{4, 6});
        CHECK(widget.effectiveMinimumSize() == QSize{4, 6});

        widget.setMaximumSize(2, 1);
        CHECK(widget.effectiveSizeHint() == QSize{2, 1});
        CHECK(widget.effectiveMinimumSize() == QSize{2, 1});
    }

    SECTION("with sizeHint") {
        CHECK(widget.effectiveSizeHint() == QSize{});

        widget.sizeHintOverride = {5, 3};

        CHECK(widget.effectiveSizeHint() == QSize{5, 3});

        SECTION("maximum size") {
            widget.setMaximumSize(2, 1);
            CHECK(widget.effectiveSizeHint() == QSize{2, 1});
        }

        SECTION("minimum size") {
            widget.setMinimumSize(10, 5);
            CHECK(widget.effectiveSizeHint() == QSize{10, 5});
        }

        SECTION("minimum size hint") {
            widget.minimumSizeHintOverride = QSize{10, 5};
            CHECK(widget.effectiveSizeHint() == QSize{10, 5});

            widget.setMaximumSize(2, 1);
            CHECK(widget.effectiveSizeHint() == QSize{2, 1});
        }

        SECTION("minimum size hint + minimum size") {
            widget.setMinimumSize(10, 5);
            widget.minimumSizeHintOverride = QSize{12, 7};
            CHECK(widget.effectiveSizeHint() == QSize{10, 5});

            widget.setMaximumSize(2, 1);
            CHECK(widget.effectiveSizeHint() == QSize{2, 1});
        }

    }
}
