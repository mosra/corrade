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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/AbstractHash.h"

namespace Corrade { namespace Utility { namespace Test {

class HashDigestTest: public TestSuite::Tester {
    public:
        HashDigestTest();

        void toHexString();
        void fromHexString();
        void debug();
};

HashDigestTest::HashDigestTest() {
    addTests({&HashDigestTest::toHexString,
              &HashDigestTest::fromHexString,
              &HashDigestTest::debug});
}

void HashDigestTest::toHexString() {
    const unsigned char rawDigest[4] = { 0xCA, 0xFE, 0x90, 0xfa };
    CORRADE_COMPARE(HashDigest<4>::fromByteArray(reinterpret_cast<const char*>(rawDigest)).hexString(), "cafe90fa");
}

void HashDigestTest::fromHexString() {
    CORRADE_COMPARE(HashDigest<4>::fromHexString("cafe90fa").hexString(), "cafe90fa");
    CORRADE_COMPARE(HashDigest<4>::fromHexString("1234abcdef").hexString(), "00000000");
    CORRADE_COMPARE(HashDigest<4>::fromHexString("babe").hexString(), "00000000");
    CORRADE_COMPARE(HashDigest<4>::fromHexString("bullshit").hexString(), "00000000");
}

void HashDigestTest::debug() {
    std::ostringstream out;
    Debug(&out) << HashDigest<4>::fromHexString("defeca7e");
    CORRADE_COMPARE(out.str(), "defeca7e\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::HashDigestTest)
