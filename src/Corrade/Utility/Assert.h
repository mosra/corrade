#ifdef CORRADE_ASSERT_INCLUDE
/* Include the user-provided header if desired. Do this before the header guard
   is defined so in case the user-provided header directly or transitively
   includes Assert.h as well, it's not skipped as a whole, causing the assert
   macros to not be defined at all. */
#include CORRADE_ASSERT_INCLUDE
#endif
#ifndef Corrade_Utility_Assert_h
#define Corrade_Utility_Assert_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

/** @file
 * @brief Macro @ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_ASSERT_OUTPUT(), @ref CORRADE_ASSERT_UNREACHABLE(), @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT_EXPRESSION(), @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE(), @ref CORRADE_NO_ASSERT, @ref CORRADE_GRACEFUL_ASSERT, @ref CORRADE_STANDARD_ASSERT, @ref CORRADE_ASSERT_INCLUDE, @ref CORRADE_ASSERT_ABORT(), @ref CORRADE_ASSERT_MESSAGE_ABORT()
 */

#include "Corrade/configure.h"
/* Pull in the dependencies only if there's at least one macro that isn't fully
   defined or overriden yet */
#if !defined(CORRADE_NO_ASSERT) && (!defined(CORRADE_ASSERT) || !defined(CORRADE_CONSTEXPR_ASSERT) || !defined(CORRADE_ASSERT_OUTPUT) || !defined(CORRADE_ASSERT_UNREACHABLE) || !defined(CORRADE_INTERNAL_ASSERT) || !defined(CORRADE_INTERNAL_CONSTEXPR_ASSERT) || !defined(CORRADE_INTERNAL_ASSERT_OUTPUT) || !defined(CORRADE_INTERNAL_ASSERT_EXPRESSION) || !defined(CORRADE_INTERNAL_ASSERT_UNREACHABLE))
#ifndef CORRADE_STANDARD_ASSERT
/* If CORRADE_ASSERT_ABORT() is defined by the user, it's assumed it pulls in
   everything it needs. Otherwise include the std::abort() definition. */
#ifndef CORRADE_ASSERT_ABORT
#include <cstdlib>
#endif
/* Similarly, if CORRADE_ASSERT_MESSAGE_ABORT() is defined, it's assumed to
   pull in Debug if it needs it or not pull it in if not. Have to include it
   with CORRADE_GRACEFUL_ASSERT tho as the asserts are using Error directly. */
#if !defined(CORRADE_ASSERT_MESSAGE_ABORT) || defined(CORRADE_GRACEFUL_ASSERT)
#include "Corrade/Utility/Debug.h"
#endif
#include "Corrade/Utility/Macros.h" /* CORRADE_LINE_STRING */
/* For CORRADE_STANDARD_ASSERT we define the macros to be no-op if NDEBUG is
   set to not need this either */
#elif !defined(NDEBUG)
#include <cassert>
#endif
#endif
/* Utility::forward(), used only by CORRADE_INTERNAL_ASSERT_EXPRESSION */
#if !defined(CORRADE_INTERNAL_ASSERT_EXPRESSION) && !defined(CORRADE_NO_ASSERT) && !(defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#include "Corrade/Utility/Move.h"
#endif

/* There's deliberately no namespace Corrade::Utility in order to avoid noise
   in Corrade Singles */

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief Disable all assertions

This macro is not defined by Corrade, but rather meant to be defined by the
user. When defined, assertions are not checked at all. See documentation of
@ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(),
@ref CORRADE_ASSERT_OUTPUT(), @ref CORRADE_ASSERT_UNREACHABLE(),
@ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(),
@ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT_EXPRESSION()
and @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE() for detailed description of
given macro behavior.
@see @ref CORRADE_GRACEFUL_ASSERT, @ref CORRADE_STANDARD_ASSERT,
    @ref CORRADE_SKIP_IF_NO_ASSERT()
*/
#define CORRADE_NO_ASSERT
#undef CORRADE_NO_ASSERT

/**
@brief Gracefully print assertions

This macro is not defined by Corrade, but rather meant to be defined by the
user. Unlike @ref CORRADE_NO_ASSERT and in case the error output is redirected
(i.e., in a test verifying the assert behavior) this macro checks assertions
and prints a message on error, but calls @cpp return @ce instead of aborting.

@attention
    Meant solely for use in tests to verify assertion behavior and message
    formatting. Is *not recommended* to be enabled in regular code as the early
    returns are likely to cause internal state mismatches, crashes and other
    undefined behavior.

See documentation of @ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(),
@ref CORRADE_ASSERT_OUTPUT() and @ref CORRADE_ASSERT_UNREACHABLE() for detailed
description of given macro behavior. The @ref CORRADE_INTERNAL_ASSERT(),
@ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
@ref CORRADE_INTERNAL_ASSERT_EXPRESSION() and
@ref CORRADE_INTERNAL_ASSERT_UNREACHABLE() are meant to check internal
conditions and thus are not affected by this macro.

When both @ref CORRADE_NO_ASSERT and @ref CORRADE_GRACEFUL_ASSERT are defined,
@ref CORRADE_NO_ASSERT has a precedence. When both @ref CORRADE_STANDARD_ASSERT
and @ref CORRADE_GRACEFUL_ASSERT are defined, @ref CORRADE_STANDARD_ASSERT has
a precedence --- i.e., the assertions *aren't* graceful in that case. This
precedence is reflected also in the @ref CORRADE_SKIP_IF_NO_ASSERT() helper
in the @relativeref{Corrade,TestSuite} library
*/
#define CORRADE_GRACEFUL_ASSERT
#undef CORRADE_GRACEFUL_ASSERT

/**
@brief Use standard assert

This macro is not defined by Corrade, but rather meant to be defined by the
user. This macro causes all  @ref CORRADE_ASSERT(),
@ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_ASSERT_OUTPUT(),
@ref CORRADE_ASSERT_UNREACHABLE(), @ref CORRADE_INTERNAL_ASSERT(),
@ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
@ref CORRADE_INTERNAL_ASSERT_EXPRESSION() and
@ref CORRADE_INTERNAL_ASSERT_UNREACHABLE() to be only wrappers around the
standard @cpp assert() @ce, using just the expression and discarding the
message, if any. This makes them more lightweight, since
@ref Corrade::Utility::Debug does not need to be pulled in, on the other hand
only the failed expression is printed to the output without any human-readable
description. See documentation of a particular assert macro for more
information.

When this macro is defined, @ref CORRADE_NO_ASSERT and the standard
@cpp NDEBUG @ce macro have the same effect.
*/
#define CORRADE_STANDARD_ASSERT
#undef CORRADE_STANDARD_ASSERT

/**
@brief Assertion include
@m_since_latest

This macro is not defined by Corrade, but rather meant to be defined by the
user. If defined, it's used as @cpp #include CORRADE_ASSERT_INCLUDE @ce at the
top of this file and its value has to include the enclosing `<>` or `""`
characters as well. The file can then contain overrides for either the
@ref CORRADE_ASSERT_ABORT() and @ref CORRADE_ASSERT_MESSAGE_ABORT() macros that
affect all asserts or modify just individual macros one by one. The desired use
case is to pass it via compiler flags to override the assert behavior for the
whole project, for example with `-DCORRADE_ASSERT_INCLUDE="<assertOverrides.h>"`
or `-DCORRADE_ASSERT_INCLUDE="\"assertOverrides.h\\""`. Note that different
toolchains and buildsystems may need various workarounds to pass the angle
brackets or inner quotes through.

If you need to also link a certain library in addition to the include, specify
it among linker flags. In case of CMake for example, while adding this define
meant editing `CMAKE_CXX_FLAGS`, specifying the library to link to means
modifying the `CMAKE_SHARED_LINKER_FLAGS` and/or `CMAKE_EXE_LINKER_FLAGS`
variables.
@todoc The \" and \\" in the above are deliberate, Doxygen stupidly eats the
    second backslash otherwise. And putting a double backslash at the front
    makes it shown twice, so it has to be inconsistent like that.
*/
#define CORRADE_ASSERT_INCLUDE
#undef CORRADE_ASSERT_INCLUDE
#endif

/** @hideinitializer
@brief Assertion abort implementation
@m_since_latest

Used by all Corrade assertion macros if @ref CORRADE_STANDARD_ASSERT is not
defined, calls @ref std::abort() by default. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro isn't used as standard @cpp assert() @ce is called instead.

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT_ABORT() @ce before including the
@ref Corrade/Utility/Assert.h header. The macro value is expected to be a
single expression without a trailing semicolon and the called function being
marked as @cpp [[noreturn]] @ce, otherwise any use of
@ref CORRADE_ASSERT_UNREACHABLE() and similar macros may result in the compiler
complaining that not all code paths return a value. Also note that if this
macro is overriden, the header doesn't @cpp #include <cstdlib> @ce for
@ref std::abort() anymore, you have to do that yourself if needed. See also
@ref CORRADE_ASSERT_MESSAGE_ABORT() for a way to override both the message
printing and the abort.

Example usage, assuming the @cpp abortWithBacktrace() @ce function is defined
somewhere else:

@snippet Utility.cpp CORRADE_ASSERT_ABORT

Alternatively you can put the @cpp #define CORRADE_ASSERT_ABORT() @ce macro
into a  dedicated header file and pass e.g.
`-DCORRADE_ASSERT_INCLUDE="\"abortWithBacktrace.h\\""` in compiler flags to
override the abort behavior for the whole project. See
@ref CORRADE_ASSERT_INCLUDE for more information.
@todoc The \" and \\" in the above are deliberate, Doxygen stupidly eats the
    second backslash otherwise. And putting a double backslash at the front
    makes it shown twice, so it has to be inconsistent like that.
*/
#ifndef CORRADE_ASSERT_ABORT
#define CORRADE_ASSERT_ABORT() std::abort()
#endif

/** @hideinitializer
@brief Assertion message and abort implementation
@m_since_latest

Used by all Corrade assertion macros if neither @ref CORRADE_STANDARD_ASSERT
nor @ref CORRADE_GRACEFUL_ASSERT is defined. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro isn't used as standard @cpp assert() @ce is called instead,
if @ref CORRADE_GRACEFUL_ASSERT is defined, the message printing cannot be
overriden, only abort behavior.

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT_MESSAGE_ABORT(...) @ce before including the
@ref Corrade/Utility/Assert.h header. The macro value is expected to be a
sequence of expressions each with a trailing semicolon. Note that if this macro
is overriden, the header doesn't @cpp #include <Corrade/Utility/Debug.h> @ce
for @relativeref{Corrade,Utility::Debug} anymore, you have to do that yourself
if needed. You can also override just @ref CORRADE_ASSERT_ABORT() if you only
need to control the abort behavior but not message printing.

Example usage, assuming the @cpp logAndReportAssertion() @ce function is
defined somewhere else. The override formats the message to a string, passes it
to this function but still also prints it to @relativeref{Corrade,Utility::Error}
to have it shown in the standard error output as well, and delegates to
@ref CORRADE_ASSERT_ABORT() at the end. You can do any abort-like operation
instead of calling that macro, but in that case it's recommended to override
the @ref CORRADE_ASSERT_ABORT() implementation directly, as it will affect all
such cases in that case, not just aborts with messages.

@snippet Utility.cpp CORRADE_ASSERT_MESSAGE_ABORT

Alternatively you can put the @cpp #define CORRADE_ASSERT_MESSAGE_ABORT(...) @ce
macro into a  dedicated header file and pass e.g.
`-DCORRADE_ASSERT_INCLUDE="\"logAndReportAssertion.h\\""` in compiler flags to
override the abort behavior for the whole project. See
@ref CORRADE_ASSERT_INCLUDE for more information.
@todoc The \" and \\" in the above are deliberate, Doxygen stupidly eats the
    second backslash otherwise. And putting a double backslash at the front
    makes it shown twice, so it has to be inconsistent like that.
*/
#ifndef CORRADE_ASSERT_MESSAGE_ABORT
#define CORRADE_ASSERT_MESSAGE_ABORT(...)                                   \
    Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << __VA_ARGS__; \
    CORRADE_ASSERT_ABORT();
#endif

/** @hideinitializer
@brief Assertion macro
@param condition    Assert condition
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

Usable for sanity checks on user input, as it prints explanational message on
error.

By default, if assertion fails, @p message is printed to error output and the
application aborts. If @ref CORRADE_GRACEFUL_ASSERT is defined *and*
@ref Corrade::Utility::Error output is redirected (i.e., in tests verifying the
assert behavior), the message is printed and the function returns with
@p returnValue instead of aborting. If @ref CORRADE_STANDARD_ASSERT is defined,
this macro expands to @cpp assert(condition) @ce, ignoring @p message. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro expands to @cpp do {} while(false) @ce.
Example usage:

@snippet Utility.cpp CORRADE_ASSERT

If the function has return type @cpp void @ce, just use an empty parameter
(allowed in C++11):

@snippet Utility.cpp CORRADE_ASSERT-void

You can use stream output operators for formatting just like when printing to
@ref Corrade::Utility::Debug output:

@snippet Utility.cpp CORRADE_ASSERT-stream

<b></b>

@m_class{m-block m-warning}

@par Problematic use cases
    Don't use this function for checking function output like below --- if
    @ref CORRADE_NO_ASSERT is defined, the macro is not expanded and thus the
    function gets never called. Use @ref CORRADE_ASSERT_OUTPUT() instead.
@par
    @snippet Utility.cpp CORRADE_ASSERT-output
@par
    Similarly, this macro shouldn't be used for asserting on unreachable code
    --- if @ref CORRADE_NO_ASSERT is defined, there's nothing left to tell the
    compiler this code is unreachable, potentially producing a compile error
    due to a missing @cpp return @ce. In this case it's better to use
    @ref CORRADE_ASSERT_UNREACHABLE() instead, which will emit a corresponding
    compiler hint in all cases.
@par
    @snippet Utility.cpp CORRADE_ASSERT-unreachable

<b></b>

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_DEBUG_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_ASSERT(condition, message, returnValue) do {} while(false)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT(condition, message, returnValue) assert(condition)
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error{} << message;                           \
            if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) \
                CORRADE_ASSERT_ABORT();                                     \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#else
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            CORRADE_ASSERT_MESSAGE_ABORT(message)                           \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#endif
#endif

/** @hideinitializer
@brief Constexpr assertion macro
@param condition    Assert condition
@param message      Message on assertion fail

Unlike @ref CORRADE_ASSERT() this macro can be used in C++11
@cpp constexpr @ce functions like this:

@snippet Utility.cpp CORRADE_CONSTEXPR_ASSERT

In a @cpp constexpr @ce context, if assertion fails, the code fails to compile.
In a non-@cpp constexpr @ce context, if assertion fails, @p message is printed
to error output and the application aborts. If @ref CORRADE_GRACEFUL_ASSERT is
defined *and* @ref Corrade::Utility::Error output is redirected (i.e., in tests
verifying the assert behavior), the message is printed and the rest of the
function gets executed as usual instead of aborting. If
@ref CORRADE_STANDARD_ASSERT is defined, @p message is ignored and the standard
@cpp assert() @ce is called if @p condition fails. If @ref CORRADE_NO_ASSERT is
defined (or if both @ref CORRADE_STANDARD_ASSERT and @cpp NDEBUG @ce are
defined), this macro expands to @cpp static_cast<void>(0) @ce.

As with @ref CORRADE_ASSERT(), you can use stream output operators for
formatting just like when printing to @ref Corrade::Utility::Debug output.

The implementation is based on the [Asserts in constexpr functions](https://akrzemi1.wordpress.com/2017/05/18/asserts-in-constexpr-functions/)
article by Andrzej Krzemieński and the followup discussion.

You can override this implementation by placing your own
@cpp #define CORRADE_CONSTEXPR_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_CONSTEXPR_DEBUG_ASSERT(),
    @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT()
*/
#ifndef CORRADE_CONSTEXPR_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_CONSTEXPR_ASSERT(condition, message) static_cast<void>(0)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        assert(!#condition);                                                \
    }(), 0))
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error{} << message;                               \
        if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) \
            CORRADE_ASSERT_ABORT();                                         \
    }(), 0))
#else
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        CORRADE_ASSERT_MESSAGE_ABORT(message)                               \
    }(), 0))
#endif
#endif

/** @hideinitializer
@brief Call output assertion macro
@param call         Assert call
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

Unlike @ref CORRADE_ASSERT(), this macro performs the call even if
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), making it usable for checking function output.
Otherwise the behavior is the same as with @ref CORRADE_ASSERT(). Example
usage:

@snippet Utility.cpp CORRADE_ASSERT_OUTPUT

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT_OUTPUT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_DEBUG_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
    @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT_OUTPUT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    static_cast<void>(call)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue) assert(call)
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error{} << message;                           \
            if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) \
                CORRADE_ASSERT_ABORT();                                     \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#else
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            CORRADE_ASSERT_MESSAGE_ABORT(message)                           \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#endif
#endif

/** @hideinitializer
@brief Assert that the code is unreachable
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

By default, if code marked with this macro is reached, @p message is printed to
error output and the application aborts. If @ref CORRADE_GRACEFUL_ASSERT is
defined *and* @ref Corrade::Utility::Error output is redirected (i.e., in tests
verifying the assert behavior), the message is printed and the function returns
with @p returnValue instead of aborting. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro expands to @cpp assert(!"unreachable code") @ce. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro hints to the compiler that given code
is not reachable, possibly helping the optimizer (using a compiler builtin on
GCC, Clang and MSVC; calling @ref CORRADE_ASSERT_ABORT(), which is
@ref std::abort() by default, otherwise). A @cpp return @ce statement can thus
be safely omitted in a code path following this macro without causing any
compiler warnings or errors. Example usage:

@snippet Utility.cpp CORRADE_ASSERT_UNREACHABLE

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT_UNREACHABLE @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_DEBUG_ASSERT_UNREACHABLE(),
    @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE(), @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT_UNREACHABLE
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#ifdef CORRADE_TARGET_GCC
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) __builtin_unreachable()
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) __assume(0)
#else
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) CORRADE_ASSERT_ABORT()
#endif
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) assert(!"unreachable code")
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue)                    \
    do {                                                                    \
        Corrade::Utility::Error{} << message;                               \
        if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) \
            CORRADE_ASSERT_ABORT();                                         \
        return returnValue;                                                 \
    } while(false)
#else
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue)                                        \
    do {                                                                    \
        CORRADE_ASSERT_MESSAGE_ABORT(message)                               \
    } while(false)
#endif
#endif

/** @hideinitializer
@brief Internal assertion macro
@param condition    Assert condition

Unlike @ref CORRADE_ASSERT() usable for sanity checks on internal state, as it
prints what failed and where instead of a user-friendly message.

By default, if assertion fails, failed condition, file and line is printed to
error output and the application aborts. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro expands to @cpp assert(condition) @ce. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro expands to @cpp do {} while(false) @ce.
Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT

<b></b>

@m_class{m-block m-warning}

@par Problematic use cases
    Don't use this function for checking function output like below --- if
    @ref CORRADE_NO_ASSERT is defined, the macro is not expanded and thus
    the function gets never called. Use @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
    instead.
@par
    @snippet Utility.cpp CORRADE_INTERNAL_ASSERT-output
@par
    Similarly, this macro shouldn't be used for asserting on unreachable code
    --- if @ref CORRADE_NO_ASSERT is defined, there's nothing left to tell the
    compiler this code is unreachable, potentially producing a compile error
    due to a missing @cpp return @ce. In this case it's better to use
    @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE() instead, which will emit a
    corresponding compiler hint in all cases.
@par
    @snippet Utility.cpp CORRADE_INTERNAL_ASSERT-unreachable

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_INTERNAL_DEBUG_ASSERT(),
    @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(),
    @ref CORRADE_ASSERT_UNREACHABLE(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_INTERNAL_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_INTERNAL_ASSERT(condition) do {} while(false)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT(condition) assert(condition)
#else
#define CORRADE_INTERNAL_ASSERT(condition)                                  \
    do {                                                                    \
        if(!(condition)) {                                                  \
            CORRADE_ASSERT_MESSAGE_ABORT("Assertion " #condition " failed at " __FILE__ ":" CORRADE_LINE_STRING) \
        }                                                                   \
    } while(false)
#endif
#endif

/** @hideinitializer
@brief Internal constexpr assertion macro
@param condition    Assert condition

Unlike @ref CORRADE_INTERNAL_ASSERT() this macro can be used in C++11
@cpp constexpr @ce functions like this:

@snippet Utility.cpp CORRADE_INTERNAL_CONSTEXPR_ASSERT

In a @cpp constexpr @ce context, if assertion fails, the code fails to compile.
In a non-@cpp constexpr @ce context, if assertion fails, failed condition, file
and line is printed to error output and the application aborts. If
@ref CORRADE_STANDARD_ASSERT is defined, the standard @cpp assert() @ce is
called if @p condition fails. If @ref CORRADE_NO_ASSERT is defined (or if both
@ref CORRADE_STANDARD_ASSERT and @cpp NDEBUG @ce are defined), this macro
expands to @cpp static_cast<void>(0) @ce.

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_CONSTEXPR_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_INTERNAL_CONSTEXPR_DEBUG_ASSERT(),
    @ref CORRADE_CONSTEXPR_ASSERT()
*/
#ifndef CORRADE_INTERNAL_CONSTEXPR_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_INTERNAL_CONSTEXPR_ASSERT(condition) static_cast<void>(0)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_CONSTEXPR_ASSERT(condition)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        assert(!#condition);                                                \
    }(), 0))
#else
#define CORRADE_INTERNAL_CONSTEXPR_ASSERT(condition)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        CORRADE_ASSERT_MESSAGE_ABORT("Assertion " #condition " failed at " __FILE__ ":" CORRADE_LINE_STRING) \
    }(), 0))
#endif
#endif

/** @hideinitializer
@brief Internal call output assertion macro
@param call         Assert call

Unlike @ref CORRADE_INTERNAL_ASSERT(), this macro performs the call even if
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), making it usable for checking function output.
Otherwise the behavior is the same as with @ref CORRADE_INTERNAL_ASSERT().
Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_OUTPUT

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT_OUTPUT @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT(),
    @ref CORRADE_INTERNAL_ASSERT_EXPRESSION()
*/
#ifndef CORRADE_INTERNAL_ASSERT_OUTPUT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    static_cast<void>(call)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call) assert(call)
#else
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    do {                                                                    \
        if(!(call)) {                                                       \
            CORRADE_ASSERT_MESSAGE_ABORT("Assertion " #call " failed at " __FILE__ ":" CORRADE_LINE_STRING) \
        }                                                                   \
    } while(false)
#endif
#endif

#if !defined(CORRADE_INTERNAL_ASSERT_EXPRESSION) && !defined(CORRADE_NO_ASSERT) && !(defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
namespace Corrade { namespace Utility { namespace Implementation {
    #ifdef CORRADE_STANDARD_ASSERT
    template<class T> T assertExpression(T&& value) {
        assert(value);
        return Corrade::Utility::forward<T>(value);
    }
    #else
    template<class T> T assertExpression(T&& value, const char* message) {
        if(!value) {
            CORRADE_ASSERT_MESSAGE_ABORT(message)
        }

        return Corrade::Utility::forward<T>(value);
    }
    #endif
}}}
#endif

/** @hideinitializer
@brief Internal expression assertion macro
@m_since_latest

A variant of @ref CORRADE_INTERNAL_ASSERT_OUTPUT() that can be used inside
expressions. Useful in cases where creating a temporary just for the assertion
would be too inconvenient --- for example, the following code, which uses
@ref CORRADE_INTERNAL_ASSERT_OUTPUT() to check that the file was read
correctly:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_EXPRESSION-without

Could be rewritten in a shorter way and without having to use
@m_class{m-doc-external} [std::move()](https://en.cppreference.com/w/cpp/utility/move)
to pass a r-value with @ref CORRADE_INTERNAL_ASSERT_EXPRESSION():

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_EXPRESSION

The macro passes the expression to a function which asserts it evaluates to
@cpp true @ce and then returns the value forwarded. That implies the expression
result type has to be at least movable. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro uses @cpp assert(value) @ce inside, unfortunately it's not
possible for the standard assert macro to show the expression. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro expands to nothing, leaving just the
parenthesized expression out of it.

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT_EXPRESSION @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_INTERNAL_DEBUG_ASSERT_EXPRESSION()

@todo In C++14 this could use an inline templated lambda, which means we could
    drop the template function, do it inline and *also* make the standard
    assert actually working.
*/
#ifndef CORRADE_INTERNAL_ASSERT_EXPRESSION
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
/* Not (...) __VA_ARGS__, because the parentheses may be important */
#define CORRADE_INTERNAL_ASSERT_EXPRESSION
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT_EXPRESSION(...) Corrade::Utility::Implementation::assertExpression(__VA_ARGS__)
#else
#define CORRADE_INTERNAL_ASSERT_EXPRESSION(...) Corrade::Utility::Implementation::assertExpression(__VA_ARGS__, "Assertion " #__VA_ARGS__ " failed at " __FILE__ ":" CORRADE_LINE_STRING)
#endif
#endif

/** @hideinitializer
@brief Internal assert that the code is unreachable
@m_since{2020,06}

Compared to @ref CORRADE_ASSERT_UNREACHABLE(), usable for sanity checks on
internal state, as it prints what failed and where instead of a user-friendly
message.

By default, if code marked with this macro is reached, message with file and
line is printed to error output and the application aborts. If
@ref CORRADE_STANDARD_ASSERT is defined, this macro expands to
@cpp assert(!"unreachable code") @ce. If @ref CORRADE_NO_ASSERT is defined (or
if both @ref CORRADE_STANDARD_ASSERT and @cpp NDEBUG @ce are defined), this
macro hints to the compiler that given code is not reachable, possibly helping
the optimizer (using a compiler builtin on GCC, Clang and MSVC; calling
@ref CORRADE_ASSERT_ABORT(), which is @ref std::abort() by default, otherwise).
A @cpp return @ce statement can thus be safely omitted in a code path following
this macro without causing any compiler warnings or errors. Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_UNREACHABLE

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT_UNREACHABLE @ce before including the
@ref Corrade/Utility/Assert.h header. See also @ref CORRADE_ASSERT_ABORT(),
@ref CORRADE_ASSERT_MESSAGE_ABORT() and @ref CORRADE_ASSERT_INCLUDE for a way
to override behavior for all assertions at once.
@see @ref CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE()
*/
#ifndef CORRADE_INTERNAL_ASSERT_UNREACHABLE
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#ifdef CORRADE_TARGET_GCC
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() __builtin_unreachable()
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() __assume(0)
#else
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() CORRADE_ASSERT_ABORT()
#endif
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() assert(!"unreachable code")
#else
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE()                                        \
    do {                                                                    \
        CORRADE_ASSERT_MESSAGE_ABORT("Reached unreachable code at " __FILE__ ":" CORRADE_LINE_STRING) \
    } while(false)
#endif
#endif

#endif
