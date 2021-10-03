#ifndef TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED
#define TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED

#include <QObject>

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZDefaultWidgetManager : public QObject {
    Q_OBJECT

public:
    virtual void setDefaultWidget(ZWidget *w) = 0;
    virtual ZWidget *defaultWidget() = 0;
    virtual bool isDefaultWidgetActive() = 0;

protected:
    ~ZDefaultWidgetManager();
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZDEFAULTWINDOWMANAGER_INCLUDED
