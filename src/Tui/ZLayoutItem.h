// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZLAYOUTITEM_INCLUDED
#define TUIWIDGETS_ZLAYOUTITEM_INCLUDED

#include <memory>

#include <QRect>
#include <QSize>

#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class TUIWIDGETS_EXPORT ZLayoutItem {
public:
    ZLayoutItem();
    virtual ~ZLayoutItem();

    ZLayoutItem &operator=(const ZLayoutItem&) = delete;

public:
    virtual void setGeometry(QRect r) = 0;
    virtual QSize sizeHint() const = 0;
    virtual SizePolicy sizePolicyH() const = 0;
    virtual SizePolicy sizePolicyV() const = 0;
    virtual bool isVisible() const = 0;

    virtual bool isSpacer() const;

    virtual ZWidget *widget();
    virtual ZLayout *layout();

    static std::unique_ptr<ZLayoutItem> wrapWidget(ZWidget *widget);
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZLAYOUTITEM_INCLUDED
