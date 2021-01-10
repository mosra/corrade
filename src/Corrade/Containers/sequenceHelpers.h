#ifndef Corrade_Containers_sequenceHelpers_h
#define Corrade_Containers_sequenceHelpers_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include "Corrade/configure.h"

namespace Corrade { namespace Containers { namespace Implementation {

#ifndef DOXYGEN_GENERATING_OUTPUT /* it should ignore, but it doesn't */
/* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
template<std::size_t ...> struct Sequence {};

/* Originally GenerateSequence was implemented as a simple linear generator:

    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };

   However, O(n) is needlessly inefficient and a much better O(log n)
   implementation can be done by splitting the work in half and then joining
   the two sequences together. For GenerateSequence<65> that'll results in the
   compiler generating in addition just GenerateSequence<33>, <32>, <16>, <8>,
   <4> and <2> instead of 64 new types. For more extreme cases that results in
   rather significant memory and time savings -- running the
   SequenceHelpersTest as-is (baseline) and with GenerateSequence<899>
   uncommented (which is the maximum template recursion depth in the original
   implementation) improved as following:

                        | before            | after
    --------------------+-------------------+---------------
    GCC 10 baseline     | 37 MB, 0.13 s     | 37 MB, 0.13 s
    Clang 10 baseline   | 91 MB, 0.15 s     | 92 MB, 0.16 s
    GCC 10 <899>        | 275 MB, 0.62 s    | 38 MB, 0.12 s
    Clang 10 <899>      | 162 MB, 0.39 s    | 92 MB, 0.17 s

   I did a test with GCC 4.8 as well, the numbers were roughly in the same
   ballpark. Original idea coming from here (libstdc++ std::integer_sequence
   implementation, which I can't use because it's only since C++14):

    https://gcc.gnu.org/legacy-ml/libstdc++/2015-11/msg00129.html

   For a more realistic use case, compiler memory usage in
   TradeMaterialDataTest from magnum 88d2acc66e0a33113e06478859bef01d20d1e677
   (before the 5000-line monster got split into smaller pieces) went down from
   318 to 309 MB. MaterialData uses up to GenerateSequence<63> so that's the
   heaviest usage of this helper in the whole codebase.

   Further optimization could be done by using Clang's __make_integer_seq from
   https://reviews.llvm.org/D14814, however I think this is good enough now. */
template<class A, class B> struct SequenceConcat;
template<std::size_t ...first, std::size_t ...second> struct SequenceConcat<Sequence<first...>, Sequence<second...>> {
    typedef Sequence<first..., (sizeof...(first) + second)...> Type;
};

template<std::size_t N> struct GenerateSequence:
    SequenceConcat<typename GenerateSequence<N/2>::Type,
                   typename GenerateSequence<N - N/2>::Type> {};
template<> struct GenerateSequence<1> { typedef Sequence<0> Type; };
template<> struct GenerateSequence<0> { typedef Sequence<> Type; };
#endif

}}}

#endif
