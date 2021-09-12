#ifndef TUIWIDGETS_UTILS_P_INCLUDED
#define TUIWIDGETS_UTILS_P_INCLUDED

namespace {
    template <class... Ts> struct overloads;

    template <class T1, class... Ts>
    struct overloads<T1, Ts...> : T1, overloads<Ts...> {
            overloads(T1 head, Ts... tail) : T1(head), overloads<Ts...>(tail...) {}
            using T1::operator();
            using overloads<Ts...>::operator();
    };

    template <class T>
    struct overloads<T> : T {
        overloads(T fn) : T(fn) {}
        using T::operator();
    };

    template <class... Ts>
    overloads<Ts...> overload(Ts... fn) {
            return overloads<Ts...>(fn...);
    }
}

#endif // TUIWIDGETS_UTILS_P_INCLUDED
