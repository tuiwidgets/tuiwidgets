#include "ZWindowContainer.h"
#include "ZWindowContainer_p.h"

#include <QVector>

TUIWIDGETS_NS_START

ZWindowContainer::ZWindowContainer() {
}

ZWindowContainer::ZWindowContainer(std::unique_ptr<ZWindowContainerPrivate> pimpl)
    : tuiwidgets_pimpl_ptr(move(pimpl))
{
}

ZWindowContainer::~ZWindowContainer() {
}

QVector<ZMenuItem> ZWindowContainer::containerMenuItems() const {
    return {};
}

ZWindowContainerPrivate::~ZWindowContainerPrivate() = default;

TUIWIDGETS_NS_END
