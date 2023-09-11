/*
    This file is part of Corrade.

    {{copyright}}

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

// {{includes}}

#pragma ACME comments off

#pragma ACME path ../
#pragma ACME local Corrade

/* For Corrade/configure.h */
#pragma ACME path ../../build/src

#pragma ACME disable CORRADE_BUILD_DEPRECATED

#pragma ACME disable CORRADE_NO_ASSERT
#pragma ACME disable CORRADE_NO_DEBUG_ASSERT
#pragma ACME enable CORRADE_STANDARD_ASSERT
#pragma ACME enable CORRADE_SINGLES_NO_DEBUG
#pragma ACME disable CORRADE_GRACEFUL_ASSERT
#pragma ACME disable DOXYGEN_GENERATING_OUTPUT

/* Array views publicize some internals for buffer protocol and for ArrayTuple,
   no need for that here */
#pragma ACME enable CORRADE_SINGLES_NO_PYTHON_COMPATIBILITY
#pragma ACME enable CORRADE_SINGLES_NO_ARRAYTUPLE_COMPATIBILITY

/* Make it possible to include Assert.h, DebugAssert.h and Macros.h multiple
   times */
#pragma ACME disable Corrade_Utility_Assert_h
#pragma ACME disable Corrade_Utility_DebugAssert_h
#pragma ACME disable Corrade_Utility_Macros_h

/* Excluded all macros by default. The libraries should then whitelist a subset
   using #pragma ACME disable. */
#pragma ACME enable _CORRADE_HELPER_PASTE2
#pragma ACME enable _CORRADE_HELPER_DEFER
#pragma ACME enable CORRADE_DEPRECATED
#pragma ACME enable CORRADE_DEPRECATED_ALIAS
#pragma ACME enable CORRADE_DEPRECATED_NAMESPACE
#pragma ACME enable CORRADE_DEPRECATED_ENUM
#pragma ACME enable CORRADE_DEPRECATED_FILE
#pragma ACME enable CORRADE_DEPRECATED_MACRO
#pragma ACME enable CORRADE_IGNORE_DEPRECATED_PUSH
#pragma ACME enable CORRADE_IGNORE_DEPRECATED_POP
#pragma ACME enable CORRADE_UNUSED
#pragma ACME enable CORRADE_FALLTHROUGH
#pragma ACME enable CORRADE_THREAD_LOCAL
#pragma ACME enable CORRADE_CONSTEXPR14
#pragma ACME enable CORRADE_ALWAYS_INLINE
#pragma ACME enable CORRADE_NEVER_INLINE
#pragma ACME enable CORRADE_ASSUME
#pragma ACME enable CORRADE_LIKELY
#pragma ACME enable CORRADE_UNLIKELY
#pragma ACME enable CORRADE_FUNCTION
#pragma ACME enable CORRADE_LINE_STRING
#pragma ACME enable CORRADE_PASSTHROUGH
#pragma ACME enable CORRADE_NOOP
#pragma ACME enable CORRADE_AUTOMATIC_INITIALIZER
#pragma ACME enable CORRADE_AUTOMATIC_FINALIZER

#pragma ACME revision * echo "$(git describe --long --match 'v*' --abbrev=4) ($(date -d @$(git log -1 --format=%at) +%Y-%m-%d))"
#pragma ACME stats loc wc -l
#pragma ACME stats preprocessed g++ -std=c++11 -P -E -x c++ - | wc -l
