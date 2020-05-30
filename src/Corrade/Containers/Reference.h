#ifndef Corrade_Containers_Reference_h
#define Corrade_Containers_Reference_h
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

#include <type_traits>

#ifndef CORRADE_NO_DEBUG
#include "Corrade/Utility/Debug.h"
#endif

/** @file
 * @brief Class @ref Corrade::Containers::Reference
 * @see @ref Corrade/Containers/ReferenceStl.h
 */

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct ReferenceConverter;
}

/**
@brief Lightweight non-owning reference wrapper

Equivalent to @ref std::reference_wrapper from C++11, provides a copyable
non-owning wrapper over references to allow storing them in containers. Unlike
@ref std::reference_wrapper, this class does not provide @cpp operator() @ce
and there are no equivalents to @ref std::ref() / @ref std::cref() as they are
not deemed necessary --- in most contexts where @ref Reference is used, passing
a plain reference works just as well. This class is trivially copyable
(@ref std::reference_wrapper is guaranteed to be so only since C++17) and also
works on incomplete types, which @ref std::reference_wrapper knows since C++20.

@section Containers-Reference-stl STL compatibility

Instances of @ref Reference are implicitly convertible to and from
@ref std::reference_wrapper if you include @ref Corrade/Containers/ReferenceStl.h.
The conversion is provided in a separate header to avoid unconditional
@cpp #include <functional> @ce, which significantly affects compile times.
Example:

@snippet Containers-stl.cpp Reference

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class is also available as a single-header, dependency-less
    [CorradeReference.h](https://github.com/mosra/magnum-singles/tree/master/CorradeReference.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information. The above
    mentioned STL compatibility is included as well, but disabled by default.
    Enable it by specifying @cpp #define CORRADE_REFERENCE_STL_COMPATIBILITY @ce
    before including the file. Including it multiple times with different
    macros defined works as well.

@see @ref Pointer, @ref Optional
*/
template<class T> class Reference {
    public:
        /** @brief Constructor */
        constexpr /*implicit*/ Reference(T& reference) noexcept: _reference{&reference} {}

        /**
         * @brief Construct a reference from external representation
         *
         * @see @ref Containers-Reference-stl
         */
        template<class U, class = decltype(Implementation::ReferenceConverter<T, U>::from(std::declval<U>()))> constexpr /*implicit*/ Reference(U other) noexcept: Reference{Implementation::ReferenceConverter<T, U>::from(other)} {}

        /**
         * @brief Construction from r-value references is not allowed
         *
         * @todo Fix LWG 2993 / LWG 3041 (details in the skipped test)
         */
        Reference(T&&) = delete;

        /**
         * @brief Construct a reference from another of a derived type
         *
         * Expects that @p T is a base of @p U.
         */
        template<class U, class = typename std::enable_if<std::is_base_of<T, U>::value>::type> constexpr /*implicit*/ Reference(Reference<U> other) noexcept: _reference{&*other} {}

        /**
         * @brief Convert the reference to external representation
         *
         * @see @ref Containers-Reference-stl
         */
        template<class U, class = decltype(Implementation::ReferenceConverter<T, U>::to(std::declval<Reference<T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::ReferenceConverter<T, U>::to(*this);
        }

        /** @brief Underlying reference */
        constexpr /*implicit*/ operator T&() const { return *_reference; }
        constexpr /*implicit*/ operator Reference<const T>() const { return *_reference; } /**< @overload */

        /** @brief Underlying reference */
        constexpr T& get() const { return *_reference; }

        /**
         * @brief Access the underlying reference
         *
         * @ref get(), @ref operator*()
         */
        constexpr T* operator->() const {
            return _reference;
        }

        /**
         * @brief Access the underlying reference
         *
         * @see @ref get(), @ref operator->()
         */
        constexpr T& operator*() const {
            return *_reference;
        }

    private:
        T* _reference;
};

#ifndef CORRADE_NO_DEBUG
/** @debugoperator{Reference} */
template<class T> Utility::Debug& operator<<(Utility::Debug& debug, const Reference<T>& value) {
    return debug << value.get();
}
#endif

}}

#endif
