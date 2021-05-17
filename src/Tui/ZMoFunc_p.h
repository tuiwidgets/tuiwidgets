#ifndef TUIWIDGETS_ZUNIQUEFUNC_P_INCLUDED
#define TUIWIDGETS_ZUNIQUEFUNC_P_INCLUDED

#include <memory>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

namespace Private {

template <typename T>
class ZMoFunc;

template<typename ReturnType, typename... Args>
class ZMoFunc<ReturnType(Args...)> {
    // Not implementing small object optimization keeps this fairly simple
public:
    ZMoFunc() {}
    ZMoFunc(std::nullptr_t) {}
    ZMoFunc(ZMoFunc&& other) : _impl(move(other._impl)) {}
    ZMoFunc(const ZMoFunc&) = delete;

    template <typename F>
    ZMoFunc(F&& f)
        : _impl(std::make_unique<Impl<std::decay_t<F>>>(std::forward<F&&>(f)))
    {
    }

public:
    void operator=(const ZMoFunc& other) = delete;
    void operator=(ZMoFunc&& other) {
        if (this == &other) return;
        _impl = move(other._impl);
    }

    explicit operator bool() const {
        return !empty();
    }

    bool empty() const {
        return _impl == nullptr;
    }

    ReturnType operator()(Args... args) const {
        return (*_impl)(args...);
    }

private:
    struct Base {
        virtual ~Base() = default;
        virtual ReturnType operator()(Args... args) = 0;
    };


    template <typename F>
    struct Impl : public Base {
        Impl(const F& f) : _f(f) {
        }

        Impl(F&& f) : _f(std::move(f)) {
        }

        virtual ReturnType operator()(Args... args) override {
            return _f(args...);
        }

        F _f;
    };

    std::unique_ptr<Base> _impl;
};

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZUNIQUEFUNC_P_INCLUDED
