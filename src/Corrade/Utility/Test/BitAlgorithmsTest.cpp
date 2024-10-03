/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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

#include <algorithm>
#include <random>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/BitArray.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/BitAlgorithms.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Format.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct BitAlgorithmsTest: TestSuite::Tester {
    explicit BitAlgorithmsTest();

    void copyMasked();
    void copyMaskedZeroSize();
    void copyMaskedZeroBitsSet();

    void copyMaskedDifferentSize();
    void copyMaskedDifferentBitsSet();
    void copyMaskedDifferentTypeSize();
    void copyMaskedNotContiguous();

    void copyMaskedBenchmarkNaive();
    void copyMaskedBenchmark();
};

const struct {
    const char* name;
    bool flipSrc;
    bool flipDst;
    bool flipMask;
    bool flipExpected;
} CopyMaskedData[]{
    {"", false, false, false, false},
    {"negative src stride", true, false, true, true},
    {"negative dst stride", false, true, false, false},
};

const struct {
    float density;
} CopyMaskedBenchmarkData[]{
    {0.125f},
    {0.25f},
    {0.5f},
    {1.0f}
};

BitAlgorithmsTest::BitAlgorithmsTest() {
    addInstancedTests({&BitAlgorithmsTest::copyMasked},
        Containers::arraySize(CopyMaskedData));

    addTests({&BitAlgorithmsTest::copyMaskedZeroSize,
              &BitAlgorithmsTest::copyMaskedZeroBitsSet,

              &BitAlgorithmsTest::copyMaskedDifferentSize,
              &BitAlgorithmsTest::copyMaskedDifferentBitsSet,
              &BitAlgorithmsTest::copyMaskedDifferentTypeSize,
              &BitAlgorithmsTest::copyMaskedNotContiguous});

    addInstancedBenchmarks({
        &BitAlgorithmsTest::copyMaskedBenchmarkNaive,
        &BitAlgorithmsTest::copyMaskedBenchmark}, 100,
        Containers::arraySize(CopyMaskedBenchmarkData));
}

void BitAlgorithmsTest::copyMasked() {
    auto&& data = CopyMaskedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    const std::uint64_t srcData[]{
        2567, 0, 1, 2, 3, 28962786, 4, 5, 72652329, 926742716872, 6, 7
    };
    std::uint64_t dstData[4];
    const std::uint64_t expectedData[]{
        2567, 28962786, 72652329, 926742716872
    };

    Containers::StridedArrayView1D<const std::uint64_t> src = srcData;
    Containers::StridedArrayView1D<std::uint64_t> dst = dstData;
    Containers::StridedArrayView1D<const std::uint64_t> expected = expectedData;

    if(data.flipSrc) src = src.flipped<0>();
    if(data.flipDst) dst = dst.flipped<0>();
    if(data.flipExpected) expected = expected.flipped<0>();

    Containers::BitArray srcMask{ValueInit, src.size()};
    if(data.flipMask) {
        srcMask.set(2);
        srcMask.set(3);
        srcMask.set(6);
        srcMask.set(11);
    } else {
        srcMask.set(0);
        srcMask.set(5);
        srcMask.set(8);
        srcMask.set(9);
    }
    Utility::copyMasked(src, srcMask, dst);
    CORRADE_COMPARE_AS(dst, expected, TestSuite::Compare::Container);
}

void BitAlgorithmsTest::copyMaskedZeroSize() {
    /* Just verify it doesn't crash or something */

    Containers::ArrayView<std::uint16_t> dst;
    Utility::copyMasked(
        Containers::ArrayView<const std::uint16_t>{},
        Containers::BitArrayView{}, dst);
    CORRADE_COMPARE(dst.data(), nullptr);
}

void BitAlgorithmsTest::copyMaskedZeroBitsSet() {
    /* Just verify it doesn't crash or something */

    const std::uint16_t src[567]{};

    Containers::ArrayView<std::uint16_t> dst;
    Utility::copyMasked(src,
        Containers::BitArray{ValueInit, Containers::arraySize(src)}, dst);
    CORRADE_COMPARE(dst.data(), nullptr);
}

void BitAlgorithmsTest::copyMaskedDifferentSize() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const char src[15]{};
    char dst[3]{};

    std::ostringstream out;
    Error redirectError{&out};
    Utility::copyMasked(src, Containers::BitArray{ValueInit, 14}, dst);
    CORRADE_COMPARE(out.str(), "Utility::copyMasked(): expected source mask size to be 15 but got 14\n");
}

void BitAlgorithmsTest::copyMaskedDifferentBitsSet() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const char src[15]{};
    char dst[3]{};
    Containers::BitArray srcMask{ValueInit, 15};
    srcMask.set(7);
    srcMask.set(9);

    std::ostringstream out;
    Error redirectError{&out};
    Utility::copyMasked(src, srcMask, dst);
    CORRADE_COMPARE(out.str(), "Utility::copyMasked(): expected 2 destination items but got 3\n");
}

void BitAlgorithmsTest::copyMaskedDifferentTypeSize() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const std::uint16_t src[15]{};
    std::uint64_t dst[3]{};
    Containers::BitArray srcMask{ValueInit, 15};
    srcMask.set(7);
    srcMask.set(9);
    srcMask.set(11);

    std::ostringstream out;
    Error redirectError{&out};
    Utility::copyMasked(
        Containers::arrayCast<2, const char>(Containers::stridedArrayView(src)),
        srcMask, Containers::arrayCast<2, char>(Containers::stridedArrayView(dst)));
    CORRADE_COMPARE(out.str(), "Utility::copyMasked(): expected second destination dimension size to be 2 but got 8\n");
}

void BitAlgorithmsTest::copyMaskedNotContiguous() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::uint16_t a[3]{};
    std::uint8_t b[3]{};
    Containers::BitArray srcMask{DirectInit, 3, true};

    std::ostringstream out;
    Error redirectError{&out};
    Utility::copyMasked(
        Containers::arrayCast<2, char>(Containers::stridedArrayView(a)).every({1, 2}),
        srcMask, Containers::arrayCast<2, char>(Containers::stridedArrayView(b)));
    Utility::copyMasked(
        Containers::arrayCast<2, char>(Containers::stridedArrayView(b)),
        srcMask, Containers::arrayCast<2, char>(Containers::stridedArrayView(a)).every({1, 2}));
    CORRADE_COMPARE(out.str(),
        "Utility::copyMasked(): second source view dimension is not contiguous\n"
        "Utility::copyMasked(): second destination view dimension is not contiguous\n");
}

CORRADE_NEVER_INLINE void copyMaskedNaive(Containers::ArrayView<const std::size_t> src, Containers::BitArrayView srcMask, Containers::ArrayView<std::size_t> dst) {
    std::size_t offset = 0;
    for(std::size_t i = 0; i != src.size(); ++i) {
        if(!srcMask[i]) continue;
        dst[offset++] = src[i];
    }
}

/* All density values times this number need to be an integer */
constexpr std::size_t BenchmarkBitCount = 1024;

void BitAlgorithmsTest::copyMaskedBenchmarkNaive() {
    auto&& data = CopyMaskedBenchmarkData[testCaseInstanceId()];
    setTestCaseDescription(format("density {}", data.density));

    std::size_t positions[BenchmarkBitCount];
    for(std::size_t i = 0; i != BenchmarkBitCount; ++i)
        positions[i] = i;

    std::random_device rd;
    std::mt19937 g{rd()};
    std::shuffle(positions, positions + BenchmarkBitCount, g);

    Containers::BitArray srcMask{ValueInit, BenchmarkBitCount};
    std::size_t bitCount = BenchmarkBitCount*data.density;
    for(std::size_t i = 0; i != bitCount; ++i)
        srcMask.set(positions[i]);
    CORRADE_COMPARE(srcMask.count(), bitCount);

    Containers::Array<std::size_t> out{NoInit, bitCount};
    CORRADE_BENCHMARK(100) {
        copyMaskedNaive(positions, srcMask, out);
    }

    /* So the benchmark isn't completely discarded */
    CORRADE_VERIFY(out[0] || out[1]);
}

void BitAlgorithmsTest::copyMaskedBenchmark() {
    auto&& data = CopyMaskedBenchmarkData[testCaseInstanceId()];
    setTestCaseDescription(format("density {}", data.density));

    std::size_t positions[BenchmarkBitCount];
    for(std::size_t i = 0; i != BenchmarkBitCount; ++i)
        positions[i] = i;

    std::random_device rd;
    std::mt19937 g{rd()};
    std::shuffle(positions, positions + BenchmarkBitCount, g);

    Containers::BitArray srcMask{ValueInit, BenchmarkBitCount};
    std::size_t bitCount = BenchmarkBitCount*data.density;
    for(std::size_t i = 0; i != bitCount; ++i)
        srcMask.set(positions[i]);
    CORRADE_COMPARE(srcMask.count(), bitCount);

    Containers::Array<std::size_t> out{NoInit, bitCount};
    CORRADE_BENCHMARK(100)
        Utility::copyMasked(positions, srcMask, out);

    /* So the benchmark isn't completely discarded */
    CORRADE_VERIFY(out[0] || out[1]);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::BitAlgorithmsTest)
