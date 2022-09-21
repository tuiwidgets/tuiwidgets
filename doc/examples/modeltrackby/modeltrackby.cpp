#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include <Tui/Misc/AbstractTableModelTrackBy.h>

void example() {
// snippet-setup-start
    Tui::Misc::AbstractTableModelTrackBy<QString> model{2};
// snippet-setup-end

// snippet-update-start
    QList<QFileInfo> files = QDir().entryInfoList(QDir::NoFilter, QDir::Name);
    QVector<Tui::Misc::AbstractTableModelTrackBy<QString>::Row> newData;
    newData.reserve(files.size());
    for (const auto &file: files) {
        Tui::Misc::AbstractTableModelTrackBy<QString>::Row row;
        row.key = file.absoluteFilePath();
        QMap<int, QVariant> column1;
        column1[Qt::DisplayRole] = file.fileName();
        row.columns.append(std::move(column1));

        QMap<int, QVariant> column2;
        column2[Qt::DisplayRole] = file.lastModified();
        row.columns.append(std::move(column2));

        newData.append(row);
    }
    model.setData(newData);
// snippet-update-end
}

int main() {
    return 0;
}
