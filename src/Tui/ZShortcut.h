#ifndef TUIWIDGETS_ZSHORTCUT_INCLUDED
#define TUIWIDGETS_ZSHORTCUT_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;
class ZKeyEvent;

class TUIWIDGETS_EXPORT ZKeySequence {
public:
    // FIXME Massive changes needed
    ZKeySequence();
    static ZKeySequence forMnemonic(const QString &c);
    static ZKeySequence forKey(int key);

private:
    // FIXME pimpl
    friend class ZShortcut;
    QString _forMnemonic;
    int _forKey = 0;
};

class ZShortcutPrivate;
class TUIWIDGETS_EXPORT ZShortcut : public QObject {
    Q_OBJECT
public:
    //explicit ZShortcut(ZWidget *parent = 0);
    explicit ZShortcut(const ZKeySequence &key, ZWidget *parent, Qt::ShortcutContext context = Qt::WindowShortcut);
    virtual ~ZShortcut() override;

public:
    bool isEnabled() const;
    void setEnabled(bool enable);

    bool matches(ZWidget *focusWidget, const ZKeyEvent *event);

Q_SIGNALS:
    void activated();

public:
    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
protected:
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

protected:
    std::unique_ptr<ZShortcutPrivate> tuiwidgets_pimpl_ptr;

private:
    Q_DISABLE_COPY(ZShortcut)
    TUIWIDGETS_DECLARE_PRIVATE(ZShortcut)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZSHORTCUT_INCLUDED
