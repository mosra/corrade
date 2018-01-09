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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"

namespace Corrade { namespace Test {

struct CppStandardTest: TestSuite::Tester {
    explicit CppStandardTest();

    void test();
};

CppStandardTest::CppStandardTest() {
    #ifdef COMPILING_AS_CPP11
    setTestName("Cpp11StandardTest");
    #elif defined(COMPILING_AS_CPP14)
    setTestName("Cpp14StandardTest");
    #elif defined(COMPILING_AS_CPP17)
    setTestName("Cpp17StandardTest");
    #else
    #error no standard version macro passed from buildsystem
    #endif

    addTests({&CppStandardTest::test});
}

void CppStandardTest::test() {
    Debug{} << "Standard version using __cplusplus:" << __cplusplus;
    Debug{} << "Standard version using CORRADE_CXX_STANDARD:" << CORRADE_CXX_STANDARD;

    #ifdef COMPILING_AS_CPP11
    CORRADE_COMPARE_AS(CORRADE_CXX_STANDARD, 201103L, TestSuite::Compare::GreaterOrEqual);
    #elif defined(COMPILING_AS_CPP14)
    CORRADE_COMPARE_AS(CORRADE_CXX_STANDARD, 201402L, TestSuite::Compare::GreaterOrEqual);
    #elif defined(COMPILING_AS_CPP17)
    CORRADE_COMPARE_AS(CORRADE_CXX_STANDARD, 201703L, TestSuite::Compare::GreaterOrEqual);
    #else
    #error no standard version macro passed from buildsystem
    #endif
}

}}

CORRADE_TEST_MAIN(Corrade::Test::CppStandardTest)
