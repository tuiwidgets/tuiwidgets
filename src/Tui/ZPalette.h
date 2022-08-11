#ifndef TUIWIDGETS_ZPALETTE_INCLUDED
#define TUIWIDGETS_ZPALETTE_INCLUDED

#include <memory>

#include <QSet>
#include <QString>
#include <QStringList>

#include <Tui/ZColor.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;

class ZPalettePrivate;

class TUIWIDGETS_EXPORT ZPalette {
public:
    enum Type : int {
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

    static ZPalette classic();
    static ZPalette black();

public:
    ZColor getColor(ZWidget *targetWidget, ZImplicitSymbol x) const;

    void setColors(QList<ColorDef> newColors);
    //void addLocalAlias(QList<AliasDef> newAliases);
    void addRules(QList<RuleDef> newRules);

    ZPalette& operator=(const ZPalette& other);

    bool isNull() const;

protected:
    ZValuePtr<ZPalettePrivate> tuiwidgets_pimpl_ptr;

private:
    static void setDefaultRules(ZPalette &p);

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPalette)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZPALETTE_INCLUDED
