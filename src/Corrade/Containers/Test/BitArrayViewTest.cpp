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

#include <sstream>

#include "Corrade/Cpu.h"
#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Containers/Test/BitArrayViewTest.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/Utility/Test/cpuVariantHelpers.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayViewTest: TestSuite::Tester {
    explicit BitArrayViewTest();

    void captureImplementations();
    void restoreImplementations();

    template<class T> void constructDefault();
    template<class T> void constructFixedSize();
    void constructFixedSizeConstexpr();
    template<class T> void constructPointerOffsetSize();
    void constructPointerOffsetSizeConstexpr();
    template<class T> void constructFixedSizeOffsetSize();
    void constructFixedSizeOffsetSizeConstexpr();
    void constructFixedSizeOffsetSizeArrayTooSmall();
    void constructNullptrSize();

    void constructOffsetTooLarge();
    void constructSizeTooLarge();

    void constructFromMutable();
    void constructCopy();

    void access();
    void accessMutableSet();
    void accessMutableReset();
    void accessMutableSetAll();
    void accessMutableResetAll();
    void accessInvalid();

    void slice();
    void sliceInvalid();

    void countAllOnes();
    void countBitPattern();

    void debug();

    private:
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(Implementation::bitCountSet) bitCountSetImplementation;
        #endif
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    std::size_t offset;
    std::size_t bit;
    std::uint32_t valueSet;
    std::uint32_t expectedSet;
    std::uint32_t valueReset;
    std::uint32_t expectedReset;
} AccessMutableData[]{
    {"no-op", 0, 6,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset", 5, 1,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, overflow", 0, 13,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset, overflow", 6, 7,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"single bit", 0, 5,
        0x00000000u, 0x00000020u,
        0xffffffffu, 0xffffffdfu},
    {"single bit, offset", 3, 2,
        0x00000000u, 0x00000020u,
        0xffffffffu, 0xffffffdfu},
    {"single bit, overflow", 0, 21,
        0x00000000u, 0x00200000u,
        0xffffffffu, 0xffdfffffu},
    {"single bit, offset, overflow", 6, 15,
        0x00000000u, 0x00200000u,
        0xffffffffu, 0xffdfffffu},
    {"bit pattern", 0, 11,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
    {"bit pattern, offset", 4, 7,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
};

const struct {
    Cpu::Features features;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    std::size_t(*function)(const char*, std::size_t, std::size_t);
} CountData[]{
    {Cpu::Scalar, nullptr},
    /* The 64-bit variants of POPCNT and BMI1 instructions aren't exposed on
       32-bit systems, and no 32-bit fallback is implemented. See the source
       for details. */
    #if defined(CORRADE_ENABLE_POPCNT) && !defined(CORRADE_TARGET_32BIT)
    {Cpu::Popcnt, bitCountSetImplementationPopcnt},
    #endif
    #if defined(CORRADE_ENABLE_POPCNT) && defined(CORRADE_ENABLE_BMI1) && !defined(CORRADE_TARGET_32BIT)
    {Cpu::Popcnt|Cpu::Bmi1, nullptr},
    #endif
};

BitArrayViewTest::BitArrayViewTest() {
    addTests({&BitArrayViewTest::constructDefault<const char>,
              &BitArrayViewTest::constructDefault<char>,
              &BitArrayViewTest::constructFixedSize<const char>,
              &BitArrayViewTest::constructFixedSize<char>,
              &BitArrayViewTest::constructFixedSizeConstexpr,
              &BitArrayViewTest::constructPointerOffsetSize<const char>,
              &BitArrayViewTest::constructPointerOffsetSize<char>,
              &BitArrayViewTest::constructPointerOffsetSizeConstexpr,
              &BitArrayViewTest::constructFixedSizeOffsetSize<const char>,
              &BitArrayViewTest::constructFixedSizeOffsetSize<char>,
              &BitArrayViewTest::constructFixedSizeOffsetSizeConstexpr,
              &BitArrayViewTest::constructFixedSizeOffsetSizeArrayTooSmall,
              &BitArrayViewTest::constructNullptrSize,

              &BitArrayViewTest::constructOffsetTooLarge,
              &BitArrayViewTest::constructSizeTooLarge,

              &BitArrayViewTest::constructFromMutable,
              &BitArrayViewTest::constructCopy,

              &BitArrayViewTest::access});

    addInstancedTests({&BitArrayViewTest::accessMutableSet,
                       &BitArrayViewTest::accessMutableReset},
        Containers::arraySize(AccessMutableData));

    addTests({&BitArrayViewTest::accessMutableSetAll,
              &BitArrayViewTest::accessMutableResetAll,
              &BitArrayViewTest::accessInvalid,

              &BitArrayViewTest::slice,
              &BitArrayViewTest::sliceInvalid});

    addInstancedTests({&BitArrayViewTest::countAllOnes},
        Utility::Test::cpuVariantCount(CountData),
        &BitArrayViewTest::captureImplementations,
        &BitArrayViewTest::restoreImplementations);

    addRepeatedInstancedTests({&BitArrayViewTest::countBitPattern}, 64*187,
        Utility::Test::cpuVariantCount(CountData),
        &BitArrayViewTest::captureImplementations,
        &BitArrayViewTest::restoreImplementations);

    addTests({&BitArrayViewTest::debug});
}

template<class> struct NameFor;
template<> struct NameFor<const char> {
    static const char* name() { return "BitArrayView"; }
};
template<> struct NameFor<char> {
    static const char* name() { return "MutableBitArrayView"; }
};

void BitArrayViewTest::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    bitCountSetImplementation = Implementation::bitCountSet;
    #endif
}

void BitArrayViewTest::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    Implementation::bitCountSet = bitCountSetImplementation;
    #endif
}

template<class T> void BitArrayViewTest::constructDefault() {
    setTestCaseTemplateName(NameFor<T>::name());

    const BasicBitArrayView<T> a;
    const BasicBitArrayView<T> b = nullptr;
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(b.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(b.data(), nullptr);

    constexpr BasicBitArrayView<T> ca;
    constexpr BasicBitArrayView<T> cb = nullptr;
    constexpr bool emptyA = ca.isEmpty();
    constexpr bool emptyB = cb.isEmpty();
    constexpr std::size_t offsetA = ca.offset();
    constexpr std::size_t offsetB = cb.offset();
    constexpr std::size_t sizeA = ca.size();
    constexpr std::size_t sizeB = cb.size();
    constexpr const void* dataA = ca.data();
    constexpr const void* dataB = cb.data();
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(offsetA, 0);
    CORRADE_COMPARE(offsetB, 0);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);
    CORRADE_COMPARE(dataA, nullptr);
    CORRADE_COMPARE(dataB, nullptr);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BasicBitArrayView<T>>::value);
}

template<class T> void BitArrayViewTest::constructFixedSize() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint16_t data[7];
    const BasicBitArrayView<T> a = data;
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 7*16);
    CORRADE_COMPARE(a.data(), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, std::uint16_t(&)[7]>::value);
}

constexpr std::uint16_t Data16[7]{};

void BitArrayViewTest::constructFixedSizeConstexpr() {
    constexpr BitArrayView ca = Data16;
    constexpr bool empty = ca.isEmpty();
    constexpr std::size_t offset = ca.offset();
    constexpr std::size_t size = ca.size();
    constexpr const void* data = ca.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(offset, 0);
    CORRADE_COMPARE(size, 7*16);
    CORRADE_COMPARE(data, &Data16);
}

template<class T> void BitArrayViewTest::constructPointerOffsetSize() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint32_t data[1];
    const BasicBitArrayView<T> a = {data, 5, 24};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_COMPARE(a.size(), 24);
    CORRADE_COMPARE(a.data(), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, typename BasicBitArrayView<T>::ErasedType*, std::size_t, std::size_t>::value);
}

constexpr std::uint32_t Data[2]{};

void BitArrayViewTest::constructPointerOffsetSizeConstexpr() {
    constexpr BitArrayView ca = {Data + 1, 5, 24};
    constexpr bool empty = ca.isEmpty();
    constexpr std::size_t offset = ca.offset();
    constexpr std::size_t size = ca.size();
    constexpr const void* data = ca.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 24);
    CORRADE_COMPARE(data, Data + 1);
}

template<class T> void BitArrayViewTest::constructFixedSizeOffsetSize() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint16_t data[7];
    const BasicBitArrayView<T> a = {data, 5, 100};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_COMPARE(a.size(), 100);
    CORRADE_COMPARE(a.data(), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, std::uint16_t(&)[7]>::value);
}

void BitArrayViewTest::constructFixedSizeOffsetSizeConstexpr() {
    constexpr BitArrayView ca = {Data16, 5, 100};
    constexpr bool empty = ca.isEmpty();
    constexpr std::size_t offset = ca.offset();
    constexpr std::size_t size = ca.size();
    constexpr const void* data = ca.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 100);
    CORRADE_COMPARE(data, &Data16);
}

void BitArrayViewTest::constructFixedSizeOffsetSizeArrayTooSmall() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint16_t data[7];

    /* This is fine */
    BitArrayView{data, 5, 107};

    std::ostringstream out;
    Error redirectError{&out};
    /* Would pass without the offset */
    BitArrayView{data, 6, 107};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    CORRADE_COMPARE(out.str(), "Containers::BitArrayView: an array of 14 bytes is not enough for 6 + 107 bits\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArrayView: an array is not large enough for 6 + 107 bits\n");
    #endif
}

void BitArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later */

    BitArrayView a{nullptr, 5, 24};
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 24);

    constexpr BitArrayView ca{nullptr, 5, 24};
    CORRADE_COMPARE(ca.data(), nullptr);
    CORRADE_COMPARE(ca.offset(), 5);
    CORRADE_VERIFY(!ca.isEmpty());
    CORRADE_COMPARE(ca.size(), 24);
}

void BitArrayViewTest::constructOffsetTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArrayView{nullptr, 8, 0};
    CORRADE_COMPARE(out.str(), "Containers::BitArrayView: offset expected to be smaller than 8 bits, got 8\n");
}

void BitArrayViewTest::constructSizeTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArrayView{nullptr, 0, std::size_t{1} << (sizeof(std::size_t)*8 - 3)};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::BitArrayView: size expected to be smaller than 2^61 bits, got 2305843009213693952\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArrayView: size expected to be smaller than 2^29 bits, got 536870912\n");
    #endif
}

void BitArrayViewTest::constructFromMutable() {
    std::uint64_t data[1]{};
    const MutableBitArrayView a{data, 5, 47};
    const BitArrayView b = a;

    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 47);
    CORRADE_COMPARE(b.data(), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BitArrayView, MutableBitArrayView>::value);

    /* It shouldn't be possible the other way around. Not using is_convertible
       to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<BitArrayView, MutableBitArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<MutableBitArrayView, BitArrayView>::value);
}

void BitArrayViewTest::constructCopy() {
    std::uint64_t data[1]{};
    BitArrayView a{data, 5, 47};

    BitArrayView b = a;
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 47);
    CORRADE_COMPARE(b.data(), &data);

    BitArrayView c{&a, 0, 1};
    c = b;
    CORRADE_COMPARE(c.offset(), 5);
    CORRADE_COMPARE(c.size(), 47);
    CORRADE_COMPARE(c.data(), &data);

    CORRADE_VERIFY(std::is_copy_constructible<BitArrayView>::value);
    CORRADE_VERIFY(std::is_copy_assignable<BitArrayView>::value);
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    CORRADE_VERIFY(std::is_trivially_copy_constructible<BitArrayView>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<BitArrayView>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<BitArrayView>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<BitArrayView>::value);
}

/* 0b0101'0101'0011'0011'0000'1111'0000'0000 << 5 */
constexpr char DataPadded[]{'\x00', '\xe0', '\x61', '\xa6', '\x0a'};

void BitArrayViewTest::access() {
    const BitArrayView a{DataPadded + 1, 5, 24};

    for(std::size_t i: {0, 1, 2, 3, 8, 9, 12, 13, 16, 18, 20, 22}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
    }

    for(std::size_t i: {4, 5, 6, 7, 10, 11, 14, 15, 17, 19, 21, 23}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!a[i]);
    }
}

void BitArrayViewTest::accessMutableSet() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint32_t valueA[]{0, data.valueSet};
    std::uint32_t valueB[]{0, data.valueSet};
    const MutableBitArrayView a{valueA + 1, data.offset, 24};
    const MutableBitArrayView b{valueB + 1, data.offset, 24};

    a.set(data.bit);
    b.set(data.bit, true);
    CORRADE_COMPARE(valueA[1], data.expectedSet);
    CORRADE_COMPARE(valueB[1], data.expectedSet);
}

void BitArrayViewTest::accessMutableReset() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint32_t valueA[]{0, data.valueReset};
    std::uint32_t valueB[]{0, data.valueReset};
    const MutableBitArrayView a{valueA + 1, data.offset, 24};
    const MutableBitArrayView b{valueB + 1, data.offset, 24};

    a.reset(data.bit);
    b.set(data.bit, false);
    CORRADE_COMPARE(valueA[1], data.expectedReset);
    CORRADE_COMPARE(valueB[1], data.expectedReset);
}

void BitArrayViewTest::accessMutableSetAll() {
    /* Empty view with an offset */
    {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 5, 0}.setAll();
        CORRADE_COMPARE(a, 0x0000000000000000ull);

    /* One aligned byte */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 0, 8}.setAll();
        CORRADE_COMPARE(a, 0x00000000ff000000ull);

    /* Less than a byte with initial offset */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 2, 6}.setAll();
        CORRADE_COMPARE(a, 0x0000000fc000000ull);

    /* Less than a byte with final offset */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 0, 6}.setAll();
        CORRADE_COMPARE(a, 0x00000003f000000ull);

    /* Less than a byte with both initial and final offset */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 3, 2}.setAll();
        CORRADE_COMPARE(a, 0x000000018000000ull);

    /* Two aligned bytes */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 2, 0, 16}.setAll();
        CORRADE_COMPARE(a, 0x00000000ffff0000ull);

    /* Two bytes with initial and final offsets */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 2, 3, 10}.setAll();
        CORRADE_COMPARE(a, 0x000000001ff80000ull);

    /* Five bytes with initial and final offsets */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 1, 1, 38}.setAll();
        CORRADE_COMPARE(a, 0x00007ffffffffe00ull);

    /* Same as above, with a boolean argument */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 1, 1, 38}.setAll(true);
        CORRADE_COMPARE(a, 0x00007ffffffffe00ull);
    }
}

void BitArrayViewTest::accessMutableResetAll() {
    /* Empty view with an offset */
    {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 5, 0}.resetAll();
        CORRADE_COMPARE(a, 0xffffffffffffffffull);

    /* One aligned byte */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 0, 8}.resetAll();
        CORRADE_COMPARE(a, 0xffffffff00ffffffull);

    /* Less than a byte with initial offset */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 2, 6}.resetAll();
        CORRADE_COMPARE(a, 0xffffffff03ffffffull);

    /* Less than a byte with final offset */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 0, 6}.resetAll();
        CORRADE_COMPARE(a, 0xffffffffc0ffffffull);

    /* Less than a byte with both initial and final offset */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 3, 3, 2}.resetAll();
        CORRADE_COMPARE(a, 0xffffffffe7ffffffull);

    /* Two aligned bytes */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 2, 0, 16}.resetAll();
        CORRADE_COMPARE(a, 0xffffffff0000ffffull);

    /* Two bytes with initial and final offsets */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 2, 3, 10}.resetAll();
        CORRADE_COMPARE(a, 0xffffffffe007ffffull);

    /* Five bytes with initial and final offsets */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 1, 1, 38}.resetAll();
        CORRADE_COMPARE(a, 0xffff8000000001ffull);

    /* Same as above, with a boolean argument */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        MutableBitArrayView{reinterpret_cast<char*>(&a) + 1, 1, 38}.setAll(false);
        CORRADE_COMPARE(a, 0xffff8000000001ffull);
    }
}

void BitArrayViewTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint64_t data[1]{};
    MutableBitArrayView view{data, 4, 53};

    std::stringstream out;
    Error redirectError{&out};
    view[53];
    view.set(53);
    view.reset(53);
    view.set(53, true);
    CORRADE_COMPARE(out.str(),
        "Containers::BitArrayView::operator[](): index 53 out of range for 53 bits\n"
        "Containers::BitArrayView::set(): index 53 out of range for 53 bits\n"
        "Containers::BitArrayView::reset(): index 53 out of range for 53 bits\n"
        "Containers::BitArrayView::set(): index 53 out of range for 53 bits\n");
}

void BitArrayViewTest::slice() {
    const char data64[8]{};
    BitArrayView view{data64, 6, 53};

    /* There isn't really any value to easily compare to, so go the hard way
       and compare pointers, offsets and sizes */
    {
        BitArrayView slice = view.slice(29, 47);
        CORRADE_COMPARE(slice.data(), data64 + 4);
        CORRADE_COMPARE(slice.offset(), 3);
        CORRADE_COMPARE(slice.size(), 18);
    } {
        BitArrayView slice = view.prefix(12);
        CORRADE_COMPARE(slice.data(), data64);
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        BitArrayView slice = view.suffix(12);
        CORRADE_COMPARE(slice.data(), data64 + 5);
        CORRADE_COMPARE(slice.offset(), 7);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        BitArrayView slice = view.exceptPrefix(12);
        CORRADE_COMPARE(slice.data(), data64 + 2);
        CORRADE_COMPARE(slice.offset(), 2);
        CORRADE_COMPARE(slice.size(), 41);
    } {
        BitArrayView slice = view.exceptSuffix(12);
        CORRADE_COMPARE(slice.data(), data64);
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 41);
    }
}

void BitArrayViewTest::sliceInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    const std::uint64_t data[1]{};
    BitArrayView view{data, 6, 53};

    std::ostringstream out;
    Error redirectError{&out};
    view.slice(47, 54);
    view.slice(47, 46);
    CORRADE_COMPARE(out.str(),
        "Containers::BitArrayView::slice(): slice [47:54] out of range for 53 bits\n"
        "Containers::BitArrayView::slice(): slice [47:46] out of range for 53 bits\n");
}

void BitArrayViewTest::countAllOnes() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Empty without and with offset, shouldn't attempt to read anything */
    {
        CORRADE_COMPARE((BitArrayView{nullptr, 0, 0}.count()), 0);
        CORRADE_COMPARE((BitArrayView{nullptr, 5, 0}.count()), 0);

    /* Less than 64 bits, should go just through the special case */
    } {
        const std::uint64_t ones[]{~0ull};

        /* Byte-aligned */
        CORRADE_COMPARE((BitArrayView{ones, 0, 56}.count()), 56);

        /* 7 bit offset at the front */
        CORRADE_COMPARE((BitArrayView{ones, 7, 49}.count()), 49);

        /* 7 bit offset at the back */
        CORRADE_COMPARE((BitArrayView{ones, 0, 49}.count()), 49);

        /* 3- and 4-bit offset at both sides */
        CORRADE_COMPARE((BitArrayView{ones, 3, 49}.count()), 49);

    /* Exactly 64 bits, should again go just through the special case (see the
       source for why) */
    } {
        const std::uint64_t ones[]{~0ull};

        /* Byte-aligned */
        CORRADE_COMPARE((BitArrayView{ones, 0, 64}.count()), 64);

        /* 7 bit offset at the front */
        CORRADE_COMPARE((BitArrayView{ones, 7, 57}.count()), 57);

        /* 7 bit offset at the back */
        CORRADE_COMPARE((BitArrayView{ones, 0, 57}.count()), 57);

        /* 3- and 4-bit offset at both sides */
        CORRADE_COMPARE((BitArrayView{ones, 3, 57}.count()), 57);

    /* 128 bits, should go just through the initial and final masking section
       with no overlap */
    } {
        const std::uint64_t ones[]{~0ull, ~0ull};

        /* Byte-aligned */
        CORRADE_COMPARE((BitArrayView{ones, 0, 128}.count()), 128);

        /* 7 bit offset at the front */
        CORRADE_COMPARE((BitArrayView{ones, 7, 121}.count()), 121);

        /* 7 bit offset at the back */
        CORRADE_COMPARE((BitArrayView{ones, 0, 121}.count()), 121);

        /* 4- and 3-bit offset at both sides */
        CORRADE_COMPARE((BitArrayView{ones, 4, 121}.count()), 121);

    /* Less than 128 bits, should go through the initial and final masking
       sections with overlap */
    } {
        const std::uint64_t ones[]{~0ull, ~0ull};

        /* Byte-aligned, 1 byte overlap from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 1, 0, 120}.count()), 120);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 0, 120}.count()), 120);

        /* Byte-aligned, 7 byte overlap from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 0, 72}.count()), 72);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 0, 72}.count()), 72);

        /* 7 bit offset at the front, 7 byte overlap from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 7, 65}.count()), 65);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 7, 65}.count()), 65);

        /* 7 bit offset at the back, 7 byte overlap from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 0, 65}.count()), 65);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 0, 65}.count()), 65);

        /* 3- and 4-bit offset at both sides */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 3, 65}.count()), 65);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 3, 65}.count()), 65);

    /* More than 128 bits, should go through also the middle section */
    } {
        const std::uint64_t ones[]{~0ull, ~0ull, ~0ull, ~0ull};

        /* 64-bit-aligned, no overlap */
        CORRADE_COMPARE((BitArrayView{ones}.count()), 256);

        /* Byte-aligned, 1 byte overlap with the middle section from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 1, 0, 248}.count()), 248);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 0, 248}.count()), 248);

        /* Byte-aligned, 7 byte overlap with the middle section from either side */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 0, 200}.count()), 200);
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 0, 0, 200}.count()), 200);

        /* Byte-aligned, 7 byte overlap with the middle section from both sides */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 0, 144}.count()), 144);

        /* 1 bit offset at the front and at the back, 7 byte overlap from both
           sides */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 1, 142}.count()), 142);

        /* 7 bit offset at the front and at the back, 7 byte overlap from both
           sides */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 7, 7, 130}.count()), 130);

        /* 3- and 4-bit offset at the front and at the back, 4- and 3- byte overlap
           from both sides */
        CORRADE_COMPARE((BitArrayView{reinterpret_cast<const char*>(ones) + 4, 3, 193}.count()), 193);
    }
}

void BitArrayViewTest::countBitPattern()  {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* A bit pattern with groups of 8/4/2/1 ones and zeros, then 9/3/1 ones and
       zeros and then 10/5/2/1 ones and zeros, 187 bits in total:

        0b___010'10101010'01100110'01100110'01111111'00000000'00111110'00000000
        01111111'11101010'10100101'01010001'11000111'00011100'00000001'11111111
        10101010'01010101'11001100'00110011'11110000'00001111'00000000'11111111

       The values are then shifted by 0 to 63 bits, a prefix is taken and it's
       expected that the calculated count is always the same for given size
       regardless of the shift. */
    constexpr std::uint64_t bits[]{
        0x0000000000000000ull,
        0xaa55cc33f00f00ffull,
        0x7feaa551c71c01ffull,
        0x02aa66667f003e00ull,
        0x0000000000000000ull
    };

    /* There's 64*187 repeats, shift ranges from 0 to 63 and size from 1 to
       187 */
    const std::uint32_t shift = testCaseRepeatId() & 0x3f;
    const std::uint32_t size = (testCaseRepeatId() >> 6) + 1;
    CORRADE_COMPARE_AS(shift, 63, TestSuite::Compare::LessOrEqual);
    CORRADE_COMPARE_AS(size, 187, TestSuite::Compare::LessOrEqual);

    std::uint8_t bitsShifted[4*8];
    for(std::size_t i = 0; i != 4; ++i) {
        std::uint64_t shifted = (bits[i + 1] << shift);
        if(shift) shifted |= bits[i] >> (64 - shift);
        for(std::size_t j = 0; j != 8; ++j)
            bitsShifted[i*8 + j] = (shifted >> j*8) & 0xff;
    }

    Containers::BitArrayView view{bitsShifted + (shift >> 3), shift & 0x07, size};

    /* Expected bit counts, should be the same for given size regardless of
       shift */
    std::size_t expected[187]{
         1,  2,  3,  4,  5,  6,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,
         9, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 14, 15, 16,
        17, 18, 18, 18, 19, 20, 20, 20, 20, 20, 21, 22, 22, 22, 23, 24,
        25, 25, 26, 26, 27, 27, 28, 28, 28, 29, 29, 30, 30, 31, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40, 41, 41, 41, 41, 41, 41, 41, 41,
        41, 41, 42, 43, 44, 44, 44, 44, 45, 46, 47, 47, 47, 47, 48, 49,
        50, 50, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 54, 55, 55, 56,
        56, 57, 57, 58, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 68,
        68, 68, 68, 68, 68, 68, 68, 68, 68, 69, 70, 71, 72, 73, 73, 73,
        73, 73, 73, 73, 73, 73, 73, 73, 74, 75, 76, 77, 78, 79, 80, 80,
        80, 81, 82, 82, 82, 83, 84, 84, 84, 85, 86, 86, 86, 87, 88, 88,
        88, 89, 89, 90, 90, 91, 91, 92, 92, 93, 93,
    };

    /* Verify that we have the shift correct with the naive counting first */
    std::size_t naiveCount = 0;
    for(std::size_t i = 0; i != size; ++i) {
        if(view[i]) ++naiveCount;
    }

    /* Set to 1 to generate data for the above table */
    #if 0
    if(shift == 0) {
        Debug d{Debug::Flag::NoNewlineAtTheEnd};
        if(naiveCount < 10) d << " " << Debug::nospace;
        d << naiveCount << Debug::nospace << ",";
        if(size % 16 == 0 || size == 187) d << Debug::newline;
        else d << Debug::space;
    }
    #else
    CORRADE_FAIL_IF(naiveCount != expected[size - 1],
        "Naive count" << naiveCount << "expected to be" << expected[size - 1] << "for" << view << "with shift" << shift << "and size" << size);
    CORRADE_FAIL_IF(view.count() != expected[size - 1],
        "Count" << view.count() << "expected to be" << expected[size - 1] << "for" << view << "with shift" << shift << "and size" << size);
    #endif
}

void BitArrayViewTest::debug() {
    /* 0b0101'0101'0011'0011'0000'1111 << 5, printed in reverse (first bit
       first), smaller sizes should cut away the last bits */
    char data[]{'\xe0', '\x61', '\xa6', '\x0a'};

    std::ostringstream out;
    Debug{&out} << BitArrayView{DataPadded + 1, 5, 24};
    Debug{&out} << MutableBitArrayView{data, 5, 24};
    Debug{&out} << BitArrayView{DataPadded + 1, 5, 19};
    CORRADE_COMPARE(out.str(),
        "{11110000, 11001100, 10101010}\n"
        "{11110000, 11001100, 10101010}\n"
        "{11110000, 11001100, 101}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayViewTest)
