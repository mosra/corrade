#ifndef Corrade_Containers_Test_EnumSetTest_h
#define Corrade_Containers_Test_EnumSetTest_h
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
#include "Containers/EnumSet.h"

namespace Corrade { namespace Containers { namespace Test {

class EnumSetTest: public TestSuite::Tester<EnumSetTest> {
    public:
        enum class Feature: int {
            Fast = 1 << 0,
            Cheap = 1 << 1,
            Tested = 1 << 2,
            Popular = 1 << 3
        };

        typedef EnumSet<Feature, int> Features;

        EnumSetTest();

        void construct();
        void operatorOr();
        void operatorAnd();
        void operatorBool();
        void operatorInverse();
};

CORRADE_ENUMSET_OPERATORS(EnumSetTest::Features)

}}}

#endif
