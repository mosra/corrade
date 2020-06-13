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

#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"

namespace {

struct StrView {
    StrView(char* data, std::size_t size): data{data}, size{size} {}

    char* data;
    std::size_t size;
};

struct ConstStrView {
    constexpr ConstStrView(const char* data, std::size_t size): data{data}, size{size} {}

    const char* data;
    std::size_t size;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct StringViewConverter<char, StrView> {
    static MutableStringView from(StrView other) {
        return MutableStringView{other.data, other.size};
    }

    static StrView to(MutableStringView other) {
        return {other.data(), other.size()};
    }
};

template<> struct StringViewConverter<const char, ConstStrView> {
    constexpr static StringView from(ConstStrView other) {
        return StringView{other.data, other.size};
    }

    constexpr static ConstStrView to(StringView other) {
        return {other.data(), other.size()};
    }
};

/* To keep the StringView API in reasonable bounds, the const-adding variants
   have to be implemented explicitly */
template<> struct StringViewConverter<const char, StrView> {
    static StringView from(StrView other) {
        return StringView{other.data, other.size};
    }
};
template<> struct StringViewConverter<char, ConstStrView> {
    constexpr static ConstStrView to(MutableStringView other) {
        return {other.data(), other.size()};
    }
};

}

namespace Test { namespace {

struct StringViewTest: TestSuite::Tester {
    explicit StringViewTest();

    template<class T> void constructDefault();
    void constructDefaultConstexpr();
    template<class T> void construct();
    void constructConstexpr();
    template<class T> void constructPointer();
    void constructPointerNull();
    void constructFromMutable();
    void constructLiteral();
    void constructLiteralEmpty();
    void constructTooLarge();

    template<class T> void convertArrayView();
    void convertExternalView();
    void convertConstFromExternalView();
    void convertToConstExternalView();

    void compareEquality();
    void compareNonEquality();

    void access();
    void accessMutable();

    void debugFlag();
    void debugFlags();
    void debug();
};

StringViewTest::StringViewTest() {
    addTests({&StringViewTest::constructDefault<const char>,
              &StringViewTest::constructDefault<char>,
              &StringViewTest::constructDefaultConstexpr,
              &StringViewTest::construct<const char>,
              &StringViewTest::construct<char>,
              &StringViewTest::constructConstexpr,
              &StringViewTest::constructPointer<const char>,
              &StringViewTest::constructPointer<char>,
              &StringViewTest::constructPointerNull,
              &StringViewTest::constructFromMutable,
              &StringViewTest::constructLiteral,
              &StringViewTest::constructLiteralEmpty,
              &StringViewTest::constructTooLarge,

              &StringViewTest::convertArrayView<const char>,
              &StringViewTest::convertArrayView<char>,
              &StringViewTest::convertExternalView,
              &StringViewTest::convertConstFromExternalView,
              &StringViewTest::convertToConstExternalView,

              &StringViewTest::compareEquality,
              &StringViewTest::compareNonEquality,

              &StringViewTest::access,
              &StringViewTest::accessMutable,

              &StringViewTest::debugFlag,
              &StringViewTest::debugFlags,
              &StringViewTest::debug});
}

template<class> struct NameFor;
template<> struct NameFor<const char> {
    static const char* name() { return "StringView"; }
};
template<> struct NameFor<char> {
    static const char* name() { return "MutableStringView"; }
};

template<class T> void StringViewTest::constructDefault() {
    setTestCaseTemplateName(NameFor<T>::name());

    const BasicStringView<T> view;
    CORRADE_VERIFY(view.isEmpty());
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlag::Global);
    CORRADE_COMPARE(static_cast<const void*>(view.data()), nullptr);

    CORRADE_VERIFY((std::is_nothrow_default_constructible<BasicStringView<T>>::value));
}

void StringViewTest::constructDefaultConstexpr() {
    constexpr StringView view;
    constexpr bool empty = view.isEmpty();
    constexpr std::size_t size = view.size();
    constexpr StringViewFlags flags = view.flags();
    constexpr const void* data = view.data();
    CORRADE_VERIFY(empty);
    CORRADE_COMPARE(size, 0);
    CORRADE_COMPARE(flags, StringViewFlag::Global);
    CORRADE_COMPARE(data, nullptr);
}

template<class T> void StringViewTest::construct() {
    setTestCaseTemplateName(NameFor<T>::name());

    char string[]{'h', 'e', 'l', 'l', '\0', '!', '!'}; /* 7 chars */
    const BasicStringView<T> view{string, 6};
    CORRADE_VERIFY(!view.isEmpty());
    CORRADE_COMPARE(view.size(), 6);
    CORRADE_COMPARE(view.flags(), StringViewFlags{});
    CORRADE_COMPARE(static_cast<const void*>(view.data()), &string[0]);

    CORRADE_VERIFY((std::is_nothrow_constructible<BasicStringView<T>, T*, std::size_t, StringViewFlags>::value));
}

void StringViewTest::constructConstexpr() {
    constexpr const char* string = "hell\0!!"; /* 7 chars + \0 at the end */
    constexpr StringView view = {string, 6, StringViewFlag::Global|StringViewFlag::NullTerminated};
    constexpr bool empty = view.isEmpty();
    constexpr std::size_t size = view.size();
    constexpr StringViewFlags flags = view.flags();
    constexpr const void* data = view.data();
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(size, 6);
    CORRADE_COMPARE(flags, StringViewFlag::Global|StringViewFlag::NullTerminated);
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1910 && defined(_DEBUG)
        CORRADE_EXPECT_FAIL("MSVC 2017+ does some crazy shit with constexpr data. But only in Debug builds.");
        #endif
        CORRADE_COMPARE(data, string);
    }
}

template<class T> void StringViewTest::constructPointer() {
    setTestCaseTemplateName(NameFor<T>::name());

    char string[] = "hello\0world!";
    const BasicStringView<T> view = string;
    CORRADE_COMPARE(view.size(), 5); /* stops at the first null terminator */
    CORRADE_COMPARE(view.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(static_cast<const void*>(view.data()), &string[0]);

    CORRADE_VERIFY((std::is_nothrow_constructible<BasicStringView<T>, T*>::value));
}

void StringViewTest::constructPointerNull() {
    StringView view = nullptr;
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlags{});
    CORRADE_COMPARE(static_cast<const void*>(view.data()), nullptr);
}

void StringViewTest::constructFromMutable() {
    char string[] = "hello\0world!";
    const MutableStringView a = string;
    const StringView b = a;
    CORRADE_COMPARE(b.size(), 5); /* stops at the first null terminator */
    CORRADE_COMPARE(b.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(static_cast<const void*>(b.data()), &string[0]);

    CORRADE_VERIFY((std::is_nothrow_constructible<StringView, MutableStringView>::value));

    /* It shouldn't be possible the other way around */
    CORRADE_VERIFY((std::is_convertible<MutableStringView, StringView>::value));
    CORRADE_VERIFY(!(std::is_convertible<StringView, MutableStringView>::value));
}

void StringViewTest::constructLiteral() {
    using namespace Literals;

    StringView view = "hell\0!"_s;
    CORRADE_COMPARE(view.size(), 6);
    CORRADE_COMPARE(view.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_COMPARE(view.data()[2], 'l');

    constexpr StringView cview = "hell\0!"_s;
    CORRADE_COMPARE(cview.size(), 6);
    CORRADE_COMPARE(cview.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_COMPARE(cview.data()[2], 'l');
}

void StringViewTest::constructLiteralEmpty() {
    using namespace Literals;

    StringView view = ""_s;
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_COMPARE(view.data()[0], '\0');
}

void StringViewTest::constructTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    StringView{nullptr, ~std::size_t{}};
    CORRADE_COMPARE(out.str(), sizeof(std::size_t) == 4 ?
        "Containers::StringView: string expected to be smaller than 2^30 bytes, got 4294967295\n" :
        "Containers::StringView: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
}

template<class T> void StringViewTest::convertArrayView() {
    setTestCaseTemplateName(NameFor<T>::name());

    char data[] = "hello!";
    ArrayView<T> array = data;
    CORRADE_COMPARE(array.size(), 7); /* includes the null terminator */

    BasicStringView<T> string = array;
    CORRADE_COMPARE(string.size(), 7); /* keeps the same size */
    CORRADE_COMPARE(string.flags(), StringViewFlags{});
    CORRADE_COMPARE(static_cast<const void*>(string.data()), &data[0]);

    BasicStringView<T> string2 = {array, StringViewFlag::NullTerminated};
    CORRADE_COMPARE(string2.size(), 7); /* keeps the same size */
    CORRADE_COMPARE(string2.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(static_cast<const void*>(string2.data()), &data[0]);

    ArrayView<T> array2 = string;
    CORRADE_COMPARE(array2.size(), 7); /* keeps the same size */
    CORRADE_COMPARE(static_cast<const void*>(array2.data()), &data[0]);
}

void StringViewTest::convertExternalView() {
    char data[]{'h', 'e', 'l', 'l', 'o'};
    StrView a{data, 5};
    CORRADE_COMPARE(static_cast<const void*>(a.data), data);
    CORRADE_COMPARE(a.size, 5);

    MutableStringView b = a;
    CORRADE_COMPARE(static_cast<const void*>(b.data()), data);
    CORRADE_COMPARE(b.size(), 5);

    StrView c = b;
    CORRADE_COMPARE(static_cast<const void*>(c.data), data);
    CORRADE_COMPARE(c.size, 5);

    constexpr const char* cdata = "hello world!";
    constexpr ConstStrView ca{cdata, 12};
    CORRADE_COMPARE(ca.data, StringView{"hello world!"});
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1910 && defined(_DEBUG)
        CORRADE_EXPECT_FAIL("MSVC 2017+ does some crazy shit with constexpr data. But only in Debug builds.");
        #endif
        CORRADE_COMPARE(static_cast<const void*>(ca.data), cdata);
    }
    CORRADE_COMPARE(ca.size, 12);

    constexpr StringView cb = ca;
    CORRADE_COMPARE(cb, StringView{"hello world!"});
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1910 && defined(_DEBUG)
        CORRADE_EXPECT_FAIL("MSVC 2017+ does some crazy shit with constexpr data. But only in Debug builds.");
        #endif
        CORRADE_COMPARE(static_cast<const void*>(ca.data), cdata);
    }
    CORRADE_COMPARE(cb.size(), 12);

    constexpr ConstStrView cc = cb;
    CORRADE_COMPARE(cc.data, StringView{"hello world!"});
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1910 && defined(_DEBUG)
        CORRADE_EXPECT_FAIL("MSVC 2017+ does some crazy shit with constexpr data. But only in Debug builds.");
        #endif
        CORRADE_COMPARE(static_cast<const void*>(ca.data), cdata);
    }
    CORRADE_COMPARE(cc.size, 12);
}

void StringViewTest::convertConstFromExternalView() {
    char data[]{'h', 'e', 'l', 'l', 'o'};
    StrView a{data, 5};
    CORRADE_COMPARE(static_cast<const void*>(a.data), data);
    CORRADE_COMPARE(a.size, 5);

    StringView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);
}

void StringViewTest::convertToConstExternalView() {
    char data[] = "hello";
    MutableStringView a = data;
    CORRADE_COMPARE(static_cast<const void*>(a.data()), data);
    CORRADE_COMPARE(a.size(), 5);

    ConstStrView b = a;
    CORRADE_COMPARE(b.data, data);
    CORRADE_COMPARE(b.size, 5);
}

void StringViewTest::compareEquality() {
    /* Trivial case */
    StringView a = "hello";
    CORRADE_VERIFY(a == a);

    /* One is null-terminated, the other is a substring, but should compare
       equal */
    const char bData[]{'h', 'e', 'l', 'l', 'o', '3'};
    StringView b{bData, 5};
    CORRADE_VERIFY(b == b);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);

    /* Verify we don't just compare a common prefix */
    StringView c = "hello!";
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);

    /* Comparison with an empty view (which is nullptr) */
    StringView empty;
    CORRADE_VERIFY(empty == empty);
    CORRADE_VERIFY(a != empty);
    CORRADE_VERIFY(empty != a);

    /* Null terminator in the middle -- it should not stop at it */
    {
        using namespace Literals;
        CORRADE_VERIFY("hello\0world"_s == (StringView{"hello\0world!", 11}));
        CORRADE_VERIFY("hello\0wOrld"_s != (StringView{"hello\0world!", 11}));
    }

    /* C strings on either side */
    CORRADE_VERIFY(a == "hello");
    CORRADE_VERIFY("hello" == a);
    CORRADE_VERIFY(c != "hello");
    CORRADE_VERIFY("hello" != c);

    /* Comparing mutable / immutable views */
    char dData[] = "hello";
    MutableStringView d = dData;
    char eData[] = "hello!";
    MutableStringView e = eData;
    CORRADE_VERIFY(a == d);
    CORRADE_VERIFY(a != e);
    CORRADE_VERIFY(d == a);
    CORRADE_VERIFY(e != a);

    /* Mutable views and immutable C strings */
    CORRADE_VERIFY(d == "hello");
    CORRADE_VERIFY(e != "hello");
    CORRADE_VERIFY("hello" == d);
    CORRADE_VERIFY("hello" != e);
}

void StringViewTest::compareNonEquality() {
    /* Test same length w/ data difference and also same prefix + extra data */
    StringView a = "hell";
    StringView b = "hella";
    StringView hello = "hello";
    StringView c = "hello";
    StringView d = "helly";
    StringView e = "hello!";

    /* Less than */
    CORRADE_VERIFY(a < hello);      CORRADE_VERIFY(!(hello < a));
    CORRADE_VERIFY(b < hello);      CORRADE_VERIFY(!(hello < b));
    CORRADE_VERIFY(!(hello < c));   CORRADE_VERIFY(!(c < hello));
    CORRADE_VERIFY(hello < d);      CORRADE_VERIFY(!(d < hello));
    CORRADE_VERIFY(hello < e);      CORRADE_VERIFY(!(e < hello));

    /* Less than or equal */
    CORRADE_VERIFY(a <= hello);     CORRADE_VERIFY(!(hello <= a));
    CORRADE_VERIFY(b <= hello);     CORRADE_VERIFY(!(hello <= b));
    CORRADE_VERIFY(hello <= c);     CORRADE_VERIFY(c <= hello);
    CORRADE_VERIFY(hello <= d);     CORRADE_VERIFY(!(d <= hello));
    CORRADE_VERIFY(hello <= e);     CORRADE_VERIFY(!(e <= hello));

    /* Greater than or equal */
    CORRADE_VERIFY(!(a >= hello));  CORRADE_VERIFY(hello >= a);
    CORRADE_VERIFY(!(b >= hello));  CORRADE_VERIFY(hello >= b);
    CORRADE_VERIFY(hello >= c);     CORRADE_VERIFY(c >= hello);
    CORRADE_VERIFY(!(hello >= d));  CORRADE_VERIFY(d >= hello);
    CORRADE_VERIFY(!(hello >= e));  CORRADE_VERIFY(e >= hello);

    /* Greater than */
    CORRADE_VERIFY(!(a > hello));  CORRADE_VERIFY(hello > a);
    CORRADE_VERIFY(!(b > hello));  CORRADE_VERIFY(hello > b);
    CORRADE_VERIFY(!(hello > c));  CORRADE_VERIFY(!(c > hello));
    CORRADE_VERIFY(!(hello > d));  CORRADE_VERIFY(d > hello);
    CORRADE_VERIFY(!(hello > e));  CORRADE_VERIFY(e > hello);

    /* Comparing with an empty view should also work */
    CORRADE_VERIFY(!(StringView{} < StringView{}));
    CORRADE_VERIFY(StringView{} < hello);
    CORRADE_VERIFY(StringView{} <= hello);
    CORRADE_VERIFY(StringView{} <= StringView{});
    CORRADE_VERIFY(StringView{} >= StringView{});
    CORRADE_VERIFY(hello >= StringView{});
    CORRADE_VERIFY(hello > StringView{});
    CORRADE_VERIFY(!(StringView{} > StringView{}));
}

void StringViewTest::access() {
    /* Use the flags so we ensure the size is always properly masked out */
    const char* string = "hello\0world!";
    const StringView view{string, 12, StringViewFlag::Global|StringViewFlag::NullTerminated};
    CORRADE_COMPARE(*view.begin(), 'h');
    CORRADE_COMPARE(*view.cbegin(), 'h');
    CORRADE_COMPARE(*(view.end() - 1), '!');
    CORRADE_COMPARE(*(view.cend() - 1), '!');
    CORRADE_COMPARE(view[6], 'w');
}

void StringViewTest::accessMutable() {
    /* Use the flags so we ensure the size is always properly masked out */
    char string[] = "hello\0world!";
    const MutableStringView view{string, 12, StringViewFlag::Global|StringViewFlag::NullTerminated};
    view[5] = ' ';
    *view.begin() = 'I';
    ++*view.begin();
    *(view.end() - 1) = '>';
    ++*(view.cend() - 1);
    CORRADE_COMPARE(view, StringView{"Jello world?"});
}

void StringViewTest::debugFlag() {
    std::ostringstream out;

    Debug{&out} << StringViewFlag::Global << StringViewFlag(0xf0f0u);
    CORRADE_COMPARE(out.str(), "Containers::StringViewFlag::Global Containers::StringViewFlag(0xf0f0)\n");
}

void StringViewTest::debugFlags() {
    std::ostringstream out;

    Debug{&out} << (Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated) << Containers::StringViewFlags{};
    CORRADE_COMPARE(out.str(), "Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated Containers::StringViewFlags{}\n");
}

void StringViewTest::debug() {
    using namespace Literals;

    std::ostringstream out;
    /* The operator<< is implemented directly in Debug, testing here to have
       everything together */
    Debug{&out} << "lolwat, using iostream to\0test string views?!"_s;
    CORRADE_COMPARE(out.str(), (std::string{"lolwat, using iostream to\0test string views?!\n", 46}));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringViewTest)
