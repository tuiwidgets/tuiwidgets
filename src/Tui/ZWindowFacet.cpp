// SPDX-License-Identifier: BSL-1.0

#include "ZWindowFacet.h"
#include "ZWindowFacet_p.h"

#include <QRect>

TUIWIDGETS_NS_START

ZWindowFacet::ZWindowFacet() : tuiwidgets_pimpl_ptr(std::make_unique<ZWindowFacetPrivate>()) {
}

ZWindowFacet::ZWindowFacet(std::unique_ptr<ZWindowFacetPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZWindowFacet::~ZWindowFacet() {
}

bool ZWindowFacet::isExtendViewport() const {
    return false;
}

bool ZWindowFacet::isManuallyPlaced() const {
    auto *const p = tuiwidgets_impl();
    return p->manuallyPlaced;
}

void ZWindowFacet::autoPlace(const QSize &available, ZWidget *self) {
    auto *const p = tuiwidgets_impl();
    if (!p->manuallyPlaced) {
        QRect rect;
        rect.setSize(self->geometry().size());
        rect.moveCenter({available.width() / 2, available.height() / 2});
        self->setGeometry(rect);
    }
}

void ZWindowFacet::setManuallyPlaced(bool manual) {
    auto *const p = tuiwidgets_impl();
    p->manuallyPlaced = manual;
}

ZWindowContainer *ZWindowFacet::container() const {
    auto *const p = tuiwidgets_impl();
    return p->container;
}

void ZWindowFacet::setContainer(ZWindowContainer *container) {
    auto *const p = tuiwidgets_impl();
    p->container = container;
}

bool ZWindowFacet::event(QEvent *event) {
    return QObject::event(event);
}

bool ZWindowFacet::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZWindowFacet::timerEvent(QTimerEvent *event) {
    return QObject::timerEvent(event);
}

void ZWindowFacet::childEvent(QChildEvent *event) {
    return QObject::childEvent(event);
}

void ZWindowFacet::customEvent(QEvent *event) {
    return QObject::customEvent(event);
}

void ZWindowFacet::connectNotify(const QMetaMethod &signal) {
    return QObject::connectNotify(signal);
}

void ZWindowFacet::disconnectNotify(const QMetaMethod &signal) {
    return QObject::disconnectNotify(signal);
}

ZWindowFacetPrivate::~ZWindowFacetPrivate() = default;

TUIWIDGETS_NS_END
