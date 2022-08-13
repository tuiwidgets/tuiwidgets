// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZSYMBOL_INCLUDED
#define TUIWIDGETS_ZSYMBOL_INCLUDED

#include <QMetaType>
#include <QString>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class TUIWIDGETS_EXPORT ZSymbol {
public:
    constexpr ZSymbol() = default;
    ZSymbol(const ZSymbol&) = default;
    explicit ZSymbol(QString str) : id(lookup(str, true)) {}

    QString toString() const;

    ZSymbol &operator=(const ZSymbol&) = default;

    friend bool operator==(const ZSymbol &s1, const ZSymbol &s2) {
        return s1.id == s2.id;
    }

    friend bool operator!=(const ZSymbol &s1, const ZSymbol &s2) {
        return !(s1 == s2);
    }

    friend bool operator<(const ZSymbol &s1, const ZSymbol &s2) {
        return s1.id < s2.id;
    }

    friend bool operator<=(const ZSymbol &s1, const ZSymbol &s2){
        return s1.id <= s2.id;
    }

    friend bool operator>(const ZSymbol &s1, const ZSymbol &s2) {
        return s2 < s1;
    }

    friend bool operator>=(const ZSymbol &s1, const ZSymbol &s2) {
        return s2 <= s1;
    }

    explicit operator bool() const {
        return id != 0;
    }

    friend uint qHash(const ZSymbol &key) {
        return key.id;
    }

    friend struct std::hash<ZSymbol>;

private:
    static int lookup(QString str, bool create);

private:
    int id = 0;
};

QDebug operator<<(QDebug dbg, const ZSymbol &message);

#define TUISYM_LITERAL(x) ([] { static ::Tui::ZSymbol m{QString::fromUtf8(x)}; return m; }())

class TUIWIDGETS_EXPORT ZImplicitSymbol : public ZSymbol {
public:
    constexpr ZImplicitSymbol() = default;
    ZImplicitSymbol(const ZSymbol &other) : ZSymbol(other) {}
    ZImplicitSymbol(QString str) : ZSymbol(str) {}
    template <int N> ZImplicitSymbol(const char(&literal)[N]) : ZSymbol(QString::fromUtf8(literal)) {}
};

TUIWIDGETS_NS_END

Q_DECLARE_METATYPE(Tui::ZSymbol);
Q_DECLARE_TYPEINFO(Tui::ZSymbol, Q_MOVABLE_TYPE);

namespace std
{
    template<> struct hash<Tui::ZSymbol>
    {
        std::size_t operator()(const Tui::ZSymbol &key) const noexcept
        {
            return key.id;
        }
    };
}

#endif // TUIWIDGETS_ZSYMBOL_INCLUDED
