#ifndef Corrade_Utility_Assert_h
#define Corrade_Utility_Assert_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Macro @ref CORRADE_ASSERT(), @ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_ASSERT_OUTPUT(), @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_ASSERT_UNREACHABLE()
 */

#if !defined(CORRADE_NO_ASSERT) || defined(CORRADE_GRACEFUL_ASSERT)
#include <cstdlib>

#include "Corrade/Utility/Debug.h"
#endif

/** @hideinitializer
@brief Assertion macro
@param condition    Assert condition
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

Usable for sanity checks on user input, as it prints explanational message on
error.

By default, if assertion fails, @p message is printed to error output and the
application aborts. If @cpp CORRADE_GRACEFUL_ASSERT @ce is defined, the message
is printed and the function returns with @p returnValue. If
@cpp CORRADE_NO_ASSERT @ce is defined, this macro compiles to
@cpp do {} while(0) @ce. Example usage:

@snippet Utility.cpp CORRADE_ASSERT

If the function has return type @cpp void @ce, just use an empty parameter
(allowed in C++11):

@snippet Utility.cpp CORRADE_ASSERT-void

You can use stream output operators for formatting just like when printing to
@ref Corrade::Utility::Debug output:

@snippet Utility.cpp CORRADE_ASSERT-stream

@attention
    Don't use this function for checking function output like this:
@attention
    @snippet Utility.cpp CORRADE_ASSERT-output
@attention
    If @cpp CORRADE_NO_ASSERT @ce is defined, the macro is not expanded and
    thus the function gets never called. See @ref CORRADE_ASSERT_OUTPUT() for a
    possible solution.

@see @ref CORRADE_CONSTEXPR_ASSERT(), @ref CORRADE_INTERNAL_ASSERT(),
    @ref CORRADE_ASSERT_UNREACHABLE()
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << message;                           \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#elif defined(CORRADE_NO_ASSERT)
#define CORRADE_ASSERT(condition, message, returnValue) do {} while(0)
#else
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << message;                           \
            std::abort();                                                   \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
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
to error output and the application aborts. If @cpp CORRADE_GRACEFUL_ASSERT @ce
is defined, the message is printed and the rest of the function gets executed
as usual. If @cpp CORRADE_NO_ASSERT @ce is defined, this macro compiles to
@cpp static_cast<void>(0) @ce.

As with @ref CORRADE_ASSERT(), you can use stream output operators for
formatting just like when printing to @ref Corrade::Utility::Debug output.

@see @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT()
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error{} << message;                               \
    }(), 0))
#elif defined(CORRADE_NO_ASSERT)
#define CORRADE_CONSTEXPR_ASSERT(condition, message) static_cast<void>(0)
#else
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error{} << message;                               \
        std::abort();                                                       \
    }(), 0))
#endif

/** @hideinitializer
@brief Call output assertion macro
@param call         Assert call
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

Unlike @ref CORRADE_ASSERT(), this macro performs the call even if
@cpp CORRADE_NO_ASSERT @ce is defined, making it usable for checking function
output. Otherwise the behavior is the same as with @ref CORRADE_ASSERT().
Example usage:

@snippet Utility.cpp CORRADE_ASSERT_OUTPUT

@see @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error{} << message;                           \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#elif defined(CORRADE_NO_ASSERT)
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    static_cast<void>(call)
#else
#define CORRADE_ASSERT_OUTPUT(call, message, returnValue)                   \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error{} << message;                           \
            std::abort();                                                   \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#endif

/** @hideinitializer
@brief Internal assertion macro
@param condition    Assert condition

Unlike @ref CORRADE_ASSERT() usable for sanity checks on internal state, as it
prints what failed and where instead of a user-friendly message.

By default, if assertion fails, failed condition, file and line is printed to
error output and the application aborts. If @cpp CORRADE_NO_ASSERT @ce is
defined, this macro compiles to @cpp do {} while(0) @ce. Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT

@attention
    Don't use this function for checking function output like this:
@attention
    @snippet Utility.cpp CORRADE_INTERNAL_ASSERT-output
@attention
    If `CORRADE_NO_ASSERT` is defined, the macro is not expanded and thus the
    function gets never called. Use @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
    instead.

@see @ref CORRADE_INTERNAL_CONSTEXPR_ASSERT(),
    @ref CORRADE_ASSERT_UNREACHABLE()
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_ASSERT(condition) do {} while(0)
#else
#define CORRADE_INTERNAL_ASSERT(condition)                                  \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << "Assertion " #condition " failed in " __FILE__ " on line" << __LINE__; \
            std::abort();                                                   \
        }                                                                   \
    } while(false)
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
@cpp CORRADE_NO_ASSERT @ce is defined, this macro compiles to
@cpp static_cast<void>(0) @ce.
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_CONSTEXPR_ASSERT(condition) static_cast<void>(0)
#else
#define CORRADE_INTERNAL_CONSTEXPR_ASSERT(condition)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        Corrade::Utility::Error() << "Assertion " #condition " failed in " __FILE__ " on line" << __LINE__; \
        std::abort();                                                       \
    }(), 0))
#endif

/** @hideinitializer
@brief Internal call output assertion macro
@param call         Assert call

Unlike @ref CORRADE_INTERNAL_ASSERT(), this macro performs the call even if
@cpp CORRADE_NO_ASSERT @ce is defined, making it usable for checking function
output. Otherwise the behavior is the same as with @ref CORRADE_INTERNAL_ASSERT().
Example usage:

@snippet Utility.cpp CORRADE_INTERNAL_ASSERT_OUTPUT
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    static_cast<void>(call)
#else
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error() << "Assertion " #call " failed in " __FILE__ " on line" << __LINE__; \
            std::abort();                                                   \
        }                                                                   \
    } while(false)
#endif

/** @hideinitializer
@brief Assert that the following code is unreachable

By default, if code marked with this macro is reached, message with file and
line is printed to error output and the application aborts. If
@cpp CORRADE_NO_ASSERT @ce is defined, this macro hints to the compiler that
given code is not reachable, possibly improving performance. Example usage:

@snippet Utility.cpp CORRADE_ASSERT_UNREACHABLE

@see @ref CORRADE_ASSERT(), @ref CORRADE_INTERNAL_ASSERT()
*/
#ifdef CORRADE_NO_ASSERT
#if defined(__GNUC__)
#define CORRADE_ASSERT_UNREACHABLE() __builtin_unreachable()
#else
#define CORRADE_ASSERT_UNREACHABLE() std::abort()
#endif
#else
#define CORRADE_ASSERT_UNREACHABLE()                                        \
    do {                                                                    \
        Corrade::Utility::Error() << "Reached unreachable code in " __FILE__ " on line" << __LINE__; \
        std::abort();                                                       \
    } while(false)
#endif

#endif
