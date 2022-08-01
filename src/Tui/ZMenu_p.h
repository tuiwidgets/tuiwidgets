#ifndef TUIWIDGETS_ZMENU_P_INCLUDED
#define TUIWIDGETS_ZMENU_P_INCLUDED

#include <QVector>

#include <Tui/ZMenu.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>
#include <Tui/ZWindowFacet.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

inline namespace IPrivate {
    class MenuWindowFacet : public ZWindowFacet {
        Q_OBJECT
    public:
        bool isExtendViewport() const override { return true; }
    };
}

class ZMenuPrivate : public ZWidgetPrivate {
public:
    ZMenuPrivate(ZWidget *pub);
    ~ZMenuPrivate() override;

public:
    QVector<ZMenuItem> items;
    int selected = 0;
    int textWidth = 0;
    int shortcutWidth = 0;
    QPointer<ZMenubar> parentMenu;

    mutable MenuWindowFacet windowFacet;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENU_P_INCLUDED
