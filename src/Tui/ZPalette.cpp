#include "ZPalette.h"
#include "ZPalette_p.h"

#include <QMap>

#include <Tui/ZColor.h>
#include <Tui/ZWidget.h>

TUIWIDGETS_NS_START

Tui::ZPalettePrivate::ZPalettePrivate() {

}

ZPalette::ZPalette() {
}

ZPalette::ZPalette(const ZPalette &other) = default;

ZPalette::~ZPalette() {

}

ZColor ZPalette::getColor(ZWidget *targetWidget, ZImplicitSymbol x) const {
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

bool ZPalette::isNull() const {
    auto *const p = tuiwidgets_impl();

    return p->colorDefinitions.isEmpty() && p->rules.isEmpty() && p->localAlias.isEmpty();
}

ZPalette &ZPalette::operator=(const ZPalette &other) = default;

void ZPalette::setDefaultRules(ZPalette &p) {
    p.addRules({
        {{ QStringLiteral("window") }, {
             { p.Publish, "bg", "window.bg" },

             { p.Publish, "window.bg", "window.default.bg" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.default.frame.focused.fg" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.default.frame.focused.control.fg" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.default.frame.unfocused.fg" },

             { p.Publish, "scrollbar.bg", "window.default.scrollbar.bg" },
             { p.Publish, "scrollbar.fg", "window.default.scrollbar.fg" },
             { p.Publish, "scrollbar.control.bg", "window.default.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.default.scrollbar.control.fg" },

             { p.Publish, "text.bg", "window.default.text.bg" },
             { p.Publish, "text.fg", "window.default.text.fg" },
             { p.Publish, "text.selected.bg", "window.default.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.default.text.selected.fg" },

             { p.Publish, "control.bg", "window.default.control.bg" },
             { p.Publish, "control.fg", "window.default.control.fg" },
             { p.Publish, "control.focused.bg", "window.default.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.default.control.focused.fg" },
             { p.Publish, "control.disabled.bg", "window.default.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.default.control.disabled.fg" },
             { p.Publish, "control.shortcut.bg", "window.default.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.default.control.shortcut.fg" },

             { p.Publish, "dataview.bg", "window.default.dataview.bg" },
             { p.Publish, "dataview.fg", "window.default.dataview.fg" },
             { p.Publish, "dataview.selected.bg", "window.default.dataview.selected.bg" },
             { p.Publish, "dataview.selected.fg", "window.default.dataview.selected.fg" },
             { p.Publish, "dataview.selected.focused.bg", "window.default.dataview.selected.focused.bg" },
             { p.Publish, "dataview.selected.focused.fg", "window.default.dataview.selected.focused.fg" },
             { p.Publish, "dataview.disabled.bg", "window.default.dataview.disabled.bg" },
             { p.Publish, "dataview.disabled.fg", "window.default.dataview.disabled.fg" },
             { p.Publish, "dataview.disabled.selected.bg", "window.default.dataview.disabled.selected.bg" },
             { p.Publish, "dataview.disabled.selected.fg", "window.default.dataview.disabled.selected.fg" },

             { p.Publish, "button.bg", "window.default.button.bg" },
             { p.Publish, "button.fg", "window.default.button.fg" },
             { p.Publish, "button.default.bg", "window.default.button.default.bg" },
             { p.Publish, "button.default.fg", "window.default.button.default.fg" },
             { p.Publish, "button.focused.bg", "window.default.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.default.button.focused.fg" },
             { p.Publish, "button.disabled.bg", "window.default.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.default.button.disabled.fg" },
             { p.Publish, "button.shortcut.bg", "window.default.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.default.button.shortcut.fg" },

             { p.Publish, "lineedit.bg", "window.default.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.default.lineedit.fg" },
             { p.Publish, "lineedit.focused.bg", "window.default.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.default.lineedit.focused.fg" },
        }},

        {{ QStringLiteral("window"), QStringLiteral("dialog") }, {
             { p.Publish, "bg", "window.bg" },

             { p.Publish, "window.bg", "window.gray.bg" },
             { p.Publish, "window.frame.focused.bg", "window.bg" },
             { p.Publish, "window.frame.focused.fg", "window.gray.frame.focused.fg" },
             { p.Publish, "window.frame.focused.control.bg", "window.bg" },
             { p.Publish, "window.frame.focused.control.fg", "window.gray.frame.focused.control.fg" },
             { p.Publish, "window.frame.unfocused.bg", "window.bg" },
             { p.Publish, "window.frame.unfocused.fg", "window.gray.frame.unfocused.fg" },

             { p.Publish, "scrollbar.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.fg", "window.gray.scrollbar.control.fg" },
             { p.Publish, "scrollbar.control.bg", "window.gray.scrollbar.control.bg" },
             { p.Publish, "scrollbar.control.fg", "window.gray.scrollbar.control.fg" },

             { p.Publish, "text.bg", "window.gray.text.bg" },
             { p.Publish, "text.fg", "window.gray.text.fg" },
             { p.Publish, "text.selected.bg", "window.gray.text.selected.bg" },
             { p.Publish, "text.selected.fg", "window.gray.text.selected.fg" },

             { p.Publish, "control.bg", "window.gray.control.bg" },
             { p.Publish, "control.fg", "window.gray.control.fg" },
             { p.Publish, "control.focused.bg", "window.gray.control.focused.bg" },
             { p.Publish, "control.focused.fg", "window.gray.control.focused.fg" },
             { p.Publish, "control.disabled.bg", "window.gray.control.disabled.bg" },
             { p.Publish, "control.disabled.fg", "window.gray.control.disabled.fg" },
             { p.Publish, "control.shortcut.bg", "window.gray.control.shortcut.bg" },
             { p.Publish, "control.shortcut.fg", "window.gray.control.shortcut.fg" },

             { p.Publish, "dataview.bg", "window.gray.dataview.bg" },
             { p.Publish, "dataview.fg", "window.gray.dataview.fg" },
             { p.Publish, "dataview.selected.bg", "window.gray.dataview.selected.bg" },
             { p.Publish, "dataview.selected.fg", "window.gray.dataview.selected.fg" },
             { p.Publish, "dataview.selected.focused.bg", "window.gray.dataview.selected.focused.bg" },
             { p.Publish, "dataview.selected.focused.fg", "window.gray.dataview.selected.focused.fg" },
             { p.Publish, "dataview.disabled.bg", "window.gray.dataview.disabled.bg" },
             { p.Publish, "dataview.disabled.fg", "window.gray.dataview.disabled.fg" },
             { p.Publish, "dataview.disabled.selected.bg", "window.gray.dataview.disabled.selected.bg" },
             { p.Publish, "dataview.disabled.selected.fg", "window.gray.dataview.disabled.selected.fg" },

             { p.Publish, "button.bg", "window.gray.button.bg" },
             { p.Publish, "button.fg", "window.gray.button.fg" },
             { p.Publish, "button.default.bg", "window.gray.button.default.bg" },
             { p.Publish, "button.default.fg", "window.gray.button.default.fg" },
             { p.Publish, "button.focused.bg", "window.gray.button.focused.bg" },
             { p.Publish, "button.focused.fg", "window.gray.button.focused.fg" },
             { p.Publish, "button.disabled.bg", "window.gray.button.disabled.bg" },
             { p.Publish, "button.disabled.fg", "window.gray.button.disabled.fg" },
             { p.Publish, "button.shortcut.bg", "window.gray.button.shortcut.bg" },
             { p.Publish, "button.shortcut.fg", "window.gray.button.shortcut.fg" },

             { p.Publish, "lineedit.bg", "window.gray.lineedit.bg" },
             { p.Publish, "lineedit.fg", "window.gray.lineedit.fg" },
             { p.Publish, "lineedit.focused.bg", "window.gray.lineedit.focused.bg" },
             { p.Publish, "lineedit.focused.fg", "window.gray.lineedit.focused.fg" },
       }},

       {{ QStringLiteral("window"), QStringLiteral("cyan") }, {
            { p.Publish, "bg", "window.bg" },

            { p.Publish, "window.bg", "window.cyan.bg" },
            { p.Publish, "window.frame.focused.bg", "window.bg" },
            { p.Publish, "window.frame.focused.fg", "window.cyan.frame.focused.fg" },
            { p.Publish, "window.frame.focused.control.bg", "window.bg" },
            { p.Publish, "window.frame.focused.control.fg", "window.cyan.frame.focused.control.fg" },
            { p.Publish, "window.frame.unfocused.bg", "window.bg" },
            { p.Publish, "window.frame.unfocused.fg", "window.cyan.frame.unfocused.fg" },

            { p.Publish, "scrollbar.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.fg", "window.cyan.scrollbar.control.fg" },
            { p.Publish, "scrollbar.control.bg", "window.cyan.scrollbar.control.bg" },
            { p.Publish, "scrollbar.control.fg", "window.cyan.scrollbar.control.fg" },

            { p.Publish, "text.bg", "window.cyan.text.bg" },
            { p.Publish, "text.fg", "window.cyan.text.fg" },
            { p.Publish, "text.selected.bg", "window.cyan.text.selected.bg" },
            { p.Publish, "text.selected.fg", "window.cyan.text.selected.fg" },

            { p.Publish, "control.bg", "window.cyan.control.bg" },
            { p.Publish, "control.fg", "window.cyan.control.fg" },
            { p.Publish, "control.focused.bg", "window.cyan.control.focused.bg" },
            { p.Publish, "control.focused.fg", "window.cyan.control.focused.fg" },
            { p.Publish, "control.disabled.bg", "window.cyan.control.disabled.bg" },
            { p.Publish, "control.disabled.fg", "window.cyan.control.disabled.fg" },
            { p.Publish, "control.shortcut.bg", "window.cyan.control.shortcut.bg" },
            { p.Publish, "control.shortcut.fg", "window.cyan.control.shortcut.fg" },

            { p.Publish, "dataview.bg", "window.cyan.dataview.bg" },
            { p.Publish, "dataview.fg", "window.cyan.dataview.fg" },
            { p.Publish, "dataview.selected.bg", "window.cyan.dataview.selected.bg" },
            { p.Publish, "dataview.selected.fg", "window.cyan.dataview.selected.fg" },
            { p.Publish, "dataview.selected.focused.bg", "window.cyan.dataview.selected.focused.bg" },
            { p.Publish, "dataview.selected.focused.fg", "window.cyan.dataview.selected.focused.fg" },
            { p.Publish, "dataview.disabled.bg", "window.cyan.dataview.disabled.bg" },
            { p.Publish, "dataview.disabled.fg", "window.cyan.dataview.disabled.fg" },
            { p.Publish, "dataview.disabled.selected.bg", "window.cyan.dataview.disabled.selected.bg" },
            { p.Publish, "dataview.disabled.selected.fg", "window.cyan.dataview.disabled.selected.fg" },

            { p.Publish, "button.bg", "window.cyan.button.bg" },
            { p.Publish, "button.fg", "window.cyan.button.fg" },
            { p.Publish, "button.default.bg", "window.cyan.button.default.bg" },
            { p.Publish, "button.default.fg", "window.cyan.button.default.fg" },
            { p.Publish, "button.focused.bg", "window.cyan.button.focused.bg" },
            { p.Publish, "button.focused.fg", "window.cyan.button.focused.fg" },
            { p.Publish, "button.disabled.bg", "window.cyan.button.disabled.bg" },
            { p.Publish, "button.disabled.fg", "window.cyan.button.disabled.fg" },
            { p.Publish, "button.shortcut.bg", "window.cyan.button.shortcut.bg" },
            { p.Publish, "button.shortcut.fg", "window.cyan.button.shortcut.fg" },

            { p.Publish, "lineedit.bg", "window.cyan.lineedit.bg" },
            { p.Publish, "lineedit.fg", "window.cyan.lineedit.fg" },
            { p.Publish, "lineedit.focused.bg", "window.cyan.lineedit.focused.bg" },
            { p.Publish, "lineedit.focused.fg", "window.cyan.lineedit.focused.fg" },
       }},

    });
}

ZPalette ZPalette::classic() {
    ZPalette p;
    p.setColors({
        { "root.bg", Colors::black},
        { "root.fg", {0x80, 0x80, 0x80}},

        { "menu.bg", Colors::lightGray},
        { "menu.fg", Colors::black},
        { "menu.disabled.bg", Colors::lightGray},
        { "menu.disabled.fg", Colors::darkGray},
        { "menu.shortcut.bg", Colors::lightGray},
        { "menu.shortcut.fg", Colors::red},
        { "menu.selected.bg", Colors::green},
        { "menu.selected.fg", Colors::black},
        { "menu.selected.disabled.bg", Colors::green},
        { "menu.selected.disabled.fg", Colors::darkGray},
        { "menu.selected.shortcut.bg", Colors::green},
        { "menu.selected.shortcut.fg", Colors::red},

        { "window.default.bg", Colors::blue},
        { "window.default.frame.focused.fg", Colors::brightWhite},
        { "window.default.frame.focused.control.fg", Colors::brightGreen},
        { "window.default.frame.unfocused.fg", Colors::lightGray},
        { "window.default.scrollbar.fg", Colors::blue},
        { "window.default.scrollbar.bg", { 0, 0x55, 0xaa}},
        { "window.default.scrollbar.control.fg", Colors::blue},
        { "window.default.scrollbar.control.bg", Colors::cyan},
        { "window.default.text.fg", Colors::brightYellow},
        { "window.default.text.bg", Colors::blue},
        { "window.default.text.selected.fg", Colors::blue},
        { "window.default.text.selected.bg", Colors::lightGray},
        { "window.default.control.bg", Colors::blue},
        { "window.default.control.fg", Colors::lightGray},
        { "window.default.control.focused.bg", Colors::blue},
        { "window.default.control.focused.fg", Colors::brightWhite},
        { "window.default.control.disabled.bg", Colors::blue},
        { "window.default.control.disabled.fg", Colors::black},
        { "window.default.control.shortcut.bg", Colors::blue},
        { "window.default.control.shortcut.fg", Colors::brightYellow},
        { "window.default.dataview.bg", Colors::cyan},
        { "window.default.dataview.fg", Colors::black},
        { "window.default.dataview.selected.bg", Colors::cyan},
        { "window.default.dataview.selected.fg", Colors::brightYellow},
        { "window.default.dataview.selected.focused.bg", Colors::green},
        { "window.default.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.default.dataview.disabled.bg", Colors::lightGray},
        { "window.default.dataview.disabled.fg", Colors::darkGray},
        { "window.default.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.default.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.default.button.bg", Colors::green},
        { "window.default.button.fg", Colors::black},
        { "window.default.button.default.bg", Colors::green},
        { "window.default.button.default.fg", Colors::brightCyan},
        { "window.default.button.focused.bg", Colors::green},
        { "window.default.button.focused.fg", Colors::brightWhite},
        { "window.default.button.disabled.bg", Colors::lightGray},
        { "window.default.button.disabled.fg", Colors::darkGray},
        { "window.default.button.shortcut.bg", Colors::green},
        { "window.default.button.shortcut.fg", Colors::brightYellow},
        { "window.default.lineedit.bg", Colors::lightGray},
        { "window.default.lineedit.fg", Colors::black},
        { "window.default.lineedit.focused.bg", Colors::lightGray},
        { "window.default.lineedit.focused.fg", Colors::brightWhite},

        { "window.gray.bg", Colors::lightGray},
        { "window.gray.frame.focused.fg", Colors::brightWhite},
        { "window.gray.frame.focused.control.fg", Colors::brightGreen},
        { "window.gray.frame.unfocused.fg", Colors::black},
        { "window.gray.scrollbar.fg", Colors::cyan},
        { "window.gray.scrollbar.bg", Colors::blue},
        { "window.gray.scrollbar.control.fg", Colors::cyan},
        { "window.gray.scrollbar.control.bg", Colors::blue},
        { "window.gray.text.fg", Colors::lightGray},
        { "window.gray.text.bg", Colors::black},
        { "window.gray.text.selected.fg", Colors::lightGray},
        { "window.gray.text.selected.bg", Colors::brightWhite},
        { "window.gray.control.bg", Colors::lightGray},
        { "window.gray.control.fg", Colors::black},
        { "window.gray.control.focused.bg", Colors::lightGray},
        { "window.gray.control.focused.fg", Colors::brightWhite},
        { "window.gray.control.disabled.bg", Colors::lightGray},
        { "window.gray.control.disabled.fg", Colors::darkGray},
        { "window.gray.control.shortcut.bg", Colors::lightGray},
        { "window.gray.control.shortcut.fg", Colors::brightYellow},
        { "window.gray.dataview.bg", Colors::cyan},
        { "window.gray.dataview.fg", Colors::black},
        { "window.gray.dataview.selected.bg", Colors::cyan},
        { "window.gray.dataview.selected.fg", Colors::brightYellow},
        { "window.gray.dataview.selected.focused.bg", Colors::green},
        { "window.gray.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.gray.dataview.disabled.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.fg", Colors::lightGray},
        { "window.gray.dataview.disabled.selected.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.gray.button.bg", Colors::green},
        { "window.gray.button.fg", Colors::black},
        { "window.gray.button.default.bg", Colors::green},
        { "window.gray.button.default.fg", Colors::brightCyan},
        { "window.gray.button.focused.bg", Colors::green},
        { "window.gray.button.focused.fg", Colors::brightWhite},
        { "window.gray.button.disabled.bg", Colors::lightGray},
        { "window.gray.button.disabled.fg", Colors::darkGray},
        { "window.gray.button.shortcut.bg", Colors::green},
        { "window.gray.button.shortcut.fg", Colors::brightYellow},
        { "window.gray.lineedit.bg", Colors::blue},
        { "window.gray.lineedit.fg", Colors::brightWhite},
        { "window.gray.lineedit.focused.bg", Colors::green},
        { "window.gray.lineedit.focused.fg", Colors::brightWhite},

        { "window.cyan.bg", Colors::cyan},
        { "window.cyan.frame.focused.fg", Colors::brightWhite},
        { "window.cyan.frame.focused.control.fg", Colors::brightGreen},
        { "window.cyan.frame.unfocused.fg", Colors::lightGray},
        { "window.cyan.scrollbar.fg", Colors::cyan},
        { "window.cyan.scrollbar.bg", Colors::blue},
        { "window.cyan.scrollbar.control.fg", Colors::cyan},
        { "window.cyan.scrollbar.control.bg", Colors::blue},
        { "window.cyan.text.fg", Colors::black},
        { "window.cyan.text.bg", Colors::cyan},
        { "window.cyan.text.selected.fg", Colors::brightWhite},
        { "window.cyan.text.selected.bg", Colors::cyan},
        { "window.cyan.control.bg", Colors::cyan},
        { "window.cyan.control.fg", Colors::black},
        { "window.cyan.control.focused.bg", Colors::cyan},
        { "window.cyan.control.focused.fg", Colors::brightWhite},
        { "window.cyan.control.disabled.bg", Colors::cyan},
        { "window.cyan.control.disabled.fg", Colors::darkGray},
        { "window.cyan.control.shortcut.bg", Colors::cyan},
        { "window.cyan.control.shortcut.fg", Colors::brightYellow},
        { "window.cyan.dataview.bg", Colors::brightBlue},
        { "window.cyan.dataview.fg", Colors::black},
        { "window.cyan.dataview.selected.bg", Colors::brightBlue},
        { "window.cyan.dataview.selected.fg", Colors::brightYellow},
        { "window.cyan.dataview.selected.focused.bg", Colors::green},
        { "window.cyan.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.cyan.dataview.disabled.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.fg", Colors::black},
        { "window.cyan.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.cyan.button.bg", Colors::green},
        { "window.cyan.button.fg", Colors::black},
        { "window.cyan.button.default.bg", Colors::green},
        { "window.cyan.button.default.fg", Colors::brightCyan},
        { "window.cyan.button.focused.bg", Colors::green},
        { "window.cyan.button.focused.fg", Colors::brightWhite},
        { "window.cyan.button.disabled.bg", Colors::lightGray},
        { "window.cyan.button.disabled.fg", Colors::darkGray},
        { "window.cyan.button.shortcut.bg", Colors::green},
        { "window.cyan.button.shortcut.fg", Colors::brightYellow},
        { "window.cyan.lineedit.bg", Colors::blue},
        { "window.cyan.lineedit.fg", Colors::brightWhite},
        { "window.cyan.lineedit.focused.bg", Colors::green},
        { "window.cyan.lineedit.focused.fg", Colors::brightWhite},

    });

    setDefaultRules(p);
    return p;
}

ZPalette ZPalette::black() {
    ZPalette p;
    p.setColors({
        { "root.bg", Colors::black},
        { "root.fg", {0x80, 0x80, 0x80}},

        { "menu.bg", Colors::lightGray},
        { "menu.fg", Colors::black},
        { "menu.disabled.bg", Colors::lightGray},
        { "menu.disabled.fg", Colors::darkGray},
        { "menu.shortcut.bg", Colors::lightGray},
        { "menu.shortcut.fg", Colors::red},
        { "menu.selected.bg", Colors::green},
        { "menu.selected.fg", Colors::black},
        { "menu.selected.disabled.bg", Colors::green},
        { "menu.selected.disabled.fg", Colors::darkGray},
        { "menu.selected.shortcut.bg", Colors::green},
        { "menu.selected.shortcut.fg", Colors::red},

        { "window.default.bg", Colors::black},
        { "window.default.frame.focused.fg", Colors::brightWhite},
        { "window.default.frame.focused.control.fg", Colors::brightGreen},
        { "window.default.frame.unfocused.fg", Colors::lightGray},
        { "window.default.scrollbar.fg", Colors::black},
        { "window.default.scrollbar.bg", { 0, 0x55, 0xaa}},
        { "window.default.scrollbar.control.fg", Colors::brightWhite},
        { "window.default.scrollbar.control.bg", Colors::darkGray},
        { "window.default.text.fg", Colors::brightYellow},
        { "window.default.text.bg", Colors::black},
        { "window.default.text.selected.fg", Colors::black},
        { "window.default.text.selected.bg", Colors::lightGray},
        { "window.default.control.bg", Colors::black},
        { "window.default.control.fg", Colors::lightGray},
        { "window.default.control.focused.bg", Colors::black},
        { "window.default.control.focused.fg", Colors::brightWhite},
        { "window.default.control.disabled.bg", Colors::black},
        { "window.default.control.disabled.fg", Colors::darkGray},
        { "window.default.control.shortcut.bg", Colors::black},
        { "window.default.control.shortcut.fg", Colors::brightYellow},
        { "window.default.dataview.bg", Colors::brightWhite},
        { "window.default.dataview.fg", Colors::darkGray},
        { "window.default.dataview.selected.bg", Colors::brightWhite},
        { "window.default.dataview.selected.fg", Colors::black},
        { "window.default.dataview.selected.focused.bg", Colors::brightGreen},
        { "window.default.dataview.selected.focused.fg", Colors::black},
        { "window.default.dataview.disabled.bg", Colors::lightGray},
        { "window.default.dataview.disabled.fg", Colors::darkGray},
        { "window.default.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.default.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.default.button.bg", Colors::green},
        { "window.default.button.fg", Colors::black},
        { "window.default.button.default.bg", Colors::green},
        { "window.default.button.default.fg", Colors::brightCyan},
        { "window.default.button.focused.bg", Colors::green},
        { "window.default.button.focused.fg", Colors::brightWhite},
        { "window.default.button.disabled.bg", Colors::lightGray},
        { "window.default.button.disabled.fg", Colors::darkGray},
        { "window.default.button.shortcut.bg", Colors::green},
        { "window.default.button.shortcut.fg", Colors::brightYellow},
        { "window.default.lineedit.bg", Colors::lightGray},
        { "window.default.lineedit.fg", Colors::black},
        { "window.default.lineedit.focused.bg", Colors::lightGray},
        { "window.default.lineedit.focused.fg", Colors::brightWhite},

        { "window.gray.bg", Colors::lightGray},
        { "window.gray.frame.focused.fg", Colors::brightWhite},
        { "window.gray.frame.focused.control.fg", Colors::brightGreen},
        { "window.gray.frame.unfocused.fg", Colors::black},
        { "window.gray.scrollbar.fg", Colors::cyan},
        { "window.gray.scrollbar.bg", Colors::black},
        { "window.gray.scrollbar.control.fg", Colors::cyan},
        { "window.gray.scrollbar.control.bg", Colors::black},
        { "window.gray.text.fg", Colors::lightGray},
        { "window.gray.text.bg", Colors::black},
        { "window.gray.text.selected.fg", Colors::lightGray},
        { "window.gray.text.selected.bg", Colors::brightWhite},
        { "window.gray.control.bg", Colors::lightGray},
        { "window.gray.control.fg", Colors::black},
        { "window.gray.control.focused.bg", Colors::lightGray},
        { "window.gray.control.focused.fg", Colors::brightWhite},
        { "window.gray.control.disabled.bg", Colors::lightGray},
        { "window.gray.control.disabled.fg", Colors::darkGray},
        { "window.gray.control.shortcut.bg", Colors::lightGray},
        { "window.gray.control.shortcut.fg", Colors::brightYellow},
        { "window.gray.dataview.bg", Colors::brightWhite},
        { "window.gray.dataview.fg", Colors::darkGray},
        { "window.gray.dataview.selected.bg", Colors::brightWhite},
        { "window.gray.dataview.selected.fg", Colors::black},
        { "window.gray.dataview.selected.focused.bg", Colors::brightGreen},
        { "window.gray.dataview.selected.focused.fg", Colors::black},
        { "window.gray.dataview.disabled.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.fg", Colors::lightGray},
        { "window.gray.dataview.disabled.selected.bg", Colors::darkGray},
        { "window.gray.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.gray.button.bg", Colors::green},
        { "window.gray.button.fg", Colors::black},
        { "window.gray.button.default.bg", Colors::green},
        { "window.gray.button.default.fg", Colors::brightCyan},
        { "window.gray.button.focused.bg", Colors::green},
        { "window.gray.button.focused.fg", Colors::brightWhite},
        { "window.gray.button.disabled.bg", Colors::lightGray},
        { "window.gray.button.disabled.fg", Colors::darkGray},
        { "window.gray.button.shortcut.bg", Colors::green},
        { "window.gray.button.shortcut.fg", Colors::brightYellow},
        { "window.gray.lineedit.bg", Colors::black},
        { "window.gray.lineedit.fg", Colors::brightWhite},
        { "window.gray.lineedit.focused.bg", Colors::green},
        { "window.gray.lineedit.focused.fg", Colors::brightWhite},

        { "window.cyan.bg", Colors::cyan},
        { "window.cyan.frame.focused.fg", Colors::brightWhite},
        { "window.cyan.frame.focused.control.fg", Colors::brightGreen},
        { "window.cyan.frame.unfocused.fg", Colors::lightGray},
        { "window.cyan.scrollbar.fg", Colors::cyan},
        { "window.cyan.scrollbar.bg", Colors::black},
        { "window.cyan.scrollbar.control.fg", Colors::cyan},
        { "window.cyan.scrollbar.control.bg", Colors::black},
        { "window.cyan.text.fg", Colors::black},
        { "window.cyan.text.bg", Colors::cyan},
        { "window.cyan.text.selected.fg", Colors::brightWhite},
        { "window.cyan.text.selected.bg", Colors::cyan},
        { "window.cyan.control.bg", Colors::cyan},
        { "window.cyan.control.fg", Colors::black},
        { "window.cyan.control.focused.bg", Colors::cyan},
        { "window.cyan.control.focused.fg", Colors::brightWhite},
        { "window.cyan.control.disabled.bg", Colors::cyan},
        { "window.cyan.control.disabled.fg", Colors::darkGray},
        { "window.cyan.control.shortcut.bg", Colors::cyan},
        { "window.cyan.control.shortcut.fg", Colors::brightYellow},
        { "window.cyan.dataview.bg", Colors::blue},
        { "window.cyan.dataview.fg", Colors::black},
        { "window.cyan.dataview.selected.bg", Colors::blue},
        { "window.cyan.dataview.selected.fg", Colors::brightYellow},
        { "window.cyan.dataview.selected.focused.bg", Colors::green},
        { "window.cyan.dataview.selected.focused.fg", Colors::brightWhite},
        { "window.cyan.dataview.disabled.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.fg", Colors::black},
        { "window.cyan.dataview.disabled.selected.bg", Colors::lightGray},
        { "window.cyan.dataview.disabled.selected.fg", Colors::brightWhite},
        { "window.cyan.button.bg", Colors::green},
        { "window.cyan.button.fg", Colors::black},
        { "window.cyan.button.default.bg", Colors::green},
        { "window.cyan.button.default.fg", Colors::brightCyan},
        { "window.cyan.button.focused.bg", Colors::green},
        { "window.cyan.button.focused.fg", Colors::brightWhite},
        { "window.cyan.button.disabled.bg", Colors::lightGray},
        { "window.cyan.button.disabled.fg", Colors::darkGray},
        { "window.cyan.button.shortcut.bg", Colors::green},
        { "window.cyan.button.shortcut.fg", Colors::brightYellow},
        { "window.cyan.lineedit.bg", Colors::black},
        { "window.cyan.lineedit.fg", Colors::brightWhite},
        { "window.cyan.lineedit.focused.bg", Colors::green},
        { "window.cyan.lineedit.focused.fg", Colors::brightWhite},

    });

    setDefaultRules(p);
    return p;
}

TUIWIDGETS_NS_END
