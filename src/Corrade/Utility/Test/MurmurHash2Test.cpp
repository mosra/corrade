/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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
#include "Corrade/Utility/MurmurHash2.h"

namespace Corrade { namespace Utility { namespace Test {

class MurmurHash2Test: public TestSuite::Tester {
    public:
        MurmurHash2Test();

        void test32();
        void test64();
        void constexprCall();
        void constructor();
};

MurmurHash2Test::MurmurHash2Test() {
    addTests({&MurmurHash2Test::test32,
              &MurmurHash2Test::test64,
              &MurmurHash2Test::constexprCall,
              &MurmurHash2Test::constructor});
}

void MurmurHash2Test::test32() {
    constexpr const unsigned int digest1 = Implementation::MurmurHash2<4>{}(23, "string", 6);
    constexpr const unsigned int digest2 = Implementation::MurmurHash2<4>{}(23, "four", 4);
    CORRADE_COMPARE(digest1, 3435905073u);
    CORRADE_COMPARE(digest2, 2072697618u);
}

void MurmurHash2Test::test64() {
    constexpr const unsigned long long digest1 = Implementation::MurmurHash2<8>{}(23, "string", 6);
    constexpr const unsigned long long digest2 = Implementation::MurmurHash2<8>{}(23, "eightbit", 8);
    CORRADE_COMPARE(digest1, 7441339218310318127ull);
    CORRADE_COMPARE(digest2, 14685337704530366946ull);
}

void MurmurHash2Test::constexprCall() {
    constexpr auto digest1 = MurmurHash2::digest("hello");
    CORRADE_COMPARE(digest1, MurmurHash2{}("hello", 5));

    constexpr auto digest2 = MurmurHash2{}("hello");
    CORRADE_COMPARE(digest2, MurmurHash2{}("hello", 5));
}

void MurmurHash2Test::constructor() {

    /* All should give the same value */
    CORRADE_COMPARE(MurmurHash2()("hello"), MurmurHash2()("hello", 5));
    CORRADE_COMPARE(MurmurHash2()(std::string("hello")), MurmurHash2()("hello", 5));
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MurmurHash2Test)
