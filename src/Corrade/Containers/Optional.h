#ifndef Corrade_Containers_Optional_h
#define Corrade_Containers_Optional_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::Optional, tag type @ref Corrade::Containers::NullOptT, tag @ref Corrade::Containers::NullOpt, function @ref Corrade::Containers::optional()
 * @see @ref Corrade/Containers/OptionalStl.h
 */

#include <new>
#include <type_traits>
#include <utility>

#include "Corrade/Containers/Tags.h"
#include "Corrade/Containers/constructHelpers.h"
#include "Corrade/Utility/Assert.h"
#ifndef CORRADE_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct OptionalConverter;
}

/**
@brief Null optional initialization tag type

Used to make initialization of null @ref Optional more explicit.
*/
/* Explicit constructor to avoid ambiguous calls when using {} */
struct NullOptT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    struct Init{};
    constexpr explicit NullOptT(Init) {}
    #endif
};

/**
@brief Null optional initialization tag

Use for explicit initialization of null @ref Optional.
*/
constexpr NullOptT NullOpt{NullOptT::Init{}};

/**
@brief Lightweight optional value

Equivalent to @ref std::optional from C++17, provides an optional checked
storage for object of type @p T. The optional object can be seen as a container
of @p T objects with maximal size 1 and can be in two states, either empty or
having a value. A non-allocating counterpart to @ref Pointer.

A common use for an optional object is for a return value of function that can
fail --- like @ref Pointer, but without the unnecessary allocation overhead.
Similarly to @ref Pointer, the presence of an object can be checked using
@ref operator bool(). The stored object can be accessed using
@ref operator->(), @ref operator*() or using implicit conversion, while attempt
to access a stored object in an empty state leads to assertion error.

Unlike @ref std::optional, this class does not provide a @cpp constexpr @ce
implementation or ordering operators, which makes it fairly simple and
lightweight. If you need the extra features, use the standard @ref std::optional.

@section Containers-Optional-stl STL compatibility

Instances of @ref Optional are *explicitly* copy- and move-convertible to and
from @ref std::optional if you include @ref Corrade/Containers/OptionalStl.h
and build your code with C++17 enabled. The conversion is provided in a
separate header to avoid unconditional @cpp #include <optional> @ce, which
significantly affects compile times. Additionally, the @ref optional(T&&)
overload also allows for such a conversion. Example:

@snippet Containers-stl17.cpp Optional

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradeOptional.h](https://github.com/mosra/magnum-singles/tree/master/CorradeOptional.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information. The above
    mentioned STL compatibility is included as well, but disabled by default.
    Enable it by compiling as C++17 and specifying
    @cpp #define CORRADE_OPTIONAL_STL_COMPATIBILITY @ce before including the
    file. Including it multiple times with different macros defined works as
    well.

@see @ref NullOpt, @ref optional(T&&), @ref optional(Args&&... args),
    @ref Reference, @ref Array1
*/
template<class T> class Optional {
    public:
        /**
         * @brief Default constructor
         *
         * Creates an optional object in empty state.
         * @see @ref operator bool(), @ref emplace()
         */
        /*implicit*/ Optional(NullOptT = NullOpt) noexcept: _set{false} {}

        /**
         * @brief Construct optional object by copy
         *
         * Stores a copy of passed object.
         * @see @ref operator bool(), @ref operator->(), @ref operator*()
         */
        /*implicit*/ Optional(const T& value) noexcept(std::is_nothrow_copy_assignable<T>::value): _set{true} {
            /* Can't use {}, see the GCC 4.8-specific overload for details */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            Implementation::construct(_value, value);
            #else
            new(&_value) T{value};
            #endif
        }

        /**
         * @brief Construct optional object by move
         *
         * Moves the passed object to internal storage.
         * @see @ref operator bool(), @ref operator->(), @ref operator*()
         */
        /*implicit*/ Optional(T&& value) noexcept(std::is_nothrow_move_assignable<T>::value): _set{true} {
            /* Can't use {}, see the GCC 4.8-specific overload for details */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            Implementation::construct(_value, std::move(value));
            #else
            new(&_value) T{std::move(value)};
            #endif
        }

        /**
         * @brief Construct optional object in-place
         *
         * Constructs the value by passing @p args to its constructor in-place.
         * @see @ref operator bool(), @ref operator->(), @ref operator*(),
         *      @ref emplace()
         */
        template<class ...Args> /*implicit*/ Optional(InPlaceInitT, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value): _set{true} {
            Implementation::construct(_value, std::forward<Args>(args)...);
        }

        /**
         * @brief Copy-construct an optional from external representation
         *
         * @see @ref Containers-Optional-stl, @ref optional(T&&)
         */
        template<class U, class = decltype(Implementation::OptionalConverter<T, U>::from(std::declval<const U&>()))> explicit Optional(const U& other) noexcept(std::is_nothrow_copy_constructible<T>::value): Optional{Implementation::OptionalConverter<T, U>::from(other)} {}

        /**
         * @brief Move-construct an optional from external representation
         *
         * @see @ref Containers-Optional-stl, @ref optional(T&&)
         */
        template<class U, class = decltype(Implementation::OptionalConverter<T, U>::from(std::declval<U&&>()))> explicit Optional(U&& other) noexcept(std::is_nothrow_move_constructible<T>::value): Optional{Implementation::OptionalConverter<T, U>::from(std::move(other))} {}

        /** @brief Copy constructor */
        Optional(const Optional<T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value);

        /** @brief Move constructor */
        Optional(Optional<T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value);

        /**
         * @brief Copy assignment
         *
         * If the object already contains a value, calls its destructor.
         * Copy-constructs the value from @p other using placement-new.
         */
        Optional<T>& operator=(const Optional<T>& other) noexcept(std::is_nothrow_copy_assignable<T>::value);

        /**
         * @brief Move assignment
         *
         * If both objects contain a value, the value is swapped. Otherwise,
         * if the object contains a value, calls its destructor. If @p other
         * contains a value, move-constructs the value from it using placement
         * new.
         */
        Optional<T>& operator=(Optional<T>&& other) noexcept(std::is_nothrow_move_assignable<T>::value);

        /**
         * @brief Copy-convert the optional to external representation
         *
         * @see @ref Containers-Optional-stl
         */
        template<class U, class = decltype(Implementation::OptionalConverter<T, U>::to(std::declval<const Optional<T>&>()))> explicit operator U() const & {
            return Implementation::OptionalConverter<T, U>::to(*this);
        }

        /**
         * @brief Move-convert the optional to external representation
         *
         * @see @ref Containers-Optional-stl
         */
        template<class U, class = decltype(Implementation::OptionalConverter<T, U>::to(std::declval<Optional<T>&&>()))> explicit operator U() && {
            return Implementation::OptionalConverter<T, U>::to(std::move(*this));
        }

        /**
         * @brief Clear the contained value
         *
         * If the object contains a value, calls its destructor. Compared to
         * @ref operator=(Optional<T>&&) this doesn't require the type to be
         * move-assignable.
         */
        Optional<T>& operator=(NullOptT) noexcept;

        /**
         * @brief Destructor
         *
         * If the optional object is not empty, calls destructor on stored
         * value.
         */
        ~Optional() { if(_set) _value.~T(); }

        /**
         * @brief Whether the optional object has a value
         *
         * Returns @cpp true @ce if the optional object has a value,
         * @cpp false @ce otherwise.
         */
        explicit operator bool() const { return _set; }

        /**
         * @brief Equality comparison to another optional
         *
         * Returns @cpp true @ce if either both instances are empty or both
         * instances have a value and the values compare equal, @cpp false @ce
         * otherwise.
         */
        bool operator==(const Optional<T>& other) const {
            return (!_set && !other._set) || (_set && other._set && _value == other._value);
        }

        /**
         * @brief Non-equality comparison to another optional
         *
         * Returns negation of @ref operator==(const Optional<T>&) const.
         */
        bool operator!=(const Optional<T>& other) const { return !operator==(other); }

        /**
         * @brief Equality comparison to a null optional
         *
         * Returns @cpp true @ce if the instance is empty, @cpp false @ce
         * otherwise.
         * @see @ref operator bool()
         */
        bool operator==(NullOptT) const { return !_set; }

        /**
         * @brief Non-equality comparison to a null optional
         *
         * Returns @cpp true @ce if the instance has a value, @cpp false @ce
         * otherwise.
         * @see @ref operator bool()
         */
        bool operator!=(NullOptT) const { return _set; }

        /**
         * @brief Equality comparison to a value
         *
         * Returns @cpp true @ce if the instance has a value which compares
         * equal to @p other, @cpp false @ce otherwise.
         */
        bool operator==(const T& other) const {
            return _set ? _value == other : false;
        }

        /**
         * @brief Non-equality comparison to a value
         *
         * Returns negation of @ref operator!=(const T&) const.
         */
        bool operator!=(const T& other) const { return !operator==(other); }

        /**
         * @brief Access the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool(), @ref operator*()
         */
        T* operator->() {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", &_value);
            return &_value;
        }

        /** @overload */
        const T* operator->() const {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", &_value);
            return &_value;
        }

        /**
         * @brief Access the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool(), @ref operator->()
         */
        T& operator*() & {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", _value);
            return _value;
        }

        /** @overload */
        T&& operator*() && {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", std::move(_value));
            return std::move(_value);
        }

        /** @overload */
        const T& operator*() const & {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", _value);
            return _value;
        }

        #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4
        /** @overload */
        /* This causes ambiguous overload on GCC 4.8 (and I assume 4.9 as
           well), so disabling it there. See also the corresponding test. */
        const T&& operator*() const && {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", std::move(_value));
            return std::move(_value);
        }
        #endif

        /**
         * @brief Emplace a new value
         *
         * If the object already contains a value, calls its destructor.
         * Constructs the value by passing @p args to its constructor using
         * placement new.
         */
        template<class ...Args> T& emplace(Args&&... args);

    private:
        union {
            T _value;
        };
        bool _set;
};

/** @relates Optional
@brief Equality comparison of a null optional and an optional

See @ref Optional::operator==(NullOptT) const for more information.
*/
template<class T> bool operator==(NullOptT, const Optional<T>& b) { return b == NullOpt; }

/** @relates Optional
@brief Non-euality comparison of a null optional and an optional

See @ref Optional::operator!=(NullOptT) const for more information.
*/
template<class T> bool operator!=(NullOptT, const Optional<T>& b) { return b != NullOpt; }

/** @relates Optional
@brief Equality comparison of a value and an optional

See @ref Optional::operator==(const T&) const for more information.
*/
template<class T> bool operator==(const T& a, const Optional<T>& b) { return b == a; }

/** @relates Optional
@brief Non-equality comparison of a value and an optional

See @ref Optional::operator!=(const T&) const for more information.
*/
template<class T> bool operator!=(const T& a, const Optional<T>& b) { return b != a; }

namespace Implementation {
    /* The default implementation has a Type member, but the user-provided
       ones don't. This is used to disambiguate whether optional(T&&) should
       return Optional<T> or U where U is result of the conversion from T. */
    template<class T> struct DeducedOptionalConverter { typedef T Type; };
}

/** @relatesalso Optional
@brief Make an optional

Convenience alternative to @ref Optional::Optional(const T&) or @ref Optional::Optional(T&&).
The following two lines are equivalent:

@snippet Containers.cpp optional

@attention Note that for types that are constructible from their own non-
    @cpp const @ce reference the call gets ambiguous between this function and
    @ref optional(Args&&... args). Such case is impossible to detect at compile
    time and you're advised to use the @ref Optional constructor explicitly to
    avoid surprising behavior.

@see @ref optional(Args&&... args), @ref pointer(T*)
*/
template<class T> inline
#ifdef DOXYGEN_GENERATING_OUTPUT
Optional<typename std::decay<T>::type>
#else
Optional<typename Implementation::DeducedOptionalConverter<typename std::decay<T>::type>::Type>
#endif
optional(T&& value) {
    return Optional<typename std::decay<T>::type>{std::forward<T>(value)};
}

/** @relatesalso Optional
@brief Make an optional

Convenience alternative to @ref Optional::Optional(InPlaceInitT, Args&&... args).
The following two lines are equivalent:

@snippet Containers.cpp optional-inplace

@attention Note that for types that are constructible from their own non-
    @cpp const @ce reference the call gets ambiguous between this function and
    @ref optional(T&&). Such case is impossible to detect at compile time and
    you're advised to use the @ref Optional constructor explicitly to avoid
    surprising behavior.

@see @ref optional(T&&), @ref pointer(Args&&... args)
*/
template<class T, class ...Args> inline Optional<T> optional(Args&&... args) {
    return Optional<T>{InPlaceInit, std::forward<Args>(args)...};
}

/** @relatesalso Optional
@brief Make an optional from external representation

@see @ref Containers-Optional-stl
*/
template<class T> inline auto optional(T&& other) -> decltype(Implementation::DeducedOptionalConverter<typename std::decay<T>::type>::from(std::forward<T>(other))) {
    return Implementation::DeducedOptionalConverter<typename std::decay<T>::type>::from(std::forward<T>(other));
}

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{NullOptT} */
inline Utility::Debug& operator<<(Utility::Debug& debug, NullOptT) {
    return debug << "Containers::NullOpt";
}

/** @debugoperator{Optional} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, const Optional<T>& value) {
    if(!value) return debug << NullOpt;
    else return debug << *value;
}
#endif

template<class T> Optional<T>::Optional(const Optional<T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value): _set(other._set) {
    if(_set)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_value, other._value);
        #else
        new(&_value) T{other._value};
        #endif
}

template<class T> Optional<T>::Optional(Optional<T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value): _set(other._set) {
    if(_set)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_value, std::move(other._value));
        #else
        new(&_value) T{std::move(other._value)};
        #endif
}

template<class T> Optional<T>& Optional<T>::operator=(const Optional<T>& other) noexcept(std::is_nothrow_copy_assignable<T>::value) {
    if(_set) _value.~T();
    if((_set = other._set))
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_value, other._value);
        #else
        new(&_value) T{other._value};
        #endif
    return *this;
}

template<class T> Optional<T>& Optional<T>::operator=(Optional<T>&& other) noexcept(std::is_nothrow_move_assignable<T>::value) {
    if(_set && other._set) {
        using std::swap;
        swap(other._value, _value);
    } else {
        if(_set) _value.~T();
        if((_set = other._set))
            /* Can't use {}, see the GCC 4.8-specific overload for details */
            #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
            Implementation::construct(_value, std::move(other._value));
            #else
            new(&_value) T{std::move(other._value)};
            #endif
    }
    return *this;
}

template<class T> Optional<T>& Optional<T>::operator=(NullOptT) noexcept {
    if(_set) _value.~T();
    _set = false;
    return *this;
}

template<class T> template<class ...Args> T& Optional<T>::emplace(Args&&... args) {
    /* Done like this instead of std::swap() so it works for non-movable /
       non-copyable types as well. */
    if(_set) _value.~T();
    _set = true;
    Implementation::construct<T>(_value, std::forward<Args>(args)...);
    return _value;
}

}}

#endif
