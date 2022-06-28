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
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayViewTest: TestSuite::Tester {
    explicit BitArrayViewTest();

    template<class T> void constructDefault();
    void constructDefaultConstexpr();
    template<class T> void construct();
    void constructConstexpr();
    void constructFromMutable();
    void constructOffsetTooLarge();
    void constructSizeTooLarge();

    void access();
    void accessMutable();
    void accessInvalid();

    void slice();
    void sliceInvalid();

    void debug();
};

BitArrayViewTest::BitArrayViewTest() {
    addTests({&BitArrayViewTest::constructDefault<const char>,
              &BitArrayViewTest::constructDefault<char>,
              &BitArrayViewTest::constructDefaultConstexpr,
              &BitArrayViewTest::construct<const char>,
              &BitArrayViewTest::construct<char>,
              &BitArrayViewTest::constructConstexpr,
              &BitArrayViewTest::constructFromMutable,
              &BitArrayViewTest::constructOffsetTooLarge,
              &BitArrayViewTest::constructSizeTooLarge,

              &BitArrayViewTest::access,
              &BitArrayViewTest::accessMutable,
              &BitArrayViewTest::accessInvalid,

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

    const BasicBitArrayView<T> view;
    CORRADE_VERIFY(view.isEmpty());
    CORRADE_COMPARE(view.offset(), 0);
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(static_cast<const void*>(view.data()), nullptr);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BasicBitArrayView<T>>::value);
}

void BitArrayViewTest::constructDefaultConstexpr() {
    constexpr BitArrayView view;
    constexpr bool empty = view.isEmpty();
    constexpr std::size_t offset = view.offset();
    constexpr std::size_t size = view.size();
    constexpr const void* data = view.data();
    CORRADE_VERIFY(empty);
    CORRADE_COMPARE(offset, 0);
    CORRADE_COMPARE(size, 0);
    CORRADE_COMPARE(data, nullptr);
}

template<class T> void BitArrayViewTest::construct() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint64_t data = 0x0fffff700u;
    const BasicBitArrayView<T> view{&data, 5, 47};
    CORRADE_VERIFY(!view.isEmpty());
    CORRADE_COMPARE(view.offset(), 5);
    CORRADE_COMPARE(view.size(), 47);
    CORRADE_COMPARE(static_cast<const void*>(view.data()), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicBitArrayView<T>, T*, std::size_t, std::size_t>::value);
}

constexpr char Data[8]{
    0,
    0,
    1 << 3, /* bit 15 << 4 */
    0,
    0,
    1 << 1, /* bit 37 << 4 */
    0,
    0
};

void BitArrayViewTest::constructConstexpr() {
    constexpr BitArrayView view{Data, 5, 47};
    constexpr bool empty = view.isEmpty();
    constexpr std::size_t offset = view.offset();
    constexpr std::size_t size = view.size();
    constexpr const void* data = view.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 47);
    CORRADE_COMPARE(data, &Data);
}

void BitArrayViewTest::constructFromMutable() {
    std::uint64_t data = 0x0fffff700u;
    const MutableBitArrayView a{&data, 5, 47};
    const BitArrayView b = a;

    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 47);
    CORRADE_COMPARE(static_cast<const void*>(b.data()), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<BitArrayView, MutableBitArrayView>::value);

    /* It shouldn't be possible the other way around */
    CORRADE_VERIFY(std::is_convertible<MutableBitArrayView, BitArrayView>::value);
    CORRADE_VERIFY(!std::is_convertible<BitArrayView, MutableBitArrayView>::value);
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

void BitArrayViewTest::access() {
    /* Alone, bit 15 is in byte 1 and bit 4 in byte 0, but together it's
       in byte 2 instead of byte 1 + 0 */
    const std::uint64_t data = ((1ull << 37) | (1ull << 15)) << 4;

    const BitArrayView view{&data, 4, 60};
    CORRADE_VERIFY(!view[14]);
    CORRADE_VERIFY(view[15]);
    CORRADE_VERIFY(!view[16]);

    CORRADE_VERIFY(!view[36]);
    CORRADE_VERIFY(view[37]);
    CORRADE_VERIFY(!view[38]);

    constexpr BitArrayView cview{Data, 4, 60};
    CORRADE_VERIFY(!cview[14]);
    CORRADE_VERIFY(cview[15]);
    CORRADE_VERIFY(!cview[16]);

    CORRADE_VERIFY(!cview[36]);
    CORRADE_VERIFY(cview[37]);
    CORRADE_VERIFY(!cview[38]);
}

void BitArrayViewTest::accessMutable() {
    std::uint64_t data = ((1ull << 36) | (1ull << 37) | (1ull << 38) | (1ull << 39) | (1ull << 40)) << 4;
    MutableBitArrayView view{&data, 4, 53};

    CORRADE_VERIFY(!view[15]);
    view.set(15);
    CORRADE_VERIFY(!view[14]);
    CORRADE_VERIFY(view[15]);
    CORRADE_VERIFY(!view[16]);

    CORRADE_VERIFY(view[37]);
    view.reset(37);
    CORRADE_VERIFY(view[36]);
    CORRADE_VERIFY(!view[37]);
    CORRADE_VERIFY(view[38]);

    CORRADE_VERIFY(!view[17]);
    view.set(17, true);
    CORRADE_VERIFY(!view[16]);
    CORRADE_VERIFY(view[17]);
    CORRADE_VERIFY(!view[18]);

    CORRADE_VERIFY(view[39]);
    view.set(39, false);
    CORRADE_VERIFY(view[38]);
    CORRADE_VERIFY(!view[39]);
    CORRADE_VERIFY(view[40]);
}

void BitArrayViewTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint64_t data;
    MutableBitArrayView view{&data, 4, 53};

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
    const std::uint64_t data{};
    BitArrayView view{&data, 6, 53};

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

    constexpr BitArrayView cview{Data, 6, 53};
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

    const std::uint64_t data{};
    BitArrayView view{&data, 6, 53};

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
    /* 0b111'1100'1100'1111'0000'0101'0101 << 7, which is
       {0b10000000, 0b00101010, 0b01111000, 0b11100110, 0b11} */
    const std::uint8_t data[]{0x80, 0x2a, 0x78, 0xe6, 0x03};
    Debug{&out} << BitArrayView{&data, 7, 27};
    CORRADE_COMPARE(out.str(), "{10101010, 00001111, 00110011, 111}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayViewTest)
