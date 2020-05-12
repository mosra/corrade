#ifndef Corrade_Utility_Implementation_Resource_h
#define Corrade_Utility_Implementation_Resource_h
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

#include <cstring>
#include <algorithm>
#include <Corrade/Containers/ArrayView.h>

namespace Corrade { namespace Utility { namespace Implementation {

inline Containers::ArrayView<const char> resourceFilenameAt(const unsigned int* const positions, const unsigned char* const filenames, const std::size_t i) {
    /* Every position pair denotes end offsets of one file, filename is first */
    const std::size_t begin = i == 0 ? 0 : positions[2*(i - 1)];
    const std::size_t end = positions[2*i];
    return {reinterpret_cast<const char*>(filenames) + begin, end - begin};
}

inline Containers::ArrayView<const char> resourceDataAt(const unsigned int* const positions, const unsigned char* const data, const std::size_t i) {
    /* Every position pair denotes end offsets of one file, data is second */
    const std::size_t begin = i == 0 ? 0 : positions[2*(i - 1) + 1];
    const std::size_t end = positions[2*i + 1];
    return {reinterpret_cast<const char*>(data) + begin, end - begin};
}

/* Assuming the filenames are sorted, look up a particular filename. Returns
   either its index or count if not found. */
inline std::size_t resourceLookup(const unsigned int count, const unsigned int* const positionData, const unsigned char* const filenames, const Containers::ArrayView<const char> filename) {
    /* Like std::map, but without crazy allocations using std::lower_bound and
       a std::lexicographical_compare */
    struct Position {
        unsigned int filename;
        unsigned int data;
    };
    auto positions = Containers::arrayCast<const Position>(Containers::arrayView(positionData, count*2));
    const Position* found = std::lower_bound(positions.begin(), positions.end(), filename,
        [positions, filenames](const Position& position, const Containers::ArrayView<const char> filename) {
            const std::size_t end = position.filename;
            const std::size_t begin = &position == positions ? 0 : (&position - 1)->filename;
            return std::lexicographical_compare(filenames + begin, filenames + end,
                filename.begin(), filename.end());
        });

    /* No lower bound found */
    if(found == positions.end()) return count;

    /* Check that the filenames match --- it only returns a lower bound, not an
       exact match */
    const std::size_t i = found - positions.begin();
    const Containers::ArrayView<const char> foundFilename = resourceFilenameAt(positionData, filenames, i);
    if(filename.size() != foundFilename.size() || std::memcmp(filename, foundFilename, filename.size())) return count;

    /* Return the found index */
    return i;
}

}}}

#endif
