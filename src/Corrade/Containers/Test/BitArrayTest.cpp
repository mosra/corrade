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

#include "Corrade/Containers/BitArray.h"
#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayTest: TestSuite::Tester {
    explicit BitArrayTest();

    void constructDefault();
    void constructValueInit();
    void constructValueInitZeroSize();
    void constructValueInitSizeTooLarge();
    void constructDirectInit();
    void constructDirectInitZeroSize();
    void constructDirectInitSizeTooLarge();
    void constructNoInit();
    void constructNoInitZeroSize();
    void constructNoInitSizeTooLarge();
    void constructTakeOwnership();
    void constructTakeOwnershipOffsetTooLarge();
    void constructTakeOwnershipSizeTooLarge();
    void constructMove();

    void convertView();
    void convertMutableView();

    void access();
    void accessMutable();
    void accessInvalid();

    template<class T> void slice();

    void release();

    void defaultDeleter();
    void customDeleter();
    void customDeleterNullData();
    void customDeleterZeroSize();
    void customDeleterMovedOutInstance();

    void debug();
};

const struct {
    const char* name;
    bool value;
} ConstructDirectInitData[]{
    {"true", true},
    {"false", false}
};

BitArrayTest::BitArrayTest() {
    addTests({&BitArrayTest::constructDefault,
              &BitArrayTest::constructValueInit,
              &BitArrayTest::constructValueInitZeroSize,
              &BitArrayTest::constructValueInitSizeTooLarge});

    addInstancedTests({&BitArrayTest::constructDirectInit},
        Containers::arraySize(ConstructDirectInitData));

    addTests({&BitArrayTest::constructDirectInitZeroSize,
              &BitArrayTest::constructDirectInitSizeTooLarge,
              &BitArrayTest::constructNoInit,
              &BitArrayTest::constructNoInitZeroSize,
              &BitArrayTest::constructNoInitSizeTooLarge,
              &BitArrayTest::constructTakeOwnership,
              &BitArrayTest::constructTakeOwnershipOffsetTooLarge,
              &BitArrayTest::constructTakeOwnershipSizeTooLarge,
              &BitArrayTest::constructMove,

              &BitArrayTest::convertView,
              &BitArrayTest::convertMutableView,

              &BitArrayTest::access,
              &BitArrayTest::accessMutable,
              &BitArrayTest::accessInvalid,

              &BitArrayTest::slice<const BitArray>,
              &BitArrayTest::slice<BitArray>,

              &BitArrayTest::release,

              &BitArrayTest::defaultDeleter,
              &BitArrayTest::customDeleter,
              &BitArrayTest::customDeleterNullData,
              &BitArrayTest::customDeleterZeroSize,
              &BitArrayTest::customDeleterMovedOutInstance,

              &BitArrayTest::debug});
}

template<class T> struct ConstTraits;
template<> struct ConstTraits<BitArray> {
    typedef MutableBitArrayView ViewType;
    static const char* name() { return "BitArray"; }
};
template<> struct ConstTraits<const BitArray> {
    typedef BitArrayView ViewType;
    static const char* name() { return "const BitArray"; }
};

void BitArrayTest::constructDefault() {
    BitArray a;
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(!a.data());
}

void BitArrayTest::constructValueInit() {
    BitArray a{Corrade::ValueInit, 97};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 97);
    CORRADE_VERIFY(a.data());

    for(std::size_t i = 0; i != a.size(); ++i) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!a[i]);
    }
}

void BitArrayTest::constructValueInitZeroSize() {
    BitArray a{Corrade::ValueInit, 0};
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(!a.data());
}

void BitArrayTest::constructValueInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArray{Corrade::ValueInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3)};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^61 bits, got 2305843009213693952\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
    #endif
}

void BitArrayTest::constructDirectInit() {
    auto&& data = ConstructDirectInitData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    BitArray a{Corrade::DirectInit, 97, data.value};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 97);
    CORRADE_VERIFY(a.data());

    for(std::size_t i = 0; i != a.size(); ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(a[i], data.value);
    }
}

void BitArrayTest::constructDirectInitZeroSize() {
    BitArray a{Corrade::DirectInit, 0, true};
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(!a.data());
}

void BitArrayTest::constructDirectInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArray{Corrade::DirectInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3), true};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^61 bits, got 2305843009213693952\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
    #endif
}

void BitArrayTest::constructNoInit() {
    BitArray a{Corrade::NoInit, 97};
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 97);
    CORRADE_VERIFY(a.data());
    /* Contents can be just anything */
}

void BitArrayTest::constructNoInitZeroSize() {
    BitArray a{Corrade::NoInit, 0};
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(!a.data());
}

void BitArrayTest::constructNoInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArray{Corrade::NoInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3)};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^61 bits, got 2305843009213693952\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
    #endif
}

void BitArrayTest::constructTakeOwnership() {
    /* Arguments passed to deleter and cases when deleter is called tested more
       thoroughly in customDeleter*() */

    std::uint64_t data{};
    {
        BitArray a{&data, 5, 52, [](char* data, std::size_t size) {
            ++data[0];
            ++data[size - 1];
        }};
        CORRADE_VERIFY(!a.isEmpty());
        CORRADE_COMPARE(a.offset(), 5);
        CORRADE_COMPARE(a.size(), 52);
        CORRADE_COMPARE(static_cast<const void*>(a.data()), &data);
        CORRADE_VERIFY(a.deleter());
    }

    CORRADE_COMPARE(data, (1ull << 56) | 1ull);
}

void BitArrayTest::constructTakeOwnershipOffsetTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArray{nullptr, 8, 0, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out.str(), "Containers::BitArray: offset expected to be smaller than 8 bits, got 8\n");
}

void BitArrayTest::constructTakeOwnershipSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    BitArray{nullptr, 0, std::size_t{1} << (sizeof(std::size_t)*8 - 3), [](char*, std::size_t) {}};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^61 bits, got 2305843009213693952\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
    #endif
}

void BitArrayTest::constructMove() {
    auto myDeleter = [](char* data, std::size_t) {
        delete[] data;
    };
    BitArray a{new char[5], 7, 31, myDeleter};
    char* data = a.data();
    CORRADE_VERIFY(data);

    BitArray b{Utility::move(a)};
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), 31);
    CORRADE_VERIFY(!a.deleter());
    CORRADE_VERIFY(b.deleter() == myDeleter);

    auto noDeleter = [](char*, std::size_t) {};
    BitArray c{reinterpret_cast<char*>(0x3), 2, 3, noDeleter};
    c = Utility::move(b);
    CORRADE_COMPARE(b.data(), reinterpret_cast<const void*>(0x3));
    CORRADE_COMPARE(c.data(), data);
    CORRADE_COMPARE(b.offset(), 2);
    CORRADE_COMPARE(c.offset(), 7);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(c.size(), 31);
    CORRADE_VERIFY(b.deleter() == noDeleter);
    CORRADE_VERIFY(c.deleter() == myDeleter);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<BitArray>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<BitArray>::value);
}

void BitArrayTest::convertView() {
    std::uint64_t data{};
    const BitArray a{&data, 7, 31, [](char*, std::size_t) {}};

    const BitArrayView cb = a;
    CORRADE_COMPARE(cb.data(), reinterpret_cast<const void*>(&data));
    CORRADE_COMPARE(cb.offset(), 7);
    CORRADE_COMPARE(cb.size(), 31);

    /* It shouldn't be possible to create a mutable view from a const BitArray */
    CORRADE_VERIFY(std::is_convertible<const BitArray, BitArrayView>::value);
    CORRADE_VERIFY(!std::is_convertible<const BitArray, MutableBitArrayView>::value);
}

void BitArrayTest::convertMutableView() {
    std::uint64_t data{};
    BitArray a{&data, 7, 31, [](char*, std::size_t) {}};

    const MutableBitArrayView b = a;
    CORRADE_COMPARE(b.data(), reinterpret_cast<const void*>(&data));
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), 31);

    const BitArrayView cb = a;
    CORRADE_COMPARE(cb.data(), reinterpret_cast<const void*>(&data));
    CORRADE_COMPARE(cb.offset(), 7);
    CORRADE_COMPARE(cb.size(), 31);
}

void BitArrayTest::access() {
    /* Mostly the same as BitArrayViewTest::access(), except that it's a
       non-owning BitArray */

    /* Alone, bit 15 is in byte 1 and bit 4 in byte 0, but together it's
       in byte 2 instead of byte 1 + 0 */
    std::uint64_t data = ((1ull << 37) | (1ull << 15)) << 4;
    const BitArray array{&data, 4, 60, [](char*, std::size_t) {}};

    CORRADE_VERIFY(!array[14]);
    CORRADE_VERIFY(array[15]);
    CORRADE_VERIFY(!array[16]);

    CORRADE_VERIFY(!array[36]);
    CORRADE_VERIFY(array[37]);
    CORRADE_VERIFY(!array[38]);
}

void BitArrayTest::accessMutable() {
    /* Mostly the same as BitArrayViewTest::accessMutable(), except that it's a
       non-owning BitArray */

    std::uint64_t data = ((1ull << 36) | (1ull << 37) | (1ull << 38) | (1ull << 39) | (1ull << 40)) << 4;
    BitArray array{&data, 4, 53, [](char*, std::size_t) {}};

    CORRADE_VERIFY(!array[15]);
    array.set(15);
    CORRADE_VERIFY(!array[14]);
    CORRADE_VERIFY(array[15]);
    CORRADE_VERIFY(!array[16]);

    CORRADE_VERIFY(array[37]);
    array.reset(37);
    CORRADE_VERIFY(array[36]);
    CORRADE_VERIFY(!array[37]);
    CORRADE_VERIFY(array[38]);

    CORRADE_VERIFY(!array[17]);
    array.set(17, true);
    CORRADE_VERIFY(!array[16]);
    CORRADE_VERIFY(array[17]);
    CORRADE_VERIFY(!array[18]);

    CORRADE_VERIFY(array[39]);
    array.set(39, false);
    CORRADE_VERIFY(array[38]);
    CORRADE_VERIFY(!array[39]);
    CORRADE_VERIFY(array[40]);
}

void BitArrayTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint64_t data;
    BitArray array{&data, 4, 53, [](char*, std::size_t) {}};

    std::stringstream out;
    Error redirectError{&out};
    array[53];
    array.set(53);
    array.reset(53);
    array.set(53, true);
    CORRADE_COMPARE(out.str(),
        "Containers::BitArray::operator[](): index 53 out of range for 53 bits\n"
        "Containers::BitArray::set(): index 53 out of range for 53 bits\n"
        "Containers::BitArray::reset(): index 53 out of range for 53 bits\n"
        "Containers::BitArray::set(): index 53 out of range for 53 bits\n");
}

template<class T> void BitArrayTest::slice() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    std::uint64_t data{};
    T a{&data, 6, 53, [](char*, std::size_t) {}};

    /* These delegate to BitArrayView so we only need to verify that a correct
       function gets propagated, not everything */
    {
        typename ConstTraits<T>::ViewType slice = a.slice(29, 47);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data() + 4);
        CORRADE_COMPARE(slice.offset(), 3);
        CORRADE_COMPARE(slice.size(), 18);
    } {
        typename ConstTraits<T>::ViewType slice = a.sliceSize(29, 18);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data() + 4);
        CORRADE_COMPARE(slice.offset(), 3);
        CORRADE_COMPARE(slice.size(), 18);
    } {
        typename ConstTraits<T>::ViewType slice = a.prefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data());
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        typename ConstTraits<T>::ViewType slice = a.suffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data() + 5);
        CORRADE_COMPARE(slice.offset(), 7);
        CORRADE_COMPARE(slice.size(), 12);
    } {
        typename ConstTraits<T>::ViewType slice = a.exceptPrefix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data() + 2);
        CORRADE_COMPARE(slice.offset(), 2);
        CORRADE_COMPARE(slice.size(), 41);
    } {
        typename ConstTraits<T>::ViewType slice = a.exceptSuffix(12);
        CORRADE_COMPARE(static_cast<const void*>(slice.data()), a.data());
        CORRADE_COMPARE(slice.offset(), 6);
        CORRADE_COMPARE(slice.size(), 41);
    }
}

void BitArrayTest::release() {
    std::uint64_t data{};
    BitArray a{&data, 6, 53, [](char*, std::size_t) {}};

    const void* released = a.release();
    CORRADE_COMPARE(released, &data);

    /* Post-release state should be the same as of a default-constructed
       instance -- with zero offset, size and data */
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(!a.data());
}

void BitArrayTest::defaultDeleter() {
    BitArray a{Corrade::ValueInit, 97};
    CORRADE_VERIFY(a.deleter() == nullptr);
}

int CustomDeleterCallCount = 0;

void BitArrayTest::customDeleter() {
    CustomDeleterCallCount = 0;
    std::uint64_t data = 0xcecececececececeull;
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        BitArray a{&data, 7, 26, [](char* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], '\xce');
            CORRADE_COMPARE(size, 5); /* amount of bytes spanned by 7 + 26 bits */
            ++CustomDeleterCallCount;
        }};
        CORRADE_COMPARE(reinterpret_cast<const void*>(a.data()), &data);
        CORRADE_COMPARE(a.offset(), 7);
        CORRADE_COMPARE(a.size(), 26);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void BitArrayTest::customDeleterNullData() {
    CustomDeleterCallCount = 0;
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        BitArray a{nullptr, 7, 26, [](char* data, std::size_t size) {
            CORRADE_VERIFY(!data);
            CORRADE_COMPARE(size, 5); /* amount of bytes spanned by 7 + 26 bits */
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(!a.data());
        CORRADE_COMPARE(a.offset(), 7);
        CORRADE_COMPARE(a.size(), 26);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter should be called even in case the data is null. Consistent
       with Array, where e.g. in Array<char, Utility::Path::MapDeleter> the
       data can be null for an empty file, but the fd should still get properly
       closed after. */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void BitArrayTest::customDeleterZeroSize() {
    CustomDeleterCallCount = 0;
    std::uint64_t data = 0xcecececececececeull;
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        BitArray a{&data, 0, 0, [](char* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], '\xce');
            CORRADE_COMPARE(size, 0);
            ++CustomDeleterCallCount;
        }};
        CORRADE_COMPARE(reinterpret_cast<const void*>(a.data()), &data);
        CORRADE_COMPARE(a.offset(), 0);
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* Variant of the above, while not as common, the deleter should
       unconditionally get called here as well */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void BitArrayTest::customDeleterMovedOutInstance() {
    CustomDeleterCallCount = 0;
    std::uint64_t data = 0xcecececececececeull;
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        BitArray a{&data, 7, 26, [](char*, std::size_t) {
            ++CustomDeleterCallCount;
        }};
        CORRADE_COMPARE(CustomDeleterCallCount, 0);

        BitArray b = std::move(a);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter got reset to nullptr in a, which means the function gets
       called only once */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void BitArrayTest::debug() {
    /* Delegates to BitArrayView, so it's the same output as in
       BitArrayViewTest::debug() */

    std::ostringstream out;
    /* 0b111'1100'1100'1111'0000'0101'0101 << 7 */
    std::uint64_t data = 0x7ccf055ull << 7;
    Debug{&out} << BitArray{&data, 7, 27, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out.str(), "{10101010, 00001111, 00110011, 111}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayTest)
