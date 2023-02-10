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

#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Containers {

namespace Test { namespace {

struct StridedBitArrayViewTest: TestSuite::Tester {
    explicit StridedBitArrayViewTest();

    template<class T> void constructDefault();
    template<class T> void construct();
    void constructConstexpr();
    void constructArray();
    void constructNullptrSize();
    void constructZeroStride();
    void constructNegativeStride();

    template<class T> void constructSizeStride();
    void constructSizeStrideConstexpr();
    void constructSizeStrideArray();
    template<class T> void constructSizeOnly();
    void constructSizeOnlyConstexpr();
    void constructSizeOnlyArray();

    void constructOffsetTooLarge();
    void constructSizeTooLarge();
    void constructViewTooSmall();
    void constructBeginOffsetTooSmall();

    void constructFromMutable();
    template<class T> void constructFromView();
    void constructFromViewConstexpr();
    void constructFromMutableView();
    void constructCopy();

    template<class T> void construct3DDefault();
    template<class T> void construct3D();
    void construct3DConstexpr();
    void construct3DNullptrSize();
    void construct3DZeroStride();
    void construct3DNegativeStride();

    template<class T> void construct3DSizeStride();
    void construct3DSizeStrideConstexpr();
    template<class T> void construct3DSizeOnly();
    void construct3DSizeOnlyConstexpr();
    void construct3DOneSizeZero();

    /* No construct3DOffsetTooLarge(), it's no different from the 1D case */
    void construct3DSizeTooLarge();
    void construct3DViewTooSmall();
    /* No construct3DBeginTooSmall(), it's no different from the 1D case */

    void construct3DFromView();
    void construct3DFromLessDimensions();

    void asContiguous();
    void asContiguousNonContiguous();

    void access();
    void accessMutableSet();
    void accessMutableReset();
    void accessZeroStride();
    void accessZeroStrideMutableSet();
    void accessZeroStrideMutableReset();
    void accessNegativeStride();
    void accessNegativeStrideMutableSet();
    void accessNegativeStrideMutableReset();
    void accessInvalid();

    void access3D();
    void access3DMutable();
    void access3DZeroStride();
    void access3DZeroStrideMutable();
    void access3DNegativeStride();
    void access3DNegativeStrideMutable();
    void access3DInvalid();

    void slice();
    void sliceInvalid();
    void slice3D();
    void slice3DInvalid();
    void slice3DFirstDimension();
    void slice3DFirstDimensionInvalid();

    void every();
    void everyInvalid();
    void every3D();
    void every3DInvalid();
    void every3DFirstDimension();

    void transposed();
    void transposedToSelf();
    void flipped();
    void flippedZeroSize();
    void flipped3D();
    void flipped3DZeroSize();
    void broadcasted();
    void broadcasted3D();
    void broadcastedInvalid();

    void debug();
    void debug3D();
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    std::size_t offset;
    std::ptrdiff_t stride;
    std::size_t bit;
    std::uint32_t valueSet;
    std::uint32_t expectedSet;
    std::uint32_t valueReset;
    std::uint32_t expectedReset;
} AccessMutableData[]{
    /* Same as AccessMutableData in BitArrayViewTest, with strided variants
       added */
    {"no-op", 0, 1, 6,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset", 5, 1, 1,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, overflow", 0, 1, 13,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset, overflow", 6, 1, 7,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, strided", 5, 4, 2,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"single bit", 0, 1, 5,
        0x00000000u, 0x00000020u,
        0xffffffffu, 0xffffffdfu},
    {"single bit, offset", 3, 1, 2,
        0x00000000u, 0x00000020u,
        0xffffffffu, 0xffffffdfu},
    {"single bit, overflow", 0, 1, 21,
        0x00000000u, 0x00200000u,
        0xffffffffu, 0xffdfffffu},
    {"single bit, offset, overflow", 6, 1, 15,
        0x00000000u, 0x00200000u,
        0xffffffffu, 0xffdfffffu},
    {"single bit, strided", 3, 6, 3,
        0x00000000u, 0x00200000u,
        0xffffffffu, 0xffdfffffu},
    {"bit pattern", 0, 1, 11,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
    {"bit pattern, offset", 4, 1, 7,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
    {"bit pattern, strided", 2, 3, 3,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    std::size_t offset;
    std::size_t bit;
    std::uint32_t valueSet;
    std::uint32_t expectedSet;
    std::uint32_t valueReset;
    std::uint32_t expectedReset;
} AccessMutableZeroStrideData[]{
    {"no-op", 0, 7,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset", 3, 4,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, overflow", 0, 257,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset, overflow", 6, 257,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"single bit", 0, 5,
        0x00000000u, 0x00000001u,
        0xffffffffu, 0xfffffffeu},
    {"single bit, offset", 3, 4,
        0x00000000u, 0x00000008u,
        0xffffffffu, 0xfffffff7u},
    {"single bit, overflow", 0, 50007,
        0x00000000u, 0x00000001u,
        0xffffffffu, 0xfffffffeu},
    {"single bit, offset, overflow", 7, 1479896,
        0x00000000u, 0x00000080u,
        0xffffffffu, 0xffffff7fu},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    std::size_t offset;
    std::ptrdiff_t stride;
    std::size_t bit;
    std::uint32_t valueSet;
    std::uint32_t expectedSet;
    std::uint32_t valueReset;
    std::uint32_t expectedReset;
} AccessMutableNegativeStrideData[]{
    {"no-op", 0, -1, 6,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset", 5, -1, 1,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, overflow", 0, -1, 13,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"no-op, offset, overflow", 6, -1, 7,
        0xffffffffu, 0xffffffffu,
        0x00000000u, 0x00000000u},
    {"single bit", 0, -1, 2,
        0x00000000u, 0x00400000u,
        0xffffffffu, 0xffbfffffu},
    {"single bit, offset", 7, -1, 0,
        0x00000000u, 0x80000000u,
        0xffffffffu, 0x7fffffffu},
    {"single bit, overflow", 0, -1, 15,
        0x00000000u, 0x00000200u,
        0xffffffffu, 0xfffffdffu},
    {"single bit, offset, overflow", 6, -1, 21,
        0x00000000u, 0x00000200u,
        0xffffffffu, 0xfffffdffu},
    {"single bit, strided", 3, -3, 6,
        0x00000000u, 0x00000200u,
        0xffffffffu, 0xfffffdffu},
    {"bit pattern", 0, -1, 13,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
    {"bit pattern, offset", 4, -1, 17,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
    {"bit pattern, strided", 2, -5, 3,
        0x01234567u, 0x01234d67u,
        0x89abcdefu, 0x89abc5efu},
};

StridedBitArrayViewTest::StridedBitArrayViewTest() {
    addTests({&StridedBitArrayViewTest::constructDefault<const char>,
              &StridedBitArrayViewTest::constructDefault<char>,
              &StridedBitArrayViewTest::construct<const char>,
              &StridedBitArrayViewTest::construct<char>,
              &StridedBitArrayViewTest::constructConstexpr,
              &StridedBitArrayViewTest::constructArray,
              &StridedBitArrayViewTest::constructNullptrSize,
              &StridedBitArrayViewTest::constructZeroStride,
              &StridedBitArrayViewTest::constructNegativeStride,

              &StridedBitArrayViewTest::constructSizeStride<const char>,
              &StridedBitArrayViewTest::constructSizeStride<char>,
              &StridedBitArrayViewTest::constructSizeStrideConstexpr,
              &StridedBitArrayViewTest::constructSizeStrideArray,
              &StridedBitArrayViewTest::constructSizeOnly<const char>,
              &StridedBitArrayViewTest::constructSizeOnly<char>,
              &StridedBitArrayViewTest::constructSizeOnlyConstexpr,
              &StridedBitArrayViewTest::constructSizeOnlyArray,

              &StridedBitArrayViewTest::constructOffsetTooLarge,
              &StridedBitArrayViewTest::constructSizeTooLarge,
              &StridedBitArrayViewTest::constructViewTooSmall,
              &StridedBitArrayViewTest::constructBeginOffsetTooSmall,

              &StridedBitArrayViewTest::constructFromMutable,
              &StridedBitArrayViewTest::constructFromView<const char>,
              &StridedBitArrayViewTest::constructFromView<char>,
              &StridedBitArrayViewTest::constructFromViewConstexpr,
              &StridedBitArrayViewTest::constructFromMutableView,
              &StridedBitArrayViewTest::constructCopy,

              &StridedBitArrayViewTest::construct3DDefault<const char>,
              &StridedBitArrayViewTest::construct3DDefault<char>,
              &StridedBitArrayViewTest::construct3D<const char>,
              &StridedBitArrayViewTest::construct3D<char>,
              &StridedBitArrayViewTest::construct3DConstexpr,
              &StridedBitArrayViewTest::construct3DNullptrSize,
              &StridedBitArrayViewTest::construct3DZeroStride,
              &StridedBitArrayViewTest::construct3DNegativeStride,

              &StridedBitArrayViewTest::construct3DSizeStride<const char>,
              &StridedBitArrayViewTest::construct3DSizeStride<char>,
              &StridedBitArrayViewTest::construct3DSizeStrideConstexpr,
              &StridedBitArrayViewTest::construct3DSizeOnly<const char>,
              &StridedBitArrayViewTest::construct3DSizeOnly<char>,
              &StridedBitArrayViewTest::construct3DSizeOnlyConstexpr,
              &StridedBitArrayViewTest::construct3DOneSizeZero,

              &StridedBitArrayViewTest::construct3DSizeTooLarge,
              &StridedBitArrayViewTest::construct3DViewTooSmall,

              &StridedBitArrayViewTest::construct3DFromView,
              &StridedBitArrayViewTest::construct3DFromLessDimensions,

              &StridedBitArrayViewTest::asContiguous,
              &StridedBitArrayViewTest::asContiguousNonContiguous,

              &StridedBitArrayViewTest::access});

    addInstancedTests({&StridedBitArrayViewTest::accessMutableSet,
                       &StridedBitArrayViewTest::accessMutableReset},
        Containers::arraySize(AccessMutableData));

    addTests({&StridedBitArrayViewTest::accessZeroStride});

    addInstancedTests({&StridedBitArrayViewTest::accessZeroStrideMutableSet,
                       &StridedBitArrayViewTest::accessZeroStrideMutableReset},
        Containers::arraySize(AccessMutableZeroStrideData));

    addTests({&StridedBitArrayViewTest::accessNegativeStride});

    addInstancedTests({&StridedBitArrayViewTest::accessNegativeStrideMutableSet,
                       &StridedBitArrayViewTest::accessNegativeStrideMutableReset},
        Containers::arraySize(AccessMutableNegativeStrideData));

    addTests({&StridedBitArrayViewTest::accessInvalid,

              &StridedBitArrayViewTest::access3D,
              &StridedBitArrayViewTest::access3DMutable,
              &StridedBitArrayViewTest::access3DZeroStride,
              &StridedBitArrayViewTest::access3DZeroStrideMutable,
              &StridedBitArrayViewTest::access3DNegativeStride,
              &StridedBitArrayViewTest::access3DNegativeStrideMutable,
              &StridedBitArrayViewTest::access3DInvalid,

              &StridedBitArrayViewTest::slice,
              &StridedBitArrayViewTest::sliceInvalid,
              &StridedBitArrayViewTest::slice3D,
              &StridedBitArrayViewTest::slice3DInvalid,
              &StridedBitArrayViewTest::slice3DFirstDimension,
              &StridedBitArrayViewTest::slice3DFirstDimensionInvalid,

              &StridedBitArrayViewTest::every,
              &StridedBitArrayViewTest::everyInvalid,
              &StridedBitArrayViewTest::every3D,
              &StridedBitArrayViewTest::every3DInvalid,
              &StridedBitArrayViewTest::every3DFirstDimension,

              &StridedBitArrayViewTest::transposed,
              &StridedBitArrayViewTest::transposedToSelf,
              &StridedBitArrayViewTest::flipped,
              &StridedBitArrayViewTest::flippedZeroSize,
              &StridedBitArrayViewTest::flipped3D,
              &StridedBitArrayViewTest::flipped3DZeroSize,
              &StridedBitArrayViewTest::broadcasted,
              &StridedBitArrayViewTest::broadcasted3D,
              &StridedBitArrayViewTest::broadcastedInvalid,

              &StridedBitArrayViewTest::debug,
              &StridedBitArrayViewTest::debug3D});
}

template<class> struct NameFor;
template<> struct NameFor<const char> {
    static const char* name() { return "StridedBitArrayView"; }
};
template<> struct NameFor<char> {
    static const char* name() { return "MutableStridedBitArrayView"; }
};

template<class T> void StridedBitArrayViewTest::constructDefault() {
    setTestCaseTemplateName(NameFor<T>::name());

    const BasicStridedBitArrayView<1, T> a;
    const BasicStridedBitArrayView<1, T> b = nullptr;
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(b.offset(), 0);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);
    CORRADE_COMPARE(b.stride(), 0);

    constexpr BasicStridedBitArrayView<1, T> ca;
    constexpr BasicStridedBitArrayView<1, T> cb = nullptr;
    constexpr const void* dataA = ca.data();
    constexpr const void* dataB = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    bool emptyA = ca.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    bool emptyB = cb.isEmpty();
    constexpr std::size_t offsetA = ca.offset();
    constexpr std::size_t offsetB = cb.offset();
    /* These two started failing with "C2131: expression did not evaluate to a
       constant" after a removal of the (ErasedType*, std::size_t, std::size_t)
       constructor in f7644abea5675a1559edc7cd7b5e2ccecb28e63b. I don't see how
       is that even related to anything. MSVC, *what the fuck*. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr
    #endif
    std::size_t sizeA = ca.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr
    #endif
    std::size_t sizeB = cb.size();
    constexpr std::ptrdiff_t strideA = ca.stride();
    constexpr std::ptrdiff_t strideB = cb.stride();
    CORRADE_COMPARE(dataA, nullptr);
    CORRADE_COMPARE(dataB, nullptr);
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(offsetA, 0);
    CORRADE_COMPARE(offsetB, 0);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);
    CORRADE_COMPARE(strideA, 0);
    CORRADE_COMPARE(strideB, 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BasicBitArrayView<T>>::value);
}

template<class T> void StridedBitArrayViewTest::construct() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint64_t data[2]{};
    const BasicBitArrayView<T> a{data, 3, 26};
    const BasicStridedBitArrayView<1, T> b = {a, data + 1, 5, 7, 3};
    CORRADE_COMPARE(b.data(), data + 1);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<1, T>, BasicBitArrayView<T>, void*, std::size_t, std::size_t, std::ptrdiff_t>::value);
}

constexpr std::uint64_t Data64[2]{};

void StridedBitArrayViewTest::constructConstexpr() {
    constexpr BitArrayView ca{Data64, 3, 26};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, Data64 + 1, 5, 7, 3};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    bool empty = cb.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::ptrdiff_t stride = cb.stride();
    CORRADE_COMPARE(data, Data64 + 1);
    CORRADE_COMPARE(offset, 5);
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(size, 7);
    CORRADE_COMPARE(stride, 3);
}

void StridedBitArrayViewTest::constructArray() {
    /* Compared to construct[Constexpr](), size and stride is wrapped in {}.
       Just to verify that this doesn't cause a compilation error, it isn't any
       special overload. */

    std::uint64_t data[2]{};
    BitArrayView a{data, 3, 26};
    StridedBitArrayView1D b = {a, data + 1, 5, {7}, {3}};
    CORRADE_COMPARE(b.data(), data+ 1);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 3);

    constexpr BitArrayView ca{Data64, 3, 26};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, Data64 + 1, 5, {7}, {3}};
    CORRADE_COMPARE(cb.data(), Data64 + 1);
    CORRADE_COMPARE(cb.offset(), 5);
    CORRADE_COMPARE(cb.size(), 7);
    CORRADE_COMPARE(cb.stride(), 3);
}

void StridedBitArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later */

    StridedBitArrayView1D a{{nullptr, 5, 24}, nullptr, 5, 7, 3};
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 7);
    CORRADE_COMPARE(a.stride(), 3);

    constexpr StridedBitArrayView1D ca{{nullptr, 5, 24}, nullptr, 5, 7, 3};
    CORRADE_COMPARE(ca.data(), nullptr);
    CORRADE_COMPARE(ca.offset(), 5);
    CORRADE_VERIFY(!ca.isEmpty());
    CORRADE_COMPARE(ca.size(), 7);
    CORRADE_COMPARE(ca.stride(), 3);
}

void StridedBitArrayViewTest::constructZeroStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in accessZeroStride() */

    char data[3]{};
    BitArrayView a{data, 3, 8};
    StridedBitArrayView1D b = {a, data + 1, 7, 100, 0};
    CORRADE_COMPARE(b.data(), data + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), 100);
    CORRADE_COMPARE(b.stride(), 0);
}

void StridedBitArrayViewTest::constructNegativeStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in accessNegativeStride() */

    char data[5]{};
    BitArrayView a{data, 3, 26};
    StridedBitArrayView1D b = {a, data + 4, 2 /* complement to 7 */, 7, -3};
    CORRADE_COMPARE(b.data(), data + 4);
    CORRADE_COMPARE(b.offset(), 2);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), -3);
}

template<class T> void StridedBitArrayViewTest::constructSizeStride() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[4]{};
    const BasicBitArrayView<T> a{data, 5, 24};
    const BasicStridedBitArrayView<1, T> b = {a, 7, 3};
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<1, T>, BasicBitArrayView<T>, std::size_t, std::ptrdiff_t>::value);
}

constexpr std::uint32_t Data32[1]{};

void StridedBitArrayViewTest::constructSizeStrideConstexpr() {
    constexpr BitArrayView ca{Data32, 5, 24};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, 7, 3};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::ptrdiff_t stride = cb.stride();
    CORRADE_COMPARE(data, Data32);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 7);
    CORRADE_COMPARE(stride, 3);
}

void StridedBitArrayViewTest::constructSizeStrideArray() {
    /* Compared to constructSizeStride[Constexpr](), size and stride is wrapped
       in {}. Just to verify that this doesn't cause a compilation error, it
       isn't any special overload. */

    char data[4]{};
    BitArrayView a{data, 5, 24};
    StridedBitArrayView1D b = {a, {7}, {3}};
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 3);

    constexpr BitArrayView ca{Data32, 5, 24};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, {7}, {3}};
    CORRADE_COMPARE(cb.data(), Data32);
    CORRADE_COMPARE(cb.offset(), 5);
    CORRADE_VERIFY(!cb.isEmpty());
    CORRADE_COMPARE(cb.size(), 7);
    CORRADE_COMPARE(cb.stride(), 3);
}

template<class T> void StridedBitArrayViewTest::constructSizeOnly() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[4]{};
    const BasicBitArrayView<T> a{data, 5, 24};
    const BasicStridedBitArrayView<1, T> b = {a, 7};
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 1);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<1, T>, BasicBitArrayView<T>, std::size_t, std::ptrdiff_t>::value);
}

void StridedBitArrayViewTest::constructSizeOnlyConstexpr() {
    constexpr BitArrayView ca{Data32, 5, 24};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, 7};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::ptrdiff_t stride = cb.stride();
    CORRADE_COMPARE(data, Data32);
    CORRADE_COMPARE(offset, 5);
    CORRADE_COMPARE(size, 7);
    CORRADE_COMPARE(stride, 1);
}

void StridedBitArrayViewTest::constructSizeOnlyArray() {
    /* Compared to constructSizeStride[Constexpr](), size and stride is wrapped
       in {}. Just to verify that this doesn't cause a compilation error, it
       isn't any special overload. */

    char data[4]{};
    BitArrayView a{data, 5, 24};
    StridedBitArrayView1D b = {a, {7}};
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 1);

    constexpr BitArrayView ca{Data32, 5, 24};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView1D cb = {ca, {7}};
    CORRADE_COMPARE(cb.data(), Data32);
    CORRADE_COMPARE(cb.offset(), 5);
    CORRADE_VERIFY(!cb.isEmpty());
    CORRADE_COMPARE(cb.size(), 7);
    CORRADE_COMPARE(cb.stride(), 1);
}

void StridedBitArrayViewTest::constructOffsetTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    StridedBitArrayView1D{BitArrayView{nullptr, 0, 0}, nullptr, 8, 0, 1};
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: offset expected to be smaller than 8 bits, got 8\n");
}

void StridedBitArrayViewTest::constructSizeTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    /* Creating a view with zero stride, otherwise this would get caught by
       other asserts already */
    StridedBitArrayView1D{BitArrayView{nullptr, 0, 1}, std::size_t{1} << (sizeof(std::size_t)*8 - 3), 0};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: size expected to be smaller than 2^61 bits, got {2305843009213693952}\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: size expected to be smaller than 2^29 bits, got {536870912}\n");
    #endif
}

void StridedBitArrayViewTest::constructViewTooSmall() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    StridedBitArrayView1D{BitArrayView{nullptr, 0, 15}, nullptr, 0, 8, 2};
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: data size 15 is not enough for {8} bits of stride {2}\n");
}

void StridedBitArrayViewTest::constructBeginOffsetTooSmall() {
    CORRADE_SKIP_IF_NO_ASSERT();

    /* This is fine as the pointer is larger */
    char a[3]{};
    StridedBitArrayView1D{BitArrayView{a, 7, 15}, a + 1, 6, 4, 2};

    std::ostringstream out;
    Error redirectError{&out};
    StridedBitArrayView1D{BitArrayView{a, 7, 15}, a, 6, 4, 2};
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: offset 6 is less than data offset 7 in the same byte\n");
}

void StridedBitArrayViewTest::constructFromMutable() {
    std::uint64_t data[1]{};
    const MutableBitArrayView a{data, 5, 47};
    const MutableStridedBitArrayView1D b{a, 11, 4};
    const StridedBitArrayView1D c = b;

    CORRADE_VERIFY(!c.isEmpty());
    CORRADE_COMPARE(c.offset(), 5);
    CORRADE_COMPARE(c.size(), 11);
    CORRADE_COMPARE(c.stride(), 4);
    CORRADE_COMPARE(c.data(), &data);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedBitArrayView1D, MutableStridedBitArrayView1D>::value);

    /* It shouldn't be possible the other way around. Not using is_convertible
       to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StridedBitArrayView1D, MutableStridedBitArrayView1D>::value);
    CORRADE_VERIFY(!std::is_constructible<MutableStridedBitArrayView1D, StridedBitArrayView1D>::value);
}

template<class T> void StridedBitArrayViewTest::constructFromView() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[4]{};
    const BasicBitArrayView<T> view{data, 5, 24};

    BasicStridedBitArrayView<1, T> b = view;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 24);
    CORRADE_COMPARE(b.stride(), 1);

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<1, T>, BasicBitArrayView<T>>::value);
}

void StridedBitArrayViewTest::constructFromViewConstexpr() {
    constexpr BitArrayView view{Data32, 5, 24};

    constexpr StridedBitArrayView1D cb = view;
    CORRADE_COMPARE(cb.data(), Data32);
    CORRADE_COMPARE(cb.offset(), 5);
    CORRADE_COMPARE(cb.size(), 24);
    CORRADE_COMPARE(cb.stride(), 1);
}

void StridedBitArrayViewTest::constructFromMutableView() {
    char data[4]{};
    const MutableBitArrayView view{data, 5, 24};

    StridedBitArrayView1D b = view;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 24);
    CORRADE_COMPARE(b.stride(), 1);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedBitArrayView1D, MutableBitArrayView>::value);

    /* It shouldn't be possible the other way around. Not using is_convertible
       to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StridedBitArrayView1D, MutableBitArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<MutableStridedBitArrayView1D, BitArrayView>::value);
}

void StridedBitArrayViewTest::constructCopy() {
    std::uint64_t data[1]{};
    StridedBitArrayView1D a{BitArrayView{data, 5, 47}, 11, 4};

    StridedBitArrayView1D b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 11);
    CORRADE_COMPARE(b.stride(), 4);

    int data2[3];
    StridedBitArrayView1D c{data2, 0, 5};
    c = b;
    CORRADE_COMPARE(c.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 11);
    CORRADE_COMPARE(b.stride(), 4);

    CORRADE_VERIFY(std::is_copy_constructible<StridedBitArrayView1D>::value);
    CORRADE_VERIFY(std::is_copy_assignable<StridedBitArrayView1D>::value);
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    CORRADE_VERIFY(std::is_trivially_copy_constructible<StridedBitArrayView1D>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<StridedBitArrayView1D>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<StridedBitArrayView1D>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<StridedBitArrayView1D>::value);
}

template<class T> void StridedBitArrayViewTest::construct3DDefault() {
    setTestCaseTemplateName(NameFor<T>::name());

    BasicStridedBitArrayView<3, T> a;
    BasicStridedBitArrayView<3, T> b = nullptr;
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(a.offset(), 0);
    CORRADE_COMPARE(b.offset(), 0);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(b.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(b.stride(), (Stride3D{0, 0, 0}));

    constexpr BasicStridedBitArrayView<3, T> ca;
    constexpr BasicStridedBitArrayView<3, T> cb = nullptr;
    constexpr const void* dataA = ca.data();
    constexpr const void* dataB = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedDimensions<3, bool> emptyA = ca.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedDimensions<3, bool> emptyB = cb.isEmpty();
    constexpr std::size_t offsetA = ca.offset();
    constexpr std::size_t offsetB = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Size3D sizeA = ca.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Size3D sizeB = cb.size();
    constexpr Stride3D strideA = ca.stride();
    constexpr Stride3D strideB = cb.stride();
    CORRADE_COMPARE(dataA, nullptr);
    CORRADE_COMPARE(dataB, nullptr);
    CORRADE_COMPARE(offsetA, 0);
    CORRADE_COMPARE(offsetB, 0);
    CORRADE_COMPARE(emptyA, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(emptyB, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(sizeA, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(sizeB, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(strideA, (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(strideB, (Stride3D{0, 0, 0}));

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BasicStridedBitArrayView<3, T>>::value);
}

template<class T> void StridedBitArrayViewTest::construct3D() {
    setTestCaseTemplateName(NameFor<T>::name());

    std::uint64_t data[4]{};
    BasicBitArrayView<T> a{data, 5, 4*64 - 5};
    BasicStridedBitArrayView<3, T> b = {a, data + 1, 7, {3, 4, 5}, {55, 11, 2}};

    CORRADE_COMPARE(b.data(), data + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55, 11, 2}));

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<3, T>, BasicBitArrayView<T>, void*, std::size_t, Size3D, Stride3D>::value);
}

constexpr std::uint64_t Data643D[4]{};

void StridedBitArrayViewTest::construct3DConstexpr() {
    constexpr BitArrayView ca{Data643D, 5, 4*64 - 5};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView3D cb{ca, Data643D + 1, 7, {3, 4, 5}, {55, 11, 2}};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedDimensions<3, bool> empty = cb.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Size3D size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Stride3D stride = cb.stride();
    CORRADE_COMPARE(data, Data643D + 1);
    CORRADE_COMPARE(offset, 7);
    CORRADE_COMPARE(empty, (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(size, (Size3D{3, 4, 5}));
    CORRADE_COMPARE(stride, (Stride3D{55, 11, 2}));

    /* This is also expected to work -- stride() returns a const&, but size()
       a value and without it marked as const it wouldn't be possible to call
       operator[] in a constexpr context. C++ FFS. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t size0 = cb.size()[0];
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::ptrdiff_t stride0 = cb.stride()[0];
    CORRADE_COMPARE(size0, 3);
    CORRADE_COMPARE(stride0, 55);
}

void StridedBitArrayViewTest::construct3DNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later */

    StridedBitArrayView3D a{{nullptr, 5, 4*64 - 5}, nullptr, 7, {3, 4, 5}, {55, 11, 2}};
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(a.offset(), 7);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(a.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{55, 11, 2}));

    constexpr StridedBitArrayView3D ca{{nullptr, 5, 4*64 - 5}, nullptr, 7, {3, 4, 5}, {55, 11, 2}};
    CORRADE_COMPARE(ca.data(), nullptr);
    CORRADE_COMPARE(ca.offset(), 7);
    CORRADE_COMPARE(ca.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(ca.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{55, 11, 2}));
}

void StridedBitArrayViewTest::construct3DZeroStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in access3DZeroStride() */

    char data[3]{};
    BitArrayView a{data, 3, 16};
    StridedBitArrayView3D b{a, data + 1, 7, {2, 100, 4}, {4, 0, 1}};
    CORRADE_COMPARE(b.data(), data + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{2, 100, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{4, 0, 1}));
}

void StridedBitArrayViewTest::construct3DNegativeStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in access3DNegativeStride() */

    char data[23]{};
    BitArrayView a{data, 2, 22*8};
    StridedBitArrayView3D b{a, data + 17, 7, {3, 4, 5}, {-55, 11, -2}};
    CORRADE_COMPARE(b.data(), data + 17);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{-55, 11, -2}));
}

template<class T> void StridedBitArrayViewTest::construct3DSizeStride() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[23]{};
    BasicStridedBitArrayView<3, T> b = {BasicBitArrayView<T>{data, 7, 23*8 - 7}, {3, 4, 5}, {55, 11, 2}};

    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55, 11, 2}));

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<3, T>, BasicBitArrayView<T>, Size3D, Stride3D>::value);
}

void StridedBitArrayViewTest::construct3DSizeStrideConstexpr() {
    constexpr BitArrayView ca{Data643D, 7, 4*64 - 7};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView3D cb = {ca, {3, 4, 5}, {55, 11, 2}};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedDimensions<3, bool> empty = cb.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Size3D size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Stride3D stride = cb.stride();
    CORRADE_COMPARE(data, Data643D);
    CORRADE_COMPARE(offset, 7);
    CORRADE_COMPARE(empty, (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(size, (Size3D{3, 4, 5}));
    CORRADE_COMPARE(stride, (Stride3D{55, 11, 2}));
}

template<class T> void StridedBitArrayViewTest::construct3DSizeOnly() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[15]{};
    BasicStridedBitArrayView<3, T> b = {BasicBitArrayView<T>{data, 7, 15*8 - 7}, {3, 4, 5}};

    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{20, 5, 1}));

    CORRADE_VERIFY(std::is_nothrow_constructible<BasicStridedBitArrayView<3, T>, BasicBitArrayView<T>, Size3D>::value);
}

void StridedBitArrayViewTest::construct3DSizeOnlyConstexpr() {
    constexpr BitArrayView ca{Data643D, 7, 4*64 - 7};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedBitArrayView3D cb = {ca, {3, 4, 5}};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    const void* data = cb.data();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    std::size_t offset = cb.offset();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    StridedDimensions<3, bool> empty = cb.isEmpty();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Size3D size = cb.size();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, but also this is a cursed ancient compiler */
    #endif
    Stride3D stride = cb.stride();
    CORRADE_COMPARE(data, Data643D);
    CORRADE_COMPARE(offset, 7);
    CORRADE_COMPARE(empty, (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(size, (Size3D{3, 4, 5}));
    CORRADE_COMPARE(stride, (Stride3D{20, 5, 1}));
}

void StridedBitArrayViewTest::construct3DOneSizeZero() {
    int data[1]{};

    /* Assertion shouldn't fire because size in second dimension is zero */
    std::ostringstream out;
    Error redirectError{&out};
    StridedBitArrayView3D a{BitArrayView{data, 0, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out.str(), "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedBitArrayViewTest::construct3DSizeTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    /* Creating a view with zero stride, otherwise this would get caught by
       other asserts already */
    StridedBitArrayView3D{BitArrayView{nullptr, 0, 1}, {1,
        std::size_t{1} << (sizeof(std::size_t)*8 - 3), 1}, {1, 0, 1}};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Containers::StridedBitArrayView: size expected to be smaller than 2^61 bits, got {1, 2305843009213693952, 1}\n");
    #else
    CORRADE_COMPARE(out.str(), "Containers::StridedBitArrayView: size expected to be smaller than 2^29 bits, got {1, 536870912, 1}\n");
    #endif
}

void StridedBitArrayViewTest::construct3DViewTooSmall() {
    CORRADE_SKIP_IF_NO_ASSERT();

    int data[3]{};

    std::ostringstream out;
    Error redirectError{&out};
    StridedBitArrayView3D{BitArrayView{data}, {2, 5, 3}, {48, 24, 8}};
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView: data size 96 is not enough for {2, 5, 3} bits of stride {48, 24, 8}\n");
}

void StridedBitArrayViewTest::construct3DFromView() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<StridedBitArrayView1D, BitArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedBitArrayView3D, BitArrayView>::value);
}

void StridedBitArrayViewTest::construct3DFromLessDimensions() {
    /* 0b10
         01
         10'0000'0000 << 2 */
    char data[]{'\x00', '\x98'};
    StridedBitArrayView1D a{BitArrayView{data + 1, 2, 6}};
    StridedBitArrayView2D b{{data + 1, 2, 6}, {3, 2}};

    StridedBitArrayView3D a3 = a;
    CORRADE_COMPARE(a3.data(), data + 1);
    CORRADE_COMPARE(a3.offset(), 2);
    CORRADE_COMPARE(a3.size(), (Size3D{1, 1, 6}));
    CORRADE_COMPARE(a3.stride(), (Stride3D{6, 6, 1}));
    CORRADE_VERIFY(!a3[0][0][0]);
    CORRADE_VERIFY( a3[0][0][1]);
    CORRADE_VERIFY(!a3[0][0][3]);

    StridedBitArrayView3D b3 = b;
    CORRADE_COMPARE(b3.data(), data + 1);
    CORRADE_COMPARE(b3.offset(), 2);
    CORRADE_COMPARE(b3.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{6, 2, 1}));
    CORRADE_VERIFY( b3[0][0][1]);
    CORRADE_VERIFY( b3[0][1][0]);
    CORRADE_VERIFY(!b3[0][1][1]);
    CORRADE_VERIFY(!b3[0][2][0]);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedBitArrayView3D, StridedBitArrayView2D>::value);
    /* Construction the other way shouldn't be possible */
    CORRADE_VERIFY(!std::is_constructible<StridedBitArrayView2D, StridedBitArrayView3D>::value);
}

void StridedBitArrayViewTest::asContiguous() {
    /* Mostly just a copy of StridedArrayViewTest::asContiguous(), without
       filling in anything and with additional checks for offset() */

    std::uint32_t data[1]{};
    BitArrayView a{data, 2, 2*3*5};
    StridedBitArrayView3D b{a, {5, 3, 2}, {6, 2, 1}};

    CORRADE_VERIFY(b.isContiguous<2>());
    CORRADE_VERIFY(b.isContiguous<1>());
    CORRADE_VERIFY(b.isContiguous<0>());
    CORRADE_VERIFY(b.isContiguous());

    {
        BitArrayView bc = b.asContiguous();
        CORRADE_COMPARE(bc.data(), b.data());
        CORRADE_COMPARE(bc.offset(), 2);
        CORRADE_COMPARE(bc.size(), 5*3*2);

        StridedBitArrayView1D b0 = b.asContiguous<0>();
        CORRADE_COMPARE(b0.data(), b.data());
        CORRADE_COMPARE(b0.offset(), 2);
        CORRADE_COMPARE(b0.size(), 5*3*2);
        CORRADE_COMPARE(b0.stride(), 1);

        StridedBitArrayView2D b1 = b.asContiguous<1>();
        CORRADE_COMPARE(b1.data(), b.data());
        CORRADE_COMPARE(b1.offset(), 2);
        CORRADE_COMPARE(b1.size(), (Size2D{5, 3*2}));
        CORRADE_COMPARE(b1.stride(), (Stride2D{3*2, 1}));

        /* This should return the exact same view */
        StridedBitArrayView3D b2 = b.asContiguous<2>();
        CORRADE_COMPARE(b2.data(), b.data());
        CORRADE_COMPARE(b2.offset(), b.offset());
        CORRADE_COMPARE(b2.size(), b.size());
        CORRADE_COMPARE(b2.stride(), b.stride());

    /* Non-contiguous in the first dimension */
    } {
        StridedBitArrayView3D c{a, {2, 3, 2}, {2*6, 2, 1}};
        CORRADE_VERIFY(c.isContiguous<2>());
        CORRADE_VERIFY(c.isContiguous<1>());
        CORRADE_VERIFY(!c.isContiguous<0>());
        CORRADE_VERIFY(!c.isContiguous());

        StridedBitArrayView2D c1 = c.asContiguous<1>();
        CORRADE_COMPARE(c1.data(), c.data());
        CORRADE_COMPARE(c1.offset(), 2);
        CORRADE_COMPARE(c1.size(), (Size2D{2, 3*2}));
        CORRADE_COMPARE(c1.stride(), (Stride2D{2*6, 1}));

        /* This should return the exact same view */
        StridedBitArrayView3D c2 = c.asContiguous<2>();
        CORRADE_COMPARE(c2.data(), c.data());
        CORRADE_COMPARE(c2.offset(), c.offset());
        CORRADE_COMPARE(c2.size(), c.size());
        CORRADE_COMPARE(c2.stride(), c.stride());

    /* Non-contiguous in the second dimension */
    } {
        StridedBitArrayView3D d{a, {5, 1, 2}, {6, 2*2, 1}};
        CORRADE_VERIFY(d.isContiguous<2>());
        CORRADE_VERIFY(!d.isContiguous<1>());
        CORRADE_VERIFY(!d.isContiguous<0>());

        /* This should return the exact same view */
        StridedBitArrayView3D d2 = d.asContiguous<2>();
        CORRADE_COMPARE(d2.data(), d.data());
        CORRADE_COMPARE(d2.offset(), d.offset());
        CORRADE_COMPARE(d2.size(), d.size());
        CORRADE_COMPARE(d2.stride(), d.stride());

    /* Not contigous in the third dimension, can't create any view */
    } {
        StridedBitArrayView3D e{a, {5, 3, 1}, {6, 2, 2}};
        CORRADE_VERIFY(!e.isContiguous<2>());
        CORRADE_VERIFY(!e.isContiguous<1>());
        CORRADE_VERIFY(!e.isContiguous<0>());

    /* "Broadcast" */
    } {
        StridedBitArrayView3D f{a, {5, 3, 2}, {6, 0, 1}};
        CORRADE_VERIFY(f.isContiguous<2>());
        CORRADE_VERIFY(!f.isContiguous<1>());
        CORRADE_VERIFY(!f.isContiguous<0>());

        /* This should again return the exact same view */
        StridedBitArrayView3D f2 = f.asContiguous<2>();
        CORRADE_COMPARE(f2.data(), f.data());
        CORRADE_COMPARE(f2.offset(), f.offset());
        CORRADE_COMPARE(f2.size(), f.size());
        CORRADE_COMPARE(f2.stride(), f.stride());
    }

    /* Packed block of bits, but strides not in order / negative */
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<2>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<1>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<0>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<2>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<1>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<0>());
}

void StridedBitArrayViewTest::asContiguousNonContiguous() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* Taken from above */
    std::uint32_t data[16]{};
    BitArrayView a{data, 7, 2*3*5*4};
    StridedBitArrayView3D c{a, {2, 3, 2}, {2*6*4, 2*4, 4}};
    StridedBitArrayView3D d{a, {5, 1, 2}, {6*4, 2*2*4, 4}};
    StridedBitArrayView3D e{a, {5, 3, 1}, {6*4, 2*4, 2*4}};

    std::ostringstream out;
    Error redirectError{&out};
    c.asContiguous();
    c.asContiguous<0>();
    d.asContiguous<1>();
    e.asContiguous<2>();
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous\n"
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous from dimension 0\n"
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous from dimension 1\n"
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous from dimension 2\n");
}

/* 0b0101'0101'0011'0011'0000'1111'0000'0000 << 5
       0   1   0  1   1   0   1  1 */
constexpr char DataPadded[]{'\x00', '\xe0', '\x61', '\xa6', '\x0a'};

void StridedBitArrayViewTest::access() {
    const StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 8, 3};

    for(std::size_t i: {0, 1, 3, 4, 6}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
    }

    for(std::size_t i: {2, 5, 7}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!a[i]);
    }
}

void StridedBitArrayViewTest::accessMutableSet() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE_AS(data.stride, 0, TestSuite::Compare::Greater);

    std::uint32_t valueA[]{0, data.valueSet};
    std::uint32_t valueB[]{0, data.valueSet};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{valueA + 1, data.offset, 24}, std::size_t(24/data.stride), data.stride};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{valueB + 1, data.offset, 24}, std::size_t(24/data.stride), data.stride};

    a.set(data.bit);
    b.set(data.bit, true);
    CORRADE_COMPARE(valueA[1], data.expectedSet);
    CORRADE_COMPARE(valueB[1], data.expectedSet);
}

void StridedBitArrayViewTest::accessMutableReset() {
    auto&& data = AccessMutableData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE_AS(data.stride, 0, TestSuite::Compare::Greater);

    std::uint32_t valueA[]{0, data.valueReset};
    std::uint32_t valueB[]{0, data.valueReset};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{valueA + 1, data.offset, 24}, std::size_t(24/data.stride), data.stride};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{valueB + 1, data.offset, 24}, std::size_t(24/data.stride), data.stride};

    a.reset(data.bit);
    b.set(data.bit, false);
    CORRADE_COMPARE(valueA[1], data.expectedReset);
    CORRADE_COMPARE(valueB[1], data.expectedReset);
}

void StridedBitArrayViewTest::accessZeroStride() {
    /* Picks the initial bit in 1111'0000, thus all values are one */
    StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 100, 0};
    /* Picks one bit before the initial 1111'0000, thus all values are zero */
    StridedBitArrayView1D b{BitArrayView{DataPadded + 1, 4, 24}, 100, 0};
    for(std::size_t i = 0; i != 100; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
        CORRADE_VERIFY(!b[i]);
    }
}

void StridedBitArrayViewTest::accessZeroStrideMutableSet() {
    auto&& data = AccessMutableZeroStrideData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint32_t valueA[]{0, data.valueSet};
    std::uint32_t valueB[]{0, data.valueSet};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{valueA + 1, data.offset, 24}, 2000000, 0};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{valueB + 1, data.offset, 24}, 2000000, 0};

    a.set(data.bit);
    b.set(data.bit, true);
    CORRADE_COMPARE(valueA[1], data.expectedSet);
    CORRADE_COMPARE(valueB[1], data.expectedSet);
}

void StridedBitArrayViewTest::accessZeroStrideMutableReset() {
    auto&& data = AccessMutableZeroStrideData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint32_t valueA[]{0, data.valueReset};
    std::uint32_t valueB[]{0, data.valueReset};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{valueA + 1, data.offset, 24}, 2000000, 0};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{valueB + 1, data.offset, 24}, 2000000, 0};

    a.reset(data.bit);
    b.set(data.bit, false);
    CORRADE_COMPARE(valueA[1], data.expectedReset);
    CORRADE_COMPARE(valueB[1], data.expectedReset);
}

void StridedBitArrayViewTest::accessNegativeStride() {
    /* Like access(), but with the numbers reversed */

    const StridedBitArrayView1D a{BitArrayView{DataPadded + 4, 2 /* complement to 7 */, 24}, 8, -3};

    for(std::size_t i: {7, 6, 4, 3, 1}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
    }

    for(std::size_t i: {5, 2, 0}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!a[i]);
    }
}

void StridedBitArrayViewTest::accessNegativeStrideMutableSet() {
    auto&& data = AccessMutableNegativeStrideData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE_AS(data.stride, 0, TestSuite::Compare::Less);

    std::uint32_t valueA[]{0, data.valueSet};
    std::uint32_t valueB[]{0, data.valueSet};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{reinterpret_cast<char*>(valueA) + 7, data.offset, 24}, std::size_t(24/-data.stride), data.stride};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{reinterpret_cast<char*>(valueB) + 7, data.offset, 24}, std::size_t(24/-data.stride), data.stride};

    a.set(data.bit);
    b.set(data.bit, true);
    CORRADE_COMPARE(valueA[1], data.expectedSet);
    CORRADE_COMPARE(valueB[1], data.expectedSet);
}

void StridedBitArrayViewTest::accessNegativeStrideMutableReset() {
    auto&& data = AccessMutableNegativeStrideData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE_AS(data.stride, 0, TestSuite::Compare::Less);

    std::uint32_t valueA[]{0, data.valueReset};
    std::uint32_t valueB[]{0, data.valueReset};
    const MutableStridedBitArrayView1D a{MutableBitArrayView{reinterpret_cast<char*>(valueA) + 7, data.offset, 24}, std::size_t(24/-data.stride), data.stride};
    const MutableStridedBitArrayView1D b{MutableBitArrayView{reinterpret_cast<char*>(valueB) + 7, data.offset, 24}, std::size_t(24/-data.stride), data.stride};

    a.reset(data.bit);
    b.set(data.bit, false);
    CORRADE_COMPARE(valueA[1], data.expectedReset);
    CORRADE_COMPARE(valueB[1], data.expectedReset);
}

void StridedBitArrayViewTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::uint64_t data[1]{};
    MutableStridedBitArrayView1D a{MutableBitArrayView{data, 4, 53}, 26, 2};

    std::stringstream out;
    Error redirectError{&out};
    a[26];
    a.set(26);
    a.reset(26);
    a.set(26, true);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::operator[](): index 26 out of range for 26 elements\n"
        "Containers::StridedBitArrayView::set(): index 26 out of range for 26 bits\n"
        "Containers::StridedBitArrayView::reset(): index 26 out of range for 26 bits\n"
        "Containers::StridedBitArrayView::set(): index 26 out of range for 26 bits\n");
}

/* Three slices, each with four rows plus one row padding and nine bits plus
   two bit padding, picking every 2nd bit; bit pattern being subdivided into a
   half and alternating between the outer and inner slice, and the whole data
   shifted by 7 bits, thus with 11*5 bits per slice the second slice is shifted
   by 6 bits and the third by 5.

    Bits 0 to 54:         Bits 55 to 109:        Bits 110 to 164:
    Bytes 0 to 7:         Bytes 7 to 15:         Bytes 15 to 22:

    0b001'1111'1111       0b000'0000'0000        0b001'1111'1111
        1  1 1  1 1           0  0 0  0 0            1  1 1  1 1
    0b000'0000'0000       0b001'0000'1111        0b000'0000'0000
        0  0 0  0 0           1  0 0  1 1            0  0 0  0 0
    0b001'0011'0011       0b000'0000'0000        0b001'0011'0011
        1  0 1  0 1           0  0 0  0 0            1  0 1  0 1
    0b000'0000'0000       0b001'0101'0101        0b000'0000'0000
        0  0 0  0 0           1  1 1  1 1            0  0 0  0 0
    0b000'0000'0000 << 7  0b000'0000'0000 << 6   0b000'0000'0000 << 5 */
constexpr char DataPadded3D[]{
    '\x00',
    '\x80', '\xff', '\x00', '\x60', '\x26', '\x00', '\x00', '\x00'| /* shared */
    '\x00', '\x00', '\x1e', '\x02', '\x80', '\xaa', '\x00', '\x00'| /* shared */
    '\xe0', '\x3f', '\x00', '\x98', '\x09', '\x00', '\x00', '\x00'
};

void StridedBitArrayViewTest::access3D() {
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    /* Size and stride should be just a suffix no matter which view gets
       chosen */
    for(std::size_t i0 = 0; i0 != 3; ++i0) {
        CORRADE_ITERATION(i0);

        StridedBitArrayView2D slice = a[i0];
        CORRADE_COMPARE(slice.size(), (Size2D{4, 5}));
        CORRADE_COMPARE(slice.stride(), (Stride2D{11, 2}));

        for(std::size_t i1 = 0; i1 != 4; ++i1) {
            CORRADE_ITERATION(i1);

            StridedBitArrayView1D row = slice[i1];
            CORRADE_COMPARE(row.size(), 5);
            CORRADE_COMPARE(row.stride(), 2);
        }
    }

    const StridedBitArrayView2D slice0 = a[0];
    const StridedBitArrayView1D row00 = slice0[0];
    const StridedBitArrayView1D row01 = slice0[1];
    const StridedBitArrayView1D row02 = slice0[2];
    const StridedBitArrayView1D row03 = slice0[3];
    CORRADE_COMPARE(slice0.offset(), 7);
    CORRADE_COMPARE(row00.offset(), slice0.offset());
    CORRADE_COMPARE(row01.offset(), 2);
    CORRADE_COMPARE(row02.offset(), 5);
    CORRADE_COMPARE(row03.offset(), 0);

    const StridedBitArrayView2D slice1 = a[1];
    const StridedBitArrayView1D row10 = slice1[0];
    const StridedBitArrayView1D row11 = slice1[1];
    const StridedBitArrayView1D row12 = slice1[2];
    const StridedBitArrayView1D row13 = slice1[3];
    CORRADE_COMPARE(slice1.offset(), 6);
    CORRADE_COMPARE(row10.offset(), slice1.offset());
    CORRADE_COMPARE(row11.offset(), 1);
    CORRADE_COMPARE(row12.offset(), 4);
    CORRADE_COMPARE(row13.offset(), 7);

    const StridedBitArrayView2D slice2 = a[2];
    const StridedBitArrayView1D row20 = slice2[0];
    const StridedBitArrayView1D row21 = slice2[1];
    const StridedBitArrayView1D row22 = slice2[2];
    const StridedBitArrayView1D row23 = slice2[3];
    CORRADE_COMPARE(slice2.offset(), 5);
    CORRADE_COMPARE(row20.offset(), slice2.offset());
    CORRADE_COMPARE(row21.offset(), 0);
    CORRADE_COMPARE(row22.offset(), 3);
    CORRADE_COMPARE(row23.offset(), 6);

    /* All one / all zero rows */
    for(std::size_t i: {0, 1, 2, 3, 4}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(row00[i]);
        CORRADE_VERIFY(row13[i]);
        CORRADE_VERIFY(row20[i]);

        CORRADE_VERIFY(!row01[i]);
        CORRADE_VERIFY(!row03[i]);
        CORRADE_VERIFY(!row10[i]);
        CORRADE_VERIFY(!row12[i]);
        CORRADE_VERIFY(!row21[i]);
        CORRADE_VERIFY(!row23[i]);
    }

    /* Slices 0 and 2 are the same */
    for(std::size_t i: {0, 2, 4}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(row02[i]);
        CORRADE_VERIFY(row22[i]);
    }
    for(std::size_t i: {1, 3}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!row02[i]);
        CORRADE_VERIFY(!row22[i]);
    }

    CORRADE_VERIFY(row11[0]);
    CORRADE_VERIFY(row11[1]);
    CORRADE_VERIFY(!row11[2]);
    CORRADE_VERIFY(!row11[3]);
    CORRADE_VERIFY(row11[4]);
}

void StridedBitArrayViewTest::access3DMutable() {
    /* Mutable access is currently limited to a single dimension so just verify
       that accessing the last dimension preserves the mutability. Everything
       else is tested well enough in accessMutable{Set,Reset}() already. */

    std::uint32_t data[]{
        0x00000000u,
        0x00ffff00u,

        0xff0000ffu,
        0xffffffffu,
    };
    MutableStridedBitArrayView3D a{MutableBitArrayView{data}, {2, 2, 32}};

    a[1][0].set(20);
    a[1][0].set(11, true);
    a[0][1].reset(11);
    a[0][1].set(20, false);

    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView({
        0x00000000u,
        0x00eff700u,

        0xff1008ffu,
        0xffffffffu,
    }), TestSuite::Compare::Container);
}

void StridedBitArrayViewTest::access3DZeroStride() {
    /* Compared to access3D(), with the first and last stride zero, effectively
       picks the first row of the first slice:

        0b...1
             1
        0b...0
             0
        0b...1
             1
        0b...0 << 7
             0 */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {0, 11, 0}};

    /* Size and stride should be just a suffix no matter which view gets
       chosen */
    for(std::size_t i0 = 0; i0 != 3; ++i0) {
        CORRADE_ITERATION(i0);

        StridedBitArrayView2D slice = a[i0];
        CORRADE_COMPARE(slice.size(), (Size2D{4, 5}));
        CORRADE_COMPARE(slice.stride(), (Stride2D{11, 0}));

        for(std::size_t i1 = 0; i1 != 4; ++i1) {
            CORRADE_ITERATION(i1);

            StridedBitArrayView1D row = slice[i1];
            CORRADE_COMPARE(row.size(), 5);
            CORRADE_COMPARE(row.stride(), 0);
        }
    }

    /* All slices are the same */
    for(std::size_t i0 = 0; i0 != a.size()[0]; ++i0) {
        CORRADE_ITERATION(i0);

        const StridedBitArrayView2D slice = a[i0];
        const StridedBitArrayView1D row0 = slice[0];
        const StridedBitArrayView1D row1 = slice[1];
        const StridedBitArrayView1D row2 = slice[2];
        const StridedBitArrayView1D row3 = slice[3];
        CORRADE_COMPARE(slice.offset(), 7);
        CORRADE_COMPARE(row0.offset(), slice.offset());
        CORRADE_COMPARE(row1.offset(), 2);
        CORRADE_COMPARE(row2.offset(), 5);
        CORRADE_COMPARE(row3.offset(), 0);

        /* All bits in a particular row are the same */
        for(std::size_t i2 = 0; i2 != 5; ++i2) {
            CORRADE_ITERATION(i2);
            CORRADE_VERIFY(row0[i2]);
            CORRADE_VERIFY(!row1[i2]);
            CORRADE_VERIFY(row2[i2]);
            CORRADE_VERIFY(!row3[i2]);
        }
    }
}

void StridedBitArrayViewTest::access3DZeroStrideMutable() {
    /* Like access3DMutable() but with the second stride zero, so just a single
       row of data */

    std::uint32_t data[]{
        0x00ffff00u,
        0xff0000ffu
    };
    MutableStridedBitArrayView3D a{MutableBitArrayView{data}, {2, 2, 32}, {32, 0, 1}};

    a[1][0].set(20);
    a[1][0].set(11, true);
    a[0][1].reset(11);
    a[0][1].set(20, false);

    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView({
        0x00eff700u,
        0xff1008ffu,
    }), TestSuite::Compare::Container);
}

void StridedBitArrayViewTest::access3DNegativeStride() {
    /* Compared to access3D(), with the first and last stride negative,
       effectively flips the first and last slice (which are the same) and
       the rows themselves */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, DataPadded3D + 16, 5, {3, 4, 5}, {-55, 11, -2}};

    /* Size and stride should be just a suffix no matter which view gets
       chosen */
    for(std::size_t i0 = 0; i0 != 3; ++i0) {
        CORRADE_ITERATION(i0);

        StridedBitArrayView2D slice = a[i0];
        CORRADE_COMPARE(slice.size(), (Size2D{4, 5}));
        CORRADE_COMPARE(slice.stride(), (Stride2D{11, -2}));

        for(std::size_t i1 = 0; i1 != 4; ++i1) {
            CORRADE_ITERATION(i1);

            StridedBitArrayView1D row = slice[i1];
            CORRADE_COMPARE(row.size(), 5);
            CORRADE_COMPARE(row.stride(), -2);
        }
    }

    const StridedBitArrayView2D slice0 = a[0];
    const StridedBitArrayView1D row00 = slice0[0];
    const StridedBitArrayView1D row01 = slice0[1];
    const StridedBitArrayView1D row02 = slice0[2];
    const StridedBitArrayView1D row03 = slice0[3];
    CORRADE_COMPARE(slice0.offset(), 5);
    CORRADE_COMPARE(row00.offset(), slice0.offset());
    CORRADE_COMPARE(row01.offset(), 0);
    CORRADE_COMPARE(row02.offset(), 3);
    CORRADE_COMPARE(row03.offset(), 6);

    const StridedBitArrayView2D slice1 = a[1];
    const StridedBitArrayView1D row10 = slice1[0];
    const StridedBitArrayView1D row11 = slice1[1];
    const StridedBitArrayView1D row12 = slice1[2];
    const StridedBitArrayView1D row13 = slice1[3];
    CORRADE_COMPARE(slice1.offset(), 6);
    CORRADE_COMPARE(row10.offset(), slice1.offset());
    CORRADE_COMPARE(row11.offset(), 1);
    CORRADE_COMPARE(row12.offset(), 4);
    CORRADE_COMPARE(row13.offset(), 7);

    const StridedBitArrayView2D slice2 = a[2];
    const StridedBitArrayView1D row20 = slice2[0];
    const StridedBitArrayView1D row21 = slice2[1];
    const StridedBitArrayView1D row22 = slice2[2];
    const StridedBitArrayView1D row23 = slice2[3];
    CORRADE_COMPARE(slice2.offset(), 7);
    CORRADE_COMPARE(row20.offset(), slice2.offset());
    CORRADE_COMPARE(row21.offset(), 2);
    CORRADE_COMPARE(row22.offset(), 5);
    CORRADE_COMPARE(row23.offset(), 0);

    /* All one / all zero rows */
    for(std::size_t i: {4, 3, 2, 1, 0}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(row00[i]);
        CORRADE_VERIFY(row13[i]);
        CORRADE_VERIFY(row20[i]);

        CORRADE_VERIFY(!row01[i]);
        CORRADE_VERIFY(!row03[i]);
        CORRADE_VERIFY(!row10[i]);
        CORRADE_VERIFY(!row12[i]);
        CORRADE_VERIFY(!row21[i]);
        CORRADE_VERIFY(!row23[i]);
    }

    /* Slices 0 and 2 are the same, with flipped order compared to access3D() */
    for(std::size_t i: {4, 2, 0}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(row02[i]);
        CORRADE_VERIFY(row22[i]);
    }
    for(std::size_t i: {3, 1}) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(!row02[i]);
        CORRADE_VERIFY(!row22[i]);
    }

    CORRADE_VERIFY(row11[4]);
    CORRADE_VERIFY(row11[3]);
    CORRADE_VERIFY(!row11[2]);
    CORRADE_VERIFY(!row11[1]);
    CORRADE_VERIFY(row11[0]);
}

void StridedBitArrayViewTest::access3DNegativeStrideMutable() {
    /* Like access3DMutable() but with all strides and indices negative */

    std::uint32_t data[]{
        0x00000000u,
        0x00ffff00u,

        0xff0000ffu,
        0xffffffffu,
    };
    MutableStridedBitArrayView3D a{MutableBitArrayView{data}, reinterpret_cast<char*>(data) + 15, 7, {2, 2, 32}, {-64, -32, -1}};

    a[0][1].set(11);
    a[0][1].set(20, true);
    a[1][0].reset(20);
    a[1][0].set(11, false);

    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView({
        0x00000000u,
        0x00eff700u,

        0xff1008ffu,
        0xffffffffu,
    }), TestSuite::Compare::Container);
}

void StridedBitArrayViewTest::access3DInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::stringstream out;
    Error redirectError{&out};

    std::uint32_t data[1]{};
    MutableStridedBitArrayView3D b{MutableBitArrayView{data, 7, 24}, {1, 2, 3}, {24, 12, 4}};
    b[1];
    b[0][2];
    b[0][1].set(3);
    b[0][0].reset(3);
    b[0][1].set(3, false);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::operator[](): index 1 out of range for 1 elements\n"
        "Containers::StridedBitArrayView::operator[](): index 2 out of range for 2 elements\n"
        "Containers::StridedBitArrayView::set(): index 3 out of range for 3 bits\n"
        "Containers::StridedBitArrayView::reset(): index 3 out of range for 3 bits\n"
        "Containers::StridedBitArrayView::set(): index 3 out of range for 3 bits\n");
}

void StridedBitArrayViewTest::slice() {
    /* Same input as in access() */
    const StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 8, 3};

    StridedBitArrayView1D b1 = a.slice(1, 5);
    CORRADE_COMPARE(b1.data(), DataPadded + 2);
    CORRADE_COMPARE(b1.offset(), 0);
    CORRADE_COMPARE(b1.size(), 4);
    CORRADE_COMPARE(b1.stride(), 3);
    CORRADE_VERIFY( b1[0]);
    CORRADE_VERIFY(!b1[1]);
    CORRADE_VERIFY( b1[2]);
    CORRADE_VERIFY( b1[3]);

    StridedBitArrayView1D b2 = a.sliceSize(1, 4);
    CORRADE_COMPARE(b2.data(), DataPadded + 2);
    CORRADE_COMPARE(b2.offset(), 0);
    CORRADE_COMPARE(b2.size(), 4);
    CORRADE_COMPARE(b2.stride(), 3);
    CORRADE_VERIFY( b2[0]);
    CORRADE_VERIFY(!b2[1]);
    CORRADE_VERIFY( b2[2]);
    CORRADE_VERIFY( b2[3]);

    StridedBitArrayView1D c = a.prefix(4);
    CORRADE_COMPARE(c.data(), DataPadded + 1);
    CORRADE_COMPARE(c.offset(), 5);
    CORRADE_COMPARE(c.size(), 4);
    CORRADE_COMPARE(c.stride(), 3);
    CORRADE_VERIFY( c[0]);
    CORRADE_VERIFY( c[1]);
    CORRADE_VERIFY(!c[2]);
    CORRADE_VERIFY( c[3]);

    StridedBitArrayView1D d = a.suffix(4);
    CORRADE_COMPARE(d.data(), DataPadded + 3);
    CORRADE_COMPARE(d.offset(), 1);
    CORRADE_COMPARE(d.size(), 4);
    CORRADE_COMPARE(d.stride(), 3);
    CORRADE_VERIFY( d[0]);
    CORRADE_VERIFY(!d[1]);
    CORRADE_VERIFY( d[2]);
    CORRADE_VERIFY(!d[3]);

    StridedBitArrayView1D e = a.exceptPrefix(5);
    CORRADE_COMPARE(e.data(), DataPadded + 3);
    CORRADE_COMPARE(e.offset(), 4);
    CORRADE_COMPARE(e.size(), 3);
    CORRADE_COMPARE(e.stride(), 3);
    CORRADE_VERIFY(!e[0]);
    CORRADE_VERIFY( e[1]);
    CORRADE_VERIFY(!e[2]);

    StridedBitArrayView1D f = a.exceptSuffix(5);
    CORRADE_COMPARE(f.data(), DataPadded + 1);
    CORRADE_COMPARE(f.offset(), 5);
    CORRADE_COMPARE(f.size(), 3);
    CORRADE_COMPARE(f.stride(), 3);
    CORRADE_VERIFY( f[0]);
    CORRADE_VERIFY( f[1]);
    CORRADE_VERIFY(!f[2]);
}

void StridedBitArrayViewTest::sliceInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    const StridedBitArrayView1D a{BitArrayView{nullptr, 5, 24}, 5, 3};

    std::ostringstream out;
    Error redirectError{&out};
    a.slice(5, 6);
    a.slice(2, 1);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StridedBitArrayView::slice(): slice [2:1] out of range for 5 elements\n");
}

void StridedBitArrayViewTest::slice3D() {
    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    StridedBitArrayView3D b1 = a.slice({0, 1, 2}, {1, 3, 5});
    CORRADE_COMPARE(b1.data(), DataPadded3D + 3);
    CORRADE_COMPARE(b1.offset(), 6);
    CORRADE_COMPARE(b1.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b1.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY(!b1[0][0][0]);
    CORRADE_VERIFY(!b1[0][0][1]);
    CORRADE_VERIFY(!b1[0][0][2]);
    CORRADE_VERIFY( b1[0][1][0]);
    CORRADE_VERIFY(!b1[0][1][1]);
    CORRADE_VERIFY( b1[0][1][2]);

    StridedBitArrayView3D b2 = a.sliceSize({0, 1, 2}, {1, 2, 3});
    CORRADE_COMPARE(b2.data(), DataPadded3D + 3);
    CORRADE_COMPARE(b2.offset(), 6);
    CORRADE_COMPARE(b2.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b2.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY(!b2[0][0][0]);
    CORRADE_VERIFY(!b2[0][0][1]);
    CORRADE_VERIFY(!b2[0][0][2]);
    CORRADE_VERIFY( b2[0][1][0]);
    CORRADE_VERIFY(!b2[0][1][1]);
    CORRADE_VERIFY( b2[0][1][2]);

    StridedBitArrayView3D c = a.prefix({1, 3, 1});
    CORRADE_COMPARE(c.data(), DataPadded3D + 1);
    CORRADE_COMPARE(c.offset(), 7);
    CORRADE_COMPARE(c.size(), (Size3D{1, 3, 1}));
    CORRADE_COMPARE(c.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( c[0][0][0]);
    CORRADE_VERIFY(!c[0][1][0]);
    CORRADE_VERIFY( c[0][2][0]);

    StridedBitArrayView3D d = a.suffix({1, 3, 1});
    CORRADE_COMPARE(d.data(), DataPadded3D + 18);
    CORRADE_COMPARE(d.offset(), 0);
    CORRADE_COMPARE(d.size(), (Size3D{1, 3, 1}));
    CORRADE_COMPARE(d.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY(!d[0][0][0]);
    CORRADE_VERIFY( d[0][1][0]);
    CORRADE_VERIFY(!d[0][2][0]);

    StridedBitArrayView3D e = a.exceptPrefix({2, 2, 2});
    CORRADE_COMPARE(e.data(), DataPadded3D + 18);
    CORRADE_COMPARE(e.offset(), 7);
    CORRADE_COMPARE(e.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(e.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( e[0][0][0]);
    CORRADE_VERIFY(!e[0][0][1]);
    CORRADE_VERIFY( e[0][0][2]);
    CORRADE_VERIFY(!e[0][1][0]);
    CORRADE_VERIFY(!e[0][1][1]);
    CORRADE_VERIFY(!e[0][1][2]);

    StridedBitArrayView3D f = a.exceptSuffix({2, 2, 2});
    CORRADE_COMPARE(f.data(), DataPadded3D + 1);
    CORRADE_COMPARE(f.offset(), 7);
    CORRADE_COMPARE(f.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(f.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( f[0][0][0]);
    CORRADE_VERIFY( f[0][0][1]);
    CORRADE_VERIFY( f[0][0][2]);
    CORRADE_VERIFY(!f[0][1][0]);
    CORRADE_VERIFY(!f[0][1][1]);
    CORRADE_VERIFY(!f[0][1][2]);
}

void StridedBitArrayViewTest::slice3DInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    StridedBitArrayView3D a{BitArrayView{nullptr, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    std::ostringstream out;
    Error redirectError{&out};
    a.slice({1, 0, 1}, {3, 5, 3});
    a.slice({2, 0, 1}, {0, 4, 3});
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::slice(): slice [{1, 0, 1}:{3, 5, 3}] out of range for {3, 4, 5} elements in dimension 1\n"
        "Containers::StridedBitArrayView::slice(): slice [{2, 0, 1}:{0, 4, 3}] out of range for {3, 4, 5} elements in dimension 0\n");
}

void StridedBitArrayViewTest::slice3DFirstDimension() {
    /* Same input as in access3D(), except that it's limited to three rows and
       two values in each */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 3, 2}, {55, 11, 2}};

    StridedBitArrayView3D b1 = a.slice(1, 2);
    CORRADE_COMPARE(b1.data(), DataPadded3D + 8);
    CORRADE_COMPARE(b1.offset(), 6);
    CORRADE_COMPARE(b1.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(b1.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY(!b1[0][0][0]);
    CORRADE_VERIFY(!b1[0][0][1]);
    CORRADE_VERIFY( b1[0][1][0]);
    CORRADE_VERIFY( b1[0][1][1]);
    CORRADE_VERIFY(!b1[0][2][0]);
    CORRADE_VERIFY(!b1[0][2][1]);

    StridedBitArrayView3D b2 = a.sliceSize(1, 1);
    CORRADE_COMPARE(b2.data(), DataPadded3D + 8);
    CORRADE_COMPARE(b2.offset(), 6);
    CORRADE_COMPARE(b2.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(b2.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY(!b2[0][0][0]);
    CORRADE_VERIFY(!b2[0][0][1]);
    CORRADE_VERIFY( b2[0][1][0]);
    CORRADE_VERIFY( b2[0][1][1]);
    CORRADE_VERIFY(!b2[0][2][0]);
    CORRADE_VERIFY(!b2[0][2][1]);

    StridedBitArrayView3D c = a.prefix(1);
    CORRADE_COMPARE(c.data(), DataPadded3D + 1);
    CORRADE_COMPARE(c.offset(), 7);
    CORRADE_COMPARE(c.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(c.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( c[0][0][0]);
    CORRADE_VERIFY( c[0][0][1]);
    CORRADE_VERIFY(!c[0][1][0]);
    CORRADE_VERIFY(!c[0][1][1]);
    CORRADE_VERIFY( c[0][2][0]);
    CORRADE_VERIFY(!c[0][2][1]);

    StridedBitArrayView3D d = a.suffix(1);
    CORRADE_COMPARE(d.data(), DataPadded3D + 15);
    CORRADE_COMPARE(d.offset(), 5);
    CORRADE_COMPARE(d.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(d.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( d[0][0][0]);
    CORRADE_VERIFY( d[0][0][1]);
    CORRADE_VERIFY(!d[0][1][0]);
    CORRADE_VERIFY(!d[0][1][1]);
    CORRADE_VERIFY( d[0][2][0]);
    CORRADE_VERIFY(!d[0][2][1]);

    StridedBitArrayView3D e = a.exceptPrefix(2);
    CORRADE_COMPARE(e.data(), DataPadded3D + 15);
    CORRADE_COMPARE(e.offset(), 5);
    CORRADE_COMPARE(e.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(e.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( e[0][0][0]);
    CORRADE_VERIFY( e[0][0][1]);
    CORRADE_VERIFY(!e[0][1][0]);
    CORRADE_VERIFY(!e[0][1][1]);
    CORRADE_VERIFY( e[0][2][0]);
    CORRADE_VERIFY(!e[0][2][1]);

    StridedBitArrayView3D f = a.exceptSuffix(2);
    CORRADE_COMPARE(f.data(), DataPadded3D + 1);
    CORRADE_COMPARE(f.offset(), 7);
    CORRADE_COMPARE(f.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(f.stride(), (Stride3D{55, 11, 2}));
    CORRADE_VERIFY( f[0][0][0]);
    CORRADE_VERIFY( f[0][0][1]);
    CORRADE_VERIFY(!f[0][1][0]);
    CORRADE_VERIFY(!f[0][1][1]);
    CORRADE_VERIFY( f[0][2][0]);
    CORRADE_VERIFY(!f[0][2][1]);
}

void StridedBitArrayViewTest::slice3DFirstDimensionInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    StridedBitArrayView3D a{BitArrayView{nullptr, 7, 23*8}, {3, 3, 2}, {55, 11, 2}};

    std::ostringstream out;
    Error redirectError{&out};
    a.slice(3, 4);
    a.slice(2, 1);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::slice(): slice [3:4] out of range for 3 elements\n"
        "Containers::StridedBitArrayView::slice(): slice [2:1] out of range for 3 elements\n");
}

void StridedBitArrayViewTest::every() {
    /* Same input as in access() */
    const StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 8, 3};

    /* No-op */
    StridedBitArrayView1D b = a.every(1);
    CORRADE_COMPARE(b.data(), DataPadded + 1);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 8);
    CORRADE_COMPARE(b.stride(), 3);
    CORRADE_VERIFY( b[0]);
    CORRADE_VERIFY( b[1]);
    CORRADE_VERIFY(!b[2]);
    CORRADE_VERIFY( b[3]);
    CORRADE_VERIFY( b[4]);
    CORRADE_VERIFY(!b[5]);
    CORRADE_VERIFY( b[6]);
    CORRADE_VERIFY(!b[7]);

    /* Data and offset stays the same, size and stride is adjusted */
    StridedBitArrayView1D c = a.every(2);
    CORRADE_COMPARE(c.data(), DataPadded + 1);
    CORRADE_COMPARE(c.offset(), 5);
    CORRADE_COMPARE(c.size(), 4);
    CORRADE_COMPARE(c.stride(), 6);
    CORRADE_VERIFY( c[0]);
    CORRADE_VERIFY(!c[1]);
    CORRADE_VERIFY( c[2]);
    CORRADE_VERIFY( c[3]);

    CORRADE_COMPARE(a.every(4).size(), 2);
    CORRADE_COMPARE(a.every(8).size(), 1);
}

void StridedBitArrayViewTest::everyInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};

    StridedBitArrayView1D{}.every(0);
    CORRADE_COMPARE(out.str(), "Containers::StridedBitArrayView::every(): expected a non-zero step, got {0}\n");
}

void StridedBitArrayViewTest::every3D() {
    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    /* Data and offset stays the same, size and stride is adjusted */
    StridedBitArrayView3D b = a.every({4, 3, 2});
    CORRADE_COMPARE(b.data(), DataPadded3D + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55*4, 11*3, 2*2}));
    CORRADE_VERIFY( b[0][0][0]);
    CORRADE_VERIFY( b[0][0][1]);
    CORRADE_VERIFY( b[0][0][2]);
    CORRADE_VERIFY(!b[0][1][0]);
    CORRADE_VERIFY(!b[0][1][1]);
    CORRADE_VERIFY(!b[0][1][2]);
}

void StridedBitArrayViewTest::every3DInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};

    StridedBitArrayView3D{}.every({3, 0, 4});
    CORRADE_COMPARE(out.str(), "Containers::StridedBitArrayView::every(): expected a non-zero step, got {3, 0, 4}\n");
}

void StridedBitArrayViewTest::every3DFirstDimension() {
    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    /* Data and offset stays the same, size and stride is adjusted in the first
       dimension */
    StridedBitArrayView3D b = a.every(2);
    CORRADE_COMPARE(b.data(), DataPadded3D + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{2, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55*2, 11, 2}));

    /* Both slices are the same */
    for(std::size_t i: {0, 1}) {
        CORRADE_VERIFY( b[i][2][0]);
        CORRADE_VERIFY(!b[i][2][1]);
        CORRADE_VERIFY( b[i][2][2]);
        CORRADE_VERIFY(!b[i][2][3]);
        CORRADE_VERIFY( b[i][2][4]);
    }
}

void StridedBitArrayViewTest::transposed() {
    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};
    CORRADE_VERIFY( a[1][1][0]);
    CORRADE_VERIFY( a[1][1][1]);
    CORRADE_VERIFY(!a[1][1][2]);
    CORRADE_VERIFY(!a[1][1][3]);
    CORRADE_VERIFY( a[1][1][4]);

    /* Data, offset and size stay the same */
    StridedBitArrayView3D b = a.transposed<1, 2>();
    CORRADE_COMPARE(b.data(), DataPadded3D + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{3, 5, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55, 2, 11}));
    CORRADE_VERIFY( b[1][0][1]);
    CORRADE_VERIFY( b[1][1][1]);
    CORRADE_VERIFY(!b[1][2][1]);
    CORRADE_VERIFY(!b[1][3][1]);
    CORRADE_VERIFY( b[1][4][1]);
}

void StridedBitArrayViewTest::transposedToSelf() {
    char data[5]{};
    StridedBitArrayView3D a{{data + 1, 3, 24}, {2, 3, 4}};

    /* Should be a no-op */
    StridedBitArrayView3D b = a.transposed<1, 1>();
    CORRADE_COMPARE(b.data(), data + 1);
    CORRADE_COMPARE(b.offset(), 3);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{12, 4, 1}));
}

void StridedBitArrayViewTest::flipped() {
    /* Same input as in access() */
    const StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 8, 3};
    CORRADE_VERIFY( a[0]);
    CORRADE_VERIFY( a[1]);
    CORRADE_VERIFY(!a[2]);
    CORRADE_VERIFY( a[3]);
    CORRADE_VERIFY( a[4]);
    CORRADE_VERIFY(!a[5]);
    CORRADE_VERIFY( a[6]);
    CORRADE_VERIFY(!a[7]);

    /* Size stays the same; data, offset gets recalculated and stride negated */
    StridedBitArrayView1D b = a.flipped<0>();
    CORRADE_COMPARE(b.data(), DataPadded + 4);
    CORRADE_COMPARE(b.offset(), 2);
    CORRADE_COMPARE(b.size(), 8);
    CORRADE_COMPARE(b.stride(), -3);
    CORRADE_VERIFY( b[7]);
    CORRADE_VERIFY( b[6]);
    CORRADE_VERIFY(!b[5]);
    CORRADE_VERIFY( b[4]);
    CORRADE_VERIFY( b[3]);
    CORRADE_VERIFY(!b[2]);
    CORRADE_VERIFY( b[1]);
    CORRADE_VERIFY(!b[0]);

    /* Flipping twice results in the same thing */
    CORRADE_VERIFY(a.flipped<0>().flipped<0>().data() == a.data());
    CORRADE_VERIFY(a.flipped<0>().flipped<0>().offset() == a.offset());
    CORRADE_VERIFY(a.flipped<0>().flipped<0>().stride() == a.stride());
}

void StridedBitArrayViewTest::flippedZeroSize() {
    /* Same as flipped() above, except that the size is 0 */
    StridedBitArrayView1D a{BitArrayView{DataPadded + 1, 5, 24}, 0, 3};

    /* Should not result in any difference in data or offset -- especially not
       any overflowing values */
    StridedBitArrayView1D b = a.flipped<0>();
    CORRADE_COMPARE(b.data(), DataPadded + 1);
    CORRADE_COMPARE(b.offset(), 5);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(b.stride(), -3);
}

void StridedBitArrayViewTest::flipped3D() {
    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};
    CORRADE_VERIFY( a[1][1][0]);
    CORRADE_VERIFY( a[1][1][1]);
    CORRADE_VERIFY(!a[1][1][2]);
    CORRADE_VERIFY(!a[1][1][3]);
    CORRADE_VERIFY( a[1][1][4]);

    /* Size stays the same; data, offset gets recalculated, stride negated in
       given dimension */
    StridedBitArrayView3D b = a.flipped<2>();
    CORRADE_COMPARE(b.data(), DataPadded3D + 2);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{3, 4, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55, 11, -2}));
    CORRADE_VERIFY( b[1][1][4]);
    CORRADE_VERIFY( b[1][1][3]);
    CORRADE_VERIFY(!b[1][1][2]);
    CORRADE_VERIFY(!b[1][1][1]);
    CORRADE_VERIFY( b[1][1][0]);
}

void StridedBitArrayViewTest::flipped3DZeroSize() {
    /* Same as flipped3D() above, except that second dimension size is 0 */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 0, 5}, {55, 11, 2}};

    /* Should not result in any difference in data or offset -- especially not
       any overflowing values */
    StridedBitArrayView3D b = a.flipped<1>();
    CORRADE_COMPARE(b.data(), DataPadded3D + 1);
    CORRADE_COMPARE(b.offset(), 7);
    CORRADE_COMPARE(b.size(), (Size3D{3, 0, 5}));
    CORRADE_COMPARE(b.stride(), (Stride3D{55, -11, 2}));
}

void StridedBitArrayViewTest::broadcasted() {
    /* Picks the initial bit in access(), thus all values are one. Data and
       offset stay the same, size and stride is different. */
    const StridedBitArrayView1D a = StridedBitArrayView1D{BitArrayView{DataPadded + 1, 5, 24}, 1, 3}.broadcasted<0>(100);
    CORRADE_COMPARE(a.data(), DataPadded + 1);
    CORRADE_COMPARE(a.offset(), 5);
    CORRADE_COMPARE(a.size(), 100);
    CORRADE_COMPARE(a.stride(), 0);

    /* Picks one bit before the initial in access(), thus all values are zero */
    const StridedBitArrayView1D b = StridedBitArrayView1D{BitArrayView{DataPadded + 1, 4, 24}, 1, 3}.broadcasted<0>(100);
    CORRADE_COMPARE(b.data(), DataPadded + 1);
    CORRADE_COMPARE(b.offset(), 4);
    CORRADE_COMPARE(b.size(), 100);
    CORRADE_COMPARE(b.stride(), 0);

    for(std::size_t i = 0; i != 100; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_VERIFY(a[i]);
        CORRADE_VERIFY(!b[i]);
    }
}

void StridedBitArrayViewTest::broadcasted3D() {
    /* Takes only the first row in every slice from access3D(). Data and offset
       stay the same, size and stride are different. */
    const StridedBitArrayView3D a = StridedBitArrayView3D{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 1, 5}, {55, 11, 2}}.broadcasted<1>(100);
    CORRADE_COMPARE(a.data(), DataPadded3D + 1);
    CORRADE_COMPARE(a.offset(), 7);
    CORRADE_COMPARE(a.size(), (Size3D{3, 100, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{55, 0, 2}));

    /* The first row is either all ones or all zeros */
    for(std::size_t i = 0; i != 100; ++i) {
        CORRADE_ITERATION(i);
        for(std::size_t j = 0; j != 5; ++j) {
            CORRADE_ITERATION(j);
            CORRADE_VERIFY( a[0][i][j]);
            CORRADE_VERIFY(!a[1][i][j]);
            CORRADE_VERIFY( a[2][i][j]);
        }
    }
}

void StridedBitArrayViewTest::broadcastedInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    /* Same input as in access3D() */
    StridedBitArrayView3D a{BitArrayView{DataPadded3D + 1, 7, 23*8}, {3, 4, 5}, {55, 11, 2}};

    std::ostringstream out;
    Error redirectError{&out};
    a.broadcasted<2>(16);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedBitArrayView::broadcasted(): can't broadcast dimension 2 with 5 elements\n");
}

void StridedBitArrayViewTest::debug() {
    /* 0b0101'0101'0011'0011'0000'1111 << 5
          1 1  1 1  0 1  0 1  0 0  1 1 */
    char data[]{'\xe0', '\x61', '\xa6', '\x0a'};

    std::ostringstream out;
    /* Testing also the BitArrayView to check for potential ambiguous overloads
       due to it being convertible to StridedBitArrayView */
    Debug{&out} << BitArrayView{DataPadded + 1, 5, 24};
    Debug{&out} << MutableBitArrayView{data, 5, 24};
    /* Compared to the usual stride of 3 bits this has 2 to test also correct
       bit group separation */
    Debug{&out} << StridedBitArrayView1D{BitArrayView{DataPadded + 1, 5, 24}, 12, 2};
    Debug{&out} << MutableStridedBitArrayView1D{MutableBitArrayView{data, 5, 24}, 12, 2};
    Debug{&out} << StridedBitArrayView1D{BitArrayView{DataPadded + 1, 5, 24}, 9, 2};
    CORRADE_COMPARE(out.str(),
        "{11110000, 11001100, 10101010}\n"
        "{11110000, 11001100, 10101010}\n"
        "{11001010, 1111}\n"
        "{11001010, 1111}\n"
        "{11001010, 1}\n");
}

void StridedBitArrayViewTest::debug3D() {
    /* See DataPadded3D for details */
    char data[]{
        '\x80', '\xff', '\x00', '\x60', '\x26', '\x00', '\x00', '\x00'| /* shared */
        '\x00', '\x00', '\x1e', '\x02', '\x80', '\xaa', '\x00', '\x00'| /* shared */
        '\xe0', '\x3f', '\x00', '\x98', '\x09', '\x00', '\x00', '\x00'
    };

    std::ostringstream out;
    /* Compared to the usual four rows this has only two to avoid overly
       verbose output. Bit group separation tested in the 1D case above
       already. */
    Debug{&out} << StridedBitArrayView3D{BitArrayView{DataPadded3D + 1, 7, 165}, {3, 2, 5}, {55, 11, 2}};
    Debug{&out} << MutableStridedBitArrayView3D{MutableBitArrayView{data, 7, 165}, {3, 2, 5}, {55, 11, 2}};
    CORRADE_COMPARE(out.str(),
        "{{{11111}, {00000}}, {{00000}, {11001}}, {{11111}, {00000}}}\n"
        "{{{11111}, {00000}}, {{00000}, {11001}}, {{11111}, {00000}}}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedBitArrayViewTest)
