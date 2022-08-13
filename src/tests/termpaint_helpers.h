// SPDX-License-Identifier: BSL-1.0

#ifndef TESTS_TERMPAINT_HELPERS
#define TESTS_TERMPAINT_HELPERS

#include <memory>

#include <termpaint.h>

template<typename T>
using DEL = void(T*);
template<typename T, DEL<T> del> struct Deleter{
    void operator()(T* t) { del(t); }
};

template<typename T, DEL<T> del>
struct unique_cptr : public std::unique_ptr<T, Deleter<T, del>> {
    operator T*() {
        return this->get();
    }

    static unique_cptr take_ownership(T* owning_raw_ptr) {
        unique_cptr ret;
        ret.reset(owning_raw_ptr);
        return ret;
    }
};

using usurface_ptr = unique_cptr<termpaint_surface, termpaint_surface_free>;

struct TermpaintFixture {
    TermpaintFixture(int width = 1, int height = 1) {

        auto free = [] (termpaint_integration* ptr) {
            termpaint_integration_deinit(ptr);
        };
        auto write = [] (termpaint_integration* ptr, const char *data, int length) {
            (void)ptr; (void)data; (void)length;
        };
        auto flush = [] (termpaint_integration* ptr) {
            (void)ptr;
        };
        termpaint_integration_init(&integration, free, write, flush);
        terminal.reset(termpaint_terminal_new(&integration));
        surface = termpaint_terminal_get_surface(terminal);
        termpaint_surface_resize(surface, width, height);
        termpaint_terminal_set_event_cb(terminal, [](void *, termpaint_event *) {}, nullptr);
    }

    unique_cptr<termpaint_terminal, termpaint_terminal_free_with_restore> terminal;
    termpaint_surface *surface;
    termpaint_integration integration;
};


#endif // TESTS_TERMPAINT_HELPERS
