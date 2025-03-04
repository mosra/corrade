/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include "Corrade/Containers/ArrayView.h" /* arraySize() */
#include "Corrade/Containers/BitArray.h"
#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayTest: TestSuite::Tester {
    explicit BitArrayTest();

    void constructDefault();
    void constructValueInit();
    void constructValueInitZeroSize();
    #ifdef CORRADE_TARGET_32BIT
    void constructValueInitSizeTooLarge();
    #endif
    void constructDirectInit();
    void constructDirectInitZeroSize();
    #ifdef CORRADE_TARGET_32BIT
    void constructDirectInitSizeTooLarge();
    #endif
    void constructNoInit();
    void constructNoInitZeroSize();
    #ifdef CORRADE_TARGET_32BIT
    void constructNoInitSizeTooLarge();
    #endif
    void constructTakeOwnership();
    void constructTakeOwnershipOffsetTooLarge();
    #ifdef CORRADE_TARGET_32BIT
    void constructTakeOwnershipSizeTooLarge();
    #endif
    void constructMove();

    void constructZeroNullPointerAmbiguity();

    void convertView();
    void convertMutableView();

    void access();
    void accessMutableSet();
    void accessMutableReset();
    void accessMutableSetAll();
    void accessMutableResetAll();
    void accessInvalid();

    template<class T> void slice();

    void count();

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

/* Same as in BitArrayViewTest */
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

BitArrayTest::BitArrayTest() {
    addTests({&BitArrayTest::constructDefault,
              &BitArrayTest::constructValueInit,
              &BitArrayTest::constructValueInitZeroSize,
              #ifdef CORRADE_TARGET_32BIT
              &BitArrayTest::constructValueInitSizeTooLarge
              #endif
              });

    addInstancedTests({&BitArrayTest::constructDirectInit},
        Containers::arraySize(ConstructDirectInitData));

    addTests({&BitArrayTest::constructDirectInitZeroSize,
              #ifdef CORRADE_TARGET_32BIT
              &BitArrayTest::constructDirectInitSizeTooLarge,
              #endif
              &BitArrayTest::constructNoInit,
              &BitArrayTest::constructNoInitZeroSize,
              #ifdef CORRADE_TARGET_32BIT
              &BitArrayTest::constructNoInitSizeTooLarge,
              #endif
              &BitArrayTest::constructTakeOwnership,
              &BitArrayTest::constructTakeOwnershipOffsetTooLarge,
              #ifdef CORRADE_TARGET_32BIT
              &BitArrayTest::constructTakeOwnershipSizeTooLarge,
              #endif
              &BitArrayTest::constructMove,

              &BitArrayTest::constructZeroNullPointerAmbiguity,

              &BitArrayTest::convertView,
              &BitArrayTest::convertMutableView,

              &BitArrayTest::access});

    addInstancedTests({&BitArrayTest::accessMutableSet,
                       &BitArrayTest::accessMutableReset},
        Containers::arraySize(AccessMutableData));

    addTests({&BitArrayTest::accessMutableSetAll,
              &BitArrayTest::accessMutableResetAll,
              &BitArrayTest::accessInvalid,

              &BitArrayTest::slice<const BitArray>,
              &BitArrayTest::slice<BitArray>,

              &BitArrayTest::count,

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
    BitArray a1;
    BitArray a2 = nullptr;
    CORRADE_VERIFY(a1.isEmpty());
    CORRADE_VERIFY(a2.isEmpty());
    CORRADE_COMPARE(a1.offset(), 0);
    CORRADE_COMPARE(a2.offset(), 0);
    CORRADE_COMPARE(a1.size(), 0);
    CORRADE_COMPARE(a2.size(), 0);
    CORRADE_VERIFY(!a1.data());
    CORRADE_VERIFY(!a2.data());
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

#ifdef CORRADE_TARGET_32BIT
void BitArrayTest::constructValueInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    BitArray{Corrade::ValueInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3)};
    CORRADE_COMPARE(out, "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
}
#endif

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

#ifdef CORRADE_TARGET_32BIT
void BitArrayTest::constructDirectInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    BitArray{Corrade::DirectInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3), true};
    CORRADE_COMPARE(out, "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
}
#endif

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

#ifdef CORRADE_TARGET_32BIT
void BitArrayTest::constructNoInitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    BitArray{Corrade::NoInit, std::size_t{1} << (sizeof(std::size_t)*8 - 3)};
    CORRADE_COMPARE(out, "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
}
#endif

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

    Containers::String out;
    Error redirectError{&out};
    BitArray{nullptr, 8, 0, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out, "Containers::BitArray: offset expected to be smaller than 8 bits, got 8\n");
}

#ifdef CORRADE_TARGET_32BIT
void BitArrayTest::constructTakeOwnershipSizeTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    BitArray{nullptr, 0, std::size_t{1} << (sizeof(std::size_t)*8 - 3), [](char*, std::size_t) {}};
    CORRADE_COMPARE(out, "Containers::BitArray: size expected to be smaller than 2^29 bits, got 536870912\n");
}
#endif

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

/* Without a corresponding SFINAE check in the std::nullptr_t constructor, this
   is ambiguous, but *only* if the size_t overload has a second 64-bit
   argument. If both would be the same, it wouldn't be ambigous, if the size_t
   overload second argument was 32-bit and the other 16-bit it wouldn't be
   either. */
int integerArrayOverload(std::size_t, long long) {
    return 76;
}
int integerArrayOverload(const BitArray&, int) {
    return 39;
}

void BitArrayTest::constructZeroNullPointerAmbiguity() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverload(25, 2), 76);
    CORRADE_COMPARE(integerArrayOverload(nullptr, 2), 39);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverload(0, 3), 76);
}

void BitArrayTest::convertView() {
    std::uint64_t data{};
    const BitArray a{&data, 7, 31, [](char*, std::size_t) {}};

    const BitArrayView cb = a;
    CORRADE_COMPARE(cb.data(), reinterpret_cast<const void*>(&data));
    CORRADE_COMPARE(cb.offset(), 7);
    CORRADE_COMPARE(cb.size(), 31);

    /* It shouldn't be possible to create a mutable view from a const BitArray.
       Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<BitArrayView, const BitArray>::value);
    CORRADE_VERIFY(!std::is_constructible<MutableBitArrayView, const BitArray>::value);
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

    /* 0b0101'0101'0011'0011'0000'1111 << 5 */
    char data[]{'\xe0', '\x61', '\xa6', '\x0a'};
    const BitArray a{data, 5, 24, [](char*, std::size_t) {}};

    for(std::size_t i: {0, 1, 2, 3, 8, 9, 12, 13, 16, 18, 20, 22}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
    }

    for(std::size_t i: {4, 5, 6, 7, 10, 11, 14, 15, 17, 19, 21, 23}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!a[i]);
    }
}

void BitArrayTest::accessMutableSet() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Same as in BitArrayView -- the implementation deliberately doesn't
       delegate to it so it has to be tested fully */

    std::uint32_t valueA[]{data.valueSet};
    std::uint32_t valueB[]{data.valueSet};
    BitArray a{reinterpret_cast<char*>(valueA), data.offset, 24, [](char*, std::size_t) {}};
    BitArray b{reinterpret_cast<char*>(valueB), data.offset, 24, [](char*, std::size_t) {}};

    a.set(data.bit);
    b.set(data.bit, true);
    CORRADE_COMPARE(valueA[0], data.expectedSet);
    CORRADE_COMPARE(valueB[0], data.expectedSet);
}

void BitArrayTest::accessMutableReset() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Same as in BitArrayView -- the implementation deliberately doesn't
       delegate to it so it has to be tested fully */

    std::uint32_t valueA[]{data.valueReset};
    std::uint32_t valueB[]{data.valueReset};
    BitArray a{reinterpret_cast<char*>(valueA), data.offset, 24, [](char*, std::size_t) {}};
    BitArray b{reinterpret_cast<char*>(valueB), data.offset, 24, [](char*, std::size_t) {}};

    a.reset(data.bit);
    b.set(data.bit, false);
    CORRADE_COMPARE(valueA[0], data.expectedReset);
    CORRADE_COMPARE(valueB[0], data.expectedReset);
}

void BitArrayTest::accessMutableSetAll() {
    /* A single case from BitArrayViewTest::accessMutableSetAll(), just to
       verify that all data including bit offset are passed through to the
       underlying API */
    {
        std::uint64_t a = 0x0000000000000000ull;
        BitArray{reinterpret_cast<char*>(&a) + 1, 1, 38, [](char*, std::size_t) {}}.setAll();
        CORRADE_COMPARE(a, 0x00007ffffffffe00ull);

    /* Same as above, with a boolean argument */
    } {
        std::uint64_t a = 0x0000000000000000ull;
        BitArray{reinterpret_cast<char*>(&a) + 1, 1, 38, [](char*, std::size_t) {}}.setAll(true);
        CORRADE_COMPARE(a, 0x00007ffffffffe00ull);
    }
}

void BitArrayTest::accessMutableResetAll() {
    /* A single case from BitArrayViewTest::accessMutableSetAll(), just to
       verify that all data including bit offset are passed through to the
       underlying API */
    {
        std::uint64_t a = 0xffffffffffffffffull;
        BitArray{reinterpret_cast<char*>(&a) + 1, 1, 38, [](char*, std::size_t) {}}.resetAll();
        CORRADE_COMPARE(a, 0xffff8000000001ffull);

    /* Same as above, with a boolean argument */
    } {
        std::uint64_t a = 0xffffffffffffffffull;
        BitArray{reinterpret_cast<char*>(&a) + 1, 1, 38, [](char*, std::size_t) {}}.setAll(false);
        CORRADE_COMPARE(a, 0xffff8000000001ffull);
    }
}

void BitArrayTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint64_t data;
    BitArray array{&data, 4, 53, [](char*, std::size_t) {}};

    Containers::String out;
    Error redirectError{&out};
    array[53];
    array.set(53);
    array.reset(53);
    array.set(53, true);
    CORRADE_COMPARE(out,
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

void BitArrayTest::count() {
    /* A single case from BitArrayViewTest::countBitPattern(), just to verify
       that all data including bit offset are passed through to the underlying
       API */
    std::uint64_t data = 0xa55cc33f00f00ffull << 7;
    BitArray a{&data, 7, 56, [](char*, std::size_t) {}};
    CORRADE_COMPARE(a.count(), 28);
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

        BitArray b = Utility::move(a);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter got reset to nullptr in a, which means the function gets
       called only once */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void BitArrayTest::debug() {
    /* Delegates to BitArrayView, so it's the same output as in
       BitArrayViewTest::debug() */
    char data[]{'\xe0', '\x61', '\xa6', '\x0a'};

    Containers::String out;
    Debug{&out} << BitArray{data, 5, 19, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out, "{11110000, 11001100, 101}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayTest)
