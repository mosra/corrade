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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/AbstractHash.h"
#include "Corrade/Utility/Sha1.h"

namespace Corrade { namespace Utility { namespace Test {

struct Sha1Test: TestSuite::Tester {
    explicit Sha1Test();

    void emptyString();
    void exact64bytes();
    void exactOneBlockPadding();
    void twoBlockPadding();
};

Sha1Test::Sha1Test() {
    addTests({&Sha1Test::emptyString,
              &Sha1Test::exact64bytes,
              &Sha1Test::exactOneBlockPadding,
              &Sha1Test::twoBlockPadding});
}

void Sha1Test::emptyString() {
    CORRADE_COMPARE(Sha1::digest(""),
                    Sha1::Digest::fromHexString("da39a3ee5e6b4b0d3255bfef95601890afd80709"));
}

void Sha1Test::exact64bytes() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456789f1234"),
                    Sha1::Digest::fromHexString("d9aa447706df8797b4f5fe94caa9f6ea723a87c8"));
}

void Sha1Test::exactOneBlockPadding() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e12345"),
                    Sha1::Digest::fromHexString("4cc8d5cfacbb575ddeeed504dd4f7cc09a9d49a3"));
}

void Sha1Test::twoBlockPadding() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456"),
                    Sha1::Digest::fromHexString("40e94c62ada5dc762f3e9c472001ca64a67d2cbb"));
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::Sha1Test)
