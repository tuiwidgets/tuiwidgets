// SPDX-License-Identifier: BSL-1.0

#include "ZDialog.h"
#include "ZDialog_p.h"

#include <QRect>
#include <QTimer>

#include <Tui/ZWindowFacet.h>

TUIWIDGETS_NS_START

ZDialogPrivate::ZDialogPrivate(ZWidget *pub) : ZWindowPrivate(pub) {
}

ZDialogPrivate::~ZDialogPrivate() {
}

ZDialog::ZDialog(ZWidget *parent) : ZDialog(parent, std::make_unique<ZDialogPrivate>(this)) {
}


ZDialog::ZDialog(ZWidget *parent, std::unique_ptr<ZDialogPrivate> pimpl) : ZWindow(parent, move(pimpl)) {
    setPaletteClass({QStringLiteral("window"), QStringLiteral("dialog")});
    setStackingLayer(1000);
    QTimer::singleShot(0, this, &ZDialog::showHandler);
}

ZDialog::~ZDialog() {
}

QObject *ZDialog::facet(const QMetaObject &metaObject) const {
    auto *const p = tuiwidgets_impl();
    if (metaObject.className() == ZDefaultWidgetManager::staticMetaObject.className()) {
        return &p->defaultWidgetManager;
    } else {
        return ZWindow::facet(metaObject);
    }
}

void ZDialog::reject() {
    auto *const p = tuiwidgets_impl();
    setVisible(false);
    rejected();
    if (p->options & ZWindow::Option::DeleteOnClose) {
        deleteLater();
    }
}

void ZDialog::keyEvent(ZKeyEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->key() == Key_Escape && event->modifiers() == 0) {
        reject();
    } else if (event->key() == Key_Enter && event->modifiers() == 0) {
        if (p->defaultWidgetManager.defaultWidget()) {
            p->defaultWidgetManager.defaultWidget()->event(event);
        }
    } else {
        ZWindow::keyEvent(event);
    }
}

void ZDialog::showHandler() {
    auto *const p = tuiwidgets_impl();
    if (!isVisible()) {
        return;
    }
    if (parentWidget()) {
        p->alreadyShown = true;
        if (geometry().isNull()) {
            setGeometry(QRect({}, effectiveSizeHint()));
            setDefaultPlacement(AlignCenter);
            ZWindowFacet *windowFacet = static_cast<ZWindowFacet*>(facet(ZWindowFacet::staticMetaObject));
            if (windowFacet) {
                if (!windowFacet->isManuallyPlaced()) {
                    windowFacet->autoPlace(parentWidget()->geometry().size(), this);
                }
            }
        }
    }
}

bool ZDialog::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == ZEventType::show()) {
        if (!p->alreadyShown) {
            QTimer::singleShot(0, this, &ZDialog::showHandler);
        }
    }

    return ZWindow::event(event);
}


bool ZDialog::eventFilter(QObject *watched, QEvent *event) {
    return ZWindow::eventFilter(watched, event);
}

QSize ZDialog::sizeHint() const {
    return ZWindow::sizeHint();
}

QSize ZDialog::minimumSizeHint() const {
    return ZWindow::minimumSizeHint();
}

QRect ZDialog::layoutArea() const {
    return ZWindow::layoutArea();
}

ZWidget *ZDialog::resolveSizeHintChain() {
    return ZWindow::resolveSizeHintChain();
}

void ZDialog::timerEvent(QTimerEvent *event) {
    return ZWindow::timerEvent(event);
}

void ZDialog::childEvent(QChildEvent *event) {
    return ZWindow::childEvent(event);
}

void ZDialog::customEvent(QEvent *event) {
    return ZWindow::customEvent(event);
}

void ZDialog::connectNotify(const QMetaMethod &signal) {
    return ZWindow::connectNotify(signal);
}

void ZDialog::disconnectNotify(const QMetaMethod &signal) {
    return ZWindow::disconnectNotify(signal);
}

void ZDialog::paintEvent(ZPaintEvent *event) {
    return ZWindow::paintEvent(event);
}

void ZDialog::pasteEvent(ZPasteEvent *event) {
    return ZWindow::pasteEvent(event);
}

void ZDialog::focusInEvent(ZFocusEvent *event) {
    return ZWindow::focusInEvent(event);
}

void ZDialog::focusOutEvent(ZFocusEvent *event) {
    return ZWindow::focusOutEvent(event);
}

void ZDialog::resizeEvent(ZResizeEvent *event) {
    return ZWindow::resizeEvent(event);
}

void ZDialog::moveEvent(ZMoveEvent *event) {
    return ZWindow::moveEvent(event);
}

QVector<ZMenuItem> ZDialog::systemMenu() {
    return ZWindow::systemMenu();
}

void ZDialog::closeEvent(ZCloseEvent *event) {
    return ZWindow::closeEvent(event);
}


TUIWIDGETS_NS_END
