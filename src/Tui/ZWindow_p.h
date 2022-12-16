// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZWINDOW_P_INCLUDED
#define TUIWIDGETS_ZWINDOW_P_INCLUDED

#include <Tui/ZWidget_p.h>
#include <Tui/ZWindow.h>
#include <Tui/ZBasicWindowFacet.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWindowPrivate : public ZWidgetPrivate {
public:
    ZWindowPrivate(ZWidget *pub);
    ~ZWindowPrivate() override = default;

public:
    void ensureAutoPlacement();
    void startInteractiveGeometry(ZWindow *pub);
    void cancelInteractiveGeometry(ZWindow *pub);
    void finalizeInteractiveGeometry(ZWindow *pub);

public:
    ZWindow::Options options;
    QString windowTitle;
    Edges borders = TopEdge | RightEdge | BottomEdge | LeftEdge;
    mutable std::unique_ptr<ZBasicWindowFacet> windowFacet;
    bool interactiveMode = false;
    QRect interactiveInitialGeometry;
    bool interactiveInitialManuallyPlaced = false;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWINDOW_P_INCLUDED
