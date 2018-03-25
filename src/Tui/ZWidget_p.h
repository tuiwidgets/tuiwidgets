#ifndef TUIWIDGETS_ZWIDGET_P_INCLUDED
#define TUIWIDGETS_ZWIDGET_P_INCLUDED

#include <QRect>

#include <Tui/tuiwidgets_internal.h>

#include <Tui/ZPalette.h>

#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

#define tui_magic_v0 0xbdf78943

class ZTerminal;

class ZWidgetPrivate {
public:
    ZWidgetPrivate(ZWidget *pub);
    virtual ~ZWidgetPrivate();

    void updateRequestEvent(ZPaintEvent *event);

    ZTerminal *findTerminal() const;

    void unsetTerminal();
    void setManagingTerminal(ZTerminal *terminal);

    bool isFocusable() const { return enabled && focusPolicy != Qt::FocusPolicy::NoFocus; }

    // variables
    QRect geometry;
    Qt::FocusPolicy focusPolicy = Qt::NoFocus;
    FocusContainerMode focusMode = FocusContainerMode::None;
    int focusOrder = 0;

    bool enabled = true;
    bool visible = true;

    QSize minimumSize;
    QSize maximumSize;
    ZLayout *layout = nullptr;

    QMargins contentsMargins;

    ZPalette palette;
    QStringList paletteClass;

    ZTerminal *terminal = nullptr;

    ZCommandManager* commandManager = nullptr;

    // back door
    static ZWidgetPrivate* get(ZWidget *widget) { return widget->tuiwidgets_impl(); }
    static const ZWidgetPrivate* get(const ZWidget *widget) { return widget->tuiwidgets_impl(); }

    // internal
    const unsigned int tui_magic = tui_magic_v0;
    ZWidget* pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZWidget)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWIDGET_P_INCLUDED
