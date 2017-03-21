#ifndef Corrade_Utility_Assert_h
#define Corrade_Utility_Assert_h
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
 * @brief Macro @ref CORRADE_ASSERT(), @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(), @ref CORRADE_ASSERT_UNREACHABLE()
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
application aborts. If `CORRADE_GRACEFUL_ASSERT` is defined, the message is
printed and the function returns with @p returnValue. If `CORRADE_NO_ASSERT` is
defined, this macro does nothing. Example usage:
@code
T operator[](std::size_t pos) const {
    CORRADE_ASSERT(pos < size(), "Index out of range", T());
    return data[pos];
}
@endcode

If the function has return type `void`, just use empty parameter (allowed in
C++11):
@code
void compile() {
    CORRADE_ASSERT(!sources.empty(), "No sources added", );

    // ...
}
@endcode

You can use stream output operators for formatting just like when printing to
Debug output:
@code
CORRADE_ASSERT(pos < size(), "Cannot access element" << pos << "in array of size" << size(), );
@endcode

@attention Don't use this function for checking function output like this:
    @code
    CORRADE_ASSERT(initialize(userParam), "Initialization failed: wrong parameter" << userParam, );
    @endcode
    If `CORRADE_NO_ASSERT` is defined, the macro is not expanded and thus the
    function gets never called. See CORRADE_INTERNAL_ASSERT_OUTPUT() for
    possible solution.

@see @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_ASSERT_UNREACHABLE()
@todo find a way that @p returnValue gets validated but doesn't get included in
    the code unless `CORRADE_GRACEFUL_ASSERT` is used
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << message;                           \
            return returnValue;                                             \
        }                                                                   \
    } while(false)
#else
#ifdef CORRADE_NO_ASSERT
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
#endif

/** @hideinitializer
@brief Internal assertion macro
@param condition    Assert condition

Unlike @ref CORRADE_ASSERT() usable for sanity checks on internal state, as it
prints what failed and where instead of user-friendly message.

By default, if assertion fails, failed condition, file and line is printed to
error output and the application aborts. If `CORRADE_NO_ASSERT` is defined,
this macro does nothing. Example usage:
@code
CORRADE_INTERNAL_ASSERT(!nullptr);
@endcode

@attention Don't use this function for checking function output like this:
    @code
    CORRADE_INTERNAL_ASSERT(initialize());
    @endcode
    If `CORRADE_NO_ASSERT` is defined, the macro is not expanded and thus the
    function gets never called. Use @ref CORRADE_INTERNAL_ASSERT_OUTPUT()
    instead.

@see @ref CORRADE_ASSERT_UNREACHABLE()
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_ASSERT(condition) do {} while(0)
#else
#define CORRADE_INTERNAL_ASSERT(condition)                                  \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << "Assertion" << #condition << "failed in" << __FILE__ << "on line" << __LINE__; \
            std::abort();                                                   \
        }                                                                   \
    } while(false)
#endif

/** @hideinitializer
@brief Internal call output assertion macro
@param call         Assert call

Unlike @ref CORRADE_INTERNAL_ASSERT(), this macro performs the call even if
`CORRADE_NO_ASSERT` is defined, making it usable for checking function output.
Otherwise the behavior is the same as with @ref CORRADE_INTERNAL_ASSERT().
Example usage:
@code
CORRADE_INTERNAL_ASSERT_OUTPUT(initialize());
@endcode
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    static_cast<void>(call)
#else
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    do {                                                                    \
        if(!(call)) {                                                       \
            Corrade::Utility::Error() << "Assertion" << #call << "failed in" << __FILE__ << "on line" << __LINE__; \
            std::abort();                                                   \
        }                                                                   \
    } while(false)
#endif

/** @hideinitializer
@brief Assert that the following code is unreachable

By default, if code marked with this macro is reached, message with file and
line is printed to error output and the application aborts. If
`CORRADE_NO_ASSERT` is defined, this macro hints to the compiler that given
code is not reachable, possibly improving performance. Example usage:
@code
switch(flag) {
    case Flag::A: return foo;
    case Flag::B: return bar;
    default: CORRADE_ASSERT_UNREACHABLE();
}
@endcode
@see @ref CORRADE_ASSERT()
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
        Corrade::Utility::Error() << "Reached unreachable code in" << __FILE__ << "on line" << __LINE__; \
        std::abort();                                                       \
    } while(false)
#endif

#endif
