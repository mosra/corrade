/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

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

    void dimensionsConstructDefault();
    void dimensionsConstruct();
    void dimensionsConstruct3D();
    void dimensionsConstructView();
    void dimensionsConstructNoInit();
    void dimensionsConvertView();
    void dimensionsConvertScalar();
    void dimensionsConvertScalar3D();
    void dimensionsCompare();
    void dimensionsAccess();
    void dimensionsAccessInvalid();
    void dimensionsRangeFor();

    void constructEmpty();
    void constructEmptyVoid();
    void constructEmptyConstVoid();
    void constructNullptr();
    void constructNullptrVoid();
    void constructNullptrConstVoid();
    void constructNullptrSize();
    void construct();
    void constructVoid();
    void constructConstVoid();
    void constructVoidFrom();
    void constructConstVoidFrom();
    void constructSizeArray();
    void constructZeroStride();
    void constructNegativeStride();
    void constructInvalid();
    void constructInvalidVoid();
    void constructInvalidConstVoid();
    void constructFixedSize();
    void constructFixedSizeVoid();
    void constructFixedSizeConstVoid();
    void constructDerived();
    void constructView();
    void constructViewVoid();
    void constructViewConstVoid();
    void constructStaticView();
    void constructStaticViewVoid();
    void constructStaticViewConstVoid();
    void constructInitializerList();

    void construct3DEmpty();
    void construct3DEmptyVoid();
    void construct3DEmptyConstVoid();
    void construct3DNullptr();
    void construct3DNullptrVoid();
    void construct3DNullptrConstVoid();
    void construct3DNullptrSize();
    void construct3D();
    void construct3DVoid();
    void construct3DConstVoid();
    void construct3DVoidFrom();
    void construct3DConstVoidFrom();
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
    void construct3DInvalid();
    void construct3DInvalidVoid();
    void construct3DInvalidConstVoid();
    void construct3DFixedSize();
    void construct3DFixedSizeVoid();
    void construct3DFixedSizeConstVoid();
    void construct3DDerived();
    void construct3DView();
    void construct3DViewVoid();
    void construct3DViewConstVoid();
    void construct3DStaticView();
    void construct3DStaticViewVoid();
    void construct3DStaticViewConstVoid();

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

    void emptyCheck();

    void isContiguous();
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

    void every();
    void everyNegative();
    void everyInvalid();
    void every2D();
    void every2DNegative();
    void every2DInvalid();
    void every2DFirstDimension();

    void transposed();
    void flipped();
    void flipped3D();
    void broadcasted();
    void broadcasted3D();
    void broadcastedInvalid();

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
};

typedef StridedDimensions<1, std::size_t> Size1D;
typedef StridedDimensions<1, std::ptrdiff_t> Stride1D;
typedef StridedDimensions<2, std::size_t> Size2D;
typedef StridedDimensions<2, std::ptrdiff_t> Stride2D;
typedef StridedDimensions<3, std::size_t> Size3D;
typedef StridedDimensions<3, std::ptrdiff_t> Stride3D;
typedef StridedArrayView1D<int> StridedArrayView1Di;
typedef StridedArrayView1D<const int> ConstStridedArrayView1Di;
typedef StridedArrayView2D<int> StridedArrayView2Di;
typedef StridedArrayView3D<int> StridedArrayView3Di;
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
    addTests({&StridedArrayViewTest::dimensionsConstructDefault,
              &StridedArrayViewTest::dimensionsConstruct,
              &StridedArrayViewTest::dimensionsConstruct3D,
              &StridedArrayViewTest::dimensionsConstructView,
              &StridedArrayViewTest::dimensionsConstructNoInit,
              &StridedArrayViewTest::dimensionsConvertView,
              &StridedArrayViewTest::dimensionsConvertScalar,
              &StridedArrayViewTest::dimensionsConvertScalar3D,
              &StridedArrayViewTest::dimensionsCompare,
              &StridedArrayViewTest::dimensionsAccess,
              &StridedArrayViewTest::dimensionsAccessInvalid,
              &StridedArrayViewTest::dimensionsRangeFor,

              &StridedArrayViewTest::constructEmpty,
              &StridedArrayViewTest::constructEmptyVoid,
              &StridedArrayViewTest::constructEmptyConstVoid,
              &StridedArrayViewTest::constructNullptr,
              &StridedArrayViewTest::constructNullptrVoid,
              &StridedArrayViewTest::constructNullptrConstVoid,
              &StridedArrayViewTest::constructNullptrSize,
              &StridedArrayViewTest::construct,
              &StridedArrayViewTest::constructVoid,
              &StridedArrayViewTest::constructConstVoid,
              &StridedArrayViewTest::constructVoidFrom,
              &StridedArrayViewTest::constructConstVoidFrom,
              &StridedArrayViewTest::constructSizeArray,
              &StridedArrayViewTest::constructZeroStride,
              &StridedArrayViewTest::constructNegativeStride,
              &StridedArrayViewTest::constructInvalid,
              &StridedArrayViewTest::constructInvalidVoid,
              &StridedArrayViewTest::constructInvalidConstVoid,
              &StridedArrayViewTest::constructFixedSize,
              &StridedArrayViewTest::constructFixedSizeVoid,
              &StridedArrayViewTest::constructFixedSizeConstVoid,
              &StridedArrayViewTest::constructDerived,
              &StridedArrayViewTest::constructView,
              &StridedArrayViewTest::constructViewVoid,
              &StridedArrayViewTest::constructViewConstVoid,
              &StridedArrayViewTest::constructStaticView,
              &StridedArrayViewTest::constructStaticViewVoid,
              &StridedArrayViewTest::constructStaticViewConstVoid,
              &StridedArrayViewTest::constructInitializerList,

              &StridedArrayViewTest::construct3DEmpty,
              &StridedArrayViewTest::construct3DEmptyVoid,
              &StridedArrayViewTest::construct3DEmptyConstVoid,
              &StridedArrayViewTest::construct3DNullptr,
              &StridedArrayViewTest::construct3DNullptrVoid,
              &StridedArrayViewTest::construct3DNullptrConstVoid,
              &StridedArrayViewTest::construct3DNullptrSize,
              &StridedArrayViewTest::construct3D,
              &StridedArrayViewTest::construct3DVoid,
              &StridedArrayViewTest::construct3DConstVoid,
              &StridedArrayViewTest::construct3DVoidFrom,
              &StridedArrayViewTest::construct3DConstVoidFrom,
              &StridedArrayViewTest::construct3DZeroStride,
              &StridedArrayViewTest::construct3DNegativeStride,
              &StridedArrayViewTest::construct3DPackedSizeStride,
              &StridedArrayViewTest::construct3DPackedSizeStrideVoid,
              &StridedArrayViewTest::construct3DPackedSizeStrideConstVoid,
              &StridedArrayViewTest::construct3DPackedSizeOnly,
              &StridedArrayViewTest::construct3DOneSizeZero,
              &StridedArrayViewTest::construct3DOneSizeZeroVoid,
              &StridedArrayViewTest::construct3DOneSizeZeroConstVoid,
              &StridedArrayViewTest::construct3DInvalid,
              &StridedArrayViewTest::construct3DInvalidVoid,
              &StridedArrayViewTest::construct3DInvalidConstVoid,
              &StridedArrayViewTest::construct3DFixedSize,
              &StridedArrayViewTest::construct3DFixedSizeVoid,
              &StridedArrayViewTest::construct3DFixedSizeConstVoid,
              &StridedArrayViewTest::construct3DDerived,
              &StridedArrayViewTest::construct3DView,
              &StridedArrayViewTest::construct3DViewVoid,
              &StridedArrayViewTest::construct3DViewConstVoid,
              &StridedArrayViewTest::construct3DStaticView,
              &StridedArrayViewTest::construct3DStaticViewVoid,
              &StridedArrayViewTest::construct3DStaticViewConstVoid,

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

              &StridedArrayViewTest::emptyCheck,

              &StridedArrayViewTest::isContiguous,
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

              &StridedArrayViewTest::every,
              &StridedArrayViewTest::everyNegative,
              &StridedArrayViewTest::everyInvalid,
              &StridedArrayViewTest::every2D,
              &StridedArrayViewTest::every2DNegative,
              &StridedArrayViewTest::every2DInvalid,
              &StridedArrayViewTest::every2DFirstDimension,

              &StridedArrayViewTest::transposed,
              &StridedArrayViewTest::flipped,
              &StridedArrayViewTest::flipped3D,
              &StridedArrayViewTest::broadcasted,
              &StridedArrayViewTest::broadcasted3D,
              &StridedArrayViewTest::broadcastedInvalid,

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
              &StridedArrayViewTest::castInflateVoidInvalid});
}

void StridedArrayViewTest::dimensionsConstructDefault() {
    Size3D a1;
    Size3D a2{ValueInit};
    CORRADE_COMPARE(a1[0], 0);
    CORRADE_COMPARE(a1[1], 0);
    CORRADE_COMPARE(a1[2], 0);
    CORRADE_COMPARE(a2[0], 0);
    CORRADE_COMPARE(a2[1], 0);
    CORRADE_COMPARE(a2[2], 0);

    constexpr Size3D ca1;
    constexpr Size3D ca2{ValueInit};
    CORRADE_COMPARE(ca1[0], 0);
    CORRADE_COMPARE(ca1[1], 0);
    CORRADE_COMPARE(ca1[2], 0);
    CORRADE_COMPARE(ca2[0], 0);
    CORRADE_COMPARE(ca2[1], 0);
    CORRADE_COMPARE(ca2[2], 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<Size3D>::value);
    CORRADE_VERIFY((std::is_nothrow_constructible<Size3D, ValueInitT>::value));

    /* Implicit conversion from ValueInitT not allowed */
    CORRADE_VERIFY(!(std::is_convertible<ValueInitT, Size3D>::value));
}

void StridedArrayViewTest::dimensionsConstruct() {
    Size1D a = 37;
    CORRADE_COMPARE(a[0], 37);

    constexpr Size1D ca = 37;
    CORRADE_COMPARE(ca[0], 37);

    CORRADE_VERIFY((std::is_nothrow_constructible<Size1D, std::size_t>::value));
}

void StridedArrayViewTest::dimensionsConstruct3D() {
    Size3D a = {1, 37, 4564};
    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(a[1], 37);
    CORRADE_COMPARE(a[2], 4564);

    constexpr Size3D ca = {1, 37, 4564};
    CORRADE_COMPARE(ca[0], 1);
    CORRADE_COMPARE(ca[1], 37);
    CORRADE_COMPARE(ca[2], 4564);

    CORRADE_VERIFY((std::is_nothrow_constructible<Size3D, std::size_t, std::size_t, std::size_t>::value));
}

constexpr std::size_t SizeData[]{34, 67, 98989};

void StridedArrayViewTest::dimensionsConstructView() {
    std::size_t sizes[]{1, 37, 4564};

    Size3D a = sizes;
    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(a[1], 37);
    CORRADE_COMPARE(a[2], 4564);

    constexpr Size3D ca = SizeData;
    CORRADE_COMPARE(ca[0], 34);
    CORRADE_COMPARE(ca[1], 67);
    CORRADE_COMPARE(ca[2], 98989);

    CORRADE_VERIFY((std::is_nothrow_constructible<Size3D, Containers::StaticArrayView<3, const std::size_t>>::value));
}

void StridedArrayViewTest::dimensionsConstructNoInit() {
    Size3D a{1, 37, 4564};

    new(&a)Size3D{NoInit};
    {
        #if defined(__GNUC__) && __GNUC__ >= 10 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 10+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a[0], 1);
        CORRADE_COMPARE(a[1], 37);
        CORRADE_COMPARE(a[2], 4564);
    }

    CORRADE_VERIFY((std::is_nothrow_constructible<Size1D, NoInitT>::value));

    /* Implicit conversion from NoInitT not allowed */
    CORRADE_VERIFY(!(std::is_convertible<NoInitT, Size1D>::value));
}

constexpr Size3D Sizes{34, 67, 98989};

void StridedArrayViewTest::dimensionsConvertView() {
    Size3D a{1, 37, 4564};

    Containers::StaticArrayView<3, const std::size_t> view = a;
    CORRADE_COMPARE(view[0], 1);
    CORRADE_COMPARE(view[1], 37);
    CORRADE_COMPARE(view[2], 4564);

    constexpr Containers::StaticArrayView<3, const std::size_t> cview = Sizes;
    CORRADE_COMPARE(cview[0], 34);
    CORRADE_COMPARE(cview[1], 67);
    CORRADE_COMPARE(cview[2], 98989);
}

void StridedArrayViewTest::dimensionsConvertScalar() {
    Size1D a = 1337;
    std::size_t b = a;
    CORRADE_COMPARE(b, 1337);

    constexpr Size1D ca = 1337;
    constexpr std::size_t cb = ca;
    CORRADE_COMPARE(cb, 1337);
}

void StridedArrayViewTest::dimensionsConvertScalar3D() {
    CORRADE_VERIFY((std::is_convertible<Size1D, std::size_t>::value));
    CORRADE_VERIFY(!(std::is_convertible<Size3D, std::size_t>::value));
}

void StridedArrayViewTest::dimensionsCompare() {
    Size3D a{1, 37, 4564};
    Size3D b{1, 37, 4564};
    Size3D c{1, 37, 4565};

    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(!(a == c));
    CORRADE_VERIFY(a != c);
}

void StridedArrayViewTest::dimensionsAccess() {
    Size3D a{7, 13, 29};

    CORRADE_COMPARE(*a.begin(), 7);
    CORRADE_COMPARE(*a.cbegin(), 7);
    CORRADE_COMPARE(*(a.end() - 1), 29);
    CORRADE_COMPARE(*(a.cend() - 1), 29);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cabegin = *Sizes.begin();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cacbegin = *Sizes.cbegin();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t caend = *(Sizes.end() - 1);
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cacend = *(Sizes.cend() - 1);
    CORRADE_COMPARE(cabegin, 34);
    CORRADE_COMPARE(cacbegin, 34);
    CORRADE_COMPARE(caend, 98989);
    CORRADE_COMPARE(cacend, 98989);
}

void StridedArrayViewTest::dimensionsAccessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Size3D a{3, 12, 76};
    const Size3D ca{3, 12, 76};

    std::ostringstream out;
    Error redirectError{&out};

    a[3];
    /* To avoid sanitizers getting angry */
    reinterpret_cast<const Size2D&>(ca)[2];

    CORRADE_COMPARE(out.str(),
        "Containers::StridedDimensions::operator[](): dimension 3 out of range for 3 dimensions\n"
        "Containers::StridedDimensions::operator[](): dimension 2 out of range for 2 dimensions\n");
}

void StridedArrayViewTest::dimensionsRangeFor() {
    Size3D a{6, 12, 28};
    for(std::size_t& i: a) ++i;
    CORRADE_COMPARE(a, (Size3D{7, 13, 29}));

    const Size3D ca = a;
    std::size_t sum = 1;
    for(std::size_t i: ca) sum *= i;
    CORRADE_COMPARE(sum, 29*13*7);
}

void StridedArrayViewTest::constructEmpty() {
    StridedArrayView1Di a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr StridedArrayView1Di ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructEmptyVoid() {
    VoidStridedArrayView1D a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr VoidStridedArrayView1D ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructEmptyConstVoid() {
    ConstVoidStridedArrayView1D a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr ConstVoidStridedArrayView1D ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructNullptr() {
    StridedArrayView1Di a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr StridedArrayView1Di ca = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructNullptrVoid() {
    VoidStridedArrayView1D a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr VoidStridedArrayView1D ca = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructNullptrConstVoid() {
    ConstVoidStridedArrayView1D a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr ConstVoidStridedArrayView1D ca = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. passing a desired layout to a function
       that allocates the memory later */
    StridedArrayView1Di a({nullptr, 40}, nullptr, 5, 8);
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);

    constexpr StridedArrayView1Di ca({nullptr, 40}, nullptr, 5, 8);
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(ca.stride(), 8);
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
        CORRADE_VERIFY(!b.empty());
        CORRADE_COMPARE(b.size(), 10);
        CORRADE_COMPARE(b.stride(), 8);
        CORRADE_COMPARE(b[2], 7853268);
        CORRADE_COMPARE(b[4], 234810);

        auto c = stridedArrayView(a, &a[0].value, 10, 8);
        CORRADE_VERIFY((std::is_same<decltype(c), StridedArrayView1Di>::value));
        CORRADE_VERIFY(c.data() == a);
        CORRADE_VERIFY(!c.empty());
        CORRADE_COMPARE(c.size(), 10);
        CORRADE_COMPARE(c.stride(), 8);
        CORRADE_COMPARE(c[2], 7853268);
        CORRADE_COMPARE(c[4], 234810);

        auto c2 = stridedArrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c2), StridedArrayView1Di>::value));
        CORRADE_VERIFY(c2.data() == a);
        CORRADE_VERIFY(!c2.empty());
        CORRADE_COMPARE(c2.size(), 10);
        CORRADE_COMPARE(c2.stride(), 8);
        CORRADE_COMPARE(c2[2], 7853268);
        CORRADE_COMPARE(c2[4], 234810);
    }

    {
        constexpr ConstStridedArrayView1Di cb = {Struct, &Struct[0].value, 10, 8};
        CORRADE_VERIFY(cb.data() == Struct);
        CORRADE_VERIFY(!cb.empty());
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 8);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);

        constexpr auto cc = stridedArrayView(Struct, &Struct[0].value, 10, 8);
        CORRADE_VERIFY((std::is_same<decltype(cc), const ConstStridedArrayView1Di>::value));
        CORRADE_VERIFY(cc.data() == Struct);
        CORRADE_VERIFY(!cc.empty());
        CORRADE_COMPARE(cc.size(), 10);
        CORRADE_COMPARE(cc.stride(), 8);
        CORRADE_COMPARE(cc[2], 7853268);
        CORRADE_COMPARE(cc[4], 234810);

        constexpr auto cc2 = stridedArrayView(cb);
        CORRADE_VERIFY((std::is_same<decltype(cc2), const ConstStridedArrayView1Di>::value));
        CORRADE_VERIFY(cc2.data() == Struct);
        CORRADE_VERIFY(!cc2.empty());
        CORRADE_COMPARE(cc2.size(), 10);
        CORRADE_COMPARE(cc2.stride(), 8);
        CORRADE_COMPARE(cc2[2], 7853268);
        CORRADE_COMPARE(cc2[4], 234810);
    }
}

void StridedArrayViewTest::constructVoid() {
    struct {
        int value;
        int other;
    } a[10]{};

    VoidStridedArrayView1D b = {a, &a[0].value, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::constructConstVoid() {
    const struct {
        int value;
        int other;
    } a[10]{};

    ConstVoidStridedArrayView1D b = {a, &a[0].value, 10, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 8);

    constexpr ConstVoidStridedArrayView1D cb = {Struct, &Struct[0].value, 10, 8};
    CORRADE_VERIFY(cb.data() == Struct);
    CORRADE_VERIFY(!cb.empty());
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 8);
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
}

void StridedArrayViewTest::constructSizeArray() {
    /* Compared to construct(), size and stride is wrapped in {} */

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

void StridedArrayViewTest::constructZeroStride() {
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

    constexpr ConstStridedArrayView1Di cc = {Struct, &Struct[0].other, 10, 0};
    CORRADE_VERIFY(cc.data() == &Struct[0].other);
    CORRADE_COMPARE(cc.size(), 10);
    CORRADE_COMPARE(cc.stride(), 0);
    CORRADE_COMPARE(cc[2], 23125);
    CORRADE_COMPARE(cc[4], 23125);
}

void StridedArrayViewTest::constructNegativeStride() {
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

    constexpr ConstStridedArrayView1Di cc = {Struct, &Struct[9].value, 10, -8};
    CORRADE_VERIFY(cc.data() == &Struct[9].value);
    CORRADE_COMPARE(cc.size(), 10);
    CORRADE_COMPARE(cc.stride(), -8);
    CORRADE_COMPARE(cc[9 - 2], 7853268); /* ID 2 if it wouldn't be negative */
    CORRADE_COMPARE(cc[9 - 4], 234810); /* ID 4 if it wouldn't be negative */
}

void StridedArrayViewTest::constructInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct {
        int value;
        int other;
    } a[10]{};

    std::ostringstream out;
    Error redirectError{&out};

    StridedArrayView1Di{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

void StridedArrayViewTest::constructInvalidVoid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct {
        int value;
        int other;
    } a[10]{};

    std::ostringstream out;
    Error redirectError{&out};

    VoidStridedArrayView1D{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

void StridedArrayViewTest::constructInvalidConstVoid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const struct {
        int value;
        int other;
    } a[10]{};

    std::ostringstream out;
    Error redirectError{&out};

    ConstVoidStridedArrayView1D{a, &a[0].value, 10, 9};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 80 is not enough for {10} elements of stride {9}\n");
}

/* Needs to be here in order to use it in constexpr */
constexpr const int Array10[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

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
        CORRADE_VERIFY((std::is_same<decltype(b), StridedArrayView1Di>::value));
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
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value));
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }
}

void StridedArrayViewTest::constructFixedSizeVoid() {
    int a[10]{};

    VoidStridedArrayView1D b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */
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
}

void StridedArrayViewTest::constructView() {
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
        CORRADE_VERIFY((std::is_same<decltype(b), StridedArrayView1Di>::value));
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
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value));
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }
}

void StridedArrayViewTest::constructViewVoid() {
    int a[10]{};
    ArrayView<int> view = a;

    VoidStridedArrayView1D b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::constructViewConstVoid() {
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
}

void StridedArrayViewTest::constructStaticView() {
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
        CORRADE_VERIFY((std::is_same<decltype(b), StridedArrayView1Di>::value));
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
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value));
        CORRADE_VERIFY(cb.data() == Array10);
        CORRADE_COMPARE(cb.size(), 10);
        CORRADE_COMPARE(cb.stride(), 4);
        CORRADE_COMPARE(cb[2], 7853268);
        CORRADE_COMPARE(cb[4], 234810);
    }
}

void StridedArrayViewTest::constructStaticViewVoid() {
    int a[10];
    StaticArrayView<10, int> view = a;

    VoidStridedArrayView1D b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::constructStaticViewConstVoid() {
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
}

void StridedArrayViewTest::constructInitializerList() {
    std::initializer_list<int> a = {3, 5, 7};
    ConstStridedArrayView1Di b = stridedArrayView(a);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b.back(), 7);

    /* R-value init list should work too */
    CORRADE_COMPARE(stridedArrayView<int>({3, 5, 7}).front(), 3);
}

void StridedArrayViewTest::construct3DEmpty() {
    StridedArrayView3Di a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    constexpr StridedArrayView3Di ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{0, 0, 0}));
}

void StridedArrayViewTest::construct3DEmptyVoid() {
    VoidStridedArrayView3D a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::construct3DEmptyConstVoid() {
    ConstVoidStridedArrayView3D a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    constexpr ConstVoidStridedArrayView3D ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{0, 0, 0}));
}

void StridedArrayViewTest::construct3DNullptr() {
    StridedArrayView3Di a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    constexpr StridedArrayView3Di ca = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{0, 0, 0}));
}

void StridedArrayViewTest::construct3DNullptrVoid() {
    VoidStridedArrayView3D a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::construct3DNullptrConstVoid() {
    ConstVoidStridedArrayView3D a = nullptr;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(a.stride(), (Stride3D{0, 0, 0}));

    constexpr ConstVoidStridedArrayView3D ca = nullptr;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), (Size3D{0, 0, 0}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{0, 0, 0}));
}

void StridedArrayViewTest::construct3DNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */
    StridedArrayView3Di a{{nullptr, 20}, {5, 7, 3}, {16, 8, 1}};
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), (Size3D{5, 7, 3}));
    CORRADE_COMPARE(a.stride(), (Stride3D{16, 8, 1}));

    constexpr StridedArrayView3Di ca{{nullptr, 20}, {5, 7, 3}, {16, 8, 1}};
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), (Size3D{5, 7, 3}));
    CORRADE_COMPARE(ca.stride(), (Stride3D{16, 8, 1}));
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
    CORRADE_COMPARE(b.empty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b[0][0][0], 2);
    CORRADE_COMPARE(b[0][0][1], 16);
    CORRADE_COMPARE(b[0][0][2], 7853268);
    CORRADE_COMPARE(b[0][1][1], 234810);

    constexpr ConstStridedArrayView3Di cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(cb.data() == Cube);
    CORRADE_COMPARE(cb.empty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(cb[0][0][0], 2);
    CORRADE_COMPARE(cb[0][0][1], 16);
    CORRADE_COMPARE(cb[0][0][2], 7853268);
    CORRADE_COMPARE(cb[0][1][1], 234810);
}

void StridedArrayViewTest::construct3DVoid() {
    Plane a[2]{};

    VoidStridedArrayView3D b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.empty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));

    /** @todo constexpr but not const? c++14? */
}

void StridedArrayViewTest::construct3DConstVoid() {
    const Plane a[2]{};

    ConstVoidStridedArrayView3D b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.empty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(b.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));

    constexpr ConstVoidStridedArrayView3D cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(cb.data() == Cube);
    CORRADE_COMPARE(cb.empty(), (StridedDimensions<3, bool>{false, false, false}));
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{48, 24, 8}));
}

void StridedArrayViewTest::construct3DVoidFrom() {
    Plane a[2]{};

    StridedArrayView3Di b = {a, &a[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};
    VoidStridedArrayView3D bv = b;
    CORRADE_VERIFY(bv.data() == a);
    CORRADE_COMPARE(bv.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(bv.stride(), (Stride3D{48, 24, 8}));

    /** @todo constexpr but not const? c++14? */
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

    constexpr ConstStridedArrayView3Di cb = {Cube, &Cube[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), 0, sizeof(Plane::Row::Item)}};
    CORRADE_VERIFY(cb.data() == Cube);
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{48, 0, 8}));
    CORRADE_COMPARE(cb[0][0][0], 2);
    CORRADE_COMPARE(cb[0][0][1], 16);
    CORRADE_COMPARE(cb[0][0][2], 7853268);
    CORRADE_COMPARE(cb[0][1][1], 16);
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

    constexpr ConstStridedArrayView3Di cb = {Cube, &Cube[1].plane[0].row[2].value, {2, 2, 3}, {-std::ptrdiff_t(sizeof(Plane)), sizeof(Plane::Row), -std::ptrdiff_t(sizeof(Plane::Row::Item))}};
    CORRADE_VERIFY(cb.data() == &Cube[1].plane[0].row[2].value);
    CORRADE_COMPARE(cb.size(), (Size3D{2, 2, 3}));
    CORRADE_COMPARE(cb.stride(), (Stride3D{-48, 24, -8}));
    CORRADE_COMPARE(cb[1][0][2], 2);
    CORRADE_COMPARE(cb[1][0][1], 16);
    CORRADE_COMPARE(cb[1][0][0], 7853268);
    CORRADE_COMPARE(cb[1][1][1], 234810);
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
}

/* size-only constructor not provided for void overloads as there's little
   chance one would want an implicit stride of 1 */

void StridedArrayViewTest::construct3DOneSizeZero() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    StridedArrayView3Di a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out.str(), "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DOneSizeZeroVoid() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    VoidStridedArrayView3D a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out.str(), "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DOneSizeZeroConstVoid() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Assertion shouldn't fire because size in second dimension is zero */
    int data[1];
    ConstVoidStridedArrayView3D a{{data, 0}, {5, 0, 3}, {46, 54, 22}};
    CORRADE_COMPARE(out.str(), "");
    CORRADE_COMPARE(a.data(), &data[0]);
}

void StridedArrayViewTest::construct3DInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Plane a[2];

    std::ostringstream out;
    Error redirectError{&out};

    StridedArrayView3Di{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DInvalidVoid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Plane a[2];

    std::ostringstream out;
    Error redirectError{&out};

    VoidStridedArrayView3D{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DInvalidConstVoid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const Plane a[2]{};

    std::ostringstream out;
    Error redirectError{&out};

    ConstVoidStridedArrayView3D{a, &a[0].plane[0].row[0].value, {2, 5, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView: data size 96 is not enough for {2, 5, 3} elements of stride {48, 24, 8}\n");
}

void StridedArrayViewTest::construct3DFixedSize() {
    CORRADE_VERIFY((std::is_convertible<int(&)[10], StridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_convertible<int(&)[10], StridedArrayView3Di>::value));
}

void StridedArrayViewTest::construct3DFixedSizeVoid() {
    CORRADE_VERIFY((std::is_convertible<int(&)[10], VoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<int(&)[10], VoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::construct3DFixedSizeConstVoid() {
    CORRADE_VERIFY((std::is_convertible<int(&)[10], ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<int(&)[10], ConstVoidStridedArrayView3D>::value));
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
    #ifndef CORRADE_MSVC2019_COMPATIBILITY
    /* Implicit pointer downcast not constexpr on MSVC 2015, causes an ICE on
       MSVC 2017 and 2019 (but only in the 3D case, not for 1D) */
    constexpr
    #endif
    Containers::StridedArrayView2D<const Base> ca{DerivedArray, {5, 1}};
    #ifndef CORRADE_MSVC2019_COMPATIBILITY
    /* Implicit pointer downcast not constexpr on MSVC 2015, causes an ICE on
       MSVC 2017 and 2019 (but only in the 3D case, not for 1D) */
    constexpr
    #endif
    Containers::StridedArrayView2D<const Base> cav{cbv};

    CORRADE_VERIFY(ca.data() == &DerivedArray[0]);
    CORRADE_VERIFY(cav.data() == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(ca.stride(), (Stride2D{2, 2}));
    CORRADE_COMPARE(cav.size(), (Size2D{5, 1}));
    CORRADE_COMPARE(cav.stride(), (Stride2D{2, 2}));
}

void StridedArrayViewTest::construct3DView() {
    CORRADE_VERIFY((std::is_convertible<ArrayView<int>, StridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_convertible<ArrayView<int>, StridedArrayView3Di>::value));
}

void StridedArrayViewTest::construct3DViewVoid() {
    CORRADE_VERIFY((std::is_convertible<ArrayView<int>, VoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<ArrayView<int>, VoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::construct3DViewConstVoid() {
    CORRADE_VERIFY((std::is_convertible<ArrayView<int>, ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<ArrayView<int>, ConstVoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::construct3DStaticView() {
    CORRADE_VERIFY((std::is_convertible<StaticArrayView<10, int>, StridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_convertible<StaticArrayView<10, int>, StridedArrayView3Di>::value));
}

void StridedArrayViewTest::construct3DStaticViewVoid() {
    CORRADE_VERIFY((std::is_convertible<StaticArrayView<10, int>, VoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<StaticArrayView<10, int>, VoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::construct3DStaticViewConstVoid() {
    CORRADE_VERIFY((std::is_convertible<StaticArrayView<10, int>, ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<StaticArrayView<10, int>, ConstVoidStridedArrayView3D>::value));
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

    CORRADE_VERIFY((std::is_constructible<bool, StridedArrayView1Di>::value));
    CORRADE_VERIFY((std::is_constructible<bool, VoidStridedArrayView1D>::value));
    CORRADE_VERIFY((std::is_constructible<bool, ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, StridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, VoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, ConstVoidStridedArrayView1D>::value));
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
        CORRADE_VERIFY((std::is_same<decltype(b), StridedArrayView1Di>::value));
        CORRADE_COMPARE(b.data(), &data[0]);
        CORRADE_COMPARE(b.size(), 5);
    }

    constexpr ConstIntView ca{Array10, 10};
    CORRADE_COMPARE(ca.data, Array10);
    CORRADE_COMPARE(ca.size, 10);

    {
        /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang
           3.8 (Travis ASan build). ¯\_(ツ)_/¯ */
        #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
        constexpr
        #endif
        ConstStridedArrayView1Di cb = ca;
        CORRADE_COMPARE(cb.data(), Array10);
        CORRADE_COMPARE(cb.size(), 10);
    } {
        /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang
           3.8 (Travis ASan build). ¯\_(ツ)_/¯. Have to use const to make the
           type check pass. */
        #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
        constexpr
        #else
        const
        #endif
        auto cb = stridedArrayView(ca);
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstStridedArrayView1Di>::value));
        CORRADE_COMPARE(cb.data(), Array10);
        CORRADE_COMPARE(cb.size(), 10);
    }

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, Containers::StridedArrayView1D<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, Containers::StridedArrayView1D<float>>::value));
}

void StridedArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstStridedArrayView1Di b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, Containers::StridedArrayView1D<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, Containers::StridedArrayView1D<const float>>::value));
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
    CORRADE_COMPARE(a.empty(), (Bools3D{false, false, false}));

    StridedArrayView3Di b{{nullptr, 6}, {1, 0, 3}, {24, 12, 4}};
    CORRADE_VERIFY(!b);
    CORRADE_COMPARE(b.empty(), (Bools3D{false, true, false}));

    constexpr ConstStridedArrayView3Di ca{Array6, {1, 2, 3}, {24, 12, 4}};
    constexpr bool boolCa = !!ca;
    constexpr Bools3D emptyCa = ca.empty();
    CORRADE_VERIFY(boolCa);
    CORRADE_COMPARE(emptyCa, (Bools3D{false, false, false}));

    constexpr ConstStridedArrayView3Di cb{{nullptr, 6}, {1, 0, 3}, {24, 12, 4}};
    constexpr bool boolCb = !!cb;
    constexpr Bools3D emptyCb = cb.empty();
    CORRADE_VERIFY(!boolCb);
    CORRADE_COMPARE(emptyCb, (Bools3D{false, true, false}));

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY((std::is_constructible<bool, StridedArrayView3Di>::value));
    CORRADE_VERIFY((std::is_constructible<bool, VoidStridedArrayView3D>::value));
    CORRADE_VERIFY((std::is_constructible<bool, ConstVoidStridedArrayView3D>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, StridedArrayView3Di>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, VoidStridedArrayView3D>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, ConstVoidStridedArrayView3D>::value));

    /* Implicit conversion to bool from empty is allowed only for 1D */
    CORRADE_VERIFY((std::is_convertible<Bools1D, bool>::value));
    CORRADE_VERIFY(!(std::is_convertible<Bools3D, bool>::value));
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
    /* Conversion to a multi-dimensional type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, StridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, StridedArrayView3Di>::value));
}

void StridedArrayViewTest::convert3DConstFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, ConstStridedArrayView1Di>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, ConstStridedArrayView3Di>::value));
}

void StridedArrayViewTest::convert3DVoidFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, VoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, VoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::convert3DConstVoidFromExternalView() {
    /* Conversion to a multi-dimensional type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, ConstVoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::convert3DConstVoidFromConstExternalView() {
    /* Conversion to a multi-dimensional type is not allowed */
    CORRADE_VERIFY((std::is_convertible<ConstIntView, ConstVoidStridedArrayView1D>::value));
    CORRADE_VERIFY(!(std::is_convertible<ConstIntView, ConstVoidStridedArrayView3D>::value));
}

void StridedArrayViewTest::emptyCheck() {
    StridedArrayView1Di a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    constexpr StridedArrayView1Di ca;
    CORRADE_VERIFY(!ca);
    constexpr bool caEmpty = ca.empty();
    CORRADE_VERIFY(caEmpty);

    int b[5];
    StridedArrayView1Di c = {b, 5, 4};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());

    constexpr ConstStridedArrayView1Di cb = {Array10, 10, 4};
    CORRADE_VERIFY(cb);
    constexpr bool cbEmpty = cb.empty();
    CORRADE_VERIFY(!cbEmpty);
}

void StridedArrayViewTest::isContiguous() {
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

    /* Sparse variants */
    StridedArrayView3Di c{a, {2, 3, 2}, {2*6*4, 2*4, 4}};
    CORRADE_VERIFY(c.isContiguous<2>());
    CORRADE_VERIFY(c.isContiguous<1>());
    CORRADE_VERIFY(!c.isContiguous<0>());
    CORRADE_VERIFY(!c.isContiguous());

    StridedArrayView3Di d{a, {5, 1, 2}, {6*4, 2*2*4, 4}};
    CORRADE_VERIFY(d.isContiguous<2>());
    CORRADE_VERIFY(!d.isContiguous<1>());
    CORRADE_VERIFY(!d.isContiguous<0>());

    StridedArrayView3Di e{a, {5, 3, 1}, {6*4, 2*4, 2*4}};
    CORRADE_VERIFY(!e.isContiguous<2>());
    CORRADE_VERIFY(!e.isContiguous<1>());
    CORRADE_VERIFY(!e.isContiguous<0>());

    /* "Broadcast" */
    StridedArrayView3Di f{a, {5, 3, 2}, {6*4, 0, 4}};
    CORRADE_VERIFY(f.isContiguous<2>());
    CORRADE_VERIFY(!f.isContiguous<1>());
    CORRADE_VERIFY(!f.isContiguous<0>());

    /* Packed block of memory, but strides not in order / negative */
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<2>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<1>());
    CORRADE_VERIFY(!b.flipped<2>().isContiguous<0>());
    CORRADE_VERIFY(!(b.transposed<1, 2>().isContiguous<2>()));
    CORRADE_VERIFY(!(b.transposed<1, 2>().isContiguous<1>()));
    CORRADE_VERIFY(!(b.transposed<1, 2>().isContiguous<0>()));
}

void StridedArrayViewTest::asContiguous() {
    int a[2*3*5];
    StridedArrayView1Di b{a, 2*3*5, 4};
    ArrayView<int> bc = b.asContiguous();
    CORRADE_VERIFY(bc.data() == a);
    CORRADE_COMPARE(bc.size(), 2*3*5);

    StridedArrayView3Di c{a, {5, 3, 2}, {6*4, 2*4, 4}};
    ArrayView<int> cc = c.asContiguous();
    CORRADE_VERIFY(cc.data() == a);
    CORRADE_COMPARE(cc.size(), 2*3*5);
}

void StridedArrayViewTest::asContiguousNonContiguous() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    int a[2*3*5];

    std::ostringstream out;
    Error redirectError{&out};
    StridedArrayView1Di{a, 2*3*5, 4}.flipped<0>().asContiguous();
    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView::asContiguous(): the view is not contiguous\n");
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    StridedArrayView1Di a;
    a.front();
    a.back();

    int data[5];
    StridedArrayView1Di b = data;
    b[5];

    CORRADE_COMPARE(out.str(),
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
    CORRADE_COMPARE(b[0][1][2], 232342);

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

    int a[7];
    const StridedArrayView3Di b = {a, {7, 1, 1}};
    b.front().front().front() = 0;
    *(*(*(b.begin() + 1)).begin()).begin() = 1;
    *(*(*(b.cbegin() + 2)).cbegin()).begin() = 2;
    b[3][0][0] = 3;
    *((*((*(b.end() - 3)).end() - 1)).end() - 1) = 4;
    *((*((*(b.end() - 2)).end() - 1)).end() - 1) = 5;
    b.back().back().back() = 6;

    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a[5], 5);
    CORRADE_COMPARE(a[6], 6);
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
}

void StridedArrayViewTest::access3DInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    StridedArrayView3Di a{{nullptr, 1}, {1, 0, 1}, {4, 0, 4}};
    a.front().back().size();
    a.back().front().size();

    int data[6];
    StridedArrayView3Di b{data, {1, 2, 3}, {24, 12, 4}};
    b[0][1][5];

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView::back(): view is empty\n"
        "Containers::StridedArrayView::front(): view is empty\n"
        "Containers::StridedArrayView::operator[](): index 5 out of range for 3 elements\n");
}

void StridedArrayViewTest::iterator() {
    auto&& data = IteratorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    struct {
        int value;
        int:32;
    } d[7]{{443}, {1}, {2}, {3}, {4}, {5}, {6}};

    /* Verifying also that iterators of different views are not compared equal */
    StridedArrayView1Di a{d, &d[0].value, 7, data.stride1};
    if(data.flipped) a = a.flipped<0>();
    StridedArrayView1Di b;

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

    CORRADE_COMPARE(*(a.begin() + 2), data.dataBegin1);
    CORRADE_COMPARE(*(2 + a.begin()), data.dataBegin1);
    CORRADE_COMPARE(*(a.end() - 2), data.dataEnd1);
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

    StridedArrayView1Di b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StridedArrayView1Di c1 = a.prefix(3);
    CORRADE_COMPARE(c1.size(), 3);
    CORRADE_COMPARE(c1[0], 1);
    CORRADE_COMPARE(c1[1], 2);
    CORRADE_COMPARE(c1[2], 3);

    StridedArrayView1Di c2 = a.except(2);
    CORRADE_COMPARE(c2.size(), 3);
    CORRADE_COMPARE(c2[0], 1);
    CORRADE_COMPARE(c2[1], 2);
    CORRADE_COMPARE(c2[2], 3);

    StridedArrayView1Di d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);
}

void StridedArrayViewTest::sliceInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView1Di a = data;

    std::ostringstream out;
    Error redirectError{&out};

    a.slice(5, 6);
    a.slice(2, 1);

    CORRADE_COMPARE(out.str(),
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

    StridedArrayView3Di b = a.slice({0, 1, 1}, {1, 2, 3});
    CORRADE_COMPARE(b.size(), (Size3D{1, 1, 2}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b[0][0][0], 234810);
    CORRADE_COMPARE(b[0][0][1], 232342);

    StridedArrayView3Di c1 = a.prefix({1, 1, 3});
    CORRADE_COMPARE(c1.size(), (Size3D{1, 1, 3}));
    CORRADE_COMPARE(c1.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c1[0][0][0], 2);
    CORRADE_COMPARE(c1[0][0][1], 16);
    CORRADE_COMPARE(c1[0][0][2], 7853268);

    StridedArrayView3Di c2 = a.except({1, 1, 0});
    CORRADE_COMPARE(c2.size(), (Size3D{1, 1, 3}));
    CORRADE_COMPARE(c2.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c2[0][0][0], 2);
    CORRADE_COMPARE(c2[0][0][1], 16);
    CORRADE_COMPARE(c2[0][0][2], 7853268);

    StridedArrayView3Di d = a.suffix({0, 1, 2});
    CORRADE_COMPARE(d.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(d.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(d[0][0][0], 232342);
    CORRADE_COMPARE(d[1][0][0], 0);
}

void StridedArrayViewTest::slice3DInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    std::ostringstream out;
    Error redirectError{&out};

    a.slice({1, 0, 1}, {2, 4, 3});
    a.slice({2, 0, 1}, {0, 4, 3});

    CORRADE_COMPARE(out.str(),
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

    StridedArrayView3Di b = a.slice(0, 1);
    CORRADE_COMPARE(b.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(b.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(b[0][0][0], 2);
    CORRADE_COMPARE(b[0][0][1], 16);

    StridedArrayView3Di c1 = a.prefix(1);
    CORRADE_COMPARE(c1.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(c1.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c1[0][0][0], 2);
    CORRADE_COMPARE(c1[0][0][1], 16);
    CORRADE_COMPARE(c1[0][0][2], 7853268);

    StridedArrayView3Di c2 = a.except(1);
    CORRADE_COMPARE(c2.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(c2.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(c2[0][0][0], 2);
    CORRADE_COMPARE(c2[0][0][1], 16);
    CORRADE_COMPARE(c2[0][0][2], 7853268);

    StridedArrayView3Di d = a.suffix(1);
    CORRADE_COMPARE(d.size(), (Size3D{1, 2, 3}));
    CORRADE_COMPARE(d.stride(), (Stride3D{48, 24, 8}));
    CORRADE_COMPARE(d[0][0][0], 23);
    CORRADE_COMPARE(d[0][0][1], 76);
    CORRADE_COMPARE(d[0][0][2], 0);
}

void StridedArrayViewTest::slice3DFirstDimensionInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView3Di a = {data, {5, 1, 1}};

    std::ostringstream out;
    Error redirectError{&out};

    a.slice(5, 6);
    a.slice(2, 1);

    CORRADE_COMPARE(out.str(),
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

    StridedArrayView3Di d1 = a.prefix<3>(2);
    CORRADE_COMPARE(d1.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(d1.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(d1[0][0][0], 1);
    CORRADE_COMPARE(d1[1][0][0], 2);

    StridedArrayView3Di d2 = a.except<3>(3);
    CORRADE_COMPARE(d2.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(d2.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(d2[0][0][0], 1);
    CORRADE_COMPARE(d2[1][0][0], 2);

    StridedArrayView3Di e = a.suffix<3>(3);
    CORRADE_COMPARE(e.size(), (Size3D{2, 1, 1}));
    CORRADE_COMPARE(e.stride(), (Stride3D{4, 4, 4}));
    CORRADE_COMPARE(e[0][0][0], 4);
    CORRADE_COMPARE(e[1][0][0], 5);
}

void StridedArrayViewTest::sliceDimensionUpInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView1Di a = data;

    std::ostringstream out;
    Error redirectError{&out};

    a.slice<3>(5, 6);
    a.slice<3>(1, 0);

    CORRADE_COMPARE(out.str(),
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

    StridedArrayView2Di d1 = a.prefix<2>({1, 2, 3});
    CORRADE_COMPARE(d1.size(), (Size2D{1, 2}));
    CORRADE_COMPARE(d1.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(d1[0][0], 2);
    CORRADE_COMPARE(d1[0][1], -100);

    StridedArrayView2Di d2 = a.except<2>({1, 0, 0});
    CORRADE_COMPARE(d2.size(), (Size2D{1, 2}));
    CORRADE_COMPARE(d2.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(d2[0][0], 2);
    CORRADE_COMPARE(d2[0][1], -100);

    StridedArrayView2Di e = a.suffix<2>({0, 1, 2});
    CORRADE_COMPARE(e.size(), (Size2D{2, 1}));
    CORRADE_COMPARE(e.stride(), (Stride2D{48, 24}));
    CORRADE_COMPARE(e[0][0], 232342);
    CORRADE_COMPARE(e[1][0], 0);
}

void StridedArrayViewTest::sliceDimensionDownInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Plane data[2]{
        {{{{{2, 23125}, {16, 1}, {7853268, -2}}},
         {{{-100, 5}, {234810, 1}, {232342, -22222}}}}},
        {{{{{0, 0}, {0, 0}, {0, 0}}},
         {{{0, 0}, {0, 0}, {0, 0}}}}}
    };

    StridedArrayView3Di a = {data, &data[0].plane[0].row[0].value, {2, 2, 3}, {sizeof(Plane), sizeof(Plane::Row), sizeof(Plane::Row::Item)}};

    std::ostringstream out;
    Error redirectError{&out};

    a.slice<2>({0, 1, 4}, {1, 2, 5});
    a.slice<1>({0, 1, 0}, {1, 0, 1});

    CORRADE_COMPARE(out.str(),
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
    CORRADE_COMPARE(second2D.size(), (Containers::StridedArrayView2D<short>::Size{1, 2}));
    CORRADE_COMPARE(second2D.stride(), (Containers::StridedArrayView2D<short>::Stride{sizeof(Data)*2, sizeof(Data)}));
    CORRADE_COMPARE_AS(second2D[0],
        Containers::stridedArrayView<short>({3, 11}),
        TestSuite::Compare::Container);
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

void StridedArrayViewTest::everyInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    StridedArrayView1Di{}.every(0);
    CORRADE_COMPARE(out.str(), "Containers::StridedArrayView::every(): step in dimension 0 is zero\n");
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

void StridedArrayViewTest::every2DInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    StridedArrayView2Di{}.every({3, 0});
    CORRADE_COMPARE(out.str(), "Containers::StridedArrayView::every(): step in dimension 1 is zero\n");
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct {
        int value;
        int:32;
    } data[8]{
        {0}, {1}, {2}, {3},

        {12}, {13}, {14}, {15},
    };

    StridedArrayView3Di a{data, &data[0].value, {2, 1, 4}, {32, 32, 8}};

    std::ostringstream out;
    Error redirectError{&out};

    a.broadcasted<2>(16);
    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView::broadcasted(): can't broadcast dimension 2 with 4 elements\n");
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
    CORRADE_VERIFY((std::is_same<decltype(b), Containers::StridedArrayView1D<int>>::value));
    CORRADE_VERIFY((std::is_same<decltype(bv), Containers::StridedArrayView1D<int>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cbv), Containers::StridedArrayView1D<const int>>::value));
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

     struct CORRADE_ALIGNAS(2) {
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
        std::ostringstream out;
        Error redirectError{&out};
        Containers::arrayCast<int>(a);
        Containers::arrayCast<int>(av);
        Containers::arrayCast<const int>(cav);
        CORRADE_COMPARE(out.str(),
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

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

    std::ostringstream out;
    Error redirectError{&out};
    arrayCast<2, unsigned int>(a);
    arrayCast<3, unsigned int>(a);
    arrayCast<2, Rgb>(b);
    arrayCast<3, Rgb>(b);
    arrayCast<2, Rgb>(c);
    arrayCast<3, Rgb>(c);
    arrayCast<2, Rgb>(d);
    arrayCast<3, Rgb>(d);
    CORRADE_COMPARE(out.str(),
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
    out.str({});
    arrayCast<2, unsigned short>(StridedArrayView2D<Rgb>{image, {2, 3}, {18, 6}}.flipped<1>());
    arrayCast<2, unsigned short>(StridedArrayView2D<Rgb>{image, {2, 3}, {18, 0}});
    CORRADE_COMPARE(out.str(),
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct Rgb {
        unsigned short r, g, b;
    };

    Rgb image[6]{
        {0x11, 0x33, 0x55}, {0x22, 0x44, 0x66}, {0xaa, 0xcc, 0xee},
        {0x77, 0x99, 0xbb}, {0x88, 0xaa, 0xcc}, {0xbb, 0xdd, 0xff}
    };

    StridedArrayView2D<void> a{image, &image[0], {2, 3}, {18, 6}};
    StridedArrayView2D<void> b{image, &image[0], {1, 3}, {2, 6}};

    std::ostringstream out;
    Error redirectError{&out};
    arrayCast<3, unsigned int>(a, 3);
    arrayCast<3, unsigned int>(b, 3);
    CORRADE_COMPARE(out.str(),
        "Containers::arrayCast(): can't fit 3 4-byte items into a stride of 6\n"
        "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedArrayViewTest)
