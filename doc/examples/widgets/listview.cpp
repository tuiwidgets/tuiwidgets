#include <QStringListModel>

#include <Tui/ZHBoxLayout.h>
#include <Tui/ZListView.h>
#include <Tui/ZWidget.h>

void exampleListView(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-start
    Tui::ZListView *listview = new Tui::ZListView(dialog);
    listview->setItems({"item1", "item2", "item3"});
    listview->setCurrentIndex(listview->model()->index(1, 0));

    listview->setMinimumSize(10, 5); // currently required
    layout->addWidget(listview);
// snippet-end
}

void exampleListViewModel(Tui::ZWidget *dialog, Tui::ZHBoxLayout *layout) {
// snippet-model-start
    std::unique_ptr<QAbstractItemModel> model;
    model = std::make_unique<QStringListModel>(QStringList{"item1", "item2", "item3"});
    Tui::ZListView *listview = new Tui::ZListView(dialog);
    listview->setModel(model.get());
    listview->setCurrentIndex(model->index(1, 0));

    listview->setMinimumSize(10, 5); // currently required
    layout->addWidget(listview);
// snippet-model-end
}

