#ifndef TUIWIDGETS_ZWINDOWFACET_INCLUDED
#define TUIWIDGETS_ZWINDOWFACET_INCLUDED

#include <QObject>

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowFacetPrivate;
class TUIWIDGETS_EXPORT ZWindowFacet : public QObject {
    Q_OBJECT
public:
    virtual bool isExtendViewport() const;
    virtual bool isManuallyPlaced() { return true; }
    virtual void autoPlace(const QSize &available, ZWidget *self) { (void)available; (void)self; }

protected:
    ZWindowFacet();
    ZWindowFacet(std::unique_ptr<ZWindowFacetPrivate> pimpl);
    ~ZWindowFacet();

    std::unique_ptr<ZWindowFacetPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZWindowFacet)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOWFACET_INCLUDED
