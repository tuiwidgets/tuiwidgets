// SPDX-License-Identifier: BSL-1.0

#ifndef TUIWIDGETS_UTILS_P_INCLUDED
#define TUIWIDGETS_UTILS_P_INCLUDED

#include <QList>
#include <QPointer>

#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

inline namespace IPrivate {

    template <typename T>
    QList<QPointer<T>> toQPointerList(QList<T*> input) {
        QList<QPointer<T>> result;
        result.reserve(input.size());
        for (T* ptr: input) {
            result.append(ptr);
        }
        return result;
    }

    template <typename T>
    QList<QPointer<T>> toQPointerListWithCast(QList<QObject*> input) {
        QList<QPointer<T>> result;
        result.reserve(input.size());
        for (QObject* ptr: input) {
            result.append(qobject_cast<T*>(ptr));
        }
        return result;
    }

}

TUIWIDGETS_NS_END

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
