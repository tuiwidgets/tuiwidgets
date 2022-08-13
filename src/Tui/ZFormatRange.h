// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZFORMATRANGE_INCLUDED
#define TUIWIDGETS_ZFORMATRANGE_INCLUDED

#include <Tui/ZTextStyle.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZFormatRangePrivate;

class TUIWIDGETS_EXPORT ZFormatRange {
public:
    ZFormatRange();
    ZFormatRange(int start, int length, ZTextStyle format, ZTextStyle formattingChar, int userData = 0);
    ZFormatRange(const ZFormatRange&);
    ZFormatRange(ZFormatRange&&);
    ~ZFormatRange();

    ZFormatRange &operator=(const ZFormatRange&);
    ZFormatRange &operator=(ZFormatRange&&);

public:
    int start() const;
    void setStart(int start);

    int length() const;
    void setLength(int length);

    ZTextStyle format() const;
    void setFormat(const ZTextStyle &format);

    ZTextStyle formattingChar() const;
    void setFormattingChar(const ZTextStyle &formattingChar);

    int userData() const;
    void setUserData(int userData);

private:
    ZValuePtr<ZFormatRangePrivate> tuiwidgets_pimpl_ptr;

    TUIWIDGETS_DECLARE_PRIVATE(ZFormatRange)
};


TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZFORMATRANGE_INCLUDED
