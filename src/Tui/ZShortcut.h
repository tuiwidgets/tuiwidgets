#ifndef TUIWIDGETS_ZSHORTCUT_INCLUDED
#define TUIWIDGETS_ZSHORTCUT_INCLUDED

#include <functional>
#include <memory>

#include <QObject>

#include <Tui/ZMoFunc_p.h>
#include <Tui/ZValuePtr.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZWidget;
class ZKeyEvent;

class ZPendingKeySequenceCallbacksPrivate;
class TUIWIDGETS_EXPORT ZPendingKeySequenceCallbacks {
public:
    ZPendingKeySequenceCallbacks();
    ~ZPendingKeySequenceCallbacks();

public:
    void setPendingSequenceStarted(std::function<void()> callback);
    void setPendingSequenceFinished(std::function<void(bool matched)> callback);
    void setPendingSequenceUpdated(std::function<void()> callback);

protected:
    ZValuePtr<ZPendingKeySequenceCallbacksPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZPendingKeySequenceCallbacks)
};

class ZKeySequencePrivate;
class TUIWIDGETS_EXPORT ZKeySequence {
public:
    ZKeySequence();
    ~ZKeySequence();

public:
    // FIXME Massive changes needed
    static ZKeySequence forMnemonic(const QString &c);
    static ZKeySequence forKey(int key, Qt::KeyboardModifiers modifiers = {});
    static ZKeySequence forShortcut(const QString &c, Qt::KeyboardModifiers modifiers = Qt::ControlModifier);
    static ZKeySequence forShortcutSequence(const QString &c, Qt::KeyboardModifiers modifiers,
                                            const QString &c2, Qt::KeyboardModifiers modifiers2);
    static ZKeySequence forShortcutSequence(const QString &c, Qt::KeyboardModifiers modifiers,
                                            int key2, Qt::KeyboardModifiers modifiers2);

protected:
    ZValuePtr<ZKeySequencePrivate> tuiwidgets_pimpl_ptr;

private:
    friend class ZShortcut;
    TUIWIDGETS_DECLARE_PRIVATE(ZKeySequence)
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

    void setEnabledDelegate(Private::ZMoFunc<bool()>&& delegate);

    bool matches(ZWidget *focusWidget, const ZKeyEvent *event) const;

Q_SIGNALS:
    void activated();

public:
    bool event(QEvent *event) override;
    // public virtuals from base class override everything for later ABI compatibility
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
