#include "ZPalette.h"
#include "ZPalette_p.h"

#include <QMap>

#include <Tui/ZColor.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

Tui::ZPalettePrivate::ZPalettePrivate() {

}

ZPalette::ZPalette() : tuiwidgets_pimpl_ptr(new ZPalettePrivate()) {
}

ZPalette::ZPalette(const ZPalette &other) = default;

ZPalette::~ZPalette() {

}

ZColor ZPalette::getColor(ZWidget *targetWidget, ZImplicitSymbol x) const {
    auto *const p = tuiwidgets_impl();

    // TODO Most stupid impl., needs optimization

    QList<ZWidget*> widgets;
    {
        ZWidget *w = targetWidget;
        while (w) {
            widgets.prepend(w);
            w = w->parentWidget();
        }
    }

    QHash<ZSymbol, ZColor> defs;
    QList<const RuleDef*> rules;

    for (ZWidget *w : widgets) {
        QSet<QString> widgetClasses = w->paletteClass().toSet();

        const ZPalettePrivate &pal = *w->palette().tuiwidgets_pimpl_ptr;

        for (const auto &rule : qAsConst(pal.rules)) {
            rules.append(&rule);
        }

        QMap<int, QList<const RuleDef*>> matchingRulesByLen;

        for (const RuleDef *rule : rules) {
            if (widgetClasses.contains(rule->classes)) {
                matchingRulesByLen[rule->classes.size()].append(rule);
            }
        }

        for (auto &rs : qAsConst(matchingRulesByLen)) {
            for (const RuleDef* r : qAsConst(rs)) {
                for (const RuleCmd &cmd : r->cmds) {
                    if (cmd.type == Publish || w == targetWidget) {
                        if (defs.contains(cmd.reference)) {
                            defs[cmd.name] = defs[cmd.reference];
                        }
                    }
                }
            }
        }

        for (ZSymbol key : pal.colorDefinitions.keys()) {
            defs[key] = pal.colorDefinitions[key];
        }
    }

    if (defs.contains(x)) {
        return defs.value(x);
    }
    return {0xff, 0, 0};
}

void ZPalette::setColors(QList<ZPalette::ColorDef> newColors) {
    auto *const p = tuiwidgets_impl();

    for (const auto &nc : newColors) {
        p->colorDefinitions[nc.name] = nc.color;
    }
}

void ZPalette::addLocalAlias(QList<ZPalette::AliasDef> newAliases) {
    auto *const p = tuiwidgets_impl();

    for (const auto &nc : newAliases) {
        p->localAlias[nc.name] = nc.fallback;
    }
}

void ZPalette::addRules(QList<ZPalette::RuleDef> newRules) {
    auto *const p = tuiwidgets_impl();

    p->rules += newRules;
}

ZPalette &ZPalette::operator=(const ZPalette &other) = default;

TUIWIDGETS_NS_END
