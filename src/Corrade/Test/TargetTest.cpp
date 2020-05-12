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

/* Including this first so we test it guesses everything properly without
   accidental help from standard headers */
#include "Corrade/configure.h"

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

#include "configure.h"

namespace Corrade { namespace Test { namespace {

struct TargetTest: TestSuite::Tester {
    explicit TargetTest();

    void system();
    void architecture();
    void endian();
    void compiler();
    void stl();
    void simd();
};

TargetTest::TargetTest() {
    addTests({&TargetTest::system,
              &TargetTest::architecture,
              &TargetTest::endian,
              &TargetTest::compiler,
              &TargetTest::stl,
              &TargetTest::simd});
}

void TargetTest::system() {
    std::ostringstream out;
    #ifdef CORRADE_TARGET_APPLE
    Debug{&out} << "CORRADE_TARGET_APPLE";
    #ifndef CORRADE_TARGET_UNIX
    CORRADE_VERIFY(!"CORRADE_TARGET_APPLE defined but CORRADE_TARGET_UNIX not");
    #endif
    #endif

    #ifdef CORRADE_TARGET_ANDROID
    Debug{&out} << "CORRADE_TARGET_ANDROID";
    #ifndef CORRADE_TARGET_UNIX
    CORRADE_VERIFY(!"CORRADE_TARGET_ANDROID defined but CORRADE_TARGET_UNIX not");
    #endif
    #endif

    #ifdef CORRADE_TARGET_EMSCRIPTEN
    Debug{&out} << "CORRADE_TARGET_EMSCRIPTEN";
    #ifdef CORRADE_TARGET_UNIX
    CORRADE_VERIFY(!"CORRADE_TARGET_EMSCRIPTEN defined but CORRADE_TARGET_UNIX as well");
    #endif
    #endif

    #ifdef CORRADE_TARGET_UNIX
    Debug{&out} << "CORRADE_TARGET_UNIX";
    #endif

    #ifdef CORRADE_TARGET_WINDOWS_RT
    Debug{&out} << "CORRADE_TARGET_WINDOWS_RT";
    #ifndef CORRADE_TARGET_WINDOWS
    CORRADE_VERIFY(!"CORRADE_TARGET_WINDOWS_RT defined but CORRADE_TARGET_WINDOWS not");
    #endif
    #ifdef CORRADE_TARGET_UNIX
    CORRADE_VERIFY(!"CORRADE_TARGET_WINDOWS_RT defined but CORRADE_TARGET_UNIX as well");
    #endif
    #endif

    #ifdef CORRADE_TARGET_WINDOWS
    Debug{&out} << "CORRADE_TARGET_WINDOWS";
    #ifdef CORRADE_TARGET_UNIX
    CORRADE_VERIFY(!"CORRADE_TARGET_WINDOWS defined but CORRADE_TARGET_UNIX as well");
    #endif
    #endif

    Debug{Debug::Flag::NoNewlineAtTheEnd} << out.str();
    CORRADE_VERIFY(!out.str().empty() || !"No suitable CORRADE_TARGET_* defined");
}

void TargetTest::architecture() {
    std::ostringstream out;
    int unique = 0;

    #ifdef CORRADE_TARGET_X86
    ++unique;
    Debug{&out} << "CORRADE_TARGET_X86";
    #endif

    #ifdef CORRADE_TARGET_ARM
    ++unique;
    Debug{&out} << "CORRADE_TARGET_ARM";
    #endif

    #ifdef CORRADE_TARGET_POWERPC
    ++unique;
    Debug{&out} << "CORRADE_TARGET_POWERPC";
    #endif

    #ifdef CORRADE_TARGET_EMSCRIPTEN
    ++unique;
    Debug{&out} << "CORRADE_TARGET_EMSCRIPTEN";
    #endif

    Debug{Debug::Flag::NoNewlineAtTheEnd} << out.str();
    CORRADE_VERIFY(!out.str().empty() || !"No suitable CORRADE_TARGET_* defined");
    CORRADE_COMPARE(unique, 1);
}

void TargetTest::endian() {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
    Debug{} << "CORRADE_TARGET_BIG_ENDIAN";
    #endif

    union {
        char bytes[4];
        int number;
    } caster;
    caster.number = 0x03020100;
    #ifdef CORRADE_TARGET_BIG_ENDIAN
    CORRADE_COMPARE(caster.bytes[0], 3);
    #else
    CORRADE_COMPARE(caster.bytes[0], 0);
    #endif
}

void TargetTest::compiler() {
    std::ostringstream out;

    #ifdef CORRADE_TARGET_GCC
    Debug{&out} << "CORRADE_TARGET_GCC";
    #endif

    #ifdef CORRADE_TARGET_CLANG
    Debug{&out} << "CORRADE_TARGET_CLANG";
    #endif

    #ifdef CORRADE_TARGET_APPLE_CLANG
    Debug{&out} << "CORRADE_TARGET_APPLE_CLANG";
    #endif

    #ifdef CORRADE_TARGET_CLANG_CL
    Debug{&out} << "CORRADE_TARGET_CLANG_CL";
    #endif

    #ifdef CORRADE_TARGET_MSVC
    Debug{&out} << "CORRADE_TARGET_MSVC";
    #endif

    #ifdef CORRADE_TARGET_MINGW
    Debug{&out} << "CORRADE_TARGET_MINGW";
    #endif

    Debug{Debug::Flag::NoNewlineAtTheEnd} << out.str();
    CORRADE_VERIFY(!out.str().empty() || !"No suitable CORRADE_TARGET_* defined");

    #if defined(CMAKE_CORRADE_TARGET_GCC) != defined(CORRADE_TARGET_GCC)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_GCC");
    #endif

    #if defined(CMAKE_CORRADE_TARGET_CLANG) != defined(CORRADE_TARGET_CLANG)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_CLANG");
    #endif

    #if defined(CMAKE_CORRADE_TARGET_APPLE_CLANG) != defined(CORRADE_TARGET_APPLE_CLANG)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_APPLE_CLANG");
    #endif

    #if defined(CMAKE_CORRADE_TARGET_CLANG_CL) != defined(CORRADE_TARGET_CLANG_CL)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_CLANG_CL");
    #endif

    #if defined(CMAKE_CORRADE_TARGET_MSVC) != defined(CORRADE_TARGET_MSVC)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_MSVC");
    #endif

    #if defined(CMAKE_CORRADE_TARGET_MINGW) != defined(CORRADE_TARGET_MINGW)
    CORRADE_VERIFY(!"Inconsistency in CMake-defined CORRADE_TARGET_MINGW");
    #endif

    #if defined(CORRADE_TARGET_CLANG) && defined(CORRADE_TARGET_MSVC) == defined(CORRADE_TARGET_GCC)
    CORRADE_VERIFY(!"Clang should have either a MSVC or a GCC frontend, but not both");
    #endif
}

void TargetTest::stl() {
    std::ostringstream out;
    int unique = 0;

    #ifdef CORRADE_TARGET_LIBSTDCXX
    ++unique;
    Debug{&out} << "CORRADE_TARGET_LIBSTDCXX";
    #endif

    #ifdef CORRADE_TARGET_LIBCXX
    ++unique;
    Debug{&out} << "CORRADE_TARGET_LIBCXX";
    #endif

    #ifdef CORRADE_TARGET_DINKUMWARE
    ++unique;
    Debug{&out} << "CORRADE_TARGET_DINKUMWARE";
    #endif

    Debug{Debug::Flag::NoNewlineAtTheEnd} << out.str();
    CORRADE_VERIFY(!out.str().empty() || !"No suitable CORRADE_TARGET_* defined");
    CORRADE_COMPARE(unique, 1);
}

void TargetTest::simd() {
    std::ostringstream out;

    #ifdef CORRADE_TARGET_SSE2
    Debug{&out} << "CORRADE_TARGET_SSE2";
    #endif

    Debug{Debug::Flag::NoNewlineAtTheEnd} << out.str();
    if(out.str().empty()) Debug{} << "No suitable CORRADE_TARGET_* defined";
    CORRADE_VERIFY(true);
}

}}}

CORRADE_TEST_MAIN(Corrade::Test::TargetTest)
