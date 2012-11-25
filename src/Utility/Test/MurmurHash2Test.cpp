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

#include "MurmurHash2Test.h"

#include "Utility/MurmurHash2.h"

CORRADE_TEST_MAIN(Corrade::Utility::Test::MurmurHash2Test)

namespace Corrade { namespace Utility { namespace Test {

MurmurHash2Test::MurmurHash2Test() {
    addTests(&MurmurHash2Test::test32,
             &MurmurHash2Test::test64);
}

void MurmurHash2Test::test32() {
    CORRADE_COMPARE(MurmurHash2Implementation<4>(23)(reinterpret_cast<const unsigned char*>("string"), 6), 3435905073u);
    CORRADE_COMPARE(MurmurHash2Implementation<4>(23)(reinterpret_cast<const unsigned char*>("four"), 4), 2072697618u);
}
void MurmurHash2Test::test64() {
    CORRADE_COMPARE(MurmurHash2Implementation<8>(23)(reinterpret_cast<const unsigned char*>("string"), 6), 7441339218310318127ull);
    CORRADE_COMPARE(MurmurHash2Implementation<8>(23)(reinterpret_cast<const unsigned char*>("eightbit"), 8), 14685337704530366946ull);
}

}}}
