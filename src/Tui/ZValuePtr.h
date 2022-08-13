// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_ZVALUEPTR_INCLUDED
#define TUIWIDGETS_ZVALUEPTR_INCLUDED

#include <memory>
#include <type_traits>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START


template <typename T>
class TUIWIDGETS_EXPORT ZValuePtr {
public:
    ZValuePtr() : data(new T()) {
    }

    explicit ZValuePtr(T &other) : data(new T(other)) {
    }

    explicit ZValuePtr(T &&other) : data(new T(std::forward<T&&>(other))) {
    }

    template<typename Arg1, typename... Args, typename = typename std::enable_if<
                  std::conditional<
                     std::is_same<typename std::remove_cv<typename std::remove_reference<Arg1>::type>::type, ZValuePtr<T>>::value,
                     std::false_type, // make sure not to conflict with copy/move etc constructor or cause incomplete type errors (libc++)
                     std::is_constructible<T, Arg1, Args&&...>
                  >::type::value, void>::type>
    explicit ZValuePtr(Arg1 &&arg1, Args&&... args) : data(new T(std::forward<Arg1>(arg1), std::forward<Args>(args)...)) {
    }

    ~ZValuePtr() = default; // data destructor is fine.

    ZValuePtr(const ZValuePtr &other) : data(new T(*other.data)) {
    }

    ZValuePtr(ZValuePtr &&other) : data(std::exchange(other.data, std::make_unique<T>())) {
        // Safety over perf: Moved from state is default constructed state.
    }

    ZValuePtr &operator=(const ZValuePtr &other) {
        data.reset(new T(*other.data));
        return *this;
    }

    ZValuePtr &operator=(ZValuePtr &&other) {
        // Safety over perf: Moved from state is default constructed state.
        data = std::exchange(other.data, std::make_unique<T>());
        return *this;
    }

public:
    T *get() { return data.get(); }
    const T *get() const { return data.get(); }

    T &operator*() { return *data; }
    const T &operator*() const { return *data; }

    T &operator->() { return *data; }
    const T &operator->() const { return *data; }

private:
    std::unique_ptr<T> data;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZVALUEPTR_INCLUDED
