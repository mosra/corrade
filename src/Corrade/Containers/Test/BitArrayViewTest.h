#ifndef Corrade_Containers_Test_BitArrayViewTest_h
#define Corrade_Containers_Test_BitArrayViewTest_h
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

/* Contains additional variants of BitArrayView algorithms that are included
   just for historical / testing / benchmark comparison purposes; referenced
   explicitly via function pointers from the respective test case and benchmark
   instances */

#include "Corrade/Cpu.h"
#ifdef CORRADE_ENABLE_POPCNT
#include "Corrade/Utility/IntrinsicsAvx.h"
#endif

namespace Corrade { namespace Containers { namespace Test { namespace {

/* The 64-bit variants of POPCNT and BMI1 instructions aren't exposed on 32-bit
   systems, and no 32-bit fallback is implemented for the library version
   either. See the source for details. */
#if defined(CORRADE_ENABLE_POPCNT) && !defined(CORRADE_TARGET_32BIT)
/* Copy of the functions in BitArrayView.cpp, keep in sync */
CORRADE_ALWAYS_INLINE std::uint64_t maskBefore(const std::uint64_t before) {
    CORRADE_INTERNAL_DEBUG_ASSERT(before < 64);
    return ~((1ull << before) - 1);
}
CORRADE_ALWAYS_INLINE std::uint64_t maskAfter(const std::uint64_t after) {
    CORRADE_INTERNAL_DEBUG_ASSERT(after <= 64);
    const std::uint64_t i = 1ull << (after - 1);
    return (i|(i - 1));
}

/* Extracted out of BitArrayView.cpp for history preservation -- the variant
   with POPCNT+BMI1 is faster and BMI1 is nowadays available everywhere POPCNT
   is so it makes no sense to keep both */
CORRADE_NEVER_INLINE CORRADE_ENABLE_POPCNT typename std::size_t bitCountSetImplementationPopcnt(const char* const data, const std::size_t bitOffset, const std::size_t size) {
    CORRADE_ASSUME(bitOffset < 8);

    /* If there are no bits to go through, bail. This has to be here, because
       with non-zero `bitOffset` the code will always read at least 1 byte,
       which would crash with inaccessible or nullptr `data`. */
    if(!size)
        return std::size_t{0};

    const std::size_t bitEndOffset = bitOffset + size;
    /* Points past the last byte of which we take at least one bit */
    const char* const end = data + ((bitEndOffset + 7) >> 3);
    /* Used by either branch below, so calculate always */
    const std::uint64_t initialMask = maskBefore(bitOffset);

    /* A special case for when we have 64 or less bits to process. Reading the
       bytes backwards, the last iteration then masks out bits both at the
       start and at the end and returns the popcount. */
    /** @todo figure out a more optimized way to read the bytes, see the
        POPCNT+BMI1 variant for details */
    if(bitEndOffset <= 64) {
        CORRADE_INTERNAL_DEBUG_ASSERT(end - data <= 8);
        union {
            char b[8];
            std::uint64_t v;
        } value{};
        switch(end - data) {
            /** @todo the order probably needs to be flipped for BE */
            case 8: value.b[7] = data[7]; CORRADE_FALLTHROUGH
            case 7: value.b[6] = data[6]; CORRADE_FALLTHROUGH
            case 6: value.b[5] = data[5]; CORRADE_FALLTHROUGH
            case 5: value.b[4] = data[4]; CORRADE_FALLTHROUGH
            case 4: value.b[3] = data[3]; CORRADE_FALLTHROUGH
            case 3: value.b[2] = data[2]; CORRADE_FALLTHROUGH
            case 2: value.b[1] = data[1]; CORRADE_FALLTHROUGH
            case 1: value.b[0] = data[0];
                    return std::size_t(_mm_popcnt_u64(value.v & initialMask & maskAfter(bitEndOffset)));
        }

        CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    /* Find the next 8-byte aligned position. Since it's always larger than
       `data`, it always has a full initial byte. And as the shorter cases were
       handled above, it's never past `end`. */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 8) & ~0x07);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && i <= end && reinterpret_cast<std::uintptr_t>(i) % 8 == 0);

    std::size_t out = 0;

    /* Unconditionally process the first eight unaligned bytes, masking out
       *bits* before `bitOffset` and *bytes* after `i`. As the shorter cases
       were handled above, there's always at least eight bytes so this doesn't
       read past `end`. */
    {
        CORRADE_INTERNAL_DEBUG_ASSERT(data + 8 >= i && data + 8 <= end);
        out += _mm_popcnt_u64(*reinterpret_cast<const std::uint64_t*>(data) & initialMask & maskAfter((i - data)*8));
    }

    /* Process all aligned groups of eight bytes until we have the last 8 or
       less bytes left. The condition is < and not <= to ensure the last
       (potentially incomplete) byte is not processed here -- we need to avoid
       all masking in the fast path. */
    for(; i + 8 < end; i += 8)
        out += _mm_popcnt_u64(*reinterpret_cast<const std::uint64_t*>(i));

    /* Unconditionally process the last eight unaligned bytes, masking out
       *bytes* before `i` and *bits* after `bitEndOffset`. Again, as the
       shorter cases were handled above, there's always *at least* eight bytes
       so this doesn't before `data`, it's also always *more than* eight bytes
       since `data` so the mask is never all zeros. */
    {
        CORRADE_INTERNAL_DEBUG_ASSERT(end - 8 <= i && end - 8 > data);
        const char* const eightBytesBeforeEnd = end - 8;
        out += _mm_popcnt_u64(*reinterpret_cast<const std::uint64_t*>(eightBytesBeforeEnd) & maskBefore((i - eightBytesBeforeEnd)*8 & 0x3f) & maskAfter(bitEndOffset - (eightBytesBeforeEnd - data)*8));
    }

    return out;
}
#endif

}}}}

#endif
