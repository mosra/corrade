/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2020 Jonathan Hale <squareys@googlemail.com>

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

#include "ErrorString.h"

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/Debug.h"

#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS)
#include <string.h>
#endif

#ifdef CORRADE_TARGET_WINDOWS
#include "Corrade/Containers/StringView.h"

#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>
#endif

namespace Corrade { namespace Utility { namespace Implementation {

void printErrnoErrorString(Debug& debug, const int error) {
    debug << "error" << error;

    /* If we are on a known system, print also a string equivalent of the
       message, which may or may not be localized and may depend on the system
       in question. Of course std::strerror() is not thread-safe so we won't
       even bother with that -- on Unix we'll use strerror_r(), on Windows
       strerror_s() and otherwise just stay with errno alone. */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* A 256 byte buffer should be big enough for most error messages. The
       functions should make the string null-terminated. The function has two
       different signatures based on a phase of the moon, #undef'ing
       _GNU_SOURCE to get the sane signature always didn't seem like a good
       idea. The POSIX variant returns int(0) on success, while the GNU variant
       may return a pointer to a statically allocated string instead of filling
       the buffer. Sigh. */
    #if ((_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_APPLE)
    char string[256];
    CORRADE_INTERNAL_ASSERT_OUTPUT(strerror_r(error, string, Containers::arraySize(string)) == 0);
    #else
    char buffer[256];
    const char* const string = strerror_r(error, buffer, Containers::arraySize(buffer));
    #endif
    debug << "(" << Debug::nospace << string << Debug::nospace << ")";
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* "Your string message can be, at most, 94 characters long." Wow ok, not
       gonna trust you on that tho. */
    char string[256];
    CORRADE_INTERNAL_ASSERT_OUTPUT(strerror_s(string, Containers::arraySize(string), error) == 0);
    debug << "(" << Debug::nospace << string << Debug::nospace << ")";
    #endif
}

#ifdef CORRADE_TARGET_WINDOWS
void printWindowsErrorString(Debug& debug, unsigned int errorCode) {
    WCHAR errorStringW[256];
    const std::size_t sizeW = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, errorStringW, Containers::arraySize(errorStringW), nullptr);

    /* Cut off final newline that FormatMessage adds and convert to UTF-8. Yes,
       a \r\n, IT'S WINDOWS, BABY!!! */
    char errorString[256];
    const std::size_t size = WideCharToMultiByte(CP_UTF8, 0, errorStringW, sizeW - 2, errorString, Containers::arraySize(errorString), nullptr, nullptr);

    /* Print both the error code and the string so it's still somewhat helpful
       even when we have no chance of understanding what's being said in the
       localized text */
    debug << "error" << errorCode << "(" << Debug::nospace << Containers::StringView{errorString, size} << Debug::nospace << ")";
}
#endif

}}}
