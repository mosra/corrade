#ifndef Corrade_Utility_Macros_h
#define Corrade_Utility_Macros_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
 * @brief Macro @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_AUTOMATIC_INITIALIZER(), @ref CORRADE_AUTOMATIC_FINALIZER()
 */

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Internal macro implementation */
#define _CORRADE_HELPER_PASTE2(a, b) a ## b
#define _CORRADE_HELPER_PASTE(a, b) _CORRADE_HELPER_PASTE2(a, b)
#define _CORRADE_HELPER_STR(X) #X
#define _CORRADE_HELPER_DEFER(M, ...) M(__VA_ARGS__)
#endif

/** @hideinitializer
@brief Deprecation mark

Marked function, class or typedef will emit deprecation warning on supported
compilers (GCC, Clang, MSVC):
@code
class CORRADE_DEPRECATED("use Bar instead") Foo;
CORRADE_DEPRECATED("use bar() instead") void foo();
@endcode

Does not work on template aliases and enum values, use
@ref CORRADE_DEPRECATED_ALIAS() and @ref CORRADE_DEPRECATED_ENUM() instead.
@see @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__GNUC__) || defined(__clang__)
#define CORRADE_DEPRECATED(message) __attribute((deprecated(message)))
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED(message) __declspec(deprecated(message))
#else
#define CORRADE_DEPRECATED(message)
#endif

/** @hideinitializer
@brief Alias deprecation mark

Marked alias will emit deprecation warning on supported compilers (GCC, Clang):
@code
template<class T> using Foo CORRADE_DEPRECATED_ALIAS("use Bar instead") = Bar<T>;
@endcode
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ENUM(),
    @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__GNUC__) || defined(__clang__)
#define CORRADE_DEPRECATED_ALIAS(message) __attribute((deprecated(message)))
#else
#define CORRADE_DEPRECATED_ALIAS(message)
#endif

/** @hideinitializer
@brief Enum deprecation mark

Marked enum or enum value will emit deprecation warning on supported compilers
(C++17 feature, Clang and MSVC 2015):
@code
enum class CORRADE_DEPRECATED_ENUM("use Bar instead") Foo {};

enum class Bar {
    Fizz = 0,
    Buzz = 1,
    Baz CORRADE_DEPRECATED_ENUM("use Bar::Buzz instead") = 1
};
@endcode
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__clang__)
#define CORRADE_DEPRECATED_ENUM(message) __attribute((deprecated(message)))
#elif defined(_MSC_VER) && _MSC_VER >= 1900
#define CORRADE_DEPRECATED_ENUM(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_ENUM(message)
#endif

/** @hideinitializer
@brief File deprecation mark

Putting this in a file will emit deprecation warning when given file is
included or compiled (GCCC, Clang):
@code
CORRADE_DEPRECATED_FILE("use Bar.h instead")
@endcode
*/
#if defined(__clang__)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this file is deprecated: " message)))
#elif defined(__GNUC__)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#else
#define CORRADE_DEPRECATED_FILE(message)
#endif

/** @hideinitializer
@brief Automatic initializer
@param function Initializer function name of type int(*)().

Function passed as argument will be called even before entering `main()`
function. This is usable when e.g. automatically registering plugins or data
resources without forcing the user to write additional code in `main()`.
@attention This macro does nothing in static libraries.
*/
#define CORRADE_AUTOMATIC_INITIALIZER(function)                             \
    namespace {                                                             \
        struct Initializer_##function { static const int i; };              \
        const int Initializer_##function::i = function();                   \
    }

/** @hideinitializer
@brief Automatic initializer
@param function Finalizer function name of type int(*)().

Function passed as argument will be called even before entering `main()`
function. This is usable in conjuction with CORRADE_AUTOMATIC_INITIALIZER()
when there is need to properly discard initialized data.
@attention This macro does nothing in static libraries.
*/
#define CORRADE_AUTOMATIC_FINALIZER(function)                               \
    class Finalizer_##function {                                            \
        public:                                                             \
            Finalizer_##function() {}                                       \
            ~Finalizer_##function() { function(); }                         \
    } Finalizer_##function;

#endif
