#ifndef Corrade_Containers_AnyReference_h
#define Corrade_Containers_AnyReference_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <type_traits>

#ifndef CORRADE_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif

/** @file
 * @brief Class @ref Corrade::Containers::AnyReference
 * @m_since_latest
 */

namespace Corrade { namespace Containers {

/**
@brief Lightweight non-owning l-value and r-value reference wrapper
@m_since_latest

Combination of a @ref Reference and @ref MoveReference that accepts both
l-value and r-value references. The main use case is for APIs that take a list
of instances to *optionally* take over the ownership of --- each instance
remembers whether a l-value or a r-value reference was used to construct it and
exposes that through @ref isRvalue().

Like with @ref MoveReference, use in a @cpp constexpr @ce context is not
envisioned for this class and so the API is not @cpp constexpr @ce. There's no
STL equivalent and thus no conversion interface from/to an external
representation exists either.
*/
template<class T> class AnyReference {
    public:
        /**
         * @brief Construct from a l-value
         *
         * When this constructor is used, @ref isRvalue() returns @cpp false @ce.
         */
        /*implicit*/ AnyReference(T& reference) noexcept: _reference{&reference}, _isRvalue{false} {}

        /**
         * @brief Construct from a r-value
         *
         * When this constructor is used, @ref isRvalue() returns @cpp true @ce.
         */
        /*implicit*/ AnyReference(T&& reference) noexcept: _reference{&reference}, _isRvalue{true} {}

        /**
         * @brief Construct a reference from another of a derived type
         *
         * Expects that @p T is a base of @p U. The @ref isRvalue() state is
         * copied from @p other unchanged.
         */
        template<class U, class = typename std::enable_if<std::is_base_of<T, U>::value>::type> /*implicit*/ AnyReference(AnyReference<U> other) noexcept: _reference{other._reference}, _isRvalue{other._isRvalue} {}

        /** @brief Underlying reference */
        /*implicit*/ operator T&() const { return *_reference; }
        /** @overload */
        /*implicit*/ operator AnyReference<const T>() const {
            AnyReference<const T> out = *_reference;
            out._isRvalue = _isRvalue;
            return out;
        }

        /** @brief Underlying reference */
        T& get() const { return *_reference; }

        /**
         * @brief Whether the underlying reference is r-value
         *
         * Returns @cpp false @ce if the reference was constructed using
         * @ref AnyReference(T&), @cpp true @ce if using @ref AnyReference(T&&).
         */
        bool isRvalue() const { return _isRvalue; }

        /**
         * @brief Access the underlying reference
         *
         * @ref get(), @ref operator*()
         */
        T* operator->() const { return _reference; }

        /**
         * @brief Access the underlying reference
         *
         * @see @ref get(), @ref operator->()
         */
        T& operator*() const { return *_reference; }

    private:
        /* For the conversion constructor */
        template<class U> friend class AnyReference;

        T* _reference;
        bool _isRvalue;
};

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{AnyReference} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, AnyReference<T> value) {
    return debug << value.get();
}
#endif

}}

#endif
