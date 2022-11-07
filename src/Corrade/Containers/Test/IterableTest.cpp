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

/* Deliberately including first to make sure it works without ArrayView,
   *Reference or StridedArrayView being included first */
#include "Corrade/Containers/Iterable.h"

#include <sstream>
#include <vector>

#include "Corrade/Containers/AnyReference.h"
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/Containers/MoveReference.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct IterableTest: TestSuite::Tester {
    explicit IterableTest();

    void constructDefault();

    void arrayView();
    void arrayViewMutableToConst();
    template<template<class> class T> void arrayViewReference();
    template<template<class> class T> void arrayViewReferenceMutableToConst();
    void arrayViewMutableReferenceToConst();

    void stridedArrayView();
    void stridedArrayViewMutableToConst();
    template<template<class> class T> void stridedArrayViewReference();
    template<template<class> class T> void stridedArrayViewReferenceMutableToConst();
    void stridedArrayViewMutableReferenceToConst();

    void initializerList();
    void initializerListReference();
    void initializerListReferenceMutableToConst();
    void cArray();
    void array();
    void stlVector();

    void access();
    void accessInvalid();

    void iterator();
    void rangeBasedFor();
    void rangeBasedForReference();

    void overloadsWithForwardDeclaredType();
};

using namespace Containers::Literals;

constexpr struct {
    const char* name;
    bool flipped;
    std::ptrdiff_t stride;
    int dataBegin1, dataEnd1, dataBeginIncrement1, dataEndDecrement1;
} IteratorData[]{
    {"", false, 8, 2, 5, 1, 6},
    {"zero stride", false, 0, 443, 443, 443, 443},
    {"flipped", true, 8, 4, 1, 5, 443}
};

IterableTest::IterableTest() {
    addTests({&IterableTest::constructDefault,

              &IterableTest::arrayView,
              &IterableTest::arrayViewMutableToConst,
              &IterableTest::arrayViewReference<Reference>,
              &IterableTest::arrayViewReference<MoveReference>,
              &IterableTest::arrayViewReference<AnyReference>,
              &IterableTest::arrayViewReferenceMutableToConst<Reference>,
              &IterableTest::arrayViewReferenceMutableToConst<MoveReference>,
              &IterableTest::arrayViewReferenceMutableToConst<AnyReference>,
              &IterableTest::arrayViewMutableReferenceToConst,

              &IterableTest::stridedArrayView,
              &IterableTest::stridedArrayViewMutableToConst,
              &IterableTest::stridedArrayViewReference<Reference>,
              &IterableTest::stridedArrayViewReference<MoveReference>,
              &IterableTest::stridedArrayViewReference<AnyReference>,
              &IterableTest::stridedArrayViewReferenceMutableToConst<Reference>,
              &IterableTest::stridedArrayViewReferenceMutableToConst<MoveReference>,
              &IterableTest::stridedArrayViewReferenceMutableToConst<AnyReference>,
              &IterableTest::stridedArrayViewMutableReferenceToConst,

              &IterableTest::initializerList,
              &IterableTest::initializerListReference,
              &IterableTest::initializerListReferenceMutableToConst,
              &IterableTest::cArray,
              &IterableTest::array,
              &IterableTest::stlVector,

              &IterableTest::access,
              &IterableTest::accessInvalid});

    addInstancedTests({&IterableTest::iterator},
        Containers::arraySize(IteratorData));

    addTests({&IterableTest::rangeBasedFor,
              &IterableTest::rangeBasedForReference,

              &IterableTest::overloadsWithForwardDeclaredType});
}

void IterableTest::constructDefault() {
    Iterable<const int> ai;
    Iterable<const int> ai2 = nullptr;
    CORRADE_COMPARE(ai.data(), nullptr);
    CORRADE_COMPARE(ai2.data(), nullptr);
    CORRADE_COMPARE(ai.size(), 0);
    CORRADE_COMPARE(ai2.size(), 0);
    CORRADE_COMPARE(ai.stride(), 0);
    CORRADE_COMPARE(ai2.stride(), 0);
    CORRADE_VERIFY(ai.isEmpty());
    CORRADE_VERIFY(ai2.isEmpty());

    constexpr Iterable<const int> cai = nullptr;
    CORRADE_COMPARE(cai.data(), nullptr);
    CORRADE_COMPARE(cai.size(), 0);
    CORRADE_COMPARE(cai.stride(), 0);
    CORRADE_VERIFY(cai.isEmpty());
}

void IterableTest::arrayView() {
    int data[]{5, 0, -26};
    ArrayView<int> a = data;

    Iterable<int> ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), 4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, ArrayView<const int>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, ArrayView<const int>>::value);
}

void IterableTest::arrayViewMutableToConst() {
    int data[]{5, 0, -26};
    ArrayView<int> a = data;

    Iterable<const int> ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), 4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

template<template<class> class T> struct ReferenceTraits;
template<> struct ReferenceTraits<Reference> {
    typedef int& Type;
    static const char* name() { return "Reference"; }
};
template<> struct ReferenceTraits<MoveReference> {
    typedef int&& Type;
    static const char* name() { return "MoveReference"; }
};
template<> struct ReferenceTraits<AnyReference> {
    typedef int&& Type;
    static const char* name() { return "AnyReference"; }
};

template<template<class> class T> void IterableTest::arrayViewReference() {
    setTestCaseTemplateName(ReferenceTraits<T>::name());

    int dataA = 5;
    int dataB = 0;
    int dataC = -26;
    T<int> data[]{static_cast<typename ReferenceTraits<T>::Type>(dataA),
                  static_cast<typename ReferenceTraits<T>::Type>(dataB),
                  static_cast<typename ReferenceTraits<T>::Type>(dataC)};
    ArrayView<T<int>> a = data;

    Iterable<int> ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(T<int>));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, ArrayView<T<const int>>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, ArrayView<T<const int>>>::value);
}

template<template<class> class T> void IterableTest::arrayViewReferenceMutableToConst() {
    setTestCaseTemplateName(ReferenceTraits<T>::name());

    int dataA = 5;
    int dataB = 0;
    int dataC = -26;
    T<int> data[]{static_cast<typename ReferenceTraits<T>::Type>(dataA),
                  static_cast<typename ReferenceTraits<T>::Type>(dataB),
                  static_cast<typename ReferenceTraits<T>::Type>(dataC)};
    ArrayView<const T<int>> a = data;

    Iterable<const int> ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(T<int>));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, ArrayView<const Reference<const int>>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, ArrayView<const Reference<const int>>>::value);
}

void IterableTest::arrayViewMutableReferenceToConst() {
    const int dataA = 5;
    const int dataB = 0;
    const int dataC = -26;
    Reference<const int> data[]{
        #ifdef CORRADE_TARGET_MSVC
        /* Without the explicit construction, MSVC (2015, 2017, 2019, 2022,
           including /permissive-) complains that it's attempting to use the
           deleted Reference<T>(T&&) constructor. No issue on GCC or Clang. */
        Reference<const int>{dataA},
        Reference<const int>{dataB},
        Reference<const int>{dataC}
        #else
        dataA, dataB, dataC
        #endif
    };
    ArrayView<Reference<const int>> a = data;

    Iterable<const int> ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(void*));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    /* const Reference is fine, Reference<const> is not */
    CORRADE_VERIFY(std::is_constructible<Iterable<int>, ArrayView<const Reference<int>>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, ArrayView<const Reference<const int>>>::value);
}

void IterableTest::stridedArrayView() {
    int data[]{-26, 0, 5};
    StridedArrayView1D<int> a = data;

    Iterable<int> ai = a.flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, ArrayView<const int>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, ArrayView<const int>>::value);
}

void IterableTest::stridedArrayViewMutableToConst() {
    int data[]{-26, 0, 5};
    StridedArrayView1D<int> a = data;

    Iterable<const int> ai = a.flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

template<template<class> class T> void IterableTest::stridedArrayViewReference() {
    setTestCaseTemplateName(ReferenceTraits<T>::name());

    int dataA = -26;
    int dataB = 0;
    int dataC = 5;
    T<int> data[]{static_cast<typename ReferenceTraits<T>::Type>(dataA),
                  static_cast<typename ReferenceTraits<T>::Type>(dataB),
                  static_cast<typename ReferenceTraits<T>::Type>(dataC)};
    StridedArrayView1D<T<int>> a = data;

    Iterable<int> ai = a.template flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(T<int>)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, StridedArrayView1D<const int>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, StridedArrayView1D<T<const int>>>::value);
}

template<template<class> class T> void IterableTest::stridedArrayViewReferenceMutableToConst() {
    setTestCaseTemplateName(ReferenceTraits<T>::name());

    int dataA = -26;
    int dataB = 0;
    int dataC = 5;
    T<int> data[]{static_cast<typename ReferenceTraits<T>::Type>(dataA),
                  static_cast<typename ReferenceTraits<T>::Type>(dataB),
                  static_cast<typename ReferenceTraits<T>::Type>(dataC)};
    StridedArrayView1D<const T<int>> a = data;

    Iterable<const int> ai = a.template flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(T<int>)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    CORRADE_VERIFY(std::is_constructible<Iterable<const int>, StridedArrayView1D<const Reference<const int>>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, StridedArrayView1D<const Reference<const int>>>::value);
}

void IterableTest::stridedArrayViewMutableReferenceToConst() {
    const int dataA = -26;
    const int dataB = 0;
    const int dataC = 5;
    Reference<const int> data[]{
        #ifdef CORRADE_TARGET_MSVC
        /* Without the explicit construction, MSVC (2015, 2017, 2019, 2022,
           including /permissive-) complains that it's attempting to use the
           deleted Reference<T>(T&&) constructor. No issue on GCC or Clang. */
        Reference<const int>{dataA},
        Reference<const int>{dataB},
        Reference<const int>{dataC}
        #else
        dataA, dataB, dataC
        #endif
    };
    StridedArrayView1D<Reference<const int>> a = data;

    Iterable<const int> ai = a.flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(void*)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);

    /* const Reference is fine, Reference<const> is not */
    CORRADE_VERIFY(std::is_constructible<Iterable<int>, StridedArrayView1D<const Reference<int>>>::value);
    CORRADE_VERIFY(!std::is_constructible<Iterable<int>, StridedArrayView1D<const Reference<const int>>>::value);
}

void IterableTest::initializerList() {
    /* Capture correct function name */
    CORRADE_VERIFY(true);

    /* Capturing this way to be able to verify the contents without having to
       explicitly specify the type and without the initializer list going out
       of scope too early */
    [](const Iterable<const int>& ai) {
        CORRADE_VERIFY(ai.data());
        CORRADE_COMPARE(ai.size(), 3);
        /* It's always a reference, having an initializer_list<T> overload
           would cause nasty ambiguities */
        CORRADE_COMPARE(ai.stride(), sizeof(AnyReference<const int>));
        CORRADE_VERIFY(!ai.isEmpty());

        CORRADE_COMPARE(ai[0], 5);
        CORRADE_COMPARE(ai[1], 0);
        CORRADE_COMPARE(ai[2], -26);
    }({5, 0, -26});
}

void IterableTest::initializerListReference() {
    struct NonCopyable {
        explicit NonCopyable(int a): a{a} {}

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = default;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = default;

        int a;
    };

    /* Capture correct function name */
    CORRADE_VERIFY(true);

    /* Capturing this way to be able to verify the contents without having to
       explicitly specify the type and without the initializer list going out
       of scope too early */
    [](const Iterable<NonCopyable>& ai) {
        CORRADE_VERIFY(ai.data());
        CORRADE_COMPARE(ai.size(), 3);
        CORRADE_COMPARE(ai.stride(), sizeof(AnyReference<const int>));
        CORRADE_VERIFY(!ai.isEmpty());

        CORRADE_COMPARE(ai[0].a, 5);
        CORRADE_COMPARE(ai[1].a, 0);
        CORRADE_COMPARE(ai[2].a, -26);
    }({NonCopyable{5}, NonCopyable{0}, NonCopyable{-26}});
}

void IterableTest::initializerListReferenceMutableToConst() {
    int dataA = 5;
    int dataB = 0;
    int dataC = -26;
    std::initializer_list<AnyReference<int>> data{
        dataA,
        dataB,
        dataC
    };

    Iterable<const int> ai = data;
    CORRADE_COMPARE(ai.data(), data.begin());
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(AnyReference<const int>));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

void IterableTest::cArray() {
    int data[]{5, 0, -26};

    Iterable<const int> ai = data;
    CORRADE_COMPARE(ai.data(), data);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), 4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

void IterableTest::array() {
    Array<int> a{Corrade::InPlaceInit, {5, 0, -26}};

    Iterable<int> ai = a;
    CORRADE_COMPARE(ai.data(), a.data());
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), 4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

void IterableTest::stlVector() {
    std::vector<int> a{{5, 0, -26}};

    Iterable<int> ai = a;
    CORRADE_COMPARE(ai.data(), a.data());
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), 4);
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], 5);
    CORRADE_COMPARE(ai[1], 0);
    CORRADE_COMPARE(ai[2], -26);
}

void IterableTest::access() {
    int data[]{-26, 0, 8, 7, 6, 4, 5};
    const StridedArrayView1D<int> a = data;
    const Iterable<int> ai = a.flipped<0>();

    CORRADE_COMPARE(ai.front(), 5);
    CORRADE_COMPARE(ai.back(), -26);

    /* The array is non-owning, so it should provide write access to the data */
    ai.front() = 3;
    ++*(ai.begin() + 1);
    *(ai.cbegin() + 2) = -6;
    ai[3] = 14;
    --*(ai.end() - 3);
    *(ai.cend() - 2) = 111;
    ai.back() *= 2;

    CORRADE_COMPARE(data[0], -52);
    CORRADE_COMPARE(data[1], 111);
    CORRADE_COMPARE(data[2], 7);
    CORRADE_COMPARE(data[3], 14);
    CORRADE_COMPARE(data[4], -6);
    CORRADE_COMPARE(data[5], 5);
    CORRADE_COMPARE(data[6], 3);
}

void IterableTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    int data[]{5, 0, -26};

    Iterable<const int> ai = Containers::arrayView(data).prefix(std::size_t{0});
    Iterable<const int> bi = data;
    CORRADE_COMPARE(bi.size(), 3);

    std::ostringstream out;
    Error redirectError{&out};
    ai.front();
    ai.back();
    bi[3];
    CORRADE_COMPARE(out.str(),
        "Containers::Iterable::front(): view is empty\n"
        "Containers::Iterable::back(): view is empty\n"
        "Containers::Iterable::operator[](): index 3 out of range for 3 elements\n");
}

void IterableTest::iterator() {
    /* Mostly just a copy of StridedArrayViewTest::iterator(), reduced to a
       single dimension */

    auto&& data = IteratorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    struct {
        int value;
        int:32;
    } d[7]{{443}, {1}, {2}, {3}, {4}, {5}, {6}};

    /* Verifying also that iterators of different views and iterators of
       different strides are not comparable */
    StridedArrayView1D<int> a{d, &d[0].value, 7, data.stride};
    if(data.flipped) a = a.flipped<0>();
    StridedArrayView1D<int> b;

    Iterable<int> ai = a;
    Iterable<int> aEvery2i = a.every(2);
    Iterable<int> bi = b;

    CORRADE_VERIFY(ai.begin() == ai.begin());
    /* These are equal if stride is zero */
    CORRADE_COMPARE(ai.begin() != aEvery2i.begin(), data.stride != 0);
    CORRADE_VERIFY(ai.begin() != bi.begin());
    CORRADE_VERIFY(!(ai.begin() != ai.begin()));
    /* These are equal if stride is zero */
    CORRADE_COMPARE(!(ai.begin() == aEvery2i.begin()), data.stride != 0);
    CORRADE_VERIFY(!(ai.begin() == bi.begin()));
    CORRADE_VERIFY(ai.begin() != ai.begin() + 1);

    CORRADE_VERIFY(ai.begin() < ai.begin() + 1);
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(aEvery2i.begin() < ai.begin() + 1), data.stride != 0);
    CORRADE_VERIFY(!(ai.begin() < ai.begin()));
    CORRADE_VERIFY(ai.begin() <= ai.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(ai.begin() <= aEvery2i.begin()), data.stride != 0);
    CORRADE_VERIFY(!(ai.begin() + 1 <= ai.begin()));

    CORRADE_VERIFY(ai.begin() + 1 > ai.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(ai.begin() + 1 > aEvery2i.begin()), data.stride != 0);
    CORRADE_VERIFY(!(ai.begin() > ai.begin()));
    CORRADE_VERIFY(ai.begin() >= ai.begin());
    /* These can compare if stride is zero */
    CORRADE_COMPARE(!(ai.begin() >= aEvery2i.begin()), data.stride != 0);
    CORRADE_VERIFY(!(ai.begin() >= ai.begin() + 1));

    CORRADE_VERIFY(ai.cbegin() == ai.begin());
    CORRADE_VERIFY(ai.cbegin() != bi.begin());
    CORRADE_VERIFY(ai.cend() == ai.end());
    CORRADE_VERIFY(ai.cend() != bi.end());

    CORRADE_COMPARE(*(ai.begin() + 2), data.dataBegin1);
    CORRADE_COMPARE(*(ai.begin() += 2), data.dataBegin1);
    CORRADE_COMPARE(*(2 + ai.begin()), data.dataBegin1);
    CORRADE_COMPARE(*(ai.end() - 2), data.dataEnd1);
    CORRADE_COMPARE(*(ai.end() -= 2), data.dataEnd1);
    CORRADE_COMPARE(ai.end() - ai.begin(), ai.size());

    CORRADE_COMPARE(*(++ai.begin()), data.dataBeginIncrement1);
    CORRADE_COMPARE(*(--ai.end()), data.dataEndDecrement1);
}

void IterableTest::rangeBasedFor() {
    int data[]{7, 5, 0, -26, 33};
    Iterable<int> ai = Containers::stridedArrayView(data).slice(1, 4).flipped<0>();

    int i = 0;
    for(int& x: ai)
        x = ++i;

    CORRADE_COMPARE(data[0], 7);
    CORRADE_COMPARE(data[1], 3);
    CORRADE_COMPARE(data[2], 2);
    CORRADE_COMPARE(data[3], 1);
    CORRADE_COMPARE(data[4], 33);
}

void IterableTest::rangeBasedForReference() {
    int data0 = 7;
    int data1 = 5;
    int data2 = 0;
    int data3 = -26;
    int data4 = 33;
    Reference<int> data[]{data0, data1, data2, data3, data4};
    Iterable<int> ai = Containers::stridedArrayView(data).slice(1, 4).flipped<0>();

    int i = 0;
    for(int& x: ai)
        x = ++i;

    CORRADE_COMPARE(data0, 7);
    CORRADE_COMPARE(data1, 3);
    CORRADE_COMPARE(data2, 2);
    CORRADE_COMPARE(data3, 1);
    CORRADE_COMPARE(data4, 33);
}

struct ForwardDeclared;

void IterableTest::overloadsWithForwardDeclaredType() {
    /* If there's a set of overloads with some taking references or iterables
       of types that have just forward declarations, it should still work --
       we don't need the type (or its size) for anything here, and we
       especially don't want to be forced to include complete definitions of
       everything. */

    struct Type {
        const char* foo(int) const { return "int"; }
        const char* foo(const ForwardDeclared&) const { return "ForwardDeclared"; }
        const char* foo(const Iterable<const int>&) const { return "Iterable<int>"; }
        const char* foo(const Iterable<const ForwardDeclared>&) const { return "Iterable<ForwardDeclared>"; }
    } type;

    int b;
    const auto& a = *reinterpret_cast<const ForwardDeclared*>(&b);

    CORRADE_COMPARE(type.foo(3), "int"_s);
    CORRADE_COMPARE(type.foo({3, 7}), "Iterable<int>"_s);
    CORRADE_COMPARE(type.foo(a), "ForwardDeclared"_s);
    CORRADE_COMPARE(type.foo({a, a}), "Iterable<ForwardDeclared>"_s);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::IterableTest)
