#ifndef Corrade_Containers_Optional_h
#define Corrade_Containers_Optional_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
 */

#include <new>
#include <type_traits>

#include "Corrade/Containers/Tags.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

#ifdef CORRADE_BUILD_DEPRECATED
namespace Implementation { template<class, class> class OptionalConverter; }
#endif

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

Equivalent to `std::optional` from C++17, provides an optional checked storage
for object of type @p T. The optional object can be seen as a container of @p T
objects with maximal size 1 and can be in two states, either empty or having a
value. A non-allocating counterpart to @ref Pointer.

A common use for an optional object is for a return value of function that can
fail --- like @ref std::unique_ptr, but without the unnecessary allocation
overhead. Like with @ref std::unique_ptr, the presence of an object can be
checked using @ref operator bool(). The stored object can be accessed using
@ref operator->(), @ref operator*() or using implicit conversion, while attempt
to access a stored object in an empty state leads to assertion error.

Unlike `std::optional`, this class does not provide a @cpp constexpr @ce
implementation or ordering operators, which makes it fairly simple and
lightweight. If you need the extra features, use the standard `std::optional`.
@see @ref NullOpt, @ref optional(T&&), @ref optional(Args&&... args),
    @ref Reference
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
            new(&_value.v) T{value};
        }

        /**
         * @brief Construct optional object by move
         *
         * Moves the passed object to internal storage.
         * @see @ref operator bool(), @ref operator->(), @ref operator*()
         */
        /*implicit*/ Optional(T&& value) noexcept(std::is_nothrow_move_assignable<T>::value): _set{true} {
            new(&_value.v) T{std::move(value)};
        }

        /**
         * @brief Construct optional object in-place
         *
         * Constructs the value by passing @p args to its constructor in-place.
         * @see @ref operator bool(), @ref operator->(), @ref operator*(),
         *      @ref emplace()
         */
        template<class ...Args> /*implicit*/ Optional(InPlaceInitT, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value): _set{true} {
            new(&_value.v) T{std::forward<Args>(args)...};
        }

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
        ~Optional() { if(_set) _value.v.~T(); }

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
            return (!_set && !other._set) || (_set && other._set && _value.v == other._value.v);
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
            return _set ? _value.v == other : false;
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
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", &_value.v);
            return &_value.v;
        }

        /** @overload */
        const T* operator->() const {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", &_value.v);
            return &_value.v;
        }

        /**
         * @brief Access the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool(), @ref operator->()
         */
        T& operator*() {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", _value.v);
            return _value.v;
        }

        /** @overload */
        const T& operator*() const {
            CORRADE_ASSERT(_set, "Containers::Optional: the optional is empty", _value.v);
            return _value.v;
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief @copybrief operator*()
         * @deprecated Included only for compatibility with `std::optional`.
         *      Use @ref operator*() instead.
         */
        CORRADE_DEPRECATED("use operator*() instead") T& value() { return operator*(); }

        /** @brief @copybrief operator*()
         * @deprecated Included only for compatibility with `std::optional`.
         *      Use @ref operator*() instead.
         */
        CORRADE_DEPRECATED("use operator*() instead") const T& value() const { return operator*(); }
        #endif

        /**
         * @brief Emplace a new value
         *
         * If the object already contains a value, calls its destructor.
         * Constructs the value by passing @p args to its constructor using
         * placement new.
         */
        template<class ...Args> T& emplace(Args&&... args);

        #ifndef DOXYGEN_GENERATING_OUTPUT
        #ifdef CORRADE_BUILD_DEPRECATED
        /* Used by Magnum to provide backwards compatibility with (previously
           bundled) std::optional. I hate myself for this. */
        template<class U, class V = decltype(Implementation::OptionalConverter<T, U>::to(std::declval<Optional<T>>()))> operator U() const & {
            return Implementation::OptionalConverter<T, U>::to(*this);
        }
        template<class U, class V = decltype(Implementation::OptionalConverter<T, U>::to(std::declval<Optional<T>>()))> operator U() && {
            return Implementation::OptionalConverter<T, U>::to(std::move(*this));
        }
        #endif
        #endif

    private:
        union Storage {
            constexpr Storage() noexcept: _{} {}
            ~Storage() {}

            T v;
            char _;
        } _value;
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
template<class T> inline Optional<typename std::decay<T>::type> optional(T&& value) {
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

/** @debugoperator{Optional} */
inline Utility::Debug& operator<<(Utility::Debug& debug, NullOptT) {
    return debug << "Containers::NullOpt";
}

/** @debugoperator{Optional} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, const Optional<T>& value) {
    if(!value) return debug << NullOpt;
    else return debug << *value;
}

template<class T> Optional<T>::Optional(const Optional<T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value): _set(other._set) {
    if(_set) new(&_value.v) T{other._value.v};
}

template<class T> Optional<T>::Optional(Optional<T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value): _set(other._set) {
    if(_set) new(&_value.v) T{std::move(other._value.v)};
}

template<class T> Optional<T>& Optional<T>::operator=(const Optional<T>& other) noexcept(std::is_nothrow_copy_assignable<T>::value) {
    if(_set) _value.v.~T();
    if((_set = other._set)) new(&_value.v) T{other._value.v};
    return *this;
}

template<class T> Optional<T>& Optional<T>::operator=(Optional<T>&& other) noexcept(std::is_nothrow_move_assignable<T>::value) {
    if(_set && other._set) {
        using std::swap;
        swap(other._value.v, _value.v);
    } else {
        if(_set) _value.v.~T();
        if((_set = other._set)) new(&_value.v) T{std::move(other._value.v)};
    }
    return *this;
}

template<class T> Optional<T>& Optional<T>::operator=(NullOptT) noexcept {
    if(_set) _value.v.~T();
    _set = false;
    return *this;
}

template<class T> template<class ...Args> T& Optional<T>::emplace(Args&&... args) {
    if(_set) _value.v.~T();
    _set = true;
    new(&_value.v) T{std::forward<Args>(args)...};
    return _value.v;
}

}}

#endif
