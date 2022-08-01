#ifndef TUIWIDGETS_ZLISTVIEW_P_INCLUDED
#define TUIWIDGETS_ZLISTVIEW_P_INCLUDED

#include <Tui/ZListView.h>

#include <QPointer>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZListViewPrivate : public ZWidgetPrivate {
public:
    ZListViewPrivate(ZWidget *pub);
    ~ZListViewPrivate() override;

public:
    ZStyledTextLine styledText;
    QAbstractItemModel *model = nullptr;
    QPointer<QItemSelectionModel> selectionModel;
    int lastSelectedRow = 0;
    int scrollPosition = 0;
    QAbstractItemModel *allocatedModel = nullptr;

    TUIWIDGETS_DECLARE_PUBLIC(ZListView)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLISTVIEW_P_INCLUDED
