#ifndef Corrade_Interconnect_Interconnect_h
#define Corrade_Interconnect_Interconnect_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#ifdef CORRADE_BUILD_DEPRECATED
/** @file
 * @brief Forward declarations for the @ref Corrade::Interconnect namespace
 * @m_deprecated_since_latest Design of the @ref Corrade::Interconnect library
 *      relies on member function pointers being unique, which is impossible to
 *      guarantee across all platform configurations and compilers, leading to
 *      subtle hard-to-discover bugs. The library is thus scheduled for
 *      removal, at the moment with no builtin replacement.
 */
#endif

#include "Corrade/configure.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include <cstddef>

#include "Corrade/Utility/DeprecationMacros.h"

#ifndef _CORRADE_NO_DEPRECATED_INTERCONNECT
CORRADE_DEPRECATED_FILE("the Interconnect library is broken by design and thus obsolete")
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Interconnect {

class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") Connection;
class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") Emitter;
class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") Receiver;

namespace Implementation {
    struct ConnectionData;
    class SignalData;
}

template<std::size_t, std::size_t, class, class> class CORRADE_DEPRECATED("the Interconnect library is broken by design and thus obsolete") StateMachine;

}}
#endif
#else
#error the Interconnect library is broken by design and thus obsolete
#endif

#endif
