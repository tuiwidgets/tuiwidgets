#include "ZImage.h"
#include "ZImage_p.h"

#include <termpaint_image.h>

#include "ZPainter_p.h"
#include "ZTerminal_p.h"

TUIWIDGETS_NS_START

ZImage::ZImage(const ZTerminal *terminal, int width, int height)
    : tuiwidgets_pimpl_ptr(new ZImageData(ZTerminalPrivate::get(terminal)->terminal, width, height)) {
}


ZImage::ZImage(QSharedDataPointer<ZImageData> pimpl) : tuiwidgets_pimpl_ptr(pimpl) {
}

std::unique_ptr<ZImage> ZImage::fromFile(const ZTerminal *terminal, QString fileName) {
    termpaint_surface *surface = termpaint_image_load(ZTerminalPrivate::get(terminal)->terminal,
                                                      fileName.toUtf8().data());
    if (!surface) {
        return nullptr;
    }

    return std::unique_ptr<ZImage>(new ZImage(QSharedDataPointer<ZImageData>{new ZImageData(ZTerminalPrivate::get(terminal)->terminal, surface)}));
}

std::unique_ptr<ZImage> ZImage::fromByteArray(const ZTerminal *terminal, QByteArray data) {
    termpaint_surface *surface = termpaint_image_load_from_buffer(ZTerminalPrivate::get(terminal)->terminal,
                                                                  data.data(), data.size());
    if (!surface) {
        return nullptr;
    }

    return std::unique_ptr<ZImage>(new ZImage(QSharedDataPointer<ZImageData>{new ZImageData(ZTerminalPrivate::get(terminal)->terminal, surface)}));
}

ZImage::ZImage(const ZImage &other) : tuiwidgets_pimpl_ptr(other.tuiwidgets_pimpl_ptr) {
    if (tuiwidgets_pimpl_ptr.constData()->hasPainters()) {
        tuiwidgets_pimpl_ptr.detach();
    }
}

ZImage::ZImage(ZImage &&other) : tuiwidgets_pimpl_ptr(std::move(other.tuiwidgets_pimpl_ptr)) {
    // ignore hasPainters() because using outstanding painter from the moved from object is
    // undefined anyway.
}

ZImage &ZImage::operator=(const ZImage & other) {
    tuiwidgets_pimpl_ptr = other.tuiwidgets_pimpl_ptr;
    if (tuiwidgets_pimpl_ptr.constData()->hasPainters()) {
        tuiwidgets_pimpl_ptr.detach();
    }
    return *this;
}

ZImage &ZImage::operator=(ZImage && other) {
    // ignore hasPainters() because using outstanding painter from the moved from object is
    // undefined anyway.
    tuiwidgets_pimpl_ptr = other.tuiwidgets_pimpl_ptr;
    return *this;
}

ZImage::~ZImage() {
}

bool ZImage::operator==(const ZImage &other) {
    auto *const p = tuiwidgets_pimpl_ptr.data();
    return termpaint_surface_same_contents(p->surface, other.tuiwidgets_pimpl_ptr.data()->surface);
}

bool ZImage::operator!=(const ZImage &other) {
    return !(*this == other);
}

void ZImage::swap(ZImage &other) {
    tuiwidgets_pimpl_ptr.swap(other.tuiwidgets_pimpl_ptr);
}

int ZImage::width() const {
    auto *const p = tuiwidgets_pimpl_ptr.data();
    return termpaint_surface_width(p->surface);
}

int ZImage::height() const {
    auto *const p = tuiwidgets_pimpl_ptr.data();
    return termpaint_surface_height(p->surface);
}

QSize ZImage::size() const {
    return { width(), height() };
}

bool ZImage::save(const QString &fileName) const {
    auto *surface = tuiwidgets_pimpl_ptr->surface;
    return termpaint_image_save(surface, fileName.toUtf8().data());
}

QByteArray ZImage::saveToByteArray() const {
    QByteArray ret;
    auto *surface = tuiwidgets_pimpl_ptr->surface;
    char *data = termpaint_image_save_alloc_buffer(surface);
    if (data) {
        ret = data;
        termpaint_image_save_dealloc_buffer(data);
    }
    return ret;

}

ZPainter ZImage::painter() {
    auto *const p = tuiwidgets_pimpl_ptr.data();
    std::shared_ptr<char> token = p->hasPainter.lock();
    if (!token) {
        token = std::make_shared<char>();
    }
    return ZPainter(std::make_unique<ZPainterPrivate>(p->surface,
                                                      termpaint_surface_width(p->surface),
                                                      termpaint_surface_height(p->surface),
                                                      token));
}

ZImageData::ZImageData(termpaint_terminal *terminal, int width, int height)
    : terminal(terminal)
{
    surface = termpaint_terminal_new_surface(terminal,
                                             width,
                                             height);
}

ZImageData::ZImageData(termpaint_terminal *terminal, termpaint_surface *surface)
    : terminal(terminal), surface(surface) {
}

ZImageData::ZImageData(const ZImageData &other)
    : QSharedData (other), terminal(other.terminal)
{
    int width = termpaint_surface_width(other.surface);
    int height = termpaint_surface_height(other.surface);

    surface = termpaint_terminal_new_surface(terminal,
                                             width,
                                             height);
    termpaint_surface_copy_rect(other.surface, 0, 0, width, height,
                                surface, 0, 0, TERMPAINT_COPY_NO_TILE, TERMPAINT_COPY_NO_TILE);
}

ZImageData &ZImageData::operator=(const ZImageData &other) {
    termpaint_surface_free(surface);
    hasPainter.reset();
    int width = termpaint_surface_width(other.surface);
    int height = termpaint_surface_height(other.surface);
    terminal = other.terminal;
    surface = termpaint_terminal_new_surface(terminal,
                                             width,
                                             height);
    termpaint_surface_copy_rect(other.surface, 0, 0, width, height,
                                surface, 0, 0, TERMPAINT_COPY_NO_TILE, TERMPAINT_COPY_NO_TILE);
    return *this;
}

ZImageData::~ZImageData() {
    termpaint_surface_free(surface);
}

ZImage ZImageData::createForTesting(termpaint_terminal *terminal, int width, int height) {
    return ZImage{QSharedDataPointer<Tui::ZImageData>(new Tui::ZImageData(terminal, width, height))};
}

TUIWIDGETS_NS_END
