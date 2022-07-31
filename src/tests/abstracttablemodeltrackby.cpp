// Modeltester assumes wide contract for item model, which we generally don't want
// #define MODELTESTER_COMPAT

#include <Tui/Misc/AbstractTableModelTrackBy.h>

#include <random>
#include <unordered_set>

#ifdef MODELTESTER_COMPAT
#include <QAbstractItemModelTester>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

#include "signalrecorder.h"

#include "catchwrapper.h"

static QVariant v(QString x) {
    return x;
}

using Row = Tui::Misc::AbstractTableModelTrackBy<int>::Row;


namespace {
    struct PermanetIndexChecker {
        struct IndexAndString {
            QPersistentModelIndex idx;
            QString value;
        };

        void prepare(const Tui::Misc::AbstractTableModelTrackBy<int> &model) {
            snapshot.clear();
            for (int i = 0; i < model.rowCount(); i++) {
                snapshot.append({model.index(i, 0), model.index(i, 0).data().toString()});
            }
        }

        int check() {
            int valid = 0;
            for (const auto &ias: snapshot) {
                if (ias.idx.isValid()) {
                    CHECK(ias.idx.data().toString() == ias.value);
                    ++valid;
                }
            }
            return valid;
        }

        QVector<IndexAndString> snapshot;
    };
}

void recordAllModelSignals(SignalRecorder &recorder, const QAbstractItemModel *model) {
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsAboutToBeInserted));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsAboutToBeMoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsAboutToBeRemoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsInserted));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsMoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::columnsRemoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::dataChanged));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::headerDataChanged));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::layoutAboutToBeChanged));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::layoutChanged));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::modelAboutToBeReset));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::modelReset));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsAboutToBeInserted));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsAboutToBeMoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsAboutToBeRemoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsInserted));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsMoved));
    recorder.watchSignal(model, RECORDER_SIGNAL(&QAbstractItemModel::rowsRemoved));
}

TEST_CASE("abstracttablemodeltrackby-add-change-remove", "") {
    Tui::Misc::AbstractTableModelTrackBy<int> model(1);
#ifdef MODELTESTER_COMPAT
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
#endif

    PermanetIndexChecker pidxChecker;

    SignalRecorder recorder;
    recordAllModelSignals(recorder, &model);

    CHECK(model.rowCount() == 0);


    QVector<Row> rows1 = {
        { 1, {{{Qt::DisplayRole, v("row1")}}}}
    };

    model.setData(rows1);

    CHECK(model.rowCount() == 1);
    CHECK(model.index(0, 0).data().toString() == "row1");

    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsAboutToBeInserted, QModelIndex(), 0, 0));
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsInserted, QModelIndex(), 0, 0));
    CHECK(recorder.noMoreSignal());

    QVector<Row> rows2 = {
        { 1, {{{Qt::DisplayRole, v("row1.1")}}}}
    };

    QPersistentModelIndex pidx = model.index(0, 0);
    model.setData(rows2);
    CHECK(pidx.isValid());
    CHECK(pidx.data().toString() == "row1.1");

    CHECK(model.rowCount() == 1);
    CHECK(model.index(0, 0).data().toString() == "row1.1");

    CHECK(recorder.consumeFirst(&QAbstractItemModel::dataChanged, model.index(0,0), model.index(0,0), QVector<int>()));
    CHECK(recorder.noMoreSignal());

    QVector<Row> rows3 = {
    };

    pidxChecker.prepare(model);
    model.setData(rows3);
    pidxChecker.check();

    CHECK(model.rowCount() == 0);
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsAboutToBeRemoved, QModelIndex(), 0, 0));
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsRemoved, QModelIndex(), 0, 0));
    CHECK(recorder.noMoreSignal());
}

TEST_CASE("abstracttablemodeltrackby-add-add-swap", "") {
    Tui::Misc::AbstractTableModelTrackBy<int> model(1);
#ifdef MODELTESTER_COMPAT
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
#endif

    PermanetIndexChecker pidxChecker;

    SignalRecorder recorder;
    recordAllModelSignals(recorder, &model);

    CHECK(model.rowCount() == 0);


    QVector<Row> rows1 = {
        { 1, {{{Qt::DisplayRole, v("row1")}}}}
    };

    model.setData(rows1);

    CHECK(model.rowCount() == 1);
    CHECK(model.index(0, 0).data().toString() == "row1");

    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsAboutToBeInserted, QModelIndex(), 0, 0));
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsInserted, QModelIndex(), 0, 0));
    CHECK(recorder.noMoreSignal());

    QVector<Row> rows2 = {
        { 1, {{{Qt::DisplayRole, v("row1")}}}},
        { 2, {{{Qt::DisplayRole, v("row2")}}}},
    };

    pidxChecker.prepare(model);
    model.setData(rows2);
    pidxChecker.check();

    CHECK(model.rowCount() == 2);
    CHECK(model.index(0, 0).data().toString() == "row1");
    CHECK(model.index(1, 0).data().toString() == "row2");

    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsAboutToBeInserted, QModelIndex(), 1, 1));
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsInserted, QModelIndex(), 1, 1));
    CHECK(recorder.noMoreSignal());

    QVector<Row> rows3 = {
        { 2, {{{Qt::DisplayRole, v("row2")}}}},
        { 1, {{{Qt::DisplayRole, v("row1")}}}},
    };

    pidxChecker.prepare(model);
    model.setData(rows3);
    pidxChecker.check();

    CHECK(model.rowCount() == 2);
    CHECK(model.index(0, 0).data().toString() == "row2");
    CHECK(model.index(1, 0).data().toString() == "row1");

    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsAboutToBeMoved, QModelIndex(), 1, 1, QModelIndex(), 0));
    CHECK(recorder.consumeFirst(&QAbstractItemModel::rowsMoved, QModelIndex(), 1, 1, QModelIndex(), 0));
    CHECK(recorder.noMoreSignal());
}

TEST_CASE("abstracttablemodeltrackby-random", "") {
    Tui::Misc::AbstractTableModelTrackBy<int> model(1);
#ifdef MODELTESTER_COMPAT
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
#endif

    PermanetIndexChecker pidxChecker;

    QVector<Row> rows;
    for (int i = 0; i < 1000; i++) {
        rows.append(Row{ i, {{{Qt::DisplayRole, v(QStringLiteral("row%0").arg(QString::number(i)))}}}});
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    auto *rng = QRandomGenerator::global();
#else
    std::random_device rd;
    auto rng = std::make_unique<std::mt19937>(rd());
#endif

    std::unordered_set<int> prev_keys;

    for (int repeat = 0; repeat < 200; repeat++) {
        std::shuffle(rows.begin(), rows.end(), *rng);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        int len = rng->bounded(rows.size());
#else
        int len = std::uniform_int_distribution<int>(0, rows.size() - 1)(*rng);
#endif

        QVector<Row> subset = rows;
        subset.resize(len);

        pidxChecker.prepare(model);
        model.setData(subset);
        int reusable_indicies = pidxChecker.check();

        CHECK(model.rowCount() == len);

        for (int i = 0; i < len; i++) {
            CHECK(model.index(i, 0).data().toString() == rows[i].columns[0][Qt::DisplayRole].toString());
        }

        std::unordered_set<int> keys;
        int common_keys = 0;
        for (const auto& row: subset) {
            keys.insert(row.key);
            if (prev_keys.count(row.key)) {
                ++common_keys;
            }
        }
        CHECK(reusable_indicies == common_keys);

        prev_keys = keys;
    }

}
