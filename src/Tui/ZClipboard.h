// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZCLIPBOARD_INCLUDED
#define TUIWIDGETS_ZCLIPBOARD_INCLUDED

#include <memory>

#include <QObject>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZClipboardPrivate;
class TUIWIDGETS_EXPORT ZClipboard : public QObject {
    Q_OBJECT
public:
    ZClipboard();
    ~ZClipboard() override;

public:
    virtual void clear();
    virtual QString contents() const;
    virtual void setContents(const QString &text);

Q_SIGNALS:
    void contentsChanged();

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
    ZClipboard(std::unique_ptr<ZClipboardPrivate> pimpl);

    std::unique_ptr<ZClipboardPrivate> tuiwidgets_pimpl_ptr;

private:
    TUIWIDGETS_DECLARE_PRIVATE(ZClipboard)
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZCLIPBOARD_INCLUDED
