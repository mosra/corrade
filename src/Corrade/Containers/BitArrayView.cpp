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

#include <cstdint>
#include <cstring>

#include "Corrade/Cpu.h"
#include "Corrade/Utility/Debug.h"
#ifdef CORRADE_ENABLE_POPCNT
#include "Corrade/Utility/IntrinsicsAvx.h"
#endif
#include "Corrade/Utility/Math.h"

namespace Corrade { namespace Containers {

/* Yes, I'm also surprised this works. On Windows (MSVC, clang-cl and MinGw) it
   needs an explicit export otherwise the symbol doesn't get exported. */
template<> template<> CORRADE_UTILITY_EXPORT void BasicBitArrayView<char>::setAll() const {
    /* If there are no bits to go through, bail. Otherwise the code touches at
       least one byte. */
    const std::size_t size = _sizeOffset >> 3;
    if(!size)
        return;

    std::uint8_t* const data = static_cast<std::uint8_t*>(_data);

    const std::size_t bitOffset = _sizeOffset & 0x07;
    const std::size_t bitEndOffset = bitOffset + size;
    /* All bits before `bitOffset` are 0, with `bitOffset == 0` this is 0xff */
    const std::uint8_t initialMask = ~((1 << bitOffset) - 1);
    /* All bits after `bitEndOffset % 7` are 0, with `bitEndOffset % 7 == 0`
       this is 0xff */
    /** @todo branch is ew! */
    const std::uint8_t finalMask = bitEndOffset & 0x07 ? (1ull << (bitEndOffset & 0x07)) - 1 : 0xff;

    /* A special case for when there's just one byte to modify, in which case
       we have to mask out both the initial and the final offset */
    if(bitEndOffset <= 8) {
        /* Keep bits before `bitOffset` and after `bitEndOffset`, set
           everything in between to 1 */
        data[0] |= initialMask & finalMask;
        return;
    }

    /* Keep bits before `bitOffset`, set everything after to 1 */
    data[0] |= initialMask;

    /* Last, potentially partial byte that we have to modify. Everything before
       is full bytes, for which we can efficiently use memset(). */
    const std::size_t lastByteOffset = (bitEndOffset - 1) >> 3;
    std::memset(data + 1, '\xff', lastByteOffset - 1);

    /* Keep bits after `bitEndOffset`, set everything before to 1 */
    data[lastByteOffset] |= finalMask;
}

/* Yes, I'm also surprised this works. On Windows (MSVC, clang-cl and MinGw) it
   needs an explicit export otherwise the symbol doesn't get exported. */
template<> template<> CORRADE_UTILITY_EXPORT void BasicBitArrayView<char>::resetAll() const {
    /* If there are no bits to go through, bail. Otherwise the code touches at
       least one byte. */
    const std::size_t size = _sizeOffset >> 3;
    if(!size)
        return;

    char* const data = static_cast<char*>(_data);

    const std::size_t bitOffset = _sizeOffset & 0x07;
    const std::size_t bitEndOffset = bitOffset + size;
    /* All bits after `bitOffset` are 0, with `bitOffset == 0` this is 0 */
    const std::uint8_t initialMask = (1ull << bitOffset) - 1;
    /* All bits before `bitEndOffset % 7` are 0, with `bitEndOffset % 7 == 0`
       this is 0 */
    /** @todo branch is ew! */
    const std::size_t finalMask = bitEndOffset & 0x07 ? ~((1ull << (bitEndOffset & 0x07)) - 1) : 0x00;

    /* A special case for when there's just one byte to modify, in which case
       we have to mask out both the initial and the final offset */
    if(bitEndOffset <= 8) {
        /* Keep bits before `bitOffset` and after `bitEndOffset`, zero-out
           everything in between */
        data[0] &= initialMask | finalMask;
        return;
    }

    /* Keep bits before `bitOffset`, zero-out everything after */
    data[0] &= initialMask;

    /* Last, potentially partial byte that we have to modify. Everything before
       is full bytes, for which we can efficiently use memset(). */
    const std::size_t lastByteOffset = (bitEndOffset - 1) >> 3;
    std::memset(data + 1, '\x00', lastByteOffset - 1);

    /* Keep bits after `bitEndOffset`, zero-out everything before */
    data[lastByteOffset] &= finalMask;
}

namespace Implementation {

namespace {

/* Platform-specific bit counting implementation, currently using just the
   64bit popcnt instructions if available and nothing fancier. The algorithm
   is split into four parts:

    1.  If there's less than 8 bytes in total, the code loads the data byte by
        byte, masks off both the initial bits and the final bits and returns a
        single 64-bit popcount.
    2.  Otherwise, an 8-bit aligned position `i` is calculated, which is now
        guaranteed to be in range of the view.
    3.  The first 8 (unaligned) bytes are loaded directly, the initial bits
        masked off and the bytes past `i` as well to not count them twice, and
        a popcount is executed on these.
    4.  A loop goes through groups of (aligned) 8 bytes until there's 8 or less
        left, executing popcount directly without any masking.
    5.  Finally, the last 8 bytes are loaded, the final bits masked off and
        the bytes already processed by step 4 as well to not count them twice,
        and a last popcount is executed on the result. */

/* Returns a value with bits below `before` and above and including `after` set
   to 0. For `before == 0` and `after == 64` returns all bits set. Can't be
   called with `before == 64` or `after == 0`. */
/** @todo when ANDed together, this is 8 operations, could probably do
    something closer to what _bextr_u64() does (4 operations,
    `(src >> before) & ((1 << size) - 1)`) however there it doesn't keep the
    bits in original positions which might be problematic in other uses of
    these helpers */
CORRADE_ALWAYS_INLINE std::uint64_t maskBefore(const std::uint64_t before) {
    CORRADE_INTERNAL_DEBUG_ASSERT(before < 64);
    return ~((1ull << before) - 1);
}
CORRADE_ALWAYS_INLINE std::uint64_t maskAfter(const std::uint64_t after) {
    CORRADE_INTERNAL_DEBUG_ASSERT(after && after <= 64);
    const std::uint64_t i = 1ull << (after - 1);
    return (i|(i - 1));
}
#if defined(CORRADE_ENABLE_POPCNT) && defined(CORRADE_ENABLE_BMI1) && !defined(CORRADE_TARGET_32BIT)
/* Like maskBefore() / maskAfter() above, but combined together into a single
   instruction. Gives the result right-shifted by `before` so it's not the same
   operation. */
CORRADE_ALWAYS_INLINE CORRADE_ENABLE(BMI1) std::uint64_t maskBeforeAfter(Cpu::Bmi1T, const std::uint64_t value, const std::uint64_t before, const std::uint64_t after) {
    CORRADE_INTERNAL_DEBUG_ASSERT(before < 64 && after > before && after <= 64);
    return _bextr_u64(value, before, after - before);
}
#endif

/* The 64-bit variants of POPCNT and BMI1 instructions aren't exposed on 32-bit
   systems for some reason. 32-bit x86 isn't that important nowadays so there
   it uses just the scalar code, I won't bother making a 32-bit variant. */
#if defined(CORRADE_ENABLE_POPCNT) && defined(CORRADE_ENABLE_BMI1) && !defined(CORRADE_TARGET_32BIT)
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE(POPCNT,BMI1) typename std::decay<decltype(bitCountSet)>::type bitCountSetImplementation(CORRADE_CPU_DECLARE(Cpu::Popcnt|Cpu::Bmi1)) {
  return [](const char* const data, const std::size_t bitOffset, const std::size_t size) CORRADE_ENABLE(POPCNT,BMI1) {
    CORRADE_ASSUME(bitOffset < 8);

    /* If there are no bits to go through, bail. This has to be here, because
       with non-zero `bitOffset` the code will always read at least 1 byte,
       which would crash with inaccessible or nullptr `data`. */
    if(!size)
        return std::size_t{0};

    const std::size_t bitEndOffset = bitOffset + size;
    /* Points past the last byte of which we take at least one bit */
    const char* const end = data + ((bitEndOffset + 7) >> 3);

    /* A special case for when we have 64 or less bits to process. Reading the
       bytes backwards, the last iteration then masks out bits both at the
       start and at the end and returns the popcount. */
    /** @todo Figure out a more optimized way to read the bytes, reading them
        this way makes the operation for 64 bits slower than two (overlapping)
        popcont() calls on >64 bits! OTOH here we explicitly handle the partial
        byte both at the start and at the end, which the overlapping sequence
        below doesn't (and putting it there would probably cause a slowdown for
        everything else). */
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
                    return std::size_t(_mm_popcnt_u64(maskBeforeAfter(Cpu::Bmi1, value.v, bitOffset, bitEndOffset)));
        }

        CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    /* Find the next 8-byte aligned position. Since it's always larger than
       `data`, it always has a full initial byte. And as the shorter cases were
       handled above, it's never past `end`. */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 8) & ~0x07);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && i <= end && reinterpret_cast<std::uintptr_t>(i) % 8 == 0);

    std::size_t out = 0;

    /* Unconditionally process the first eight unaligned bytes, masking out
       *bits* before `bitOffset` and *bytes* after `i`. As the shorter cases
       were handled above, there's always at least eight bytes so this doesn't
       read past `end`. */
    {
        CORRADE_INTERNAL_DEBUG_ASSERT(data + 8 >= i && data + 8 <= end);
        out += _mm_popcnt_u64(maskBeforeAfter(Cpu::Bmi1, *reinterpret_cast<const std::uint64_t*>(data), bitOffset, (i - data)*8));
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
        out += _mm_popcnt_u64(maskBeforeAfter(Cpu::Bmi1, *reinterpret_cast<const std::uint64_t*>(eightBytesBeforeEnd), (i - eightBytesBeforeEnd)*8, bitEndOffset - (eightBytesBeforeEnd - data)*8));
    }

    return out;
  };
}
#endif

/* http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel and
   https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation for a
   64-bit version. */
CORRADE_ALWAYS_INLINE std::uint64_t popcount(Cpu::ScalarT, std::uint64_t v) {
    /* On Emscripten this *does* expand to a WASM i64.popcnt instruction with
       -O3 (https://godbolt.org/z/4dhsKjq5e), but to play safe I'll just use
       the builtin directly. */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    return __builtin_popcountll(v);
    #else
    v = v - ((v >> 1) & 0x5555555555555555ull);
    v = (v & 0x3333333333333333ull) + ((v >> 2) & 0x3333333333333333ull);
    v = (v + (v >> 4)) & 0x0f0f0f0f0f0f0f0full;
    return (v*0x0101010101010101ull) >> 56;
    #endif
}

CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(bitCountSet)>::type bitCountSetImplementation(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
  return [](const char* const data, const std::size_t bitOffset, const std::size_t size) -> std::size_t {
    CORRADE_ASSUME(bitOffset < 8);

    /* If there are no bits to go through, bail. This has to be here, because
       with non-zero `bitOffset` the code will always read at least 1 byte,
       which would crash with inaccessible or nullptr `data`. */
    if(!size)
        return 0;

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
                    return popcount(Cpu::Scalar, value.v & initialMask & maskAfter(bitEndOffset));
        }

        CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    /* Find the next 8-byte aligned position. Since it's always larger than
       `data`, it always has a full initial byte. And as the shorter cases were
       handled above, it's never past `end`. */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 8) & ~0x07);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && i <= end && reinterpret_cast<std::uintptr_t>(i) % 8 == 0);

    std::size_t out = 0;

    /* Unconditionally process the first eight unaligned bytes, masking out
       *bits* before `bitOffset` and *bytes* after `i`. As the shorter cases
       were handled above, there's always at least eight bytes so this doesn't
       read past `end`. */
    {
        CORRADE_INTERNAL_DEBUG_ASSERT(data + 8 >= i && data + 8 <= end);
        out += popcount(Cpu::Scalar, *reinterpret_cast<const std::uint64_t*>(data) & initialMask & maskAfter((i - data)*8));
    }

    /* Process all aligned groups of eight bytes until we have the last 8 or
       less bytes left. The condition is < and not <= to ensure the last
       (potentially incomplete) byte is not processed here -- we need to avoid
       all masking in the fast path. */
    for(; i + 8 < end; i += 8)
        out += popcount(Cpu::Scalar, *reinterpret_cast<const std::uint64_t*>(i));

    /* Unconditionally process the last eight unaligned bytes, masking out
       *bytes* before `i` and *bits* after `bitEndOffset`. Again, as the
       shorter cases were handled above, there's always *at least* eight bytes
       so this doesn't before `data`, it's also always *more than* eight bytes
       since `data` so the mask is never all zeros. */
    {
        CORRADE_INTERNAL_DEBUG_ASSERT(end - 8 <= i && end - 8 > data);
        const char* const eightBytesBeforeEnd = end - 8;
        out += popcount(Cpu::Scalar, *reinterpret_cast<const std::uint64_t*>(eightBytesBeforeEnd) & maskBefore((i - eightBytesBeforeEnd)*8) & maskAfter(bitEndOffset - (eightBytesBeforeEnd - data)*8));
    }

    return out;
  };
}

}

#ifdef CORRADE_TARGET_X86
CORRADE_UTILITY_CPU_DISPATCHER(bitCountSetImplementation, Cpu::Popcnt, Cpu::Bmi1)
#else
CORRADE_UTILITY_CPU_DISPATCHER(bitCountSetImplementation)
#endif

CORRADE_UTILITY_CPU_DISPATCHED(bitCountSetImplementation, std::size_t CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(bitCountSet)(const char* data, std::size_t offset, std::size_t size))({
    return bitCountSetImplementation(CORRADE_CPU_SELECT(Cpu::Default))(data, offset, size);
})

}

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
