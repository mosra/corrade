#ifndef Corrade_Utility_DebugStl_h
#define Corrade_Utility_DebugStl_h
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
@brief STL compatibility for @ref Corrade::Utility::Debug
@m_since{2019,10}

Including this header allows you to use STL types such as @ref std::string or
@ref std::tuple with @ref Corrade::Utility::Debug. See @ref Utility-Debug-stl
for more information.
*/

#include <iosfwd>
#include <string>
/* this one doesn't add much on top of <string>, so it doesn't need to be
   separate */
#include <tuple>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility {

/** @relatesalso Debug
@brief Print a @ref std::string to debug output
*/
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, const std::string& value);

/** @relatesalso Debug
@brief Print a @ref std::basic_string to debug output

All other types than exactly @ref std::string are printed as containers.
*/
template<class T> Debug& operator<<(Debug& debug, const std::basic_string<T>& value) {
    return debug << Containers::ArrayView<const T>{value.data(), value.size()};
}

namespace Implementation {
    /* Used by operator<<(Debug&, std::tuple<>...) */
    template<class T> inline void tupleDebugOutput(Debug&, const T&, Sequence<>) {}
    template<class T, std::size_t i, std::size_t ...sequence> void tupleDebugOutput(Debug& debug, const T& tuple, Sequence<i, sequence...>) {
        debug << std::get<i>(tuple);
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4127) /* conditional expression is constant (of course) */
        #endif
        if(i + 1 != std::tuple_size<T>::value)
            debug << Debug::nospace << ",";
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif
        tupleDebugOutput(debug, tuple, Sequence<sequence...>{});
    }
}

/** @relatesalso Debug
@brief Print a @ref std::tuple to debug output

Prints the value as @cb{.shell-session} (first, second, third...) @ce. Unlike
@ref operator<<(Debug& debug, const Iterable& value), the output is not
affected by @ref Debug::Flag::Packed / @ref Debug::packed.
*/
template<class ...Args> Debug& operator<<(Debug& debug, const std::tuple<Args...>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Debug::Flag::NoSpace));

    debug << "(" << Debug::nospace;
    Implementation::tupleDebugOutput(debug, value, typename Implementation::GenerateSequence<sizeof...(Args)>::Type{});
    debug << Debug::nospace << ")";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
}

namespace Implementation {

/* Used by Debug::operator<<(Implementation::DebugOstreamFallback&&) */
struct DebugOstreamFallback {
    template<class T> /*implicit*/ DebugOstreamFallback(const T& t): applier(&DebugOstreamFallback::applyImpl<T>), value(&t) {}

    void apply(std::ostream& s) const {
        (this->*applier)(s);
    }

    template<class T> void applyImpl(std::ostream& s) const {
        s << *static_cast<const T*>(value);
    }

    using ApplierFunc = void(DebugOstreamFallback::*)(std::ostream&) const;
    const ApplierFunc applier;
    const void* value;
};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
/* This is in order to support printing types that have ostream operator<<
   implemented */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Implementation::DebugOstreamFallback&& value);
#endif

}}

#endif
