#ifndef Corrade_Utility_Assert_h
#define Corrade_Utility_Assert_h
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
 * @brief Macro @ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_ASSERT_UNREACHABLE(), @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE(), @ref CORRADE_ASSUME(), @ref CORRADE_NO_ASSERT, @ref CORRADE_GRACEFUL_ASSERT, @ref CORRADE_STANDARD_ASSERT
 */

#include "Corrade/configure.h"
#if !defined(CORRADE_NO_ASSERT) && (!defined(CORRADE_ASSERT) || !defined(CORRADE_CONSTEXPR_ASSERT) || !defined(CORRADE_ASSERT_OUTPUT) || !defined(CORRADE_ASSERT_UNREACHABLE) || !defined(CORRADE_INTERNAL_ASSERT) || !defined(CORRADE_INTERNAL_CONSTEXPR_ASSERT) || !defined(CORRADE_INTERNAL_ASSERT_OUTPUT) || !defined(CORRADE_INTERNAL_ASSERT_UNREACHABLE) || !defined(CORRADE_ASSUME))
#ifndef CORRADE_STANDARD_ASSERT
#include <cstdlib>

#include "Corrade/Utility/Debug.h"
#elif !defined(NDEBUG)
#include <cassert>
#endif
#endif

namespace Corrade { namespace Utility {

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief Disable all assertions

This macro is not defined by Corrade, but rather meant to be defined by the
user. When defined, assertions are not checked at all. See documentation of
@ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(),
@ref CORRADE_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT(),
@ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
and @ref CORRADE_ASSERT_UNREACHABLE() for detailed description of given macro
behavior.
@see @ref CORRADE_GRACEFUL_ASSERT, @ref CORRADE_STANDARD_ASSERT
*/
#define CORRADE_NO_ASSERT
#undef CORRADE_NO_ASSERT

/**
@brief Gracefully print assertions

This macro is not defined by Corrade, but rather meant to be defined by the
user. Unlike @ref CORRADE_NO_ASSERT and in case the error output is redirected
(i.e., in a test verifying the assert behavior) this macro checks assertions
and prints a message on error, but does not call @ref std::abort(). Useful for
testing assertion behavior. See documentation of @ref CORRADE_ASSERT(),
@ref CORRADE_CONSTEXPR_ASSERT() and @ref CORRADE_ASSERT_OUTPUT() for detailed
description of given macro behavior. The @ref CORRADE_INTERNAL_ASSERT(),
@ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
and @ref CORRADE_ASSERT_UNREACHABLE() are meant to check internal conditions
and thus are not affected by this macro.

When both @ref CORRADE_NO_ASSERT and @ref CORRADE_GRACEFUL_ASSERT are defined,
@ref CORRADE_NO_ASSERT has a precedence.
*/
#define CORRADE_GRACEFUL_ASSERT
#undef CORRADE_GRACEFUL_ASSERT

/**
@brief Use standard assert

This macro is not defined by Corrade, but rather meant to be defined by the
user. This macro causes all  @ref CORRADE_ASSERT(),
@ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_ASSERT_OUTPUT(),
@ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(),
@ref CORRADE_INTERNAL_ASSERT_OUTPUT() and @ref CORRADE_ASSERT_UNREACHABLE() to
be only wrappers around the standard @cpp assert() @ce, using just the
expression and discarding the message, if any. This makes them more
lightweight, since @ref Corrade::Utility::Debug does not need to be pulled in,
on the other hand only the failed expression is printed to the output without
any human-readable description. See documentation of a particular assert macro
for more information.

When this macro is defined, @ref CORRADE_NO_ASSERT and the standard
@cpp NDEBUG @ce macro have the same effect.
*/
#define CORRADE_STANDARD_ASSERT
#undef CORRADE_STANDARD_ASSERT
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
this macro compiles to @cpp assert(condition) @ce, ignoring @p message. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro compiles to @cpp do {} while(0) @ce.
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

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT(),
    @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_ASSERT(condition, message, returnValue) do {} while(0)
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error{} << message;                           \
            if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) std::abort(); \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT(condition, message, returnValue) assert(condition)
#else
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << message; \
            std::abort();                                                   \
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
defined), this macro compiles to @cpp static_cast<void>(0) @ce.

As with @ref CORRADE_ASSERT(), you can use stream output operators for
formatting just like when printing to @ref Corrade::Utility::Debug output.

The implementation is based on the [Asserts in constexpr functions](https://akrzemi1.wordpress.com/2017/05/18/asserts-in-constexpr-functions/)
article by Andrzej Krzemieński and the followup discussion.

You can override this implementation by placing your own
@cpp #define CORRADE_CONSTEXPR_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_CONSTEXPR_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_CONSTEXPR_ASSERT(condition, message) static_cast<void>(0)
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error{} << message;                               \
        if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) std::abort(); \
    }(), 0))
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        assert(!#condition);                                                \
    }(), 0))
#else
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << message; \
        std::abort();                                                       \
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
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT_OUTPUT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    static_cast<void>(call)
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error{} << message;                           \
            if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) std::abort(); \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue) assert(call)
#else
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << message; \
            std::abort();                                                   \
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
defined, this macro compiles to @cpp assert(!"unreachable code") @ce. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro hints to the compiler that given code
is not reachable, possibly helping the optimizer (using a compiler builtin on
GCC, Clang and MSVC; calling @ref std::abort() otherwise). Example usage:

@snippet Utility.cpp CORRADE_ASSERT_UNREACHABLE

You can override this implementation by placing your own
@cpp #define CORRADE_ASSERT_UNREACHABLE @ce before including the
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_INTERNAL_ASSERT_UNREACHABLE(), @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_ASSERT_UNREACHABLE
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#if defined(__GNUC__)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) __builtin_unreachable()
#elif defined(_MSC_VER)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) __assume(0)
#else
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) std::abort()
#endif
#elif defined(CORRADE_GRACEFUL_ASSERT)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue)                    \
    do {                                                                    \
        Corrade::Utility::Error{} << message;                               \
        if(Corrade::Utility::Error::defaultOutput() == Corrade::Utility::Error::output()) std::abort(); \
        return returnValue;                                                 \
    } while(false)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue) assert(!"unreachable code")
#else
#define CORRADE_ASSERT_UNREACHABLE(message, returnValue)                                        \
    do {                                                                    \
        Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << message; \
        std::abort();                                                       \
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
defined, this macro compiles to @cpp assert(condition) @ce. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro compiles to @cpp do {} while(0) @ce.
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
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(),
    @ref CORRADE_ASSERT_UNREACHABLE(), @ref CORRADE_ASSUME()
*/
#ifndef CORRADE_INTERNAL_ASSERT
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#define CORRADE_INTERNAL_ASSERT(condition) do {} while(0)
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT(condition) assert(condition)
#else
#define CORRADE_INTERNAL_ASSERT(condition)                                  \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << "Assertion " #condition " failed at " __FILE__ ":" CORRADE_LINE_STRING; \
            std::abort();                                                   \
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
compiles to @cpp static_cast<void>(0) @ce.

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_CONSTEXPR_ASSERT @ce before including the
@ref Corrade/Utility/Assert.h header.
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
        Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << "Assertion " #condition " failed at " __FILE__ ":" CORRADE_LINE_STRING; \
        std::abort();                                                       \
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
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_INTERNAL_ASSERT_EXPRESSION()
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
            Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << "Assertion " #call " failed at " __FILE__ ":" CORRADE_LINE_STRING; \
            std::abort();                                                   \
        }                                                                   \
    } while(false)
#endif
#endif

#if !defined(CORRADE_INTERNAL_ASSERT_EXPRESSION) && !defined(CORRADE_NO_ASSERT) && !(defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
namespace Implementation {
    #ifdef CORRADE_STANDARD_ASSERT
    template<class T> T assertExpression(T&& value) {
        assert(value);
        return std::forward<T>(value);
    }
    #else
    template<class T> T assertExpression(T&& value, const char* message) {
        if(!value) {
            Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << message;
            std::abort();
        }

        return std::forward<T>(value);
    }
    #endif
}
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

Could be rewritten in a shorter way and without having to use @cpp std::move() @ce
to pass a r-value with @ref CORRADE_INTERNAL_ASSERT_EXPRESSION():

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_EXPRESSION

The macro passes the expression to a function which asserts it evaluates to
@cpp true @ce and then returns the value forwarded. That implies the expression
result type has to be at least movable. If @ref CORRADE_STANDARD_ASSERT is
defined, this macro uses @cpp assert(value) @ce inside, unfortunately it's not
possible for the standard assert macro to show the expression. If
@ref CORRADE_NO_ASSERT is defined (or if both @ref CORRADE_STANDARD_ASSERT and
@cpp NDEBUG @ce are defined), this macro compiles to nothing, leaving just the
parenthesized expression out of it.

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT_EXPRESSION @ce before including the
@ref Corrade/Utility/Assert.h header.

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
@ref CORRADE_STANDARD_ASSERT is defined, this macro compiles to
@cpp assert(!"unreachable code") @ce. If @ref CORRADE_NO_ASSERT is defined (or
if both @ref CORRADE_STANDARD_ASSERT and @cpp NDEBUG @ce are defined), this
macro hints to the compiler that given code is not reachable, possibly helping
the optimizer (using a compiler builtin on GCC, Clang and MSVC; calling
@ref std::abort() otherwise). Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_UNREACHABLE

You can override this implementation by placing your own
@cpp #define CORRADE_INTERNAL_ASSERT_UNREACHABLE @ce before including the
@ref Corrade/Utility/Assert.h header.
*/
#ifndef CORRADE_INTERNAL_ASSERT_UNREACHABLE
#if defined(CORRADE_NO_ASSERT) || (defined(CORRADE_STANDARD_ASSERT) && defined(NDEBUG))
#if defined(__GNUC__)
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() __assume(0)
#else
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() std::abort()
#endif
#elif defined(CORRADE_STANDARD_ASSERT)
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE() assert(!"unreachable code")
#else
#define CORRADE_INTERNAL_ASSERT_UNREACHABLE()                                        \
    do {                                                                    \
        Corrade::Utility::Error{Corrade::Utility::Error::defaultOutput()} << "Reached unreachable code at " __FILE__ ":" CORRADE_LINE_STRING; \
        std::abort();                                                       \
    } while(false)
#endif
#endif

/** @hideinitializer
@brief Assume a condition
@m_since{2020,06}

Compared to @ref CORRADE_INTERNAL_ASSERT() this macro does not handle the case
when the condition isn't @cpp true @ce in any way --- only provides a hint to
the compiler, possibly improving performance. Uses a compiler builtin on GCC,
Clang and MSVC; expands to an empty @cpp do while @ce otherwise. Example usage:

@snippet Utility.cpp CORRADE_ASSUME

You can override this implementation by placing your own
@cpp #define CORRADE_ASSUME @ce before including the
@ref Corrade/Utility/Assert.h header.

@see @ref CORRADE_ASSERT(), @ref CORRADE_ASSERT_UNREACHABLE()
*/
#ifndef CORRADE_ASSUME
#ifdef __clang__
#define CORRADE_ASSUME(condition) __builtin_assume(condition)
#elif defined(_MSC_VER)
#define CORRADE_ASSUME(condition) __assume(condition)
#elif defined(__GNUC__)
#define CORRADE_ASSUME(condition)                                           \
    do {                                                                    \
        if(!(condition)) __builtin_unreachable(); \
    } while(false)
#else
#define CORRADE_ASSUME(condition) do {} while(false)
#endif
#endif

}}

#endif
