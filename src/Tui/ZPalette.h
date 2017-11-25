#ifndef TUIWIDGETS_ZPALETTE_INCLUDED
#define TUIWIDGETS_ZPALETTE_INCLUDED

#include <memory>

#include <QSet>
#include <QString>
#include <QStringList>

#include <Tui/ZColor.h>

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
        QString name;
        ZColor color;
    };

    struct AliasDef {
        QString name;
        QString fallback;
    };

    struct RuleCmd {
        Type type;
        QString name;
        QString reference;
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
    ZColor getColor(Tui::ZWidget *targetWidget, QString x) const;

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
