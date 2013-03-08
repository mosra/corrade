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

#include "TestSuite/Tester.h"
#include "Utility/utilities.h"

namespace Corrade { namespace Utility { namespace Test {

class UtilitiesTest: public TestSuite::Tester {
    public:
        UtilitiesTest();

        void pow2();
        void log2();
};

UtilitiesTest::UtilitiesTest() {
    addTests({&UtilitiesTest::pow2,
              &UtilitiesTest::log2});
}

void UtilitiesTest::pow2() {
    CORRADE_COMPARE(Utility::pow2(10), 1024);
}

void UtilitiesTest::log2() {
    CORRADE_COMPARE(Utility::log2(2153), 11);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::UtilitiesTest)
