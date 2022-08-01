#ifndef TUIWIDGETS_ZMENUITEM_INCLUDED
#define TUIWIDGETS_ZMENUITEM_INCLUDED

#include <functional>

#include <Tui/ZValuePtr.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZMenuItemPrivate;

class TUIWIDGETS_EXPORT ZMenuItem {
public:
    ZMenuItem();
    ZMenuItem(const QString &markup, const QString &fakeShortcut, ZImplicitSymbol command, QVector<ZMenuItem> subitems);
    ZMenuItem(const QString &markup, QObject *context, std::function<QVector<ZMenuItem>()> subitemsGenerator);
    ZMenuItem(const ZMenuItem&);
    ZMenuItem(ZMenuItem&&);
    ~ZMenuItem();

    ZMenuItem &operator=(const ZMenuItem&);
    ZMenuItem &operator=(ZMenuItem&&);

public:
    const QString &markup() const;
    void setMarkup(const QString &markup);
    const QString &fakeShortcut() const;
    void setFakeShortcut(const QString &fakeShortcut);
    const ZImplicitSymbol &command() const;
    void setCommand(const ZImplicitSymbol &command);
    const QVector<ZMenuItem> subitems() const;
    void setSubitems(const QVector<ZMenuItem> &subitems);
    void setSubitemsGenerator(QObject *context, std::function<QVector<ZMenuItem>()> subitemsGenerator);
    bool hasSubitems() const;

protected:
    ZValuePtr<ZMenuItemPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZMenuItem)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZMENUITEM_INCLUDED
