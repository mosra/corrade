#ifndef Corrade_Utility_VisibilityMacros_h
#define Corrade_Utility_VisibilityMacros_h
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
 * @brief Macro @ref CORRADE_VISIBILITY_EXPORT, @ref CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT, @ref CORRADE_VISIBILITY_IMPORT, @ref CORRADE_VISIBILITY_STATIC, @ref CORRADE_VISIBILITY_LOCAL
 */

#include "Corrade/configure.h"

/** @hideinitializer
@brief Export a symbol into a shared library

On non-DLL platforms, if the GCC/Clang `-fvisibility=hidden` flag is used
(recommended, enabled by default when you use the `CORRADE_USE_PEDANTIC_FLAGS`
@ref corrade-cmake "CMake property"), symbols are not exported from shared
libraries by default and you need to mark them as such in order to make them
usable from outside code. For example:

@snippet Utility.cpp CORRADE_VISIBILITY_EXPORT

On Windows when building DLLs, this gets complicated further --- everything is
implicitly hidden and you need to `dllexport` the symbols when building the
library (using the @ref CORRADE_VISIBILITY_EXPORT) but `dllimport` them when
using the library (with the @ref CORRADE_VISIBILITY_IMPORT). In practice that
means you need to define some additional macro when compiling the library and
then @cpp #define @ce an alias to one of these based on presence of that macro.
If you use CMake, such macro is implicitly provided in a form of
`<target>_EXPORTS` when compiling any file that's a part of a shared library,
so then the full setup might look like this:

@code{.cmake}
add_library(MyLibrary SHARED MyLibrary.cpp) # MyLibrary_EXPORTS defined

add_executable(my-application main.cpp) # MyLibrary_EXPORTS not defined
target_link_librariees(my-application PRIVATE MyLibrary)
@endcode

@snippet Utility.cpp CORRADE_VISIBILITY_EXPORT-dllexport

On non-DLL platforms both @ref CORRADE_VISIBILITY_EXPORT and
@ref CORRADE_VISIBILITY_IMPORT macros are defined to the same thing, so the
above code works correctly everywhere. If your library can be built as static
as well, have a look at @ref CORRADE_VISIBILITY_STATIC.

@see @ref CORRADE_VISIBILITY_LOCAL, @ref CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT
*/
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_EXPORT __declspec(dllexport)
#else
#define CORRADE_VISIBILITY_EXPORT __attribute__((visibility("default")))
#endif

/** @hideinitializer
@brief Export inline class member into a shared library
@m_since{2019,10}

If the GCC/Clang `-fvisibility-inlines-hidden` flag is used (enabled by default
when you use the `CORRADE_USE_PEDANTIC_FLAGS` @ref corrade-cmake "CMake property"),
@cpp inline @ce functions and class methods are not exported. This is generally
a good thing but may cause issues when you need to compare pointers to such
inline functions (for example signals defined in
@ref Corrade::Interconnect::Emitter "Interconnect::Emitter" subclasses) --- an
inline function defined in a shared library may have a different address inside
and outside of that library.

For free functions (and on Windows for member functions in a non-DLL-exported
class) @ref CORRADE_VISIBILITY_EXPORT / @ref CORRADE_VISIBILITY_IMPORT can be
used without issues, this macro is meant to be used on inline members of an
already DLL-exported class, since `dllexport`ing members of already
`dllexport`ed type is not allowed:

@snippet Utility.cpp CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT

This macro is empty on Windows, which means there's no need for a corresponding
`*_IMPORT` variant of it. Note that this macro exports the symbol even on
static libraries, in that case you might want to use
@ref CORRADE_VISIBILITY_STATIC instead.
*/
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT
#else
#define CORRADE_VISIBILITY_INLINE_MEMBER_EXPORT __attribute__((visibility("default")))
#endif

/** @hideinitializer
@brief Import a symbol from a shared library

To be used in tandem with @ref CORRADE_VISIBILITY_EXPORT, see its documentation
for more information.
*/
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_IMPORT __declspec(dllimport)
#else
#define CORRADE_VISIBILITY_IMPORT __attribute__((visibility("default")))
#endif

/** @hideinitializer
@brief Public symbol in a static library

Defined as empty --- to be consistent with hidden visibility by default,
symbols in static libraries shouldn't be exported either. This macro is
provided mainly as a self-documenting alternative to the
@ref CORRADE_VISIBILITY_EXPORT / @ref CORRADE_VISIBILITY_IMPORT macros in case
a library is built as static instead of dynamic.
*/
#define CORRADE_VISIBILITY_STATIC

/** @hideinitializer
@brief Local symbol

The symbol name will not be exported into a shared or static library. See
@ref CORRADE_VISIBILITY_EXPORT for an example.
*/
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_LOCAL
#else
#define CORRADE_VISIBILITY_LOCAL __attribute__((visibility("hidden")))
#endif

#endif
