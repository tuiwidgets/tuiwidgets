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
    // It also allows to be safe against deletion/reassignment while the function is running.
public:
    ZMoFunc() {}
    ZMoFunc(std::nullptr_t) {}
    ZMoFunc(ZMoFunc &&other) : _impl(move(other._impl)) {}
    ZMoFunc(const ZMoFunc&) = delete;

    template <typename F>
    ZMoFunc(F&& f)
        : _impl(new Impl<std::decay_t<F>>(std::forward<F&&>(f)))
    {
    }

public:
    void operator=(const ZMoFunc &other) = delete;
    void operator=(ZMoFunc &&other) {
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

        int refs = 1;
        void addRef() {
            refs++;
        }
        void release() {
            refs--;
            if (refs == 0) {
                delete this;
            }
        }
        struct Deleter {
            void operator()(Base *t) {
                t->release();
            }
        };

        struct KeepAlive {
            KeepAlive(Base *b) : b(b) {
                b->addRef();
            }

            ~KeepAlive() {
                b->release();
            }

            Base *b;
        };
    };


    template <typename F>
    struct Impl : public Base {
        Impl(const F &f) : _f(f) {
        }

        Impl(F &&f) : _f(std::move(f)) {
        }

        virtual ReturnType operator()(Args... args) override {
            typename Base::KeepAlive ka{this};
            return _f(args...);
        }

        F _f;
    };

    std::unique_ptr<Base, typename Base::Deleter> _impl;
};

}

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZUNIQUEFUNC_P_INCLUDED
