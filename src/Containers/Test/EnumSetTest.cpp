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

#include "Containers/EnumSet.h"
#include "TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

class EnumSetTest: public TestSuite::Tester {
    public:
        EnumSetTest();

        void construct();
        void operatorOr();
        void operatorAnd();
        void operatorBool();
        void operatorInverse();
        void compare();
};

enum class Feature: int {
    Fast = 1 << 0,
    Cheap = 1 << 1,
    Tested = 1 << 2,
    Popular = 1 << 3
};

typedef EnumSet<Feature, int, 15> Features;

CORRADE_ENUMSET_OPERATORS(Features)

EnumSetTest::EnumSetTest() {
    addTests(&EnumSetTest::construct,
             &EnumSetTest::operatorOr,
             &EnumSetTest::operatorAnd,
             &EnumSetTest::operatorBool,
             &EnumSetTest::operatorInverse,
             &EnumSetTest::compare);
}

void EnumSetTest::construct() {
    Features noFeatures;
    CORRADE_COMPARE(int(noFeatures), 0);

    Features features = Feature::Cheap;
    CORRADE_COMPARE(int(features), 2);
}

void EnumSetTest::operatorOr() {
    Features features = Feature::Cheap|Feature::Fast;
    CORRADE_COMPARE(int(features), 3);

    CORRADE_COMPARE(int(features|Feature::Tested), 7);
    CORRADE_COMPARE(int(Feature::Tested|features), 7);

    features |= Feature::Tested;
    CORRADE_COMPARE(int(features), 7);
}

void EnumSetTest::operatorAnd() {
    CORRADE_COMPARE(int(Feature::Cheap & Feature::Fast), 0);

    Features features = Feature::Popular|Feature::Fast|Feature::Cheap;
    CORRADE_COMPARE(int(features & Feature::Popular), 8);
    CORRADE_COMPARE(int(Feature::Popular & features), 8);

    CORRADE_COMPARE(int(features & Feature::Tested), 0);

    Features features2 = Feature::Popular|Feature::Fast|Feature::Tested;
    CORRADE_COMPARE(int(features & features2), 9);

    features &= features2;
    CORRADE_COMPARE(int(features), 9);
}

void EnumSetTest::operatorBool() {
    CORRADE_COMPARE(!!(Features()), false);

    Features features = Feature::Cheap|Feature::Fast;
    CORRADE_COMPARE(!!(features & Feature::Popular), false);
    CORRADE_COMPARE(!!(features & Feature::Cheap), true);
}

void EnumSetTest::operatorInverse() {
    CORRADE_COMPARE(int(~Features()), 15);
    CORRADE_COMPARE(int(~(Feature::Popular|Feature::Cheap)), 5);
    CORRADE_COMPARE(int(~Feature::Popular), 7);
}

void EnumSetTest::compare() {
    Features features = Feature::Popular|Feature::Fast|Feature::Cheap;
    CORRADE_VERIFY(features == features);
    CORRADE_VERIFY(!(features != features));
    CORRADE_VERIFY(Feature::Cheap == Features(Feature::Cheap));
    CORRADE_VERIFY(Feature::Cheap != Features(Feature::Popular));

    CORRADE_VERIFY(Features() <= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular >= Features());
    CORRADE_VERIFY(Feature::Popular <= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular >= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular <= features);
    CORRADE_VERIFY(features >= Feature::Popular);
    CORRADE_VERIFY(features <= features);
    CORRADE_VERIFY(features >= features);

    CORRADE_VERIFY(features <= (Feature::Popular|Feature::Fast|Feature::Cheap|Feature::Tested));
    CORRADE_VERIFY(!(features >= (Feature::Popular|Feature::Fast|Feature::Cheap|Feature::Tested)));
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::EnumSetTest)
