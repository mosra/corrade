#ifndef Corrade_Containers_Optional_h
#define Corrade_Containers_Optional_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::Optional, tag type @ref Corrade::Containers::NullOptT, tag @ref Corrade::Containers::NullOpt
 */

#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Containers {

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
@brief Optional value

Equivalent to `std::optional` from C++17, provides an optional checked storage
for object of type @p T. The optional object can be seen as a container of @p T
objects with maximal size 1 and can be in two states, either empty or having a
value.

A common use for an optional object is for a return value of function that can
fail --- like @ref std::unique_ptr, but without the unnecessary allocation
overhead. Like with @ref std::unique_ptr, the presence of an object can be
checked using @ref operator bool(). The stored object can be accessed using
@ref operator->(), @ref operator*() or using implicit conversion, while attempt
to access a stored object in an empty state leads to assertion error.

Unlike `std::optional`, this class does not provide a @cpp constexpr @ce
implementation or any comparison operators, which makes it simpler and more
lightweight.
*/
template<class T> class Optional {
    public:
        /**
         * @brief Default constructor
         *
         * Creates an optional object in empty state.
         * @see @ref operator bool(), @ref emplace()
         */
        /*implicit*/ Optional() noexcept: _set{false} {}

        /**
         * @brief Null constructor
         *
         * Creates an optional object in empty state.
         * @see @ref operator bool(), @ref emplace()
         */
        /*implicit*/ Optional(NullOptT) noexcept: _set{false} {}

        /**
         * @brief Construct optional object by copy
         *
         * Stores a copy of passed object.
         * @see @ref operator bool(), @ref operator->()
         */
        /*implicit*/ Optional(const T& value): _set{true} {
            new(&_value.v) T{value};
        }

        /**
         * @brief Construct optional object by move
         *
         * Moves the passed object to internal storage.
         * @see @ref operator bool(), @ref operator->()
         */
        /*implicit*/ Optional(T&& value): _set{true} {
            new(&_value.v) T{std::move(value)};
        }

        /**
         * @brief Construct optional object in place
         *
         * Constructs the value by passing @p args to its constructor in-place.
         * @see @ref operator bool(), @ref operator->(), @ref emplace()
         */
        template<class ...Args> /*implicit*/ Optional(InPlaceInitT, Args&&... args): _set{true} {
            new(&_value.v) T{std::forward<Args>(args)...};
        }

        /** @brief Copy constructor */
        Optional(const Optional<T>& other);

        /** @brief Move constructor */
        Optional(Optional<T>&& other);

        /**
         * @brief Copy assignment
         *
         * If the object already contains a value, calls its destructor.
         * Copy-constructs the value from @p other using placement-new.
         */
        Optional<T>& operator=(const Optional<T>& other);

        /**
         * @brief Move assignment
         *
         * If both objects contain a value, the value is swapped. Otherwise,
         * if the object contains a value, calls its destructor. If @p other
         * contains a value, move-constructs the value from it using placement
         * new.
         */
        Optional<T>& operator=(Optional<T>&& other);

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
         * @brief Get the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool()
         */
        operator T&() {
            CORRADE_INTERNAL_ASSERT(_set);
            return _value.v;
        }

        /** @overload */
        operator const T&() const {
            CORRADE_INTERNAL_ASSERT(_set);
            return _value.v;
        }

        /**
         * @brief Access the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool()
         */
        T* operator->() {
            CORRADE_INTERNAL_ASSERT(_set);
            return &_value.v;
        }

        /** @overload */
        const T* operator->() const {
            CORRADE_INTERNAL_ASSERT(_set);
            return &_value.v;
        }

        /**
         * @brief Access the stored object
         *
         * Expects that the optional object has a value.
         * @see @ref operator bool()
         */
        T& operator*() {
            CORRADE_INTERNAL_ASSERT(_set);
            return _value.v;
        }

        /** @overload */
        const T& operator*() const {
            CORRADE_INTERNAL_ASSERT(_set);
            return _value.v;
        }

        /**
         * @brief Emplace a new value
         *
         * If the object already contains a value, calls its destructor.
         * Constructs the value by passing @p args to its constructor using
         * placement new.
         */
        template<class ...Args> T& emplace(Args&&... args);

    private:
        union Storage {
            constexpr Storage() noexcept: _{} {}
            ~Storage() {}

            T v;
            char _;
        } _value;
        bool _set;
};

template<class T> Optional<T>::Optional(const Optional<T>& other): _set(other._set) {
    if(_set) new(&_value.v) T{other._value.v};
}

template<class T> Optional<T>::Optional(Optional<T>&& other): _set(other._set) {
    if(_set) new(&_value.v) T{std::move(other._value.v)};
}

template<class T> Optional<T>& Optional<T>::operator=(const Optional<T>& other) {
    if(_set) _value.v.~T();
    if((_set = other._set)) new(&_value.v) T{other._value.v};
    return *this;
}

template<class T> Optional<T>& Optional<T>::operator=(Optional<T>&& other) {
    if(_set && other._set) {
        using std::swap;
        swap(other._value.v, _value.v);
    } else {
        if(_set) _value.v.~T();
        if((_set = other._set)) new(&_value.v) T{std::move(other._value.v)};
    }
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
