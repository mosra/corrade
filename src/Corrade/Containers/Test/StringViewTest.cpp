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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
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
    void constructEmpty();
    void constructNullptr();
    void constructFromMutable();
    void constructLiteral();
    void constructLiteralEmpty();
    void constructTooLarge();

    template<class T> void convertArrayView();
    template<class T> void convertVoidArrayView();
    void convertExternalView();
    void convertConstFromExternalView();
    void convertToConstExternalView();

    void compareEquality();
    void compareNonEquality();

    void access();
    void accessMutable();
    void accessInvalid();

    void sliceInvalid();
    void sliceNullptr();
    void slice();
    void slicePointer();
    void sliceFlags();

    void split();
    void splitFlags();
    void splitMultipleCharacters();
    void splitMultipleCharactersFlags();
    void splitNullView();

    void partition();
    void partitionFlags();

    void hasPrefix();
    void hasPrefixEmpty();
    void hasSuffix();
    void hasSuffixEmpty();

    void stripPrefix();
    void stripPrefixFlags();
    void stripPrefixInvalid();
    void stripSuffix();
    void stripSuffixFlags();
    void stripSuffixInvalid();

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
              &StringViewTest::constructEmpty,
              &StringViewTest::constructNullptr,
              &StringViewTest::constructFromMutable,
              &StringViewTest::constructLiteral,
              &StringViewTest::constructLiteralEmpty,
              &StringViewTest::constructTooLarge,

              &StringViewTest::convertArrayView<const char>,
              &StringViewTest::convertArrayView<char>,
              &StringViewTest::convertVoidArrayView<const char>,
              &StringViewTest::convertVoidArrayView<char>,
              &StringViewTest::convertExternalView,
              &StringViewTest::convertConstFromExternalView,
              &StringViewTest::convertToConstExternalView,

              &StringViewTest::compareEquality,
              &StringViewTest::compareNonEquality,

              &StringViewTest::access,
              &StringViewTest::accessMutable,
              &StringViewTest::accessInvalid,

              &StringViewTest::sliceInvalid,
              &StringViewTest::sliceNullptr,
              &StringViewTest::slice,
              &StringViewTest::slicePointer,
              &StringViewTest::sliceFlags,

              &StringViewTest::split,
              &StringViewTest::splitFlags,
              &StringViewTest::splitMultipleCharacters,
              &StringViewTest::splitMultipleCharactersFlags,
              &StringViewTest::splitNullView,

              &StringViewTest::partition,
              &StringViewTest::partitionFlags,

              &StringViewTest::hasPrefix,
              &StringViewTest::hasPrefixEmpty,
              &StringViewTest::hasSuffix,
              &StringViewTest::hasSuffixEmpty,

              &StringViewTest::stripPrefix,
              &StringViewTest::stripPrefixFlags,
              &StringViewTest::stripPrefixInvalid,
              &StringViewTest::stripSuffix,
              &StringViewTest::stripSuffixFlags,
              &StringViewTest::stripSuffixInvalid,

              &StringViewTest::debugFlag,
              &StringViewTest::debugFlags,
              &StringViewTest::debug});
}

using namespace Literals;

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
    StringView view = static_cast<const char*>(nullptr);
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlags{});
    CORRADE_COMPARE(static_cast<const void*>(view.data()), nullptr);
}

void StringViewTest::constructEmpty() {
    StringView view = "";
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlag::NullTerminated);
    CORRADE_VERIFY(view.data());
    CORRADE_COMPARE(view.data()[0], '\0');
}

void StringViewTest::constructNullptr() {
    /* It's the default constructor, just with the default argument explicit */

    StringView view = nullptr;
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlag::Global);
    CORRADE_COMPARE(static_cast<const void*>(view.data()), nullptr);

    constexpr StringView cview = nullptr;
    CORRADE_COMPARE(cview.size(), 0);
    CORRADE_COMPARE(cview.flags(), StringViewFlag::Global);
    CORRADE_COMPARE(static_cast<const void*>(cview.data()), nullptr);
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
    StringView view = ""_s;
    CORRADE_COMPARE(view.size(), 0);
    CORRADE_COMPARE(view.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_VERIFY(view.data());
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

template<class T> void StringViewTest::convertVoidArrayView() {
    using VoidT = typename std::conditional<std::is_const<T>::value, const void, void>::type;

    setTestCaseTemplateName(NameFor<T>::name());

    char data[] = "hello!";
    BasicStringView<T> string = data;
    CORRADE_COMPARE(string.size(), 6); /* without the null terminator */
    CORRADE_COMPARE(string.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(static_cast<const void*>(string.data()), &data[0]);

    ArrayView<VoidT> array = string;
    CORRADE_COMPARE(array.size(), 6); /* keeps the same size */
    CORRADE_COMPARE(static_cast<const void*>(array.data()), &data[0]);
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

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    StringView cb = ca;
    CORRADE_COMPARE(cb, StringView{"hello world!"});
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1910 && defined(_DEBUG)
        CORRADE_EXPECT_FAIL("MSVC 2017+ does some crazy shit with constexpr data. But only in Debug builds.");
        #endif
        CORRADE_COMPARE(static_cast<const void*>(ca.data), cdata);
    }
    CORRADE_COMPARE(cb.size(), 12);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstStrView cc = cb;
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
    CORRADE_VERIFY("hello\0world"_s == (StringView{"hello\0world!", 11}));
    CORRADE_VERIFY("hello\0wOrld"_s != (StringView{"hello\0world!", 11}));

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
    CORRADE_COMPARE(view.front(), 'h');
    CORRADE_COMPARE(*(view.end() - 1), '!');
    CORRADE_COMPARE(*(view.cend() - 1), '!');
    CORRADE_COMPARE(view.back(), '!');
    CORRADE_COMPARE(view[6], 'w');
}

void StringViewTest::accessMutable() {
    /* Use the flags so we ensure the size is always properly masked out */
    char string[] = "hello\0world!";
    const MutableStringView view{string, 12, StringViewFlag::Global|StringViewFlag::NullTerminated};
    view[5] = ' ';
    *view.begin() = 'I';
    ++*view.begin();
    ++view.front();
    *(view.end() - 1) = '>';
    ++*(view.cend() - 1);
    ++view.back();
    CORRADE_COMPARE(view, StringView{"Kello world@"});
}

void StringViewTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    /* Use an empty literal to have flags set, testing that the implementation
       uses size() and not _size */
    StringView a = ""_s;
    CORRADE_VERIFY(a.flags());

    a.front();
    a.back();
    CORRADE_COMPARE(out.str(),
        "Containers::StringView::front(): view is empty\n"
        "Containers::StringView::back(): view is empty\n");
}

void StringViewTest::sliceInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    /* Basically the same as in ArrayViewTest::sliceInvalid() */

    /* Do it this way to avoid (reasonable) warnings about out-of-bounds array
       access with `a - 1`. Also use the flags so we ensure the size is always
       properly masked out. */
    const char* data = "Bhello";
    StringView a{data + 1, 5, StringViewFlag::Global|StringViewFlag::NullTerminated};

    std::ostringstream out;
    Error redirectError{&out};

    /* Testing both pointer and size versions */
    a.slice(a.data() - 1, a.data());
    a.slice(a.data() + 5, a.data() + 6);
    a.slice(5, 6);
    a.slice(a.data() + 2, a.data() + 1);
    a.slice(2, 1);

    CORRADE_COMPARE(out.str(),
        "Containers::StringView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::StringView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StringView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StringView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::StringView::slice(): slice [2:1] out of range for 5 elements\n");
}

void StringViewTest::sliceNullptr() {
    /* Basically the same as in ArrayViewTest::sliceNullptr() -- we want the
       same semantics as this is useful for parsers */

    MutableStringView a{nullptr, 5};

    MutableStringView b = a.prefix(nullptr);
    CORRADE_VERIFY(!b.data());
    CORRADE_COMPARE(b.size(), 0);

    MutableStringView c = a.suffix(nullptr);
    CORRADE_VERIFY(!c.data());
    CORRADE_COMPARE(c.size(), 5);

    constexpr MutableStringView ca{nullptr, 5};

    constexpr MutableStringView cb = ca.prefix(nullptr);
    CORRADE_VERIFY(!cb.data());
    CORRADE_COMPARE(cb.size(), 0);

    /* constexpr MutableStringView cc = ca.suffix(nullptr) won't compile because
       arithmetic on nullptr is not allowed */

    char data[5];
    MutableStringView d{data, 5};

    MutableStringView e = d.prefix(nullptr);
    CORRADE_VERIFY(!e.data());
    CORRADE_COMPARE(e.size(), 0);

    MutableStringView f = d.suffix(nullptr);
    CORRADE_VERIFY(!f.data());
    CORRADE_COMPARE(f.size(), 0);

    constexpr StringView cd = "things"_s;
    constexpr StringView ce = cd.prefix(nullptr);
    CORRADE_VERIFY(!ce.data());
    CORRADE_COMPARE(ce.size(), 0);

    constexpr StringView cf = cd.suffix(nullptr);
    CORRADE_VERIFY(!cf.data());
    CORRADE_COMPARE(cf.size(), 0);
}

void StringViewTest::slice() {
    /* Use the flags so we ensure the size is always properly masked out */
    char data[] = "hello";
    MutableStringView a{data, 5, StringViewFlag::Global|StringViewFlag::NullTerminated};

    CORRADE_COMPARE(a.slice(1, 4), "ell"_s);
    CORRADE_COMPARE(a.prefix(3), "hel"_s);
    CORRADE_COMPARE(a.except(2), "hel"_s);
    CORRADE_COMPARE(a.suffix(2), "llo"_s);

    constexpr StringView ca = "hello"_s;
    constexpr StringView cb = ca.slice(1, 4);
    CORRADE_COMPARE(cb, "ell");

    constexpr StringView cc1 = ca.prefix(3);
    constexpr StringView cc2 = ca.except(2);
    CORRADE_COMPARE(cc1, "hel");
    CORRADE_COMPARE(cc2, "hel");

    constexpr StringView cd = ca.suffix(2);
    CORRADE_COMPARE(cd, "llo");
}

void StringViewTest::slicePointer() {
    /* Use the flags so we ensure the size is always properly masked out */
    char data[] = "hello";
    MutableStringView a{data, 5, StringViewFlag::Global|StringViewFlag::NullTerminated};

    CORRADE_COMPARE(a.slice(data + 1, data + 4), "ell"_s);
    CORRADE_COMPARE(a.prefix(data + 3), "hel"_s);
    CORRADE_COMPARE(a.suffix(data + 2), "llo"_s);

    /* Not constexpr on MSVC 2015; not constexpr on GCC 4.8 (probably because
       of arithmetic on the C string literal). Interestingly enough, `cb`
       compiles fine locally but not on Travis CI (both have 4.8.5). */
    #if !(defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER <= 1900) && !(defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5)
    constexpr const char* cdata = "hello";
    constexpr StringView ca{cdata, 5};
    constexpr StringView cb = ca.slice(cdata + 1, cdata + 4);
    CORRADE_COMPARE(cb, "ell");

    constexpr StringView cc = ca.prefix(cdata + 3);
    CORRADE_COMPARE(cc, "hel");

    constexpr StringView cd = ca.suffix(cdata + 2);
    CORRADE_COMPARE(cd, "llo");
    #endif
}

void StringViewTest::sliceFlags() {
    StringView globalNullTerminated = "hello"_s;
    CORRADE_COMPARE(globalNullTerminated.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    StringView nullTerminated = "hello";
    CORRADE_COMPARE(nullTerminated.flags(), StringViewFlag::NullTerminated);

    StringView none{"hello", 5};
    CORRADE_COMPARE(none.flags(), StringViewFlags{});

    /* Null-terminated flag stays if it's a suffix */
    CORRADE_COMPARE(globalNullTerminated.prefix(5).flags(),
        StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_COMPARE(globalNullTerminated.prefix(globalNullTerminated.data() + 5).flags(),
        StringViewFlag::Global|StringViewFlag::NullTerminated);

    CORRADE_COMPARE(nullTerminated.prefix(5).flags(),
        StringViewFlag::NullTerminated);
    CORRADE_COMPARE(nullTerminated.prefix(nullTerminated.data() + 5).flags(),
        StringViewFlag::NullTerminated);

    CORRADE_COMPARE(none.prefix(5).flags(), StringViewFlags{});
    CORRADE_COMPARE(none.prefix(none.data() + 5).flags(), StringViewFlags{});

    /* Global flag stays always */
    CORRADE_COMPARE(globalNullTerminated.prefix(4).flags(),
        StringViewFlag::Global);
    CORRADE_COMPARE(globalNullTerminated.prefix(globalNullTerminated.data() + 4).flags(),
        StringViewFlag::Global);

    CORRADE_COMPARE(nullTerminated.prefix(4).flags(), StringViewFlags{});
    CORRADE_COMPARE(nullTerminated.prefix(nullTerminated.data() + 4).flags(), StringViewFlags{});

    CORRADE_COMPARE(none.prefix(4).flags(), StringViewFlags{});
    CORRADE_COMPARE(none.prefix(none.data() + 4).flags(), StringViewFlags{});
}

void StringViewTest::split() {
    /* Empty */
    CORRADE_COMPARE_AS(""_s.split('/'),
        Array<StringView>{},
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(""_s.splitWithoutEmptyParts('/'),
        Array<StringView>{},
        TestSuite::Compare::Container);

    /* Only delimiter */
    CORRADE_COMPARE_AS("/"_s.split('/'),
        arrayView({""_s, ""_s}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS("/"_s.splitWithoutEmptyParts('/'),
        Array<StringView>{},
        TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS("abcdef"_s.split('/'),
        arrayView({"abcdef"_s}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS("abcdef"_s.split('/'),
        arrayView({"abcdef"_s}),
        TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS("ab/c/def"_s.split('/'),
        arrayView({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS("ab/c/def"_s.splitWithoutEmptyParts('/'),
        arrayView({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS("ab//c/def//"_s.split('/'),
        arrayView({"ab"_s, ""_s, "c"_s, "def"_s, ""_s, ""_s}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS("ab//c/def//"_s.splitWithoutEmptyParts('/'),
        arrayView({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);
}

void StringViewTest::splitFlags() {
    /* All flags come from the slice() implementation, so just verify the edge
       cases */

    /* Usual case -- all global, only the last null-terminated */
    {
        Array<StringView> a = "a/b/c"_s.split('/');
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s, "c"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    } {
        Array<StringView> a = "a/b///c"_s.splitWithoutEmptyParts('/');
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s, "c"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    }

    /* Found at the end -- last empty (if not skipped) is null-terminated */
    {
        Array<StringView> a = "a/b/"_s.split('/');
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s, ""_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    } {
        Array<StringView> a = "a/b//"_s.splitWithoutEmptyParts('/');
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
    }

    /* Not found -- the only item is null-terminated */
    {
        Array<StringView> a = "ab"_s.split('/');
        CORRADE_COMPARE_AS(a, arrayView({"ab"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    } {
        Array<StringView> a = "ab"_s.splitWithoutEmptyParts('/');
        CORRADE_COMPARE_AS(a, arrayView({"ab"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    }
}

void StringViewTest::splitMultipleCharacters() {
    constexpr Containers::StringView delimiters = ".:;"_s;

    /* Empty */
    CORRADE_COMPARE_AS(""_s.splitWithoutEmptyParts(delimiters),
        Array<StringView>{},
        TestSuite::Compare::Container);

    /* Only delimiters */
    CORRADE_COMPARE_AS(delimiters.splitWithoutEmptyParts(delimiters),
        Array<StringView>{},
        TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS("abcdef"_s.splitWithoutEmptyParts(delimiters),
        array({"abcdef"_s}),
        TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS("ab:c;def"_s.splitWithoutEmptyParts(delimiters),
        array({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS("ab:c;;def."_s.splitWithoutEmptyParts(delimiters),
        array({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);

    /* Default is whitespace */
    CORRADE_COMPARE_AS("ab c  \t \ndef\r"_s.splitWithoutEmptyParts(),
        array({"ab"_s, "c"_s, "def"_s}),
        TestSuite::Compare::Container);
}

void StringViewTest::splitMultipleCharactersFlags() {
    constexpr Containers::StringView delimiters = ".:;"_s;

    /* All flags come from the slice() implementation, so just verify the edge
       cases */

    /* Usual case -- all global, only the last null-terminated */
    {
        Array<StringView> a = "a.:b;c"_s.splitWithoutEmptyParts(delimiters);
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s, "c"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    /* Found at the end -- last is not null-terminated because there are
       characters after */
    } {
        Array<StringView> a = "a.b;::"_s.splitWithoutEmptyParts(delimiters);
        CORRADE_COMPARE_AS(a, arrayView({"a"_s, "b"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);

    /* Not found -- the only item is null-terminated */
    } {
        Array<StringView> a = "ab"_s.splitWithoutEmptyParts(delimiters);
        CORRADE_COMPARE_AS(a, arrayView({"ab"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    }
}

void StringViewTest::splitNullView() {
    CORRADE_COMPARE_AS(StringView{}.split(' '),
        Array<StringView>{},
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(StringView{}.splitWithoutEmptyParts(' '),
        Array<StringView>{},
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(StringView{}.splitWithoutEmptyParts(" "),
        Array<StringView>{},
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(StringView{}.splitWithoutEmptyParts(),
        Array<StringView>{},
        TestSuite::Compare::Container);
}

void StringViewTest::partition() {
    /* Happy case */
    CORRADE_COMPARE_AS("ab=c"_s.partition('='),
        (Array3<StringView>{"ab", "=", "c"}),
        TestSuite::Compare::Container);

    /* Two occurences */
    CORRADE_COMPARE_AS("ab=c=d"_s.partition('='),
        (Array3<StringView>{"ab", "=", "c=d"}),
        TestSuite::Compare::Container);

    /* Not found */
    CORRADE_COMPARE_AS("abc"_s.partition('='),
        (Array3<StringView>{"abc", "", ""}),
        TestSuite::Compare::Container);

    /* Empty string -- all are non-null */
    CORRADE_COMPARE_AS(""_s.partition('='),
        (Array3<StringView>{"", "", ""}),
        TestSuite::Compare::Container);
    for(StringView a: ""_s.partition('='))
        CORRADE_VERIFY(a.data());

    /* Nullptr string -- all are non-null */
    CORRADE_COMPARE_AS(StringView{}.partition('='),
        (Array3<StringView>{"", "", ""}),
        TestSuite::Compare::Container);
    for(StringView a: StringView{}.partition('='))
        CORRADE_VERIFY(!a.data());
}

void StringViewTest::partitionFlags() {
    /* All flags come from the slice() implementation, so just verify the edge
       cases */

    /* Usual case -- all global, only the last null-terminated */
    {
        Array3<StringView> a = "ab=c"_s.partition('=');
        CORRADE_COMPARE_AS(a, (Array3<StringView>{"ab", "=", "c"}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    /* Found at the end -- last two null-terminated */
    } {
        Array3<StringView> a = "ab="_s.partition('=');
        CORRADE_COMPARE_AS(a, (Array3<StringView>{"ab", "=", ""}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    /* Not found -- all three null-terminated */
    } {
        Array3<StringView> a = "ab"_s.partition('=');
        CORRADE_COMPARE_AS(a, (Array3<StringView>{"ab", "", ""}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    /* Empty -- all three null-terminated as well */
    } {
        Array3<StringView> a = ""_s.partition('=');
        CORRADE_COMPARE_AS(a, (Array3<StringView>{"", "", ""}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a[1].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a[2].flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    /* Null pointer -- no flags (a literal nullptr would have the Global flag
       set) */
    } {
        const char* zero{};
        Array3<StringView> a = StringView{zero}.partition('=');
        CORRADE_COMPARE_AS(a, (Array3<StringView>{"", "", ""}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(a[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(a[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(a[2].flags(), StringViewFlags{});
    }
}

void StringViewTest::hasPrefix() {
    CORRADE_VERIFY("overcomplicated"_s.hasPrefix("over"));
    CORRADE_VERIFY(!"overcomplicated"_s.hasPrefix("oven"));
}

void StringViewTest::hasPrefixEmpty() {
    CORRADE_VERIFY(!""_s.hasPrefix("overcomplicated"));
    CORRADE_VERIFY("overcomplicated"_s.hasPrefix(""));
    CORRADE_VERIFY(""_s.hasPrefix(""));
}

void StringViewTest::hasSuffix() {
    CORRADE_VERIFY("overcomplicated"_s.hasSuffix("complicated"));
    CORRADE_VERIFY(!"overcomplicated"_s.hasSuffix("somplicated"));
    CORRADE_VERIFY(!"overcomplicated"_s.hasSuffix("overcomplicated even more"));
}

void StringViewTest::hasSuffixEmpty() {
    CORRADE_VERIFY(!""_s.hasSuffix("overcomplicated"));
    CORRADE_VERIFY("overcomplicated"_s.hasSuffix(""));
    CORRADE_VERIFY(""_s.hasSuffix(""));
}

void StringViewTest::stripPrefix() {
    CORRADE_COMPARE("overcomplicated"_s.stripPrefix("over"), "complicated");
    CORRADE_COMPARE("overcomplicated"_s.stripPrefix(""), "overcomplicated");

    /* Only a null view results in a null output */
    CORRADE_VERIFY(""_s.stripPrefix("").data());
    CORRADE_VERIFY(!StringView{}.stripPrefix("").data());
}

void StringViewTest::stripPrefixFlags() {
    CORRADE_COMPARE("overcomplicated"_s.stripPrefix("over").flags(),
        StringViewFlag::Global|StringViewFlag::NullTerminated);
    CORRADE_COMPARE("overcomplicated"_s.stripPrefix("").flags(),
        StringViewFlag::Global|StringViewFlag::NullTerminated);
}

void StringViewTest::stripPrefixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectOutput{&out};
    "overcomplicated"_s.stripPrefix("complicated");
    CORRADE_COMPARE(out.str(), "Containers::StringView::stripPrefix(): string doesn't begin with complicated\n");
}

void StringViewTest::stripSuffix() {
    CORRADE_COMPARE("overcomplicated"_s.stripSuffix("complicated"), "over");
    CORRADE_COMPARE("overcomplicated"_s.stripSuffix(""), "overcomplicated");

    /* Only a null view results in a null output */
    CORRADE_VERIFY(""_s.stripSuffix("").data());
    CORRADE_VERIFY(!StringView{}.stripSuffix("").data());
}

void StringViewTest::stripSuffixFlags() {
    CORRADE_COMPARE("overcomplicated"_s.stripSuffix("complicated").flags(),
        StringViewFlag::Global);
    CORRADE_COMPARE("overcomplicated"_s.stripSuffix("").flags(),
        StringViewFlag::Global|StringViewFlag::NullTerminated);
}

void StringViewTest::stripSuffixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectOutput{&out};
    "overcomplicated"_s.stripSuffix("over");
    CORRADE_COMPARE(out.str(), "Containers::StringView::stripSuffix(): string doesn't end with over\n");
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
    std::ostringstream out;
    /* The operator<< is implemented directly in Debug, testing here to have
       everything together */
    Debug{&out} << "lolwat, using iostream to\0test string views?!"_s;
    CORRADE_COMPARE(out.str(), (std::string{"lolwat, using iostream to\0test string views?!\n", 46}));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringViewTest)
