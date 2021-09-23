#include "ZWindowFacet.h"
#include "ZWindowFacet_p.h"

TUIWIDGETS_NS_START

ZWindowFacet::ZWindowFacet() {
}

ZWindowFacet::ZWindowFacet(std::unique_ptr<ZWindowFacetPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZWindowFacet::~ZWindowFacet() {
}

bool ZWindowFacet::isExtendViewport() const { return false; }

bool ZWindowFacet::isManuallyPlaced() const { return true; }

void ZWindowFacet::autoPlace(const QSize &available, ZWidget *self) { (void)available; (void)self; }

ZWindowFacetPrivate::~ZWindowFacetPrivate() = default;

TUIWIDGETS_NS_END
