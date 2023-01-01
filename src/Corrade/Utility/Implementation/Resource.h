#ifndef Corrade_Utility_Implementation_Resource_h
#define Corrade_Utility_Implementation_Resource_h
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

#include <algorithm> /* std::lower_bound() */
#include <Corrade/Containers/StringView.h>
#include <Corrade/Containers/ArrayView.h>

namespace Corrade { namespace Utility { namespace Implementation {

inline Containers::StringView resourceFilenameAt(const unsigned int* const positions, const unsigned char* const filenames, const std::size_t i) {
    /* Every position pair denotes end offsets of one file, filename is first
       and the upper 8 bits are reserved for padding */
    const std::size_t begin = (i == 0 ? 0 : positions[2*(i - 1)]) & 0x00ffffffu;
    const std::size_t end = (positions[2*i]) & 0x00ffffffu;
    return {reinterpret_cast<const char*>(filenames) + begin, end - begin, Containers::StringViewFlag::Global};
}

inline Containers::StringView resourceDataAt(const unsigned int* const positions, const unsigned char* const data, const std::size_t i) {
    /* Every position pair denotes end offsets of one file, data is second */
    const std::size_t begin = i == 0 ? 0 : positions[2*(i - 1) + 1];
    const std::size_t end = positions[2*i + 1];
    /* If there's any padding after (contained in the upper 8 bits of
       filename), the data can be marked as null-terminated. This can be either
       deliberate (a single null byte added after) or "accidental" due to for
       example padding for alignment. */
    const std::size_t padding = (positions[2*i + 0] >> 24 & 0xff);
    return {reinterpret_cast<const char*>(data) + begin, end - begin - padding, Containers::StringViewFlag::Global|(padding ? Containers::StringViewFlag::NullTerminated : Containers::StringViewFlag{})};
}

/* Assuming the filenames are sorted, look up a particular filename. Returns
   either its index or count if not found. */
inline std::size_t resourceLookup(const unsigned int count, const unsigned int* const positionData, const unsigned char* const filenames, const Containers::StringView filename) {
    /* Like std::map, but without crazy allocations using std::lower_bound and
       a std::lexicographical_compare */
    struct Position {
        unsigned int filenamePadding;
        unsigned int data;
    };
    auto positions = Containers::arrayCast<const Position>(Containers::arrayView(positionData, count*2));
    const Position* found = std::lower_bound(positions.begin(), positions.end(), filename,
        [positions, filenames](const Position& position, const Containers::StringView filename) {
            /* The upper 8 bits of filename are reserved for padding */
            const std::size_t end = position.filenamePadding & 0x00ffffffu;
            const std::size_t begin = &position == positions ? 0 : (&position - 1)->filenamePadding & 0x00ffffffu;
            /* Not constructing a temporary StringView here as this shall be
               faster */
            /** @todo Actually, temporary StringView *could* be faster because
                it uses the much faster memcmp() internally, while this damn
                thing is a dumb loop (there go the WILD DREAMS of <algorithm>
                having specialized variants for certain data types). But to
                actually beat the STL map, we have to ditch std::lower_bound()
                because it requires us to do one more comparison at the end to
                see if the string is indeed equal. Which is completely
                unnecessary, as that's provided by memcmp() already (and yes,
                map::at() suffers from the same problem). The rest, according
                to perf, is strange extra overhead in operator<(), operator!=()
                on top of the memcmp() call which I can't tell where it comes
                from and why, and inlining those calls doesn't seem to help
                that much. */
            return std::lexicographical_compare(filenames + begin, filenames + end,
                filename.begin(), filename.end());
        });

    /* No lower bound found */
    if(found == positions.end()) return count;

    /* Check that the filenames match --- it only returns a lower bound, not an
       exact match */
    const std::size_t i = found - positions.begin();
    if(filename != resourceFilenameAt(positionData, filenames, i)) return count;

    /* Return the found index */
    return i;
}

}}}

#endif
