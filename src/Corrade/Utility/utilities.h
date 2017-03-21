#ifndef Corrade_Utility_utilities_h
#define Corrade_Utility_utilities_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Basic utilities
 */

#include <cstddef>
#include <cstring>

#include "Corrade/Utility/visibility.h"
#include "Corrade/configure.h"
#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/System.h"
#endif

namespace Corrade { namespace Utility {

/** @{ @name Type utilities */

/**
 * @brief Cast type to another of the same size
 *
 * Unlike `reinterpret_cast` this doesn't break strict-aliasing rules.
 */
template<class To, class From> inline To bitCast(const From& from) {
    /* Based on https://github.com/chromium/chromium/blob/trunk/base/basictypes.h#L306 */
    static_assert(sizeof(From) == sizeof(To), "Utility::bitCast(): resulting type must have the same size");

    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
}

/*@}*/

#ifdef CORRADE_BUILD_DEPRECATED
/**
 * @copybrief System::sleep()
 * @deprecated Use @ref System::sleep() instead.
 */
inline CORRADE_DEPRECATED("Use System::sleep() instead") void sleep(std::size_t ms) {
    return System::sleep(ms);
}
#endif

}}

#endif
