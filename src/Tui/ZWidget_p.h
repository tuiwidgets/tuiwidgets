#ifndef TUIWIDGETS_ZWIDGET_P_INCLUDED
#define TUIWIDGETS_ZWIDGET_P_INCLUDED

#include <QRect>

#include <Tui/tuiwidgets_internal.h>

#include <QPointer>
#include <QVector>

#include <Tui/ZPalette.h>
#include <Tui/ListNode_p.h>

#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

#define tui_magic_v0 0xbdf78943

class ZTerminal;

struct FocusHistoryTag;

template <typename CALLABLE>
void zwidgetForEachDescendant(ZWidget *start, CALLABLE&& callable) {
    QVector<QPointer<QObject>> todo;
    for (QObject *x : start->children()) {
        todo.append(x);
    }
    while (todo.size()) {
        QObject *o = todo.takeLast();
        if (!o) continue;
        for (QObject *x : o->children()) {
            todo.append(x);
        }
        callable(o);
    }
}

class ZWidgetPrivate {
public:
    ZWidgetPrivate(ZWidget *pub);
    virtual ~ZWidgetPrivate();

    void updateRequestEvent(ZPaintEvent *event);

    ZTerminal *findTerminal() const;

    void unsetTerminal();
    void setManagingTerminal(ZTerminal *terminal);

    bool isFocusable() const { return enabled && focusPolicy != Qt::FocusPolicy::NoFocus; }

    void updateEffectivelyEnabledRecursively();
    void updateEffectivelyVisibleRecursively();

    // variables
    QRect geometry;
    Qt::FocusPolicy focusPolicy = Qt::NoFocus;
    FocusContainerMode focusMode = FocusContainerMode::None;
    int focusOrder = 0;
    int stackingLayer = 0;

    bool enabled = true;
    bool visible = true;

    bool effectivelyEnabled = true;
    bool effectivelyVisible = true;

    QSize minimumSize;
    QSize maximumSize = {tuiMaxSize, tuiMaxSize};
    SizePolicy sizePolicyH = SizePolicy::Preferred;
    SizePolicy sizePolicyV = SizePolicy::Preferred;
    QPointer<ZLayout> layout;

    QMargins contentsMargins;

    ZPalette palette;
    QStringList paletteClass;

    CursorStyle cursorStyle = CursorStyle::Unset;
    int cursorColorR = -1, cursorColorG = -1, cursorColorB = -1;

    ZTerminal *terminal = nullptr;
    ListNode<ZWidgetPrivate> focusHistory;

    ZCommandManager *commandManager = nullptr;

    // scratch storage for ZTerminal::doLayout
    int doLayoutScratchDepth;

    // back door
    static ZWidgetPrivate *get(ZWidget *widget) { return widget->tuiwidgets_impl(); }
    static const ZWidgetPrivate *get(const ZWidget *widget) { return widget->tuiwidgets_impl(); }

    // internal
    const unsigned int tui_magic = tui_magic_v0;
    ZWidget *pub_ptr;
    TUIWIDGETS_DECLARE_PUBLIC(ZWidget)
};

template<>
struct ListTrait<FocusHistoryTag> {
    static constexpr auto offset = &ZWidgetPrivate::focusHistory;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZWIDGET_P_INCLUDED
