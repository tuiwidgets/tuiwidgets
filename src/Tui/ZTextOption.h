// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZTEXTOPTION_INCLUDED
#define TUIWIDGETS_ZTEXTOPTION_INCLUDED

#include <functional>

#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZFormatRange;
class ZTextStyle;

class ZTextOptionPrivate;

class TUIWIDGETS_EXPORT ZTextOption {
public:
    enum Flag : int {
        ShowTabsAndSpaces = 1 << 0,
        ShowTabsAndSpacesWithColors = 1 << 1
        // ShowLineAndParagraphSeparators, ShowDocumentTerminator and AddSpaceForLineAndParagraphSeparators
        // only fully work with QTextDocument, no use in trying to emulate that here.
    };
    using Flags = QFlags<Flag>;

    enum TabType : int {
        LeftTab = 0,
    };

    enum WrapMode : int {
        NoWrap = 0,
        WordWrap = 1,
        WrapAnywhere = 3,
    };

    class Tab {
    public:
        Tab();
        Tab(int pos, TabType tabType) : position(pos), type(tabType) {}

    public:
        bool operator!=(const Tab &other) const;
        bool operator==(const Tab &other) const;

    public:
        int position = 0;
        TabType type = LeftTab;
    };

public:
    ZTextOption();
    ZTextOption(const ZTextOption&);
    ZTextOption(ZTextOption&&);
    ~ZTextOption();

    ZTextOption &operator=(const ZTextOption&);
    ZTextOption &operator=(ZTextOption&&);

public:
    Flags flags() const;
    void setFlags(Flags flags);

    int tabStopDistance() const;
    void setTabStopDistance(int tabStopDistance);

    QList<Tab> tabs() const;
    void setTabs(const QList<Tab> &tabStops);

    QList<int> tabArray() const;
    void setTabArray(const QList<int> &tabStops);

    WrapMode wrapMode() const;
    void setWrapMode(WrapMode mode);

    void setTrailingWhitespaceColor(std::function<ZTextStyle(const ZTextStyle &baseStyle,
                                                             const ZTextStyle &formatingChars,
                                                             const ZFormatRange*)> colorMapper);
    ZTextStyle mapTrailingWhitespaceColor(const ZTextStyle &baseStyle, const ZTextStyle &formatingChars,
                                          const ZFormatRange *range) const;

    // pos: position in full width tab, size: size of tab actually visible, hidden: number of columns the tab was shortend
    // If space is not sufficient to layout the whole tab hidden will include how much the tab was shortend on the right side
    // and pos will be incremented as well.
    // example: tab size = 4, abcdef\tx
    // display: abcdef  x
    // pos:           23
    // size = 2, hidden = 2
    // example: tab size = 4, abcd\tx
    // display: abcd    x
    // pos:         0123
    // size = 4, hidden = 0
    // example: tab size = 4, abcd\tx, only 3 cells left to render tab (width = 7)
    // display: abcd
    // pos:         123
    // size = 3, hidden = 1
    void setTabColor(std::function<ZTextStyle(int pos, int size, int hidden, const ZTextStyle &baseStyle,
                                              const ZTextStyle &formatingChars, const ZFormatRange*)> colorMapper);
    ZTextStyle mapTabColor(int pos, int size, int hidden, const ZTextStyle &baseStyle,
                           const ZTextStyle &formatingChars, const ZFormatRange *range) const;

private:
    ZValuePtr<ZTextOptionPrivate> tuiwidgets_pimpl_ptr;

    TUIWIDGETS_DECLARE_PRIVATE(ZTextOption)
};

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(ZTextOption::Flags)

TUIWIDGETS_NS_END

TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(ZTextOption::Flags)

#endif // TUIWIDGETS_ZTEXTOPTION_INCLUDED
