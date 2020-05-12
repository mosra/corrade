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

#include "Corrade/configure.h"

#ifdef CORRADE_TARGET_WINDOWS
#include <cstdint>

/* 32-bit MinGW doesn't have implicit __argc / __wargv for some reason:
   https://github.com/mirror/mingw-w64/blob/f8e2c9ac594259fd2a46746943f84bd8766d7054/mingw-w64-headers/crt/stdlib.h#L178-L189 */
#include <cstdlib>

/* Use Array, but in a way that doesn't require the whole Utility library
   to be linked */
#define CORRADE_NO_DEBUG
#define CORRADE_NO_ASSERT
#include "Corrade/Containers/Array.h"

#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>

/* Not defined by MinGW at least. Taken from
   https://docs.microsoft.com/en-us/windows/console/setconsolemode */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

using namespace Corrade;

namespace {

Containers::Array<char*> convertWideArgv(std::size_t argc, wchar_t** wargv, Containers::Array<char>& storage) {
    /* Calculate total length of all arguments, save the relative offsets */
    Containers::Array<char*> argv{Containers::ValueInit, argc + 1};
    std::size_t totalSize = 0;
    for(std::size_t i = 0; i != argc; ++i) {
        totalSize += WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
        argv[i + 1] = reinterpret_cast<char*>(totalSize);
    }

    /* Allocate the argument array, make the relative offsets absolute */
    storage = Containers::Array<char>{totalSize};
    for(std::size_t i = 0; i != argv.size(); ++i)
        argv[i] += reinterpret_cast<std::ptrdiff_t>(storage.data());

    /* Convert the arguments to sane UTF-8 */
    for(std::size_t i = 0; i != argc; ++i)
        WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], argv[i + 1] - argv[i], nullptr, nullptr);

    return argv;
}

}

extern "C" int main(int, char**);

/* extern "C" needed for MinGW -- https://sourceforge.net/p/mingw-w64/wiki2/Unicode%20apps/ */
extern "C" int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int);
extern "C" int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    /* Convert argv to UTF-8. We have __wargv, no need to use
       CommandLineToArgvW(). */
    Containers::Array<char> storage;
    #ifdef __MINGW32__
    /* Disable "warning: ISO C++ forbids taking address of function '::main'",
       the main() is mine, doesn't have any automatic constructors or anything
       attached and I know what I am doing here. */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    return main(__argc, convertWideArgv(__argc, __wargv, storage).data());
    #ifdef __MINGW32__
    #pragma GCC diagnostic pop
    #endif
}

extern "C" int wmain(int, wchar_t**);
extern "C" int wmain(int argc, wchar_t** wargv) {
    /* Set output to UTF-8 */
    SetConsoleOutputCP(CP_UTF8);

    #ifdef CORRADE_UTILITY_USE_ANSI_COLORS
    /* Enable ANSI color handling in the console */
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD currentConsoleMode;
    if(out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &currentConsoleMode))
        SetConsoleMode(out, currentConsoleMode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif

    /* Convert argv to UTF-8 */
    Containers::Array<char> storage;
    #ifdef __MINGW32__
    /* Disable "warning: ISO C++ forbids taking address of function '::main'",
       the main() is mine, doesn't have any automatic constructors or anything
       attached and I know what I am doing here. */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    return main(argc, convertWideArgv(argc, wargv, storage).data());
    #ifdef __MINGW32__
    #pragma GCC diagnostic pop
    #endif
}
#else
#error this file is needed only on Windows
#endif
