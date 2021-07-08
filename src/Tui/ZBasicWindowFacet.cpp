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

bool ZBasicWindowFacet::isManuallyPlaced() {
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

        switch (p->alignment & Qt::AlignHorizontal_Mask) {
        case Qt::AlignLeft:
            rect.moveLeft(0);
            break;
        case Qt::AlignRight:
            rect.moveRight(available.width() - 1);
            break;
        default:
            rect.moveCenter({available.width() / 2, 0});
            break;
        }

        switch (p->alignment & Qt::AlignVertical_Mask) {
        case Qt::AlignTop:
            rect.moveTop(0);
            break;
        case Qt::AlignBottom:
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

void ZBasicWindowFacet::setDefaultPlacement(Qt::Alignment align, QPoint displace) {
    auto *const p = tuiwidgets_impl();
    p->alignment = align;
    p->displacement = displace;
    p->manuallyPlaced = false;
}

void ZBasicWindowFacet::setExtendViewport(bool extend) {
    auto *const p = tuiwidgets_impl();
    p->extendViewport = extend;
}

TUIWIDGETS_NS_END

