#ifndef TUIWIDGETS_ZCHECKBOX_P_INCLUDED
#define TUIWIDGETS_ZCHECKBOX_P_INCLUDED

#include <Tui/ZCheckBox.h>

#include <Tui/ZStyledTextLine.h>
#include <Tui/ZWidget_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCheckBoxPrivate : public ZWidgetPrivate {
public:
    ZCheckBoxPrivate(ZWidget *pub);
    ~ZCheckBoxPrivate() override;

public:
    ZStyledTextLine styledText;
    Qt::CheckState state = Qt::Unchecked;
    bool tristate = false;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCHECKBOX_P_INCLUDED
