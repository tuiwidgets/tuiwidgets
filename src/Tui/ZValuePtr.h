#ifndef TUIWIDGETS_ZVALUEPTR_INCLUDED
#define TUIWIDGETS_ZVALUEPTR_INCLUDED

#include <memory>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

template <typename T>
class TUIWIDGETS_EXPORT ZValuePtr : public std::unique_ptr<T> {
public:
    using std::unique_ptr<T>::unique_ptr;
    ZValuePtr(const ZValuePtr &other) : std::unique_ptr<T>()
    {
        if (other) {
            this->reset(new T(*other.get()));
        }
    }

    ZValuePtr& operator=(const ZValuePtr& other) {
        if (other) {
            this->reset(new T(*other.get()));
        }
        return *this;
    }
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_ZVALUEPTR_INCLUDED
