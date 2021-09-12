#ifndef TUIWIDGETS_ZTEXTSTYLE_INCLUDED
#define TUIWIDGETS_ZTEXTSTYLE_INCLUDED

#include <Tui/ZColor.h>
#include <Tui/ZCommon.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZTextStylePrivate;

class TUIWIDGETS_EXPORT ZTextStyle {
public:
    ZTextStyle();
    ZTextStyle(ZColor fg, ZColor bg);
    ZTextStyle(ZColor fg, ZColor bg, ZTextAttributes attributes);
    ZTextStyle(const ZTextStyle&);
    ZTextStyle(ZTextStyle&&);
    ~ZTextStyle();

    ZTextStyle &operator=(const ZTextStyle&);
    ZTextStyle &operator=(ZTextStyle&&);

public:
    ZColor foregroundColor() const;
    void setForegroundColor(const ZColor &foregroundColor);

    ZColor backgroundColor() const;
    void setBackgroundColor(const ZColor &backgroundColor);

    ZTextAttributes attributes() const;
    void setAttributes(ZTextAttributes attributes);

    bool operator==(const ZTextStyle &other) const;
    bool operator!=(const ZTextStyle &other) const { return !(*this == other); }

protected:
    ZValuePtr<ZTextStylePrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZTextStyle)
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZTEXTSTYLE_INCLUDED
