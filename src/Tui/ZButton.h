#ifndef TUIWIDGETS_ZBUTTON_INCLUDED
#define TUIWIDGETS_ZBUTTON_INCLUDED

#include <Tui/ZShortcut.h>
#include <Tui/ZWidget.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZButtonPrivate;

class TUIWIDGETS_EXPORT ZButton : public ZWidget {
    Q_OBJECT
public:
    explicit ZButton(Tui::ZWidget *parent=nullptr);
    explicit ZButton(const QString &text, Tui::ZWidget *parent=nullptr);
    explicit ZButton(Tui::WithMarkupTag, const QString &markup, Tui::ZWidget *parent=nullptr);
    ~ZButton();

public:
    QString text() const;
    void setText(const QString &t);

    QString markup() const;
    void setMarkup(QString m);

    void setShortcut(const Tui::ZKeySequence &key);

    void setDefault(bool d);
    bool isDefault();

    bool event(QEvent *event) override;
    QSize sizeHint() const override;

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void click();

protected:
    void paintEvent(Tui::ZPaintEvent *event) override;
    void keyEvent(Tui::ZKeyEvent *event) override;

private:
    void removeShortcut();

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZButton)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZBUTTON_INCLUDED