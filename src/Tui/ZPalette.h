#ifndef TUIWIDGETS_ZPALETTE_INCLUDED
#define TUIWIDGETS_ZPALETTE_INCLUDED

#include <memory>

#include <QSet>
#include <QString>
#include <QStringList>

#include <Tui/ZColor.h>
#include <Tui/ZSymbol.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZPalettePrivate;

template <typename T>
class TUIWIDGETS_EXPORT ZValuePtr : public std::unique_ptr<T> {
public:
    using std::unique_ptr<T>::unique_ptr;
    ZValuePtr(const ZValuePtr &other) {
        if (other) {
            this->reset(new T(*other.get()));
        }
    }

    ZValuePtr& operator=(const ZValuePtr& other) {
        if (other) {
            this->reset(new T(*other.get()));
        }
        return *this;
    }
};

class TUIWIDGETS_EXPORT ZPalette {
public:
    enum Type {
        Publish,
        Local
    };

    struct ColorDef {
        ZImplicitSymbol name;
        ZColor color;
    };

    struct AliasDef {
        ZImplicitSymbol name;
        ZImplicitSymbol fallback;
    };

    struct RuleCmd {
        Type type;
        ZImplicitSymbol name;
        ZImplicitSymbol reference;
    };

    struct RuleDef {
        QSet<QString> classes;
        QList<RuleCmd> cmds;
    };

public:
    ZPalette();
    ZPalette(const ZPalette &other);
    virtual ~ZPalette();

public:
    ZColor getColor(Tui::ZWidget *targetWidget, ZImplicitSymbol x) const;

    void setColors(QList<ColorDef> newColors);
    void addLocalAlias(QList<AliasDef> newAliases);
    void addRules(QList<RuleDef> newRules);

    ZPalette& operator=(const ZPalette& other);

protected:
    ZValuePtr<ZPalettePrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPalette)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPALETTE_INCLUDED
