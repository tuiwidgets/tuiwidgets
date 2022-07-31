#ifndef TUIWIDGETS_MISC_ABSTRACTTABLEMODELTRACKBY_INCLUDED
#define TUIWIDGETS_MISC_ABSTRACTTABLEMODELTRACKBY_INCLUDED

#include <QAbstractListModel>
#include <QMap>
#include <QSet>
#include <QVector>

#include <Tui/tuiwidgets_internal.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define TUI_AbstractTableModelTrackBy_NOISVALID
#endif

TUIWIDGETS_NS_START

namespace Misc {

template<typename KEY>
class AbstractTableModelTrackBy : public QAbstractTableModel {
public:
    struct Row {
        KEY key;
        QVector<QMap<int, QVariant>> columns;
    };
public:
    AbstractTableModelTrackBy(int columns);

public:
    void setData(const QVector<Row> &data);

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void updateRow(int idx, const Row &row);
    int findRowByKey(const KEY &key);
    static bool isVariantExactlyEqual(const QVariant &a, const QVariant &b);

private:
    int _columns;
    QVector<Row> _data;
};

template<typename KEY>
AbstractTableModelTrackBy<KEY>::AbstractTableModelTrackBy(int columns) : _columns(columns) {
}

template<typename KEY>
void AbstractTableModelTrackBy<KEY>::setData(const QVector<AbstractTableModelTrackBy::Row> &data) {

    QSet<KEY> nextKeys;
    for (const auto& row: data) {
        if (row.columns.size() != _columns) {
            qWarning("AbstractItemModelTrackBy::setData: Columns count does not match. This is not supported");
            return;
        }

        if (nextKeys.contains(row.key)) {
            qWarning("AbstractItemModelTrackBy::setData: duplicate key in new data. This is not supported");
            return;
        }
        nextKeys.insert(row.key);
    }

    if (_data.size() == 0) {
        beginInsertRows(QModelIndex(), 0, data.size() - 1);
        _data = data;
        endInsertRows();
        return;
    }

    // remove all rows that are no longer needed
    for (int i = 0; i < _data.size(); i++) {
        if (!nextKeys.contains(_data[i].key)) {
            // TODO batch if next row also needs to be removed
            beginRemoveRows(QModelIndex(), i, i);
            _data.removeAt(i);
            endRemoveRows();
            --i;
        }
    }

    // reorder and add rows
    QSet<KEY> inListKeys;
    for (const auto& row: _data) {
        inListKeys.insert(row.key);
    }

    int i = 0;
    for (const auto& row: data) {
        if (i < _data.size() && _data[i].key == row.key) {
            updateRow(i, row);
        } else if (inListKeys.contains(row.key)) {
            int j = findRowByKey(row.key);
            beginMoveRows(QModelIndex(), j, j, QModelIndex(), i);
            _data.move(j, i);
            endMoveRows();
            updateRow(i, row);
        } else {
            beginInsertRows(QModelIndex(), i, i);
            _data.insert(i, row);
            endInsertRows();
        }
        ++i;
    }
}

template<typename KEY>
int AbstractTableModelTrackBy<KEY>::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return _data.size();
}

template<typename KEY>
int AbstractTableModelTrackBy<KEY>::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return _columns;
}

template<typename KEY>
QVariant AbstractTableModelTrackBy<KEY>::data(const QModelIndex &index, int role) const {
#if !defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid));
#elif !defined(MODELTESTER_COMPAT) && defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(index.isValid());
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid() || !checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid()) {
        return {};
    }
#endif
    return _data[index.row()].columns[index.column()].value(role, QVariant());
}

template<typename KEY>
QMap<int, QVariant> AbstractTableModelTrackBy<KEY>::itemData(const QModelIndex &index) const {
#if !defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid));
#elif !defined(MODELTESTER_COMPAT) && defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(index.isValid());
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid() || !checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid()) {
        return {};
    }
#endif
    return _data[index.row()].columns[index.column()];
}

template<typename KEY>
Qt::ItemFlags AbstractTableModelTrackBy<KEY>::flags(const QModelIndex &index) const {
#if !defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid));
#elif !defined(MODELTESTER_COMPAT) && defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    Q_ASSERT(index.isValid());
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid() || !checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
#elif defined(MODELTESTER_COMPAT) && !defined(TUI_AbstractTableModelTrackBy_NOISVALID)
    if (!index.isValid()) {
        return {};
    }
#endif
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
}

template<typename KEY>
void AbstractTableModelTrackBy<KEY>::updateRow(int idx, const AbstractTableModelTrackBy<KEY>::Row &row) {
    auto& currentRow = _data[idx];
    bool changed = false;
    for (int i = 0; i < _columns; i++) {
        const auto roles = currentRow.columns[i].keys();
        if (roles != row.columns[i].keys()) {
            changed = true;
            break;
        }
        for (const auto& role: roles) {
            if (!isVariantExactlyEqual(currentRow.columns[i][role], row.columns[i][role])) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        currentRow.columns = row.columns;
        dataChanged(index(idx, 0), index(idx, _columns - 1));
    }
}

template<typename KEY>
bool AbstractTableModelTrackBy<KEY>::isVariantExactlyEqual(const QVariant &a, const QVariant &b) {
    if (a.userType() != b.userType()) return false;
    return a == b;
}

template<typename KEY>
int AbstractTableModelTrackBy<KEY>::findRowByKey(const KEY &key) {
    for (int i = 0; i < _data.size(); i++) {
        if (_data[i].key == key) {
            return i;
        }
    }
    // Maybe assert instead?
    throw std::runtime_error("AbstractItemModelTrackBy::findRowByKey called on non existing key.");
}

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_MISC_ABSTRACTTABLEMODELTRACKBY_INCLUDED
