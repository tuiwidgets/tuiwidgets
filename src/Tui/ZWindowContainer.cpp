#include "ZWindowContainer.h"
#include "ZWindowContainer_p.h"

TUIWIDGETS_NS_START

ZWindowContainer::ZWindowContainer() {
}

ZWindowContainer::ZWindowContainer(std::unique_ptr<ZWindowContainerPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZWindowContainer::~ZWindowContainer() {
}

ZWindowContainerPrivate::~ZWindowContainerPrivate() = default;

TUIWIDGETS_NS_END
