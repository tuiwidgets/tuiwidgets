// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZListView.h>

#include <QStringListModel>

#include <Tui/ZPalette.h>

#include "../catchwrapper.h"
#include "../Testhelper.h"
#include "../signalrecorder.h"
#include "../vcheck_zwidget.h"

namespace {
    class SimpleModel : public QAbstractListModel {
    public:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            (void)parent;
            return rows.size();
        }

        QVariant data(const QModelIndex &index, int role) const override {
            return rows[index.row()].value(role, QVariant());
        }

        QMap<int, QVariant> itemData(const QModelIndex &index) const override {
            return rows[index.row()];
        }

        QVector<QMap<int, QVariant>> rows;
    };
}

TEST_CASE("listview-base", "") {
    bool parent = GENERATE(false, true);
    CAPTURE(parent);

    std::unique_ptr<Tui::ZWidget> w = parent ? std::make_unique<Tui::ZWidget>() : nullptr;

    SECTION("constructor") {
        // The initialisation must not break crash.
        std::unique_ptr<Tui::ZListView> lv = std::make_unique<Tui::ZListView>(w.get());
        CHECK(lv->sizePolicyH() == Tui::SizePolicy::Expanding);
        CHECK(lv->sizePolicyV() == Tui::SizePolicy::Expanding);
        CHECK(lv->focusPolicy() == Tui::StrongFocus);
        FAIL_CHECK_VEC(checkWidgetsDefaultsExcept(lv.get(), DefaultException::SizePolicyV
                                                          | DefaultException::SizePolicyH
                                                          | DefaultException::FocusPolicy));
    }

    std::unique_ptr<Tui::ZListView> lv = std::make_unique<Tui::ZListView>(w.get());

    SECTION("abi-vcheck") {
        Tui::ZWidget base;
        checkZWidgetOverrides(&base, lv.get());
    }

    SECTION("setItems") {
        lv->setItems({"a", "b", "x"});
        CHECK(lv->items() == QStringList{"a", "b", "x"});
    }

    SECTION("model") {
        SimpleModel model;
        lv->setModel(&model);
        CHECK(lv->model() == &model);
        // model goes out of scope, this should be handled in ZListView
    }

    SECTION("currentIndex-empty") {
        CHECK(lv->currentIndex() == QModelIndex());
        lv->setCurrentIndex(QModelIndex());
        CHECK(lv->currentIndex() == QModelIndex());
    }

    SECTION("currentIndex-custom-model") {
        bool withPreviousModel = GENERATE(false, true);
        CAPTURE(withPreviousModel);
        if (withPreviousModel) {
            lv->setItems({"X", "Y", "Z"});
            lv->setCurrentIndex(lv->model()->index(1, 0));
        }
        SimpleModel model;
        model.rows = {{{Qt::DisplayRole, "A"}}, {{Qt::DisplayRole, "B"}}};
        lv->setModel(&model);
        CHECK(lv->currentIndex() == model.index(0, 0));
        CHECK(lv->currentItem() == "A");
        lv->setCurrentIndex(model.index(1, 0));
        CHECK(lv->currentIndex() == model.index(1, 0));
        CHECK(lv->currentItem() == "B");
        CHECK(lv->items() == QStringList{"A", "B"});

        lv->setItems({"M", "N", "O"});
        CHECK(lv->currentIndex() == lv->model()->index(0, 0));
        CHECK(lv->items() == QStringList{"M", "N", "O"});
    }

    SECTION("other") {
        // must not crash
        lv->currentItem();
        lv->model();
        lv->selectionModel();
        lv->scrollTo(QModelIndex(), Tui::ZListView::EnsureVisible);
        lv->scrollTo(QModelIndex(), Tui::ZListView::PositionAtTop);
        lv->scrollTo(QModelIndex(), Tui::ZListView::PositionAtBottom);
        lv->scrollTo(QModelIndex(), Tui::ZListView::PositionAtCenter);
    }
}

TEST_CASE("listview", "") {
    Testhelper t("listview", "listview", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    w->setGeometry({0, 0, 15, 5});

    // The initialisation must not break down.
    SECTION("default") {
        new Tui::ZListView(w);
        t.compare();
    }

    SECTION("geometry") {
        Tui::ZListView *lv = new Tui::ZListView(w);
        lv->setGeometry({1, 1, 13, 1});
        t.compare();
    }

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});

    bool hasFocus = GENERATE(false, true);

    if (hasFocus) {
        t.namePrefix += "-focus";
        lv1->setFocus();
        UNSCOPED_INFO("focus");
    }

    SECTION("items-empty") {
        //must not break
        lv1->setFocus();
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Enter);
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_End);
        t.sendKey(Tui::Key_PageUp);
        t.sendKey(Tui::Key_PageDown);
        t.sendKey(Tui::Key_Space);
        CHECK(lv1->items().empty());
        CHECK(lv1->currentIndex().isValid() == false);
        CHECK(lv1->currentItem() == "");
    }

    QStringList qsl;
    qsl.append("A");

    SECTION("items-A") {
        lv1->setItems(qsl);
        lv1->setFocus();
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Enter);
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_End);
        t.sendKey(Tui::Key_PageUp);
        t.sendKey(Tui::Key_PageDown);
        t.sendKey(Tui::Key_Space);
        CHECK(!lv1->items().empty());
        CHECK(lv1->currentIndex().row() == 0);
        CHECK(lv1->currentItem() == "A");
    }

    qsl.append("B");

    SECTION("items-AB") {
        lv1->setItems(qsl);
        lv1->setFocus();
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Enter);
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_End);
        t.sendKey(Tui::Key_PageUp);
        t.sendKey(Tui::Key_PageDown);
        t.sendKey(Tui::Key_Space);
        CHECK(!lv1->items().empty());
        CHECK(lv1->currentItem() == "B");
        CHECK(lv1->currentIndex().row() == 1);
    }

    qsl.append("C");

    SECTION("items-ABC") {
        lv1->setItems(qsl);
        lv1->setFocus();
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Enter);
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_End);
        t.sendKey(Tui::Key_PageUp);
        t.sendKey(Tui::Key_PageDown);
        t.sendKey(Tui::Key_Space);
        CHECK(!lv1->items().empty());
        CHECK(lv1->currentItem() == "C");
        CHECK(lv1->currentIndex().row() == 2);
    }

    qsl.append("D");

    SECTION("items-ABCD") {
        lv1->setItems(qsl);
        lv1->setFocus();
        CHECK(lv1->currentItem() == "A");
        CHECK(lv1->currentIndex().row() == 0);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == "B");
        CHECK(lv1->currentIndex().row() == 1);
        t.sendKey(Tui::Key_Up);
        CHECK(lv1->currentItem() == "A");
        CHECK(lv1->currentIndex().row() == 0);
        t.sendKey(Tui::Key_Enter);
        t.sendKey(Tui::Key_Home);
        CHECK(lv1->currentItem() == "A");
        CHECK(lv1->currentIndex().row() == 0);
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == "D");
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_PageUp);
        CHECK(lv1->currentItem() == "A");
        CHECK(lv1->currentIndex().row() == 0);
        t.sendKey(Tui::Key_PageDown);
        CHECK(!lv1->items().empty());
        CHECK(lv1->currentItem() == "D");
        CHECK(lv1->currentIndex().row() == 3);
    }

    SECTION("items-ABC") {
        lv1->setItems(qsl);
        CHECK(!lv1->items().empty());
        CHECK(lv1->items() == qsl);
        t.compare();
    }

    SECTION("items-custom-model-ABC") {
        SimpleModel model;
        // Last item is clipped
        model.rows = {{{Qt::DisplayRole, "A"}}, {{Qt::DisplayRole, "B"}},
                      {{Qt::DisplayRole, "C"}}, {{Qt::DisplayRole, "D"}}};
        lv1->setModel(&model);

        CHECK(!lv1->items().empty());
        CHECK(lv1->items() == qsl);
        t.compare("items-ABC");
    }

    lv1->setItems(qsl);

    SECTION("disable") {
        lv1->setEnabled(false);
        t.compare();
    }

    SECTION("currentItem") {
        CHECK(lv1->currentItem() == "A");
    }

    SECTION("currentIndex") {
        CHECK(lv1->currentIndex().row() == 0);
    }

    SECTION("itemAppend") {
        lv1->setCurrentIndex(lv1->model()->index(1, 0));
        CHECK(lv1->currentItem() == "B");
        qsl.append("C");
        lv1->setItems(qsl);
        CHECK(lv1->currentItem() == "A");
    }

    // XXX margins are currently ignored
    //SECTION("margins") {
    //    lv1->contentsMargins();
    //    lv1->setContentsMargins({2, 2, 2, 2});
    //    //t.compare();
    //}

    SECTION("left-deco-textonly") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}, {Tui::LeftDecorationRole, QString("X ")}},
            {{ Qt::DisplayRole, QString("B")}},
            {{ Qt::DisplayRole, QString("C")}},
        };

        lv1->setModel(&model);
        t.compare();
        lv1->setItems({});
    }

    SECTION("left-deco-textonly-not-selected") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}, {Tui::LeftDecorationRole, QString("X ")}},
            {{ Qt::DisplayRole, QString("B")}},
            {{ Qt::DisplayRole, QString("C")}},
        };

        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(1, 0));
        t.compare();
        lv1->setItems({});
    }

    SECTION("left-deco-text-and-fgcolor") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}},
            {{ Qt::DisplayRole, QString("B")}},
            {{ Qt::DisplayRole, QString("C")},
             {Tui::LeftDecorationRole, QString("X ")},
             {Tui::LeftDecorationFgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::red))}},
        };

        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(2, 0));
        t.compare();
    }

    SECTION("left-deco-text-and-fgcolor-not-selected") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}},
            {{ Qt::DisplayRole, QString("B")}},
            {{ Qt::DisplayRole, QString("C")},
             {Tui::LeftDecorationRole, QString("X ")},
             {Tui::LeftDecorationFgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::red))}},
        };

        lv1->setModel(&model);
        t.compare();
    }

    SECTION("left-deco-text-and-bgcolor-space") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}, {Tui::LeftDecorationSpaceRole, 3}},
            {{ Qt::DisplayRole, QString("B")}, {Tui::LeftDecorationSpaceRole, 4}},
            {{ Qt::DisplayRole, QString("C")},
             {Tui::LeftDecorationRole, QString("X ")},
             {Tui::LeftDecorationSpaceRole, 1},
             {Tui::LeftDecorationBgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::red))}},
        };

        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(2, 0));
        t.compare();
    }

    SECTION("left-deco-text-and-bgcolor-space-not-selected") {
        SimpleModel model;
        model.rows = {
            {{ Qt::DisplayRole, QString("A")}, {Tui::LeftDecorationSpaceRole, 3}},
            {{ Qt::DisplayRole, QString("B")}, {Tui::LeftDecorationSpaceRole, 4}},
            {{ Qt::DisplayRole, QString("C")},
             {Tui::LeftDecorationRole, QString("X ")},
             {Tui::LeftDecorationSpaceRole, 1},
             {Tui::LeftDecorationBgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::red))}},
        };

        lv1->setModel(&model);
        t.compare();
    }

    auto charTest = [&] {
        SimpleModel model;
        model.rows = {
            {
                { Qt::DisplayRole, QString("Aaä")},
                {Tui::LeftDecorationRole, QString("Aa")},
                {Tui::LeftDecorationSpaceRole, 1}
            },
            {
                { Qt::DisplayRole, QString("B😎")},
                {Tui::LeftDecorationRole, QString("あ")},
                {Tui::LeftDecorationSpaceRole, 1}
            },
            {
                { Qt::DisplayRole, QString("Cあ")},
                {Tui::LeftDecorationRole, QString("😎")},
                {Tui::LeftDecorationSpaceRole, 1},
                {Tui::LeftDecorationBgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::red))}
            },
        };

        lv1->setModel(&model);
        lv1->setCurrentIndex(lv1->model()->index(1, 0));
        t.compare();
    };

    SECTION("left-deco-chars") {
        charTest();
    }

    SECTION("left-deco-chars-width-8") {
        lv1->setGeometry({1, 1, 8, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-7") {
        lv1->setGeometry({1, 1, 7, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-6") {
        lv1->setGeometry({1, 1, 6, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-5") {
        lv1->setGeometry({1, 1, 5, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-4") {
        lv1->setGeometry({1, 1, 4, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-3") {
        lv1->setGeometry({1, 1, 3, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-2") {
        lv1->setGeometry({1, 1, 2, 3});
        charTest();
    }

    SECTION("left-deco-chars-width-1") {
        lv1->setGeometry({1, 1, 1, 3});
        charTest();
    }

}

TEST_CASE("search", "") {
    Testhelper t("listview", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});
    lv1->setFocus();

    SECTION("Basics") {
        lv1->setItems({"Atest1", "Atest2", "Btest3", "Atest4", "Ctest5"});
        struct TestCase { QString ch; int start; int result; };
        auto testCase = GENERATE( TestCase{"a", 0, 1},
                                  TestCase{"b", 0, 2},
                                  TestCase{"b", 2, 2},
                                  TestCase{"a", 2, 3},
                                  TestCase{"c", 0, 4},
                                  TestCase{"d", 0, 0},
                                  TestCase{"z", 0, 0},
                                  TestCase{"a", 1, 3},
                                  TestCase{"a", 3, 0},
                                  TestCase{"a", 4, 0},
                                  TestCase{"A", 0, 1},
                                  TestCase{"B", 0, 2},
                                  TestCase{"B", 2, 2},
                                  TestCase{"A", 2, 3},
                                  TestCase{"C", 0, 4},
                                  TestCase{"D", 0, 0},
                                  TestCase{"Z", 0, 0},
                                  TestCase{"A", 1, 3},
                                  TestCase{"A", 3, 0},
                                  TestCase{"A", 4, 0});
        CAPTURE(testCase.ch);
        CAPTURE(testCase.start);
        CAPTURE(testCase.result);

        lv1->setCurrentIndex(lv1->model()->index(testCase.start, 0));
        t.sendChar(testCase.ch);
        CHECK(lv1->currentIndex() == lv1->model()->index(testCase.result, 0));
    }

    SECTION("Chars") {
        lv1->setItems({"😎test1", "あtest2", "", "\n", "\t", "2test3", "Atest4", "atest5"});
        struct TestCase { QString ch; int start; int result; };
        auto testCase = GENERATE( TestCase{"😎", 5, 0},
                                  TestCase{"あ", 0, 1},
                                  TestCase{"2", 0, 5},
                                  TestCase{"A", 0, 6},
                                  TestCase{"a", 0, 6},
                                  TestCase{"A", 6, 7},
                                  TestCase{"a", 6, 7},
                                  TestCase{" ", 0, 0},
                                  TestCase{"\n", 0, 0});
        CAPTURE(testCase.ch);
        CAPTURE(testCase.start);
        CAPTURE(testCase.result);

        lv1->setCurrentIndex(lv1->model()->index(testCase.start, 0));
        if (testCase.ch == " ") {
            t.sendKey(Tui::Key_Space);
        } else if (testCase.ch == "\n") {
            t.sendKey(Tui::Key_Enter);
        } else {
            t.sendChar(testCase.ch);
        }
        CHECK(lv1->currentIndex() == lv1->model()->index(testCase.result, 0));
    }
}

TEST_CASE("emit", "") {
    Testhelper t("listview", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});
    lv1->setFocus();

    QStringList qsl;
    qsl.append("A");
    qsl.append("B");
    qsl.append("C");
    qsl.append("D");
    lv1->setItems(qsl);

    SECTION("enter") {
        int trigger = -1;
        QObject::connect(lv1, &Tui::ZListView::enterPressed, [&trigger](int value) {
            CHECK(trigger == value);
            trigger = -1;
        });

        lv1->setFocus();
        trigger = 0;
        t.sendKey(Tui::Key_Enter);
        CHECK(trigger == -1);

        t.sendKey(Tui::Key_Down);
        trigger = 1;
        t.sendKey(Tui::Key_Enter);
        CHECK(trigger == -1);

        t.sendKey(Tui::Key_Down);
        trigger = 2;
        t.sendKey(Tui::Key_Enter);
        CHECK(trigger == -1);

        //Nothing more must happen here
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Up);
        t.sendKey(Tui::Key_Home);
        t.sendKey(Tui::Key_End);
        t.sendKey(Tui::Key_PageUp);
        t.sendKey(Tui::Key_PageDown);
        t.sendKey(Tui::Key_Space);
        CHECK(trigger == -1);
    }
}

TEST_CASE("listview-key", "") {
    Testhelper t("listview", "listview-key", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setFocusPolicy(Tui::FocusPolicy::StrongFocus);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});
    lv1->setFocus();

    QStringList qsl;
    qsl.append("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    qsl.append("bcdefghijklmnopqsrtuvwxyz1");
    qsl.append("CDEFGHIJKLMNOPQRSTUVWXYZ12");
    qsl.append("defghijklmnopqsrtuvwxyz123");
    lv1->setItems(qsl);

    SECTION("1down") {
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(1));
        CHECK(lv1->currentIndex().row() == 1);
        t.compare();
    }

    SECTION("2down") {
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(1));
        CHECK(lv1->currentIndex().row() == 1);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(2));
        CHECK(lv1->currentIndex().row() == 2);
        t.compare();
    }

    SECTION("3down") {
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(1));
        CHECK(lv1->currentIndex().row() == 1);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(2));
        CHECK(lv1->currentIndex().row() == 2);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(3));
        CHECK(lv1->currentIndex().row() == 3);
        t.compare();
    }

    SECTION("4down") {
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(3));
        CHECK(lv1->currentIndex().row() == 3);
        t.compare("3down");
    }

    SECTION("end") {
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == qsl.last());
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == qsl.last());
        CHECK(lv1->currentIndex().row() == 3);
    }

    SECTION("home") {
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == qsl.last());
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_Home);
        CHECK(lv1->currentItem() == qsl.at(0));
        CHECK(lv1->currentIndex().row() == 0);
    }

    SECTION("pagedown") {
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == qsl.at(3));
        CHECK(lv1->currentIndex().row() == 3);
    }

    SECTION("pageup") {
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == qsl.at(3));
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_PageUp);
        CHECK(lv1->currentItem() == qsl.at(0));
        CHECK(lv1->currentIndex().row() == 0);
    }

    SECTION("left") {
        t.sendKey(Tui::Key_Left);
        CHECK(lv1->currentItem() == qsl.at(0));
        CHECK(lv1->currentIndex().row() == 0);
        t.compare();
    }

    SECTION("right") {
        t.sendKey(Tui::Key_Right);
        CHECK(lv1->currentItem() == qsl.at(0));
        CHECK(lv1->currentIndex().row() == 0);
        t.compare("left");
    }

    SECTION("disable-down") {
        lv1->setFocus();
        lv1->setEnabled(false);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == qsl.at(0));
        CHECK(lv1->currentIndex().row() == 0);
        t.compare();
    }
}

TEST_CASE("listview-key30", "") {

    Testhelper t("listview", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});
    lv1->setFocus();

    QStringList qsl;
    for (int i = 1; i <= 30; i++) {
        qsl.append(QString::number(i));
    }
    lv1->setItems(qsl);

    SECTION("end") {
       t.sendKey(Tui::Key_End);
       CHECK(lv1->currentItem() == qsl.last());
       CHECK(lv1->currentIndex().row() == 29);
    }

    SECTION("pagedown") {
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "4");
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "7");
        CHECK(lv1->currentIndex().row() == 6);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "10");
        CHECK(lv1->currentIndex().row() == 9);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "13");
        CHECK(lv1->currentIndex().row() == 12);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "16");
        CHECK(lv1->currentIndex().row() == 15);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "19");
        CHECK(lv1->currentIndex().row() == 18);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "22");
        CHECK(lv1->currentIndex().row() == 21);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "25");
        CHECK(lv1->currentIndex().row() == 24);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "30");
        CHECK(lv1->currentIndex().row() == 29);
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "30");
        CHECK(lv1->currentIndex().row() == 29);
    }

    SECTION("pageup") {
        t.sendKey(Tui::Key_PageDown);
        CHECK(lv1->currentItem() == "4");
        CHECK(lv1->currentIndex().row() == 3);
        t.sendKey(Tui::Key_Down);
        CHECK(lv1->currentItem() == "5");
        CHECK(lv1->currentIndex().row() == 4);
        t.sendKey(Tui::Key_PageUp);
        CHECK(lv1->currentItem() == "3");
        CHECK(lv1->currentIndex().row() == 2);
        t.sendKey(Tui::Key_PageUp);
        CHECK(lv1->currentItem() == "1");
        CHECK(lv1->currentIndex().row() == 0);
    }

    SECTION("end-home") {
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == "30");
        CHECK(lv1->currentIndex().row() == 29);
        t.sendKey(Tui::Key_Home);
        CHECK(lv1->currentItem() == "1");
        CHECK(lv1->currentIndex().row() == 0);
        t.sendKey(Tui::Key_End);
        CHECK(lv1->currentItem() == "30");
        CHECK(lv1->currentIndex().row() == 29);
    }
}

TEST_CASE("listview-scrollTo4", "") {

    Testhelper t("listview", "listview-scrollTo4", 15, 6);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 6});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 4});
    lv1->setFocus();

    QStringList qsl;
    for (int i = 1; i <= 30; i++) {
        qsl.append(QString::number(i));
    }
    lv1->setItems(qsl);

    SECTION("start-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("start-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtTop);
        t.compare("1");
    }
    SECTION("start-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("start-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("2-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("2-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtTop);
        t.compare("1");
    }
    SECTION("2-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("2-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("3-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("3-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtTop);
        t.compare("2");
    }
    SECTION("3-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("3-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("4-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::EnsureVisible);
        t.compare("2");
    }
    SECTION("4-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtTop);
        t.compare("3");
    }
    SECTION("4-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtBottom);
        t.compare("2");
    }
    SECTION("4-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtCenter);
        t.compare("2");
    }

    SECTION("15-up-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::EnsureVisible);
        t.compare("13");
    }
    SECTION("15-up-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtTop);
        t.compare("14");
    }
    SECTION("15-up-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtBottom);
        t.compare("13");
    }
    SECTION("15-up-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtCenter);
        t.compare("13");
    }

    SECTION("15-down-ev") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::EnsureVisible);
        t.compare("13");
    }
    SECTION("15-down-top") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtTop);
        t.compare("14");
    }
    SECTION("15-down-bottom") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtBottom);
        t.compare("13");
    }
    SECTION("15-down-center") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtCenter);
        t.compare("13");
    }

    SECTION("15-16-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::EnsureVisible);
        t.compare("13");
        // 16 is visible but at at the very edge
        lv1->scrollTo(lv1->model()->index(15, 0), Tui::ZListView::EnsureVisible);
        t.compare("14");
    }
    SECTION("16-14-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(15, 0), Tui::ZListView::EnsureVisible);
        t.compare("14");
        // 14 is visible but at at the very edge
        lv1->scrollTo(lv1->model()->index(13, 0), Tui::ZListView::EnsureVisible);
        t.compare("13");
    }

    SECTION("27-ev") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::EnsureVisible);
       t.compare("25");
    }
    SECTION("27-top") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtTop);
       t.compare("26");
    }
    SECTION("27-bottom") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtBottom);
       t.compare("25");
    }
    SECTION("27-center") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtCenter);
       t.compare("25");
    }

    SECTION("28-ev") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::EnsureVisible);
       t.compare("26");
    }
    SECTION("28-top") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtTop);
       t.compare("27");
    }
    SECTION("28-bottom") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtBottom);
       t.compare("26");
    }
    SECTION("28-center") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtCenter);
       t.compare("26");
    }

    SECTION("29-ev") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::EnsureVisible);
       t.compare("27");
    }
    SECTION("29-top") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtTop);
       t.compare("27");
    }
    SECTION("29-bottom") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtBottom);
       t.compare("27");
    }
    SECTION("29-center") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtCenter);
       t.compare("27");
    }

    SECTION("end-ev") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
       t.compare("27");
    }
    SECTION("end-top") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtTop);
       t.compare("27");
    }
    SECTION("end-bottom") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtBottom);
       t.compare("27");
    }
    SECTION("end-center") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtCenter);
       t.compare("27");
    }
}

TEST_CASE("listview-scrollTo5", "") {

    Testhelper t("listview", "listview-scrollTo5", 15, 7);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 7});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 5});
    lv1->setFocus();

    QStringList qsl;
    for (int i = 1; i <= 30; i++) {
        qsl.append(QString::number(i));
    }
    lv1->setItems(qsl);

    SECTION("start-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("start-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtTop);
        t.compare("1");
    }
    SECTION("start-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("start-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("2-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("2-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtTop);
        t.compare("1");
    }
    SECTION("2-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("2-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(1, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("3-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("3-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtTop);
        t.compare("2");
    }
    SECTION("3-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("3-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(2, 0), Tui::ZListView::PositionAtCenter);
        t.compare("1");
    }

    SECTION("4-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::EnsureVisible);
        t.compare("1");
    }
    SECTION("4-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtTop);
        t.compare("3");
    }
    SECTION("4-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtBottom);
        t.compare("1");
    }
    SECTION("4-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(3, 0), Tui::ZListView::PositionAtCenter);
        t.compare("2");
    }

    SECTION("5-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(4, 0), Tui::ZListView::EnsureVisible);
        t.compare("2");
    }
    SECTION("5-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(4, 0), Tui::ZListView::PositionAtTop);
        t.compare("4");
    }
    SECTION("5-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(4, 0), Tui::ZListView::PositionAtBottom);
        t.compare("2");
    }
    SECTION("5-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(4, 0), Tui::ZListView::PositionAtCenter);
        t.compare("3");
    }

    SECTION("15-up-ev") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::EnsureVisible);
        t.compare("12");
    }
    SECTION("15-up-top") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtTop);
        t.compare("14");
    }
    SECTION("15-up-bottom") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtBottom);
        t.compare("12");
    }
    SECTION("15-up-center") {
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtCenter);
        t.compare("13");
    }

    SECTION("15-down-ev") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::EnsureVisible);
        t.compare("12");
    }
    SECTION("15-down-top") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtTop);
        t.compare("14");
    }
    SECTION("15-down-bottom") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtBottom);
        t.compare("12");
    }
    SECTION("15-down-center") {
        lv1->scrollTo(lv1->model()->index(0, 0), Tui::ZListView::EnsureVisible);
        lv1->scrollTo(lv1->model()->index(14, 0), Tui::ZListView::PositionAtCenter);
        t.compare("13");
    }

    SECTION("26-ev") {
       lv1->scrollTo(lv1->model()->index(25, 0), Tui::ZListView::EnsureVisible);
       t.compare("23");
    }
    SECTION("26-top") {
       lv1->scrollTo(lv1->model()->index(25, 0), Tui::ZListView::PositionAtTop);
       t.compare("25");
    }
    SECTION("26-bottom") {
       lv1->scrollTo(lv1->model()->index(25, 0), Tui::ZListView::PositionAtBottom);
       t.compare("23");
    }
    SECTION("26-center") {
       lv1->scrollTo(lv1->model()->index(25, 0), Tui::ZListView::PositionAtCenter);
       t.compare("24");
    }

    SECTION("27-ev") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::EnsureVisible);
       t.compare("24");
    }
    SECTION("27-top") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtTop);
       t.compare("26");
    }
    SECTION("27-bottom") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtBottom);
       t.compare("24");
    }
    SECTION("27-center") {
       lv1->scrollTo(lv1->model()->index(26, 0), Tui::ZListView::PositionAtCenter);
       t.compare("25");
    }

    SECTION("28-ev") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::EnsureVisible);
       t.compare("25");
    }
    SECTION("28-top") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtTop);
       t.compare("26");
    }
    SECTION("28-bottom") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtBottom);
       t.compare("25");
    }
    SECTION("28-center") {
       lv1->scrollTo(lv1->model()->index(27, 0), Tui::ZListView::PositionAtCenter);
       t.compare("26");
    }

    SECTION("29-ev") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::EnsureVisible);
       t.compare("26");
    }
    SECTION("29-top") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtTop);
       t.compare("26");
    }
    SECTION("29-bottom") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtBottom);
       t.compare("26");
    }
    SECTION("29-center") {
       lv1->scrollTo(lv1->model()->index(28, 0), Tui::ZListView::PositionAtCenter);
       t.compare("26");
    }

    SECTION("end-ev") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::EnsureVisible);
       t.compare("26");
    }
    SECTION("end-top") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtTop);
       t.compare("26");
    }
    SECTION("end-bottom") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtBottom);
       t.compare("26");
    }
    SECTION("end-center") {
       lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtCenter);
       t.compare("26");
    }
}

TEST_CASE("listview-resize", "") {

    Testhelper t("listview", "listview-resize", 15, 10);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 10});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setFocus();

    QStringList qsl;
    for (int i = 1; i <= 30; i++) {
        qsl.append(QString::number(i));
    }
    lv1->setItems(qsl);

    SECTION("shrink") {
        lv1->setGeometry({1, 1, 13, 8});
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtBottom);
        lv1->setCurrentIndex(lv1->model()->index(27, 0));
        lv1->setGeometry({1, 1, 13, 5});
        t.compare();
    }

    SECTION("shrink-offscreen") {
        lv1->setGeometry({1, 1, 13, 8});
        lv1->setCurrentIndex(lv1->model()->index(5, 0));
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtBottom);
        lv1->setGeometry({1, 1, 13, 5});
        t.compare();
    }

    SECTION("shrink-top") {
        lv1->setGeometry({1, 1, 13, 8});
        t.compare("shrink-top-8");
        lv1->setGeometry({1, 1, 13, 5});
        t.compare("shrink-top-5");
    }

    SECTION("grow") {
        lv1->setGeometry({1, 1, 13, 5});
        lv1->scrollTo(lv1->model()->index(29, 0), Tui::ZListView::PositionAtBottom);
        lv1->setCurrentIndex(lv1->model()->index(27, 0));
        lv1->setGeometry({1, 1, 13, 8});
        t.compare();
    }

    SECTION("grow-top") {
        lv1->setGeometry({1, 1, 13, 5});
        t.compare("grow-top-5");
        lv1->setGeometry({1, 1, 13, 8});
        t.compare("grow-top-8");
    }
}

TEST_CASE("listview-changes", "") {
    Testhelper t("listview", "unused", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});

    SECTION("model destroyed") {
        {
            QStringListModel model;
            lv1->setModel(&model);
        }
        CHECK(lv1->model() == nullptr);
    }

    SECTION("selected row removed at end") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(4, 0));
        model.removeRows(4, 1);
        CHECK(lv1->currentIndex().row() == 3);
    }

    SECTION("selected row and 1 more removed at end") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(4, 0));
        model.removeRows(3, 2);
        CHECK(lv1->currentIndex().row() == 2);
    }

    SECTION("selected row removed not at end") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(3, 0));
        model.removeRows(3, 1);
        CHECK(lv1->currentIndex().row() == 2);
    }

    SECTION("selected row and 1 more removed not at end") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(2, 0));
        model.removeRows(2, 2);
        CHECK(lv1->currentIndex().row() == 1);
    }

    SECTION("unselected row before selected row removed") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(2, 0));
        model.removeRows(1, 1);
        CHECK(lv1->currentIndex().row() == 1);
    }

    SECTION("unselected row after selected row removed") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        lv1->setCurrentIndex(model.index(2, 0));
        model.removeRows(3, 1);
        CHECK(lv1->currentIndex().row() == 2);
    }

    SECTION("currentIndex") {
        QStringListModel model({"0", "1", "2", "3", "4"});
        lv1->setModel(&model);
        CHECK(lv1->currentIndex().row() == 0);
        lv1->setCurrentIndex(model.index(2, 0));
        CHECK(lv1->currentIndex().row() == 2);
        lv1->setCurrentIndex(model.index(1, 0));
        CHECK(lv1->currentIndex().row() == 1);
    }

}

TEST_CASE("listview-color-black", "") {

    Testhelper t("listview", "listview-color-black", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});

    QStringList qsl;
    qsl.append("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    qsl.append("bcdefghijklmnopqsrtuvwxyz1");
    qsl.append("CDEFGHIJKLMNOPQRSTUVWXYZ12");
    qsl.append("defghijklmnopqsrtuvwxyz123");
    lv1->setItems(qsl);

    t.root->setPalette(Tui::ZPalette::black());

    //black
    SECTION("default") {
        t.compare();
    }

    SECTION("disable") {
        lv1->setEnabled(false);
        t.compare();
    }

    SECTION("focus") {
        lv1->setFocus();
        t.compare();
    }
}

TEST_CASE("listview-color-cyan", "") {
    Testhelper t("listview", "listview-color-cyan", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});
    w->setPaletteClass({"window", "cyan"});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});

    QStringList qsl;
    qsl.append("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    qsl.append("bcdefghijklmnopqsrtuvwxyz1");
    qsl.append("CDEFGHIJKLMNOPQRSTUVWXYZ12");
    qsl.append("defghijklmnopqsrtuvwxyz123");
    lv1->setItems(qsl);

    //cyan
    SECTION("default") {
        t.compare();
    }

    SECTION("disable") {
        lv1->setEnabled(false);
        t.compare();
    }

    SECTION("focus") {
        lv1->setFocus();
        t.compare();
    }
}

TEST_CASE("listview-color-gray", "") {
    Testhelper t("listview", "listview-color-gray", 15, 5);
    Tui::ZWindow *w = new Tui::ZWindow(t.root);
    w->setGeometry({0, 0, 15, 5});
    w->setPaletteClass({"window", "dialog"});

    Tui::ZListView *lv1 = new Tui::ZListView(w);
    lv1->setGeometry({1, 1, 13, 3});

    QStringList qsl;
    qsl.append("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    qsl.append("bcdefghijklmnopqsrtuvwxyz1");
    qsl.append("CDEFGHIJKLMNOPQRSTUVWXYZ12");
    qsl.append("defghijklmnopqsrtuvwxyz123");
    lv1->setItems(qsl);

    //gray
    SECTION("default") {
        t.compare();
    }

    SECTION("disable") {
        lv1->setEnabled(false);
        t.compare();
    }

    SECTION("focus") {
        lv1->setFocus();
        t.compare();
    }
}

TEST_CASE("listview-without-terminal", "") {
    Tui::ZWindow w;
    w.setGeometry({0, 0, 15, 5});
    Tui::ZListView *lv1 = new Tui::ZListView(&w);

    lv1->items();
    CHECK(lv1->currentIndex().isValid() == false);
    QStringList qsl;
    qsl.append("😎");
    lv1->setItems(qsl);
    lv1->currentItem();
    CHECK(lv1->currentIndex().isValid() == true);
    CHECK(lv1->currentIndex().row() == 0);
}
