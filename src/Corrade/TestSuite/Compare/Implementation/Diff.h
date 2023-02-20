#ifndef Corrade_TestSuite_Compare_Implementation_Diff_h
#define Corrade_TestSuite_Compare_Implementation_Diff_h
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

#include <cstring>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/Triple.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Implementation {

/* A simple diff algorithm from https://pynash.org/2013/02/26/diff-in-50-lines/
   which is a trimmed-down implementation used by Python's difflib, which is
   then an implementation of the Hunt–McIlroy algorithm listed on Wikipedia:
   https://en.wikipedia.org/wiki/Hunt%E2%80%93Szymanski_algorithm

   Done in a generic way to allow diffs on arbitrary types, not just lines of
   text.

   The complexity seems to be something like `O(mn log m)` where `m` is size
   of the first input and `n` size of the second input. I'm not sure if the
   Hunt–Szymanski optimization as explained on Wikipedia are present here,
   nevertheless this should be ~okay as the use in TestSuite is only when a
   comparison fails, which should be pretty rare. */
/** @todo investigate the complexity if/when this becomes a public API */

template<class T> Containers::Triple<std::size_t, std::size_t, std::size_t> longestMatchingSlice(const Containers::StridedArrayView1D<const T>& a, const Containers::StridedArrayView1D<const T>& b) {
    std::size_t startA = 0;
    std::size_t startB = 0;
    std::size_t longestSize = 0;

    /* A "map" of previous longest runs for each element -- in each iteration,
       `runs[j]` is the length of longest match ending with `a[i - 1]` and
       `b[j]`. Initially, there are no runs */
    Containers::Array<std::size_t> runs{ValueInit, b.size()};
    /* New runs collected in each iteration. Gets cleared at the start of every
       iteration and swapped with `runs` at the end of every iteration, done
       this way to avoid temporary allocations inside the loop. */
    Containers::Array<std::size_t> newRuns{NoInit, b.size()};
    /** @todo might want to pass some "scratch memory" from outside, so this
        doesn't allocate at all (or just wait for allocators?) */

    /* Go through all elements of A */
    for(std::size_t i = 0; i != a.size(); ++i) {
        /* Start with no active runs */
        /** @todo Utility::fill(), finally */
        std::memset(newRuns, 0, newRuns.size()*sizeof(std::size_t));

        /* Go through all elements of B */
        for(std::size_t j = 0; j != b.size(); ++j) {
            if(a[i] != b[j]) continue;

            /* If elements match, extend the previous line with them */
            const std::size_t runSize = newRuns[j] = (j ? runs[j - 1] : 0) + 1;

            /* If the run is longer than the current longest, update */
            if(runSize > longestSize) {
                startA = i - runSize + 1;
                startB = j - runSize + 1;
                longestSize = runSize;
            }
        }

        /* Save the new runs == discard all previously-active runs that didn't
           get extended in this iteration */
        std::swap(runs, newRuns);
    }

    return {startA, startB, longestSize};
}

template<class T> void matchingSlicesInto(Containers::Array<Containers::Triple<std::size_t, std::size_t, std::size_t>>& out, const Containers::StridedArrayView1D<const T>& a, const std::size_t aOffset, const Containers::StridedArrayView1D<const T>& b, const std::size_t bOffset) {
    /* Find the largest matching slice */
    const Containers::Triple<std::size_t, std::size_t, std::size_t> longest = longestMatchingSlice(a, b);

    /* If the ranges don't have anything in common, return without adding
       anything to the output */
    if(!longest.third())
        return;

    /* Recurse to find the largest matching slices before and after this one
       (if there's anything left), put them to the output in order */
    matchingSlicesInto(out,
        a.prefix(longest.first()), aOffset,
        b.prefix(longest.second()), bOffset);
    arrayAppend(out, InPlaceInit,
        longest.first() + aOffset,
        longest.second() + bOffset,
        longest.third());
    matchingSlicesInto(out,
        a.exceptPrefix(longest.first() + longest.third()),
                aOffset + longest.first() + longest.third(),
        b.exceptPrefix(longest.second() + longest.third()),
                bOffset + longest.second() + longest.third());
}

}}}}

#endif
