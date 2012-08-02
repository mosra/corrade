/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "MyTest.h"

#include <cmath>

namespace Corrade { namespace Examples {

MyTest::MyTest() {
    addTests(&MyTest::commutativity,
             &MyTest::associativity,
             &MyTest::sin,
             &MyTest::pi);
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
    CORRADE_COMPARE_AS(::sin(0), 0.0f, float);
}

void MyTest::pi() {
    CORRADE_EXPECT_FAIL("Need better approximation.");
    double pi = 22/7.0;
    CORRADE_COMPARE(pi, 3.14);
}

}}

CORRADE_TEST_MAIN(Corrade::Examples::MyTest)
