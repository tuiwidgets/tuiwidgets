#ifndef TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED
#define TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED

#include <QObject>

#include <Tui/ZMenuItem.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowContainerPrivate;
class TUIWIDGETS_EXPORT ZWindowContainer : public QObject {
    Q_OBJECT
public:
    ZWindowContainer();
    ~ZWindowContainer() override;

public:
    virtual QVector<ZMenuItem> containerMenuItems() const;

protected:
    ZWindowContainer(std::unique_ptr<ZWindowContainerPrivate> pimpl);

protected:
    std::unique_ptr<ZWindowContainerPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowContainer)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWCONTAINER_INCLUDED
