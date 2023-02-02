/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include <cstdint>

#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/Utility/Debug.h"

using namespace Corrade;

int main() {
{
/* [BitArrayView-operator<<] */
const std::uint64_t data[]{0b00'0101'0101'0011'0011'0000'1111 << 5};
Utility::Debug{} << Containers::BitArrayView{data, 5, 26};
/* [BitArrayView-operator<<] */
}

{
/* [StridedBitArrayView-usage] */
const char data[]{0b1111, 0b1100, 0b0011, 0b0000};
Containers::BitArrayView bits{data, 0, 32};

/* 1, 0, 1, 0 */
Containers::StridedBitArrayView1D a{bits, 4, 8};
/* [StridedBitArrayView-usage] */
}

{
/* [StridedBitArrayView-usage-reshape] */
const std::uint8_t data[]{
    0b0000'0000,
    0b0011'1100,
    0b0011'1100,
    0b0000'0000,
};
Containers::BitArrayView bits{data, 0, 32};

/* In both views [1][3] to [2][5] is all 1s */
Containers::StridedBitArrayView2D a{bits, {4, 8}, {8, 1}};
Containers::StridedBitArrayView2D b{bits, {4, 8}};
/* [StridedBitArrayView-usage-reshape] */
}

{
/* [StridedBitArrayView-operator<<] */
const std::uint64_t data[]{0b0101'0101'0011'0011'0000'1111 << 5};
Containers::BitArrayView a{data, 5, 24};
Utility::Debug{} << Containers::StridedBitArrayView2D{a, {3, 8}};
/* [StridedBitArrayView-operator<<] */
}

}
