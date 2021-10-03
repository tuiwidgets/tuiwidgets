#ifndef TUIWIDGETS_LISTNODE_P_INCLUDED
#define TUIWIDGETS_LISTNODE_P_INCLUDED

#include <QtGlobal>
#include <Tui/tuiwidgets_internal.h>

TUIWIDGETS_NS_START

template <typename Tag>
struct ListTrait;

template <typename T>
class ListNode;

template <typename T, typename Tag>
class ListHead {
public:
    ListHead(){}

    ~ListHead() {
        clear();
    }

    void clear() {
        while (first) {
            remove(first);
        }
    }

    void appendOrMoveToLast(T *e) {
        constexpr auto nodeOffset = ListTrait<Tag>::offset;
        auto& node = e->*nodeOffset;
        if (last == e) {
            return;
        }
        if (node.next) {
            //move
            remove(e);
        }
        if (last) {
            (last->*nodeOffset).next = e;
            node.prev = last;
            last = e;
        } else {
            first = e;
            last = e;
        }
    }

    void remove(T *e) {
        constexpr auto nodeOffset = ListTrait<Tag>::offset;
        auto& node = e->*nodeOffset;
        if (e == first) {
            first = node.next;
        }
        if (e == last) {
            last = node.prev;
        }
        if (node.prev) {
            (node.prev->*nodeOffset).next = node.next;
        }
        if (node.next) {
            (node.next->*nodeOffset).prev = node.prev;
        }
        node.prev = nullptr;
        node.next = nullptr;
    }

    T *first = nullptr;
    T *last = nullptr;
};


template <typename T>
class ListNode {
public:
    T *prev = nullptr;
    T *next = nullptr;
};

TUIWIDGETS_NS_END

#endif // TUIWIDGETS_LISTNODE_P_INCLUDED
