#ifndef Corrade_Containers_sequenceHelpers_h
#define Corrade_Containers_sequenceHelpers_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/configure.h"

namespace Corrade { namespace Containers { namespace Implementation {

#ifndef DOXYGEN_GENERATING_OUTPUT /* it should ignore, but it doesn't */
/** @todo C++14: use std::make_index_sequence and std::integer_sequence */
template<std::size_t ...> struct Sequence {};

/* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
    GenerateSequence<N-1, N-1, sequence...> {};

template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
    typedef Sequence<sequence...> Type;
};
#endif

}}}

#endif
