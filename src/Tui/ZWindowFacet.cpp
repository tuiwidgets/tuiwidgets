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

ZWindowFacetPrivate::~ZWindowFacetPrivate() = default;

TUIWIDGETS_NS_END
