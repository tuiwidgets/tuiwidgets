#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>
#include <QVariant>
#include <QStringListModel>

#include <Tui/ZColor.h>
#include <Tui/ZDialog.h>
#include <Tui/ZListView.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

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

void listView() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 12, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1, 1, 10, 5});
        listview.setItems({"item1", "item2", "item3"});
        listview.setCurrentIndex(listview.model()->index(1, 0));

        export_tpi(&terminal, "listview", 0, 0, 12, 7);
    }

    // focus
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1, 1, 10, 5});
        listview.setItems({"item1", "item2", "item3"});
        listview.setFocus();

        export_tpi(&terminal, "listview-focus", 0, 0, 12, 7);
    }

    // disabled
    {
        Tui::ZListView listview(&dialog);
        listview.setGeometry({1, 1, 10, 5});
        listview.setItems({"item1", "item2", "item3"});
        listview.setEnabled(false);

        export_tpi(&terminal, "listview-disabled", 0, 0, 12, 7);
    }

    {
        SimpleModel model;

        model.rows = {
            {{ Qt::DisplayRole, QString("item1")}, {Tui::LeftDecorationSpaceRole, 2}},
            {{ Qt::DisplayRole, QString("item2")}, {Tui::LeftDecorationSpaceRole, 2}},
            {{ Qt::DisplayRole, QString("item3")}, {Tui::LeftDecorationSpaceRole, 1},
             {Tui::LeftDecorationRole, QString("?")},
             {Tui::LeftDecorationFgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::brightWhite))},
             {Tui::LeftDecorationBgRole, QVariant::fromValue(Tui::ZColor(Tui::Colors::black))}},
        };

        Tui::ZListView listview(&dialog);
        listview.setGeometry({1, 1, 10, 5});
        listview.setModel(&model);

        export_tpi(&terminal, "listview-decoration", 0, 0, 12, 7);
    }
}
