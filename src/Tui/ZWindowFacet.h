#ifndef TUIWIDGETS_ZWINDOWFACET_INCLUDED
#define TUIWIDGETS_ZWINDOWFACET_INCLUDED

#include <QObject>

#include <Tui/ZWidget.h>
#include <Tui/ZWindowContainer.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowFacetPrivate;
class TUIWIDGETS_EXPORT ZWindowFacet : public QObject {
    Q_OBJECT
public:
    virtual bool isExtendViewport() const;
    virtual bool isManuallyPlaced() const;
    virtual void autoPlace(const QSize &available, ZWidget *self);
    virtual ZWindowContainer *container() const;
    virtual void setContainer(ZWindowContainer *container);

    virtual void setManuallyPlaced(bool manual);

protected:
    ZWindowFacet();
    ZWindowFacet(std::unique_ptr<ZWindowFacetPrivate> pimpl);
    ~ZWindowFacet() override;

    std::unique_ptr<ZWindowFacetPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowFacet)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWFACET_INCLUDED
