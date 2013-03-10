#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

/** @file Utility/TypeTraits.h
 * @brief Type traits
 */

#include "corradeCompatibility.h"

namespace Corrade { namespace Utility {

/**
@brief Macro for creating traits class for checking whether an class has given inner type
@param type          Inner type to look for
@param className     Resulting trait class name

See @ref Corrade::Utility::IsIterable "IsIterable" class documentation for an example.
*/
/* Two overloaded get() functions return type of different size. Templated
   get() is used when T has given attribute, non-templated otherwise. Bool
   value then indicates whether the templated version was called or not. */
#define HasType(type, className)                                            \
template<class T> class className {                                         \
    typedef char SmallType;                                                 \
    typedef short LargeType;                                                \
                                                                            \
    className() = delete;                                                   \
    template<class U> static SmallType get(U&, typename U::type* = nullptr);\
    static LargeType get(...);                                              \
    static T& reference();                                                  \
                                                                            \
    public:                                                                 \
        static const bool Value =                                           \
            sizeof(get(reference())) == sizeof(SmallType);                  \
};

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief Traits class for checking whether given class is iterable (via const_iterator)

Actually created using HasType macro:
@code
HasType(const_iterator, IsIterable)
@endcode
*/
template<class T> struct IsIterable {
    /**
     * @brief Whether given class is iterable
     *
     * True when given class has const_iterator, false otherwise.
     */
    static const bool Value;
}
#else
HasType(const_iterator, IsIterable)
#endif

}}

#endif
