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

/* Included as first to check that we *really* don't need the
   StridedBitArrayView header for anything -- the sliceBit() API shouldn't
   cause a compilation failure without it included. */
#include "Corrade/Containers/StridedArrayView.h"

#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"

namespace {

struct IntView {
    IntView(int* data, std::size_t size): data{data}, size{size} {}

    int* data;
    std::size_t size;
};

struct ConstIntView {
    constexpr ConstIntView(const int* data, std::size_t size): data{data}, size{size} {}

    const int* data;
    std::size_t size;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct ArrayViewConverter<int, IntView> {
    static ArrayView<int> from(IntView other) {
        return {other.data, other.size};
    }
};

template<> struct ArrayViewConverter<const int, ConstIntView> {
    constexpr static ArrayView<const int> from(ConstIntView other) {
        return {other.data, other.size};
    }
};

template<> struct ErasedArrayViewConverter<IntView>: ArrayViewConverter<int, IntView> {};
template<> struct ErasedArrayViewConverter<const IntView>: ArrayViewConverter<int, IntView> {};

template<> struct ErasedArrayViewConverter<ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};
template<> struct ErasedArrayViewConverter<const ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};

/* To keep the (Strided)ArrayView API in reasonable bounds, the const-adding
   variants have to be implemented explicitly */
template<> struct ArrayViewConverter<const int, IntView> {
    static ArrayView<const int> from(IntView other) {
        return {other.data, other.size};
    }
};

}

namespace Test { namespace {

struct StridedArrayViewTest: TestSuite::Tester {
    explicit StridedArrayViewTest();

    void constructDefault();
    void constructDefaultVoid();
    void constructDefaultConstVoid();
    void construct();
    void constructVoid();
    void constructConstVoid();
    void constructVoidFrom();
    void constructConstVoidFrom();
    void constructArray();
    void constructNullptrSize();
    void constructZeroStride();
    void constructNegativeStride();

    void constructSizeStride();
    void constructSizeStrideVoid();
    void constructSizeStrideConstVoid();
    void constructSizeStrideArray();

    void constructPointerSize();
    void constructPointerSizeVoid();
    void constructPointerSizeConstVoid();
    void constructFixedSize();
    void constructFixedSizeVoid();
    void constructFixedSizeConstVoid();

    void constructViewTooSmall();
    void constructViewTooSmallVoid();
    void constructViewTooSmallConstVoid();

    void constructFromView();
    void constructFromViewVoid();
    void constructFromViewConstVoid();
    void constructFromStaticView();
    void constructFromStaticViewVoid();
    void constructFromStaticViewConstVoid();
    void constructDerived();
    void constructCopy();
    void constructInitializerList();

    void construct3DDefault();
    void construct3DDefaultVoid();
    void construct3DDefaultConstVoid();
    void construct3D();
    void construct3DVoid();
    void construct3DConstVoid();
    void construct3DVoidFrom();
    void construct3DConstVoidFrom();
    void construct3DNullptrSize();
    void construct3DZeroStride();
    void construct3DNegativeStride();

    void construct3DPackedSizeStride();
    void construct3DPackedSizeStrideVoid();
    void construct3DPackedSizeStrideConstVoid();
    void construct3DPackedSizeOnly();
    /* size-only constructor not provided for void overloads as there's little
       chance one would want an implicit stride of 1 */

    void construct3DOneSizeZero();
    void construct3DOneSizeZeroVoid();
    void construct3DOneSizeZeroConstVoid();

    void construct3DFixedSize();
    void construct3DFixedSizeVoid();
    void construct3DFixedSizeConstVoid();

    void construct3DViewTooSmall();
    void construct3DViewTooSmallVoid();
    void construct3DViewTooSmallConstVoid();

    void construct3DFromView();
    void construct3DFromViewVoid();
    void construct3DFromViewConstVoid();
    void construct3DFromStaticView();
    void construct3DFromStaticViewVoid();
    void construct3DFromStaticViewConstVoid();
    void construct3DDerived();
    void construct3DFrom2DWithArrayType();
    void construct3DFromLessDimensions();

    void constructZeroNullPointerAmbiguity();
    void constructZeroNullPointerAmbiguityVoid();
    void constructZeroNullPointerAmbiguityConstVoid();

    void convertBool();
    void convertConst();
    void convertFromExternalView();
    void convertConstFromExternalView();
    void convertVoidFromExternalView();
    void convertConstVoidFromExternalView();
    void convertConstVoidFromConstExternalView();

    void convert3DBool();
    void convert3DConst();
    void convert3DFromExternalView();
    void convert3DConstFromExternalView();
    void convert3DVoidFromExternalView();
    void convert3DConstVoidFromExternalView();
    void convert3DConstVoidFromConstExternalView();

    void asContiguous();
    void asContiguousNonContiguous();

    void access();
    void accessConst();
    void accessZeroStride();
    void accessNegativeStride();
    void accessInvalid();

    void access3D();
    void access3DConst();
    void access3DZeroStride();
    void access3DNegativeStride();
    void access3DInvalid();

    void iterator();
    void iterator3D();

    void rangeBasedFor();
    void rangeBasedFor3D();
    void rangeBasedForZeroStride();
    void rangeBasedForZeroStride3D();
    void rangeBasedForNegativeStride();
    void rangeBasedForNegativeStride3D();

    void slice();
    void sliceInvalid();
    void slice3D();
    void slice3DInvalid();
    void slice3DFirstDimension();
    void slice3DFirstDimensionInvalid();

    void sliceDimensionUp();
    void sliceDimensionUpInvalid();
    void sliceDimensionDown();
    void sliceDimensionDownInvalid();

    void sliceMemberPointer();
    void sliceMemberPointerConstData();
    void sliceConstMemberPointer();
    void sliceMemberPointerDerived();
    void sliceMemberPointerNullView();
    void sliceMemberPointerEmptyView();

    void sliceMemberFunctionPointer();
    void sliceMemberFunctionPointerConstData();
    void sliceMemberFunctionPointerReturningConst();
    void sliceConstOverloadedMemberFunctionPointer();
    void sliceRvalueOverloadedMemberFunctionPointer();
    void sliceMemberFunctionPointerDerived();
    void sliceMemberFunctionPointerNullView();
    void sliceMemberFunctionPointerEmptyView();
    void sliceMemberFunctionPointerArrayType();
    void sliceMemberFunctionPointerReturningOffsetOutOfRange();

    void sliceBit();
    void sliceBitIndexTooLarge();
    void sliceBitSizeTooLarge();

    void every();
    void everyNegative();
    void everyNegativeZeroSize();
    void everyInvalid();
    void every2D();
    void every2DNegative();
    void every2DNegativeZeroSize();
    void every2DInvalid();
    void every2DFirstDimension();

    void transposed();
    void transposedToSelf();
    void flipped();
    void flippedZeroSize();
    void flipped3D();
    void flipped3DZeroSize();
    void broadcasted();
    void broadcasted3D();
    void broadcastedInvalid();
    void expandedCollapsed();
    void expandedCollapsedZeroStride();
    void expandedCollapsedNegativeStride();
    void expandedCollapsedInvalid();

    void cast();
    void castZeroStride();
    void castNegativeStride();
    void castInvalid();

    void castInflateFlatten();
    void castInflateFlattenZeroStride();
    void castInflateFlattenNegativeStride();
    void castInflateFlattenArrayView();
    void castInflateFlattenInvalid();

    void castInflateVoid();
    void castInflateVoidZeroStride();
    void castInflateVoidNegativeStride();
    void castInflateVoidInvalid();

    void size();
    void size3D();
};

typedef StridedArrayView1D<int> StridedArrayView1Di;
typedef StridedArrayView1D<const int> ConstStridedArrayView1Di;
typedef StridedArrayView2D<int> StridedArrayView2Di;
typedef StridedArrayView3D<int> StridedArrayView3Di;
typedef StridedArrayView4D<int> StridedArrayView4Di;
typedef StridedArrayView3D<const int> ConstStridedArrayView3Di;
typedef StridedArrayView1D<void> VoidStridedArrayView1D;
typedef StridedArrayView1D<const void> ConstVoidStridedArrayView1D;
typedef StridedArrayView3D<void> VoidStridedArrayView3D;
typedef StridedArrayView3D<const void> ConstVoidStridedArrayView3D;

constexpr struct {
    const char* name;
    bool flipped;
    Stride1D stride1;
    int dataBegin1, dataEnd1, dataBeginIncrement1, dataEndDecrement1;
    Stride3D stride3;
    int dataBegin3, dataEnd3, dataBeginIncrement3, dataEndDecrement3;
} IteratorData[]{
    {"", false, 8, 2, 5, 1, 6,
        {48, 24, 8}, 9, 10, 10, 11},
    {"zero stride", false, 0, 443, 443, 443, 443,
        {48, 0, 8}, 6, 7, 7, 8},
    {"flipped", true, 8, 4, 1, 5, 443,
        {48, 24, 8}, 11, 10, 10, 9}
};

StridedArrayViewTest::StridedArrayViewTest() {
    addTests({&StridedArrayViewTest::constructDefault,
              &StridedArrayViewTest::constructDefaultVoid,
              &StridedArrayViewTest::constructDefaultConstVoid,
              &StridedArrayViewTest::construct,
              &StridedArrayViewTest::constructVoid,
              &StridedArrayViewTest::constructConstVoid,
              &StridedArrayViewTest::constructVoidFrom,
              &StridedArrayViewTest::constructConstVoidFrom,
              &StridedArrayViewTest::constructArray,
              &StridedArrayViewTest::constructNullptrSize,
              &StridedArrayViewTest::constructZeroStride,
              &StridedArrayViewTest::constructNegativeStride,

              &StridedArrayViewTest::constructSizeStride,
              &StridedArrayViewTest::constructSizeStrideVoid,
              &StridedArrayViewTest::constructSizeStrideConstVoid,
              &StridedArrayViewTest::constructSizeStrideArray,

              &StridedArrayViewTest::constructPointerSize,
              &StridedArrayViewTest::constructPointerSizeVoid,
              &StridedArrayViewTest::constructPointerSizeConstVoid,
              &StridedArrayViewTest::constructFixedSize,
              &StridedArrayViewTest::constructFixedSizeVoid,
              &StridedArrayViewTest::constructFixedSizeConstVoid,

              &StridedArrayViewTest::constructViewTooSmall,
              &StridedArrayViewTest::constructViewTooSmallVoid,
              &StridedArrayViewTest::constructViewTooSmallConstVoid,

              &StridedArrayViewTest::constructFromView,
              &StridedArrayViewTest::constructFromViewVoid,
              &StridedArrayViewTest::constructFromViewConstVoid,
              &StridedArrayViewTest::constructFromStaticView,
              &StridedArrayViewTest::constructFromStaticViewVoid,
              &StridedArrayViewTest::constructFromStaticViewConstVoid,
              &StridedArrayViewTest::constructDerived,
              &StridedArrayViewTest::constructCopy,
              &StridedArrayViewTest::constructInitializerList,

              &StridedArrayViewTest::construct3DDefault,
              &StridedArrayViewTest::construct3DDefaultVoid,
              &StridedArrayViewTest::construct3DDefaultConstVoid,
              &StridedArrayViewTest::construct3D,
              &StridedArrayViewTest::construct3DVoid,
              &StridedArrayViewTest::construct3DConstVoid,
              &StridedArrayViewTest::construct3DVoidFrom,
              &StridedArrayViewTest::construct3DConstVoidFrom,
              &StridedArrayViewTest::construct3DNullptrSize,
              &StridedArrayViewTest::construct3DZeroStride,
              &StridedArrayViewTest::construct3DNegativeStride,

              &StridedArrayViewTest::construct3DPackedSizeStride,
              &StridedArrayViewTest::construct3DPackedSizeStrideVoid,
              &StridedArrayViewTest::construct3DPackedSizeStrideConstVoid,
              &StridedArrayViewTest::construct3DPackedSizeOnly,

              &StridedArrayViewTest::construct3DOneSizeZero,
              &StridedArrayViewTest::construct3DOneSizeZeroVoid,
              &StridedArrayViewTest::construct3DOneSizeZeroConstVoid,

              &StridedArrayViewTest::construct3DFixedSize,
              &StridedArrayViewTest::construct3DFixedSizeVoid,
              &StridedArrayViewTest::construct3DFixedSizeConstVoid,

              &StridedArrayViewTest::construct3DViewTooSmall,
              &StridedArrayViewTest::construct3DViewTooSmallVoid,
              &StridedArrayViewTest::construct3DViewTooSmallConstVoid,

              &StridedArrayViewTest::construct3DFromView,
              &StridedArrayViewTest::construct3DFromViewVoid,
              &StridedArrayViewTest::construct3DFromViewConstVoid,
              &StridedArrayViewTest::construct3DFromStaticView,
              &StridedArrayViewTest::construct3DFromStaticViewVoid,
              &StridedArrayViewTest::construct3DFromStaticViewConstVoid,
              &StridedArrayViewTest::construct3DDerived,
              &StridedArrayViewTest::construct3DFrom2DWithArrayType,
              &StridedArrayViewTest::construct3DFromLessDimensions,

              &StridedArrayViewTest::constructZeroNullPointerAmbiguity,
              &StridedArrayViewTest::constructZeroNullPointerAmbiguityVoid,
              &StridedArrayViewTest::constructZeroNullPointerAmbiguityConstVoid,

              &StridedArrayViewTest::convertBool,
              &StridedArrayViewTest::convertConst,
              &StridedArrayViewTest::convertFromExternalView,
              &StridedArrayViewTest::convertConstFromExternalView,
              &StridedArrayViewTest::convertVoidFromExternalView,
              &StridedArrayViewTest::convertConstVoidFromExternalView,
              &StridedArrayViewTest::convertConstVoidFromConstExternalView,

              &StridedArrayViewTest::convert3DBool,
              &StridedArrayViewTest::convert3DConst,
              &StridedArrayViewTest::convert3DFromExternalView,
              &StridedArrayViewTest::convert3DConstFromExternalView,
              &StridedArrayViewTest::convert3DVoidFromExternalView,
              &StridedArrayViewTest::convert3DConstVoidFromExternalView,
              &StridedArrayViewTest::convert3DConstVoidFromConstExternalView,

              &StridedArrayViewTest::asContiguous,
              &StridedArrayViewTest::asContiguousNonContiguous,

              &StridedArrayViewTest::access,
              &StridedArrayViewTest::accessConst,
              &StridedArrayViewTest::accessZeroStride,
              &StridedArrayViewTest::accessNegativeStride,
              &StridedArrayViewTest::accessInvalid,

              &StridedArrayViewTest::access3D,
              &StridedArrayViewTest::access3DConst,
              &StridedArrayViewTest::access3DZeroStride,
              &StridedArrayViewTest::access3DNegativeStride,
              &StridedArrayViewTest::access3DInvalid});

    addInstancedTests({
        &StridedArrayViewTest::iterator,
        &StridedArrayViewTest::iterator3D},
        Containers::arraySize(IteratorData));

    addTests({&StridedArrayViewTest::rangeBasedFor,
              &StridedArrayViewTest::rangeBasedFor3D,
              &StridedArrayViewTest::rangeBasedForZeroStride,
              &StridedArrayViewTest::rangeBasedForZeroStride3D,
              &StridedArrayViewTest::rangeBasedForNegativeStride,
              &StridedArrayViewTest::rangeBasedForNegativeStride3D,

              &StridedArrayViewTest::slice,
              &StridedArrayViewTest::sliceInvalid,
              &StridedArrayViewTest::slice3D,
              &StridedArrayViewTest::slice3DInvalid,
              &StridedArrayViewTest::slice3DFirstDimension,
              &StridedArrayViewTest::slice3DFirstDimensionInvalid,

              &StridedArrayViewTest::sliceDimensionUp,
              &StridedArrayViewTest::sliceDimensionUpInvalid,
              &StridedArrayViewTest::sliceDimensionDown,
              &StridedArrayViewTest::sliceDimensionDownInvalid,

              &StridedArrayViewTest::sliceMemberPointer,
              &StridedArrayViewTest::sliceMemberPointerConstData,
              &StridedArrayViewTest::sliceConstMemberPointer,
              &StridedArrayViewTest::sliceMemberPointerDerived,
              &StridedArrayViewTest::sliceMemberPointerNullView,
              &StridedArrayViewTest::sliceMemberPointerEmptyView,

              &StridedArrayViewTest::sliceMemberFunctionPointer,
              &StridedArrayViewTest::sliceMemberFunctionPointerConstData,
              &StridedArrayViewTest::sliceMemberFunctionPointerReturningConst,
              &StridedArrayViewTest::sliceConstOverloadedMemberFunctionPointer,
              &StridedArrayViewTest::sliceRvalueOverloadedMemberFunctionPointer,
              &StridedArrayViewTest::sliceMemberFunctionPointerDerived,
              &StridedArrayViewTest::sliceMemberFunctionPointerNullView,
              &StridedArrayViewTest::sliceMemberFunctionPointerEmptyView,
              &StridedArrayViewTest::sliceMemberFunctionPointerArrayType,
              &StridedArrayViewTest::sliceMemberFunctionPointerReturningOffsetOutOfRange,

              &StridedArrayViewTest::sliceBit,
              &StridedArrayViewTest::sliceBitIndexTooLarge,
              &StridedArrayViewTest::sliceBitSizeTooLarge,

              &StridedArrayViewTest::every,
              &StridedArrayViewTest::everyNegative,
              &StridedArrayViewTest::everyNegativeZeroSize,
              &StridedArrayViewTest::everyInvalid,
              &StridedArrayViewTest::every2D,
              &StridedArrayViewTest::every2DNegative,
              &StridedArrayViewTest::every2DNegativeZeroSize,
              &StridedArrayViewTest::every2DInvalid,
              &StridedArrayViewTest::every2DFirstDimension,

              &StridedArrayViewTest::transposed,
              &StridedArrayViewTest::transposedToSelf,
              &StridedArrayViewTest::flipped,
              &StridedArrayViewTest::flippedZeroSize,
              &StridedArrayViewTest::flipped3D,
              &StridedArrayViewTest::flipped3DZeroSize,
              &StridedArrayViewTest::broadcasted,
              &StridedArrayViewTest::broadcasted3D,
              &StridedArrayViewTest::broadcastedInvalid,
              &StridedArrayViewTest::expandedCollapsed,
              &StridedArrayViewTest::expandedCollapsedZeroStride,
              &StridedArrayViewTest::expandedCollapsedNegativeStride,
              &StridedArrayViewTest::expandedCollapsedInvalid,

              &StridedArrayViewTest::cast,
              &StridedArrayViewTest::castZeroStride,
              &StridedArrayViewTest::castNegativeStride,
              &StridedArrayViewTest::castInvalid,

              &StridedArrayViewTest::castInflateFlatten,
              &StridedArrayViewTest::castInflateFlattenZeroStride,
              &StridedArrayViewTest::castInflateFlattenNegativeStride,
              &StridedArrayViewTest::castInflateFlattenArrayView,
              &StridedArrayViewTest::castInflateFlattenInvalid,

              &StridedArrayViewTest::castInflateVoid,
              &StridedArrayViewTest::castInflateVoidZeroStride,
              &StridedArrayViewTest::castInflateVoidNegativeStride,
              &StridedArrayViewTest::castInflateVoidInvalid,

              &StridedArrayViewTest::size,
              &StridedArrayViewTest::size3D});
}

void StridedArrayViewTest::constructDefault() {
    StridedArrayView1Di a;
    StridedArrayView1Di b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);
    CORRADE_COMPARE(b.stride(), 0);

    constexpr StridedArrayView1Di ca;
    constexpr StridedArrayView1Di cb = nullptr;
    constexpr void* dataA = ca.data();
    constexpr void* dataB = cb.data();
    constexpr bool emptyA = ca.isEmpty();
    constexpr bool emptyB = cb.isEmpty();
    constexpr std::size_t sizeA = ca.size();
    constexpr std::size_t sizeB = cb.size();
    constexpr std::ptrdiff_t strideA = ca.stride();
    constexpr std::ptrdiff_t strideB = cb.stride();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);
    CORRADE_COMPARE(strideA, 0);
    CORRADE_COMPARE(strideB, 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<StridedArrayView1Di>::value);
}

void StridedArrayViewTest::constructDefaultVoid() {
    VoidStridedArrayView1D a;
    VoidStridedArrayView1D b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);
    CORRADE_COMPARE(b.stride(), 0);

    constexpr VoidStridedArrayView1D ca;
    constexpr VoidStridedArrayView1D cb = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(cb.data() == nullptr);
    CORRADE_VERIFY(ca.isEmpty());
    CORRADE_VERIFY(cb.isEmpty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(cb.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
    CORRADE_COMPARE(cb.stride(), 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<VoidStridedArrayView1D>::value);
}

void StridedArrayViewTest::constructDefaultConstVoid() {
    ConstVoidStridedArrayView1D a;
    ConstVoidStridedArrayView1D b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);
    CORRADE_COMPARE(b.stride(), 0);

    constexpr ConstVoidStridedArrayView1D ca;
    constexpr ConstVoidStridedArrayView1D cb = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(cb.data() == nullptr);
    CORRADE_VERIFY(ca.isEmpty());
    CORRADE_VERIFY(cb.isEmpty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(cb.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
    CORRADE_COMPARE(cb.stride(), 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<ConstVoidStridedArrayView1D>::value);
}

/* Needs to be here in order to use it in constexpr */
constexpr const struct {
    int value;
    int other;
} Struct[10]{
    {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
    /* Otherwise GCC 4.8 loudly complains about missing initializers */
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
};

void StridedArrayViewTest::construct() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    {
        StridedArrayView1Di b = {a, &a[0].value, 10, 8};
        CORRADE_VERIFY(b.data() == a);
        CORRADE_VERIFY(!b.isEmpty());
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 8);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);

        auto c = stridedArrayView(a, &a[0].value, 10, 8);
        CORRADE_VERIFY(std::is_same<decltype(c), StridedArrayView1Di>::value);
        CORRADE_VERIFY(c.data() == a);
        CORRADE_VERIFY(!c.isEmpty());
        CORRADE_COMPARE(c.size(), 10);
        CORRADE_COMPARE(c.stride(), 8);
        CORRADE_COMPARE(c[2], 7853268);
        CORRADE_COMPARE(c[4], 234810);

        auto c2 = stridedArrayView(b);
        CORRADE_VERIFY(std::is_same<decltype(c2), StridedArrayView1Di>::value);
        CORRADE_VERIFY(c2.data() == a);
        CORRADE_VERIFY(!c2.isEmpty());
        CORRADE_COMPARE(c2.size(), 10);
        CORRADE_COMPARE(c2.stride(), 8);
        CORRADE_COMPARE(c2[2], 7853268);
        CORRADE_COMPARE(c2[4], 234810);
    }

    {
        constexpr ConstStridedArrayView1Di cb = {Struct, &Struct[0].value, 10, 8};
        CORRADE_VERIFY(cb.data() == Struct);
        CORRADE_VERIFY(!cb.isEmpty());
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 8);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);

        constexpr auto cc = stridedArrayView(Struct, &Struct[0].value, 10, 8);
        CORRADE_VERIFY(std::is_same<decltype(cc), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cc.data() == Struct);
        CORRADE_VERIFY(!cc.isEmpty());
        CORRADE_COMPARE(cc.size(), 10);
        CORRADE_COMPARE(cc.stride(), 8);
        CORRADE_COMPARE(cc[2], 7853268);
        CORRADE_COMPARE(cc[4], 234810);

        constexpr auto cc2 = stridedArrayView(cb);
        CORRADE_VERIFY(std::is_same<decltype(cc2), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cc2.data() == Struct);
        CORRADE_VERIFY(!cc2.isEmpty());
        CORRADE_COMPARE(cc2.size(), 10);
        CORRADE_COMPARE(cc2.stride(), 8);
        CORRADE_COMPARE(cc2[2], 7853268);
        CORRADE_COMPARE(cc2[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, ArrayView<int>, int*, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructVoid() {
    struct {
        int value;
        int other;
    } a[10]{};

    VoidStridedArrayView1D b = {a, &a[0].value, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, ArrayView<int>, int*, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructConstVoid() {
    const struct {
        int value;
        int other;
    } a[10]{};

    ConstVoidStridedArrayView1D b = {a, &a[0].value, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    constexpr ConstVoidStridedArrayView1D cb = {Struct, &Struct[0].value, 10, 8};
    CORRADE_VERIFY(cb.data() == Struct);
    CORRADE_VERIFY(!cb.isEmpty());
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 8);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, ArrayView<int>, int*, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructVoidFrom() {
    struct {
        int value;
        int other;
    } a[10]{};

    StridedArrayView1Di b = {a, &a[0].value, 10, 8};
    VoidStridedArrayView1D bv = b;
    CORRADE_VERIFY(bv.data() == a);
    CORRADE_COMPARE(bv.size(), 10);
    CORRADE_COMPARE(bv.stride(), 8);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, StridedArrayView1Di>::value);
}

void StridedArrayViewTest::constructConstVoidFrom() {
    struct {
        int value;
        int other;
    } a[10]{};

    StridedArrayView1Di b = {a, &a[0].value, 10, 8};
    ConstStridedArrayView1Di cb = {a, &a[0].value, 10, 8};
    ConstVoidStridedArrayView1D bv = b;
    ConstVoidStridedArrayView1D cbv = cb;
    CORRADE_VERIFY(bv.data() == a);
    CORRADE_VERIFY(cbv.data() == a);
    CORRADE_COMPARE(bv.size(), 10);
    CORRADE_COMPARE(cbv.size(), 10);
    CORRADE_COMPARE(bv.stride(), 8);
    CORRADE_COMPARE(cbv.stride(), 8);

    constexpr ConstStridedArrayView1Di ccb = {Struct, &Struct[0].value, 10, 8};
    constexpr ConstVoidStridedArrayView1D ccbv = ccb;
    CORRADE_VERIFY(ccbv.data() == Struct);
    CORRADE_COMPARE(ccbv.size(), 10);
    CORRADE_COMPARE(ccbv.stride(), 8);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, ConstStridedArrayView1Di>::value);
}

void StridedArrayViewTest::constructArray() {
    /* Compared to construct(), size and stride is wrapped in {}. Just to
       verify that this doesn't cause a compilation error, it isn't any special
       overload. */

    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView1Di b = {a, &a[0].value, {10}, {8}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr ConstStridedArrayView1Di cc = {Struct, &Struct[0].value, {10}, {8}};
    CORRADE_VERIFY(cc.data() == Struct);
    CORRADE_COMPARE(cc.size(), 10);
    CORRADE_COMPARE(cc.stride(), 8);
    CORRADE_COMPARE(cc[2], 7853268);
    CORRADE_COMPARE(cc[4], 234810);
}

void StridedArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later */

    StridedArrayView1Di a{{nullptr, 40}, nullptr, 5, 8};
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);

    constexpr StridedArrayView1Di ca{{nullptr, 40}, nullptr, 5, 8};
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(!ca.isEmpty());
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(ca.stride(), 8);
}

void StridedArrayViewTest::constructZeroStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in accessZeroStride(). */

    struct {
        int value;
        int other;
    } a[1]{{2, 23125}};

    StridedArrayView1Di b = {a, &a[0].other, 10, 0};
    CORRADE_VERIFY(b.data() == &a[0].other);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 0);
    CORRADE_COMPARE(b[2], 23125);
    CORRADE_COMPARE(b[4], 23125);
}

void StridedArrayViewTest::constructNegativeStride() {
    /* Just verify that this doesn't assert, correctness of the actual access
       APIs is verified in accessNegativeStride(). */

    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView1Di b = {a, &a[9].value, 10, -8};
    CORRADE_VERIFY(b.data() == &a[9].value);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), -8);
    CORRADE_COMPARE(b[9 - 2], 7853268); /* ID 2 if it wouldn't be negative */
    CORRADE_COMPARE(b[9 - 4], 234810); /* ID 4 if it wouldn't be negative */
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array[10*2]{
    2, 23125, 16, 1, 7853268, -2, -100, 5, 234810, 1,
    /* Otherwise GCC 4.8 loudly complains about missing initializers */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void StridedArrayViewTest::constructSizeStride() {
    /* Compared to construct() we don't pass the pointer parameter */

    int a[10*2]{
        2, 23125, 16, 1, 7853268, -2, -100, 5, 234810, 1,
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    {
        StridedArrayView1Di b = {a, 10, 8};
        CORRADE_VERIFY(b.data() == a);
        CORRADE_VERIFY(!b.isEmpty());
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 8);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);

        auto c = stridedArrayView(arrayView(a), 10, 8);
        CORRADE_VERIFY(std::is_same<decltype(c), StridedArrayView1Di>::value);
        CORRADE_VERIFY(c.data() == a);
        CORRADE_VERIFY(!c.isEmpty());
        CORRADE_COMPARE(c.size(), 10);
        CORRADE_COMPARE(c.stride(), 8);
        CORRADE_COMPARE(c[2], 7853268);
        CORRADE_COMPARE(c[4], 234810);
    }

    {
        constexpr ConstStridedArrayView1Di cb = {Array, 10, 8};
        CORRADE_VERIFY(cb.data() == Array);
        CORRADE_VERIFY(!cb.isEmpty());
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 8);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);

        constexpr auto cc = stridedArrayView(arrayView(Array), 10, 8);
        CORRADE_VERIFY(std::is_same<decltype(cc), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cc.data() == Array);
        CORRADE_VERIFY(!cc.isEmpty());
        CORRADE_COMPARE(cc.size(), 10);
        CORRADE_COMPARE(cc.stride(), 8);
        CORRADE_COMPARE(cc[2], 7853268);
        CORRADE_COMPARE(cc[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, ArrayView<int>, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructSizeStrideVoid() {
    /* Compared to constructVoid() we don't pass the pointer parameter */

    int a[10*2]{};

    VoidStridedArrayView1D b = {a, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, ArrayView<int>, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructSizeStrideConstVoid() {
    /* Compared to constructConstVoid() we don't pass the pointer parameter */

    const int a[10*2]{};

    ConstVoidStridedArrayView1D b = {a, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    constexpr ConstVoidStridedArrayView1D cb = {Array, 10, 8};
    CORRADE_VERIFY(cb.data() == Array);
    CORRADE_VERIFY(!cb.isEmpty());
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 8);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, ArrayView<int>, std::size_t, std::ptrdiff_t>::value);
}

void StridedArrayViewTest::constructSizeStrideArray() {
    /* Compared to constructSizeStride(), size and stride is wrapped in {}.
       Just to verify that this doesn't cause a compilation error, it isn't
       any special overload. */

    int a[10*2]{
        2, 23125, 16, 1, 7853268, -2, -100, 5, 234810, 1,
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    StridedArrayView1Di b = {a, {10}, {8}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr ConstStridedArrayView1Di cc = {Array, {10}, {8}};
    CORRADE_VERIFY(cc.data() == Array);
    CORRADE_COMPARE(cc.size(), 10);
    CORRADE_COMPARE(cc.stride(), 8);
    CORRADE_COMPARE(cc[2], 7853268);
    CORRADE_COMPARE(cc[4], 234810);
}

/* Needs to be here in order to use it in constexpr */
constexpr const int Array10[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

void StridedArrayViewTest::constructPointerSize() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

    {
        StridedArrayView1Di b = {a, 6};
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 6);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    } {
        auto b = stridedArrayView(a, 6);
        CORRADE_VERIFY(std::is_same<decltype(b), StridedArrayView1Di>::value);
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 6);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    }

    {
        constexpr ConstStridedArrayView1Di cb = {Array10, 6};
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 6);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    } {
        constexpr auto cb = stridedArrayView(Array10, 6);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 6);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, int*, std::size_t>::value);
}

void StridedArrayViewTest::constructPointerSizeVoid() {
    int a[10]{};

    VoidStridedArrayView1D b = {a, 6};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 6);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, int*, std::size_t>::value);
}

void StridedArrayViewTest::constructPointerSizeConstVoid() {
    const int a[10]{};

    ConstVoidStridedArrayView1D b = {a, 6};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 6);
    CORRADE_COMPARE(b.stride(), 4);

    constexpr ConstVoidStridedArrayView1D cb = {Array10, 6};
    CORRADE_VERIFY(cb.data() == Array10);
    CORRADE_COMPARE(cb.size(), 6);
    CORRADE_COMPARE(cb.stride(), 4);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, const int*, std::size_t>::value);
}

void StridedArrayViewTest::constructFixedSize() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

    {
        StridedArrayView1Di b = a;
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    } {
        auto b = stridedArrayView(a);
        CORRADE_VERIFY(std::is_same<decltype(b), StridedArrayView1Di>::value);
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    }

    {
        constexpr ConstStridedArrayView1Di cb = Array10;
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    } {
        constexpr auto cb = stridedArrayView(Array10);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, int(&)[3]>::value);
}

void StridedArrayViewTest::constructFixedSizeVoid() {
    int a[10]{};

    VoidStridedArrayView1D b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, int(&)[3]>::value);
}

void StridedArrayViewTest::constructFixedSizeConstVoid() {
    const int a[10]{};

    ConstVoidStridedArrayView1D b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    constexpr ConstVoidStridedArrayView1D cb = Array10;
    CORRADE_VERIFY(cb.data() == Array10);
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 4);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, const int[3]>::value);
}

void StridedArrayViewTest::constructViewTooSmall() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct {
        int value;
        int other;
    } a[10]{};

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView1Di{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

void StridedArrayViewTest::constructViewTooSmallVoid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct {
        int value;
        int other;
    } a[10]{};

    Containers::String out;
    Error redirectError{&out};

    VoidStridedArrayView1D{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

void StridedArrayViewTest::constructViewTooSmallConstVoid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const struct {
        int value;
        int other;
    } a[10]{};

    Containers::String out;
    Error redirectError{&out};

    ConstVoidStridedArrayView1D{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

void StridedArrayViewTest::constructFromView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    ArrayView<int> view = a;

    {
        StridedArrayView1Di b = view;
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    } {
        auto b = stridedArrayView(view);
        CORRADE_VERIFY(std::is_same<decltype(b), StridedArrayView1Di>::value);
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    }

    constexpr ArrayView<const int> cview = Array10;

    {
        constexpr ConstStridedArrayView1Di cb = cview;
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    } {
        constexpr auto cb = stridedArrayView(cview);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, ArrayView<int>>::value);
}

void StridedArrayViewTest::constructFromViewVoid() {
    int a[10]{};
    ArrayView<int> view = a;

    VoidStridedArrayView1D b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, ArrayView<int>>::value);
}

void StridedArrayViewTest::constructFromViewConstVoid() {
    const int a[10]{};
    ArrayView<const int> view = a;

    ConstVoidStridedArrayView1D b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    constexpr ArrayView<const int> cview = Array10;
    constexpr ConstVoidStridedArrayView1D cb = cview;
    CORRADE_VERIFY(cb.data() == Array10);
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 4);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, ArrayView<const int>>::value);
}

void StridedArrayViewTest::constructFromStaticView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    StaticArrayView<10, int> view = a;

    {
        StridedArrayView1Di b = view;
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    } {
        auto b = stridedArrayView(view);
        CORRADE_VERIFY(std::is_same<decltype(b), StridedArrayView1Di>::value);
        CORRADE_VERIFY(b.data() == a);
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 4);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);
    }

    constexpr StaticArrayView<10, const int> cview = Array10;

    {
        constexpr ConstStridedArrayView1Di cb = cview;
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    } {
        constexpr auto cb = stridedArrayView(cview);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value);
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView1Di, StaticArrayView<10, int>>::value);
}

void StridedArrayViewTest::constructFromStaticViewVoid() {
    int a[10];
    StaticArrayView<10, int> view = a;

    VoidStridedArrayView1D b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView1D, StaticArrayView<10, int>>::value);
}

void StridedArrayViewTest::constructFromStaticViewConstVoid() {
    int a[10]{};
    StaticArrayView<10, int> b = a;
    StaticArrayView<10, const int> cb = a;

    ConstVoidStridedArrayView1D c = b;
    ConstVoidStridedArrayView1D cc = cb;
    CORRADE_VERIFY(c.data() == a);
    CORRADE_VERIFY(cc.data() == a);
    CORRADE_COMPARE(c.size(), 10);
    CORRADE_COMPARE(cc.size(), 10);
    CORRADE_COMPARE(c.stride(), 4);
    CORRADE_COMPARE(cc.stride(), 4);

    constexpr StaticArrayView<10, const int> ccb = Array10;
    constexpr ConstVoidStridedArrayView1D ccc = ccb;
    CORRADE_VERIFY(ccc.data() == Array10);
    CORRADE_COMPARE(ccc.size(), 10);
    CORRADE_COMPARE(ccc.stride(), 4);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, StaticArrayView<10, int>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView1D, StaticArrayView<10, const int>>::value);
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(): i{} {}
    short i;
};
struct Derived: Base {
    constexpr Derived() {}
};
constexpr Derived DerivedArray[5]
    /* This missing makes MSVC2015 complain it's not constexpr, but if present
       then GCC 4.8 fails to build. Eh. ¯\_(ツ)_/¯ */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    {}
    #endif
    ;

void StridedArrayViewTest::constructDerived() {
    /* Valid use case: constructing Containers::StridedArrayView<Math::Vector<3, Float>>
       from Containers::StridedArrayView<Color3> because the data have the same size
       and data layout */

    Derived b[5];
    Containers::StridedArrayView1D<Derived> bv{b};
    Containers::StridedArrayView1D<Base> a{b};
    Containers::StridedArrayView1D<Base> av{bv};

    CORRADE_VERIFY(a.data() == &b[0]);
    CORRADE_VERIFY(av.data() == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 2);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(av.stride(), 2);

    constexpr Containers::StridedArrayView1D<const Derived> cbv{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StridedArrayView1D<const Base> ca{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StridedArrayView1D<const Base> cav{cbv};

    CORRADE_VERIFY(ca.data() == &DerivedArray[0]);
    CORRADE_VERIFY(cav.data() == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(ca.stride(), 2);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(cav.stride(), 2);

    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::StridedArrayView1D<Base>, Derived(&)[5]>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::StridedArrayView1D<Base>, Containers::StridedArrayView1D<Derived>>::value);
}

void StridedArrayViewTest::constructCopy() {
    int data[30];
    StridedArrayView1Di a{data, 15, 8};

    StridedArrayView1Di b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 15);
    CORRADE_COMPARE(b.stride(), 8);

    int data2[3];
    StridedArrayView1Di c{data2};
    c = b;
    CORRADE_COMPARE(c.data(), &data[0]);
    CORRADE_COMPARE(c.size(), 15);
    CORRADE_COMPARE(c.stride(), 8);

    CORRADE_VERIFY(std::is_copy_constructible<StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_copy_assignable<StridedArrayView1Di>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<StridedArrayView1Di>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<StridedArrayView1Di>::value);
}

void StridedArrayViewTest::constructInitializerList() {
    std::initializer_list<int> a = {3, 5, 7};
    ConstStridedArrayView1Di b = stridedArrayView(a);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b.back(), 7);

    /* R-value init list should work too */
    CORRADE_COMPARE(stridedArrayView<int>({3, 5, 7}).front(), 3);
}

void StridedArrayViewTest::construct3DDefault() {
    StridedArrayView3Di a;
    StridedArrayView3Di b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(b.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(b.stride(), (Stride3D{0, 0, 0}));

    constexpr StridedArrayView3Di ca;
    constexpr StridedArrayView3Di cb = nullptr;
    constexpr void* dataA = ca.data();
    constexpr void* dataB = cb.data();
    constexpr StridedDimensions<3, bool> emptyA = ca.isEmpty();
    constexpr StridedDimensions<3, bool> emptyB = cb.isEmpty();
    constexpr Size3D sizeA = ca.size();
    constexpr Size3D sizeB = cb.size();
    constexpr Stride3D strideA = ca.stride();
    constexpr Stride3D strideB = cb.stride();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_COMPARE(emptyA, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(emptyB, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(sizeA, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(sizeB, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(strideA, (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(strideB, (Stride3D{0, 0, 0}));

    CORRADE_VERIFY(std::is_nothrow_default_constructible<StridedArrayView3Di>::value);
}

void StridedArrayViewTest::construct3DDefaultVoid() {
    VoidStridedArrayView3D a;
    VoidStridedArrayView3D b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(b.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(b.stride(), (Stride3D{0, 0, 0}));

    constexpr VoidStridedArrayView3D ca;
    constexpr VoidStridedArrayView3D cb = nullptr;
    constexpr void* dataB = cb.data();
    constexpr void* dataA = ca.data();
    constexpr StridedDimensions<3, bool> emptyA = ca.isEmpty();
    constexpr StridedDimensions<3, bool> emptyB = cb.isEmpty();
    constexpr Size3D sizeA = ca.size();
    constexpr Size3D sizeB = cb.size();
    constexpr Stride3D strideA = ca.stride();
    constexpr Stride3D strideB = cb.stride();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_COMPARE(emptyA, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(emptyB, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(sizeA, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(sizeB, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(strideA, (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(strideB, (Stride3D{0, 0, 0}));

    CORRADE_VERIFY(std::is_nothrow_default_constructible<VoidStridedArrayView3D>::value);
}

void StridedArrayViewTest::construct3DDefaultConstVoid() {
    ConstVoidStridedArrayView3D a;
    ConstVoidStridedArrayView3D b = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(b.data() == nullptr);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(b.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(b.stride(), (Stride3D{0, 0, 0}));

    constexpr ConstVoidStridedArrayView3D ca;
    constexpr ConstVoidStridedArrayView3D cb = nullptr;
    constexpr const void* dataA = ca.data();
    constexpr const void* dataB = cb.data();
    constexpr StridedDimensions<3, bool> emptyA = ca.isEmpty();
    constexpr StridedDimensions<3, bool> emptyB = cb.isEmpty();
    constexpr Size3D sizeA = ca.size();
    constexpr Size3D sizeB = cb.size();
    constexpr Stride3D strideA = ca.stride();
    constexpr Stride3D strideB = cb.stride();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_COMPARE(emptyA, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(emptyB, (StridedDimensions<3, bool>{true, true, true}));
    CORRADE_COMPARE(sizeA, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(sizeB, (Size3D{0, 0, 0}));
    CORRADE_COMPARE(strideA, (Stride3D{0, 0, 0}));
    CORRADE_COMPARE(strideB, (Stride3D{0, 0, 0}));

    CORRADE_VERIFY(std::is_nothrow_default_constructible<ConstVoidStridedArrayView1D>::value);
}

/* Needs to be here in order to use it in constexpr */
constexpr const struct Plane {
    struct Row {
        struct Item {
            int value;
            int other;
        } row[3];
    } plane[2];
} Cube[2]{
    {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
     {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
    {{{{{0, 0}, {0, 0}, {0, 0}}},
     {{{0, 0}, {0, 0}, {0, 0}}}}}
};

void StridedArrayViewTest::construct3D() {
    Plane a[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b[0][0][0], 2);
    CORRADE_COMPARE(b[0][0][1], 16);
    CORRADE_COMPARE(b[0][0][2], 7853268);
    CORRADE_COMPARE(b[0][1][1], 234810);

    constexpr ConstStridedArrayView3Di cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(cb.data() == Cube);
    CORRADE_COMPARE(cb.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(cb[0][0][0], 2);
    CORRADE_COMPARE(cb[0][0][1], 16);
    CORRADE_COMPARE(cb[0][0][2], 7853268);
    CORRADE_COMPARE(cb[0][1][1], 234810);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView3Di, ArrayView<int>, int*, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DVoid() {
    Plane a[2]{};

    VoidStridedArrayView3D b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView3D, ArrayView<int>, int*, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DConstVoid() {
    const Plane a[2]{};

    ConstVoidStridedArrayView3D b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));

    constexpr ConstVoidStridedArrayView3D cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(cb.data() == Cube);
    CORRADE_COMPARE(cb.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{48, 24, 8}));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView3D, ArrayView<int>, int*, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DVoidFrom() {
    Plane a[2]{};

    StridedArrayView3Di b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    VoidStridedArrayView3D bv = b;
    CORRADE_VERIFY(bv.data() == a);
    CORRADE_COMPARE(bv.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(bv.stride(), (Stride3D{48, 24, 8}));

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView3D, StridedArrayView3Di>::value);
}

void StridedArrayViewTest::construct3DConstVoidFrom() {
    Plane a[2]{};

    StridedArrayView3Di b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    ConstStridedArrayView3Di cb = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    ConstVoidStridedArrayView3D bv = b;
    ConstVoidStridedArrayView3D cbv = cb;
    CORRADE_VERIFY(bv.data() == a);
    CORRADE_VERIFY(cbv.data() == a);
    CORRADE_COMPARE(bv.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cbv.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(bv.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(cbv.stride(), (Stride3D{48, 24, 8}));

    constexpr ConstStridedArrayView3Di ccb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    constexpr ConstVoidStridedArrayView3D ccbv = ccb;
    CORRADE_VERIFY(ccbv.data() == Cube);
    CORRADE_COMPARE(ccbv.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(ccbv.stride(), (Stride3D{48, 24, 8}));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView3D, StridedArrayView3Di>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView3D, ConstStridedArrayView3Di>::value);
}

void StridedArrayViewTest::construct3DNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */

    StridedArrayView3Di a{{nullptr, 20}, {5, 7, 3}, {16, 8, 1}};
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(a.size(), (Size3D{5, 7, 3}));
    CORRADE_COMPARE(a.stride(), (Stride3D{16, 8, 1}));

    constexpr StridedArrayView3Di ca{{nullptr, 20}, {5, 7, 3}, {16, 8, 1}};
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.isEmpty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(ca.size(), (Size3D{5, 7, 3}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{16, 8, 1}));
}

void StridedArrayViewTest::construct3DZeroStride() {
    Plane a[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), 0, sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 0, 8}));
    CORRADE_COMPARE(b[0][0][0], 2);
    CORRADE_COMPARE(b[0][0][1], 16);
    CORRADE_COMPARE(b[0][0][2], 7853268);
    CORRADE_COMPARE(b[0][1][1], 16);
}

void StridedArrayViewTest::construct3DNegativeStride() {
    Plane a[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di b = {a, &a[1].plane[0].row[2].value, {2, 2, 3}, {-std::ptrdiff_t(sizeof(Plane)), sizeof(Plane::Row), -std::ptrdiff_t(sizeof(Plane::Row::Item))}};
    CORRADE_VERIFY(b.data() == &a[1].plane[0].row[2].value);
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{-48, 24, -8}));
    CORRADE_COMPARE(b[1][0][2], 2);
    CORRADE_COMPARE(b[1][0][1], 16);
    CORRADE_COMPARE(b[1][0][0], 7853268);
    CORRADE_COMPARE(b[1][1][1], 234810);
}

/* Two images, each 3 rows by 5 pixels */
constexpr int PackedData[2*3*5] {
     0,  1,  2,  3,  4,
     5,  6,  7,  8,  9,
    10, 11, 12, 13, 14,

    15, 16, 17, 18, 19,
    20, 21, 22, 23, 24,
    25, 26, 27, 28, 29
};

void StridedArrayViewTest::construct3DPackedSizeStride() {
    ConstStridedArrayView3Di a{PackedData, {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_VERIFY(a.data() == PackedData);
    CORRADE_COMPARE(a.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(a[1][1][2], 22);
    CORRADE_COMPARE(a[0][2][3], 13);

    constexpr ConstStridedArrayView3Di ca{PackedData, {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_VERIFY(ca.data() == PackedData);
    CORRADE_COMPARE(ca.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(ca[1][1][2], 22);
    CORRADE_COMPARE(ca[0][2][3], 13);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView3Di, ArrayView<int>, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DPackedSizeStrideVoid() {
    /* Same as PackedData, just not const */
    int packedData[2*3*5] {
        0,  1,  2,  3,  4,
        5,  6,  7,  8,  9,
        10, 11, 12, 13, 14,

        15, 16, 17, 18, 19,
        20, 21, 22, 23, 24,
        25, 26, 27, 28, 29
    };

    /* Should give the same result as construct3DPackedSizeStride */
    VoidStridedArrayView3D a{packedData, {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_VERIFY(a.data() == packedData);
    CORRADE_COMPARE(a.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{3*5*4, 5*4, 4}));

    /** @todo constexpr void but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidStridedArrayView3D, ArrayView<int>, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DPackedSizeStrideConstVoid() {
    /* Should give the same result as construct3DPackedSizeStride */
    ConstVoidStridedArrayView3D a{PackedData, {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_VERIFY(a.data() == PackedData);
    CORRADE_COMPARE(a.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{3*5*4, 5*4, 4}));

    constexpr ConstVoidStridedArrayView3D ca{PackedData, {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_VERIFY(ca.data() == PackedData);
    CORRADE_COMPARE(ca.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{3*5*4, 5*4, 4}));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidStridedArrayView3D, ArrayView<int>, Size3D, Stride3D>::value);
}

void StridedArrayViewTest::construct3DPackedSizeOnly() {
    /* Should give the same result as construct3DPackedSizeStride */
    ConstStridedArrayView3Di a{PackedData, {2, 3, 5}};
    CORRADE_VERIFY(a.data() == PackedData);
    CORRADE_COMPARE(a.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(a.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(a[1][1][2], 22);
    CORRADE_COMPARE(a[0][2][3], 13);

    constexpr ConstStridedArrayView3Di ca{PackedData, {2, 3, 5}};
    CORRADE_VERIFY(ca.data() == PackedData);
    CORRADE_COMPARE(ca.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(ca[1][1][2], 22);
    CORRADE_COMPARE(ca[0][2][3], 13);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView3Di, ArrayView<int>, Size3D>::value);
}

void StridedArrayViewTest::construct3DOneSizeZero() {
    Containers::String out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    StridedArrayView3Di a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out, "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DOneSizeZeroVoid() {
    Containers::String out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    VoidStridedArrayView3D a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out, "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DOneSizeZeroConstVoid() {
    Containers::String out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    ConstVoidStridedArrayView3D a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out, "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DFixedSize() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<StridedArrayView1Di, int(&)[10]>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, int(&)[10]>::value);
}

void StridedArrayViewTest::construct3DFixedSizeVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<VoidStridedArrayView1D, int(&)[10]>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidStridedArrayView3D, int(&)[10]>::value);
}

void StridedArrayViewTest::construct3DFixedSizeConstVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, int(&)[10]>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstVoidStridedArrayView3D, int(&)[10]>::value);
}

void StridedArrayViewTest::construct3DViewTooSmall() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Plane a[2];

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView3Di{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DViewTooSmallVoid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Plane a[2];

    Containers::String out;
    Error redirectError{&out};

    VoidStridedArrayView3D{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DViewTooSmallConstVoid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    const Plane a[2]{};

    Containers::String out;
    Error redirectError{&out};

    ConstVoidStridedArrayView3D{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DFromView() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<StridedArrayView1Di, ArrayView<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, ArrayView<int>>::value);
}

void StridedArrayViewTest::construct3DFromViewVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<VoidStridedArrayView1D, ArrayView<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidStridedArrayView3D, ArrayView<int>>::value);
}

void StridedArrayViewTest::construct3DFromViewConstVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, ArrayView<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstVoidStridedArrayView3D, ArrayView<int>>::value);
}

void StridedArrayViewTest::construct3DFromStaticView() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<StridedArrayView1Di, StaticArrayView<10, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, StaticArrayView<10, int>>::value);
}

void StridedArrayViewTest::construct3DFromStaticViewVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<VoidStridedArrayView1D, StaticArrayView<10, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidStridedArrayView3D, StaticArrayView<10, int>>::value);
}

void StridedArrayViewTest::construct3DFromStaticViewConstVoid() {
    /* Not using is_convertible to catch also accidental explicit conversions */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, StaticArrayView<10, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstVoidStridedArrayView3D, StaticArrayView<10, int>>::value);
}

void StridedArrayViewTest::construct3DDerived() {
    /* Valid use case: constructing Containers::StridedArrayView<Math::Vector<3, Float>>
       from Containers::StridedArrayView<Color3> because the data have the same size
       and data layout */

    Derived b[5];
    Containers::StridedArrayView2D<Derived> bv{b, {5, 1}};
    Containers::StridedArrayView2D<Base> a{b, {5, 1}};
    Containers::StridedArrayView2D<Base> av{bv};

    CORRADE_VERIFY(a.data() == &b[0]);
    CORRADE_VERIFY(av.data() == &b[0]);
    CORRADE_COMPARE(a.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(a.stride(), (Stride2D{2, 2}));
    CORRADE_COMPARE(av.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(av.stride(), (Stride2D{2, 2}));

    constexpr Containers::StridedArrayView2D<const Derived> cbv{DerivedArray, {5, 1}};
    #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG_CL) || _MSC_VER >= 1940
    /* Implicit pointer downcast not constexpr on MSVC 2015, causes an ICE on
       MSVC 2017, 2019 and 2022 (but only in the 3D case, not for 1D). Maybe
       2025 will be the year when MSVC can finally do plain C++11? */
    constexpr
    #endif
    Containers::StridedArrayView2D<const Base> ca{DerivedArray, {5, 1}};
    #if !defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_CLANG_CL) || _MSC_VER >= 1940
    /* Implicit pointer downcast not constexpr on MSVC 2015, causes an ICE on
       MSVC 2017, 2019 and 2022 (but only in the 3D case, not for 1D). Maybe
       2025 will be the year when MSVC can finally do plain C++11? */
    constexpr
    #endif
    Containers::StridedArrayView2D<const Base> cav{cbv};

    CORRADE_VERIFY(ca.data() == &DerivedArray[0]);
    CORRADE_VERIFY(cav.data() == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(ca.stride(), (Stride2D{2, 2}));
    CORRADE_COMPARE(cav.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(cav.stride(), (Stride2D{2, 2}));

    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::StridedArrayView3D<Base>, Containers::StridedArrayView3D<Derived>>::value);
}

void StridedArrayViewTest::construct3DFrom2DWithArrayType() {
    int data[2*3][5]{
        {2, 3, 1, 7, 12},
        {5, 1, 7, 12, -1},
        {1, 7, 12, 22, 15},

        {1, 7, -17, 12, 7},
        {9, 19, 1, 7, 12},
        {1, 7, 12, 4, 8}
    };

    /* Done the hard way just for comparison */
    StridedArrayView3Di a{data, &data[0][0], {2, 3, 5}, {3*5*4, 5*4, 4}};
    CORRADE_COMPARE(a[0][2][4], 15);
    CORRADE_COMPARE(a[1][0][2], -17);

    /* This is apparently possible */
    StridedArrayView2D<int[5]> b{data, {2, 3}};
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.stride(), (Stride2D{3*5*4, 5*4}));
    CORRADE_COMPARE(b[0][2][4], 15);
    CORRADE_COMPARE(b[1][0][2], -17);

    /* And now this is too */
    StridedArrayView3Di b3 = b;
    CORRADE_COMPARE(b3.data(), &data[0]);
    CORRADE_COMPARE(b3.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(b3[0][2][4], 15);
    CORRADE_COMPARE(b3[1][0][2], -17);

    /* Constructing a const view from a mutable with an array type of one
       dimension less should also be possible */
    ConstStridedArrayView3Di cb3 = b;
    CORRADE_COMPARE(cb3.data(), &data[0]);
    CORRADE_COMPARE(cb3.size(), (Size3D{2, 3, 5}));
    CORRADE_COMPARE(cb3.stride(), (Stride3D{3*5*4, 5*4, 4}));
    CORRADE_COMPARE(cb3[0][2][4], 15);
    CORRADE_COMPARE(cb3[1][0][2], -17);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView3Di, StridedArrayView2D<int[5]>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstStridedArrayView3Di, StridedArrayView2D<int[5]>>::value);
    /* Construction the other way shouldn't be possible */
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView1Di, StridedArrayView3Di>::value);
    /* Construction of a mutable view from a const or another type shouldn't be
       possible either */
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, StridedArrayView2D<const int[5]>>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, StridedArrayView2D<float[5]>>::value);
}

void StridedArrayViewTest::construct3DFromLessDimensions() {
    int data[6]{
        2, 3,
        5, -1,
        22, 15
    };
    StridedArrayView1Di a = data;
    StridedArrayView2Di b{data, {3, 2}};

    StridedArrayView3Di a3 = a;
    CORRADE_COMPARE(a3.data(), &data[0]);
    CORRADE_COMPARE(a3.size(), (Size3D{1, 1, 6}));
    CORRADE_COMPARE(a3.stride(), (Stride3D{24, 24, 4}));
    CORRADE_COMPARE(a3[0][0][3], -1);

    StridedArrayView3Di b3 = b;
    CORRADE_COMPARE(b3.data(), &data[0]);
    CORRADE_COMPARE(b3.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{24, 8, 4}));
    CORRADE_COMPARE(b3[0][0][1], 3);
    CORRADE_COMPARE(b3[0][1][0], 5);
    CORRADE_COMPARE(b3[0][1][1], -1);
    CORRADE_COMPARE(b3[0][2][1], 15);

    /* Constructing a const view from a mutable with less dimensions should
       also be possible */
    ConstStridedArrayView3Di ca3 = a;
    CORRADE_COMPARE(ca3.data(), &data[0]);
    CORRADE_COMPARE(ca3.size(), (Size3D{1, 1, 6}));
    CORRADE_COMPARE(ca3.stride(), (Stride3D{24, 24, 4}));
    CORRADE_COMPARE(ca3[0][0][3], -1);

    ConstStridedArrayView3Di cb3 = b;
    CORRADE_COMPARE(cb3.data(), &data[0]);
    CORRADE_COMPARE(cb3.size(), (Size3D{1, 3, 2}));
    CORRADE_COMPARE(cb3.stride(), (Stride3D{24, 8, 4}));
    CORRADE_COMPARE(cb3[0][0][1], 3);
    CORRADE_COMPARE(cb3[0][1][0], 5);
    CORRADE_COMPARE(cb3[0][1][1], -1);
    CORRADE_COMPARE(cb3[0][2][1], 15);

    CORRADE_VERIFY(std::is_nothrow_constructible<StridedArrayView3Di, StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstStridedArrayView3Di, StridedArrayView1Di>::value);
    /* Construction the other way shouldn't be possible */
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView1Di, StridedArrayView3Di>::value);
    /* Construction of a mutable view from a const or another type shouldn't be
       possible either */
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, ConstStridedArrayView1Di>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, StridedArrayView1D<float>>::value);
}

/* Without a corresponding SFINAE check in the std::nullptr_t constructor, this
   is ambiguous, but *only* if the size_t overload has a second 64-bit
   argument. If both would be the same, it wouldn't be ambigous, if the size_t
   overload second argument was 32-bit and the other 16-bit it wouldn't be
   either. */
int integerArrayOverload(std::size_t, long long) {
    return 76;
}
int integerArrayOverload(const ConstStridedArrayView1Di&, int) {
    return 39;
}

void StridedArrayViewTest::constructZeroNullPointerAmbiguity() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverload(25, 2), 76);
    CORRADE_COMPARE(integerArrayOverload(nullptr, 2), 39);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverload(0, 3), 76);
}

/* Same as above, just for the void StridedArrayView specialization */
int integerArrayOverloadVoid(std::size_t, long long) {
    return 67;
}
int integerArrayOverloadVoid(const VoidStridedArrayView1D&, int) {
    return 93;
}

void StridedArrayViewTest::constructZeroNullPointerAmbiguityVoid() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverloadVoid(25, 2), 67);
    CORRADE_COMPARE(integerArrayOverloadVoid(nullptr, 2), 93);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverloadVoid(0, 3), 67);
}

/* Same as above, just for the const void StridedArrayView specialization */
int integerArrayOverloadConstVoid(std::size_t, long long) {
    return 676;
}
int integerArrayOverloadConstVoid(const ConstVoidStridedArrayView1D&, int) {
    return 939;
}

void StridedArrayViewTest::constructZeroNullPointerAmbiguityConstVoid() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverloadConstVoid(25, 2), 676);
    CORRADE_COMPARE(integerArrayOverloadConstVoid(nullptr, 2), 939);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverloadConstVoid(0, 3), 676);
}

void StridedArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(StridedArrayView1Di{a});
    CORRADE_VERIFY(!StridedArrayView1Di{});
    CORRADE_VERIFY(VoidStridedArrayView1D{a});
    CORRADE_VERIFY(!VoidStridedArrayView1D{});
    CORRADE_VERIFY(ConstVoidStridedArrayView1D{a});
    CORRADE_VERIFY(!ConstVoidStridedArrayView1D{});

    constexpr ConstStridedArrayView1Di ca = Array10;
    constexpr bool boolCa = !!ca;
    CORRADE_VERIFY(boolCa);

    constexpr ConstStridedArrayView1Di cb;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(!boolCb);

    /** @todo constexpr void but not const? c++14? */

    constexpr ConstVoidStridedArrayView1D cvb = Array10;
    constexpr bool boolCvb = !!cvb;
    CORRADE_VERIFY(boolCvb);

    constexpr ConstVoidStridedArrayView1D cvc;
    constexpr bool boolCvc = !!cvc;
    CORRADE_VERIFY(!boolCvc);

    CORRADE_VERIFY(std::is_constructible<bool, StridedArrayView1Di>::value);
    CORRADE_VERIFY(std::is_constructible<bool, VoidStridedArrayView1D>::value);
    CORRADE_VERIFY(std::is_constructible<bool, ConstVoidStridedArrayView1D>::value);
    CORRADE_VERIFY(!std::is_constructible<int, StridedArrayView1Di>::value);
    CORRADE_VERIFY(!std::is_constructible<int, VoidStridedArrayView1D>::value);
    CORRADE_VERIFY(!std::is_constructible<int, ConstVoidStridedArrayView1D>::value);
}

void StridedArrayViewTest::convertConst() {
    int a[3];
    StridedArrayView1Di b = a;
    ConstStridedArrayView1Di c = b;
    CORRADE_VERIFY(c.data() == a);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c.stride(), 4);
}

void StridedArrayViewTest::convertFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    {
        StridedArrayView1Di b = a;
        CORRADE_COMPARE(b.data(), &data[0]);
        CORRADE_COMPARE(b.size(), 5);
    } {
        auto b = stridedArrayView(a);
        CORRADE_VERIFY(std::is_same<decltype(b), StridedArrayView1Di>::value);
        CORRADE_COMPARE(b.data(), &data[0]);
        CORRADE_COMPARE(b.size(), 5);
    }

    constexpr ConstIntView ca{Array10, 10};
    CORRADE_COMPARE(ca.data, Array10);
    CORRADE_COMPARE(ca.size, 10);

    {
        constexpr ConstStridedArrayView1Di cb = ca;
        CORRADE_COMPARE(cb.data(), Array10);
        CORRADE_COMPARE(cb.size(), 10);
    } {
        constexpr auto cb = stridedArrayView(ca);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value);
        CORRADE_COMPARE(cb.data(), Array10);
        CORRADE_COMPARE(cb.size(), 10);
    }

    /* Conversion from a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::StridedArrayView1D<int>, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StridedArrayView1D<float>, IntView>::value);
}

void StridedArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstStridedArrayView1Di b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion from a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::StridedArrayView1D<const int>, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StridedArrayView1D<const float>, IntView>::value);

    /* Creating a non-const view from a const type should not be possible. Not
       using is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::StridedArrayView1D<const int>, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StridedArrayView1D<int>, ConstIntView>::value);
}

void StridedArrayViewTest::convertVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    VoidStridedArrayView1D b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5);
}

void StridedArrayViewTest::convertConstVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidStridedArrayView1D b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5);

    /* Creating a non-const view from a const type should not be possible. Not
       using is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidStridedArrayView1D, ConstIntView>::value);
}

void StridedArrayViewTest::convertConstVoidFromConstExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidStridedArrayView1D b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5);
}

/* Needs to be here in order to use it in constexpr */
constexpr const int Array6[6]{ 2, 16, 7853268, -100, 234810, 0 };

void StridedArrayViewTest::convert3DBool() {
    typedef StridedDimensions<1, bool> Bools1D;
    typedef StridedDimensions<3, bool> Bools3D;

    int data[6];
    StridedArrayView3Di a{data, {1, 2, 3}, {24, 12, 4}};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.isEmpty(), (Bools3D{false, false, false}));

    StridedArrayView3Di b{{nullptr, 6}, {1, 0, 3}, {24, 12, 4}};
    CORRADE_VERIFY(!b);
    CORRADE_COMPARE(b.isEmpty(), (Bools3D{false, true, false}));

    constexpr ConstStridedArrayView3Di ca{Array6, {1, 2, 3}, {24, 12, 4}};
    constexpr bool boolCa = !!ca;
    constexpr Bools3D emptyCa = ca.isEmpty();
    CORRADE_VERIFY(boolCa);
    CORRADE_COMPARE(emptyCa, (Bools3D{false, false, false}));

    constexpr ConstStridedArrayView3Di cb{{nullptr, 6}, {1, 0, 3}, {24, 12, 4}};
    constexpr bool boolCb = !!cb;
    constexpr Bools3D emptyCb = cb.isEmpty();
    CORRADE_VERIFY(!boolCb);
    CORRADE_COMPARE(emptyCb, (Bools3D{false, true, false}));

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY(std::is_constructible<bool, StridedArrayView3Di>::value);
    CORRADE_VERIFY(std::is_constructible<bool, VoidStridedArrayView3D>::value);
    CORRADE_VERIFY(std::is_constructible<bool, ConstVoidStridedArrayView3D>::value);
    CORRADE_VERIFY(!std::is_constructible<int, StridedArrayView3Di>::value);
    CORRADE_VERIFY(!std::is_constructible<int, VoidStridedArrayView3D>::value);
    CORRADE_VERIFY(!std::is_constructible<int, ConstVoidStridedArrayView3D>::value);

    /* Implicit conversion to bool from empty is allowed only for 1D */
    CORRADE_VERIFY(std::is_convertible<Bools1D, bool>::value);
    CORRADE_VERIFY(!std::is_convertible<Bools3D, bool>::value);
}

void StridedArrayViewTest::convert3DConst() {
    int a[6];
    StridedArrayView3Di b{a, {1, 2, 3}};
    ConstStridedArrayView3Di c = b;
    CORRADE_VERIFY(c.data() == a);
    CORRADE_COMPARE(c.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(c.stride(), (Stride3D{24, 12, 4}));
}

void StridedArrayViewTest::convert3DFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StridedArrayView1Di, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<StridedArrayView3Di, IntView>::value);
}

void StridedArrayViewTest::convert3DConstFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstStridedArrayView1Di, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstStridedArrayView3Di, IntView>::value);
}

void StridedArrayViewTest::convert3DVoidFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<VoidStridedArrayView1D, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidStridedArrayView3D, IntView>::value);
}

void StridedArrayViewTest::convert3DConstVoidFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstVoidStridedArrayView3D, IntView>::value);
}

void StridedArrayViewTest::convert3DConstVoidFromConstExternalView() {
    /* Conversion to a multi-dimensional type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstVoidStridedArrayView1D, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstVoidStridedArrayView3D, ConstIntView>::value);
}

void StridedArrayViewTest::asContiguous() {
    int a[2*3*5];
    StridedArrayView3Di b{a, {5, 3, 2}, {6*4, 2*4, 4}};

    /* The array should be contiguous -- so if I fill it, it'll be a monotonic
       sequence of numbers */
    int n = 0;
    for(StridedArrayView2Di i: b)
        for(StridedArrayView1Di j: i)
            for(int& k: j)
                k = ++n;
    CORRADE_COMPARE_AS(Containers::arrayView(a), Containers::arrayView<int>({
         1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30
    }), TestSuite::Compare::Container);
    CORRADE_VERIFY(b.isContiguous<2>());
    CORRADE_VERIFY(b.isContiguous<1>());
    CORRADE_VERIFY(b.isContiguous<0>());
    CORRADE_VERIFY(b.isContiguous());

    {
        ArrayView<int> bc = b.asContiguous();
        CORRADE_COMPARE(bc.data(), b.data());
        CORRADE_COMPARE(bc.size(), 5*3*2);

        StridedArrayView1Di b0 = b.asContiguous<0>();
        CORRADE_COMPARE(b0.data(), b.data());
        CORRADE_COMPARE(b0.size(), 5*3*2);
        CORRADE_COMPARE(b0.stride(), 4);

        StridedArrayView2Di b1 = b.asContiguous<1>();
        CORRADE_COMPARE(b1.data(), b.data());
        CORRADE_COMPARE(b1.size(), (Size2D{5, 3*2}));
        CORRADE_COMPARE(b1.stride(), (Stride2D{3*2*4, 4}));

        /* This should return the exact same view */
        StridedArrayView3Di b2 = b.asContiguous<2>();
        CORRADE_COMPARE(b2.data(), b.data());
        CORRADE_COMPARE(b2.size(), b.size());
        CORRADE_COMPARE(b2.stride(), b.stride());

    /* Non-contiguous in the first dimension */
    } {
        StridedArrayView3Di c{a, {2, 3, 2}, {2*6*4, 2*4, 4}};
        CORRADE_VERIFY(c.isContiguous<2>());
        CORRADE_VERIFY(c.isContiguous<1>());
        CORRADE_VERIFY(!c.isContiguous<0>());
        CORRADE_VERIFY(!c.isContiguous());

        StridedArrayView2Di c1 = c.asContiguous<1>();
        CORRADE_COMPARE(c1.data(), c.data());
        CORRADE_COMPARE(c1.size(), (Size2D{2, 3*2}));
        CORRADE_COMPARE(c1.stride(), (Stride2D{2*6*4, 4}));

        /* This should return the exact same view */
        StridedArrayView3Di c2 = c.asContiguous<2>();
        CORRADE_COMPARE(c2.data(), c.data());
        CORRADE_COMPARE(c2.size(), c.size());
        CORRADE_COMPARE(c2.stride(), c.stride());

    /* Non-contiguous in the second dimension */
    } {
        StridedArrayView3Di d{a, {5, 1, 2}, {6*4, 2*2*4, 4}};
        CORRADE_VERIFY(d.isContiguous<2>());
        CORRADE_VERIFY(!d.isContiguous<1>());
        CORRADE_VERIFY(!d.isContiguous<0>());

        /* This should return the exact same view */
        StridedArrayView3Di d2 = d.asContiguous<2>();
        CORRADE_COMPARE(d2.data(), d.data());
        CORRADE_COMPARE(d2.size(), d.size());
        CORRADE_COMPARE(d2.stride(), d.stride());

    /* Not contigous in the third dimension, can't create any view */
    } {
        StridedArrayView3Di e{a, {5, 3, 1}, {6*4, 2*4, 2*4}};
        CORRADE_VERIFY(!e.isContiguous<2>());
        CORRADE_VERIFY(!e.isContiguous<1>());
        CORRADE_VERIFY(!e.isContiguous<0>());

    /* "Broadcast" */
    } {
        StridedArrayView3Di f{a, {5, 3, 2}, {6*4, 0, 4}};
        CORRADE_VERIFY(f.isContiguous<2>());
        CORRADE_VERIFY(!f.isContiguous<1>());
        CORRADE_VERIFY(!f.isContiguous<0>());

        /* This should again return the exact same view */
        StridedArrayView3Di f2 = f.asContiguous<2>();
        CORRADE_COMPARE(f2.data(), f.data());
        CORRADE_COMPARE(f2.size(), f.size());
        CORRADE_COMPARE(f2.stride(), f.stride());
    }

    /* Packed block of memory, but strides not in order / negative */
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<2>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<1>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<0>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<2>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<1>());
    CORRADE_VERIFY(!b.transposed<1, 2>().isContiguous<0>());
}

void StridedArrayViewTest::asContiguousNonContiguous() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* Taken from above */
    int a[2*3*5];
    StridedArrayView3Di c{a, {2, 3, 2}, {2*6*4, 2*4, 4}};
    StridedArrayView3Di d{a, {5, 1, 2}, {6*4, 2*2*4, 4}};
    StridedArrayView3Di e{a, {5, 3, 1}, {6*4, 2*4, 2*4}};

    Containers::String out;
    Error redirectError{&out};
    c.asContiguous();
    c.asContiguous<0>();
    d.asContiguous<1>();
    e.asContiguous<2>();
    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::asContiguous(): the view is not contiguous\n"
        "Containers::StridedArrayView::asContiguous(): the view is not contiguous from dimension 0\n"
        "Containers::StridedArrayView::asContiguous(): the view is not contiguous from dimension 1\n"
        "Containers::StridedArrayView::asContiguous(): the view is not contiguous from dimension 2\n");
}

void StridedArrayViewTest::access() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView1Di b{a, &a[0].value, 10, 8};
    for(std::size_t i = 0; i != b.size(); ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 9);
    CORRADE_COMPARE(b[4], 4);

    ConstStridedArrayView1Di c = {a, &a[0].value, 10, 8};
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstStridedArrayView1Di cb = {Struct, &Struct[0].value, 10, 8};

    constexpr const void* data = cb.data();
    CORRADE_VERIFY(data == Struct);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 10);

    constexpr std::ptrdiff_t stride = cb.stride();
    CORRADE_COMPARE(stride, 8);
}

void StridedArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const StridedArrayView1Di b = a;
    b.front() = 0;
    *(b.begin()+1) = 1;
    *(b.cbegin()+2) = 2;
    b[3] = 3;
    *(b.end()-3) = 4;
    *(b.cend()-2) = 5;
    b.back() = 6;

    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a[5], 5);
    CORRADE_COMPARE(a[6], 6);
}

void StridedArrayViewTest::accessZeroStride() {
    struct {
        int value;
        int other;
    } a[1]{
        {23125, 1}
    };

    StridedArrayView1Di b{a, &a[0].value, 10, 0};
    for(std::size_t i = 0; i != b.size(); ++i)
        b[i] += 1;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 0);
    CORRADE_COMPARE(b.front(), 23135);
    CORRADE_COMPARE(b.back(), 23135);
    CORRADE_COMPARE(b[4], 23135);
}

void StridedArrayViewTest::accessNegativeStride() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView1Di b{a, &a[9].value, 10, -8};
    for(std::size_t i = 0; i != b.size(); ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == &a[9].value);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), -8);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 9);
    CORRADE_COMPARE(b[4], 4);
}

void StridedArrayViewTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView1Di a;
    a.front();
    a.back();

    int data[5];
    StridedArrayView1Di b = data;
    b[5];

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::front(): view is empty\n"
        "Containers::StridedArrayView::back(): view is empty\n"
        "Containers::StridedArrayView::operator[](): index 5 out of range for 5 elements\n");
}

void StridedArrayViewTest::access3D() {
    Plane a[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di b{a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));

    CORRADE_COMPARE(b[0].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[1].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[0].stride(), (Stride2D{24, 8}));
    CORRADE_COMPARE(b[1].stride(), (Stride2D{24, 8}));
    CORRADE_COMPARE(b[0][0].size(), 3);
    CORRADE_COMPARE(b[0][1].size(), 3);
    CORRADE_COMPARE(b[0][0].stride(), 8);
    CORRADE_COMPARE(b[0][1].stride(), 8);

    CORRADE_COMPARE(b.front().back().front(), -100);
    CORRADE_COMPARE(b[0][0][1], 16);
    CORRADE_COMPARE((b[{0, 0, 1}]), 16);
    CORRADE_COMPARE(b[0][1][0], -100);
    CORRADE_COMPARE((b[{0, 1, 0}]), -100);
    CORRADE_COMPARE(b[0][1][2], 232342);
    CORRADE_COMPARE((b[{0, 1, 2}]), 232342);

    ConstStridedArrayView3Di c = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstStridedArrayView3Di cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    constexpr const void* data = cb.data();
    CORRADE_VERIFY(data == Cube);

    constexpr Size3D size = cb.size();
    CORRADE_COMPARE(size, (Size3D{2, 2, 3}));

    constexpr Stride3D stride = cb.stride();
    CORRADE_COMPARE(stride, (Stride3D{48, 24, 8}));
}

void StridedArrayViewTest::access3DConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[8];
    const StridedArrayView3Di b = {a, {4, 2, 1}};
    b.front().front().front() = 0;
    *(*(*(b.begin() + 1)).begin()).begin() = 1;
    *(*(*(b.cbegin() + 2)).cbegin()).begin() = 2;
    b[3][0][0] = 3;
    b[{0, 1, 0}] = 4;
    *((*((*(b.end() - 3)).end() - 1)).end() - 1) = 5;
    *((*((*(b.end() - 2)).end() - 1)).end() - 1) = 6;
    b.back().back().back() = 7;

    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView({0, 4, 1, 5, 2, 6, 3, 7}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::access3DNegativeStride() {
    Plane a[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di b{a, &a[1].plane[1].row[2].value, {2, 2, 3}, {-std::ptrdiff_t(sizeof(Plane)), -std::ptrdiff_t(sizeof(Plane::Row)), -std::ptrdiff_t(sizeof(Plane::Row::Item))}};

    CORRADE_VERIFY(b.data() == &a[1].plane[1].row[2].value);
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{-48, -24, -8}));

    CORRADE_COMPARE(b[0].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[1].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[0].stride(), (Stride2D{-24, -8}));
    CORRADE_COMPARE(b[1].stride(), (Stride2D{-24, -8}));
    CORRADE_COMPARE(b[0][0].size(), 3);
    CORRADE_COMPARE(b[0][1].size(), 3);
    CORRADE_COMPARE(b[0][0].stride(), -8);
    CORRADE_COMPARE(b[0][1].stride(), -8);

    CORRADE_COMPARE(b.back().front().back(), -100);
    CORRADE_COMPARE(b[1][0][0], 232342);
    CORRADE_COMPARE((b[{1, 0, 0}]), 232342);
}

void StridedArrayViewTest::access3DZeroStride() {
    Plane a[1]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}}
    };

    StridedArrayView3Di b{a, &a[0].plane[0].row[0].value, {2, 2, 3}, {0, 0, 0}};

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{0, 0, 0}));

    CORRADE_COMPARE(b[0].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[1].size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b[0].stride(), (Stride2D{0, 0}));
    CORRADE_COMPARE(b[1].stride(), (Stride2D{0, 0}));
    CORRADE_COMPARE(b[0][0].size(), 3);
    CORRADE_COMPARE(b[0][1].size(), 3);
    CORRADE_COMPARE(b[0][0].stride(), 0);
    CORRADE_COMPARE(b[0][1].stride(), 0);

    CORRADE_COMPARE(b.front().back().front(), 2);
    CORRADE_COMPARE(b[0][1][2], 2);
    CORRADE_COMPARE((b[{0, 1, 2}]), 2);
}

void StridedArrayViewTest::access3DInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView3Di a{{nullptr, 1}, {1, 0, 1}, {4, 0, 4}};
    a.front().back().size();
    a.back().front().size();

    int data[6];
    StridedArrayView3Di b{data, {1, 2, 3}, {24, 12, 4}};
    b[0][1][3];
    b[{0, 1, 5}];
    b[{0, 2, 3}];
    b[{1, 1, 3}];

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::back(): view is empty\n"
        "Containers::StridedArrayView::front(): view is empty\n"
        "Containers::StridedArrayView::operator[](): index 3 out of range for 3 elements\n"
        "Containers::StridedArrayView::operator[](): index {0, 1, 5} out of range for {1, 2, 3} elements\n"
        "Containers::StridedArrayView::operator[](): index {0, 2, 3} out of range for {1, 2, 3} elements\n"
        "Containers::StridedArrayView::operator[](): index {1, 1, 3} out of range for {1, 2, 3} elements\n");
}

void StridedArrayViewTest::iterator() {
    auto&& data = IteratorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    struct {
        int value;
        int:32;
    } d[7]{{443}, {1}, {2}, {3}, {4}, {5}, {6}};

    /* Verifying also that iterators of different views and iterators of
       different strides are not comparable */
    StridedArrayView1Di a{d, &d[0].value, 7, data.stride1};
    if(data.flipped) a = a.flipped<0>();
    StridedArrayView1Di b;

    CORRADE_VERIFY(a.begin() == a.begin());
    /* These are equal if stride is zero */
    CORRADE_COMPARE(a.begin() != a.every(2).begin(), data.stride1 != Stride1D{});
    CORRADE_VERIFY(a.begin() != b.begin());
    CORRADE_VERIFY(!(a.begin() != a.begin()));
    /* These are equal if stride is zero */
    CORRADE_COMPARE(!(a.begin() == a.every(2).begin()), data.stride1 != Stride1D{});
    CORRADE_VERIFY(!(a.begin() == b.begin()));
    CORRADE_VERIFY(a.begin() != a.begin() + 1);

    CORRADE_VERIFY(a.begin() < a.begin() + 1);
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(a.every(2).begin() < a.begin() + 1), data.stride1 != Stride1D{});
    CORRADE_VERIFY(!(a.begin() < a.begin()));
    CORRADE_VERIFY(a.begin() <= a.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(a.begin() <= a.every(2).begin()), data.stride1 != Stride1D{});
    CORRADE_VERIFY(!(a.begin() + 1 <= a.begin()));

    CORRADE_VERIFY(a.begin() + 1 > a.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(a.begin() + 1 > a.every(2).begin()), data.stride1 != Stride1D{});
    CORRADE_VERIFY(!(a.begin() > a.begin()));
    CORRADE_VERIFY(a.begin() >= a.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(a.begin() >= a.every(2).begin()), data.stride1 != Stride1D{});
    CORRADE_VERIFY(!(a.begin() >= a.begin() + 1));

    CORRADE_VERIFY(a.cbegin() == a.begin());
    CORRADE_VERIFY(a.cbegin() != b.begin());
    CORRADE_VERIFY(a.cend() == a.end());
    CORRADE_VERIFY(a.cend() != b.end());

    CORRADE_COMPARE(*(a.begin() + 2), data.dataBegin1);
    CORRADE_COMPARE(*(a.begin() += 2), data.dataBegin1);
    CORRADE_COMPARE(*(2 + a.begin()), data.dataBegin1);
    CORRADE_COMPARE(*(a.end() - 2), data.dataEnd1);
    CORRADE_COMPARE(*(a.end() -= 2), data.dataEnd1);
    CORRADE_COMPARE(a.end() - a.begin(), a.size());

    CORRADE_COMPARE(*(++a.begin()), data.dataBeginIncrement1);
    CORRADE_COMPARE(*(--a.end()), data.dataEndDecrement1);
}

void StridedArrayViewTest::iterator3D() {
    auto&& data = IteratorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    struct {
        int value;
        int:32;
    } d[12]{
        {0}, {1}, {2},
        {3}, {4}, {5},

        {6}, {7}, {8},
        {9}, {10}, {11}
    };

    /* Verifying also that iterators of different views are not compared equal */
    StridedArrayView3Di a{d, &d[0].value, {2, 2, 3}, data.stride3};
    if(data.flipped) a = a.flipped<2>();
    StridedArrayView3Di b;

    CORRADE_VERIFY(a.begin() == a.begin());
    CORRADE_VERIFY(a.begin() != b.begin());
    CORRADE_VERIFY(!(a.begin() != a.begin()));
    CORRADE_VERIFY(!(a.begin() == b.begin()));
    CORRADE_VERIFY(a.begin() != a.begin() + 1);

    CORRADE_VERIFY(a.begin() < a.begin() + 1);
    CORRADE_VERIFY(!(a.begin() < a.begin()));
    CORRADE_VERIFY(a.begin() <= a.begin());
    CORRADE_VERIFY(!(a.begin() + 1 <= a.begin()));

    CORRADE_VERIFY(a.begin() + 1 > a.begin());
    CORRADE_VERIFY(!(a.begin() > a.begin()));
    CORRADE_VERIFY(a.begin() >= a.begin());
    CORRADE_VERIFY(!(a.begin() >= a.begin() + 1));

    CORRADE_VERIFY(a.cbegin() == a.begin());
    CORRADE_VERIFY(a.cbegin() != b.begin());
    CORRADE_VERIFY(a.cend() == a.end());
    CORRADE_VERIFY(a.cend() != b.end());

    CORRADE_COMPARE(*(*((*(a.begin() + 1)).begin() + 1)).begin(), data.dataBegin3);
    CORRADE_COMPARE(*(*(1 + (*(1 + a.begin())).begin())).begin(), data.dataBegin3);
    CORRADE_COMPARE(*((*((*(a.end() - 1)).end() - 1)).end() - 2), data.dataEnd3);
    CORRADE_COMPARE(a.end() - a.begin(), a.size()[0]);

    CORRADE_COMPARE(*(++(*(++(*(++a.begin())).begin())).begin()), data.dataBeginIncrement3);
    CORRADE_COMPARE(*(--(*(--(*(--a.end())).end())).end()), data.dataEndDecrement3);
}

void StridedArrayViewTest::rangeBasedFor() {
    struct {
        int value;
        int:32;
    } data[5];
    StridedArrayView1Di a{data, &data[0].value, 5, 8};

    int i = 0;
    for(int& x: a)
        x = ++i;

    CORRADE_COMPARE(data[0].value, 1);
    CORRADE_COMPARE(data[1].value, 2);
    CORRADE_COMPARE(data[2].value, 3);
    CORRADE_COMPARE(data[3].value, 4);
    CORRADE_COMPARE(data[4].value, 5);
}

void StridedArrayViewTest::rangeBasedFor3D() {
    struct {
        int value;
        int:32;
    } data[12];
    StridedArrayView3Di a{data, &data[0].value, {2, 2, 3}, {48, 24, 8}};

    int i = 0;
    for(auto&& z: a) for(auto&& y: z) for(int& x: y)
        x = ++i;

    CORRADE_COMPARE(data[0].value, 1);
    CORRADE_COMPARE(data[1].value, 2);
    CORRADE_COMPARE(data[2].value, 3);
    CORRADE_COMPARE(data[3].value, 4);
    CORRADE_COMPARE(data[4].value, 5);
    CORRADE_COMPARE(data[5].value, 6);
    CORRADE_COMPARE(data[6].value, 7);
    CORRADE_COMPARE(data[7].value, 8);
    CORRADE_COMPARE(data[8].value, 9);
    CORRADE_COMPARE(data[9].value, 10);
    CORRADE_COMPARE(data[10].value, 11);
    CORRADE_COMPARE(data[11].value, 12);
}

void StridedArrayViewTest::rangeBasedForZeroStride() {
    int a = 0;
    StridedArrayView1Di b{{&a, 1}, 5, 0};
    for(int& i: b)
        i += 1;

    CORRADE_COMPARE(a, 5);
}

void StridedArrayViewTest::rangeBasedForZeroStride3D() {
    int data[2]{};
    StridedArrayView3Di a{data, {5, 3, 2}, {0, 0, 4}};
    for(auto&& z: a) for(auto&& y: z) for(int& x: y)
        x += 1;

    CORRADE_COMPARE(data[0], 15);
    CORRADE_COMPARE(data[1], 15);
}

void StridedArrayViewTest::rangeBasedForNegativeStride() {
    struct {
        int value;
        int:32;
    } data[5];
    StridedArrayView1Di a{data, &data[4].value, 5, -8};

    int i = 0;
    for(int& x: a)
        x = ++i;

    CORRADE_COMPARE(data[0].value, 5);
    CORRADE_COMPARE(data[1].value, 4);
    CORRADE_COMPARE(data[2].value, 3);
    CORRADE_COMPARE(data[3].value, 2);
    CORRADE_COMPARE(data[4].value, 1);
}

void StridedArrayViewTest::rangeBasedForNegativeStride3D() {
    struct {
        int value;
        int:32;
    } data[12];
    StridedArrayView3Di a{data, &data[11].value, {2, 2, 3}, {-48, -24, -8}};

    int i = 0;
    for(auto&& z: a) for(auto&& y: z) for(int& x: y)
        x = ++i;

    CORRADE_COMPARE(data[0].value, 12);
    CORRADE_COMPARE(data[1].value, 11);
    CORRADE_COMPARE(data[2].value, 10);
    CORRADE_COMPARE(data[3].value, 9);
    CORRADE_COMPARE(data[4].value, 8);
    CORRADE_COMPARE(data[5].value, 7);
    CORRADE_COMPARE(data[6].value, 6);
    CORRADE_COMPARE(data[7].value, 5);
    CORRADE_COMPARE(data[8].value, 4);
    CORRADE_COMPARE(data[9].value, 3);
    CORRADE_COMPARE(data[10].value, 2);
    CORRADE_COMPARE(data[11].value, 1);
}

void StridedArrayViewTest::slice() {
    struct {
        int value;
        float other;
    } data[5]{{1, 0.0f}, {2, 5.0f}, {3, -1.0f}, {4, 0.5f}, {5, -0.1f}};
    StridedArrayView1Di a{data, &data[0].value, 5, 8};

    StridedArrayView1Di b1 = a.slice(1, 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    StridedArrayView1Di b2 = a.sliceSize(1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    StridedArrayView1Di c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    StridedArrayView1Di d = a.exceptPrefix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    StridedArrayView1Di e = a.exceptSuffix(2);
    CORRADE_COMPARE(e.size(), 3);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);
}

void StridedArrayViewTest::sliceInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView1Di a = data;

    Containers::String out;
    Error redirectError{&out};

    a.slice(5, 6);
    a.slice(2, 1);

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StridedArrayView::slice(): slice [2:1] out of range for 5 elements\n");
}

void StridedArrayViewTest::slice3D() {
    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    StridedArrayView3Di b1 = a.slice({0, 1, 1}, {1, 2, 3});
    CORRADE_COMPARE(b1.size(), (Size3D{1, 1, 2}));
    CORRADE_COMPARE(b1.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b1[0][0][0], 234810);
    CORRADE_COMPARE(b1[0][0][1], 232342);

    StridedArrayView3Di b2 = a.sliceSize({0, 1, 1}, {1, 1, 2});
    CORRADE_COMPARE(b2.size(), (Size3D{1, 1, 2}));
    CORRADE_COMPARE(b2.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b2[0][0][0], 234810);
    CORRADE_COMPARE(b2[0][0][1], 232342);

    StridedArrayView3Di c = a.prefix({1, 1, 3});
    CORRADE_COMPARE(c.size(), (Size3D{1, 1, 3}));
    CORRADE_COMPARE(c.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c[0][0][0], 2);
    CORRADE_COMPARE(c[0][0][1], 16);
    CORRADE_COMPARE(c[0][0][2], 7853268);

    StridedArrayView3Di d = a.exceptPrefix({0, 1, 2});
    CORRADE_COMPARE(d.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(d.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(d[0][0][0], 232342);
    CORRADE_COMPARE(d[1][0][0], 0);

    StridedArrayView3Di e = a.exceptSuffix({1, 1, 0});
    CORRADE_COMPARE(e.size(), (Size3D{1, 1, 3}));
    CORRADE_COMPARE(e.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(e[0][0][0], 2);
    CORRADE_COMPARE(e[0][0][1], 16);
    CORRADE_COMPARE(e[0][0][2], 7853268);
}

void StridedArrayViewTest::slice3DInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    Containers::String out;
    Error redirectError{&out};

    a.slice({1, 0, 1}, {2, 4, 3});
    a.slice({2, 0, 1}, {0, 4, 3});

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): slice [{1, 0, 1}:{2, 4, 3}] out of range for {2, 2, 3} elements in dimension 1\n"
        "Containers::StridedArrayView::slice(): slice [{2, 0, 1}:{0, 4, 3}] out of range for {2, 2, 3} elements in dimension 0\n");
}

void StridedArrayViewTest::slice3DFirstDimension() {
    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{23, 0}, {76, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    StridedArrayView3Di b1 = a.slice(1, 2);
    CORRADE_COMPARE(b1.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b1.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b1[0][0][0], 23);
    CORRADE_COMPARE(b1[0][0][1], 76);

    StridedArrayView3Di b2 = a.sliceSize(1, 1);
    CORRADE_COMPARE(b2.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b2.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b2[0][0][0], 23);
    CORRADE_COMPARE(b2[0][0][1], 76);

    StridedArrayView3Di c = a.prefix(1);
    CORRADE_COMPARE(c.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(c.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c[0][0][0], 2);
    CORRADE_COMPARE(c[0][0][1], 16);
    CORRADE_COMPARE(c[0][0][2], 7853268);

    StridedArrayView3Di d = a.exceptPrefix(1);
    CORRADE_COMPARE(d.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(d.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(d[0][0][0], 23);
    CORRADE_COMPARE(d[0][0][1], 76);
    CORRADE_COMPARE(d[0][0][2], 0);

    StridedArrayView3Di e = a.exceptSuffix(1);
    CORRADE_COMPARE(e.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(e.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(e[0][0][0], 2);
    CORRADE_COMPARE(e[0][0][1], 16);
    CORRADE_COMPARE(e[0][0][2], 7853268);
}

void StridedArrayViewTest::slice3DFirstDimensionInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView3Di a = {data, {5, 1, 1}};

    Containers::String out;
    Error redirectError{&out};

    a.slice(5, 6);
    a.slice(2, 1);

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StridedArrayView::slice(): slice [2:1] out of range for 5 elements\n");
}

void StridedArrayViewTest::sliceDimensionUp() {
    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView1Di a = data;

    StridedArrayView3Di b = a.slice<3>(1, 4);
    CORRADE_COMPARE(b.size(), (Size3D{3, 1, 1}));
    CORRADE_COMPARE(b.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(b[0][0][0], 2);
    CORRADE_COMPARE(b[1][0][0], 3);
    CORRADE_COMPARE(b[2][0][0], 4);

    StridedArrayView3Di c = a.slice<3>();
    CORRADE_COMPARE(c.size(), (Size3D{5, 1, 1}));
    CORRADE_COMPARE(c.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(c[0][0][0], 1);
    CORRADE_COMPARE(c[1][0][0], 2);
    CORRADE_COMPARE(c[2][0][0], 3);

    StridedArrayView3Di d = a.prefix<3>(2);
    CORRADE_COMPARE(d.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(d.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(d[0][0][0], 1);
    CORRADE_COMPARE(d[1][0][0], 2);

    StridedArrayView3Di e = a.exceptPrefix<3>(3);
    CORRADE_COMPARE(e.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(e.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(e[0][0][0], 4);
    CORRADE_COMPARE(e[1][0][0], 5);

    StridedArrayView3Di f = a.exceptSuffix<3>(3);
    CORRADE_COMPARE(f.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(f.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(f[0][0][0], 1);
    CORRADE_COMPARE(f[1][0][0], 2);
}

void StridedArrayViewTest::sliceDimensionUpInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView1Di a = data;

    Containers::String out;
    Error redirectError{&out};

    a.slice<3>(5, 6);
    a.slice<3>(1, 0);

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): slice [{5}:{6}] out of range for {5} elements in dimension 0\n"
        "Containers::StridedArrayView::slice(): slice [{1}:{0}] out of range for {5} elements in dimension 0\n");
}

void StridedArrayViewTest::sliceDimensionDown() {
    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    StridedArrayView2Di b = a.slice<2>({0, 1, 1}, {2, 2, 3});
    CORRADE_COMPARE(b.size(), (Size2D{2, 1}));
    CORRADE_COMPARE(b.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(b[0][0], 234810);
    CORRADE_COMPARE(b[1][0], 0);

    StridedArrayView2Di c = a.slice<2>();
    CORRADE_COMPARE(c.size(), (Size2D{2, 2}));
    CORRADE_COMPARE(c.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(c[0][0], 2);
    CORRADE_COMPARE(c[1][0], 0);

    StridedArrayView2Di d = a.prefix<2>({1, 2, 3});
    CORRADE_COMPARE(d.size(), (Size2D{1, 2}));
    CORRADE_COMPARE(d.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(d[0][0], 2);
    CORRADE_COMPARE(d[0][1], -100);

    StridedArrayView2Di e = a.exceptPrefix<2>({0, 1, 2});
    CORRADE_COMPARE(e.size(), (Size2D{2, 1}));
    CORRADE_COMPARE(e.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(e[0][0], 232342);
    CORRADE_COMPARE(e[1][0], 0);

    StridedArrayView2Di f = a.exceptSuffix<2>({1, 0, 0});
    CORRADE_COMPARE(f.size(), (Size2D{1, 2}));
    CORRADE_COMPARE(f.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(f[0][0], 2);
    CORRADE_COMPARE(f[0][1], -100);
}

void StridedArrayViewTest::sliceDimensionDownInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    Containers::String out;
    Error redirectError{&out};

    a.slice<2>({0, 1, 4}, {1, 2, 5});
    a.slice<1>({0, 1, 0}, {1, 0, 1});

    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): slice [{0, 1, 4}:{1, 2, 5}] out of range for {2, 2, 3} elements in dimension 2\n"
        "Containers::StridedArrayView::slice(): slice [{0, 1, 0}:{1, 0, 1}] out of range for {2, 2, 3} elements in dimension 1\n");
}

void StridedArrayViewTest::sliceMemberPointer() {
    struct Data {
        float first;
        short second;
        char third;
    };

    Data data[]{
        {1.5f, 3, 'a'},
        {-0.5f, 11, '7'}
    };
    Containers::StridedArrayView1D<Data> view = data;

    Containers::StridedArrayView1D<float> first = view.slice(&Data::first);
    CORRADE_COMPARE(first.data(), &view[0].first);
    CORRADE_COMPARE(first.size(), 2);
    CORRADE_COMPARE(first.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(first,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);

    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    CORRADE_COMPARE(second.data(), &view[0].second);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    Containers::StridedArrayView1D<char> third = view.slice(&Data::third);
    CORRADE_COMPARE(third.data(), &view[0].third);
    CORRADE_COMPARE(third.size(), 2);
    CORRADE_COMPARE(third.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(third,
        Containers::stridedArrayView<char>({'a', '7'}),
        TestSuite::Compare::Container);

    /* Should work for multiple dimensions as well */
    Containers::StridedArrayView2D<Data> view2D{data, {1, 2}};
    Containers::StridedArrayView2D<short> second2D = view2D.slice(&Data::second);
    CORRADE_COMPARE(second2D.data(), &view2D[0][0].second);
    CORRADE_COMPARE(second2D.size(), (Containers::Size2D{1, 2}));
    CORRADE_COMPARE(second2D.stride(), (Containers::Stride2D{sizeof(Data)*2, sizeof(Data)}));
    CORRADE_COMPARE_AS(second2D[0],
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberPointerConstData() {
    struct Data {
        float first;
        short second;
    };

    const Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<const Data> view = data;

    /* Here the member pointer is not const but the original view is, so it
       should correctly add const where needed */
    auto second = view.slice(&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), &view[0].second);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceConstMemberPointer() {
    struct Data {
        float first;
        const short second;
    };

    Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<Data> view = data;

    /* Here the member pointer is not const but the original view is, so it
       should correctly add const where needed */
    auto second = view.slice(&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), &view[0].second);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberPointerDerived() {
    struct Data {
        int nothing;
        float a;
    };

    struct Derived: Data {
        /* C++, why the F you need this?! */
        /*implicit*/ Derived(int nothing, float a): Data{nothing, a} {}
    };

    const Derived data[]{
        {0, 1.5f},
        {0, -0.5f}
    };
    Containers::StridedArrayView1D<const Derived> view = data;

    /* Here the member pointer is actually &Data::a in both cases, and both
       should work the same */
    auto floats1 = view.slice(&Data::a);
    auto floats2 = view.slice(&Derived::a);
    CORRADE_VERIFY(std::is_same<decltype(floats1), Containers::StridedArrayView1D<const float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(floats2), Containers::StridedArrayView1D<const float>>::value);
    CORRADE_COMPARE(floats1.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(floats2.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(floats1.size(), 2);
    CORRADE_COMPARE(floats2.size(), 2);
    CORRADE_COMPARE(floats1.stride(), sizeof(Derived));
    CORRADE_COMPARE(floats2.stride(), sizeof(Derived));
    CORRADE_COMPARE_AS(floats1,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(floats2,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberPointerNullView() {
    struct Data {
        float first;
        short second;
    };
    Containers::StridedArrayView1D<Data> view{nullptr, 5};
    Containers::StridedArrayView1D<const Data> cview{nullptr, 5};

    /* Slicing a null view should still return a null pointer even if the view
       has non-zero size */
    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    Containers::StridedArrayView1D<const short> csecond = cview.slice(&Data::second);
    CORRADE_COMPARE(second.data(), nullptr);
    CORRADE_COMPARE(csecond.data(), nullptr);
    CORRADE_COMPARE(second.size(), 5);
    CORRADE_COMPARE(csecond.size(), 5);
    CORRADE_COMPARE(second.stride(), 8);
    CORRADE_COMPARE(csecond.stride(), 8);
}

void StridedArrayViewTest::sliceMemberPointerEmptyView() {
    struct Data {
        float first;
        short second;
    } data[1]{};
    Containers::StridedArrayView1D<Data> view{data, 0};
    Containers::StridedArrayView1D<const Data> cview{data, 0};

    /* Compared to above, slicing a zero-sized non-null view should apply the
       offset */
    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    Containers::StridedArrayView1D<const short> csecond = cview.slice(&Data::second);
    CORRADE_COMPARE(second.data(), &data->second);
    CORRADE_COMPARE(csecond.data(), &data->second);
    CORRADE_COMPARE(second.size(), 0);
    CORRADE_COMPARE(csecond.size(), 0);
    CORRADE_COMPARE(second.stride(), 8);
    CORRADE_COMPARE(csecond.stride(), 8);
}

void StridedArrayViewTest::sliceMemberFunctionPointer() {
    class Data {
        public:
            /*implicit*/ Data(float first, short second, char third): _first{first}, _second{second}, _third{third} {}

            float& first() { return _first; }
            short& second() { return _second; }
            char& third() { return _third; }
        private:
            float _first;
            short _second;
            /* so the last member is at sizeof(T) - 1, to test the assert */
            char:8;
            char _third;
    };

    /* The rest is identical to sliceMemberPointer(), except that the member
       pointers are actually functions */
    Data data[]{
        {1.5f, 3, 'a'},
        {-0.5f, 11, '7'}
    };
    Containers::StridedArrayView1D<Data> view = data;

    Containers::StridedArrayView1D<float> first = view.slice(&Data::first);
    CORRADE_COMPARE(first.data(), reinterpret_cast<char*>(data) + 0);
    CORRADE_COMPARE(first.size(), 2);
    CORRADE_COMPARE(first.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(first,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);

    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    CORRADE_COMPARE(second.data(), reinterpret_cast<char*>(data) + 4);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    Containers::StridedArrayView1D<char> third = view.slice(&Data::third);
    CORRADE_COMPARE(third.data(), reinterpret_cast<char*>(data) + 7);
    CORRADE_COMPARE(third.size(), 2);
    CORRADE_COMPARE(third.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(third,
        Containers::stridedArrayView<char>({'a', '7'}),
        TestSuite::Compare::Container);

    /* Should work for multiple dimensions as well */
    Containers::StridedArrayView2D<Data> view2D{data, {1, 2}};
    Containers::StridedArrayView2D<short> second2D = view2D.slice(&Data::second);
    CORRADE_COMPARE(second2D.data(), reinterpret_cast<char*>(data) + 4);
    CORRADE_COMPARE(second2D.size(), (Containers::Size2D{1, 2}));
    CORRADE_COMPARE(second2D.stride(), (Containers::Stride2D{sizeof(Data)*2, sizeof(Data)}));
    CORRADE_COMPARE_AS(second2D[0],
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberFunctionPointerConstData() {
    class Data {
        public:
            /*implicit*/ Data(float first, short second): _first{first}, _second{second} {}

            /* Clang complains this function is unused, but without it the
               _first member would be unused as well, and that one is vital to
               test proper offset calculation, so leave it. */
            CORRADE_UNUSED const float& first() const { return _first; }
            const short& second() const { return _second; }
        private:
            float _first;
            short _second;
    };

    const Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<const Data> view = data;

    /* Here the member pointer is not const but the original view is, so it
       should correctly add const where needed */
    auto second = view.slice(&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberFunctionPointerReturningConst() {
    class Data {
        public:
            /*implicit*/ Data(float first, short second): _first{first}, _second{second} {}

            /* Clang complains this function is unused, but without it the
               _first member would be unused as well, and that one is vital to
               test proper offset calculation, so leave it. */
            CORRADE_UNUSED const float& first() { return _first; }
            const short& second() { return _second; }
        private:
            float _first;
            short _second;
    };

    Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<Data> view = data;

    /* Here the member pointer is not const but the original view is, so it
       should correctly add const where needed */
    auto second = view.slice(&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceConstOverloadedMemberFunctionPointer() {
    class Data {
        public:
            /*implicit*/ Data(float first, short second): _first{first}, _second{second} {}

            short& second() { return _second; }
            const short& second() const { return _second; }

            float& firstNonOverloaded() { return _first; }
            const short& secondNonOverloaded() const { return _second; }
        private:
            float _first;
            short _second;
    };

    Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<Data> view = data;
    Containers::StridedArrayView1D<const Data> cview = data;

    /* It should pick the non-const overload for mutable view and const for
       const view, without being ambiguous */
    auto second = view.slice(&Data::second);
    auto csecond = cview.slice(&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<short>>::value);
    CORRADE_VERIFY(std::is_same<decltype(csecond), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(csecond.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(csecond.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE(csecond.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(csecond,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    /* Check that all const combinations of non-overloaded functions work as
       well, not just mutable functions on mutable types and const functions on
       const types */
    auto firstNonOverloaded = view.slice(&Data::firstNonOverloaded);
    auto secondNonOverloaded = view.slice(&Data::secondNonOverloaded);
    auto csecondNonOverloaded = cview.slice(&Data::secondNonOverloaded);
    CORRADE_VERIFY(std::is_same<decltype(firstNonOverloaded), Containers::StridedArrayView1D<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(secondNonOverloaded), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_VERIFY(std::is_same<decltype(csecondNonOverloaded), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(firstNonOverloaded.data(), reinterpret_cast<const char*>(data));
    CORRADE_COMPARE(secondNonOverloaded.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(csecondNonOverloaded.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(firstNonOverloaded.size(), 2);
    CORRADE_COMPARE(secondNonOverloaded.size(), 2);
    CORRADE_COMPARE(csecondNonOverloaded.size(), 2);
    CORRADE_COMPARE(firstNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE(secondNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE(csecondNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(firstNonOverloaded,
        Containers::stridedArrayView<float>({1.5f, -0.5f}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(secondNonOverloaded,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(csecondNonOverloaded,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    /* This shouldn't even compile since it's asking for a mutable function on
       a const type. And it's fine to be a hard error, but should be a "no
       matching overload" rather than a compile error somewhere inside */
    #if 0
    cview.slice(&Data::firstNonOverloaded);
    #endif
}

void StridedArrayViewTest::sliceRvalueOverloadedMemberFunctionPointer() {
    /* This is the case with Pointer or Triple */

    class Data {
        public:
            /*implicit*/ Data(float first, short second): _first{first}, _second{second} {}

            short& second() & { return _second; }
            /* Clang complains this function is unused. But its presence is
               vital to the test case, so leave it. */
            CORRADE_UNUSED short&& second() && { return Utility::move(_second); }
            const short& second() const & { return _second; }
            /* Clang complains this function is unused. But its presence is
               vital to the test case, so leave it. */
            CORRADE_UNUSED const short&& second() const && { return Utility::move(_second); }

            float& firstNonOverloaded() & { return _first; }
            const short& secondNonOverloaded() const & { return _second; }
        private:
            float _first;
            short _second;
    };

    Data data[]{
        {1.5f, 3},
        {-0.5f, 11}
    };
    Containers::StridedArrayView1D<Data> view = data;
    Containers::StridedArrayView1D<const Data> cview = data;

    #if (defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 7) || defined(CORRADE_MSVC2015_COMPATIBILITY)
    CORRADE_WARN("GCC 4.8, 4.9, 5, 6 and MSVC 2015 need an explicit template parameter to disambiguate for rvalue overloads");
    #endif
    /* It should prefer the & overload and ignore the && */
    auto second = view.slice
        #if (defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 7) || defined(CORRADE_MSVC2015_COMPATIBILITY)
        <short>
        #endif
        (&Data::second);
    auto csecond = cview.slice
        #if (defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 7) || defined(CORRADE_MSVC2015_COMPATIBILITY)
        <short>
        #endif
        (&Data::second);
    CORRADE_VERIFY(std::is_same<decltype(second), Containers::StridedArrayView1D<short>>::value);
    CORRADE_VERIFY(std::is_same<decltype(csecond), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(second.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(csecond.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(second.size(), 2);
    CORRADE_COMPARE(csecond.size(), 2);
    CORRADE_COMPARE(second.stride(), sizeof(Data));
    CORRADE_COMPARE(csecond.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(second,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(csecond,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    /* Check that all const combinations of non-overloaded functions work as
       well, not just mutable functions on mutable types and const functions on
       const types */
    auto firstNonOverloaded = view.slice(&Data::firstNonOverloaded);
    auto secondNonOverloaded = view.slice(&Data::secondNonOverloaded);
    auto csecondNonOverloaded = cview.slice(&Data::secondNonOverloaded);
    CORRADE_VERIFY(std::is_same<decltype(firstNonOverloaded), Containers::StridedArrayView1D<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(secondNonOverloaded), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_VERIFY(std::is_same<decltype(csecondNonOverloaded), Containers::StridedArrayView1D<const short>>::value);
    CORRADE_COMPARE(firstNonOverloaded.data(), reinterpret_cast<const char*>(data));
    CORRADE_COMPARE(secondNonOverloaded.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(csecondNonOverloaded.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(firstNonOverloaded.size(), 2);
    CORRADE_COMPARE(secondNonOverloaded.size(), 2);
    CORRADE_COMPARE(csecondNonOverloaded.size(), 2);
    CORRADE_COMPARE(firstNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE(secondNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE(csecondNonOverloaded.stride(), sizeof(Data));
    CORRADE_COMPARE_AS(firstNonOverloaded,
        Containers::stridedArrayView<float>({1.5f, -0.5f}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(secondNonOverloaded,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(csecondNonOverloaded,
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);

    /* This shouldn't even compile since it's asking for a mutable function on
       a const type. And it's fine to be a hard error, but should be a "no
       matching overload" rather than a compile error somewhere inside */
    #if 0
    cview.slice(&Data::firstNonOverloaded);
    #endif
}

void StridedArrayViewTest::sliceMemberFunctionPointerDerived() {
    /* Like sliceMemberPointerDerived(), just with functions instead */

    class Data {
        public:
            /*implicit*/ Data(float a): _a{a} {}
            const float& a() const { return _a; }
            int nothing;
        private:
            float _a;
    };

    struct Derived: Data {
        using Data::Data;
    };

    const Derived data[]{
        {1.5f},
        {-0.5f}
    };
    Containers::StridedArrayView1D<const Derived> view = data;

    /* Here the member pointer is actually &Data::a in both cases, and both
       should work the same */
    auto floats1 = view.slice(&Data::a);
    auto floats2 = view.slice(&Derived::a);
    CORRADE_VERIFY(std::is_same<decltype(floats1), Containers::StridedArrayView1D<const float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(floats2), Containers::StridedArrayView1D<const float>>::value);
    CORRADE_COMPARE(floats1.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(floats2.data(), reinterpret_cast<const char*>(data) + 4);
    CORRADE_COMPARE(floats1.size(), 2);
    CORRADE_COMPARE(floats2.size(), 2);
    CORRADE_COMPARE(floats1.stride(), sizeof(Derived));
    CORRADE_COMPARE(floats2.stride(), sizeof(Derived));
    CORRADE_COMPARE_AS(floats1,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(floats2,
        Containers::stridedArrayView({1.5f, -0.5f}),
        TestSuite::Compare::Container);
}

void StridedArrayViewTest::sliceMemberFunctionPointerNullView() {
    class Data {
        public:
            /* The getters should *not* be called in this case at all */
            short& second() {
                CORRADE_EXPECT_FAIL("This shouldn't be called.");
                return _data[1];
            }
            const short& second() const {
                CORRADE_EXPECT_FAIL("This shouldn't be called.");
                return _data[1];
            }
            short& secondLvalue() & {
                CORRADE_EXPECT_FAIL("This shouldn't be called.");
                return _data[1];
            }
            const short& secondLvalue() const & {
                CORRADE_EXPECT_FAIL("This shouldn't be called.");
                return _data[1];
            }
            short& secondNonOverloaded() {
                CORRADE_EXPECT_FAIL("This shouldn't be called.");
                return _data[1];
            }
        private:
            short _data[2];
    };
    Containers::StridedArrayView1D<Data> view{nullptr, 5};
    Containers::StridedArrayView1D<const Data> cview{nullptr, 5};

    /* To capture a correct test case name */
    CORRADE_VERIFY(true);

    /* Slicing a null view should still return a null pointer even if the view
       has non-zero size */
    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    Containers::StridedArrayView1D<short> secondLvalue = view.slice(&Data::secondLvalue);
    Containers::StridedArrayView1D<const short> csecond = cview.slice(&Data::second);
    Containers::StridedArrayView1D<const short> csecondLvalue = cview.slice(&Data::secondLvalue);
    Containers::StridedArrayView1D<short> secondNonOverloaded = view.slice(&Data::secondNonOverloaded);
    CORRADE_COMPARE(second.data(), nullptr);
    CORRADE_COMPARE(secondLvalue.data(), nullptr);
    CORRADE_COMPARE(csecond.data(), nullptr);
    CORRADE_COMPARE(csecondLvalue.data(), nullptr);
    CORRADE_COMPARE(secondNonOverloaded.data(), nullptr);

    CORRADE_COMPARE(second.size(), 5);
    CORRADE_COMPARE(secondLvalue.size(), 5);
    CORRADE_COMPARE(csecond.size(), 5);
    CORRADE_COMPARE(csecondLvalue.size(), 5);
    CORRADE_COMPARE(secondNonOverloaded.size(), 5);

    CORRADE_COMPARE(second.stride(), 4);
    CORRADE_COMPARE(secondLvalue.stride(), 4);
    CORRADE_COMPARE(csecond.stride(), 4);
    CORRADE_COMPARE(csecondLvalue.stride(), 4);
    CORRADE_COMPARE(secondNonOverloaded.stride(), 4);
}

void StridedArrayViewTest::sliceMemberFunctionPointerEmptyView() {
    static void* thisNotExpected;

    struct Data {
        /* Even though the view is non-null, the getters should *not* be called
           on this pointer pointing to outside of the view */
        short& second() {
            CORRADE_VERIFY(this != thisNotExpected);
            return data[1];
        }
        const short& second() const {
            CORRADE_VERIFY(this != thisNotExpected);
            return data[1];
        }
        short& secondLvalue() & {
            CORRADE_VERIFY(this != thisNotExpected);
            return data[1];
        }
        const short& secondLvalue() const & {
            CORRADE_VERIFY(this != thisNotExpected);
            return data[1];
        }
        short& secondNonOverloaded() {
            CORRADE_VERIFY(this != thisNotExpected);
            return data[1];
        }

        /* Ugh. GCC 14 (and possibly other versions) in Release warn that
           "array subscript ‘int (**)(...)[0]’ is partly outside array bounds
           of ‘std::conditional<false, const char, char>::type [6]’ {aka
           ‘char [6]’}" if this has a size less than a pointer. POINTER. No
           idea where that comes from, since i'm calling a MEMBER FUNCTION
           POINTER on an char[sizeof(Data)] array. Making the array as large as
           a pointer "fixes" the warning. */
        short data[4];
    } data[1]{};
    thisNotExpected = data;
    Containers::StridedArrayView1D<Data> view{data, 0};
    Containers::StridedArrayView1D<const Data> cview{data, 0};

    /* To capture a correct test case name */
    CORRADE_VERIFY(true);

    /* Compared to above, slicing a zero-sized non-null view should apply the
       offset */
    Containers::StridedArrayView1D<short> second = view.slice(&Data::second);
    Containers::StridedArrayView1D<short> secondLvalue = view.slice(&Data::secondLvalue);
    Containers::StridedArrayView1D<const short> csecond = cview.slice(&Data::second);
    Containers::StridedArrayView1D<const short> csecondLvalue = cview.slice(&Data::secondLvalue);
    Containers::StridedArrayView1D<short> secondNonOverloaded = view.slice(&Data::secondNonOverloaded);
    CORRADE_COMPARE(second.data(), &data->data[1]);
    CORRADE_COMPARE(secondLvalue.data(), &data->data[1]);
    CORRADE_COMPARE(csecond.data(), &data->data[1]);
    CORRADE_COMPARE(csecondLvalue.data(), &data->data[1]);
    CORRADE_COMPARE(secondNonOverloaded.data(), &data->data[1]);

    CORRADE_COMPARE(second.size(), 0);
    CORRADE_COMPARE(secondLvalue.size(), 0);
    CORRADE_COMPARE(csecond.size(), 0);
    CORRADE_COMPARE(csecondLvalue.size(), 0);
    CORRADE_COMPARE(secondNonOverloaded.size(), 0);

    CORRADE_COMPARE(second.stride(), 8);
    CORRADE_COMPARE(secondLvalue.stride(), 8);
    CORRADE_COMPARE(csecond.stride(), 8);
    CORRADE_COMPARE(csecondLvalue.stride(), 8);
    CORRADE_COMPARE(secondNonOverloaded.stride(), 8);
}

void StridedArrayViewTest::sliceMemberFunctionPointerArrayType() {
    /* MSVC 2015 to 2022 (and likely any future version as well) ICEs when
       trying to call a member function pointer with an array return type. The
       only workaround is reinterpret_cast'ing the pointer to a
       non-array-reference return type. */

    struct Data {
        public:
            /*implicit*/ Data(float a, float b, float c, float d, float e): floats{a, b, c, d, e} {}

            auto data() -> float(&)[5] { return floats; }
            auto dataLvalue() & -> float(&)[5] { return floats; }
            auto dataConst() const -> const float(&)[5] { return floats; }
            auto dataConstLvalue() const & -> const float(&)[5] { return floats; }

            double something;
            int other;
            float floats[5];
    };

    Data data[]{
        {1.5f, 0.3f, 3.14f, 2.7f, 0.1f},
        {2.3f, -7.6f, 0.2f, -1.1f, 0.0f}
    };
    Containers::StridedArrayView1D<Data> view = data;

    /* Verify that all four variants work and return a correct offset */
    int i = 0;
    for(Containers::StridedArrayView1D<float[5]> view5: {
        view.slice(&Data::data),
        view.slice(&Data::dataLvalue)
    }) {
        CORRADE_ITERATION(i++);
        CORRADE_COMPARE(view5.data(), reinterpret_cast<const char*>(data) + 12);
        CORRADE_COMPARE(view5.size(), 2);
        CORRADE_COMPARE(view5.stride(), sizeof(Data));
        CORRADE_COMPARE(view5[0][1], 0.3f);
        CORRADE_COMPARE(view5[1][3], -1.1f);
    }

    int j = 0;
    for(Containers::StridedArrayView1D<const float[5]> view5: {
        view.slice(&Data::dataConst),
        view.slice(&Data::dataConstLvalue)
    }) {
        CORRADE_ITERATION(j++);
        CORRADE_COMPARE(view5.data(), reinterpret_cast<const char*>(data) + 12);
        CORRADE_COMPARE(view5.size(), 2);
        CORRADE_COMPARE(view5.stride(), sizeof(Data));
        CORRADE_COMPARE(view5[0][1], 0.3f);
        CORRADE_COMPARE(view5[1][3], -1.1f);
    }

    /* Verify that direct member slice "just works" without crashing or
       workarounds */
    Containers::StridedArrayView1D<float[5]> viewDirect = view.slice(&Data::floats);
    CORRADE_COMPARE(viewDirect.data(), reinterpret_cast<const char*>(data) + 12);
    CORRADE_COMPARE(viewDirect.size(), 2);
    CORRADE_COMPARE(viewDirect.stride(), sizeof(Data));
    CORRADE_COMPARE(viewDirect[0][1], 0.3f);
    CORRADE_COMPARE(viewDirect[1][3], -1.1f);
}

void StridedArrayViewTest::sliceMemberFunctionPointerReturningOffsetOutOfRange() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct Data {
        public:
            /* UB, hello! */
            float& first() { return *(&_first + 1); }
            const float& first() const { return *(&_first - 1); }
            const float& firstNonOverloaded() const { return *(&_first - 1); }
        private:
            float _first;
    };

    Data data[1];
    Containers::StridedArrayView1D<Data> view = data;
    Containers::StridedArrayView1D<const Data> cview = data;

    Containers::String out;
    Error redirectError{&out};
    view.slice(&Data::first);
    cview.slice(&Data::first);
    /* Just to verify the fifth overload calls to the same helper as all
       others */
    view.slice(&Data::firstNonOverloaded);
    cview.slice(&Data::firstNonOverloaded);
    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::slice(): member function slice returned offset 4 for a 4-byte type\n"
        "Containers::StridedArrayView::slice(): member function slice returned offset -4 for a 4-byte type\n"
        "Containers::StridedArrayView::slice(): member function slice returned offset -4 for a 4-byte type\n"
        "Containers::StridedArrayView::slice(): member function slice returned offset -4 for a 4-byte type\n");
}

void StridedArrayViewTest::sliceBit() {
    const bool bools[]{true, false, true};
    Containers::StridedArrayView1D<const bool> boolsView = bools;
    StridedBitArrayView1D bits = boolsView.sliceBit(0);
    CORRADE_COMPARE(bits.data(), bools);
    CORRADE_COMPARE(bits.offset(), 0);
    CORRADE_COMPARE(bits.size(), 3);
    CORRADE_COMPARE(bits.stride(), 8);
    CORRADE_VERIFY( bits[0]);
    CORRADE_VERIFY(!bits[1]);
    CORRADE_VERIFY( bits[2]);

    /* More dimensions, mutable */
    float floats[]{
        0.0f, -13.0f, 2.0f,
        -0.1f, 1.0f, -0.0f
    };
    Containers::StridedArrayView2D<float> floatsView{floats, {2, 3}};
    /** @todo possibly should be bit 7 on BE instead? */
    StridedBitArrayView2D signs = floatsView.sliceBit(31);
    CORRADE_COMPARE(signs.data(), reinterpret_cast<char*>(floats) + 3);
    CORRADE_COMPARE(signs.offset(), 7);
    CORRADE_COMPARE(signs.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(signs.stride(), (Stride2D{12*8, 4*8}));
    CORRADE_VERIFY(!signs[0][0]);
    CORRADE_VERIFY( signs[0][1]);
    CORRADE_VERIFY(!signs[0][2]);
    CORRADE_VERIFY( signs[1][0]);
    CORRADE_VERIFY(!signs[1][1]);
    CORRADE_VERIFY( signs[1][2]);
}

void StridedArrayViewTest::sliceBitIndexTooLarge() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::StridedArrayView3D<float> view;

    Containers::String out;
    Error redirectError{&out};
    view.sliceBit(32);
    CORRADE_COMPARE(out, "Containers::StridedArrayView::sliceBit(): index 32 out of range for a 32-bit type\n");
}

void StridedArrayViewTest::sliceBitSizeTooLarge() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::StridedArrayView3D<bool> view{nullptr, {1, std::size_t{1} << (sizeof(std::size_t)*8 - 3), 1}, {0, 0, 0}};

    Containers::String out;
    Error redirectError{&out};
    view.sliceBit(0);
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out, "Containers::StridedArrayView::sliceBit(): size expected to be smaller than 2^61 bits, got {1, 2305843009213693952, 1}\n");
    #else
    CORRADE_COMPARE(out, "Containers::StridedArrayView::sliceBit(): size expected to be smaller than 2^29 bits, got {1, 536870912, 1}\n");
    #endif
}

void StridedArrayViewTest::every() {
    int data[]{0, 1, 2, 3, 4, 5, 6, 7};
    StridedArrayView1Di a = data;

    StridedArrayView1Di b = a.every(1);
    CORRADE_COMPARE(b.size(), 8);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[0], 0);
    CORRADE_COMPARE(b[1], 1);
    CORRADE_COMPARE(b[2], 2);

    StridedArrayView1Di c = a.every(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c.stride(), 12);
    CORRADE_COMPARE(c[0], 0);
    CORRADE_COMPARE(c[1], 3);
    CORRADE_COMPARE(c[2], 6);

    CORRADE_COMPARE(a.every(7).size(), 2);
    CORRADE_COMPARE(a.every(10).size(), 1);
}

void StridedArrayViewTest::everyNegative() {
    int data[]{0, 1, 2, 3, 4, 5, 6, 7};
    StridedArrayView1Di a = data;

    StridedArrayView1Di b = a.every(-1);
    CORRADE_COMPARE(b.size(), 8);
    CORRADE_COMPARE(b.stride(), -4);
    CORRADE_COMPARE(b[0], 7);
    CORRADE_COMPARE(b[1], 6);
    CORRADE_COMPARE(b[2], 5);

    StridedArrayView1Di c = a.every(-3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c.stride(), -12);
    CORRADE_COMPARE(c[0], 7);
    CORRADE_COMPARE(c[1], 4);
    CORRADE_COMPARE(c[2], 1);

    StridedArrayView1Di d = a.every(-7);
    CORRADE_COMPARE(d.size(), 2);
    CORRADE_COMPARE(d.stride(), -28);
    CORRADE_COMPARE(d[0], 7);

    StridedArrayView1Di e = a.every(-10);
    CORRADE_COMPARE(e.size(), 1);
    CORRADE_COMPARE(e.stride(), -40);
    CORRADE_COMPARE(e[0], 7);
}

void StridedArrayViewTest::everyNegativeZeroSize() {
    int data[1]{};
    StridedArrayView1Di a{data, 0, 8};

    /* Should not result in any difference in the data pointer -- especially
       not any overflowing values */
    StridedArrayView1Di b = a.every(-1);
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(b.stride(), -8);
}

void StridedArrayViewTest::everyInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView1Di{}.every(0);
    CORRADE_COMPARE(out, "Containers::StridedArrayView::every(): expected a non-zero step, got {0}\n");
}

void StridedArrayViewTest::every2D() {
    int data[]{0, 1, 2, 3, 4, 5, 6, 7,
               4, 5, 6, 7, 8, 9, 10, 11,
               8, 9, 10, 11, 12, 13, 14, 15};
    StridedArrayView2Di a{data, {3, 8}, {32, 4}};

    StridedArrayView2Di b = a.every({2, 3});
    CORRADE_COMPARE(b.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride2D{64, 12}));
    CORRADE_COMPARE(b[0][0], 0);
    CORRADE_COMPARE(b[0][1], 3);
    CORRADE_COMPARE(b[0][2], 6);
    CORRADE_COMPARE(b[1][0], 8);
    CORRADE_COMPARE(b[1][1], 11);
    CORRADE_COMPARE(b[1][2], 14);
}

void StridedArrayViewTest::every2DNegative() {
    int data[]{0, 1, 2, 3, 4, 5, 6, 7,
               4, 5, 6, 7, 8, 9, 10, 11,
               8, 9, 10, 11, 12, 13, 14, 15};
    StridedArrayView2Di a{data, {3, 8}, {32, 4}};

    StridedArrayView2Di b = a.every({-2, -3});
    CORRADE_COMPARE(b.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride2D{-64, -12}));
    CORRADE_COMPARE(b[0][0], 15);
    CORRADE_COMPARE(b[0][1], 12);
    CORRADE_COMPARE(b[0][2], 9);
    CORRADE_COMPARE(b[1][0], 7);
    CORRADE_COMPARE(b[1][1], 4);
    CORRADE_COMPARE(b[1][2], 1);
}

void StridedArrayViewTest::every2DNegativeZeroSize() {
    /* Same as every2DNegative() above, except that the second dimension size
       is 0 */
    int data[1]{};
    StridedArrayView2Di a{data, {3, 0}, {32, 4}};

    /* Should not result in any difference in the data pointer -- especially
       not any overflowing values */
    StridedArrayView2Di b = a.every({2, -3});
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), (Size2D{2, 0}));
    CORRADE_COMPARE(b.stride(), (Stride2D{64, -12}));
}

void StridedArrayViewTest::every2DInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    StridedArrayView2Di{}.every({3, 0});
    CORRADE_COMPARE(out, "Containers::StridedArrayView::every(): expected a non-zero step, got {3, 0}\n");
}

void StridedArrayViewTest::every2DFirstDimension() {
    int data[]{0, 1, 2, 3, 4, 5, 6, 7,
               4, 5, 6, 7, 8, 9, 10, 11,
               8, 9, 10, 11, 12, 13, 14, 15};
    StridedArrayView2Di a{data, {3, 8}, {32, 4}};

    StridedArrayView2Di b = a.every(2);
    CORRADE_COMPARE(b.size(), (Size2D{2, 8}));
    CORRADE_COMPARE(b.stride(), (Stride2D{64, 4}));
    CORRADE_COMPARE(b[0][0], 0);
    CORRADE_COMPARE(b[0][1], 1);
    CORRADE_COMPARE(b[0][2], 2);
    CORRADE_COMPARE(b[1][0], 8);
    CORRADE_COMPARE(b[1][1], 9);
    CORRADE_COMPARE(b[1][2], 10);
}

void StridedArrayViewTest::transposed() {
    struct {
        int value;
        int:32;
    } data[24]{
        {0}, {1}, {2}, {3},
        {4}, {5}, {6}, {7},
        {8}, {9}, {10}, {11},

        {12}, {13}, {14}, {15},
        {16}, {17}, {18}, {19},
        {20}, {21}, {22}, {23},
    };

    StridedArrayView3Di a{data, &data[0].value, {2, 3, 4}, {96, 32, 8}};
    CORRADE_COMPARE(a[0][1][0], 4);
    CORRADE_COMPARE(a[0][1][1], 5);
    CORRADE_COMPARE(a[0][1][2], 6);
    CORRADE_COMPARE(a[0][1][3], 7);

    StridedArrayView3Di b = a.transposed<1, 2>();
    CORRADE_COMPARE(b[0][0][1], 4);
    CORRADE_COMPARE(b[0][1][1], 5);
    CORRADE_COMPARE(b[0][2][1], 6);
    CORRADE_COMPARE(b[0][3][1], 7);
}

void StridedArrayViewTest::transposedToSelf() {
    int data[24]{};
    StridedArrayView3Di a{data, {2, 3, 4}};

    /* Should be a no-op */
    StridedArrayView3Di b = a.transposed<1, 1>();
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 16, 4}));
}

void StridedArrayViewTest::flipped() {
    int data[5]{0, 1, 2, 3, 4};

    StridedArrayView1Di a = data;
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);

    StridedArrayView1Di b = a.flipped<0>();
    CORRADE_COMPARE(b[0], 4);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 2);
    CORRADE_COMPARE(b[3], 1);
    CORRADE_COMPARE(b[4], 0);

    /* Flipping twice results in the same thing */
    CORRADE_VERIFY(a.flipped<0>().flipped<0>().data() == data);
}

void StridedArrayViewTest::flippedZeroSize() {
    int data[1]{};
    StridedArrayView1Di a{data, 0, 8};

    /* Should not result in any difference in the data pointer -- especially
       not any overflowing values */
    StridedArrayView1Di b = a.flipped<0>();
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(b.stride(), -8);
}

void StridedArrayViewTest::flipped3D() {
    struct {
        int value;
        int:32;
    } data[24]{
        {0}, {1}, {2}, {3},
        {4}, {5}, {6}, {7},
        {8}, {9}, {10}, {11},

        {12}, {13}, {14}, {15},
        {16}, {17}, {18}, {19},
        {20}, {21}, {22}, {23},
    };

    StridedArrayView3Di a{data, &data[0].value, {2, 3, 4}, {96, 32, 8}};
    CORRADE_COMPARE(a[1][0][1], 13);
    CORRADE_COMPARE(a[1][1][1], 17);
    CORRADE_COMPARE(a[1][2][1], 21);

    StridedArrayView3Di b = a.flipped<1>();
    CORRADE_COMPARE(b[1][0][1], 21);
    CORRADE_COMPARE(b[1][1][1], 17);
    CORRADE_COMPARE(b[1][2][1], 13);
}

void StridedArrayViewTest::flipped3DZeroSize() {
    /* Same as flipped3D() above, except that the second dimension size is 0 */
    int data[1]{};
    StridedArrayView3Di a{data, {2, 0, 4}, {96, 32, 8}};

    /* Should not result in any difference in the data pointer -- especially
       not any overflowing values */
    StridedArrayView3Di b = a.flipped<1>();
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), (Size3D{2, 0, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{96, -32, 8}));
}

void StridedArrayViewTest::broadcasted() {
    int data[1]{5};

    StridedArrayView1Di a = data;
    CORRADE_COMPARE(a.size(), 1);
    CORRADE_COMPARE(a.stride(), 4);
    CORRADE_COMPARE(a[0], 5);

    StridedArrayView1Di b = a.broadcasted<0>(12);
    CORRADE_COMPARE(b.size(), 12);
    CORRADE_COMPARE(b.stride(), 0);
    CORRADE_COMPARE(b[7], 5);
}

void StridedArrayViewTest::broadcasted3D() {
    struct {
        int value;
        int:32;
    } data[8]{
        {0}, {1}, {2}, {3},

        {12}, {13}, {14}, {15},
    };

    StridedArrayView3Di a{data, &data[0].value, {2, 1, 4}, {32, 32, 8}};
    CORRADE_COMPARE(a.size(), (Size3D{2, 1, 4}));
    CORRADE_COMPARE(a.stride(), (Stride3D{32, 32, 8}));
    CORRADE_COMPARE(a[1][0][1], 13);
    CORRADE_COMPARE(a[1][0][2], 14);
    CORRADE_COMPARE(a[1][0][3], 15);

    StridedArrayView3Di b = a.broadcasted<1>(3);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 4}));
    CORRADE_COMPARE(b.stride(), (Stride3D{32, 0, 8}));
    CORRADE_COMPARE(b[1][0][1], 13);
    CORRADE_COMPARE(b[1][1][1], 13);
    CORRADE_COMPARE(b[1][2][1], 13);
    CORRADE_COMPARE(b[1][0][2], 14);
    CORRADE_COMPARE(b[1][1][2], 14);
    CORRADE_COMPARE(b[1][2][2], 14);
    CORRADE_COMPARE(b[1][0][3], 15);
    CORRADE_COMPARE(b[1][1][3], 15);
    CORRADE_COMPARE(b[1][2][3], 15);
}

void StridedArrayViewTest::broadcastedInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct {
        int value;
        int:32;
    } data[8]{
        {0}, {1}, {2}, {3},

        {12}, {13}, {14}, {15},
    };

    StridedArrayView3Di a{data, &data[0].value, {2, 1, 4}, {32, 32, 8}};

    Containers::String out;
    Error redirectError{&out};

    a.broadcasted<2>(16);
    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::broadcasted(): can't broadcast dimension 2 with 4 elements\n");
}

void StridedArrayViewTest::expandedCollapsed() {
    /* Three blocks of 6x2 pairs, first pair item always ends with 0, second
       with 1 */
    int data[]{
        00, 01, 10, 11, 20, 21, 30, 31, 40, 41, 50, 51,
        60, 61, 70, 71, 80, 81, 90, 91, 100, 101, 110, 111,

        120, 121, 130, 131, 140, 141, 150, 151, 160, 161, 170, 171,
        180, 181, 190, 191, 200, 201, 210, 211, 220, 221, 230, 231,

        240, 241, 250, 251, 260, 261, 270, 271, 280, 281, 290, 291,
        300, 301, 310, 311, 320, 321, 330, 331, 340, 341, 350, 351
    };

    StridedArrayView3Di a0{data, {3, 12, 2}};
    StridedArrayView2Di a1{data, {3, 24}};
    StridedArrayView2Di a2{data, {36, 2}};

    /* All three should expand to exactly the same */
    StridedArrayView4Di b[]{
        a0.expanded<1>(Size2D{2, 6}),
        a1.expanded<1>(Size3D{2, 6, 2}),
        a2.expanded<0>(Size3D{3, 2, 6})
    };
    for(std::size_t i = 0; i != Containers::arraySize(b); ++i) {
        CORRADE_ITERATION(i);

        CORRADE_COMPARE(b[i].data(), static_cast<void*>(data));
        CORRADE_COMPARE(b[i].size(), (Size4D{3, 2, 6, 2}));
        CORRADE_COMPARE(b[i].stride(), (Stride4D{96, 48, 8, 4}));

        /* Just a random sanity check */
        CORRADE_COMPARE(b[i][0][1][5][0], 110);
        CORRADE_COMPARE(b[i][1][0][4][1], 161);
        CORRADE_COMPARE(b[i][2][1][2][0], 320);
    }

    /* Collapsing them makes them equivalent to the originals again */
    StridedArrayView3Di c0 = b[0].collapsed<1, 2>();
    CORRADE_COMPARE(c0.data(), static_cast<void*>(data));
    CORRADE_COMPARE(c0.size(), a0.size());
    CORRADE_COMPARE(c0.stride(), a0.stride());

    StridedArrayView2Di c1 = b[1].collapsed<1, 3>();
    CORRADE_COMPARE(c1.data(), static_cast<void*>(data));
    CORRADE_COMPARE(c1.size(), a1.size());
    CORRADE_COMPARE(c1.stride(), a1.stride());

    StridedArrayView2Di c2 = b[2].collapsed<0, 3>();
    CORRADE_COMPARE(c2.data(), static_cast<void*>(data));
    CORRADE_COMPARE(c2.size(), a2.size());
    CORRADE_COMPARE(c2.stride(), a2.stride());

    /* These should all be a no-op, i.e. giving back the original view */
    StridedArrayView3Di d[]{
        a0.expanded<0>(Size1D{3}),
        a0.expanded<1>(Size1D{12}),
        a0.expanded<2>(Size1D{2})
    };
    for(std::size_t i = 0; i != Containers::arraySize(d); ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(d[i].data(), static_cast<void*>(data));
        CORRADE_COMPARE(d[i].size(), a0.size());
        CORRADE_COMPARE(d[i].stride(), a0.stride());
    }
}

void StridedArrayViewTest::expandedCollapsedZeroStride() {
    /* Compared to expandedCollapsed() it's just the first value in each group,
       broadcasted */
    int data[]{
        110, 111,

        230, 231,

        350, 351,
    };

    StridedArrayView3Di a = StridedArrayView3Di{data, {3, 1, 2}}.broadcasted<1>(12);

    StridedArrayView4Di b = a.expanded<1>(Size2D{2, 6});

    CORRADE_COMPARE(b.data(), a.data());
    CORRADE_COMPARE(b.size(), (Size4D{3, 2, 6, 2}));
    CORRADE_COMPARE(b.stride(), (Stride4D{8, 0, 0, 4}));

    CORRADE_COMPARE(b[0][1][5][0], 110);
    CORRADE_COMPARE(b[1][0][4][1], 231);
    CORRADE_COMPARE(b[2][1][2][0], 350);

    /* Collapsing them makes them equivalent to the originals again */
    StridedArrayView3Di c = b.collapsed<1, 2>();
    CORRADE_COMPARE(c.data(), a.data());
    CORRADE_COMPARE(c.size(), a.size());
    CORRADE_COMPARE(c.stride(), a.stride());

    /* No-op, giving back the original view */
    StridedArrayView3Di d = a.expanded<1>(Size1D{12});
    CORRADE_COMPARE(d.data(), a.data());
    CORRADE_COMPARE(d.size(), a.size());
    CORRADE_COMPARE(d.stride(), a.stride());
}

void StridedArrayViewTest::expandedCollapsedNegativeStride() {
    /* Data like in expandedCollapsed(), but with the middle dimension flipped
       which should result in the same data being at the same index */
    int data[]{
        110, 111, 100, 101, 90, 91, 80, 81, 70, 71, 60, 61,
        50, 51, 40, 41, 30, 31, 20, 21, 10, 11, 00, 01,

        230, 231, 220, 221, 210, 211, 200, 201, 190, 191, 180, 181,
        170, 171, 160, 161, 150, 151, 140, 141, 130, 131, 120, 121,

        350, 351, 340, 341, 330, 331, 320, 321, 310, 311, 300, 301,
        290, 291, 280, 281, 270, 271, 260, 261, 250, 251, 240, 241
    };

    StridedArrayView3Di a = StridedArrayView3Di{data, {3, 12, 2}}.flipped<1>();

    StridedArrayView4Di b = a.expanded<1>(Size2D{2, 6});

    CORRADE_COMPARE(b.data(), a.data());
    CORRADE_COMPARE(b.size(), (Size4D{3, 2, 6, 2}));
    CORRADE_COMPARE(b.stride(), (Stride4D{96, -48, -8, 4}));

    /* Same as in expandedCollapsed() */
    CORRADE_COMPARE(b[0][1][5][0], 110);
    CORRADE_COMPARE(b[1][0][4][1], 161);
    CORRADE_COMPARE(b[2][1][2][0], 320);

    /* Collapsing them makes them equivalent to the originals again */
    StridedArrayView3Di c = b.collapsed<1, 2>();
    CORRADE_COMPARE(c.data(), a.data());
    CORRADE_COMPARE(c.size(), a.size());
    CORRADE_COMPARE(c.stride(), a.stride());

    /* No-op, giving back the original view */
    StridedArrayView3Di d = a.expanded<1>(Size1D{12});
    CORRADE_COMPARE(d.data(), a.data());
    CORRADE_COMPARE(d.size(), a.size());
    CORRADE_COMPARE(d.stride(), a.stride());
}

void StridedArrayViewTest::expandedCollapsedInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    int data[39]{};

    StridedArrayView2Di a{data, {3, 13}};
    StridedArrayView3Di b0{data, {4, 3, 2}, {24, 8, 4}};
    StridedArrayView3Di b1{data, {4, 3, 2}, {36, 8, 4}};
    StridedArrayView3Di b2{data, {4, 3, 2}, {36, 12, 4}};

    /* These are fine */
    b0.collapsed<0, 3>();
    b0.flipped<0>().flipped<1>().flipped<2>().collapsed<0, 3>();
    b1.collapsed<1, 2>();
    b1.flipped<1>().flipped<2>().collapsed<1, 2>();

    Containers::String out;
    Error redirectError{&out};
    a.expanded<1>(Size1D{14});
    a.expanded<1>(Size2D{2, 6});
    a.expanded<1>(Size3D{2, 3, 2});
    b1.collapsed<0, 3>();
    b1.flipped<0>().flipped<1>().flipped<2>().collapsed<0, 3>();
    b2.collapsed<0, 3>();
    b2.flipped<0>().flipped<1>().flipped<2>().collapsed<0, 3>();
    CORRADE_COMPARE(out,
        "Containers::StridedArrayView::expanded(): product of {14} doesn't match 13 elements in dimension 1\n"
        "Containers::StridedArrayView::expanded(): product of {2, 6} doesn't match 13 elements in dimension 1\n"
        "Containers::StridedArrayView::expanded(): product of {2, 3, 2} doesn't match 13 elements in dimension 1\n"
        "Containers::StridedArrayView::collapsed(): expected dimension 0 stride to be 24 but got 36\n"
        "Containers::StridedArrayView::collapsed(): expected dimension 0 stride to be -24 but got -36\n"
        "Containers::StridedArrayView::collapsed(): expected dimension 1 stride to be 8 but got 12\n"
        "Containers::StridedArrayView::collapsed(): expected dimension 1 stride to be -8 but got -12\n");
}

void StridedArrayViewTest::cast() {
    struct {
        short a;
        short b;
        int c;
    } data[5]{{1, 10, 0}, {2, 20, 0}, {3, 30, 0}, {4, 40, 0}, {5, 50, 0}};
    Containers::StridedArrayView1D<short> a{data, &data[0].a, 5, 8};
    VoidStridedArrayView1D av{data, &data[0].a, 5, 8};
    ConstVoidStridedArrayView1D cav{data, &data[0].a, 5, 8};
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);
    CORRADE_COMPARE(av.stride(), 8);
    CORRADE_COMPARE(cav.stride(), 8);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 4);

    auto b = Containers::arrayCast<int>(a);
    auto bv = Containers::arrayCast<int>(av);
    auto cbv = Containers::arrayCast<const int>(cav);
    CORRADE_VERIFY(std::is_same<decltype(b), Containers::StridedArrayView1D<int>>::value);
    CORRADE_VERIFY(std::is_same<decltype(bv), Containers::StridedArrayView1D<int>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cbv), Containers::StridedArrayView1D<const int>>::value);
    CORRADE_COMPARE(static_cast<void*>(b.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<void*>(bv.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<const void*>(cbv.data()), static_cast<const void*>(a.data()));
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(bv.stride(), 8);
    CORRADE_COMPARE(cbv.stride(), 8);
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (30 << 16) | 3); /* 1966083 on LE */
    CORRADE_COMPARE(b[3], (40 << 16) | 4); /* 2621444 on LE */
    #else
    CORRADE_COMPARE(b[2], (3 << 16) | 30); /* 196638 on BE */
    CORRADE_COMPARE(b[3], (4 << 16) | 40); /* 262184 on BE */
    #endif
}

void StridedArrayViewTest::castZeroStride() {
    struct {
        short a;
        short b;
        int c;
    } data[1]{{5, 50, 0}};
    auto a = Containers::StridedArrayView1D<short>{data, &data[0].a, 5, 0};
    VoidStridedArrayView1D av = a;
    ConstVoidStridedArrayView1D cav = a;

    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(a.stride(), 0);
    CORRADE_COMPARE(av.stride(), 0);
    CORRADE_COMPARE(cav.stride(), 0);
    CORRADE_COMPARE(a[2], 5);
    CORRADE_COMPARE(a[3], 5);

    auto b = Containers::arrayCast<int>(a);
    auto bv = Containers::arrayCast<int>(av);
    auto cbv = Containers::arrayCast<const int>(cav);
    CORRADE_COMPARE(static_cast<void*>(b.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<void*>(bv.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<const void*>(cbv.data()), static_cast<const void*>(a.data()));
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(cbv.size(), 5);
    CORRADE_COMPARE(b.stride(), 0);
    CORRADE_COMPARE(bv.stride(), 0);
    CORRADE_COMPARE(cbv.stride(), 0);
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (50 << 16) | 5);
    CORRADE_COMPARE(b[3], (50 << 16) | 5);
    #else
    CORRADE_COMPARE(b[2], (5 << 16) | 50);
    CORRADE_COMPARE(b[3], (5 << 16) | 50);
    #endif
}

void StridedArrayViewTest::castNegativeStride() {
    struct {
        short a;
        short b;
        int c;
    } data[5]{{5, 50, 0}, {4, 40, 0}, {3, 30, 0}, {2, 20, 0}, {1, 10, 0}};
    auto a = Containers::StridedArrayView1D<short>{data, &data[0].a, 5, 8}.flipped<0>();
    VoidStridedArrayView1D av = a;
    ConstVoidStridedArrayView1D cav = a;

    /* Data are reversed and view flipped, so it should give the same results
       as the cast() test above */

    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(a.stride(), -8);
    CORRADE_COMPARE(av.stride(), -8);
    CORRADE_COMPARE(cav.stride(), -8);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 4);

    auto b = Containers::arrayCast<int>(a);
    auto bv = Containers::arrayCast<int>(av);
    auto cbv = Containers::arrayCast<const int>(cav);
    CORRADE_COMPARE(static_cast<void*>(b.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<void*>(bv.data()), static_cast<void*>(a.data()));
    CORRADE_COMPARE(static_cast<const void*>(cbv.data()), static_cast<const void*>(a.data()));
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(cbv.size(), 5);
    CORRADE_COMPARE(b.stride(), -8);
    CORRADE_COMPARE(bv.stride(), -8);
    CORRADE_COMPARE(cbv.stride(), -8);
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (30 << 16) | 3); /* 1966083 on LE */
    CORRADE_COMPARE(b[3], (40 << 16) | 4); /* 2621444 on LE */
    #else
    CORRADE_COMPARE(b[2], (3 << 16) | 30); /* 196638 on BE */
    CORRADE_COMPARE(b[3], (4 << 16) | 40); /* 262184 on BE */
    #endif
}

void StridedArrayViewTest::castInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

     struct alignas(2) {
        char a;
        char b;
    } data[5] {{1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}};
    Containers::StridedArrayView1D<char> a{data, &data[0].a, 5, 2};
    VoidStridedArrayView1D av = a;
    ConstVoidStridedArrayView1D cav = a;
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(a.stride(), 2);
    CORRADE_COMPARE(av.stride(), 2);
    CORRADE_COMPARE(cav.stride(), 2);

    /* Check the alignment to avoid unaligned reads on platforms where it
       matters (such as Emscripten) */
    CORRADE_VERIFY(reinterpret_cast<std::uintptr_t>(data)%2 == 0);

    auto b = Containers::arrayCast<short>(a);
    auto bv = Containers::arrayCast<short>(av);
    auto cbv = Containers::arrayCast<const short>(cav);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(cbv.size(), 5);
    CORRADE_COMPARE(b.stride(), 2);
    CORRADE_COMPARE(bv.stride(), 2);
    CORRADE_COMPARE(cbv.stride(), 2);
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (30 << 8) | 3); /* 7683 on LE */
    CORRADE_COMPARE(b[3], (40 << 8) | 4); /* 10244 on LE */
    #else
    CORRADE_COMPARE(b[2], (3 << 8) | 30); /* 798 on BE */
    CORRADE_COMPARE(b[3], (4 << 8) | 40); /* 1064 on BE */
    #endif

    {
        Containers::String out;
        Error redirectError{&out};
        Containers::arrayCast<int>(a);
        Containers::arrayCast<int>(av);
        Containers::arrayCast<const int>(cav);
        CORRADE_COMPARE(out,
            "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n"
            "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n"
            "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n");
    }
}

void StridedArrayViewTest::castInflateFlatten() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView2D<Rgb> a{image, {2, 3}, {18, 6}};
    CORRADE_COMPARE(a[1][1].r, 0x88);
    CORRADE_COMPARE(a[0][2].b, 0xee);

    /* Inflate but keep the dimension count */
    StridedArrayView2D<unsigned short> b2 = arrayCast<2, unsigned short>(a);
    CORRADE_COMPARE(b2.size(), (Size2D{2, 9}));
    CORRADE_COMPARE(b2.stride(), (Stride2D{18, 2}));
    CORRADE_COMPARE(b2[1][3], 0x88);
    CORRADE_COMPARE(b2[0][8], 0xee);

    /* Inflate */
    StridedArrayView3D<unsigned short> b3 = arrayCast<3, unsigned short>(a);
    CORRADE_COMPARE(b3.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{18, 6, 2}));
    CORRADE_COMPARE(b3[1][1][0], 0x88);
    CORRADE_COMPARE(b3[0][2][2], 0xee);

    /* Flatten but keep the dimension count */
    StridedArrayView3D<Rgb> c3 = arrayCast<3, Rgb>(b3);
    CORRADE_COMPARE(c3.size(), (Size3D{2, 3, 1}));
    CORRADE_COMPARE(c3.stride(), (Stride3D{18, 6, 6}));
    CORRADE_COMPARE(c3[1][1][0].r, 0x88);
    CORRADE_COMPARE(c3[0][2][0].b, 0xee);

    /* Flatten */
    StridedArrayView2D<Rgb> c2 = arrayCast<2, Rgb>(b3);
    CORRADE_COMPARE(c2.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(c2.stride(), (Stride2D{18, 6}));
    CORRADE_COMPARE(c2[1][1].r, 0x88);
    CORRADE_COMPARE(c2[0][2].b, 0xee);
}

void StridedArrayViewTest::castInflateFlattenZeroStride() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55},
        {0x77, 0x99, 0xbb}
    };

    StridedArrayView2D<Rgb> a{image, {2, 3}, {6, 0}};
    CORRADE_COMPARE(a[1][1].r, 0x77);
    CORRADE_COMPARE(a[0][2].b, 0x55);

    /* Inflate */
    StridedArrayView3D<unsigned short> b3 = arrayCast<3, unsigned short>(a);
    CORRADE_COMPARE(b3.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{6, 0, 2}));
    CORRADE_COMPARE(b3[1][1][0], 0x77);
    CORRADE_COMPARE(b3[0][2][2], 0x55);

    /* Flatten but keep the dimension count */
    StridedArrayView3D<Rgb> c3 = arrayCast<3, Rgb>(b3);
    CORRADE_COMPARE(c3.size(), (Size3D{2, 3, 1}));
    CORRADE_COMPARE(c3.stride(), (Stride3D{6, 0, 6}));
    CORRADE_COMPARE(c3[1][1][0].r, 0x77);
    CORRADE_COMPARE(c3[0][2][0].b, 0x55);

    /* Flatten */
    StridedArrayView2D<Rgb> c2 = arrayCast<2, Rgb>(b3);
    CORRADE_COMPARE(c2.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(c2.stride(), (Stride2D{6, 0}));
    CORRADE_COMPARE(c2[1][1].r, 0x77);
    CORRADE_COMPARE(c2[0][2].b, 0x55);
}

void StridedArrayViewTest::castInflateFlattenNegativeStride() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    auto a = StridedArrayView2D<Rgb>{image, {2, 3}, {18, 6}}.flipped<1>();
    CORRADE_COMPARE(a.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(a.stride(), (Stride2D{18, -6}));
    CORRADE_COMPARE(a[1][1].r, 0x88);
    CORRADE_COMPARE(a[0][0].b, 0xee);

    /* Inflate */
    StridedArrayView3D<unsigned short> b3 = arrayCast<3, unsigned short>(a);
    CORRADE_COMPARE(b3.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b3.stride(), (Stride3D{18, -6, 2}));
    CORRADE_COMPARE(b3[1][1][0], 0x88);
    CORRADE_COMPARE(b3[0][0][2], 0xee);

    /* Flatten, but keep the dimension count */
    StridedArrayView3D<Rgb> c3 = arrayCast<3, Rgb>(b3);
    CORRADE_COMPARE(c3.size(), (Size3D{2, 3, 1}));
    CORRADE_COMPARE(c3.stride(), (Stride3D{18, -6, 6}));
    CORRADE_COMPARE(c3[1][1][0].r, 0x88);
    CORRADE_COMPARE(c3[0][0][0].b, 0xee);

    /* Flatten */
    StridedArrayView2D<Rgb> c2 = arrayCast<2, Rgb>(b3);
    CORRADE_COMPARE(c2.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(c2.stride(), (Stride2D{18, -6}));
    CORRADE_COMPARE(c2[1][1].r, 0x88);
    CORRADE_COMPARE(c2[0][0].b, 0xee);
}

void StridedArrayViewTest::castInflateFlattenArrayView() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb data[]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}
    };

    StridedArrayView2D<const short> a = arrayCast<2, const short>(arrayView(data));
    CORRADE_COMPARE(a.size(), (Size2D{2, 3}));
    CORRADE_COMPARE(a.stride(), (Stride2D{6, 2}));
    CORRADE_COMPARE(a[0][1], 0x33);
    CORRADE_COMPARE(a[1][2], 0x66);
}

void StridedArrayViewTest::castInflateFlattenInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct Rgb {
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView3D<unsigned short> a{image, &image[0].r, {2, 3, 3}, {18, 6, 2}};
    CORRADE_COMPARE(a[1][1][0], 0x88);
    CORRADE_COMPARE(a[0][2][2], 0xee);

    StridedArrayView3D<unsigned short> b{image, &image[0].g, {2, 3, 1}, {18, 6, 6}};
    CORRADE_COMPARE(b[1][1][0], 0xaa);
    CORRADE_COMPARE(b[0][2][0], 0xcc);

    StridedArrayView3D<unsigned short> c = a.flipped<2>();

    StridedArrayView3D<unsigned short> d{image, &image[0].r, {2, 1, 3}, {18, 2, 6}};

    Containers::String out;
    Error redirectError{&out};
    arrayCast<2, unsigned int>(a);
    arrayCast<3, unsigned int>(a);
    arrayCast<2, Rgb>(b);
    arrayCast<3, Rgb>(b);
    arrayCast<2, Rgb>(c);
    arrayCast<3, Rgb>(c);
    arrayCast<2, Rgb>(d);
    arrayCast<3, Rgb>(d);
    CORRADE_COMPARE(out,
        "Containers::arrayCast(): last dimension needs to have byte size equal to new type size in order to be flattened, expected 4 but got 6\n"
        "Containers::arrayCast(): last dimension needs to have byte size divisible by new type size in order to be flattened, but for a 4-byte type got 6\n"
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 2 but got 6\n"
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 2 but got 6\n"
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 2 but got -2\n"
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 2 but got -2\n"
        "Containers::arrayCast(): can't fit a 6-byte type into a stride of 2\n"
        "Containers::arrayCast(): can't fit a 6-byte type into a stride of 2\n");

    /* Inflating w/ keeping dimension count for zero and negative strides.
       Negative/zero strides *could* probably work but my brainz are too low
       right now to figure that out */
    out = {};
    arrayCast<2, unsigned short>(StridedArrayView2D<Rgb>{image, {2, 3}, {18, 6}}.flipped<1>());
    arrayCast<2, unsigned short>(StridedArrayView2D<Rgb>{image, {2, 3}, {18, 0}});
    CORRADE_COMPARE(out,
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 6 but got -6\n"
        "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride 6 but got 0\n");
}

void StridedArrayViewTest::castInflateVoid() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView2D<void> a{image, {2, 3}, {18, 6}};
    StridedArrayView2D<const void> ca{image, {2, 3}, {18, 6}};
    StridedArrayView3D<unsigned short> b = arrayCast<3, unsigned short>(a, 3);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{18, 6, 2}));
    CORRADE_COMPARE(b[1][1][0], 0x88);
    CORRADE_COMPARE(b[0][2][2], 0xee);

    StridedArrayView3D<const unsigned short> cb = arrayCast<3, const unsigned short>(ca, 3);
    CORRADE_COMPARE(cb.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{18, 6, 2}));
    CORRADE_COMPARE(cb[1][1][0], 0x88);
    CORRADE_COMPARE(cb[0][2][2], 0xee);
}

void StridedArrayViewTest::castInflateVoidZeroStride() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[3]{
        {0x11, 0x33, 0x55},
        {0x77, 0x99, 0xbb}
    };

    StridedArrayView2D<void> a = StridedArrayView2D<Rgb>{image, {2, 3}, {6, 0}};
    StridedArrayView3D<unsigned short> b = arrayCast<3, unsigned short>(a, 3);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{6, 0, 2}));
    CORRADE_COMPARE(b[1][1][0], 0x77);
    CORRADE_COMPARE(b[0][0][2], 0x55);
}

void StridedArrayViewTest::castInflateVoidNegativeStride() {
    struct Rgb {
        /* Not using a 8bit type here in order to properly test the size
           calculation in asserts */
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView2D<void> a = StridedArrayView2D<Rgb>{image, {2, 3}, {18, 6}}.flipped<1>();
    StridedArrayView3D<unsigned short> b = arrayCast<3, unsigned short>(a, 3);
    CORRADE_COMPARE(b.size(), (Size3D{2, 3, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{18, -6, 2}));
    /* Same as in castInflateVoid(), just inverted in second dimension */
    CORRADE_COMPARE(b[1][1][0], 0x88);
    CORRADE_COMPARE(b[0][0][2], 0xee);
}

void StridedArrayViewTest::castInflateVoidInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct Rgb {
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView2D<void> a{image, &image[0], {2, 3}, {18, 6}};
    StridedArrayView2D<void> b{image, &image[0], {1, 3}, {2, 6}};

    Containers::String out;
    Error redirectError{&out};
    arrayCast<3, unsigned int>(a, 3);
    arrayCast<3, unsigned int>(b, 3);
    CORRADE_COMPARE(out,
        "Containers::arrayCast(): can't fit 3 4-byte items into a stride of 6\n"
        "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n");
}

void StridedArrayViewTest::size() {
    int data[17];
    StridedArrayView1D<int> view{data};
    CORRADE_COMPARE(arraySize(view), 17);
}

void StridedArrayViewTest::size3D() {
    int data[17*7*13];
    StridedArrayView3D<int> view{data, {17, 7, 13}};
    CORRADE_COMPARE(arraySize(view), 17);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedArrayViewTest)
