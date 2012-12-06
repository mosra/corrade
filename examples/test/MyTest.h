#ifndef Corrade_Examples_MyTest_h
#define Corrade_Examples_MyTest_h
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

#include <TestSuite/Tester.h>

namespace Corrade { namespace Examples {

class MyTest: public Corrade::TestSuite::Tester {
    public:
        MyTest();

        void commutativity();
        void associativity();
        void pi();
        void sin();
};

}}

#endif
