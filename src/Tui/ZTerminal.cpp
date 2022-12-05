// SPDX-License-Identifier: BSL-1.0

#include <Tui/ZTerminal.h>
#include <Tui/ZTerminal_p.h>

#include <limits>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QVector>

#include <Tui/ZEvent.h>
#include <Tui/ZPainter_p.h>
#include <Tui/ZShortcutManager_p.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZTextMetrics_p.h>
#include <Tui/ZImage_p.h>
#include <Tui/ZWidget.h>
#include <Tui/ZWidget_p.h>

TUIWIDGETS_NS_START

class ZTerminal::TerminalConnectionPrivate {
public:
    static TerminalConnectionPrivate *get(ZTerminal::TerminalConnection *data) { return data->tuiwidgets_pimpl_ptr.get(); }

    bool backspaceIsX08 = false;
    int width = 0;
    int height = 0;

    ZTerminal::TerminalConnectionDelegate *delegate;
    ZTerminalPrivate *terminal;
};

static ZSymbol extendedCharset = TUISYM_LITERAL("extendedCharset");

ZTerminalPrivate::ZTerminalPrivate(ZTerminal *pub, ZTerminal::Options options)
    : options(options)
{
    pub_ptr = pub;
    QObject::connect(QThread::currentThread()->eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock,
                     pub, &ZTerminal::dispatcherIsAboutToBlock);
}

ZTerminalPrivate::~ZTerminalPrivate() {
    deinitTerminal();
}

ZTerminalPrivate *ZTerminalPrivate::get(ZTerminal *terminal) {
    return terminal->tuiwidgets_impl();
}

const ZTerminalPrivate *ZTerminalPrivate::get(const ZTerminal *terminal) {
    return terminal->tuiwidgets_impl();
}

bool ZTerminalPrivate::mainWidgetFullyAttached() {
    return mainWidget.data() && ZWidgetPrivate::get(mainWidget.data())->terminal == this->pub();
}

void ZTerminalPrivate::setFocus(ZWidget *w) {
    if (!w) {
        focusWidget = nullptr;
    } else {
        focusWidget = ZWidgetPrivate::get(w);
        focusHistory.appendOrMoveToLast(focusWidget);
    }
    Q_EMIT pub()->focusChanged();
}

ZWidget *ZTerminalPrivate::focus() {
    return focusWidget ? focusWidget->pub() : nullptr;
}

void ZTerminalPrivate::setKeyboardGrab(ZWidget *w) {
    keyboardGrabWidget = w;
    keyboardGrabHandler = {};
}

void ZTerminalPrivate::setKeyboardGrab(ZWidget *w, Private::ZMoFunc<void(QEvent*)> handler) {
    keyboardGrabWidget = w;
    keyboardGrabHandler = std::move(handler);
}

ZWidget *ZTerminalPrivate::keyboardGrab() const {
    if (!keyboardGrabWidget && keyboardGrabHandler) {
        keyboardGrabHandler = {};
    }
    return keyboardGrabWidget;
}

ZShortcutManager *ZTerminalPrivate::ensureShortcutManager() {
    if (!shortcutManager) {
        shortcutManager = std::make_unique<ZShortcutManager>(pub());
    }
    return shortcutManager.get();
}

void ZTerminal::setCursorStyle(CursorStyle style) {
    auto *const p = tuiwidgets_impl();
    p->terminalCursorStyle = style;
    switch (style) {
        case CursorStyle::Unset:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_TERM_DEFAULT, true);
            break;
        case CursorStyle::Bar:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_BAR, true);
            break;
        case CursorStyle::Block:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_BLOCK, true);
            break;
        case CursorStyle::Underline:
            termpaint_terminal_set_cursor_style(p->terminal, TERMPAINT_CURSOR_STYLE_UNDERLINE, true);
            break;
    }
}

void ZTerminal::setCursorPosition(QPoint cursorPosition) {
    auto *const p = tuiwidgets_impl();
    termpaint_terminal_set_cursor_position(p->terminal, cursorPosition.x(), cursorPosition.y());
    p->terminalCursorPosition = cursorPosition;
    const bool cursorVisible = cursorPosition != QPoint{-1, -1};
    termpaint_terminal_set_cursor_visible(p->terminal, cursorVisible);
    p->terminalCursorVisible  = cursorVisible;
}

void ZTerminal::setCursorColor(int cursorColorR, int cursorColorG, int cursorColorB) {
    auto *const p = tuiwidgets_impl();
    if (cursorColorR != -1) {
        termpaint_terminal_set_color(p->terminal, TERMPAINT_COLOR_SLOT_CURSOR,
                                     cursorColorR,
                                     cursorColorG,
                                     cursorColorB);
        p->terminalCursorR = cursorColorR;
        p->terminalCursorG = cursorColorG;
        p->terminalCursorB = cursorColorB;
    } else {
        termpaint_terminal_reset_color(p->terminal, TERMPAINT_COLOR_SLOT_CURSOR);
        p->terminalCursorR = -1;
        p->terminalCursorG = -1;
        p->terminalCursorB = -1;
    }
}

void ZTerminal::registerPendingKeySequenceCallbacks(const ZPendingKeySequenceCallbacks &callbacks) {
    auto *const p = tuiwidgets_impl();
    p->ensureShortcutManager()->registerPendingKeySequenceCallbacks(callbacks);
}

void ZTerminal::maybeRequestLayout(ZWidget *w) {
    // TODO ignore calls when the neighborhood of w does not do automatic layout
    requestLayout(w);
}

void ZTerminal::requestLayout(ZWidget *w) {
    auto *const p = tuiwidgets_impl();

    if (p->testingLayoutRequestTrackingClosure) {
        p->testingLayoutRequestTrackingClosure(w);
    }

    if (p->layoutGeneration > 0) {
        // doLayout is running, so do synchronous layout
        QEvent request(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(w, &request);
    } else {
        QPointer<ZWidget> wp = w;
        if (!p->layoutPendingWidgets.contains(wp)) {
            p->layoutPendingWidgets.append(std::move(wp));
            if (!p->layoutRequested) {
                QCoreApplication::postEvent(this, new QEvent(QEvent::LayoutRequest), Qt::EventPriority::HighEventPriority);
                p->layoutRequested = true;
            }
        }
    }
}

bool ZTerminal::isLayoutPending() const {
    auto *const p = tuiwidgets_impl();
    return p->layoutPendingWidgets.size() > 0;
}

namespace {
    struct LayoutGenerationUpdaterScope {
        LayoutGenerationUpdaterScope(int &generation) : _generation(generation) {
            if (_generation < 0) {
                // unnested layout
                _generation = -_generation + 1;
                wrapGenerationIfNeeded();
            } else {
                // nested layout
                qWarning("ZTerminal::doLayout: Called nested, this should not happen");
                nested = true;

                wrapGenerationIfNeeded();
                _generation += 1;
            }
        }

        ~LayoutGenerationUpdaterScope() {
            if (!nested) {
                _generation = -(_generation + 1);
            } else {
                _generation += 1;
            }
        }

    private:
        void wrapGenerationIfNeeded() {
            if (_generation > std::numeric_limits<decltype(ZTerminalPrivate::layoutGeneration)>::max() - 10) {
                _generation = 2;
            }
        }

        int& _generation;
        bool nested = false;
    };
}

void ZTerminal::doLayout() {
    auto *const p = tuiwidgets_impl();
    auto copy = p->layoutPendingWidgets;
    p->layoutPendingWidgets.clear();
    LayoutGenerationUpdaterScope generationUpdater(p->layoutGeneration);
    for (auto &w: copy) {
        int depth = 0;
        ZWidget *wTmp = w;
        while (wTmp) {
            ++depth;
            wTmp = wTmp->parentWidget();
        }
        auto *wp = ZWidgetPrivate::get(w);
        wp->doLayoutScratchDepth = depth;
    }

    std::sort(copy.begin(), copy.end(), [](const ZWidget *const a, const ZWidget *const b) {
        return ZWidgetPrivate::get(a)->doLayoutScratchDepth < ZWidgetPrivate::get(b)->doLayoutScratchDepth;
    });

    for (auto &w: copy) {
        if (w.isNull()) continue;
        QEvent request(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(w.data(), &request);
    }
}

int ZTerminal::currentLayoutGeneration() {
    auto *const p = tuiwidgets_impl();
    if (p->layoutGeneration < 0) {
        // outside of doLayout, always return a new value
        if (p->layoutGeneration < std::numeric_limits<decltype(p->layoutGeneration)>::min() + 10) {
            p->layoutGeneration = -2;
        }
        p->layoutGeneration -= 1;
        return -(p->layoutGeneration);
    } else {
        // inside doLayout, the value is stable
        return p->layoutGeneration;
    }
}

bool ZTerminalPrivate::setTestLayoutRequestTracker(std::function<void (ZWidget *)> closure) {
    if (testingLayoutRequestTrackingClosure) {
        return false;
    }
    testingLayoutRequestTrackingClosure = closure;
    return true;
}

void ZTerminalPrivate::resetTestLayoutRequestTracker() {
    testingLayoutRequestTrackingClosure = {};
}

void ZTerminalPrivate::processPaintingAndUpdateOutput(bool fullRepaint) {
    if (mainWidgetFullyAttached()) {
        Q_EMIT pub()->beforeRendering();

        if (pub()->isLayoutPending()) {
            pub()->doLayout();
        }
        cursorPosition = QPoint{-1, -1};
        const QSize minSize = mainWidget->minimumSize().expandedTo(mainWidget->minimumSizeHint());
        {
            int geoWidth = std::max(minSize.width(), termpaint_surface_width(surface));
            int geoHeight = std::max(minSize.height(), termpaint_surface_height(surface));
            if (mainWidget->geometry().width() != geoWidth || mainWidget->geometry().height() != geoHeight) {
                mainWidget->setGeometry({0, 0, geoWidth, geoHeight});

                if (pub()->isLayoutPending()) {
                    pub()->doLayout();
                }
            }
        }

        std::unique_ptr<ZPainter> paint;
        std::unique_ptr<ZImage> img;
        if (minSize.width() > termpaint_surface_width(surface) || minSize.height() > termpaint_surface_height(surface)) {
            viewportActive = true;
            viewportRange.setX(std::min(0, termpaint_surface_width(surface) - minSize.width()));
            viewportRange.setY(std::min(0, termpaint_surface_height(surface) - minSize.height() - 1));
            adjustViewportOffset();
            img = std::make_unique<ZImage>(pub(), std::max(minSize.width(), termpaint_surface_width(surface)),
                                           std::max(minSize.height(), termpaint_surface_height(surface)));
            paint = std::make_unique<ZPainter>(img->painter());
        } else {
            viewportActive = false;
            viewportUI = false;
            viewportRange.setX(0);
            viewportRange.setY(0);
            viewportOffset.setX(0);
            viewportOffset.setY(0);
            paint = std::make_unique<ZPainter>(pub()->painter());
        }
        paint->setWidget(mainWidget.data());
        ZPaintEvent event(ZPaintEvent::update, paint.get());
        QCoreApplication::sendEvent(mainWidget.data(), &event);
        if (initState == ZTerminalPrivate::InitState::Ready) {
            QPoint realCursorPosition = cursorPosition + viewportOffset;
            const bool cursorVisible = !(realCursorPosition.x() < 0
                                   || realCursorPosition.y() < 0
                                   || realCursorPosition.x() >= termpaint_surface_width(surface)
                                   || realCursorPosition.y() >= termpaint_surface_height(surface));
            if (cursorVisible) {
                pub()->setCursorPosition(realCursorPosition);
                CursorStyle style = CursorStyle::Unset;
                if (focusWidget) {
                    style = focusWidget->cursorStyle;
                    pub()->setCursorColor(focusWidget->cursorColorR,
                                          focusWidget->cursorColorG,
                                          focusWidget->cursorColorB);
                }
                pub()->setCursorStyle(style);
            } else {
                pub()->setCursorPosition({-1, -1});
            }
        }

        if (viewportActive) {
            ZPainter terminalPainter = pub()->painter();
            terminalPainter.clear(ZColor::defaultColor(), ZColor::defaultColor());
            terminalPainter.drawImage(viewportOffset.x(), viewportOffset.y(), *img);
            if(viewportUI) {
                terminalPainter.writeWithColors(0, termpaint_surface_height(surface) - 1, QStringLiteral("←↑→↓ ESC"),
                                                ZColor::defaultColor(), ZColor::defaultColor());
            } else {
                terminalPainter.writeWithColors(0, termpaint_surface_height(surface) - 1, QStringLiteral("F6 Scroll"),
                                                ZColor::defaultColor(), ZColor::defaultColor());
            }
        }
        Q_EMIT pub()->afterRendering();
        if (fullRepaint) {
            pub()->updateOutputForceFullRepaint();
        } else {
            pub()->updateOutput();
        }
    }
}

void ZTerminal::dispatcherIsAboutToBlock() {
    auto *const p = tuiwidgets_impl();
    if (p->mainWidgetFullyAttached()) {
        if (!p->focusWidget || !p->focusWidget->enabled || !p->focusWidget->pub()->isVisibleTo(p->mainWidget.data())) {
            bool focusWasSet = false;
            ZWidgetPrivate *w = p->focusHistory.last;
            while (w) {
                if (w->effectivelyEnabled && w->pub()->isVisibleTo(p->mainWidget.data())) {
                    w->pub()->setFocus();
                    focusWasSet = true;
                    break;
                }
                w = w->focusHistory.prev;
            }
            if (!focusWasSet) {
                p->setFocus(p->mainWidget.data());
            }
        }
        if (p->focusWidget) {
            // ensure that attaching widgets with focus can't steal focus across message loop interations
            p->focusWidget->focusCount = p->focusCounter;
        }
    }
}

ZTerminal::ZTerminal(QObject *parent)
    : ZTerminal({}, parent)
{
}

ZTerminal::ZTerminal(Options options, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, options))
{
    auto *const p = tuiwidgets_impl();

    if (!tuiwidgets_impl()->initTerminal(options, nullptr)) {
        QByteArray utf8 = QStringLiteral("This application needs to be run in a terminal\r\n").toUtf8();
        p->showErrorWithoutTerminal(utf8);

        QPointer<ZTerminal> weak = this;
        QTimer::singleShot(0, [weak] {
            if (!weak.isNull()) {
                QCoreApplication::quit();
            }
        });
    }
}

ZTerminal::ZTerminal(ZTerminal::FileDescriptor fd, Options options, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, options))
{
    tuiwidgets_impl()->initTerminal(options, &fd);
    // TODO some kind of handling if init terminal did fail.
}

ZTerminal::ZTerminal(const ZTerminal::OffScreen &offscreen, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, Options()))
{
    tuiwidgets_impl()->initOffscreen(offscreen);
}

ZTerminal::ZTerminal(ZTerminal::TerminalConnection *connection, Options options, QObject *parent)
    : QObject(parent), tuiwidgets_pimpl_ptr(std::make_unique<ZTerminalPrivate>(this, options))
{
    tuiwidgets_impl()->initExternal(ZTerminal::TerminalConnectionPrivate::get(connection), options);
}


bool ZTerminalPrivate::initTerminal(ZTerminal::Options options, ZTerminal::FileDescriptor *fd) {
    return setupInternalConnection(options, fd);
}

void ZTerminalPrivate::initOffscreen(const ZTerminal::OffScreen &offscreen) {
    auto *const offscreenData = ZTerminal::OffScreenData::get(&offscreen);

    auto free = [] (termpaint_integration *ptr) {
        termpaint_integration_deinit(ptr);
    };
    auto write = [] (termpaint_integration *ptr, const char *data, int length) {
        Q_UNUSED(ptr);
        Q_UNUSED(data);
        Q_UNUSED(length);
    };
    auto flush = [] (termpaint_integration *ptr) {
        Q_UNUSED(ptr);
    };
    memset(&integration, 0, sizeof(integration));
    termpaint_integration_init(&integration, free, write, flush);

    callbackRequested = false;
    terminal = termpaint_terminal_new(&integration);
    if (!offscreenData->capabilities.value(extendedCharset, true)) {
        termpaint_terminal_disable_capability(terminal, TERMPAINT_CAPABILITY_EXTENDED_CHARSET);
    }
    surface = termpaint_terminal_get_surface(terminal);
    termpaint_surface_resize(surface, offscreenData->width, offscreenData->height);
    termpaint_terminal_set_event_cb(terminal, [](void *, termpaint_event *) {}, nullptr);
    initState = ZTerminalPrivate::InitState::Ready;
    // There should be no way for main widget to already be set here. So no need to call attachMainWidgetStage2.
}

void ZTerminalPrivate::initCommon() {
    termpaint_terminal_set_raw_input_filter_cb(terminal, raw_filter, pub());
    termpaint_terminal_set_event_cb(terminal, event_handler, pub());

    // start terminal detection as soon as the event loop is active.
    // Otherwise when the application has a long init time before reaching the
    // event loop the time window where unrelated events in the terminal
    // can break auto detection can be too large and the timeout can trigger
    // too soon.
    QTimer::singleShot(0, pub(), [this] {
        if (!options.testFlag(ZTerminal::DisableAutoDetectTimeoutMessage)) {
            autoDetectTimeoutTimer.reset(new QTimer(pub()));
            autoDetectTimeoutTimer->setSingleShot(true);
            autoDetectTimeoutTimer->start(10000);
            QObject::connect(autoDetectTimeoutTimer.get(), &QTimer::timeout, pub(), [this] {
                QByteArray utf8 = autoDetectTimeoutMessage.toUtf8();
                if (externalConnection) {
                    externalConnection->delegate->write(utf8.data(), utf8.size());
                    externalConnection->delegate->flush();
                } else {
                    internalConnection_integration_write(utf8.data(), utf8.size());
                    internalConnection_integration_flush();
                }
            });
        }
        termpaint_terminal_auto_detect(terminal);
    });

    callbackTimer.setSingleShot(true);
    QObject::connect(&callbackTimer, &QTimer::timeout, pub(), [terminal=terminal] {
        termpaint_terminal_callback(terminal);
    });
}

void ZTerminalPrivate::deinitTerminal() {
    if (initState == ZTerminalPrivate::InitState::Deinit) {
        // already done
        return;
    }
    if (initState == ZTerminalPrivate::InitState::Paused) {
        termpaint_terminal_free(terminal);
    } else {
        termpaint_terminal_free_with_restore(terminal);
    }
    terminal = nullptr;
    termpaint_integration_deinit(&integration);

    if (externalConnection) {
        externalConnection->delegate->deinit(awaitingResponse);
        externalConnection->terminal = nullptr;
    } else {
        deinitTerminalForInternalConnection();
    }

    initState = ZTerminalPrivate::InitState::Deinit;
}

void ZTerminalPrivate::inputFromConnection(const char *data, int length) {
    callbackTimer.stop();
    callbackRequested = false;
    awaitingResponse = false;

    termpaint_terminal_add_input_data(terminal, data, length);
    QByteArray peek = QByteArray(termpaint_terminal_peek_input_buffer(terminal), termpaint_terminal_peek_input_buffer_length(terminal));
    if (peek.length()) {
        ZRawSequenceEvent event(ZRawSequenceEvent::pending, peek);
        QCoreApplication::sendEvent(pub(), &event);
    }
    if (callbackRequested) {
        callbackTimer.start(100);
    }
}

void ZTerminalPrivate::externalWasResized() {
    if (options.testFlag(ZTerminal::DisableAutoResize)) {
        return;
    }
    pub()->resize(externalConnection->width, externalConnection->height);
}

void ZTerminal::TerminalConnection::terminalInput(const char *data, int length) {
    auto *const p = tuiwidgets_impl();
    if (p->terminal) {
        p->terminal->inputFromConnection(data, length);
    } else {
        qWarning("ZTerminal::TerminalConnection::terminalInput: No terminal associated!");
    }
}

_Bool ZTerminalPrivate::raw_filter(void *user_data, const char *data, unsigned length, _Bool overflow) {
    // TODO what to do about _overflow?
    ZTerminal *that = static_cast<ZTerminal*>(user_data);
    QByteArray rawSequence = QByteArray(data, length);
    ZRawSequenceEvent event{rawSequence};
    return QCoreApplication::sendEvent(that, &event);
}

void ZTerminalPrivate::event_handler(void *user_data, termpaint_event *event) {
    ZTerminal *that = static_cast<ZTerminal*>(user_data);
    ZTerminalNativeEvent tuiEvent{event};
    QCoreApplication::sendEvent(that, &tuiEvent);
}

void ZTerminalPrivate::integration_request_callback() {
    callbackRequested = true;
}

void ZTerminalPrivate::integration_awaiting_response() {
    awaitingResponse = true;
}

ZTerminal::~ZTerminal() {
    if (tuiwidgets_impl()->mainWidgetFullyAttached()) {
        ZWidgetPrivate::get(tuiwidgets_impl()->mainWidget.data())->unsetTerminal();
    }
}

bool ZTerminal::isDefaultTerminalAvailable() {
    return ZTerminalPrivate::terminalAvailableForInternalConnection();
}

ZPainter ZTerminal::painter() {
    auto *surface = tuiwidgets_impl()->surface;
    return ZPainter(std::make_unique<ZPainterPrivate>(surface,
                                                      termpaint_surface_width(surface),
                                                      termpaint_surface_height(surface)));
}

ZTextMetrics ZTerminal::textMetrics() const {
    auto *surface = tuiwidgets_impl()->surface;
    return ZTextMetrics(std::make_shared<ZTextMetricsPrivate>(surface));
}

ZWidget *ZTerminal::mainWidget() const {
    return tuiwidgets_impl()->mainWidget.data();
}

void ZTerminal::setMainWidget(ZWidget *w) {
    auto *const p = tuiwidgets_impl();

    if (w == p->mainWidget.data()) {
        return;
    }
    if (p->mainWidget) {
        ZWidgetPrivate::get(p->mainWidget.data())->unsetTerminal();
        // clear all state relating to widgets
        p->focusWidget = nullptr;
        p->focusHistory.clear();
        p->keyboardGrabWidget = nullptr;
        p->keyboardGrabHandler = {};
        p->layoutPendingWidgets.clear();
        LayoutGenerationUpdaterScope generationUpdater(p->layoutGeneration);
    }
    tuiwidgets_impl()->mainWidget = w;
    if (p->initState == ZTerminalPrivate::InitState::Ready || p->initState == ZTerminalPrivate::InitState::Paused) {
        p->attachMainWidgetStage2();
    }
}

void ZTerminalPrivate::attachMainWidgetStage2() {
    ZWidgetPrivate::get(mainWidget)->setManagingTerminal(pub());
    sendOtherChangeEvent(ZOtherChangeEvent::all().subtract({TUISYM_LITERAL("terminal")}));

    // TODO respect minimal widget size and add system managed scrolling if terminal is too small
    mainWidget->setGeometry({0, 0, termpaint_surface_width(surface), termpaint_surface_height(surface)});

    QPointer<ZWidget> newFocus;
    uint64_t highestFocus = 0;

    auto f = [&](QObject *w) {
        auto widget = qobject_cast<ZWidget*>(w);
        if (widget) {
            auto *const wPriv = ZWidgetPrivate::get(widget);
            if (highestFocus < wPriv->focusCount
                    && widget->isVisible() && widget->isEnabled()) {
                newFocus = widget;
                highestFocus = wPriv->focusCount;
            }
        }
    };

    f(mainWidget);
    zwidgetForEachDescendant(mainWidget, f);

    if (newFocus && (!pub()->focusWidget() || highestFocus > ZWidgetPrivate::get(pub()->focusWidget())->focusCount)) {
        newFocus->setFocus();
    }

    pub()->update();
}

void ZTerminalPrivate::sendOtherChangeEvent(QSet<ZSymbol> unchanged) {

    if (!mainWidgetFullyAttached()) return;

    ZOtherChangeEvent change(unchanged);

    auto f = [&](QObject *w) {
        QCoreApplication::sendEvent(w, &change);
        change.setAccepted(true);
    };

    f(mainWidget.data());
    zwidgetForEachDescendant(mainWidget.data(), f);
}

ZWidget *ZTerminal::focusWidget() const {
    if (tuiwidgets_impl()->focusWidget) {
        return tuiwidgets_impl()->focusWidget->pub();
    } else {
        return nullptr;
    }
}

ZWidget *ZTerminal::keyboardGrabber() const {
    auto *const p = tuiwidgets_impl();
    return p->keyboardGrab();
}

void ZTerminal::update() {
    if (tuiwidgets_impl()->updateRequested) {
        return;
    }
    tuiwidgets_impl()->updateRequested = true;
    // XXX ZTerminal uses updateRequest with null painter internally
    QCoreApplication::postEvent(this, new ZPaintEvent(ZPaintEvent::update, nullptr), Qt::LowEventPriority);
}

void ZTerminal::forceRepaint() {
    auto *const p = tuiwidgets_impl();
    p->processPaintingAndUpdateOutput(true);
}

ZImage ZTerminal::grabCurrentImage() const {
    auto *const surface = tuiwidgets_impl()->surface;
    ZImage img = ZImage(this, width(), height());
    termpaint_surface_copy_rect(surface, 0, 0, width(), height(),
                                ZImageData::get(&img)->surface, 0, 0,
                                TERMPAINT_COPY_NO_TILE, TERMPAINT_COPY_NO_TILE);
    return img;
}

QPoint ZTerminal::grabCursorPosition() const {
    auto *const p = tuiwidgets_impl();
    return p->terminalCursorPosition;
}

bool ZTerminal::grabCursorVisibility() const {
    auto *const p = tuiwidgets_impl();
    return p->terminalCursorVisible;
}

CursorStyle ZTerminal::grabCursorStyle() const {
    auto *const p = tuiwidgets_impl();
    return p->terminalCursorStyle;
}

std::tuple<int, int, int> ZTerminal::grabCursorColor() const {
    auto *const p = tuiwidgets_impl();
    return { p->terminalCursorR, p->terminalCursorG, p->terminalCursorB };
}

int ZTerminal::width() const {
    const auto *const surface = tuiwidgets_impl()->surface;
    return termpaint_surface_width(surface);

}

int ZTerminal::height() const {
    const auto *const surface = tuiwidgets_impl()->surface;
    return termpaint_surface_height(surface);
}

void ZTerminal::resize(int width, int height) {
    auto *const p = tuiwidgets_impl();
    termpaint_surface_resize(p->surface, width, height);
    if (p->mainWidgetFullyAttached()) {
        const QSize minSize = p->mainWidget->minimumSize().expandedTo(p->mainWidget->minimumSizeHint());
        p->mainWidget->setGeometry({0, 0, std::max(minSize.width(), termpaint_surface_width(p->surface)),
                                    std::max(minSize.height(), termpaint_surface_height(p->surface))});
        requestLayout(p->mainWidget);
    }
    forceRepaint();
}

void ZTerminalPrivate::updateNativeTerminalState() {
    if (titleNeedsUpdate) {
        termpaint_terminal_set_title(terminal, title.toUtf8().data(), TERMPAINT_TITLE_MODE_ENSURE_RESTORE);
        titleNeedsUpdate = false;
    }
    if (iconTitleNeedsUpdate) {
        termpaint_terminal_set_icon_title(terminal, title.toUtf8().data(), TERMPAINT_TITLE_MODE_ENSURE_RESTORE);
        iconTitleNeedsUpdate = true;
    }
}

void ZTerminal::updateOutput() {
    auto *const p = tuiwidgets_impl();
    if (p->initState == ZTerminalPrivate::InitState::InInitWithoutPendingPaintRequest) {
        p->initState = ZTerminalPrivate::InitState::InInitWithPendingPaintRequest;
    } else if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
        // already requested
    } else if (p->initState == ZTerminalPrivate::InitState::Ready) {
        p->updateNativeTerminalState();
        termpaint_terminal_flush(p->terminal, false);
    }
}

void ZTerminal::updateOutputForceFullRepaint() {
    auto *const p = tuiwidgets_impl();
    if (p->initState == ZTerminalPrivate::InitState::InInitWithoutPendingPaintRequest) {
        p->initState = ZTerminalPrivate::InitState::InInitWithPendingPaintRequest;
    } else if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
        // already requested
    } else if (p->initState == ZTerminalPrivate::InitState::Ready) {
        p->updateNativeTerminalState();
        termpaint_terminal_flush(tuiwidgets_impl()->terminal, true);
    }
}

QString ZTerminal::title() const {
    auto *const p = tuiwidgets_impl();
    return p->title;
}

void ZTerminal::setTitle(const QString &title) {
    auto *const p = tuiwidgets_impl();
    if (p->title == title) {
        return;
    }
    p->title = title;
    p->titleNeedsUpdate = true;
}

QString ZTerminal::iconTitle() const {
    auto *const p = tuiwidgets_impl();
    return p->iconTitle;
}

void ZTerminal::setIconTitle(const QString &title) {
    auto *const p = tuiwidgets_impl();
    if (p->iconTitle == title) {
        return;
    }
    p->iconTitle = title;
    p->iconTitleNeedsUpdate = true;
}

void ZTerminal::setAutoDetectTimeoutMessage(const QString &message) {
    auto *const p = tuiwidgets_impl();
    p->autoDetectTimeoutMessage = message;
}

QString ZTerminal::autoDetectTimeoutMessage() const {
    auto *const p = tuiwidgets_impl();
    return p->autoDetectTimeoutMessage;
}

bool ZTerminal::hasCapability(ZSymbol cap) const {
    auto *const p = tuiwidgets_impl();
    if (cap == extendedCharset) {
        return termpaint_terminal_capable(p->terminal, TERMPAINT_CAPABILITY_EXTENDED_CHARSET);
    }
    return false;
}

void ZTerminal::pauseOperation() {
    auto *const p = tuiwidgets_impl();
    if (p->initState != ZTerminalPrivate::InitState::Ready) return;
    if (p->externalConnection) {
        termpaint_terminal_pause(p->terminal);
        p->externalConnection->delegate->pause();
    } else {
        p->pauseTerminalForInternalConnection();
    }
    p->initState = ZTerminalPrivate::InitState::Paused;
}

void ZTerminal::unpauseOperation() {
    auto *const p = tuiwidgets_impl();
    if (p->initState != ZTerminalPrivate::InitState::Paused) return;
    if (p->externalConnection) {
        termpaint_terminal_unpause(p->terminal);
        p->externalConnection->delegate->unpause();
    } else {
        p->unpauseTerminalForInternalConnection();
    }
    p->initState = ZTerminalPrivate::InitState::Ready;
    updateOutputForceFullRepaint();
}

bool ZTerminal::isPaused() const {
    auto *const p = tuiwidgets_impl();
    return p->initState == ZTerminalPrivate::InitState::Paused;
}

std::unique_ptr<ZKeyEvent> ZTerminal::translateKeyEvent(const ZTerminalNativeEvent &nativeEvent) {
    termpaint_event *native = static_cast<termpaint_event*>(nativeEvent.nativeEventPointer());

    Tui::KeyboardModifiers modifiers = {};
    unsigned nativeModifier = 0;
    if (native->type == TERMPAINT_EV_KEY) {
        nativeModifier = native->key.modifier;
    } else if (native->type == TERMPAINT_EV_CHAR) {
        nativeModifier = native->c.modifier;
        if (nativeModifier == TERMPAINT_MOD_SHIFT) {
            // some keyboard modes assert shift for normal uppercase letters,
            // ignore that here as it's not generally meaningful.
            nativeModifier = 0;
        }
    }

    if (nativeModifier & TERMPAINT_MOD_SHIFT) {
        modifiers |= Tui::ShiftModifier;
    }
    if (nativeModifier & TERMPAINT_MOD_CTRL) {
        modifiers |= Tui::ControlModifier;
    }
    if (nativeModifier & TERMPAINT_MOD_ALT) {
        modifiers |= Tui::AltModifier;
    }

    if (native->type == TERMPAINT_EV_KEY) {
        int key = Tui::Key_unknown;
        if (native->key.atom == termpaint_input_page_up()) {
            key = Tui::Key_PageUp;
        } else if (native->key.atom == termpaint_input_page_down()) {
            key = Tui::Key_PageDown;
        } else if (native->key.atom == termpaint_input_arrow_right()) {
            key = Tui::Key_Right;
        } else if (native->key.atom == termpaint_input_arrow_left()) {
            key = Tui::Key_Left;
        } else if (native->key.atom == termpaint_input_arrow_down()) {
            key = Tui::Key_Down;
        } else if (native->key.atom == termpaint_input_arrow_up()) {
            key = Tui::Key_Up;
        } else if (native->key.atom == termpaint_input_tab()) {
            key = Tui::Key_Tab;
        } else if  (native->key.atom == termpaint_input_enter()) {
            key = Tui::Key_Enter;
        } else if  (native->key.atom == termpaint_input_backspace()) {
            key = Tui::Key_Backspace;
        } else if  (native->key.atom == termpaint_input_context_menu()) {
            key = Tui::Key_Menu;
        } else if  (native->key.atom == termpaint_input_delete()) {
            key = Tui::Key_Delete;
        } else if  (native->key.atom == termpaint_input_home()) {
            key = Tui::Key_Home;
        } else if  (native->key.atom == termpaint_input_insert()) {
            key = Tui::Key_Insert;
        } else if  (native->key.atom == termpaint_input_end()) {
            key = Tui::Key_End;
        } else if  (native->key.atom == termpaint_input_space()) {
            key = Tui::Key_Space;
        } else if  (native->key.atom == termpaint_input_escape()) {
            key = Tui::Key_Escape;
        } else if  (native->key.atom == termpaint_input_f1()) {
            key = Tui::Key_F1;
        } else if  (native->key.atom == termpaint_input_f2()) {
            key = Tui::Key_F2;
        } else if  (native->key.atom == termpaint_input_f3()) {
            key = Tui::Key_F3;
        } else if  (native->key.atom == termpaint_input_f4()) {
            key = Tui::Key_F4;
        } else if  (native->key.atom == termpaint_input_f5()) {
            key = Tui::Key_F5;
        } else if  (native->key.atom == termpaint_input_f6()) {
            key = Tui::Key_F6;
        } else if  (native->key.atom == termpaint_input_f7()) {
            key = Tui::Key_F7;
        } else if  (native->key.atom == termpaint_input_f8()) {
            key = Tui::Key_F8;
        } else if  (native->key.atom == termpaint_input_f9()) {
            key = Tui::Key_F9;
        } else if  (native->key.atom == termpaint_input_f10()) {
            key = Tui::Key_F10;
        } else if  (native->key.atom == termpaint_input_f11()) {
            key = Tui::Key_F11;
        } else if  (native->key.atom == termpaint_input_f12()) {
            key = Tui::Key_F12;
        } else if  (native->key.atom == termpaint_input_numpad_divide()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_division;
        } else if  (native->key.atom == termpaint_input_numpad_multiply()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_multiply;
        } else if  (native->key.atom == termpaint_input_numpad_subtract()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_Minus;
        } else if  (native->key.atom == termpaint_input_numpad_add()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_Plus;
        } else if  (native->key.atom == termpaint_input_numpad_enter()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_Enter;
        } else if  (native->key.atom == termpaint_input_numpad_decimal()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_Period;
        } else if  (native->key.atom == termpaint_input_numpad0()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_0;
        } else if  (native->key.atom == termpaint_input_numpad1()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_1;
        } else if  (native->key.atom == termpaint_input_numpad2()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_2;
        } else if  (native->key.atom == termpaint_input_numpad3()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_3;
        } else if  (native->key.atom == termpaint_input_numpad4()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_4;
        } else if  (native->key.atom == termpaint_input_numpad5()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_5;
        } else if  (native->key.atom == termpaint_input_numpad6()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_6;
        } else if  (native->key.atom == termpaint_input_numpad7()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_7;
        } else if  (native->key.atom == termpaint_input_numpad8()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_8;
        } else if  (native->key.atom == termpaint_input_numpad9()) {
            modifiers |= Tui::KeypadModifier;
            key = Tui::Key_9;
        }
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, QString()) };
    } else if (native->type == TERMPAINT_EV_CHAR) {
        const QString text = QString::fromUtf8(native->c.string, native->c.length);
        if (text == QStringLiteral(" ")) {
            return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(Tui::Key_Space, modifiers, QString()) };
        }
        if (text == QStringLiteral("\n")) {
            return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(Tui::Key_Enter, modifiers, QString()) };
        }
        int key = Tui::Key_unknown;
        return std::unique_ptr<ZKeyEvent>{ new ZKeyEvent(key, modifiers, text) };
    }

    return nullptr;
}

void ZTerminal::dispatchKeyboardEvent(ZKeyEvent &translated) {
    auto *const p = tuiwidgets_impl();
    if (p->keyboardGrabWidget) {
        if (p->keyboardGrabHandler) {
            p->keyboardGrabHandler(&translated);
        } else {
            QCoreApplication::sendEvent(p->keyboardGrabWidget, &translated);
        }
    } else if (!p->shortcutManager || !p->shortcutManager->process(&translated)) {
        QPointer<ZWidget> w = tuiwidgets_impl()->focus();
        while (w) {
            translated.accept();
            bool processed = QCoreApplication::sendEvent(w, &translated);
            if ((processed && translated.isAccepted()) || !w) {
                break;
            }
            w = w->parentWidget();
        }
    }
}

void ZTerminal::dispatchPasteEvent(ZPasteEvent &translated) {
    auto *const p = tuiwidgets_impl();
    if (p->keyboardGrabWidget) {
        if (p->keyboardGrabHandler) {
            p->keyboardGrabHandler(&translated);
        } else {
            QCoreApplication::sendEvent(p->keyboardGrabWidget, &translated);
        }
    } else {
        QPointer<ZWidget> w = tuiwidgets_impl()->focus();
        while (w) {
            translated.accept();
            bool processed = QCoreApplication::sendEvent(w, &translated);
            if ((processed && translated.isAccepted()) || !w) {
                break;
            }
            w = w->parentWidget();
        }
    }
}

void ZTerminalPrivate::adjustViewportOffset() {
    viewportOffset.setX(std::min(0, std::max(viewportRange.x(), viewportOffset.x())));
    viewportOffset.setY(std::min(0, std::max(viewportRange.y(), viewportOffset.y())));
}

bool ZTerminalPrivate::viewportKeyEvent(ZKeyEvent *translated) {
    if (viewportUI) {
        if (translated->key() == Tui::Key_F6 && translated->modifiers() == 0) {
            viewportUI = false;
            return false;
        } else if (translated->key() == Tui::Key_Escape) {
            viewportUI = false;
        } else if(translated->key() == Tui::Key_Left) {
            viewportOffset.setX(viewportOffset.x() + 1);
        } else if(translated->key() == Tui::Key_Right) {
            viewportOffset.setX(viewportOffset.x() - 1);
        } else if(translated->key() == Tui::Key_Down) {
            viewportOffset.setY(viewportOffset.y() - 1);
        } else if(translated->key() == Tui::Key_Up) {
            viewportOffset.setY(viewportOffset.y() + 1);
        }
        adjustViewportOffset();
        pub()->update();
    } else if (viewportActive && translated->key() == Tui::Key_F6 && translated->modifiers() == 0) {
        viewportUI = true;
        pub()->update();
    } else {
        return false;
    }
    return true;
}

bool ZTerminal::event(QEvent *event) {
    auto *const p = tuiwidgets_impl();
    if (event->type() == ZEventType::rawSequence()) {
        return false;
    }
    if (event->type() == ZEventType::terminalNativeEvent()) {
        termpaint_event *native = static_cast<termpaint_event*>(static_cast<ZTerminalNativeEvent*>(event)->nativeEventPointer());
        if (native->type == TERMPAINT_EV_CHAR || native->type == TERMPAINT_EV_KEY) {
            std::unique_ptr<ZKeyEvent> translated = translateKeyEvent(*static_cast<ZTerminalNativeEvent*>(event));
            if (translated) {
                if (!p->viewportKeyEvent(translated.get())) {
                    dispatchKeyboardEvent(*translated);
                }
                if (!translated->isAccepted()) {
                    if (translated->modifiers() == Tui::ControlModifier
                        && translated->text() == QStringLiteral("l")) {
                        forceRepaint();
                    }
                }
            }
        } else if (native->type == TERMPAINT_EV_PASTE) {
            if (native->paste.initial) {
                p->pasteTemp = QString::fromUtf8(native->paste.string, native->paste.length);
            } else {
                p->pasteTemp += QString::fromUtf8(native->paste.string, native->paste.length);
            }
            if (native->paste.final) {
                std::unique_ptr<ZPasteEvent> translated = std::make_unique<ZPasteEvent>(p->pasteTemp);
                dispatchPasteEvent(*translated);
            }
        } else if (native->type == TERMPAINT_EV_REPAINT_REQUESTED) {
            update();
        } else if (native->type == TERMPAINT_EV_AUTO_DETECT_FINISHED) {
            termpaint_terminal_auto_detect_apply_input_quirks(p->terminal, p->backspaceIsX08);
            if (termpaint_terminal_might_be_supported(p->terminal)
                    || (p->options & ZTerminal::ForceIncompatibleTerminals)) {
                p->autoDetectTimeoutTimer = nullptr;
                QByteArray nativeOptions;
                if (p->options & (ZTerminal::AllowInterrupt | ZTerminal::AllowQuit | ZTerminal::AllowSuspend)) {
                    nativeOptions.append(" +kbdsig ");
                }
                if (p->options & DisableAlternativeScreen) {
                    nativeOptions.append(" -altscreen ");
                }

                if (p->options & ConservativeTrueColorOutput) {
                    termpaint_terminal_disable_capability(p->terminal, TERMPAINT_CAPABILITY_TRUECOLOR_MAYBE_SUPPORTED);
                }

                termpaint_terminal_setup_fullscreen(p->terminal,
                                                    termpaint_surface_width(p->surface),
                                                    termpaint_surface_height(p->surface),
                                                    nativeOptions.data());

                if (p->initState == ZTerminalPrivate::InitState::InInitWithPendingPaintRequest) {
                    update();
                }
                p->initState = ZTerminalPrivate::InitState::Ready;

                if (!termpaint_terminal_might_be_supported(p->terminal)) {
                    incompatibleTerminalDetected();
                }

                if (!(p->options & DisableTaggedPaste) && termpaint_terminal_capable(p->terminal, TERMPAINT_CAPABILITY_MAY_TRY_TAGGED_PASTE)) {
                    termpaint_terminal_request_tagged_paste(p->terminal, true);
                }

                if (p->mainWidget) {
                    p->attachMainWidgetStage2();
                }
            } else {
                if (isSignalConnected(QMetaMethod::fromSignal(&ZTerminal::incompatibleTerminalDetected))) {
                    QPointer<ZTerminal> weak = this;
                    QTimer::singleShot(0, [weak] {
                        if (!weak.isNull()) {
                            weak->tuiwidgets_impl()->deinitTerminal();
                            weak->incompatibleTerminalDetected();
                        }
                    });
                } else {
                    QByteArray utf8 = QStringLiteral("Terminal auto detection failed. If this repeats the terminal might be incompatible.\r\n").toUtf8();
                    if (p->externalConnection) {
                        p->externalConnection->delegate->write(utf8.data(), utf8.size());
                        p->externalConnection->delegate->flush();
                    } else {
                        p->internalConnection_integration_write(utf8.data(), utf8.size());
                        p->internalConnection_integration_flush();
                    }
                    QPointer<ZTerminal> weak = this;
                    QTimer::singleShot(0, [weak] {
                        if (!weak.isNull()) {
                            weak->tuiwidgets_impl()->deinitTerminal();
                            QCoreApplication::quit();
                        }
                    });
                }
            }
        }

        return true; // ???
    }
    if (event->type() == ZEventType::updateRequest()) {
        // XXX ZTerminal uses updateRequest with null painter internally
        p->updateRequested = false;
        p->processPaintingAndUpdateOutput(false);
    }
    if (event->type() == QEvent::LayoutRequest) {
        Q_EMIT beforeRendering();
        p->layoutRequested = false;
        doLayout();
    }

    return QObject::event(event);
}

bool ZTerminal::eventFilter(QObject *watched, QEvent *event) {
    return QObject::eventFilter(watched, event);
}

void ZTerminal::timerEvent(QTimerEvent *event) {
    QObject::timerEvent(event);
}

void ZTerminal::childEvent(QChildEvent *event) {
    QObject::childEvent(event);
}

void ZTerminal::customEvent(QEvent *event) {
    QObject::customEvent(event);
}

void ZTerminal::connectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe
    QObject::connectNotify(signal);
}

void ZTerminal::disconnectNotify(const QMetaMethod &signal) {
    Q_UNUSED(signal);
    // XXX needs to be thread-safe

}

ZTerminal::OffScreen::OffScreen(int width, int height) : tuiwidgets_pimpl_ptr(width, height) {
}

ZTerminal::OffScreen::OffScreen(const ZTerminal::OffScreen&) = default;
ZTerminal::OffScreen::~OffScreen() = default;
ZTerminal::OffScreen& ZTerminal::OffScreen::operator=(const ZTerminal::OffScreen&) = default;

ZTerminal::OffScreen ZTerminal::OffScreen::withCapability(ZSymbol capability) const {
    OffScreen ret = *this;
    OffScreenData::get(&ret)->capabilities[capability] = true;
    return ret;
}

ZTerminal::OffScreen ZTerminal::OffScreen::withoutCapability(ZSymbol capability) const {
    OffScreen ret = *this;
    OffScreenData::get(&ret)->capabilities[capability] = false;
    return ret;
}

ZTerminal::OffScreenData::OffScreenData(int width, int height) : width(width), height(height) {
}

void ZTerminal::TerminalConnectionDelegate::pause() {
}

void ZTerminal::TerminalConnectionDelegate::unpause() {
}

ZTerminal::TerminalConnectionDelegate::TerminalConnectionDelegate() {
}

ZTerminal::TerminalConnectionDelegate::~TerminalConnectionDelegate() {
}

ZTerminal::TerminalConnection::TerminalConnection() : tuiwidgets_pimpl_ptr(std::make_unique<ZTerminal::TerminalConnectionPrivate>()) {
}

ZTerminal::TerminalConnection::~TerminalConnection() {
}

void ZTerminal::TerminalConnection::setDelegate(ZTerminal::TerminalConnectionDelegate *delegate) {
    auto *const p = tuiwidgets_impl();
    p->delegate = delegate;
}

void ZTerminal::TerminalConnection::setBackspaceIsX08(bool val) {
    auto *const p = tuiwidgets_impl();
    p->backspaceIsX08 = val;
}

void ZTerminal::TerminalConnection::setSize(int width, int height) {
    auto *const p = tuiwidgets_impl();
    if (p->width == width && p->height == height) {
        return;
    }
    p->width = width;
    p->height = height;
    if (p->terminal) {
        p->terminal->externalWasResized();
    }
}

#ifdef Q_CC_GNU
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
#define container_of(ptr, type, member) (reinterpret_cast<type *>(reinterpret_cast<char *>(ptr) - offsetof(type, member)))

void ZTerminalPrivate::initIntegrationCommon() {
    termpaint_integration_set_request_callback(&integration, [] (termpaint_integration *ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_request_callback();
    });
    termpaint_integration_set_awaiting_response(&integration, [] (termpaint_integration *ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->integration_awaiting_response();
    });
}

void ZTerminalPrivate::initExternal(ZTerminal::TerminalConnectionPrivate *connection, ZTerminal::Options options) {
    if (connection->terminal != nullptr) {
        qWarning("ZTerminal: ZTerminal::TerminalConnection instance already associated with an ZTerminal instance. This will not work.");
        // Nothing much we can do except crash of making this a dummy instance. For now cowardly refuse to crash.
        initOffscreen(ZTerminal::OffScreen{0, 0});
        return;
    }
    connection->terminal = this;
    externalConnection = connection;

    backspaceIsX08 = connection->backspaceIsX08;

    memset(&integration, 0, sizeof(integration));
    auto free = [] (termpaint_integration *ptr) {
        (void)ptr;
        // this does not really free, because ZTerminalPrivate which contains the integration struct is externally owned
    };
    auto write = [] (termpaint_integration *ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->externalConnection->delegate->write(data, length);
    };
    auto flush = [] (termpaint_integration *ptr) {
        container_of(ptr, ZTerminalPrivate, integration)->externalConnection->delegate->flush();
    };
    termpaint_integration_init(&integration, free, write, flush);

    initIntegrationCommon();

    termpaint_integration_set_restore_sequence_updated(&integration, [] (termpaint_integration *ptr, const char *data, int length) {
        container_of(ptr, ZTerminalPrivate, integration)->externalConnection->delegate->restoreSequenceUpdated(data, length);
    });

    callbackRequested = false;
    terminal = termpaint_terminal_new(&integration);
    surface = termpaint_terminal_get_surface(terminal);
    termpaint_surface_resize(surface, connection->width, connection->height);

    initCommon();
}

thread_local uint64_t ZTerminalPrivate::focusCounter = 0;

TUIWIDGETS_NS_END
