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

#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayViewTest: TestSuite::Tester {
    explicit BitArrayViewTest();

    template<class T> void constructDefault();
    template<class T> void constructPointerSize();
    template<class T> void constructPointerSizeChar();
    void constructPointerSizeCharConstexpr();
    void constructPointerSizeNullptr();
    void constructNullptrSize();

    void constructOffsetTooLarge();
    void constructSizeTooLarge();

    void constructFromMutable();
    void constructCopy();

    void access();
    void accessMutableSet();
    void accessMutableReset();
    void accessInvalid();

    void slice();
    void sliceInvalid();

    void debug();
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

BitArrayViewTest::BitArrayViewTest() {
    addTests({&BitArrayViewTest::constructDefault<const char>,
              &BitArrayViewTest::constructDefault<char>,
              &BitArrayViewTest::constructPointerSize<const char>,
              &BitArrayViewTest::constructPointerSize<char>,
              &BitArrayViewTest::constructPointerSizeChar<const char>,
              &BitArrayViewTest::constructPointerSizeChar<char>,
              &BitArrayViewTest::constructPointerSizeCharConstexpr,
              &BitArrayViewTest::constructPointerSizeNullptr,
              &BitArrayViewTest::constructNullptrSize,

              &BitArrayViewTest::constructOffsetTooLarge,
              &BitArrayViewTest::constructSizeTooLarge,

              &BitArrayViewTest::constructFromMutable,
              &BitArrayViewTest::constructCopy,

              &BitArrayViewTest::access});

    addInstancedTests({&BitArrayViewTest::accessMutableSet,
                       &BitArrayViewTest::accessMutableReset},
        Containers::arraySize(AccessMutableData));

    addTests({&BitArrayViewTest::accessInvalid,

              &BitArrayViewTest::slice,
              &BitArrayViewTest::sliceInvalid,

              &BitArrayViewTest::debug});
}

template<class> struct NameFor;
template<> struct NameFor<const char> {
    static const char* name() { return "BitArrayView"; }
};
template<> struct NameFor<char> {
    static const char* name() { return "MutableBitArrayView"; }
};

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
    CORRADE_COMPARE(static_cast<const void*>(a.data()), nullptr);
    CORRADE_COMPARE(static_cast<const void*>(b.data()), nullptr);

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

template<class T> void BitArrayViewTest::constructPointerSize() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint32_t data[1];
    const BasicBitArrayView<T> a{data, 5, 24};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_COMPARE(a.size(), 24);
    CORRADE_COMPARE(static_cast<const void*>(a.data()), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, typename BasicBitArrayView<T>::ErasedType*, std::size_t, std::size_t>::value);
}

template<class T> void BitArrayViewTest::constructPointerSizeChar() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[4];
    const BasicBitArrayView<T> a{data, 5, 24};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_COMPARE(a.size(), 24);
    CORRADE_COMPARE(static_cast<const void*>(a.data()), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, T*, std::size_t, std::size_t>::value);
}

constexpr const char FourChars[4]{};

void BitArrayViewTest::constructPointerSizeCharConstexpr() {
    constexpr BitArrayView ca{FourChars, 5, 24};
    constexpr bool empty = ca.isEmpty();
    constexpr std::size_t offset = ca.offset();
    constexpr std::size_t size = ca.size();
    constexpr const void* data = ca.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 24);
    CORRADE_COMPARE(data, &FourChars);
}

void BitArrayViewTest::constructPointerSizeNullptr() {
    /* An explicit overload to avoid ambiguity between the char* and void*
       constructor when passing std::nullptr_t */

    BitArrayView a{nullptr, 5, 24};
    CORRADE_COMPARE(static_cast<const void*>(a.data()), nullptr);
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 24);

    constexpr BitArrayView ca{nullptr, 5, 24};
    CORRADE_COMPARE(static_cast<const void*>(ca.data()), nullptr);
    CORRADE_COMPARE(ca.offset(), 5);
    CORRADE_VERIFY(!ca.isEmpty());
    CORRADE_COMPARE(ca.size(), 24);

    CORRADE_VERIFY(std::is_nothrow_constructible<BitArrayView, std::nullptr_t, std::size_t, std::size_t>::value);
}

void BitArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later. Explicitly casting to not pick the
       std::nullptr_t overload that's tested in
       constructPointerSizeNullptr(). */

    BitArrayView a{static_cast<const char*>(nullptr), 5, 24};
    CORRADE_COMPARE(static_cast<const void*>(a.data()), nullptr);
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 24);

    constexpr BitArrayView ca{static_cast<const char*>(nullptr), 5, 24};
    CORRADE_COMPARE(static_cast<const void*>(ca.data()), nullptr);
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
    CORRADE_COMPARE(static_cast<const void*>(b.data()), &data);

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
    CORRADE_COMPARE(static_cast<const void*>(b.data()), &data);

    BitArrayView c{&a, 0, 1};
    c = b;
    CORRADE_COMPARE(c.offset(), 5);
    CORRADE_COMPARE(c.size(), 47);
    CORRADE_COMPARE(static_cast<const void*>(c.data()), &data);

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

    constexpr BitArrayView ca{DataPadded + 1, 5, 24};
    constexpr bool ca15 = ca[15];
    constexpr bool ca16 = ca[16];
    constexpr bool ca17 = ca[17];
    CORRADE_VERIFY(!ca15);
    CORRADE_VERIFY(ca16);
    CORRADE_VERIFY(!ca17);
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

constexpr char Data64[8]{};

void BitArrayViewTest::slice() {
    const std::uint64_t data64[1]{};
    BitArrayView view{data64, 6, 53};

    /* There isn't really any value to easily compare to, so go the hard way
       and compare pointers, offsets and sizes */
    {
        BitArrayView slice = view.slice(29, 47);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), view.data() + 4);
        CORRADE_COMPARE(slice.offset(), 3);
        CORRADE_COMPARE(slice.size(), 18);
    } {
        BitArrayView slice = view.prefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), view.data());
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        BitArrayView slice = view.suffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), view.data() + 5);
        CORRADE_COMPARE(slice.offset(), 7);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        BitArrayView slice = view.exceptPrefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), view.data() + 2);
        CORRADE_COMPARE(slice.offset(), 2);
        CORRADE_COMPARE(slice.size(), 41);
    } {
        BitArrayView slice = view.exceptSuffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), view.data());
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 41);
    }

    constexpr BitArrayView cview{Data64, 6, 53};
    {
        constexpr BitArrayView slice = cview.slice(29, 47);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), cview.data() + 4);
        CORRADE_COMPARE(slice.offset(), 3);
        CORRADE_COMPARE(slice.size(), 18);
    } {
        constexpr BitArrayView slice = cview.prefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), cview.data());
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        constexpr BitArrayView slice = cview.suffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), cview.data() + 5);
        CORRADE_COMPARE(slice.offset(), 7);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        constexpr BitArrayView slice = cview.exceptPrefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), cview.data() + 2);
        CORRADE_COMPARE(slice.offset(), 2);
        CORRADE_COMPARE(slice.size(), 41);
    } {
        constexpr BitArrayView slice = cview.exceptSuffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), cview.data());
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

void BitArrayViewTest::debug() {
    std::ostringstream out;
    Debug{&out} << BitArrayView{DataPadded + 1, 5, 24};
    Debug{&out} << BitArrayView{DataPadded + 1, 5, 19};
    CORRADE_COMPARE(out.str(),
        "{11110000, 11001100, 10101010}\n"
        "{11110000, 11001100, 101}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayViewTest)
