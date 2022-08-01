#include <Tui/ZMenuItem.h>
#include <Tui/ZMenuItem_p.h>

TUIWIDGETS_NS_START

ZMenuItem::ZMenuItem() = default;
ZMenuItem::ZMenuItem(const ZMenuItem&) = default;
ZMenuItem::ZMenuItem(ZMenuItem&&) = default;

ZMenuItem &ZMenuItem::operator=(const ZMenuItem&) = default;
ZMenuItem &ZMenuItem::operator=(ZMenuItem&&) = default;

ZMenuItem::ZMenuItem(const QString &markup, const QString &fakeShortcut, ZImplicitSymbol command, QVector<ZMenuItem> subitems) {
    auto *const p = tuiwidgets_impl();

    p->markup = markup;
    p->fakeShortcut = fakeShortcut;
    p->command = command;
    p->subitems = subitems;
}

ZMenuItem::ZMenuItem(const QString &markup, QObject *context, std::function<QVector<ZMenuItem>()> subitemsGenerator) {
    auto *const p = tuiwidgets_impl();

    p->markup = markup;
    p->subitemsGenerator = subitemsGenerator;
    p->subitemsGeneratorResetConnection = {};

    if (context) {
        p->subitemsGeneratorResetConnection = QObject::connect(context, &QObject::destroyed, [this] {
            auto *const p = tuiwidgets_impl();
            p->subitemsGenerator = {};
        });
    }
}

ZMenuItem::~ZMenuItem() {
    auto *const p = tuiwidgets_impl();
    QObject::disconnect(p->subitemsGeneratorResetConnection);
}

const QVector<ZMenuItem> ZMenuItem::subitems() const {
    auto *const p = tuiwidgets_impl();
    if (p->subitemsGenerator) {
        return p->subitemsGenerator();
    }
    return p->subitems;
}

void ZMenuItem::setSubitems(const QVector<ZMenuItem> &subitems) {
    auto *const p = tuiwidgets_impl();
    p->subitems = subitems;
    QObject::disconnect(p->subitemsGeneratorResetConnection);
    p->subitemsGenerator = {};
}

void ZMenuItem::setSubitemsGenerator(QObject *context, std::function<QVector<ZMenuItem>()> subitemsGenerator) {
    auto *const p = tuiwidgets_impl();
    p->subitems.clear();
    p->subitemsGenerator = subitemsGenerator;
    QObject::disconnect(p->subitemsGeneratorResetConnection);
    if (context) {
        p->subitemsGeneratorResetConnection = QObject::connect(context, &QObject::destroyed, [this] {
            auto *const p = tuiwidgets_impl();
            p->subitemsGenerator = {};
        });
    }
}

bool ZMenuItem::hasSubitems() const {
    auto *const p = tuiwidgets_impl();
    return p->subitems.size() || p->subitemsGenerator;
}

const ZImplicitSymbol &ZMenuItem::command() const {
    auto *const p = tuiwidgets_impl();
    return p->command;
}

void ZMenuItem::setCommand(const ZImplicitSymbol &command) {
    auto *const p = tuiwidgets_impl();
    p->command = command;
}

const QString &ZMenuItem::fakeShortcut() const {
    auto *const p = tuiwidgets_impl();
    return p->fakeShortcut;
}

void ZMenuItem::setFakeShortcut(const QString &fakeShortcut) {
    auto *const p = tuiwidgets_impl();
    p->fakeShortcut = fakeShortcut;
}

const QString &ZMenuItem::markup() const {
    auto *const p = tuiwidgets_impl();
    return p->markup;
}

void ZMenuItem::setMarkup(const QString &markup) {
    auto *const p = tuiwidgets_impl();
    p->markup = markup;
}


TUIWIDGETS_NS_END
