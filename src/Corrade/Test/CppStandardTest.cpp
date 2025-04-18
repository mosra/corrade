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

#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"

namespace Corrade { namespace Test { namespace {

struct CppStandardTest: TestSuite::Tester {
    explicit CppStandardTest();

    void test();
};

CppStandardTest::CppStandardTest() {
    #ifdef COMPILING_AS_CPP11
    setTestName("Cpp11StandardTest");
    #elif defined(COMPILING_AS_CPP14)
    setTestName(TEST_NAME);
    #elif defined(COMPILING_AS_CPP17)
    setTestName("Cpp17StandardTest");
    #elif defined(COMPILING_AS_CPP2A)
    setTestName("Cpp2aStandardTest");
    #else
    #error no standard version macro passed from buildsystem
    #endif

    addTests({&CppStandardTest::test});
}

void CppStandardTest::test() {
    CORRADE_INFO(
        "Standard version using __cplusplus:" << __cplusplus << Debug::newline <<
        "        Standard version using CORRADE_CXX_STANDARD:" << CORRADE_CXX_STANDARD);

    #ifdef COMPILING_AS_CPP11
    {
        #ifdef CORRADE_TARGET_MSVC
        CORRADE_EXPECT_FAIL("MSVC always compiles at least as C++14.");
        #endif
        CORRADE_COMPARE(CORRADE_CXX_STANDARD, 201103L);
        #ifdef CORRADE_TARGET_CXX14
        CORRADE_FAIL("CORRADE_TARGET_CXX14 defined for C++11.");
        #endif
    }
    #ifdef CORRADE_TARGET_MSVC
    CORRADE_COMPARE(CORRADE_CXX_STANDARD, 201402L);
    #endif

    #elif defined(COMPILING_AS_CPP14)
    #ifndef CORRADE_TARGET_CXX14
    CORRADE_FAIL("CORRADE_TARGET_CXX14 not defined for C++14.");
    #endif
    {
        /* If the cxx_std_14 feature is used, it makes the compiler use that
           or any newer. GCC 11 and Clang 16 are the first that default to
           C++17 and the standard isn't downgraded for them. */
        #if (defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 11) || (defined(CORRADE_TARGET_CLANG) && !defined(CORRADE_TARGET_CLANG_CL) && __clang_major__ >= 16)
        CORRADE_EXPECT_FAIL_IF(testName() == "Cpp14StandardTestCMakeFeatures",
            "CMake (3.20.4) doesn't properly set -std=c++14 for GCC 11+ / Clang 16+, making it default to C++17 instead.");
        #endif
        CORRADE_COMPARE(CORRADE_CXX_STANDARD, 201402L);
        #ifdef CORRADE_TARGET_CXX17
        CORRADE_FAIL("CORRADE_TARGET_CXX17 defined for C++14.");
        #endif
    }

    #elif defined(COMPILING_AS_CPP17)
    CORRADE_COMPARE(CORRADE_CXX_STANDARD, 201703L);
    #ifndef CORRADE_TARGET_CXX14
    CORRADE_FAIL("CORRADE_TARGET_CXX14 not defined for C++17.");
    #endif
    #ifndef CORRADE_TARGET_CXX17
    CORRADE_FAIL("CORRADE_TARGET_CXX17 not defined for C++17.");
    #endif
    #ifdef CORRADE_TARGET_CXX20
    CORRADE_FAIL("CORRADE_TARGET_CXX20 defined for C++17.");
    #endif

    #elif defined(COMPILING_AS_CPP2A)
    CORRADE_COMPARE_AS(CORRADE_CXX_STANDARD, 201703L,
        TestSuite::Compare::Greater);
    #ifndef CORRADE_TARGET_CXX14
    CORRADE_FAIL("CORRADE_TARGET_CXX14 not defined for C++20.");
    #endif
    #ifndef CORRADE_TARGET_CXX17
    CORRADE_FAIL("CORRADE_TARGET_CXX17 not defined for C++20.");
    #endif
    #if !defined(CORRADE_TARGET_CXX20) && CORRADE_CXX_STANDARD == 202002
    CORRADE_FAIL("CORRADE_TARGET_CXX20 not defined for C++20.");
    #endif

    #else
    #error no standard version macro passed from the buildsystem
    #endif
}

}}}

CORRADE_TEST_MAIN(Corrade::Test::CppStandardTest)
