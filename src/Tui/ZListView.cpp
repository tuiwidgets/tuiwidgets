// SPDX-License-Identifier: BSL-1.0

#include "ZListView.h"
#include "ZListView_p.h"

#include <QStringListModel>

#include <Tui/ZTerminal.h>

TUIWIDGETS_NS_START

ZListViewPrivate::ZListViewPrivate(ZWidget *pub) : ZWidgetPrivate(pub) {
}

ZListViewPrivate::~ZListViewPrivate() {
}

ZListView::ZListView(ZWidget *parent) : ZWidget(parent, std::make_unique<ZListViewPrivate>(this)) {
    setFocusPolicy(StrongFocus);
    setSizePolicyV(SizePolicy::Expanding);
    setSizePolicyH(SizePolicy::Expanding);

}

ZListView::~ZListView() {
    detachModel();
}

void ZListView::setItems(const QStringList &newItems) {
    auto *const p = tuiwidgets_impl();
    detachModel();
    p->allocatedModel = p->model = new QStringListModel(newItems);
    p->scrollPosition = 0;
    attachModel();
    setCurrentIndex(p->model->index(0, 0));
    update();
}

QStringList ZListView::items() const {
    auto *const p = tuiwidgets_impl();
    QStringList res;
    if (p->model) {
        res.reserve(p->model->rowCount());
        for (int i = 0; i < p->model->rowCount(); i++) {
            res.append(p->model->index(i, 0).data().toString());
        }
    }
    return res;
}

QString ZListView::currentItem() const {
    return currentIndex().data().toString();
}

void ZListView::setModel(QAbstractItemModel *model) {
    auto *const p = tuiwidgets_impl();
    detachModel();
    p->model = model;
    p->scrollPosition = 0;
    attachModel();
    setCurrentIndex(p->model->index(0, 0));
}

QAbstractItemModel *ZListView::model() const {
    auto *const p = tuiwidgets_impl();
    return p->model;
}

void ZListView::setCurrentIndex(QModelIndex index) {
    auto *const p = tuiwidgets_impl();
    if (index.isValid()) {
        if (p->selectionModel) {
            p->selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
        }
    }
}

QModelIndex ZListView::currentIndex() const {
    auto *const p = tuiwidgets_impl();
    if (!p->selectionModel) {
        return {};
    }
    return p->selectionModel->currentIndex();
}

QItemSelectionModel *ZListView::selectionModel() const {
    auto *const p = tuiwidgets_impl();
    return p->selectionModel;
}

QSize ZListView::sizeHint() const {
    return {10, 3};
}

void ZListView::scrollTo(const QModelIndex &index, ZListView::ScrollHint hint) {
    auto *const p = tuiwidgets_impl();
    if (!geometry().height()) return;
    const int row = index.row();
    const int visibleItems = geometry().height();
    const int maxPossibleScrollPosition = std::max(0, p->model->rowCount() - visibleItems);
    switch (hint) {
        case EnsureVisible:
            if (row < p->scrollPosition) {
                // make one item below row visible if possible
                p->scrollPosition = std::min(row - visibleItems + 2, maxPossibleScrollPosition);
            } else if (row == p->scrollPosition) {
                // make one item above row visible if possible
                p->scrollPosition = std::max(0, row - 1);
            } else if (row > p->scrollPosition + (visibleItems - 1)) {
                // make one item below row visible if possible
                p->scrollPosition = std::min(row - visibleItems + 2, maxPossibleScrollPosition);
            } else if (row == p->scrollPosition + (visibleItems - 1)) {
                // make one item below row visible if possible
                p->scrollPosition = std::min(row - visibleItems + 2, maxPossibleScrollPosition);
            }
            break;
        case PositionAtTop:
            p->scrollPosition = std::max(0, row - 1);
            break;
        case PositionAtBottom:
            p->scrollPosition = std::max(0, std::min(row - visibleItems + 2, maxPossibleScrollPosition));
            break;
        case PositionAtCenter:
            p->scrollPosition = std::max(0, row - visibleItems / 2);
            break;
    }
    p->scrollPosition = std::max(p->scrollPosition, 0);
    p->scrollPosition = std::min(p->scrollPosition, maxPossibleScrollPosition);
}

void ZListView::resizeEvent(ZResizeEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->oldSize().height() && event->size().height() && p->model && currentIndex().isValid()) {
        const int oldVisibleItems = event->oldSize().height();
        const int visibleItems = event->size().height();
        const int currentRow = currentIndex().row();

        const bool wasVisible = p->scrollPosition <= currentRow && currentRow < p->scrollPosition + oldVisibleItems;
        const bool willBeVisible = p->scrollPosition <= currentRow && currentRow < p->scrollPosition + visibleItems;

        const int maxPossibleScrollPosition = p->model->rowCount() - visibleItems;
        if (wasVisible && !willBeVisible) {
            p->scrollPosition = std::max(0, std::min(currentRow - visibleItems + 2, maxPossibleScrollPosition));
        } else {
            p->scrollPosition = std::min(p->scrollPosition, maxPossibleScrollPosition);
        }
    } else if (event->oldSize().height() == 0 && event->size().height() && p->model && currentIndex().isValid()) {
        scrollTo(currentIndex(), PositionAtCenter);
    }

    ZWidget::resizeEvent(event);
}

void ZListView::detachModel() {
    auto *const p = tuiwidgets_impl();
    if (!p->model) return;
    if (p->selectionModel) {
        // XXX: This does not match qt widgets. It should be ok as long as we don't have a setter for _selectionModel
        p->selectionModel->deleteLater();
        p->selectionModel = nullptr;
    }
    disconnect(p->model, nullptr, this, nullptr);
    if (p->model == p->allocatedModel) {
        delete p->model;
        p->allocatedModel = nullptr;
    }
    p->model = nullptr;
}

void ZListView::attachModel() {
    auto *const p = tuiwidgets_impl();
    p->selectionModel = new QItemSelectionModel(p->model, this);

    connect(p->selectionModel, &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &previous) {
        (void)previous;
        auto *const p = tuiwidgets_impl();
        if (current.isValid()) {
            p->lastSelectedRow = current.row();
            scrollTo(current, EnsureVisible);
        }
        update();
    });

    connect(p->selectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        update();
    });

    auto handler = [this] {
        auto *const p = tuiwidgets_impl();
        const QModelIndex current = currentIndex();
        if (!current.isValid() && p->model->rowCount()) {
            // It seems QItemSelectionModel already does something to avoid invalid currentIndex.
            // keep this code in case there are edge cases where it doesn't.
            setCurrentIndex(p->model->index(std::min(p->lastSelectedRow, p->model->rowCount() - 1), 0));
        }
        scrollTo(currentIndex(), EnsureVisible);
        update();
    };

    connect(p->model, &QObject::destroyed, this, [this] {
        auto *const p = tuiwidgets_impl();
        disconnect(p->model, nullptr, this, nullptr);
        p->allocatedModel = p->model = nullptr;
        if (p->selectionModel) {
            p->selectionModel->deleteLater();
        }
        p->selectionModel = nullptr;
        p->lastSelectedRow = 0;
        p->scrollPosition = 0;
    });
    connect(p->model, &QAbstractItemModel::columnsInserted, this, handler);
    connect(p->model, &QAbstractItemModel::columnsMoved, this, handler);
    connect(p->model, &QAbstractItemModel::columnsRemoved, this, handler);
    connect(p->model, &QAbstractItemModel::dataChanged, this, handler);
    connect(p->model, &QAbstractItemModel::layoutChanged, this, handler);
    connect(p->model, &QAbstractItemModel::modelReset, this, handler);
    connect(p->model, &QAbstractItemModel::rowsInserted, this, handler);
    connect(p->model, &QAbstractItemModel::rowsMoved, this, handler);
    connect(p->model, &QAbstractItemModel::rowsRemoved, this, handler);
}

void ZListView::paintEvent(ZPaintEvent *event) {
    auto *const p = tuiwidgets_impl();
    ZTextStyle baseStyle;
    ZTextStyle selectedStyle;
    ZTextStyle selectedStyleFocus;

    auto *painter = event->painter();
    auto *term = terminal();

    if (isEnabled()) {
        baseStyle = {getColor("dataview.fg"), getColor("dataview.bg")};
        selectedStyle = {getColor("dataview.selected.fg"), getColor("dataview.selected.bg")};
        selectedStyleFocus = {getColor("dataview.selected.focused.fg"), getColor("dataview.selected.focused.bg")};
    } else {
        baseStyle = {getColor("dataview.disabled.fg"), getColor("dataview.disabled.bg")};
        selectedStyle = {getColor("dataview.disabled.selected.fg"), getColor("dataview.disabled.selected.bg")};
        selectedStyleFocus = selectedStyle;
    }
    painter->clear(baseStyle.foregroundColor(), baseStyle.backgroundColor());

    if (!p->model) return;

    ZPainter clippedPainter = painter->translateAndClip(1, 0, geometry().width() - 2, geometry().height());

    const int size = p->model->rowCount();
    for (int i = 0; i < std::min(size - p->scrollPosition, geometry().height()); i++) {
        const QModelIndex idx = p->model->index(i + p->scrollPosition, 0);
        const QString itemString = idx.data().toString();

        ZTextStyle effectiveStyle;
        if (p->selectionModel && p->selectionModel->isSelected(idx)) {
            if (term && isAncestorOf(term->focusWidget())) {
                effectiveStyle = selectedStyleFocus;
            } else {
                effectiveStyle = selectedStyle;
            }
        } else {
            effectiveStyle = baseStyle;
        }
        p->styledText.setBaseStyle(effectiveStyle);

        QString itemLeftDecoration = idx.data(LeftDecorationRole).toString();
        ZColor itemLeftDecorationFg = effectiveStyle.foregroundColor();
        if (idx.data(LeftDecorationFgRole).canConvert<ZColor>()) {
            itemLeftDecorationFg = idx.data(LeftDecorationFgRole).value<ZColor>();
        }

        ZColor itemLeftDecorationBg = effectiveStyle.backgroundColor();
        if (idx.data(LeftDecorationBgRole).canConvert<ZColor>()) {
            itemLeftDecorationBg = idx.data(LeftDecorationBgRole).value<ZColor>();
        }

        int leftDecorationWidth = term->textMetrics().sizeInColumns(itemLeftDecoration);
        if (leftDecorationWidth) {
            clippedPainter.writeWithColors(0, i, itemLeftDecoration, itemLeftDecorationFg, itemLeftDecorationBg);
        }

        int itemLeftDecorationSpace = idx.data(LeftDecorationSpaceRole).toInt();

        p->styledText.setText(QStringLiteral(" ").repeated(itemLeftDecorationSpace) + itemString);

        p->styledText.write(painter, 1 + leftDecorationWidth, i, geometry().width() - 2 - leftDecorationWidth);

        if (p->selectionModel && p->selectionModel->currentIndex() == idx) {
            if (term && isAncestorOf(term->focusWidget()) && isEnabled()) {
                painter->writeWithColors(geometry().width()-1, i, QStringLiteral("«"),
                                         effectiveStyle.foregroundColor(),
                                         effectiveStyle.backgroundColor());
                painter->writeWithColors(0, i, QStringLiteral("»"),
                                         effectiveStyle.foregroundColor(),
                                         effectiveStyle.backgroundColor());
            } else {
                painter->writeWithColors(geometry().width()-1, i, QStringLiteral("←"),
                                         effectiveStyle.foregroundColor(),
                                         effectiveStyle.backgroundColor());
                painter->writeWithColors(0, i, QStringLiteral("→"),
                                         effectiveStyle.foregroundColor(),
                                         effectiveStyle.backgroundColor());
            }
        }
    }
}

void ZListView::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (!p->model) {
        ZWidget::keyEvent(event);
        return;
    }

    const int size = p->model->rowCount();

    const QModelIndex current = currentIndex();

    if (event->key() == Key_Up) {
        if (current.row() > 0) {
            setCurrentIndex(current.sibling(current.row() - 1, 0));
        }
        update();
    } else if (event->key() == Key_Down) {
        if (current.row() < size - 1) {
            setCurrentIndex(current.sibling(current.row() + 1, 0));
        }
        update();
    } else if (event->key() == Key_Home) {
        setCurrentIndex(p->model->index(0, 0));
        p->scrollPosition = 0;
        update();
    } else if (event->key() == Key_End) {
        setCurrentIndex(p->model->index(size - 1, 0));
        if (size > geometry().height()) {
            p->scrollPosition = size - geometry().height();
        }
        update();
    } else if (event->key() == Key_PageUp) {
        if (p->scrollPosition >= geometry().height()) {
            int sp = p->scrollPosition - geometry().height();
            setCurrentIndex(p->model->index(sp + geometry().height() - 1, 0));
            p->scrollPosition = sp;
        } else {
            setCurrentIndex(p->model->index(0, 0));
            p->scrollPosition = 0;
        }
        update();
    } else if (event->key() == Key_PageDown) {
        if (p->scrollPosition + 2 * geometry().height() < size) {
            int row = p->scrollPosition + geometry().height();
            setCurrentIndex(p->model->index(row, 0));
            p->scrollPosition = row;
        } else {
            if (size > geometry().height()) {
                p->scrollPosition = size - geometry().height();
                setCurrentIndex(p->model->index(p->scrollPosition + geometry().height() - 1, 0));
            } else {
                setCurrentIndex(p->model->index(size - 1, 0));
            }
        }
        update();
    } else if (event->key() == Key_Enter) {
        enterPressed(current.row());
    } else if (!event->text().isEmpty() && event->modifiers() == 0) {
        int laterMatch = -1, match = -1;
        for (int i = 0; i < size; i++) {
            const QString itemString = p->model->index(i, 0).data().toString();
            if (itemString.toLower().startsWith(event->text().toLower())) {
                if (i > current.row()) {
                    laterMatch = i;
                    break;
                } else {
                    if (match == -1) {
                        match = i;
                    }
                }
            }
        }
        if (laterMatch != -1) {
            match = laterMatch;
        }
        if (match != -1) {
            setCurrentIndex(p->model->index(match, 0));
            if (match + geometry().height() < size ) {
                p->scrollPosition = match;
            } else if (match + geometry().height() >= size) {
                p->scrollPosition = size - geometry().height();
            }
            update();
        }
    } else {
        ZWidget::keyEvent(event);
    }
}

bool ZListView::event(QEvent *event) {
    return ZWidget::event(event);
}

bool ZListView::eventFilter(QObject *watched, QEvent *event) {
    return ZWidget::eventFilter(watched, event);
}

QSize ZListView::minimumSizeHint() const {
    return ZWidget::minimumSizeHint();
}

QRect ZListView::layoutArea() const {
    return ZWidget::layoutArea();
}

QObject *ZListView::facet(const QMetaObject &metaObject) const {
    return ZWidget::facet(metaObject);
}

ZWidget *ZListView::resolveSizeHintChain() {
    return ZWidget::resolveSizeHintChain();
}

void ZListView::timerEvent(QTimerEvent *event) {
    return ZWidget::timerEvent(event);
}

void ZListView::childEvent(QChildEvent *event) {
    return ZWidget::childEvent(event);
}

void ZListView::customEvent(QEvent *event) {
    return ZWidget::customEvent(event);
}

void ZListView::connectNotify(const QMetaMethod &signal) {
    return ZWidget::connectNotify(signal);
}

void ZListView::disconnectNotify(const QMetaMethod &signal) {
    return ZWidget::disconnectNotify(signal);
}

void ZListView::pasteEvent(ZPasteEvent *event) {
    return ZWidget::pasteEvent(event);
}

void ZListView::focusInEvent(ZFocusEvent *event) {
    return ZWidget::focusInEvent(event);
}

void ZListView::focusOutEvent(ZFocusEvent *event) {
    return ZWidget::focusOutEvent(event);
}

void ZListView::moveEvent(ZMoveEvent *event) {
    return ZWidget::moveEvent(event);
}


TUIWIDGETS_NS_END
