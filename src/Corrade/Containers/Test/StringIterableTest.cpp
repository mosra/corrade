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
   String[View] or StridedArrayView being included first */
#include "Corrade/Containers/StringIterable.h"

#include <sstream>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StringIterableTest: TestSuite::Tester {
    explicit StringIterableTest();

    void constructDefault();

    template<class T> void arrayView();
    void arrayViewCharArray();
    template<class T> void arrayViewMutable();

    template<class T> void stridedArrayView();
    void stridedArrayViewCharArray();
    template<class T> void stridedArrayViewMutable();

    void initializerList();
    void cArray();
    void array();
    void stlVector();

    void access();
    void accessInvalid();

    void iterator();
    template<class T> void rangeBasedFor();

    void customIterable();
    void customIterableIndex();
    void customIterableStride();
};

using namespace Containers::Literals;

const struct {
    const char* name;
    bool flipped;
    std::ptrdiff_t stride;
    const char *dataBegin1, *dataEnd1, *dataBeginIncrement1, *dataEndDecrement1;
} IteratorData[]{
    {"", false, sizeof(const char*)*2, "2", "5", "1", "6"},
    {"zero stride", false, 0, "443", "443", "443", "443"},
    {"flipped", true, sizeof(const char*)*2, "4", "1", "5", "443"}
};

StringIterableTest::StringIterableTest() {
    addTests({&StringIterableTest::constructDefault,

              &StringIterableTest::arrayView<String>,
              &StringIterableTest::arrayView<StringView>,
              &StringIterableTest::arrayViewCharArray,
              &StringIterableTest::arrayViewMutable<MutableStringView>,
              &StringIterableTest::arrayViewMutable<char*>,

              &StringIterableTest::stridedArrayView<String>,
              &StringIterableTest::stridedArrayView<StringView>,
              &StringIterableTest::stridedArrayViewCharArray,
              &StringIterableTest::stridedArrayViewMutable<MutableStringView>,
              &StringIterableTest::stridedArrayViewMutable<char*>,

              &StringIterableTest::initializerList,
              &StringIterableTest::cArray,
              &StringIterableTest::array,
              &StringIterableTest::stlVector,

              &StringIterableTest::access,
              &StringIterableTest::accessInvalid});

    addInstancedTests({&StringIterableTest::iterator},
        Containers::arraySize(IteratorData));

    addTests<StringIterableTest>({
        &StringIterableTest::rangeBasedFor<String>,
        &StringIterableTest::rangeBasedFor<const char*>});

    addTests({&StringIterableTest::customIterable,
              &StringIterableTest::customIterableIndex,
              &StringIterableTest::customIterableStride});
}

void StringIterableTest::constructDefault() {
    StringIterable ai;
    StringIterable ai2 = nullptr;
    CORRADE_COMPARE(ai.data(), nullptr);
    CORRADE_COMPARE(ai2.data(), nullptr);
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai2.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 0);
    CORRADE_COMPARE(ai2.size(), 0);
    CORRADE_COMPARE(ai.stride(), 0);
    CORRADE_COMPARE(ai2.stride(), 0);
    CORRADE_VERIFY(ai.isEmpty());
    CORRADE_VERIFY(ai2.isEmpty());

    constexpr StringIterable cai = nullptr;
    CORRADE_COMPARE(cai.data(), nullptr);
    CORRADE_COMPARE(cai.context(), nullptr);
    CORRADE_COMPARE(cai.size(), 0);
    CORRADE_COMPARE(cai.stride(), 0);
    CORRADE_VERIFY(cai.isEmpty());
}

template<class T> struct NameTraits;
template<> struct NameTraits<String> {
    static const char* name() { return "String"; }
};
template<> struct NameTraits<StringView> {
    static const char* name() { return "StringView"; }
};
template<> struct NameTraits<MutableStringView> {
    static const char* name() { return "MutableStringView"; }
};
template<> struct NameTraits<const char*> {
    static const char* name() { return "const char*"; }
};
template<> struct NameTraits<char*> {
    static const char* name() { return "char*"; }
};

template<class T> void StringIterableTest::arrayView() {
    setTestCaseTemplateName(NameTraits<T>::name());

    T data[]{"hello"_s, "world"_s, "!\0this is here too"_s};
    ArrayView<T> a = data;

    StringIterable ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(T));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
}

void StringIterableTest::arrayViewCharArray() {
    const char* data[]{"hello", "world", "!"};
    ArrayView<const char*> a = data;

    StringIterable ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(const char*));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!");
}

template<class T> void StringIterableTest::arrayViewMutable() {
    setTestCaseTemplateName(NameTraits<T>::name());

    /* Is a separate test case because handling the \0 would be annoying */
    char hello[] = "hello";
    char world[] = "world";
    char exclamation[] = "!";
    T data[]{hello, world, exclamation};
    ArrayView<T> a = data;

    StringIterable ai = a;
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(T));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!");
}

template<class T> void StringIterableTest::stridedArrayView() {
    setTestCaseTemplateName(NameTraits<T>::name());

    T data[]{"!\0this is here too"_s, "world"_s, "hello"_s};
    StridedArrayView1D<T> a = data;

    StringIterable ai = a.template flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(T)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
}

void StringIterableTest::stridedArrayViewCharArray() {
    const char* data[]{"!", "world", "hello"};
    StridedArrayView1D<const char*> a = data;

    StringIterable ai = a.flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(const char*)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!");
}

template<class T> void StringIterableTest::stridedArrayViewMutable() {
    setTestCaseTemplateName(NameTraits<T>::name());

    /* Is a separate test case because handling the \0 would be annoying */
    char hello[] = "hello";
    char world[] = "world";
    char exclamation[] = "!";
    T data[]{exclamation, world, hello};
    StridedArrayView1D<T> a = data;

    StringIterable ai = a.template flipped<0>();
    CORRADE_COMPARE(ai.data(), static_cast<const void*>(data + 2));
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), -std::ptrdiff_t(sizeof(T)));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!");
}

void StringIterableTest::initializerList() {
    /* Capture correct function name */
    CORRADE_VERIFY(true);

    /* Capturing this way to be able to verify the contents without having to
       explicitly specify the type and without the initializer list going out
       of scope too early */
    [](const StringIterable& ai) {
        CORRADE_VERIFY(ai.data());
        CORRADE_COMPARE(ai.context(), nullptr);
        CORRADE_COMPARE(ai.size(), 3);
        /* It's always a StringView, having an initializer_list<String> etc.
           overloads would cause nasty ambiguities */
        CORRADE_COMPARE(ai.stride(), sizeof(StringView));
        CORRADE_VERIFY(!ai.isEmpty());

        CORRADE_COMPARE(ai[0], "hello");
        CORRADE_COMPARE(ai[1], "world");
        CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
    }({"hello", "world", "!\0this is here too"_s});
}

void StringIterableTest::cArray() {
    StringView data[]{"hello", "world", "!\0this is here too"_s};

    StringIterable ai = data;
    CORRADE_COMPARE(ai.data(), data);
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(StringView));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
}

void StringIterableTest::array() {
    Array<String> a{Corrade::InPlaceInit, {"hello", "world", "!\0this is here too"_s}};

    StringIterable ai = a;
    CORRADE_COMPARE(ai.data(), a.data());
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(String));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
}

void StringIterableTest::stlVector() {
    std::vector<const char*> a{{"hello", "world", "!"}};

    StringIterable ai = a;
    CORRADE_COMPARE(ai.data(), a.data());
    CORRADE_COMPARE(ai.context(), nullptr);
    CORRADE_COMPARE(ai.size(), 3);
    CORRADE_COMPARE(ai.stride(), sizeof(const char*));
    CORRADE_VERIFY(!ai.isEmpty());

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!");
}

void StringIterableTest::access() {
    StringView data[]{"!\0this is here too"_s, "world", "hello"};
    const StridedArrayView1D<StringView> a = data;
    const StringIterable ai = a.flipped<0>();

    CORRADE_COMPARE(ai.front(), "hello");
    CORRADE_COMPARE(ai.back(), "!\0this is here too"_s);

    CORRADE_COMPARE(ai[0], "hello");
    CORRADE_COMPARE(ai[1], "world");
    CORRADE_COMPARE(ai[2], "!\0this is here too"_s);
}

void StringIterableTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    const char* data[]{"hello", "world", "!"};

    StringIterable ai = Containers::arrayView(data).prefix(std::size_t{0});
    StringIterable bi = data;
    CORRADE_COMPARE(bi.size(), 3);

    std::ostringstream out;
    Error redirectError{&out};
    ai.front();
    ai.back();
    bi[3];
    CORRADE_COMPARE(out.str(),
        "Containers::StringIterable::front(): view is empty\n"
        "Containers::StringIterable::back(): view is empty\n"
        "Containers::StringIterable::operator[](): index 3 out of range for 3 elements\n");
}

void StringIterableTest::iterator() {
    /* Mostly just a copy of IterableTest::iterator(), with ints converted to
       "ints" */

    auto&& data = IteratorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    struct {
        const char* value;
        int:32; /* plus another 32 bits of padding on 64bit */
    } d[7]{{"443"}, {"1"}, {"2"}, {"3"}, {"4"}, {"5"}, {"6"}};

    /* Verifying also that iterators of different views and iterators of
       different strides are not comparable */
    StridedArrayView1D<const char*> a{d, &d[0].value, 7, data.stride};
    if(data.flipped) a = a.flipped<0>();
    StridedArrayView1D<const char*> b;

    StringIterable ai = a;
    StringIterable aEvery2i = a.every(2);
    StringIterable bi = b;

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

template<class T> void StringIterableTest::rangeBasedFor() {
    setTestCaseTemplateName(NameTraits<T>::name());

    T data[]{"7", "5", "0", "-26", "33"};
    StringIterable ai = Containers::stridedArrayView(data).slice(1, 4).template flipped<0>();

    String concatenated;
    for(StringView x: ai)
        concatenated = concatenated + x;

    CORRADE_COMPARE(concatenated, "-2605");
}

void StringIterableTest::customIterable() {
    const char* string = "eyehandnoselegear";
    int offsets[]{0, 3, 7, 11, 14, 17};

    StringIterable iterable{offsets, string, 5, sizeof(int), [](const void* data, const void* context, std::ptrdiff_t, std::size_t) {
        auto* dataI = static_cast<const int*>(data);
        return StringView{static_cast<const char*>(context) + *dataI, std::size_t(dataI[1] - dataI[0])};
    }};
    CORRADE_COMPARE(iterable.data(), offsets);
    CORRADE_COMPARE(iterable.context(), string);
    CORRADE_COMPARE(iterable.size(), 5);
    CORRADE_COMPARE(iterable.stride(), sizeof(int));
    CORRADE_COMPARE_AS(iterable, Containers::arrayView({
        "eye"_s, "hand"_s, "nose"_s, "leg"_s, "ear"_s
    }), TestSuite::Compare::Container);

    /* Verify also that the non-iterator accessors get the right numbers */
    CORRADE_COMPARE(iterable.front(), "eye");
    CORRADE_COMPARE(iterable[3], "leg");
    CORRADE_COMPARE(iterable.back(), "ear");
}

void StringIterableTest::customIterableIndex() {
    const char* string = "eyehandnoselegear";
    int offsets[]{0, 3, 7, 11, 14, 17};

    /* Like customIterable(), but supplying a zero stride so the `data` passed
       is always the same and using the index instead */
    StringIterable iterable{offsets, string, 5, 0, [](const void* data, const void* context, std::ptrdiff_t, std::size_t i) {
        auto* dataI = static_cast<const int*>(data);
        return StringView{static_cast<const char*>(context) + dataI[i], std::size_t(dataI[i + 1] - dataI[i])};
    }};
    CORRADE_COMPARE(iterable.data(), offsets);
    CORRADE_COMPARE(iterable.context(), string);
    CORRADE_COMPARE(iterable.size(), 5);
    CORRADE_COMPARE(iterable.stride(), 0);
    CORRADE_COMPARE_AS(iterable, Containers::arrayView({
        "eye"_s, "hand"_s, "nose"_s, "leg"_s, "ear"_s
    }), TestSuite::Compare::Container);

    /* Verify also that the non-iterator accessors get the right numbers */
    CORRADE_COMPARE(iterable.front(), "eye");
    CORRADE_COMPARE(iterable[3], "leg");
    CORRADE_COMPARE(iterable.back(), "ear");
}

void StringIterableTest::customIterableStride() {
    const char* string = "eyehandnoselegear";
    Containers::Pair<long, int> offsets[]{
        {0, 666},
        {3, 666},
        {7, 666},
        {11, 666},
        {14, 666},
        {17, 666}
    };

    /* Like customIterable(), but the stride is non-trivial and has to be taken
       into account when retrieving the next offset */
    StringIterable iterable{offsets, string, 5, sizeof(Containers::Pair<long, int>), [](const void* data, const void* context, std::ptrdiff_t stride, std::size_t) {
        int current = *static_cast<const int*>(data);
        int next = *reinterpret_cast<const int*>(static_cast<const char*>(data) + stride);
        return StringView{static_cast<const char*>(context) + current, std::size_t(next - current)};
    }};
    CORRADE_COMPARE(iterable.data(), offsets);
    CORRADE_COMPARE(iterable.context(), string);
    CORRADE_COMPARE(iterable.size(), 5);
    CORRADE_COMPARE(iterable.stride(), sizeof(Containers::Pair<long, int>));
    CORRADE_COMPARE_AS(iterable, Containers::arrayView({
        "eye"_s, "hand"_s, "nose"_s, "leg"_s, "ear"_s
    }), TestSuite::Compare::Container);

    /* Verify also that the non-iterator accessors get the right numbers */
    CORRADE_COMPARE(iterable.front(), "eye");
    CORRADE_COMPARE(iterable[3], "leg");
    CORRADE_COMPARE(iterable.back(), "ear");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringIterableTest)
