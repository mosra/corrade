/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#include "Corrade/Utility/StlForwardTupleSizeElement.h"

namespace {
    struct IntFloat {
        int a;
        float b;
    };
}

/* Can't have `template<> struct std::tuple_size<IntFloat>`, on GCC 4.8 it
   leads to `error: specialization of '...' in different namespace`. */
namespace std {

/* Use the type ASAP to avoid Tester actually dragging the definition in */
template<> struct tuple_size<IntFloat> {
    /* Not using std::integral_constant to avoid having to include
       <type_traits>, which could cause the StlForwardTupleSizeElement header
       to seemingly work while it wouldn't without <type_traits> included. */
    enum: std::size_t { value = 2 };
};
template<> struct tuple_element<0, IntFloat> {
    typedef int type;
};
template<> struct tuple_element<1, IntFloat> {
    typedef float type;
};

}

#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StlForwardTupleSizeElementTest: TestSuite::Tester {
    explicit StlForwardTupleSizeElementTest();

    void test();
};

StlForwardTupleSizeElementTest::StlForwardTupleSizeElementTest() {
    addTests({&StlForwardTupleSizeElementTest::test});
}

void StlForwardTupleSizeElementTest::test() {
    /* Just verify that this compiles without error. Not testing the actual
       structured bindings as that would require C++17 while this is a feature
       that should work on C++11 too. */
    CORRADE_COMPARE(std::tuple_size<IntFloat>::value, 2);
    CORRADE_VERIFY(std::is_same<std::tuple_element<0, IntFloat>::type, int>::value);
    CORRADE_VERIFY(std::is_same<std::tuple_element<1, IntFloat>::type, float>::value);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StlForwardTupleSizeElementTest)
