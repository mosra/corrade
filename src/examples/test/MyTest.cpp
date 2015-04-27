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

#include <cmath>
#include <Corrade/TestSuite/Tester.h>

namespace Corrade { namespace Examples {

struct MyTest: TestSuite::Tester {
    explicit MyTest();

    void commutativity();
    void associativity();
    void pi();
    void sin();
    void bigEndian();
};

MyTest::MyTest() {
    addTests({&MyTest::commutativity,
              &MyTest::associativity,
              &MyTest::sin,
              &MyTest::pi,
              &MyTest::bigEndian});
}

void MyTest::commutativity() {
    CORRADE_VERIFY(5*3 == 3*5);
    CORRADE_VERIFY(15/3 == 3/15);
}

void MyTest::associativity() {
    int result = (42/(2*3))*191;
    CORRADE_COMPARE(result, 1337);
}

void MyTest::sin() {
    CORRADE_COMPARE_AS(std::sin(0), 0.0f, float);
}

void MyTest::pi() {
    CORRADE_EXPECT_FAIL("Need better approximation.");
    double pi = 22/7.0;
    CORRADE_COMPARE(pi, 3.14);
}

void MyTest::bigEndian() {
    if(!false)
        CORRADE_SKIP("No affordable big endian machines exist to test this properly.");
}

}}

CORRADE_TEST_MAIN(Corrade::Examples::MyTest)
