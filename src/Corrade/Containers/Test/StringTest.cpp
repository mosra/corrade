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

#include <cstring>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"

namespace {

struct Str {
    Str(const char* data, std::size_t size): size{size} {
        std::memcpy(this->data, data, size);
        this->data[size] = '\0';
    }

    std::size_t size;
    char data[256];
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct StringConverter<Str> {
    static String from(const Str& other) {
        return String{other.data, other.size};
    }

    static Str to(const String& other) {
        return {other.data(), other.size()};
    }
};

}

namespace Test { namespace {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void constructDefault();
    void constructTakeOwnership();
    void constructTakeOwnershipNull();
    void constructTakeOwnershipNotNullTerminated();
    void constructTakeOwnershipTooLarge();
    void constructPointer();
    void constructPointerSmall();
    void constructPointerNull();
    void constructPointerSize();
    void constructPointerSizeZero();
    void constructPointerSizeSmall();
    void constructPointerSizeNullZero();
    void constructPointerSizeNullNonZero();
    void constructPointerSizeTooLarge();

    void constructNullTerminatedGlobalView();

    void convertStringView();
    void convertStringViewSmall();
    void convertMutableStringView();
    void convertMutableStringViewSmall();
    void convertArrayView();
    void convertArrayViewSmall();
    void convertMutableArrayView();
    void convertMutableArrayViewSmall();
    void convertExternal();

    void compare();
    void compareLargeToLarge();
    void compareLargeToSmall();

    void copyConstructLarge();
    void copyLargeToLarge();
    void copyLargeToSmall();
    void copyConstructSmall();
    void copySmallToLarge();
    void copySmallToSmall();

    void moveConstructLarge();
    void moveLargeToLarge();
    void moveLargeToSmall();
    void moveConstructSmall();
    void moveSmallToLarge();
    void moveSmallToSmall();

    void access();
    void accessSmall();

    void slice();
    void slicePointer();

    void release();

    void releaseDeleterSmall();
};

StringTest::StringTest() {
    addTests({&StringTest::constructDefault,
              &StringTest::constructTakeOwnership,
              &StringTest::constructTakeOwnershipNull,
              &StringTest::constructTakeOwnershipNotNullTerminated,
              &StringTest::constructTakeOwnershipTooLarge,
              &StringTest::constructPointer,
              &StringTest::constructPointerSmall,
              &StringTest::constructPointerNull,
              &StringTest::constructPointerSize,
              &StringTest::constructPointerSizeZero,
              &StringTest::constructPointerSizeSmall,
              &StringTest::constructPointerSizeNullZero,
              &StringTest::constructPointerSizeNullNonZero,
              &StringTest::constructPointerSizeTooLarge,

              &StringTest::constructNullTerminatedGlobalView,

              &StringTest::convertStringView,
              &StringTest::convertStringViewSmall,
              &StringTest::convertMutableStringView,
              &StringTest::convertMutableStringViewSmall,
              &StringTest::convertArrayView,
              &StringTest::convertArrayViewSmall,
              &StringTest::convertMutableArrayView,
              &StringTest::convertMutableArrayViewSmall,
              &StringTest::convertExternal,

              &StringTest::compare,
              &StringTest::compareLargeToLarge,
              &StringTest::compareLargeToSmall,

              &StringTest::copyConstructLarge,
              &StringTest::copyLargeToLarge,
              &StringTest::copyLargeToSmall,
              &StringTest::copyConstructSmall,
              &StringTest::copySmallToLarge,
              &StringTest::copySmallToSmall,

              &StringTest::moveConstructLarge,
              &StringTest::moveLargeToLarge,
              &StringTest::moveLargeToSmall,
              &StringTest::moveConstructSmall,
              &StringTest::moveSmallToLarge,
              &StringTest::moveSmallToSmall,

              &StringTest::access,
              &StringTest::accessSmall,

              &StringTest::slice,
              &StringTest::slicePointer,

              &StringTest::release,

              &StringTest::releaseDeleterSmall});
}

using namespace Literals;

void StringTest::constructDefault() {
    String a;
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructTakeOwnership() {
    char data[] = "hello\0world!";

    {
        String a{data, 12, [](char* data, std::size_t size) {
            ++data[0];
            data[size - 1] = '?';
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!a.isEmpty());
        CORRADE_COMPARE(a.size(), sizeof(data) - 1);
        CORRADE_COMPARE(static_cast<const void*>(a.data()), data);
        CORRADE_VERIFY(a.deleter());
    }

    CORRADE_COMPARE((StringView{data, 12}), "iello\0world?"_s);
}

void StringTest::constructTakeOwnershipNull() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const char* data = nullptr;

    std::ostringstream out;
    Error redirectError{&out};
    String a{data, 5, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out.str(), "Containers::String: can only take ownership of a non-null null-terminated array\n");
}

void StringTest::constructTakeOwnershipNotNullTerminated() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const char data[] { 'a', '3' };

    std::ostringstream out;
    Error redirectError{&out};
    String a{data, 1, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out.str(), "Containers::String: can only take ownership of a non-null null-terminated array\n");
}

void StringTest::constructTakeOwnershipTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const char* data = "abc";

    std::ostringstream out;
    Error redirectError{&out};
    String a{data, ~std::size_t{}, [](char*, std::size_t) {}};
    CORRADE_COMPARE(out.str(), sizeof(std::size_t) == 4 ?
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n" :
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
}

void StringTest::constructPointer() {
    String a = "Allocated hello for a verbose world\0that rules";
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a.data()[0], 'A');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
    CORRADE_VERIFY(!a.deleter());
}

void StringTest::constructPointerSmall() {
    String a = "hello\0world!";
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.data()[0], 'h');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'o');
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Verify the data is really stored inside */
    CORRADE_VERIFY(a.data() >= reinterpret_cast<char*>(&a));
    CORRADE_VERIFY(a.data() < reinterpret_cast<char*>(&a + 1));

    /* Bypassing SSO */
    String aa{AllocatedInit, "hello\0world!"};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_VERIFY(!aa.isEmpty());
    CORRADE_COMPARE(aa.size(), 5);
    CORRADE_COMPARE(aa.data()[0], 'h');
    CORRADE_COMPARE(aa.data()[a.size() - 1], 'o');
    CORRADE_COMPARE(aa.data()[a.size()], '\0');
}

void StringTest::constructPointerNull() {
    String a = nullptr;
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');

    /* Bypassing SSO */
    String aa{AllocatedInit, nullptr};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 0);
    CORRADE_COMPARE(aa.data()[0], '\0');
}

void StringTest::constructPointerSize() {
    /* `that rules` doesn't get copied */
    String a{"Allocated hello\0for a verbose world\0that rules", 35};
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a.data()[0], 'A');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructPointerSizeZero() {
    String a{"Allocated hello for a verbose world", 0};
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructPointerSizeSmall() {
    String a{"this\0world\0is hell", 10}; /* `is hell` doesn't get copied */
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a.data()[0], 't');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Bypassing SSO */
    String aa{AllocatedInit, "this\0world\0is hell", 10};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 10);
    CORRADE_COMPARE(aa.data()[0], 't');
    CORRADE_COMPARE(aa.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(aa.data()[a.size()], '\0');
}

void StringTest::constructPointerSizeNullZero() {
    String a{nullptr, 0};
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');

    /* Bypassing SSO */
    String aa{AllocatedInit, nullptr, 0};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 0);
    CORRADE_COMPARE(aa.data()[0], '\0');
}

void StringTest::constructPointerSizeNullNonZero() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    String a{nullptr, 3};
    String aa{AllocatedInit, nullptr, 3};
    CORRADE_COMPARE(out.str(),
        "Containers::String: received a null string of size 3\n"
        "Containers::String: received a null string of size 3\n");
}

void StringTest::constructPointerSizeTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    String a{"abc", ~std::size_t{}};
    String aa{AllocatedInit, "abc", ~std::size_t{}};
    CORRADE_COMPARE(out.str(), sizeof(std::size_t) == 4 ?
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n"
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n" :
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n"
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
}

void StringTest::constructNullTerminatedGlobalView() {
    using namespace Literals;

    StringView local{"Allocated hello for a verbose world", 35};
    CORRADE_COMPARE(local.flags(), StringViewFlags{});

    StringView localNullTerminated = "Allocated hello for a verbose world";
    CORRADE_COMPARE(localNullTerminated.flags(), StringViewFlag::NullTerminated);

    StringView globalNullTerminated = "Allocated hello for a verbose world"_s;
    CORRADE_COMPARE(globalNullTerminated.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);

    StringView global{"Allocated hello for a verbose world", 35, StringViewFlag::Global};
    CORRADE_COMPARE(global.flags(), StringViewFlag::Global);

    /* For a local non-null-terminated string, all three convert it to a owning
       copy */
    {
        String a = String::nullTerminatedView(local);
        String b = String::nullTerminatedGlobalView(local);
        String c = String::globalView(local);
        CORRADE_COMPARE(a, local);
        CORRADE_COMPARE(b, local);
        CORRADE_COMPARE(c, local);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(c.data()) != local.data());
        CORRADE_VERIFY(!a.deleter());
        CORRADE_VERIFY(!b.deleter());
        CORRADE_VERIFY(!c.deleter());

    /* For a local null-terminated only the last two do */
    } {
        String a = String::nullTerminatedView(localNullTerminated);
        String b = String::nullTerminatedGlobalView(localNullTerminated);
        String c = String::globalView(localNullTerminated);
        CORRADE_COMPARE(a, local);
        CORRADE_COMPARE(b, local);
        CORRADE_COMPARE(c, local);
        {
            #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && defined(_DEBUG)
            CORRADE_EXPECT_FAIL("MSVC does some CRAZY SHIT with string literal addresses. But only in Debug builds.");
            #endif
            CORRADE_COMPARE(static_cast<void*>(a.data()), local.data());
        }
        CORRADE_VERIFY(static_cast<void*>(b.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(c.data()) != local.data());
        CORRADE_VERIFY(a.deleter());
        CORRADE_VERIFY(!b.deleter());
        CORRADE_VERIFY(!c.deleter());

    /* For a global null-terminated string, all three keep a view */
    } {
        String a = String::nullTerminatedView(globalNullTerminated);
        String b = String::nullTerminatedGlobalView(globalNullTerminated);
        String c = String::globalView(globalNullTerminated);
        CORRADE_COMPARE(a, local);
        CORRADE_COMPARE(b, local);
        CORRADE_COMPARE(c, local);
        {
            #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && defined(_DEBUG)
            CORRADE_EXPECT_FAIL("MSVC does some CRAZY SHIT with string literal addresses. But only in Debug builds.");
            #endif
            CORRADE_COMPARE(static_cast<void*>(a.data()), local.data());
            CORRADE_COMPARE(static_cast<void*>(b.data()), local.data());
            CORRADE_COMPARE(static_cast<void*>(c.data()), local.data());
        }
        CORRADE_VERIFY(a.deleter());
        CORRADE_VERIFY(b.deleter());
        CORRADE_VERIFY(c.deleter());

    /* For a global non-null-terminated string, only the last keeps a view */
    } {
        String a = String::nullTerminatedView(global);
        String b = String::nullTerminatedGlobalView(global);
        String c = String::globalView(global);
        CORRADE_COMPARE(a, local);
        CORRADE_COMPARE(b, local);
        CORRADE_COMPARE(c, local);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != local.data());
        {
            #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && defined(_DEBUG)
            CORRADE_EXPECT_FAIL("MSVC does some CRAZY SHIT with string literal addresses. But only in Debug builds.");
            #endif
            CORRADE_COMPARE(static_cast<void*>(c.data()), local.data());
        }
        CORRADE_VERIFY(!a.deleter());
        CORRADE_VERIFY(!b.deleter());
        CORRADE_VERIFY(c.deleter());
    }
}

void StringTest::convertStringView() {
    const String a = "Allocated hello\0for a verbose world"_s;
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    StringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Explicit conversion shouldn't be ambiguous */
    StringView aView2(a);
    CORRADE_COMPARE(aView2.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView2.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView2.data()), a.data());
}

void StringTest::convertStringViewSmall() {
    const String a = "this\0world"_s;
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    StringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Bypassing SSO */
    const String aa{AllocatedInit, "this\0world"_s};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 10);
    CORRADE_COMPARE(aa[0], 't');
}

void StringTest::convertMutableStringView() {
    char aData[] = "Allocated hello\0for a verbose world";
    String a = MutableStringView{aData, 35};
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    MutableStringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Explicit conversion shouldn't be ambiguous */
    MutableStringView aView2(a);
    CORRADE_COMPARE(aView2.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView2.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView2.data()), a.data());
}

void StringTest::convertMutableStringViewSmall() {
    char aData[] = "this\0world";
    String a = MutableStringView{aData, 10};
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    MutableStringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Bypassing SSO */
    String aa{AllocatedInit, MutableStringView{aData, 10}};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 10);
    CORRADE_COMPARE(aa[0], 't');
}

void StringTest::convertArrayView() {
    const String a = Containers::arrayView("Allocated hello\0for a verbose world").except(1);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    ArrayView<const char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertArrayViewSmall() {
    const String a = Containers::arrayView("this\0world").except(1);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<const char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Bypassing SSO */
    const String aa{AllocatedInit, Containers::arrayView("this\0world").except(1)};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 10);
    CORRADE_COMPARE(aa[0], 't');
}

void StringTest::convertMutableArrayView() {
    char aData[] = "Allocated hello\0for a verbose world";
    String a = ArrayView<char>{aData}.except(1);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    ArrayView<char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertMutableArrayViewSmall() {
    char aData[] = "this\0world";
    String a = ArrayView<char>{aData}.except(1);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    /* Bypassing SSO */
    String aa{AllocatedInit, ArrayView<char>{aData}.except(1)};
    CORRADE_VERIFY(!aa.isSmall());
    CORRADE_COMPARE(aa.size(), 10);
    CORRADE_COMPARE(aa[0], 't');
}

void StringTest::convertExternal() {
    Str a{"hello", 5};

    String b = a;
    CORRADE_COMPARE(b.data(), "hello"_s);
    CORRADE_COMPARE(b.size(), 5);

    Str c = b;
    CORRADE_COMPARE(c.data, "hello"_s);
    CORRADE_COMPARE(c.size, 5);
}

void StringTest::compare() {
    /* Trivial case */
    String a = "hello";
    CORRADE_VERIFY(a == a);

    String b{"hello3", 5};
    CORRADE_VERIFY(b == b);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);

    /* Verify we don't just compare a common prefix */
    String c = "hello!";
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);

    /* Comparison with an empty string */
    String empty;
    CORRADE_VERIFY(empty == empty);
    CORRADE_VERIFY(a != empty);
    CORRADE_VERIFY(empty != a);

    /* Null terminator in the middle -- it should not stop at it */
    {
        using namespace Literals;
        CORRADE_VERIFY(String{"hello\0world"_s} == (String{"hello\0world!", 11}));
        CORRADE_VERIFY(String{"hello\0wOrld"_s} != (String{"hello\0world!", 11}));
    }

    /* C strings on either side */
    CORRADE_VERIFY(a == "hello");
    CORRADE_VERIFY("hello" == a);
    CORRADE_VERIFY(c != "hello");
    CORRADE_VERIFY("hello" != c);

    /* Views on either side */
    CORRADE_VERIFY(a == "hello"_s);
    CORRADE_VERIFY("hello"_s == a);
    CORRADE_VERIFY(c != "hello"_s);
    CORRADE_VERIFY("hello"_s != c);

    /* Mutable views on either side */
    char dData[] = "hello";
    MutableStringView d = dData;
    CORRADE_VERIFY(a == d);
    CORRADE_VERIFY(d == a);
    CORRADE_VERIFY(c != d);
    CORRADE_VERIFY(d != c);
}

void StringTest::compareLargeToLarge() {
    String a = "Allocated hello for a verbose world";
    CORRADE_VERIFY(!a.isSmall());

    String b = "Allocated hello for a verbose world";
    CORRADE_VERIFY(!b.isSmall());

    String c = "Allocated hello for a verbose world!";
    CORRADE_VERIFY(!c.isSmall());

    CORRADE_VERIFY(a == a);
    CORRADE_VERIFY(b == b);
    CORRADE_VERIFY(c == c);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);
}

void StringTest::compareLargeToSmall() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    /* Create explicitly from heap-allocated data to avoid it being stored as
       SSO */
    char bData[] = "hello";
    String b{bData, 5, [](char*, std::size_t){}};
    CORRADE_VERIFY(!b.isSmall());

    char cData[] = "hello!";
    String c{cData, 6, [](char*, std::size_t){}};
    CORRADE_VERIFY(!c.isSmall());

    CORRADE_VERIFY(a == a);
    CORRADE_VERIFY(b == b);
    CORRADE_VERIFY(c == c);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);
}

void StringTest::copyConstructLarge() {
    char aData[] = "Allocated hello for a verbose world";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        /* A copy is made using a default deleter */
        String b = a;
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() != a.data());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!b.deleter());
    }

    /* a is deallocated as usual */
    CORRADE_COMPARE(aData[0], 'B');
}

void StringTest::copyLargeToLarge() {
    char aData[] = "Allocated hello for a verbose world";
    char bData[] = "ALLOCATED HELLO FOR A VERBOSE WORLD!!!";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        String b{bData, sizeof(bData) - 1, [](char* data, std::size_t){
            ++data[1];
        }};
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());

        /* A copy is made using a default deleter, b is deallocated */
        b = a;
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() != a.data());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!b.deleter());
        CORRADE_COMPARE(bData[1], 'M');
    }

    /* a is deallocated as usual */
    CORRADE_COMPARE(aData[0], 'B');
}

void StringTest::copyLargeToSmall() {
    char aData[] = "Allocated hello for a verbose world";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        String b = "hello";
        CORRADE_VERIFY(b.isSmall());

        /* A copy is made using a default deleter, b is overwritten */
        b = a;
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() != a.data());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!b.deleter());
    }

    /* a is deallocated as usual */
    CORRADE_COMPARE(aData[0], 'B');
}

void StringTest::copyConstructSmall() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    /* A copy is made using a SSO */
    String b = a;
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
}

void StringTest::copySmallToLarge() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    char bData[] = "ALLOCATED HELLO FOR A VERBOSE WORLD!!!";
    String b{bData, sizeof(bData) - 1, [](char* data, std::size_t){
        ++data[1];
    }};
    CORRADE_VERIFY(!b.isSmall());
    CORRADE_VERIFY(b.deleter());

    /* A copy is made using a SSO, b is deallocated */
    b = a;
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
    CORRADE_COMPARE(bData[1], 'M');
}

void StringTest::copySmallToSmall() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    String b = "HELLO!!!";
    CORRADE_VERIFY(b.isSmall());

    /* A copy is made using a SSO, original data overwritten */
    b = a;
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
}

void StringTest::moveConstructLarge() {
    char aData[] = "Allocated hello for a verbose world";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        /* Everything including the deleter is moved */
        String b = std::move(a);
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() == aData);
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());
    }

    /* b is deallocated just once */
    CORRADE_COMPARE(aData[0], 'B');
}

void StringTest::moveLargeToLarge() {
    char aData[] = "Allocated hello for a verbose world";
    char bData[] = "ALLOCATED HELLO FOR A VERBOSE WORLD!!!";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        String b{bData, sizeof(bData) - 1, [](char* data, std::size_t){
            ++data[1];
        }};
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());

        /* The two are simply swapped */
        b = std::move(a);
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() == aData);
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());

        /* No deleters fired yet */
        CORRADE_COMPARE(aData[0], 'A');
        CORRADE_COMPARE(bData[1], 'L');
    }

    /* both is deallocated as usual */
    CORRADE_COMPARE(aData[0], 'B');
    CORRADE_COMPARE(bData[1], 'M');
}

void StringTest::moveLargeToSmall() {
    char aData[] = "Allocated hello for a verbose world";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        String b = "hello";
        CORRADE_VERIFY(b.isSmall());

        /* The two are simply swapped */
        b = std::move(a);
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() == aData);
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());

        /* No deleter fired yet */
        CORRADE_COMPARE(aData[0], 'A');
    }

    /* a is deallocated as usual */
    CORRADE_COMPARE(aData[0], 'B');
}

void StringTest::moveConstructSmall() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    /* The two are simply swapped */
    String b = std::move(a);
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
}

void StringTest::moveSmallToLarge() {
    char bData[] = "ALLOCATED HELLO FOR A VERBOSE WORLD!!!";

    {
        String a = "hello";
        CORRADE_VERIFY(a.isSmall());

        String b{bData, sizeof(bData) - 1, [](char* data, std::size_t){
            ++data[1];
        }};
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(b.deleter());

        /* The two are simply swapped */
        b = std::move(a);
        CORRADE_COMPARE(b, "hello"_s);
        CORRADE_VERIFY(b.data() != a.data());
        CORRADE_VERIFY(b.isSmall());

        /* No deleters fired yet */
        CORRADE_COMPARE(bData[1], 'L');
    }

    /* b deallocated as usual */
    CORRADE_COMPARE(bData[1], 'M');
}

void StringTest::moveSmallToSmall() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    String b = "HELLO!!!";
    CORRADE_VERIFY(b.isSmall());

    /* The two are simply swapped */
    b = a;
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
}

void StringTest::access() {
    String a = "Allocated hello for a verbose world";
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_COMPARE(*a.begin(), 'A');
    CORRADE_COMPARE(*a.cbegin(), 'A');
    CORRADE_COMPARE(*(a.end() - 1), 'd');
    CORRADE_COMPARE(*(a.cend() - 1), 'd');

    a[14] = '!';
    *a.begin() = 'O';
    *(a.end() - 1) = 't';
    CORRADE_COMPARE(a, "Ollocated hell! for a verbose worlt");

    const String ca = "Allocated hello for a verbose world";
    CORRADE_VERIFY(!ca.isSmall());
    CORRADE_COMPARE(*ca.begin(), 'A');
    CORRADE_COMPARE(*ca.cbegin(), 'A');
    CORRADE_COMPARE(*(ca.end() - 1), 'd');
    CORRADE_COMPARE(*(ca.cend() - 1), 'd');
    CORRADE_COMPARE(ca[14], 'o');
}

void StringTest::accessSmall() {
    String a = "hello!";
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(*a.begin(), 'h');
    CORRADE_COMPARE(*a.cbegin(), 'h');
    CORRADE_COMPARE(*(a.end() - 1), '!');
    CORRADE_COMPARE(*(a.cend() - 1), '!');

    a[4] = '!';
    *(a.end() - 1) = '?';
    *a.begin() = 'H';
    CORRADE_COMPARE(a, "Hell!?");
}

void StringTest::slice() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior */

    String a = "hello";
    CORRADE_COMPARE(a.slice(1, 4), "ell"_s);
    CORRADE_COMPARE(a.prefix(3), "hel"_s);
    CORRADE_COMPARE(a.prefix(2).flags(), StringViewFlags{});
    CORRADE_COMPARE(a.except(2), "hel"_s);
    CORRADE_COMPARE(a.suffix(2), "llo"_s);
    CORRADE_COMPARE(a.suffix(2).flags(), StringViewFlag::NullTerminated);

    const String ca = "hello";
    CORRADE_COMPARE(ca.slice(1, 4), "ell"_s);
    CORRADE_COMPARE(ca.prefix(3), "hel"_s);
    CORRADE_COMPARE(ca.prefix(2).flags(), StringViewFlags{});
    CORRADE_COMPARE(ca.except(2), "hel"_s);
    CORRADE_COMPARE(ca.suffix(2), "llo"_s);
    CORRADE_COMPARE(ca.suffix(2).flags(), StringViewFlag::NullTerminated);
}

void StringTest::slicePointer() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior and neither the resulting flags */

    String a = "hello";
    CORRADE_COMPARE(a.slice(a.data() + 1, a.data() + 4), "ell"_s);
    CORRADE_COMPARE(a.prefix(a.data() + 3), "hel"_s);
    CORRADE_COMPARE(a.prefix(a.data() + 2).flags(), StringViewFlags{});
    CORRADE_COMPARE(a.suffix(a.data() + 2), "llo"_s);
    CORRADE_COMPARE(a.suffix(a.data() + 2).flags(), StringViewFlag::NullTerminated);

    const String ca = "hello";
    CORRADE_COMPARE(ca.slice(ca.data() + 1, ca.data() + 4), "ell"_s);
    CORRADE_COMPARE(ca.prefix(ca.data() + 3), "hel"_s);
    CORRADE_COMPARE(ca.prefix(ca.data() + 2).flags(), StringViewFlags{});
    CORRADE_COMPARE(ca.suffix(ca.data() + 2), "llo"_s);
    CORRADE_COMPARE(ca.suffix(ca.data() + 2).flags(), StringViewFlag::NullTerminated);
}

void StringTest::release() {
    String a = "Allocated hello for a verbose world";

    const void* data = a.data();
    const char* released = a.release();
    delete[] released;
    CORRADE_COMPARE(released, data);

    /* Post-release state should be the same as of a default-constructed
       instance -- with zero size, but a non-null null-terminated data */
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::releaseDeleterSmall() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    String a;
    CORRADE_VERIFY(a.isSmall());

    std::ostringstream out;
    Error redirectError{&out};
    a.deleter();
    a.release();
    CORRADE_COMPARE(out.str(),
        "Containers::String::deleter(): cannot call on a SSO instance\n"
        "Containers::String::release(): cannot call on a SSO instance\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringTest)
