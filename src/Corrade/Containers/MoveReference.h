#ifndef Corrade_Containers_MoveReference_h
#define Corrade_Containers_MoveReference_h
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
 * @brief Class @ref Corrade::Containers::MoveReference
 * @m_since_latest
 */

namespace Corrade { namespace Containers {

/**
@brief Lightweight non-owning r-value reference wrapper
@m_since_latest

Counterpart to a @ref Reference that accepts r-value references. The main use
case is to store r-value references in a container --- for example when an API
needs to take a list of instances to take over ownership of. Use in a
@cpp constexpr @ce context is not envisioned for this class and so compared to
a @ref Reference the class is not @cpp constexpr @ce.

This class is exclusively for r-value references. If you want to accept only
l-value references, use a @ref Reference; if you want to accept both, use an
@ref AnyReference.

Unlike a @ref Reference, which corresponds to a @ref std::reference_wrapper,
this class doesn't have a STL equivalent. For that reason there's also no
interface to do a conversion from/to an external representation.
*/
template<class T> class MoveReference {
    public:
        /**
         * @brief Value type
         * @m_since_latest
         */
        typedef T Type;

        /** @brief Constructor */
        /*implicit*/ MoveReference(T&& reference) noexcept: _reference{&reference} {}

        /**
         * @brief Construction from l-value references is not allowed
         *
         * A @ref Reference can be created from l-value references instead.
         */
        MoveReference(T&) = delete;

        /**
         * @brief Construct a reference from another of a derived type
         *
         * Expects that @p T is a base of @p U.
         */
        template<class U, class = typename std::enable_if<std::is_base_of<T, U>::value>::type> /*implicit*/ MoveReference(MoveReference<U> other) noexcept: _reference{other._reference} {}

        /** @brief Underlying reference */
        /*implicit*/ operator T&() const { return *_reference; }

        /* No conversion to MoveReference<const T> because const&& references
           are basically useless in practice */

        /** @brief Underlying reference */
        T& get() const { return *_reference; }

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
        template<class U> friend class MoveReference;

        T* _reference;
};

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{MoveReference} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, MoveReference<T> value) {
    return debug << value.get();
}
#endif

}}

#endif
