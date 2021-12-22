#include "ZTextOption.h"
#include "ZTextOption_p.h"

#include <QVector>

#include <Tui/ZFormatRange.h>
#include <Tui/ZTextStyle.h>

TUIWIDGETS_NS_START

ZTextOption::ZTextOption() = default;

ZTextOption::ZTextOption(const ZTextOption&) = default;

ZTextOption::ZTextOption(ZTextOption&&) = default;

ZTextOption::~ZTextOption() = default;

ZTextOption &ZTextOption::operator=(const ZTextOption &other) = default;

ZTextOption &ZTextOption::operator=(ZTextOption &&other) = default;

ZTextOption::Flags ZTextOption::flags() const {
    auto *const p = tuiwidgets_impl();
    return p->flags;
}

void ZTextOption::setFlags(ZTextOption::Flags flags) {
    auto *const p = tuiwidgets_impl();
    p->flags = flags;
}

void ZTextOption::setTabArray(const QList<int> &tabStops) {
    auto *const p = tuiwidgets_impl();
    p->tabs.clear();
    p->tabs.reserve(tabStops.size());

    for (int stop: tabStops) {
        p->tabs.append(ZTextOption::Tab(stop, LeftTab));
    }
}

void ZTextOption::setTabStopDistance(int tabStopDistance) {
    auto *const p = tuiwidgets_impl();
    p->tabDistance = std::max(tabStopDistance, 1);
}

int ZTextOption::tabStopDistance() const {
    auto *const p = tuiwidgets_impl();
    return p->tabDistance;
}

void ZTextOption::setTabs(const QList<ZTextOption::Tab> &tabStops) {
    auto *const p = tuiwidgets_impl();
    p->tabs = tabStops;
}

QList<int> ZTextOption::tabArray() const {
    auto *const p = tuiwidgets_impl();
    QList<int> result;
    result.reserve(p->tabs.size());
    for (const Tab &t: p->tabs) {
        result.append(t.position);
    }
    return result;
}

QList<ZTextOption::Tab> ZTextOption::tabs() const {
    auto *const p = tuiwidgets_impl();
    return p->tabs;
}

ZTextOption::WrapMode ZTextOption::wrapMode() const {
    auto *const p = tuiwidgets_impl();
    return p->wrapMode;
}

void ZTextOption::setWrapMode(ZTextOption::WrapMode mode) {
    auto *const p = tuiwidgets_impl();
    p->wrapMode = mode;
}

void ZTextOption::setTrailingWhitespaceColor(std::function<ZTextStyle(const ZTextStyle &baseStyle,
                                                                      const ZTextStyle &formatingChars,
                                                                      const ZFormatRange*)> colorMapper) {
    auto *const p = tuiwidgets_impl();
    p->trailingWhitespaceColor = colorMapper;
}

ZTextStyle ZTextOption::mapTrailingWhitespaceColor(const ZTextStyle &baseStyle, const ZTextStyle &formatingChars,
                                                   const ZFormatRange *range) const {
    auto *const p = tuiwidgets_impl();
    if (!p->trailingWhitespaceColor) {
        return range ? range->format() : formatingChars;
    }
    return p->trailingWhitespaceColor(baseStyle, formatingChars, range);
}

void ZTextOption::setTabColor(std::function<ZTextStyle(int pos, int size, int hidden, const ZTextStyle &baseStyle,
                                                       const ZTextStyle &formatingChars, const ZFormatRange*)> colorMapper) {
    auto *const p = tuiwidgets_impl();
    p->tabColor = colorMapper;
}

ZTextStyle ZTextOption::mapTabColor(int pos, int size, int hidden, const ZTextStyle &baseStyle, const ZTextStyle &formatingChars, const ZFormatRange *range) const {
    auto *const p = tuiwidgets_impl();
    if (!p->tabColor) {
        return range ? range->format() : formatingChars;
    }
    return p->tabColor(pos, size, hidden, baseStyle, formatingChars, range);
}

bool ZTextOption::Tab::operator!=(const ZTextOption::Tab &other) const {
    return !(*this == other);
}

bool ZTextOption::Tab::operator==(const ZTextOption::Tab &other) const {
    return type == other.type && position == other.position;
}

TUIWIDGETS_NS_END
