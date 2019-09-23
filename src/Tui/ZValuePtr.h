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

    explicit ZValuePtr(T& other) : data(new T(other)) {
    }

    explicit ZValuePtr(T&& other) : data(new T(std::forward<T&&>(other))) {
    }

    template< class... Args, typename = typename std::enable_if<std::is_constructible<T, Args&&...>::value, void>::type>
    explicit ZValuePtr(Args&&... args) : data(new T(std::forward<Args>(args)...)) {
    }

    ~ZValuePtr() = default; // data destructor is fine.

    ZValuePtr(const ZValuePtr &other) : data(new T(*other.data)) {
    }

    ZValuePtr(const ZValuePtr &&other) : data(std::exchange(move(other.data), new T())) {
        // Safety over perf: Moved from state is default constructed state.
    }

    ZValuePtr& operator=(const ZValuePtr& other) {
        data.reset(new T(*other.data));
        return *this;
    }

    ZValuePtr& operator=(const ZValuePtr&& other) {
        // Safety over perf: Moved from state is default constructed state.
        data = std::exchange(move(other.data), new T());
        return *this;
    }

public:
    T* get() { return data.get(); }
    const T* get() const { return data.get(); }

    T& operator*() { return *data; }
    const T& operator*() const { return *data; }

    T& operator->() { return *data; }
    const T& operator->() const { return *data; }

private:
    std::unique_ptr<T> data;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZVALUEPTR_INCLUDED
