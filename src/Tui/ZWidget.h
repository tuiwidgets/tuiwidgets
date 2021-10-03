#ifndef TUIWIDGETS_ZWIDGET_INCLUDED
#define TUIWIDGETS_ZWIDGET_INCLUDED

#include <memory>

#include <QObject>
#include <QMargins>

#include <Tui/ZCommon.h>
#include <Tui/ZEvent.h>
#include <Tui/ZMoFunc_p.h>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

class ZCommandManager;
class ZColor;
class ZImplicitSymbol;
class ZLayout;
class ZPalette;
class ZTerminal;

enum class FocusContainerMode {
    None,
    SubOrdering,
    Cycle
};

enum class SizePolicy {
    Fixed,
    Minimum,
    Maximum,
    Preferred,
    Expanding,
};

constexpr int tuiMaxSize = 0xffffff;

class ZWidgetPrivate;

class TUIWIDGETS_EXPORT ZWidget : public QObject {
    Q_OBJECT
public:
    explicit ZWidget(ZWidget *parent = 0);
    ~ZWidget();

protected:
    explicit ZWidget(QObject *parent, std::unique_ptr<ZWidgetPrivate> pimpl);

protected:
    std::unique_ptr<ZWidgetPrivate> tuiwidgets_pimpl_ptr;

public:
    ZWidget *parentWidget() const { return static_cast<ZWidget*>(parent()); }
    void setParent(ZWidget *newParent);
    QRect geometry() const;
    void setGeometry(const QRect &geometry);
    QRect rect() const;
    QRect contentsRect() const;
    bool isEnabled() const; // includes enabled state of parents
    bool isLocallyEnabled() const;
    void setEnabled(bool e);
    bool isVisible() const; // includes visible state of parents
    bool isLocallyVisible() const;
    void setVisible(bool v);
    void raise();
    void stackUnder(ZWidget *w);

    QSize minimumSize() const;
    void setMinimumSize(int w, int h);
    QSize maximumSize() const;
    void setMaximumSize(int w, int h);
    void setFixedSize(int w, int h);
    SizePolicy sizePolicyH() const;
    void setSizePolicyH(SizePolicy policy);
    SizePolicy sizePolicyV() const;
    void setSizePolicyV(SizePolicy policy);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    QSize effectiveSizeHint() const;
    QSize effectiveMinimumSize() const;
    virtual QRect layoutArea() const;
    ZLayout *layout() const;
    void setLayout(ZLayout *l);

    void showCursor(QPoint position);
    ZTerminal *terminal() const;

    void update();
    void updateGeometry();

    void setFocusPolicy(Qt::FocusPolicy policy);
    Qt::FocusPolicy focusPolicy() const;
    void setFocusMode(FocusContainerMode mode);
    FocusContainerMode focusMode() const;
    void setFocusOrder(int order);
    int focusOrder() const;

    QMargins contentsMargins() const;
    void setContentsMargins(QMargins m);

    const ZPalette &palette() const;
    void setPalette(const ZPalette &pal);
    ZColor getColor(const ZImplicitSymbol &x);

    QStringList paletteClass() const;
    void setPaletteClass(QStringList classes);
    void addPaletteClass(QString clazz);
    void removePaletteClass(QString clazz);

    CursorStyle cursorStyle() const;
    void setCursorStyle(CursorStyle style);
    void resetCursorColor();
    void setCursorColor(int r, int b, int g);

    void setFocus(Qt::FocusReason reason = Qt::OtherFocusReason);
    void grabKeyboard();
    void grabKeyboard(Private::ZMoFunc<void(QEvent*)> handler);
    void releaseKeyboard();

    bool isAncestorOf(const ZWidget *child) const;
    bool isEnabledTo(const ZWidget *ancestor) const;
    bool isVisibleTo(const ZWidget *ancestor) const;
    bool focus() const;
    bool isInFocusPath() const;

    QPoint mapFromTerminal(const QPoint& pos);
    QPoint mapToTerminal(const QPoint& pos);

    ZWidget const *prevFocusable() const;
    ZWidget *prevFocusable();
    ZWidget const *nextFocusable() const;
    ZWidget *nextFocusable();
    const ZWidget *placeFocus(bool last = false) const;
    ZWidget *placeFocus(bool last = false);

    virtual ZWidget *resolveSizeHintChain();

    ZCommandManager *commandManager();
    ZCommandManager *ensureCommandManager();
    void setCommandManager(ZCommandManager *cmd);

    template<typename T>
    T *findFacet() {
        ZWidget *w = this;
        while (w) {
            T *t = static_cast<T*>(w->facet(T::staticMetaObject));
            if (t) {
                return t;
            }
            w = w->parentWidget();
        }
        return nullptr;
    }

    virtual QObject *facet(const QMetaObject metaObject);

    // public virtuals from base class override everything for later ABI compatibility
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual void paintEvent(ZPaintEvent *event);
    virtual void keyEvent(ZKeyEvent *event);
    virtual void pasteEvent(ZPasteEvent *event);
    virtual void focusInEvent(ZFocusEvent *event);
    virtual void focusOutEvent(ZFocusEvent *event);
    virtual void resizeEvent(ZResizeEvent *event);
    virtual void moveEvent(ZMoveEvent *event);
    // protected virtuals from base class override everything for later ABI compatibility
    void timerEvent(QTimerEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void customEvent(QEvent *event) override;
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

private:
    Q_DISABLE_COPY(ZWidget)
    TUIWIDGETS_DECLARE_PRIVATE(ZWidget)
};

TUIWIDGETS_NS_END
#endif // TUIWIDGETS_ZWIDGET_INCLUDED
