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

#include <algorithm> /* std::copy() */
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/Algorithms.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct AlgorithmsTest: TestSuite::Tester {
    explicit AlgorithmsTest();

    void copy();
    void copyZeroSize();
    template<class T> void copyStrided1D();
    template<class T> void copyStrided2D();
    template<class T> void copyStrided3D();
    template<class T> void copyStrided4D();
    void copyStridedZeroSize();

    void copyInitializerList();
    void copyInitializerListZeroSize();
    void copyInitializerListStrided();
    void copyInitializerListStridedZeroSize();

    void copyNonMatchingSizes();
    void copyDifferentViewTypes();
    void copyInitializerListToDifferentViewTypes();
    template<class T> void copyMultiDimensionalArray();

    void copyBenchmarkFlatStdCopy();
    void copyBenchmarkFlatLoop();
    void copyBenchmarkFlat();

    void copyBenchmark1DLoop();
    void copyBenchmark2DLoop();
    void copyBenchmark3DLoop();

    void copyBenchmark1DContiguous();
    void copyBenchmark2DAllContiguous();
    void copyBenchmark3DAllContiguous();

    void copyBenchmark2DLastContiguous();
    void copyBenchmark3DLastContiguous();

    void copyBenchmark1DNonContiguous();
    void copyBenchmark2DNonContiguous();
    template<class T> void copyBenchmark3DNonContiguous();

    template<class T> void flipInPlaceFirstDimension();
    template<class T> void flipInPlaceSecondDimension();
    template<class T> void flipInPlaceThirdDimension();
    void flipInPlaceZeroSize();
    void flipInPlaceNonContigous();
};

const struct {
    const char* name;
    std::ptrdiff_t srcStride, dstStride;
    bool flipped;
} Copy1DData[]{
    {"contiguous", 1, 1, false},
    {"sparse src", 2*1, 1, false},
    {"sparse dst", 1, 2*1, false},
    {"contiguous flipped", 1, 1, true}
};

const struct {
    const char* name;
    Containers::StridedDimensions<2, std::ptrdiff_t> srcStride, dstStride;
    bool flipped, transposed;
} Copy2DData[]{
    {"contiguous", {5, 1}, {5, 1}, false, false},
    {"sparse src 0", {2*5, 1}, {5, 1}, false, false},
    {"sparse src 1", {5, 2*1}, {5, 1}, false, false},
    {"sparse dst 0", {5, 1}, {2*5, 1}, false, false},
    {"sparse dst 1", {5, 1}, {5, 2*1}, false, false},
    {"contiguous flipped", {5, 1}, {5, 1}, true, false},
    {"contiguous transposed", {5, 1}, {5, 1}, false, true}
};

const struct {
    const char* name;
    Containers::StridedDimensions<3, std::ptrdiff_t> srcStride, dstStride;
    bool flipped, transposed;
} Copy3DData[]{
    {"contiguous", {15, 5, 1}, {15, 5, 1}, false, false},
    {"sparse src 0", {2*15, 5, 1}, {15, 5, 1}, false, false},
    {"sparse src 1", {15, 2*5, 1}, {15, 5, 1}, false, false},
    {"sparse src 2", {15, 5, 2*1}, {15, 5, 1}, false, false},
    {"sparse dst 0", {15, 5, 1}, {2*15, 5, 1}, false, false},
    {"sparse dst 1", {15, 5, 1}, {15, 2*5, 1}, false, false},
    {"sparse dst 2", {15, 5, 1}, {15, 5, 2*1}, false, false},
    {"contiguous flipped", {15, 5, 1}, {15, 5, 1}, true, false},
    {"contiguous transposed", {15, 5, 1}, {15, 5, 1}, false, true}
};

const struct {
    const char* name;
    Containers::StridedDimensions<4, std::ptrdiff_t> srcStride, dstStride;
    bool flipped, transposed;
} Copy4DData[]{
    {"contiguous", {105, 15, 5, 1}, {105, 15, 5, 1}, false, false},
    {"sparse src 0", {2*105, 15, 5, 1}, {105, 15, 5, 1}, false, false},
    {"sparse src 1", {105, 2*15, 5, 1}, {105, 15, 5, 1}, false, false},
    {"sparse src 2", {105, 15, 2*5, 1}, {105, 15, 5, 1}, false, false},
    {"sparse src 3", {105, 15, 5, 2*1}, {105, 15, 5, 1}, false, false},
    {"sparse dst 0", {105, 15, 5, 1}, {2*105, 15, 5, 1}, false, false},
    {"sparse dst 1", {105, 15, 5, 1}, {105, 2*15, 5, 1}, false, false},
    {"sparse dst 2", {105, 15, 5, 1}, {105, 15, 2*5, 1}, false, false},
    {"sparse dst 3", {105, 15, 5, 1}, {105, 15, 5, 2*1}, false, false},
    {"contiguous flipped", {105, 15, 5, 1}, {105, 15, 5, 1}, true, false},
    {"contiguous transposed", {105, 15, 5, 1}, {105, 15, 5, 1}, false, true}
};

/* For testing large types (and the Duff's device branch, which is 8 bytes and
   above right now). The class explicitly fills all the data to catch potential
   errors where just a part gets copied. */
template<std::size_t size> struct Data {
    /*implicit*/ Data() = default;
    /*implicit*/ Data(unsigned char value) {
        for(std::size_t i = 0; i != size; ++i)
            data[i] = value;
    }

    unsigned char data[size];

    Data& operator++() {
        for(std::size_t i = 0; i != size; ++i)
            ++data[i];
        return *this;
    }

    bool operator==(const Data& other) const {
        for(std::size_t i = 0; i != size; ++i)
            if(data[i] != other.data[i]) return false;
        return true;
    }
};
template<std::size_t size> Debug& operator<<(Debug& debug, const Data<size>& value) {
    return debug << value.data[0];
}

template<class> struct TypeName;
template<> struct TypeName<char> {
    static const char* name() { return "char"; }
};
template<> struct TypeName<int> {
    static const char* name() { return "int"; }
};
template<> struct TypeName<Data<1>> {
    static const char* name() { return "1B"; }
};
template<> struct TypeName<Data<4>> {
    static const char* name() { return "4B"; }
};
template<> struct TypeName<Data<8>> {
    static const char* name() { return "8B"; }
};
template<> struct TypeName<Data<16>> {
    static const char* name() { return "16B"; }
};
template<> struct TypeName<Data<32>> {
    static const char* name() { return "32B"; }
};

struct Struct {
    Struct(int a = 0): a{a} {}
    int a;
};

AlgorithmsTest::AlgorithmsTest() {
    addTests({&AlgorithmsTest::copy,
              &AlgorithmsTest::copyZeroSize});

    addInstancedTests<AlgorithmsTest>({
        &AlgorithmsTest::copyStrided1D<char>,
        &AlgorithmsTest::copyStrided1D<int>,
        }, Containers::arraySize(Copy1DData));
    addInstancedTests<AlgorithmsTest>({
        &AlgorithmsTest::copyStrided2D<char>,
        &AlgorithmsTest::copyStrided2D<int>,
        }, Containers::arraySize(Copy2DData));
    addInstancedTests<AlgorithmsTest>({
        &AlgorithmsTest::copyStrided3D<char>,
        &AlgorithmsTest::copyStrided3D<int>,
        }, Containers::arraySize(Copy3DData));
    addInstancedTests<AlgorithmsTest>({
        &AlgorithmsTest::copyStrided4D<char>,
        &AlgorithmsTest::copyStrided4D<int>,
        &AlgorithmsTest::copyStrided4D<Data<32>>,
        }, Containers::arraySize(Copy4DData));

    addTests({&AlgorithmsTest::copyStridedZeroSize,

              &AlgorithmsTest::copyInitializerList,
              &AlgorithmsTest::copyInitializerListZeroSize,
              &AlgorithmsTest::copyInitializerListStrided,
              &AlgorithmsTest::copyInitializerListStridedZeroSize,

              &AlgorithmsTest::copyNonMatchingSizes,
              &AlgorithmsTest::copyDifferentViewTypes,
              &AlgorithmsTest::copyInitializerListToDifferentViewTypes,
              &AlgorithmsTest::copyMultiDimensionalArray<int>,
              &AlgorithmsTest::copyMultiDimensionalArray<Struct>});

    addBenchmarks({&AlgorithmsTest::copyBenchmarkFlatStdCopy,
                   &AlgorithmsTest::copyBenchmarkFlatLoop,
                   &AlgorithmsTest::copyBenchmarkFlat,

                   &AlgorithmsTest::copyBenchmark1DLoop,
                   &AlgorithmsTest::copyBenchmark2DLoop,
                   &AlgorithmsTest::copyBenchmark3DLoop,

                   &AlgorithmsTest::copyBenchmark1DContiguous,
                   &AlgorithmsTest::copyBenchmark2DAllContiguous,
                   &AlgorithmsTest::copyBenchmark3DAllContiguous,

                   &AlgorithmsTest::copyBenchmark2DLastContiguous,
                   &AlgorithmsTest::copyBenchmark3DLastContiguous,

                   &AlgorithmsTest::copyBenchmark1DNonContiguous,
                   &AlgorithmsTest::copyBenchmark2DNonContiguous,
                   &AlgorithmsTest::copyBenchmark3DNonContiguous<Data<1>>,
                   &AlgorithmsTest::copyBenchmark3DNonContiguous<Data<4>>,
                   &AlgorithmsTest::copyBenchmark3DNonContiguous<Data<8>>,
                   &AlgorithmsTest::copyBenchmark3DNonContiguous<Data<16>>,
                   &AlgorithmsTest::copyBenchmark3DNonContiguous<Data<32>>}, 100);

    addTests({&AlgorithmsTest::flipInPlaceFirstDimension<Data<1>>,
              &AlgorithmsTest::flipInPlaceFirstDimension<Data<8>>,
              &AlgorithmsTest::flipInPlaceFirstDimension<Data<32>>,

              &AlgorithmsTest::flipInPlaceSecondDimension<Data<1>>,
              &AlgorithmsTest::flipInPlaceSecondDimension<Data<8>>,
              &AlgorithmsTest::flipInPlaceSecondDimension<Data<32>>,

              &AlgorithmsTest::flipInPlaceThirdDimension<Data<1>>,
              &AlgorithmsTest::flipInPlaceThirdDimension<Data<8>>,
              &AlgorithmsTest::flipInPlaceThirdDimension<Data<32>>,

              &AlgorithmsTest::flipInPlaceZeroSize,
              &AlgorithmsTest::flipInPlaceNonContigous});
}

void AlgorithmsTest::copy() {
    int src[5];
    int dst[5];

    int n = 0;
    for(int& i: src) i = ++n;

    Utility::copy(Containers::arrayView(src), Containers::arrayView(dst));

    CORRADE_COMPARE_AS(Containers::arrayView(dst),
        Containers::arrayView(src),
        TestSuite::Compare::Container);
}

void AlgorithmsTest::copyZeroSize() {
    int dst[1];
    Utility::copy(nullptr, Containers::arrayView(dst).prefix(std::size_t{0}));

    /* Shouldn't crash */
    CORRADE_VERIFY(true);
}

template<class T> void AlgorithmsTest::copyStrided1D() {
    auto&& data = Copy1DData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(TypeName<T>::name());

    /* Enough so even sparse variants fit */
    Containers::Array<T> srcData{NoInit, std::size_t(data.srcStride*5*2)};
    Containers::Array<T> dstData{NoInit, std::size_t(data.dstStride*5*2)};

    Containers::StridedArrayView1D<T> src{srcData, 5, std::ptrdiff_t(data.srcStride*sizeof(T))};
    Containers::StridedArrayView1D<T> dst{dstData, 5, std::ptrdiff_t(data.dstStride*sizeof(T))};
    if(data.flipped) {
        src = src.template flipped<0>();
        dst = dst.template flipped<0>();
    }

    T n = 0;
    for(T& i: src) i = ++n;

    Utility::copy(src, dst);

    CORRADE_COMPARE_AS(dst, src, TestSuite::Compare::Container);
}

template<class T> void AlgorithmsTest::copyStrided2D() {
    auto&& data = Copy2DData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(TypeName<T>::name());

    /* Enough so even sparse variants fit */
    Containers::Array<T> srcData{NoInit, std::size_t(data.srcStride[0]*3*2)};
    Containers::Array<T> dstData{NoInit, std::size_t(data.dstStride[0]*3*2)};

    auto srcStride = data.srcStride;
    for(auto& i: srcStride) i *= sizeof(T);
    auto dstStride = data.srcStride;
    for(auto& i: dstStride) i *= sizeof(T);
    Containers::StridedArrayView2D<T> src{srcData, {3, 5}, srcStride};
    Containers::StridedArrayView2D<T> dst{dstData, {3, 5}, dstStride};
    if(data.flipped) {
        src = src.template flipped<0>();
        dst = dst.template flipped<0>();
    }
    if(data.transposed) {
        src = src.template transposed<0, 1>();
        dst = dst.template transposed<0, 1>();
    }

    T n = 0;
    for(Containers::StridedArrayView1D<T> i: src)
        for(T& j: i)
            j = ++n;

    Utility::copy(src, dst);

    /** @todo i need to figure out recursive container comparison */
    for(std::size_t i = 0; i != src.size()[0]; ++i)
        CORRADE_COMPARE_AS(dst[i], src[i], TestSuite::Compare::Container);
}

template<class T> void AlgorithmsTest::copyStrided3D() {
    auto&& data = Copy3DData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(TypeName<T>::name());

    /* Enough so even sparse variants fit */
    Containers::Array<T> srcData{NoInit, std::size_t(data.srcStride[0]*7*2)};
    Containers::Array<T> dstData{NoInit, std::size_t(data.dstStride[0]*7*2)};

    auto srcStride = data.srcStride;
    for(auto& i: srcStride) i *= sizeof(T);
    auto dstStride = data.srcStride;
    for(auto& i: dstStride) i *= sizeof(T);
    Containers::StridedArrayView3D<T> src{srcData, {7, 3, 5}, srcStride};
    Containers::StridedArrayView3D<T> dst{dstData, {7, 3, 5}, dstStride};
    if(data.flipped) {
        src = src.template flipped<0>();
        dst = dst.template flipped<0>();
    }
    if(data.transposed) {
        src = src.template transposed<0, 1>();
        dst = dst.template transposed<0, 1>();
    }

    T n = 0;
    for(Containers::StridedArrayView2D<T> i: src)
        for(Containers::StridedArrayView1D<T> j: i)
            for(T& k: j)
                k = ++n;

    Utility::copy(src, dst);

    /** @todo i need to figure out recursive container comparison */
    for(std::size_t i = 0; i != src.size()[0]; ++i)
        for(std::size_t j = 0; j != src.size()[1]; ++j)
            CORRADE_COMPARE_AS(dst[i][j], src[i][j], TestSuite::Compare::Container);
}

template<class T> void AlgorithmsTest::copyStrided4D() {
    auto&& data = Copy4DData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(TypeName<T>::name());

    /* Enough so even sparse variants fit */
    Containers::Array<T> srcData{NoInit, std::size_t(data.srcStride[0]*2*2)};
    Containers::Array<T> dstData{NoInit, std::size_t(data.dstStride[0]*2*2)};

    auto srcStride = data.srcStride;
    for(auto& i: srcStride) i *= sizeof(T);
    auto dstStride = data.srcStride;
    for(auto& i: dstStride) i *= sizeof(T);
    Containers::StridedArrayView4D<T> src{srcData, {2, 7, 3, 5}, srcStride};
    Containers::StridedArrayView4D<T> dst{dstData, {2, 7, 3, 5}, dstStride};
    if(data.flipped) {
        src = src.template flipped<0>();
        dst = dst.template flipped<0>();
    }
    if(data.transposed) {
        src = src.template transposed<0, 1>();
        dst = dst.template transposed<0, 1>();
    }

    T n = 0;
    for(Containers::StridedArrayView3D<T> i: src)
        for(Containers::StridedArrayView2D<T> j: i)
            for(Containers::StridedArrayView1D<T> k: j)
                for(T& l: k)
                    l = ++n;

    Utility::copy(src, dst);

    /** @todo i need to figure out recursive container comparison */
    for(std::size_t i = 0; i != src.size()[0]; ++i)
        for(std::size_t j = 0; j != src.size()[1]; ++j)
            for(std::size_t k = 0; k != src.size()[2]; ++k)
                CORRADE_COMPARE_AS(dst[i][j][k], src[i][j][k], TestSuite::Compare::Container);
}

void AlgorithmsTest::copyStridedZeroSize() {
    Containers::StridedArrayView1D<char> src{nullptr, 0, 16};
    Containers::StridedArrayView1D<char> dst{nullptr, 0, 16};

    /* Shouldn't crash -- the Duff's device expects a non-zero size, so there
       needs to be an extra check */
    Utility::copy(src, dst);
    CORRADE_VERIFY(true);
}

void AlgorithmsTest::copyInitializerList() {
    /* Not an int to verify the initializer list gets proper type inferred */
    unsigned dst[5];
    Utility::copy({1, 7, 2, 3, 5}, Containers::arrayView(dst));

    CORRADE_COMPARE_AS(Containers::arrayView(dst),
        Containers::arrayView<unsigned>({1, 7, 2, 3, 5}),
        TestSuite::Compare::Container);
}

void AlgorithmsTest::copyInitializerListZeroSize() {
    /* Shouldn't crash and neither should be ambiguous */
    int dst[1];
    Utility::copy({}, Containers::arrayView(dst).prefix(std::size_t{0}));

    CORRADE_VERIFY(true);
}

void AlgorithmsTest::copyInitializerListStrided() {
    /* Not an int to verify the initializer list gets proper type inferred */
    unsigned dst[10]{};
    Utility::copy({1, 7, 2, 3, 5}, Containers::stridedArrayView(Containers::arrayView(dst), 5, 8));

    CORRADE_COMPARE_AS(Containers::arrayView(dst),
        Containers::arrayView<unsigned>({1, 0, 7, 0, 2, 0, 3, 0, 5, 0}),
        TestSuite::Compare::Container);
}

void AlgorithmsTest::copyInitializerListStridedZeroSize() {
    /* Shouldn't crash and neither should be ambiguous */
    int dst[1];
    Utility::copy({}, Containers::stridedArrayView(dst).prefix(std::size_t{0}));

    CORRADE_VERIFY(true);
}

void AlgorithmsTest::copyNonMatchingSizes() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    char a[2*3*5*7]{};
    int b[2*3*5*7]{};

    /* Normal */
    Utility::copy(Containers::ArrayView<const char>{a, 2},
                  Containers::ArrayView<char>{a, 3});

    /* Strided */
    Utility::copy(Containers::StridedArrayView1D<const char>{a, 2},
                  Containers::StridedArrayView1D<char>{a, 3});
    Utility::copy(Containers::StridedArrayView2D<const char>{a, {2, 3}},
                  Containers::StridedArrayView2D<char>{a, {2, 4}});
    Utility::copy(Containers::StridedArrayView3D<const char>{a, {2, 3, 5}},
                  Containers::StridedArrayView3D<char>{a, {2, 4, 5}});
    Utility::copy(Containers::StridedArrayView4D<const char>{a, {2, 3, 5, 7}},
                  Containers::StridedArrayView4D<char>{a, {2, 3, 5, 6}});

    /* Templated variant */
    Utility::copy(Containers::StridedArrayView3D<const int>{b, {2, 3, 5}},
                  Containers::StridedArrayView3D<int>{b, {2, 3, 4}});

    /* Initializer list. There's no special code path for this, just to be
       sure it doesn't get auto-sliced or something. */
    Utility::copy({1, 2}, Containers::ArrayView<char>{a, 3});
    Utility::copy({1, 2, 3, 4}, Containers::ArrayView<char>{a, 3});

    CORRADE_COMPARE(out.str(),
        "Utility::Algorithms::copy(): sizes 2 and 3 don't match\n"

        "Utility::Algorithms::copy(): sizes 2 and 3 don't match\n"
        "Utility::Algorithms::copy(): sizes {2, 3} and {2, 4} don't match\n"
        "Utility::Algorithms::copy(): sizes {2, 3, 5} and {2, 4, 5} don't match\n"
        "Utility::Algorithms::copy(): sizes {2, 3, 5, 7} and {2, 3, 5, 6} don't match\n"
        "Utility::Algorithms::copy(): sizes {2, 3, 5, 4} and {2, 3, 4, 4} don't match\n"

        "Utility::Algorithms::copy(): sizes 2 and 3 don't match\n"
        "Utility::Algorithms::copy(): sizes 4 and 3 don't match\n");
}

void AlgorithmsTest::copyDifferentViewTypes() {
    int a[]{11, -22, 33, -44, 55};
    std::array<int, 5> b;
    Containers::Array<int> c{5};
    int data[5];
    Containers::StridedArrayView1D<int> d{data};
    std::vector<int> e(5);

    Utility::copy(a, b);
    Utility::copy(b, c);
    Utility::copy(c, d);
    Utility::copy(d, e);
    CORRADE_COMPARE_AS(Containers::arrayView(e), Containers::arrayView(a),
        TestSuite::Compare::Container);

    /* Test also multi-dimensional copies (both types are non-const, so should
       catch the complex variant) */
    Containers::StridedArrayView2D<int> f{e, {2, 2}};
    Containers::StridedArrayView2D<int> g{a, {2, 2}};
    f[1][1] = 777;
    Utility::copy(f, g);
    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView({11, -22, 33, 777, 55}),
        TestSuite::Compare::Container);
}

void AlgorithmsTest::copyInitializerListToDifferentViewTypes() {
    {
        int a[5];
        Utility::copy({11, -22, 33, -44, 55}, a);
        CORRADE_COMPARE_AS(Containers::arrayView(a),
            Containers::arrayView({11, -22, 33, -44, 55}),
            TestSuite::Compare::Container);
    } {
        std::array<int, 5> a;
        Utility::copy({11, -22, 33, -44, 55}, a);
        CORRADE_COMPARE_AS(Containers::arrayView(a),
            Containers::arrayView({11, -22, 33, -44, 55}),
            TestSuite::Compare::Container);
    } {
        std::vector<int> a(5);
        Utility::copy({11, -22, 33, -44, 55}, a);
        CORRADE_COMPARE_AS(Containers::arrayView(a),
            Containers::arrayView({11, -22, 33, -44, 55}),
            TestSuite::Compare::Container);
    }
}

template<class T> void AlgorithmsTest::copyMultiDimensionalArray() {
    setTestCaseTemplateName(std::is_same<T, int>::value ? "int" : "Struct");

    const T src[2][3]{{1, 2, 3}, {4, 5, 6}};
    T dst[2][3];

    /* This fails to compile for Struct on Clang 3.8 unless
       Implementation::arrayViewTypeFor(T(&)[]) is disabled for
       multi-dimensional arrays */
    Utility::copy(src, dst);

    CORRADE_COMPARE_AS(Containers::arrayCast<int>(dst),
        Containers::arrayCast<const int>(src),
        TestSuite::Compare::Container);
}

constexpr std::size_t Size = 16;
constexpr std::size_t Size2 = 64;
static_assert(Size*Size*Size == Size2*Size2, "otherwise the times won't match");

void AlgorithmsTest::copyBenchmarkFlatStdCopy() {
    int src[Size*Size*Size];
    int dst[Size*Size*Size];

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: src) i = ++n;

        std::copy(std::begin(src), std::end(src), std::begin(dst));

        ++base;
    }

    CORRADE_COMPARE(dst[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmarkFlatLoop() {
    int src[Size*Size*Size];
    int dst[Size*Size*Size];

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: src) i = ++n;

        for(std::size_t i = 0, max = Containers::arraySize(src); i != max; ++i)
            dst[i] = src[i];

        ++base;
    }

    CORRADE_COMPARE(dst[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmarkFlat() {
    int src[Size*Size*Size];
    int dst[Size*Size*Size];

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: src) i = ++n;

        Utility::copy(Containers::arrayView(src), Containers::arrayView(dst));

        ++base;
    }

    CORRADE_COMPARE(dst[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark1DLoop() {
    int srcData[Size*Size*Size];
    int dstData[Size*Size*Size];
    Containers::StridedArrayView1D<int> src{srcData, Size*Size*Size, 4};
    Containers::StridedArrayView1D<int> dst{dstData, Size*Size*Size, 4};

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        for(std::size_t i = 0; i != src.size(); ++i)
            dst[i] = src[i];

        ++base;
    }

    CORRADE_COMPARE(dstData[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark2DLoop() {
    int srcData[Size2*Size2];
    int dstData[Size2*Size2];
    Containers::StridedArrayView2D<int> src{srcData, {Size2, Size2},
        {Size2*4, 4}};
    Containers::StridedArrayView2D<int> dst{dstData, {Size2, Size2},
        {Size2*4, 4}};

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        /* Caching some of the calculations, otherwise it's REALLY slow */
        for(std::size_t i = 0; i != src.size()[0]; ++i) {
            auto src0 = src[i];
            auto dst0 = dst[i];
            for(std::size_t j = 0; j != src.size()[1]; ++j)
                dst0[j] = src0[j];
        }

        ++base;
    }

    CORRADE_COMPARE(dstData[Size2*Size2 - 1], Size2*Size2 + 10 - 1);
}

void AlgorithmsTest::copyBenchmark3DLoop() {
    int srcData[Size*Size*Size];
    int dstData[Size*Size*Size];
    Containers::StridedArrayView3D<int> src{srcData, {Size, Size, Size}};
    Containers::StridedArrayView3D<int> dst{dstData, {Size, Size, Size}};

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        /* Caching some of the calculations, otherwise it's REALLY slow */
        for(std::size_t i = 0; i != src.size()[0]; ++i) {
            auto src0 = src[i];
            auto dst0 = dst[i];
            for(std::size_t j = 0; j != src.size()[1]; ++j) {
                auto src1 = src0[j];
                auto dst1 = dst0[j];
                for(std::size_t k = 0; k != src.size()[2]; ++k)
                    dst1[k] = src1[k];
            }
        }

        ++base;
    }

    CORRADE_COMPARE(dstData[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark1DContiguous() {
    int srcData[Size*Size*Size];
    int dstData[Size*Size*Size];
    Containers::StridedArrayView1D<int> src{srcData, Size*Size*Size, 4};
    Containers::StridedArrayView1D<int> dst{dstData, Size*Size*Size, 4};
    CORRADE_VERIFY(src.isContiguous());
    CORRADE_VERIFY(dst.isContiguous());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: src) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dst[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark2DAllContiguous() {
    int srcData[Size2*Size2];
    int dstData[Size2*Size2];
    Containers::StridedArrayView2D<int> src{srcData, {Size2, Size2},
        {Size2*4, 4}};
    Containers::StridedArrayView2D<int> dst{dstData, {Size2, Size2},
        {Size2*4, 4}};
    CORRADE_VERIFY(src.isContiguous());
    CORRADE_VERIFY(dst.isContiguous());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size2*Size2 - 1], Size2*Size2 + 10 - 1);
}

void AlgorithmsTest::copyBenchmark3DAllContiguous() {
    int srcData[Size*Size*Size];
    int dstData[Size*Size*Size];
    Containers::StridedArrayView3D<int> src{srcData, {Size, Size, Size},
        {Size*Size*4, Size*4, 4}};
    Containers::StridedArrayView3D<int> dst{dstData, {Size, Size, Size},
        {Size*Size*4, Size*4, 4}};
    CORRADE_VERIFY(src.isContiguous());
    CORRADE_VERIFY(dst.isContiguous());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark2DLastContiguous() {
    int srcData[Size2*Size2*2];
    int dstData[Size2*Size2*2];
    Containers::StridedArrayView2D<int> src{srcData, {Size2, Size2},
        {Size2*8, 4}};
    Containers::StridedArrayView2D<int> dst{dstData, {Size2, Size2},
        {Size2*8, 4}};
    CORRADE_VERIFY(!src.isContiguous());
    CORRADE_VERIFY(!dst.isContiguous());
    CORRADE_VERIFY(src.isContiguous<1>());
    CORRADE_VERIFY(dst.isContiguous<1>());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size2*Size2 - Size2 - 1], Size2*Size2 + 10 - Size2 - 1);
}

void AlgorithmsTest::copyBenchmark3DLastContiguous() {
    int srcData[Size*Size*Size*2];
    int dstData[Size*Size*Size*2];
    Containers::StridedArrayView3D<int> src{srcData, {Size, Size, Size},
        {Size*Size*8, Size*8, 4}};
    Containers::StridedArrayView3D<int> dst{dstData, {Size, Size, Size},
        {Size*Size*8, Size*8, 4}};
    CORRADE_VERIFY(!src.isContiguous<1>());
    CORRADE_VERIFY(!dst.isContiguous<1>());
    CORRADE_VERIFY(src.isContiguous<2>());
    CORRADE_VERIFY(dst.isContiguous<2>());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size*Size*Size - Size - 1], Size*Size*Size + 10 - Size - 1);
}

void AlgorithmsTest::copyBenchmark1DNonContiguous() {
    int srcData[Size*Size*Size*2];
    int dstData[Size*Size*Size*2];
    Containers::StridedArrayView1D<int> src{srcData, Size*Size*Size, 8};
    Containers::StridedArrayView1D<int> dst{dstData, Size*Size*Size, 8};
    CORRADE_VERIFY(!src.isContiguous());
    CORRADE_VERIFY(!dst.isContiguous());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: src) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dst[Size*Size*Size - 1], Size*Size*Size + 10 - 1);
}

void AlgorithmsTest::copyBenchmark2DNonContiguous() {
    int srcData[Size2*Size2*2];
    int dstData[Size2*Size2*2];
    Containers::StridedArrayView2D<int> src{srcData, {Size2, Size2},
        {Size2*8, 8}};
    Containers::StridedArrayView2D<int> dst{dstData, {Size2, Size2},
        {Size2*8, 8}};
    CORRADE_VERIFY(!src.isContiguous<1>());
    CORRADE_VERIFY(!dst.isContiguous<1>());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(int& i: srcData) i = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size2*Size2 - 2], Size2*Size2 + 10 - 2);
}

template<class T> void AlgorithmsTest::copyBenchmark3DNonContiguous() {
    setTestCaseTemplateName(TypeName<T>::name());

    T srcData[Size*Size*Size*2*4/sizeof(T)];
    T dstData[Size*Size*Size*2*4/sizeof(T)];
    Containers::StridedArrayView3D<T> src{srcData, {Size*4/sizeof(T), Size, Size},
        {Size*Size*2*sizeof(T), Size*2*sizeof(T), 2*sizeof(T)}};
    Containers::StridedArrayView3D<T> dst{dstData, {Size*4/sizeof(T), Size, Size},
        {Size*Size*2*sizeof(T), Size*2*sizeof(T), 2*sizeof(T)}};
    CORRADE_VERIFY(!src.template isContiguous<2>());
    CORRADE_VERIFY(!dst.template isContiguous<2>());

    int base = 0;
    CORRADE_BENCHMARK(10) {
        int n = base;
        for(T& i: srcData) i.data[0] = ++n;

        Utility::copy(src, dst);

        ++base;
    }

    CORRADE_COMPARE(dstData[Size*Size*Size*4/sizeof(T) - 2].data[0], (Size*Size*Size*4/sizeof(T) + 10 - 2)%256);
}

template<class T> void AlgorithmsTest::flipInPlaceFirstDimension() {
    setTestCaseTemplateName(TypeName<T>::name());

    T data[] {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0xff, 0xfe, 0xfd, 0xfc, 0xfb, /* padding */

        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,
        0xfa, 0xf9, 0xf8, 0xf7, 0xf6 /* padding */
    };

    Containers::StridedArrayView3D<T> view{data,
        {2, 3, 7},
        {(3*7 + 5)*sizeof(T), 7*sizeof(T), sizeof(T)}
    };

    /* This creates a 4D view and then flattens it to 2D, calling the static 2D
       variant */
    Utility::flipInPlace<0>(view);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,
        0xff, 0xfe, 0xfd, 0xfc, 0xfb, /* padding stays untouched */

        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0xfa, 0xf9, 0xf8, 0xf7, 0xf6, /* padding stays untouched */
    }), TestSuite::Compare::Container);

    /* This creates a 11D view and then flattens it to 9D, calling the dynamic
       variant, and flipping back to the original state */
    Containers::StridedArrayView<10, T> view10{data,
        {1, 1, 1, 1, 1, 1, 1, 2, 3, 7},
        {0, 0, 0, 0, 0, 0, 0, (3*7 + 5)*sizeof(T), 7*sizeof(T), sizeof(T)}
    };
    Utility::flipInPlace<7>(view10);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0xff, 0xfe, 0xfd, 0xfc, 0xfb, /* padding stays untouched */

        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,
        0xfa, 0xf9, 0xf8, 0xf7, 0xf6, /* padding stays untouched */
    }), TestSuite::Compare::Container);
}

template<class T> void AlgorithmsTest::flipInPlaceSecondDimension() {
    setTestCaseTemplateName(TypeName<T>::name());

    T data[] {                                 /* vvvvvvvvvv-- padding */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xff, 0xfe,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0xfd, 0xfc,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0xfb, 0xfa,

        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0xf9, 0xf8,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xf7, 0xf6,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0xf5, 0xf4
                                               /* ^^^^^^^^^^ */
    };

    Containers::StridedArrayView3D<T> view{data,
        {2, 3, 7},
        {3*9*sizeof(T), 9*sizeof(T), sizeof(T)}
    };

    /* This creates a 4D view and then flattens it to 3D, calling the static 3D
       variant */
    Utility::flipInPlace<1>(view);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
                     /* padding stays untouched --vvvvvvvvvv */
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0xff, 0xfe,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0xfd, 0xfc,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xfb, 0xfa,

        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0xf9, 0xf8,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xf7, 0xf6,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0xf5, 0xf4
                                               /* ^^^^^^^^^^ */
    }), TestSuite::Compare::Container);

    /* This creates a 11D view and then flattens it to 10D, calling the dynamic
       variant, and flipping back to the original state */
    Containers::StridedArrayView<10, T> view10{data,
        {2, 1, 1, 1, 1, 1, 1, 1, 3, 7},
        {3*9*sizeof(T), 0, 0, 0, 0, 0, 0, 0, 9*sizeof(T), sizeof(T)}
    };
    Utility::flipInPlace<8>(view10);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
                     /* padding stays untouched --vvvvvvvvvv */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xff, 0xfe,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0xfd, 0xfc,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0xfb, 0xfa,

        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0xf9, 0xf8,
        0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xf7, 0xf6,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0xf5, 0xf4
                                               /* ^^^^^^^^^^ */
    }), TestSuite::Compare::Container);
}

template<class T> void AlgorithmsTest::flipInPlaceThirdDimension() {
    setTestCaseTemplateName(TypeName<T>::name());

    T data[] {
        /* padding
            --vvvv        vvvv        vvvv        vvvv        vvvv        vvvv        vvvv */
        0x01, 0xff, 0x02, 0xf9, 0x03, 0xf3, 0x04, 0xed, 0x05, 0xe7, 0x06, 0xe1, 0x07, 0xdb,
        0x08, 0xfe, 0x09, 0xf8, 0x0a, 0xf2, 0x0b, 0xec, 0x0c, 0xe6, 0x0d, 0xe0, 0x0e, 0xda,
        0x0f, 0xfd, 0x10, 0xf7, 0x11, 0xf1, 0x12, 0xeb, 0x13, 0xe5, 0x14, 0xdf, 0x15, 0xd9,

        0x16, 0xfc, 0x17, 0xf6, 0x18, 0xf0, 0x19, 0xea, 0x1a, 0xe4, 0x1b, 0xde, 0x1c, 0xd8,
        0x1d, 0xfb, 0x1e, 0xf5, 0x1f, 0xef, 0x20, 0xe9, 0x21, 0xe3, 0x22, 0xdd, 0x23, 0xd7,
        0x24, 0xfa, 0x25, 0xf4, 0x26, 0xee, 0x27, 0xe8, 0x28, 0xe2, 0x29, 0xdc, 0x2a, 0xd6
        /*    ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^ */
    };

    Containers::StridedArrayView3D<T> view{data,
        {2, 3, 7},
        {3*7*2*sizeof(T), 7*2*sizeof(T), 2*sizeof(T)}
    };

    /* This creates a 4D view and then flattens it to 4D, calling the static 4D
       variant */
    Utility::flipInPlace<2>(view);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
        /* padding stays untouched
            --vvvv        vvvv        vvvv        vvvv        vvvv        vvvv        vvvv */
        0x07, 0xff, 0x06, 0xf9, 0x05, 0xf3, 0x04, 0xed, 0x03, 0xe7, 0x02, 0xe1, 0x01, 0xdb,
        0x0e, 0xfe, 0x0d, 0xf8, 0x0c, 0xf2, 0x0b, 0xec, 0x0a, 0xe6, 0x09, 0xe0, 0x08, 0xda,
        0x15, 0xfd, 0x14, 0xf7, 0x13, 0xf1, 0x12, 0xeb, 0x11, 0xe5, 0x10, 0xdf, 0x0f, 0xd9,

        0x1c, 0xfc, 0x1b, 0xf6, 0x1a, 0xf0, 0x19, 0xea, 0x18, 0xe4, 0x17, 0xde, 0x16, 0xd8,
        0x23, 0xfb, 0x22, 0xf5, 0x21, 0xef, 0x20, 0xe9, 0x1f, 0xe3, 0x1e, 0xdd, 0x1d, 0xd7,
        0x2a, 0xfa, 0x29, 0xf4, 0x28, 0xee, 0x27, 0xe8, 0x26, 0xe2, 0x25, 0xdc, 0x24, 0xd6
        /*    ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^ */
    }), TestSuite::Compare::Container);

    /* This creates a 10D view and then flattens it to 10D, calling the dynamic
       variant, and flipping back to the original state */
    Containers::StridedArrayView<10, T> view10{data,
        {2, 3, 1, 1, 1, 1, 1, 1, 1, 7},
        {3*7*2*sizeof(T), 7*2*sizeof(T), 0, 0, 0, 0, 0, 0, 0, 2*sizeof(T)}
    };
    Utility::flipInPlace<9>(view10);
    CORRADE_COMPARE_AS(Containers::arrayView(data), Containers::arrayView<T>({
        /* padding stays untouched
            --vvvv        vvvv        vvvv        vvvv        vvvv        vvvv        vvvv */
        0x01, 0xff, 0x02, 0xf9, 0x03, 0xf3, 0x04, 0xed, 0x05, 0xe7, 0x06, 0xe1, 0x07, 0xdb,
        0x08, 0xfe, 0x09, 0xf8, 0x0a, 0xf2, 0x0b, 0xec, 0x0c, 0xe6, 0x0d, 0xe0, 0x0e, 0xda,
        0x0f, 0xfd, 0x10, 0xf7, 0x11, 0xf1, 0x12, 0xeb, 0x13, 0xe5, 0x14, 0xdf, 0x15, 0xd9,

        0x16, 0xfc, 0x17, 0xf6, 0x18, 0xf0, 0x19, 0xea, 0x1a, 0xe4, 0x1b, 0xde, 0x1c, 0xd8,
        0x1d, 0xfb, 0x1e, 0xf5, 0x1f, 0xef, 0x20, 0xe9, 0x21, 0xe3, 0x22, 0xdd, 0x23, 0xd7,
        0x24, 0xfa, 0x25, 0xf4, 0x26, 0xee, 0x27, 0xe8, 0x28, 0xe2, 0x29, 0xdc, 0x2a, 0xd6
        /*    ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^        ^^^^ */
    }), TestSuite::Compare::Container);
}

void AlgorithmsTest::flipInPlaceZeroSize() {
    Containers::StridedArrayView4D<char> view{nullptr, {}, {0, 0, 0, 1}};

    /* Shouldn't crash, assert or call memcpy with null pointers */
    Utility::flipInPlace<0>(view);
    Utility::flipInPlace<1>(view);
    Utility::flipInPlace<2>(view);
    Utility::flipInPlace<3>(view);
    CORRADE_VERIFY(true);
}

void AlgorithmsTest::flipInPlaceNonContigous() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    int a[2*3*7];
    Containers::StridedArrayView3D<int> b{a, {1, 3, 7}, {2*3*7*4, 7*4, 4}};
    Containers::StridedArrayView3D<int> c{a, {2, 1, 7}, {3*7*4, 2*7*4, 4}};
    Containers::StridedArrayView3D<int> d{a, {2, 3, 3}, {3*7*4, 7*4, 2*4}};

    /* This is fine, it should complain only for dimensions not contiguous
       *after* */
    Utility::flipInPlace<0>(b);
    Utility::flipInPlace<2>(d);

    std::ostringstream out;
    Error redirectError{&out};
    Utility::flipInPlace<0>(c);
    Utility::flipInPlace<1>(d);
    CORRADE_COMPARE(out.str(),
        "Utility::flipInPlace(): the view is not contiguous after dimension 0\n"
        "Utility::flipInPlace(): the view is not contiguous after dimension 1\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::AlgorithmsTest)
