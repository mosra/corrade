#ifndef Corrade_Containers_Pointer_h
#define Corrade_Containers_Pointer_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Corrade::Containers::Pointer, function @ref Corrade::Containers::pointer(), @ref Corrade::Containers::pointerCast()
 * @see @ref Corrade/Containers/PointerStl.h
 */

/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Tags.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Move.h"
#ifndef CORRADE_SINGLES_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct PointerConverter;

    /* Same as construct() utils in constructHelpers.h but not in-place. The ()
       instead of {} works around a featurebug in C++ where new T{} doesn't
       work for an explicit defaulted constructor, additionally it works around
       GCC 4.8 bugs where copy/move construction can't be done with {} for
       plain structs. For details see constructHelpers.h and the
       PointerTest::constructorExplicitInCopyInitialization() and following
       test cases. */
    template<class T, class First, class ...Next> inline T* allocate(First&& first, Next&& ...next) {
        return new T{Utility::forward<First>(first), Utility::forward<Next>(next)...};
    }
    template<class T> inline T* allocate() {
        return new T();
    }
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
    template<class T> inline T* allocate(const T& b) {
        return new T(b);
    }
    template<class T> inline T* allocate(T&& b) {
        return new T(Utility::move(b));
    }
    #endif

    /* Guards delete calls in the destructor, reset() and emplace(). If T is
       incomplete (forward declared), it picks the get(...) overload as
       decltype(sizeof(U)) fails for the incomplete type. A
        static_assert(sizeof(T), "...");
       would technically achieve the same (failing if the type is incomplete),
       but with a far worse error message than this type trait. STL unique_ptr
       in libstdc++ does exactly that, and it's very ugly.

       As definition of this class is different based on whether a type is
       defined or not (and thus differing between a header and a source file
       for a PIMPL'd class), it'd be an ODR violation, which could lead to the
       linker *theoretically* using the wrong definition when discarding
       duplicates. To avoid that for the most part, the definition is wrapped
       in an anonymous namespace and thus each compile unit should get a
       different one. There's still a scenario like

        struct ForwardDeclared;
        // IsComplete<Incomplete>::value == false
        struct ForwardDeclared {};
        // IsComplete<Incomplete>::value == true

       where this would be a problem, but let's just hope nobody tries to abuse
       it that way. It's in the Implementation namespace for that reason, to be
       used only in static_assert() inside Pointer and nowhere else. */
    namespace { template<class T> class IsComplete {
        template<class U> static char get(U*, decltype(sizeof(U))* = nullptr);
        static short get(...);
        public:
            enum: bool { value = sizeof(get(static_cast<T*>(nullptr))) == sizeof(char) };
    }; }
}

/**
@brief Lightweight unique pointer

An alternative to @ref std::unique_ptr from C++11, provides an owning move-only
wrapper over a pointer of type @p T, calling @cpp delete @ce on it on
destruction. The @ref pointer() convenience function also provides an
equivalent for C++14 @ref std::make_unique(), but on C++11 as well. Can be also
thought of as a heap-allocated counterpart to @ref Optional.

Compared to @ref std::unique_ptr, this class does proper @cpp const @ce
propagation as would be expected from any other owning container like
@ref Array or @ref String --- i.e., it's only possible to mutate the owned data
if the instance is not @cpp const @ce. There's no STL functionality with such
behavior except for the proposed @m_class{m-doc-external} [std::indirect_value](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1950r1.html).

Unlike @ref std::unique_ptr, this class does not provide custom deleters,
doesn't work with arrays and doesn't have a @cpp constexpr @ce API. On the
other hand that makes it fairly simple and lightweight. If you need a custom
deleter, use either @ref ScopeGuard or the standard @ref std::unique_ptr. For
owning array wrappers use @ref Array, which maintains a size information and
also supports custom deleters.

@section Containers-Pointer-incomplete-types Usage with incomplete types

The @ref Pointer class can wrap an incomplete (forward-declared) type, for
example to use it for the [PIMPL idiom](https://en.wikipedia.org/wiki/Opaque_pointer).
The type is expected to be defined when the destructor is called or when using
@ref reset(), as otherwise calling @cpp delete @ce on a pointer to an
incomplete type wouldn't call its destructor, leading to resource leaks. In
practice it means that a PIMPL class has to define its copy/move constructors,
assignments and the destructor in the source file as well instead of relying on
them being added implicitly by the compiler. For example, this is how a header
with the forward-declared @cpp struct State @ce would have to look like,
explicitly declaring but *not defining* a move constructor, destructor and move
assignment:

@snippet Containers.cpp Pointer-pimpl-header

The defaulted move constructor, destructor, and move assignment would then be
in the source file together with the definition of `State`:

@snippet Containers.cpp Pointer-pimpl-source

@section Containers-Pointer-stl STL compatibility

Instances of @ref Pointer are implicitly move-convertible to and from
@ref std::unique_ptr if you include @ref Corrade/Containers/PointerStl.h. The
conversion is provided in a separate header to avoid unconditional
@cpp #include <memory> @ce, which significantly affects compile times.
Additionally, the @ref pointer(T&&) overload also allows for such a conversion.
Example:

@snippet Containers-stl.cpp Pointer

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradePointer.h](https://github.com/mosra/magnum-singles/tree/master/CorradePointer.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information. The above
    mentioned STL compatibility is included as well, but disabled by default.
    Enable it by specifying @cpp #define CORRADE_POINTER_STL_COMPATIBILITY @ce
    before including the file. Including it multiple times with different
    macros defined works as well.

@see @ref pointer(T*), @ref pointer(Args&&... args), @ref pointerCast(),
    @ref Reference
*/
template<class T> class Pointer {
    static_assert(!std::is_array<T>::value, "use Containers::Array for arrays instead");

    public:
        /**
         * @brief Value type
         * @m_since_latest
         */
        typedef T Type;

        /**
         * @brief Default constructor
         *
         * Creates a @cpp nullptr @ce unique pointer.
         * @see @ref operator bool(), @ref reset()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ Pointer(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either a Pointer or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> /*implicit*/ Pointer(U) noexcept: _pointer{} {}
        /*implicit*/ Pointer() noexcept: _pointer{} {}
        #endif

        /**
         * @brief Construct a unique pointer by value
         *
         * Takes ownership of the passed pointer.
         * @see @ref operator bool(), @ref operator->()
         */
        explicit Pointer(T* pointer) noexcept: _pointer{pointer} {}

        /**
         * @brief Construct a unique pointer in-place
         *
         * Allocates a new object by passing @p args to its constructor.
         * @see @ref operator bool(), @ref operator->()
         */
        template<class ...Args> explicit Pointer(Corrade::InPlaceInitT, Args&&... args): _pointer{
            /* This works around a featurebug in C++ where new T{} doesn't work
               for an explicit defaulted constructor. Additionally it works
               around GCC 4.8 bugs where copy/move construction can't be done
               with {} for plain structs. */
            Implementation::allocate<T>(Utility::forward<Args>(args)...)
        } {}

        /**
         * @brief Construct a unique pointer from another of a derived type
         *
         * Expects that @p T is a base of @p U. In order to avoid resource
         * leaks when deleting through the base pointer, either @p U is
         * expected to be trivially destructible or @p T is expected to have a
         * virtual destructor. For downcasting (base to derived) use
         * @ref pointerCast(). Calls @ref release() on @p other.
         */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_base_of<T, U>::value, int>::type = 0
            #endif
        > /*implicit*/ Pointer(Pointer<U>&& other) noexcept: _pointer{other.release()} {
            static_assert(std::is_trivially_destructible<U>::value || std::has_virtual_destructor<T>::value, "the derived type should be trivially destructible or the base type should have a virtual destructor");
        }

        /**
         * @brief Construct a unique pointer from external representation
         *
         * @see @ref Containers-Pointer-stl, @ref pointer(T&&)
         */
        template<class U, class = decltype(Implementation::PointerConverter<T, U>::from(std::declval<U&&>()))> /*implicit*/ Pointer(U&& other) noexcept: Pointer{Implementation::PointerConverter<T, U>::from(Utility::move(other))} {}

        /** @brief Copying is not allowed */
        Pointer(const Pointer<T>&) = delete;

        /** @brief Move constructor */
        Pointer(Pointer<T>&& other) noexcept: _pointer{other._pointer} {
            other._pointer = nullptr;
        }

        /** @brief Copying is not allowed */
        Pointer<T>& operator=(const Pointer<T>&) = delete;

        /** @brief Move assignment */
        Pointer<T>& operator=(Pointer<T>&& other) noexcept {
            Utility::swap(_pointer, other._pointer);
            return *this;
        }

        /**
         * @brief Convert the unique pointer to external representation
         *
         * @see @ref Containers-Pointer-stl
         */
        template<class U, class = decltype(Implementation::PointerConverter<T, U>::to(std::declval<Pointer<T>&&>()))> /*implicit*/ operator U() && {
            return Implementation::PointerConverter<T, U>::to(Utility::move(*this));
        }

        /**
         * @brief Equality comparison to a null pointer
         *
         * Returns @cpp true @ce if the poiner is @cpp nullptr @ce,
         * @cpp false @ce otherwise.
         * @see @ref operator bool()
         */
        bool operator==(std::nullptr_t) const { return !_pointer; }

        /**
         * @brief Non-equality comparison to a null pointer
         *
         * Returns @cpp false @ce if the pointer is @cpp nullptr @ce,
         * @cpp false @ce otherwise.
         * @see @ref operator bool()
         */
        bool operator!=(std::nullptr_t) const { return _pointer; }

        /**
         * @brief Destructor
         *
         * Calls @cpp delete @ce on the stored pointer. In order to avoid
         * resource leaks when deleting the pointer, the type is expected to be
         * complete when calling the destructor. See
         * @ref Containers-Pointer-incomplete-types for more information.
         */
        ~Pointer() {
            static_assert(Implementation::IsComplete<T>::value, "attempting to delete a pointer to an incomplete type");
            delete _pointer;
        }

        /**
         * @brief Whether the pointer is non-null
         *
         * Returns @cpp false @ce if stored pointer is @cpp nullptr @ce,
         * @cpp true @ce otherwise.
         */
        explicit operator bool() const { return _pointer; }

        /**
         * @brief Underlying pointer value
         *
         * @see @ref operator bool(), @ref operator->(), @ref release()
         */
        T* get() { return _pointer; }
        const T* get() const { return _pointer; } /**< @overload */

        /**
         * @brief Access the underlying pointer
         *
         * Expects that the pointer is not @cpp nullptr @ce.
         * @see @ref operator bool(), @ref get(), @ref operator*(),
         *      @ref release()
         */
        T* operator->() {
            CORRADE_DEBUG_ASSERT(_pointer, "Containers::Pointer: the pointer is null", nullptr);
            return _pointer;
        }

        /** @overload */
        const T* operator->() const {
            CORRADE_DEBUG_ASSERT(_pointer, "Containers::Pointer: the pointer is null", nullptr);
            return _pointer;
        }

        /**
         * @brief Access the underlying pointer
         *
         * Expects that the pointer is not @cpp nullptr @ce.
         * @see @ref operator bool(), @ref get(), @ref operator->(),
         *      @ref release()
         */
        T& operator*() {
            CORRADE_DEBUG_ASSERT(_pointer, "Containers::Pointer: the pointer is null", *_pointer);
            return *_pointer;
        }

        /** @overload */
        const T& operator*() const {
            CORRADE_DEBUG_ASSERT(_pointer, "Containers::Pointer: the pointer is null", *_pointer);
            return *_pointer;
        }

        /** @todo operator->*(), is it even possible in a generic way without
            stamping out all possible variants (variables, functions, const
            functions, ...)? */

        /**
         * @brief Reset the pointer to a new value
         *
         * Calls @cpp delete @ce on the previously stored pointer and replaces
         * it with @p pointer. In order to avoid resource leaks when deleting
         * the pointer, the type is expected to be complete when calling this
         * function. See @ref Containers-Pointer-incomplete-types for more
         * information.
         * @see @ref emplace(), @ref release()
         */
        void reset(T* pointer = nullptr) {
            static_assert(Implementation::IsComplete<T>::value, "attempting to delete a pointer to an incomplete type");
            delete _pointer;
            _pointer = pointer;
        }

        /**
         * @brief Emplace a new value
         *
         * Calls @cpp delete @ce on the previously stored pointer and allocates
         * a new object by passing @p args to its constructor.
         */
        template<class ...Args> T& emplace(Args&&... args) {
            /* IsComplete<T> isn't checked here as that's guarded well enough
               by allocate<T>() below, which will fail to compile if the type
               isn't defined */
            delete _pointer;
            /* This works around a featurebug in C++ where new T{} doesn't work
               for an explicit defaulted constructor. Additionally it works
               around GCC 4.8 bugs where copy/move construction can't be done
               with {} for plain structs. */
            _pointer = Implementation::allocate<T>(Utility::forward<Args>(args)...);
            return *_pointer;
        }

        /**
         * @brief Emplace a new value of a derived type
         * @m_since_latest
         *
         * Calls @cpp delete @ce on the previously stored pointer and allocates
         * a new object of type @p U by passing @p args to its constructor. In
         * order to avoid resource leaks when deleting through the base
         * pointer, either @p U is expected to be trivially destructible or
         * @p T is expected to have a virtual destructor.
         */
        template<class U, class ...Args> U& emplace(Args&&... args) {
            static_assert(std::is_trivially_destructible<U>::value || std::has_virtual_destructor<T>::value, "the derived type should be trivially destructible or the base type should have a virtual destructor");
            /* IsComplete<T> isn't checked here as that's guarded well enough
               by allocate<U>() below, which will fail to compile if the type
               isn't defined. And if U is defined but T isn't, the assignment
               to _pointer will fail too as they're unrelated. */
            delete _pointer;
            /* This works around a featurebug in C++ where new T{} doesn't work
               for an explicit defaulted constructor. Additionally it works
               around GCC 4.8 bugs where copy/move construction can't be done
               with {} for plain structs. */
            U* const derived = Implementation::allocate<U>(Utility::forward<Args>(args)...);
            _pointer = derived;
            return *derived;
        }

        /**
         * @brief Release the pointer ownership
         *
         * Resets the stored pointer to @cpp nullptr @ce, returning the
         * previous value.
         * @see @ref get(), @ref reset()
         */
        T* release() {
            T* const out = _pointer;
            _pointer = nullptr;
            return out;
        }

    private:
        T* _pointer;
};

/** @relates Pointer
@brief Equality comparison of a null pointer and an unique pointer

See @ref Pointer::operator==(std::nullptr_t) const for more information.
*/
template<class T> bool operator==(std::nullptr_t, const Pointer<T>& b) { return b == nullptr; }

/** @relates Pointer
@brief Non-euality comparison of a null pointer and an unique pointer

See @ref Pointer::operator!=(std::nullptr_t) const for more information.
*/
template<class T> bool operator!=(std::nullptr_t, const Pointer<T>& b) { return b != nullptr; }

/** @relatesalso Pointer
@brief Make a unique pointer

Convenience alternative to @ref Pointer::Pointer(T*). The following two
lines are equivalent:

@snippet Containers.cpp pointer

@attention Note that for types that are constructible from their own pointer
    the call would get ambiguous between this function and
    @ref pointer(Args&&... args). Such case is forbidden at compile time in
    order to prevent potentially dangerous behavior and you need to explicitly
    use the @ref Pointer constructor instead.

@see @ref pointer(Args&&... args), @ref optional(T&&)
*/
template<class T> inline Pointer<T> pointer(T* pointer) {
    static_assert(!std::is_constructible<T, T*>::value, "the type is constructible from its own pointer, which is ambiguous -- explicitly use the constructor instead");
    return Pointer<T>{pointer};
}

namespace Implementation {
    template<class> struct DeducedPointerConverter;
}

/** @relatesalso Pointer
@brief Make a unique pointer from external representation

@see @ref Containers-Pointer-stl
*/
template<class T> inline auto pointer(T&& other) -> decltype(Implementation::DeducedPointerConverter<T>::from(Utility::move(other))) {
    return Implementation::DeducedPointerConverter<T>::from(Utility::move(other));
}

/** @relatesalso Pointer
@brief Downcast a pointer

While upcasting (derived to base) is handled implicitly with @ref Pointer::Pointer(Pointer<U>&&),
downcasting needs to be done explicitly. Performs @cpp static_cast<U>() @ce,
calling @ref Pointer::release() on @p pointer. You have to ensure the pointer
is actually of type @p U, as only the inheritance relation between @p T and
@p U is checked at compile time, not the actual type stored in @p pointer.

Casting with @cpp dynamic_cast<U>() @ce is not supported, as it would lead to a
destructive behavior in case the instance is not of type @p U. The standard
library provides @ref std::dynamic_pointer_cast() and friends for this case,
but they return a @ref std::shared_ptr in order to behave non-destructively.
*/
template<class U, class T> Pointer<U> pointerCast(Pointer<T>&& pointer) {
    return Pointer<U>{static_cast<U*>(pointer.release())};
}

namespace Implementation {
    template<class T, class ...Args> struct IsFirstAPointer: std::false_type {};
    template<class T> struct IsFirstAPointer<T, T*>: std::true_type {};
}

/** @relatesalso Pointer
@brief Make a unique pointer

Convenience alternative to @ref Pointer::Pointer(InPlaceInitT, Args&&... args),
similar to @ref std::make_unique() from C++14. The following two lines are
equivalent:

@snippet Containers.cpp pointer-inplace

@attention Note that for types that are constructible from their own pointer
    the call would get ambiguous between this function and @ref pointer(T*).
    Such case is forbidden at compile time in order to prevent potentially
    dangerous behavior and you need to explicitly use the @ref Pointer
    constructor instead.

@see @ref pointer(T*), @ref optional(Args&&... args)
*/
template<class T, class ...Args> inline Pointer<T> pointer(Args&&... args) {
    static_assert(!Implementation::IsFirstAPointer<T, Args...>::value || !std::is_constructible<T, T*>::value, "attempt to construct a type from its own pointer, which is ambiguous --  explicitly use the constructor instead");
    return Pointer<T>{Corrade::InPlaceInit, Utility::forward<Args>(args)...};
}

#ifndef CORRADE_SINGLES_NO_DEBUG
/** @debugoperator{Pointer} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, const Pointer<T>& value) {
    return debug << value.get();
}
#endif

}}

#endif
