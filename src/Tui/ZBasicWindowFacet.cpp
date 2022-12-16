// SPDX-License-Identifier: BSL-1.0

#include "ZBasicWindowFacet.h"
#include "ZBasicWindowFacet_p.h"

#include <QRect>

#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

ZBasicWindowFacet::ZBasicWindowFacet() : ZWindowFacet(std::make_unique<ZBasicWindowFacetPrivate>()) {
}

ZBasicWindowFacet::~ZBasicWindowFacet() {
}

bool ZBasicWindowFacet::isExtendViewport() const {
    auto *const p = tuiwidgets_impl();
    return p->extendViewport;
}

bool ZBasicWindowFacet::isManuallyPlaced() const {
    auto *const p = tuiwidgets_impl();
    return p->manuallyPlaced;
}

void ZBasicWindowFacet::autoPlace(const QSize &available, ZWidget *self) {
    auto *const p = tuiwidgets_impl();
    if (!p->manuallyPlaced) {
        const int width = self->geometry().width();
        const int height = self->geometry().height();

        QRect rect;
        rect.setWidth(width);
        rect.setHeight(height);

        switch (p->alignment & AlignHorizontal_Mask) {
        case AlignLeft:
            rect.moveLeft(0);
            break;
        case AlignRight:
            rect.moveRight(available.width() - 1);
            break;
        default:
            rect.moveCenter({available.width() / 2, 0});
            break;
        }

        switch (p->alignment & AlignVertical_Mask) {
        case AlignTop:
            rect.moveTop(0);
            break;
        case AlignBottom:
            rect.moveBottom(available.height() - 1);
            break;
        default:
            rect.moveCenter({rect.center().x(), available.height() / 2});
            break;
        }

        rect.translate(p->displacement.x(), p->displacement.y());

        if (rect.left() < 0) {
            rect.moveLeft(0);
        }
        if (rect.top() < 0) {
            rect.moveTop(0);
        }

        self->setGeometry(rect);
    }
}

void ZBasicWindowFacet::setDefaultPlacement(Alignment align, QPoint displace) {
    auto *const p = tuiwidgets_impl();
    p->alignment = align;
    p->displacement = displace;
    p->manuallyPlaced = false;
}

void ZBasicWindowFacet::setExtendViewport(bool extend) {
    auto *const p = tuiwidgets_impl();
    p->extendViewport = extend;
}

bool ZBasicWindowFacet::event(QEvent *event) {
    return ZWindowFacet::event(event);
}

bool ZBasicWindowFacet::eventFilter(QObject *watched, QEvent *event) {
    return ZWindowFacet::eventFilter(watched, event);
}

ZWindowContainer *ZBasicWindowFacet::container() const {
    return ZWindowFacet::container();
}

void ZBasicWindowFacet::setContainer(ZWindowContainer *container) {
    return ZWindowFacet::setContainer(container);
}

void ZBasicWindowFacet::setManuallyPlaced(bool manual) {
    return ZWindowFacet::setManuallyPlaced(manual);
}

void ZBasicWindowFacet::timerEvent(QTimerEvent *event) {
    return ZWindowFacet::timerEvent(event);
}

void ZBasicWindowFacet::childEvent(QChildEvent *event) {
    return ZWindowFacet::childEvent(event);
}

void ZBasicWindowFacet::customEvent(QEvent *event) {
    return ZWindowFacet::customEvent(event);
}

void ZBasicWindowFacet::connectNotify(const QMetaMethod &signal) {
    return ZWindowFacet::connectNotify(signal);
}

void ZBasicWindowFacet::disconnectNotify(const QMetaMethod &signal) {
    return ZWindowFacet::disconnectNotify(signal);
}

ZBasicWindowFacetPrivate::ZBasicWindowFacetPrivate() {
}

ZBasicWindowFacetPrivate::~ZBasicWindowFacetPrivate() {
}

TUIWIDGETS_NS_END

