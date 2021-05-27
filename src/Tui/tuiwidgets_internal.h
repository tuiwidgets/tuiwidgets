#ifndef TUIWIDGETS_INTERNAL_H
#define TUIWIDGETS_INTERNAL_H

#include <QtGlobal>

// Don't use anything from here in your code. This is part of the private implementation

// Uses reinterpret_cast instead of static_cast, because it needs to work with incomplete types.
#define TUIWIDGETS_DECLARE_PRIVATE(Class) \
    inline Class##Private* tuiwidgets_impl() { return reinterpret_cast<Class##Private *>(tuiwidgets_pimpl_ptr.get()); } \
    inline const Class##Private* tuiwidgets_impl() const { return reinterpret_cast<const Class##Private *>(tuiwidgets_pimpl_ptr.get()); } \
    friend class Class##Private;

#define TUIWIDGETS_DECLARE_PUBLIC(Class) \
    inline Class* pub() { return static_cast<Class *>(pub_ptr); } \
    inline const Class* pub() const { return static_cast<const Class *>(pub_ptr); } \
    friend class Class;

#define TUIWIDGETS_NS_INLINE v0
#define TUIWIDGETS_NS_FULL Tui::TUIWIDGETS_NS_INLINE
#define TUIWIDGETS_NS_START namespace Tui { inline namespace TUIWIDGETS_NS_INLINE {
#define TUIWIDGETS_NS_END }}

#ifndef TUIWIDGETS_EXPORT
#define TUIWIDGETS_EXPORT Q_DECL_IMPORT
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
// Starting with qt 5.12 Q_DECLARE_OPERATORS_FOR_FLAGS for enums in the `Qt´ namespace is used in that namespace so
// the usage of Q_DECLARE_OPERATORS_FOR_FLAGS for the Tui namespace can be done in the Tui namespace without
// breaking usage of the qt enums/flags. Prefer doing that so that our enums/flags don't break if users have
// operator| in their own namespaces.
#define TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(Flags) Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)
#define TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(Flags)
#else
// Before qt 5.12 Q_DECLARE_OPERATORS_FOR_FLAGS for enums in the `Qt´ namespace is used in the global namespace so
// any usage of Q_DECLARE_OPERATORS_FOR_FLAGS in a namespace breaks usage of the qt enums/flags for code in that namespace.
// So usage of Q_DECLARE_OPERATORS_FOR_FLAGS in the Tui namespace should not be done. So when built old qt versions
// also use the global namespace for operator| overloads for enums in the Tui namespace.
#define TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_IN_NAMESPACE(Flags)
#define TUIWIDGETS_DECLARE_OPERATORS_FOR_FLAGS_GLOBAL(Flags) Q_DECLARE_OPERATORS_FOR_FLAGS(Tui::TUIWIDGETS_NS_INLINE::Flags)
#endif

#endif // TUIWIDGETS_INTERNAL_H
