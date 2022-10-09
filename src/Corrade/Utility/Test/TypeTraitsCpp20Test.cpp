/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/TypeTraits.h"
#include <cstdio>

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TypeTraitsCpp20Test: TestSuite::Tester {
    explicit TypeTraitsCpp20Test();
    void isConstantEvaluatedTest();
};

TypeTraitsCpp20Test::TypeTraitsCpp20Test() {
    addTests({&TypeTraitsCpp20Test::isConstantEvaluatedTest});
}

#ifdef CORRADE_IS_CONSTANT_EVALUATED
constexpr int constevalHelper(int i) {
    if (CORRADE_IS_CONSTANT_EVALUATED)
        return i + 1;
    else {
        std::printf("");
        return i + 2;
    }
}
#endif

void TypeTraitsCpp20Test::isConstantEvaluatedTest() {
#ifndef CORRADE_IS_CONSTANT_EVALUATED
    CORRADE_SKIP("CORRADE_IS_CONSTANT_EVALUATED not supported on this compiler.");
#else
    constexpr int retConstant = constevalHelper(0);
    static_assert(retConstant == 1, "");
    CORRADE_COMPARE(retConstant, 1);
    const volatile int arg = 0;
    const int retRuntime = constevalHelper(arg);
    CORRADE_COMPARE(retRuntime, 2);
#endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TypeTraitsCpp20Test)
