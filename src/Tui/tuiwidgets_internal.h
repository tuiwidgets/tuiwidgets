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

#endif // TUIWIDGETS_INTERNAL_H
