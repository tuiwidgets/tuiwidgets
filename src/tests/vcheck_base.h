// SPDX-License-Identifier: BSL-1.0

#ifndef VCHECK_BASE_H
#define VCHECK_BASE_H

// Helpers to check if classes match the ABI rules that all virtual functions of all base classes need to be
// overridden to avoid the problem that later adding new overrides is a ABI breaking change in many cases.

// These checks are made 100% from undefined. They assume a Itanium C++ ABI or similar and an encoding for
// member-pointers that uses the least significent bit to denote virtual methods.
// It then reinterpret_cast·s its way through the virtual function table (vtable) assuming the vtable pointer is the
// first value in the objects representation.

// restrict running this to an architecture that is a) covered by CI and b) uses a suitable abi.
#if defined(__amd64__) && defined(__linux__)

#define VIRTUAL_INDEX_PROBE(N, T, M)                                                                            \
class TestProbe_ ## N ## _ ## M: public T {                                                                     \
public:                                                                                                         \
    inline static auto memptr = &TestProbe_ ## N ## _ ## M::M;                                                  \
    inline static uintptr_t vtbl_offset = (reinterpret_cast<intptr_t>(*reinterpret_cast<void**>(&memptr)) ^ 1); \
};


#define VIRTUAL_FROM_INDEX(obj, offset) *reinterpret_cast<void**>(*reinterpret_cast<unsigned char**>(obj) + offset);

#define CHECK_VIRTUAL(N, M)                                                                                     \
    auto M ## method_ptrBase = VIRTUAL_FROM_INDEX(base, TestProbe_ ## N ## _ ## M::vtbl_offset);                \
    uintptr_t M ## _Base = reinterpret_cast<uintptr_t>(M ## method_ptrBase);                                    \
                                                                                                                \
    auto M ## method_ptrDerived = VIRTUAL_FROM_INDEX(derived, TestProbe_ ## N ## _ ## M::vtbl_offset);          \
    uintptr_t M ## _Derived = reinterpret_cast<uintptr_t>(M ## method_ptrDerived);                              \
                                                                                                                \
    CHECK(M ## _Base != M ## _Derived)

#else
// disable everywhere else
#define VIRTUAL_INDEX_PROBE(N, T, M)
#define CHECK_VIRTUAL(N, M) (void)base; (void)derived

#endif

#endif // VCHECK_BASE_H
