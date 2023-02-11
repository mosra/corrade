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

#include "BitArrayView.h"

#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

Utility::Debug& operator<<(Utility::Debug& debug, BitArrayView value) {
    debug << "{" << Utility::Debug::nospace;

    const auto* data = reinterpret_cast<const unsigned char*>(value.data());
    unsigned char mask = 1 << value.offset();
    for(std::size_t i = 0, iMax = value.size(); i != iMax; ++i) {
        if(!mask) {
            ++data;
            mask = 1;
        }

        if(i && i % 8 == 0) debug << ",";

        debug << (*data & mask ? "1" : "0") << Utility::Debug::nospace;

        mask <<= 1;
    }

    return debug << "}";
}

}}
