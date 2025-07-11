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

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Memory.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct MemoryTest: TestSuite::Tester {
    explicit MemoryTest();

    template<std::size_t alignment> void allocateAlignedTrivial();
    void allocateAlignedTrivialNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    void allocateAlignedTrivialDefaultInit();
    #endif
    void allocateAlignedTrivialValueInit();
    void allocateAlignedNontrivialNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    void allocateAlignedNontrivialDefaultInit();
    #endif
    void allocateAlignedNontrivialValueInit();

    void allocateZeroSizeTrivial();
    void allocateZeroSizeNontrivial();

    void allocateExplicitAlignment();
    void allocateExplicitAlignmentNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    void allocateExplicitAlignmentDefaultInit();
    #endif
    void allocateExplicitAlignmentValueInit();

    void allocateNotMultipleOfAlignment();

    void resetCounters();
};

MemoryTest::MemoryTest() {
    addTests<MemoryTest>({
        &MemoryTest::allocateAlignedTrivial<1>,
        &MemoryTest::allocateAlignedTrivial<2>,
        &MemoryTest::allocateAlignedTrivial<4>,
        &MemoryTest::allocateAlignedTrivial<8>,
        &MemoryTest::allocateAlignedTrivial<16>,
        &MemoryTest::allocateAlignedTrivial<32>,
        &MemoryTest::allocateAlignedTrivial<64>,
        &MemoryTest::allocateAlignedTrivial<128>});

    /* https://stackoverflow.com/q/48070361. I found nothing that would give
       me the max allowed alignas value, std::max_align_t is useless. */
    #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ >= 7
    addTests<MemoryTest>({&MemoryTest::allocateAlignedTrivial<256>});
    #endif

    addTests({&MemoryTest::allocateAlignedTrivialNoInit,
              #ifdef CORRADE_BUILD_DEPRECATED
              &MemoryTest::allocateAlignedTrivialDefaultInit,
              #endif
              &MemoryTest::allocateAlignedTrivialValueInit});

    addTests({&MemoryTest::allocateAlignedNontrivialNoInit,
              #ifdef CORRADE_BUILD_DEPRECATED
              &MemoryTest::allocateAlignedNontrivialDefaultInit,
              #endif
              &MemoryTest::allocateAlignedNontrivialValueInit},
        &MemoryTest::resetCounters, &MemoryTest::resetCounters);

    addTests({&MemoryTest::allocateZeroSizeTrivial});

    addTests({&MemoryTest::allocateZeroSizeNontrivial},
        &MemoryTest::resetCounters, &MemoryTest::resetCounters);

    addRepeatedTests({
        &MemoryTest::allocateExplicitAlignment,
        &MemoryTest::allocateExplicitAlignmentNoInit,
        #ifdef CORRADE_BUILD_DEPRECATED
        &MemoryTest::allocateExplicitAlignmentDefaultInit,
        #endif
        &MemoryTest::allocateExplicitAlignmentValueInit}, 100);

    addTests({&MemoryTest::allocateNotMultipleOfAlignment});
}

template<std::size_t alignment> struct alignas(alignment) Aligned {
    char someData;
};

template<std::size_t alignment> void MemoryTest::allocateAlignedTrivial() {
    setTestCaseTemplateName(format("{}", alignment));

    Containers::Array<Containers::Array<Aligned<alignment>>> allocations;

    for(std::size_t i = 0; i != 100; ++i) {
        CORRADE_ITERATION(i);

        Containers::Array<Aligned<alignment>> data = allocateAligned<Aligned<alignment>>(i + 1);
        CORRADE_VERIFY(data.data());
        CORRADE_COMPARE(data.size(), i + 1);
        CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), alignment,
            TestSuite::Compare::Divisible);
        /* No way to verify that we *didn't* zero-initialize */

        /* Keep all allocations resident to avoid the allocator returning the
           same (aligned) pointer every time */
        arrayAppend(allocations, Utility::move(data));
    }
}

struct alignas(32) FourLongs {
    std::uint64_t data[4];
};

void MemoryTest::allocateAlignedTrivialNoInit() {
    Containers::Array<FourLongs> data = allocateAligned<FourLongs>(NoInit, 7);
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), 7);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
}

#ifdef CORRADE_BUILD_DEPRECATED
void MemoryTest::allocateAlignedTrivialDefaultInit() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    Containers::Array<FourLongs> data = allocateAligned<FourLongs>(DefaultInit, 7);
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), 7);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
    /* No way to verify that we *didn't* zero-initialize */
}
#endif

void MemoryTest::allocateAlignedTrivialValueInit() {
    Containers::Array<FourLongs> data = allocateAligned<FourLongs>(ValueInit, 7);
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), 7);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
    CORRADE_COMPARE_AS(Containers::arrayView(data[0].data),
        Containers::arrayView<std::uint64_t>({0, 0, 0, 0}),
        TestSuite::Compare::Container);
}

struct alignas(32) Immovable {
    static int constructed;
    static int destructed;

    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
    explicit Immovable() { ++constructed; }
    ~Immovable() { ++destructed; }
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;
};

int Immovable::constructed = 0;
int Immovable::destructed = 0;

void MemoryTest::resetCounters() {
    Immovable::constructed = Immovable::destructed = 0;
}

void MemoryTest::allocateAlignedNontrivialNoInit() {
    {
        Containers::Array<Immovable> data = allocateAligned<Immovable>(NoInit, 7);
        CORRADE_VERIFY(data.data());
        CORRADE_COMPARE(data.size(), 7);
        CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
            TestSuite::Compare::Divisible);
        CORRADE_COMPARE(Immovable::constructed, 0);
    }

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 7);
}

#ifdef CORRADE_BUILD_DEPRECATED
void MemoryTest::allocateAlignedNontrivialDefaultInit() {
    {
        CORRADE_IGNORE_DEPRECATED_PUSH
        Containers::Array<Immovable> data = allocateAligned<Immovable>(DefaultInit, 7);
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_VERIFY(data.data());
        CORRADE_COMPARE(data.size(), 7);
        CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
            TestSuite::Compare::Divisible);
        CORRADE_COMPARE(Immovable::constructed, 7);
    }

    CORRADE_COMPARE(Immovable::constructed, 7);
    CORRADE_COMPARE(Immovable::destructed, 7);
}
#endif

void MemoryTest::allocateAlignedNontrivialValueInit() {
    {
        Containers::Array<Immovable> data = allocateAligned<Immovable>(ValueInit, 7);
        CORRADE_VERIFY(data.data());
        CORRADE_COMPARE(data.size(), 7);
        CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
            TestSuite::Compare::Divisible);
        CORRADE_COMPARE(Immovable::constructed, 7);
    }

    CORRADE_COMPARE(Immovable::constructed, 7);
    CORRADE_COMPARE(Immovable::destructed, 7);
}

void MemoryTest::allocateZeroSizeTrivial() {
    Containers::Array<FourLongs> data = allocateAligned<FourLongs>(0);
    CORRADE_VERIFY(!data.data());
    CORRADE_COMPARE(data.size(), 0);
}

void MemoryTest::allocateZeroSizeNontrivial() {
    {
        Containers::Array<Immovable> data = allocateAligned<Immovable>(0);
        CORRADE_VERIFY(!data.data());
        CORRADE_COMPARE(data.size(), 0);
    }

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);
}

void MemoryTest::allocateExplicitAlignment() {
    Containers::Array<char> data = allocateAligned<char, 32>((testCaseRepeatId() + 1)*32);
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), (testCaseRepeatId() + 1)*32);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
}

void MemoryTest::allocateExplicitAlignmentNoInit() {
    Containers::Array<char> data = allocateAligned<char, 32>(NoInit, (testCaseRepeatId() + 1)*32);
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), (testCaseRepeatId() + 1)*32);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
}

#ifdef CORRADE_BUILD_DEPRECATED
void MemoryTest::allocateExplicitAlignmentDefaultInit() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    Containers::Array<char> data = allocateAligned<char, 32>(DefaultInit, (testCaseRepeatId() + 1)*32);
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), (testCaseRepeatId() + 1)*32);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
}
#endif

void MemoryTest::allocateExplicitAlignmentValueInit() {
    Containers::Array<char> data = allocateAligned<char, 32>(ValueInit, (testCaseRepeatId() + 1)*32);
    CORRADE_VERIFY(data.data());
    CORRADE_COMPARE(data.size(), (testCaseRepeatId() + 1)*32);
    CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(data.data()), 32,
        TestSuite::Compare::Divisible);
}

void MemoryTest::allocateNotMultipleOfAlignment() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    allocateAligned<short, 32>(17);
    CORRADE_COMPARE(out, "Utility::allocateAligned(): total byte size 34 not a multiple of a 32-byte alignment\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MemoryTest)
