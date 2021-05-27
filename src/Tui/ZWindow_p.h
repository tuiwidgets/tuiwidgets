#ifndef TUIWIDGETS_ZWINDOW_P_INCLUDED
#define TUIWIDGETS_ZWINDOW_P_INCLUDED

#include <Tui/ZWidget_p.h>
#include <Tui/ZWindow.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowPrivate : public ZWidgetPrivate {
public:
    ZWindowPrivate(ZWidget *pub);
    ~ZWindowPrivate() override = default;

public:
    ZWindow::Options options;
    QString windowTitle;
    Qt::Edges borders = Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge | Qt::LeftEdge;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOW_P_INCLUDED
