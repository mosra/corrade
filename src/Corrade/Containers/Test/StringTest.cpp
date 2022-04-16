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

#include <cstring>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
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

    void allocatedInitTagNoDefaultConstructor();
    void allocatedInitTagInlineDefinition();

    void constructDefault();
    void constructTakeOwnership();
    void constructTakeOwnershipNull();
    void constructTakeOwnershipNotNullTerminated();
    void constructTakeOwnershipTooLarge();
    void constructPointer();
    void constructPointerSmall();
    void constructPointerSmallAllocatedInit();
    void constructPointerNull();
    void constructPointerNullAllocatedInit();
    void constructPointerSize();
    void constructPointerSizeZero();
    void constructPointerSizeSmall();
    void constructPointerSizeSmallAllocatedInit();
    void constructPointerSizeNullZero();
    void constructPointerSizeNullZeroAllocatedInit();
    void constructPointerSizeNullNonZero();
    void constructPointerSizeTooLarge();

    void constructValueInit();
    void constructValueInitSmall();
    void constructValueInitTooLarge();
    void constructDirectInit();
    void constructDirectInitSmall();
    void constructDirectInitTooLarge();
    void constructNoInit();
    void constructNoInitSmall();
    void constructNoInitTooLarge();

    void constructNullTerminatedGlobalView();
    void constructNullTerminatedGlobalViewAllocatedInit();

    void convertStringView();
    void convertStringViewSmall();
    void convertStringViewSmallAllocatedInit();
    void convertMutableStringView();
    void convertMutableStringViewSmall();
    void convertMutableStringViewSmallAllocatedInit();
    void convertArrayView();
    void convertArrayViewSmall();
    void convertArrayViewSmallAllocatedInit();
    void convertMutableArrayView();
    void convertMutableArrayViewSmall();
    void convertMutableArrayViewSmallAllocatedInit();
    void convertArray();
    void convertArraySmall();
    void convertArraySmallAllocatedInit();
    void convertArrayCustomDeleter();
    void convertExternal();

    void compare();
    void compareLargeToLarge();
    void compareLargeToSmall();

    void copyConstructLarge();
    void copyConstructLargeAllocatedInit();
    void copyLargeToLarge();
    void copyLargeToSmall();
    void copyConstructSmall();
    void copyConstructSmallAllocatedInit();
    void copySmallToLarge();
    void copySmallToSmall();

    void moveConstructLarge();
    void moveConstructLargeAllocatedInit();
    void moveLargeToLarge();
    void moveLargeToSmall();
    void moveConstructSmall();
    void moveConstructSmallAllocatedInit();
    void moveSmallToLarge();
    void moveSmallToSmall();

    void access();
    void accessSmall();
    void accessInvalid();

    void slice();
    void slicePointer();

    void split();
    void splitOnAny();
    void splitOnWhitespace();

    void partition();

    void add();
    void addNullViews();

    void join();
    void joinNullViews();

    void hasPrefix();
    void hasSuffix();

    template<class T> void exceptPrefix();
    void exceptPrefixInvalid();
    void exceptPrefixDisabledOverloads();
    template<class T> void exceptSuffix();
    void exceptSuffixInvalid();
    void exceptSuffixDisabledOverloads();

    template<class T> void trimmed();

    /* Tests also contains() */
    template<class T> void find();
    template<class T> void findOr();
    template<class T> void findLast();
    template<class T> void findLastOr();

    void release();
    void releaseDeleterSmall();

    void defaultDeleter();
    void customDeleter();
    /* Unlike with Array, it's not possible to create a nullptr String with a
       custom deleter in order to guarantee that it always points to a
       null-terminated string, thus there's no customDeleterNullData() */
    void customDeleterZeroSize();
    void customDeleterMovedOutInstance();
};

StringTest::StringTest() {
    addTests({&StringTest::allocatedInitTagNoDefaultConstructor,
              &StringTest::allocatedInitTagInlineDefinition,

              &StringTest::constructDefault,
              &StringTest::constructTakeOwnership,
              &StringTest::constructTakeOwnershipNull,
              &StringTest::constructTakeOwnershipNotNullTerminated,
              &StringTest::constructTakeOwnershipTooLarge,
              &StringTest::constructPointer,
              &StringTest::constructPointerSmall,
              &StringTest::constructPointerSmallAllocatedInit,
              &StringTest::constructPointerNull,
              &StringTest::constructPointerNullAllocatedInit,
              &StringTest::constructPointerSize,
              &StringTest::constructPointerSizeZero,
              &StringTest::constructPointerSizeSmall,
              &StringTest::constructPointerSizeSmallAllocatedInit,
              &StringTest::constructPointerSizeNullZero,
              &StringTest::constructPointerSizeNullZeroAllocatedInit,
              &StringTest::constructPointerSizeNullNonZero,
              &StringTest::constructPointerSizeTooLarge,

              &StringTest::constructValueInit,
              &StringTest::constructValueInitSmall,
              &StringTest::constructValueInitTooLarge,
              &StringTest::constructDirectInit,
              &StringTest::constructDirectInitSmall,
              &StringTest::constructDirectInitTooLarge,
              &StringTest::constructNoInit,
              &StringTest::constructNoInitSmall,
              &StringTest::constructNoInitTooLarge,

              &StringTest::constructNullTerminatedGlobalView,
              &StringTest::constructNullTerminatedGlobalViewAllocatedInit,

              &StringTest::convertStringView,
              &StringTest::convertStringViewSmall,
              &StringTest::convertStringViewSmallAllocatedInit,
              &StringTest::convertMutableStringView,
              &StringTest::convertMutableStringViewSmall,
              &StringTest::convertMutableStringViewSmallAllocatedInit,
              &StringTest::convertArrayView,
              &StringTest::convertArrayViewSmall,
              &StringTest::convertArrayViewSmallAllocatedInit,
              &StringTest::convertMutableArrayView,
              &StringTest::convertMutableArrayViewSmall,
              &StringTest::convertMutableArrayViewSmallAllocatedInit,
              &StringTest::convertArray,
              &StringTest::convertArraySmall,
              &StringTest::convertArraySmallAllocatedInit,
              &StringTest::convertArrayCustomDeleter,
              &StringTest::convertExternal,

              &StringTest::compare,
              &StringTest::compareLargeToLarge,
              &StringTest::compareLargeToSmall,

              &StringTest::copyConstructLarge,
              &StringTest::copyConstructLargeAllocatedInit,
              &StringTest::copyLargeToLarge,
              &StringTest::copyLargeToSmall,
              &StringTest::copyConstructSmall,
              &StringTest::copyConstructSmallAllocatedInit,
              &StringTest::copySmallToLarge,
              &StringTest::copySmallToSmall,

              &StringTest::moveConstructLarge,
              &StringTest::moveConstructLargeAllocatedInit,
              &StringTest::moveLargeToLarge,
              &StringTest::moveLargeToSmall,
              &StringTest::moveConstructSmall,
              &StringTest::moveConstructSmallAllocatedInit,
              &StringTest::moveSmallToLarge,
              &StringTest::moveSmallToSmall,

              &StringTest::access,
              &StringTest::accessSmall,
              &StringTest::accessInvalid,

              &StringTest::slice,
              &StringTest::slicePointer,

              &StringTest::split,
              &StringTest::splitOnAny,
              &StringTest::splitOnWhitespace,

              &StringTest::partition,

              &StringTest::add,
              &StringTest::addNullViews,

              &StringTest::join,
              &StringTest::joinNullViews,

              &StringTest::hasPrefix,
              &StringTest::hasSuffix,

              &StringTest::exceptPrefix<String>,
              &StringTest::exceptPrefix<const String>,
              &StringTest::exceptPrefixInvalid,
              &StringTest::exceptPrefixDisabledOverloads,
              &StringTest::exceptSuffix<String>,
              &StringTest::exceptSuffix<const String>,
              &StringTest::exceptSuffixInvalid,
              &StringTest::exceptSuffixDisabledOverloads,

              &StringTest::trimmed<String>,
              &StringTest::trimmed<const String>,

              &StringTest::find<String>,
              &StringTest::find<const String>,
              &StringTest::findOr<String>,
              &StringTest::findOr<const String>,
              &StringTest::findLast<String>,
              &StringTest::findLast<const String>,
              &StringTest::findLastOr<String>,
              &StringTest::findLastOr<const String>,

              &StringTest::release,
              &StringTest::releaseDeleterSmall,

              &StringTest::defaultDeleter,
              &StringTest::customDeleter,
              &StringTest::customDeleterZeroSize,
              &StringTest::customDeleterMovedOutInstance});
}

template<class T> struct ConstTraits;
template<> struct ConstTraits<String> {
    typedef MutableStringView ViewType;
    static const char* name() { return "String"; }
};
template<> struct ConstTraits<const String> {
    typedef StringView ViewType;
    static const char* name() { return "const String"; }
};

using namespace Literals;

/** @todo move these to TagsTest once the tags gets used outside of String */
void StringTest::allocatedInitTagNoDefaultConstructor() {
    CORRADE_VERIFY(!std::is_default_constructible<AllocatedInitT>::value);
}

void StringTest::allocatedInitTagInlineDefinition() {
    CORRADE_VERIFY(std::is_same<decltype(AllocatedInit), const AllocatedInitT>::value);
}

void StringTest::constructDefault() {
    String a;
    CORRADE_VERIFY(!a);
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
        CORRADE_VERIFY(a);
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
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n");
    #endif
}

void StringTest::constructPointer() {
    String a = "Allocated hello for a verbose world\0that rules";
    CORRADE_VERIFY(a);
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
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.data()[0], 'h');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'o');
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Verify the data is really stored inside */
    CORRADE_VERIFY(a.data() >= reinterpret_cast<char*>(&a));
    CORRADE_VERIFY(a.data() < reinterpret_cast<char*>(&a + 1));
}

void StringTest::constructPointerSmallAllocatedInit() {
    String a{AllocatedInit, "hello\0world!"};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.data()[0], 'h');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'o');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructPointerNull() {
    String a = nullptr;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructPointerNullAllocatedInit() {
    String a{AllocatedInit, nullptr};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructPointerSize() {
    /* `that rules` doesn't get copied */
    String a{"Allocated hello\0for a verbose world\0that rules", 35};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a.data()[0], 'A');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructPointerSizeZero() {
    String a{"Allocated hello for a verbose world", 0};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructPointerSizeSmall() {
    String a{"this\0world\0is hell", 10}; /* `is hell` doesn't get copied */
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a.data()[0], 't');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructPointerSizeSmallAllocatedInit() {
    String a{AllocatedInit, "this\0world\0is hell", 10};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a.data()[0], 't');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'd');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructPointerSizeNullZero() {
    String a{nullptr, 0};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::constructPointerSizeNullZeroAllocatedInit() {
    String a{AllocatedInit, nullptr, 0};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.data()[0], '\0');
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
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n"
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n"
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n");
    #endif
}

void StringTest::constructValueInit() {
    String a{Corrade::ValueInit, 35};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a.data()[0], '\0');
    CORRADE_COMPARE(a.data()[a.size() - 1], '\0');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructValueInitSmall() {
    String a{Corrade::ValueInit, 10};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a.data()[0], '\0');
    CORRADE_COMPARE(a.data()[a.size() - 1], '\0');
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Verify the data is really stored inside */
    CORRADE_VERIFY(a.data() >= reinterpret_cast<char*>(&a));
    CORRADE_VERIFY(a.data() < reinterpret_cast<char*>(&a + 1));
}

void StringTest::constructValueInitTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    String a{Corrade::ValueInit, ~std::size_t{}};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n");
    #endif
}

void StringTest::constructDirectInit() {
    String a{Corrade::DirectInit, 35, 'X'};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a.data()[0], 'X');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'X');
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructDirectInitSmall() {
    String a{Corrade::DirectInit, 10, 'X'};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a.data()[0], 'X');
    CORRADE_COMPARE(a.data()[a.size() - 1], 'X');
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Verify the data is really stored inside */
    CORRADE_VERIFY(a.data() >= reinterpret_cast<char*>(&a));
    CORRADE_VERIFY(a.data() < reinterpret_cast<char*>(&a + 1));
}

void StringTest::constructDirectInitTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    String a{Corrade::DirectInit, ~std::size_t{}, 'X'};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n");
    #endif
}

void StringTest::constructNoInit() {
    String a{Corrade::NoInit, 35};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    /* Contents can be just anything */
    CORRADE_COMPARE(a.data()[a.size()], '\0');
}

void StringTest::constructNoInitSmall() {
    String a{Corrade::NoInit, 10};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    /* Contents can be just anything */
    CORRADE_COMPARE(a.data()[a.size()], '\0');

    /* Verify the data is really stored inside */
    CORRADE_VERIFY(a.data() >= reinterpret_cast<char*>(&a));
    CORRADE_VERIFY(a.data() < reinterpret_cast<char*>(&a + 1));
}

void StringTest::constructNoInitTooLarge() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    String a{Corrade::NoInit, ~std::size_t{}};
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^62 bytes, got 18446744073709551615\n");
    #else
    CORRADE_COMPARE(out.str(),
        "Containers::String: string expected to be smaller than 2^30 bytes, got 4294967295\n");
    #endif
}

void StringTest::constructNullTerminatedGlobalView() {
    /* For a local non-null-terminated string, both convert it to an owning
       copy */
    StringView local{"Hello!", 6};
    CORRADE_COMPARE(local.flags(), StringViewFlags{});
    {
        String a = String::nullTerminatedView(local);
        String b = String::nullTerminatedGlobalView(local);
        CORRADE_COMPARE(b, local);
        CORRADE_COMPARE(a, local);
        CORRADE_VERIFY(a.isSmall());
        CORRADE_VERIFY(b.isSmall());
        CORRADE_VERIFY(static_cast<void*>(a.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != local.data());
    }

    /* For a local null-terminated only second does */
    StringView localNullTerminated = "Hello!";
    CORRADE_COMPARE(localNullTerminated.flags(), StringViewFlag::NullTerminated);
    {
        String a = String::nullTerminatedView(localNullTerminated);
        String b = String::nullTerminatedGlobalView(localNullTerminated);
        CORRADE_COMPARE(a, localNullTerminated);
        CORRADE_COMPARE(b, localNullTerminated);
        CORRADE_COMPARE(static_cast<void*>(a.data()), localNullTerminated.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != localNullTerminated.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(b.isSmall());
        CORRADE_VERIFY(a.deleter());
        /* b is small, has no deleter */
    }

    /* For a global null-terminated string, both keep a view */
    StringView globalNullTerminated = "Hello!"_s;
    CORRADE_COMPARE(globalNullTerminated.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    {
        String a = String::nullTerminatedView(globalNullTerminated);
        String b = String::nullTerminatedGlobalView(globalNullTerminated);
        CORRADE_COMPARE(a, globalNullTerminated);
        CORRADE_COMPARE(b, globalNullTerminated);
        CORRADE_COMPARE(static_cast<void*>(a.data()), globalNullTerminated.data());
        CORRADE_COMPARE(static_cast<void*>(b.data()), globalNullTerminated.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(a.deleter());
        CORRADE_VERIFY(b.deleter());
    }

    /* For a global non-null-terminated string, neither keeps a view */
    StringView global{"Hello!", 6, StringViewFlag::Global};
    CORRADE_COMPARE(global.flags(), StringViewFlag::Global);
    {
        String a = String::nullTerminatedView(global);
        String b = String::nullTerminatedGlobalView(global);
        CORRADE_COMPARE(a, global);
        CORRADE_COMPARE(b, global);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != global.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != global.data());
        CORRADE_VERIFY(a.isSmall());
        CORRADE_VERIFY(b.isSmall());
        /* both a and b is small, has no deleter */
    }

    /* A null view is a special case. It has the flags, but a non-owning String
       can't guarantee the null-termination so an owning empty instance has to
       be made instead. */
    StringView null;
    CORRADE_VERIFY(!null.data());
    CORRADE_COMPARE(null.flags(), StringViewFlag::Global);
    {
        String a = String::nullTerminatedView(null);
        String b = String::nullTerminatedGlobalView(null);
        CORRADE_COMPARE(a, null);
        CORRADE_COMPARE(b, null);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != null.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != null.data());
        CORRADE_VERIFY(a.isSmall());
        CORRADE_VERIFY(b.isSmall());
        /* both a and b is small, has no deleter */
    }
}

void StringTest::constructNullTerminatedGlobalViewAllocatedInit() {
    /* For a local non-null-terminated string, both convert it to an owning
       copy */
    StringView local{"Hello!", 6};
    CORRADE_COMPARE(local.flags(), StringViewFlags{});
    {
        String a = String::nullTerminatedView(AllocatedInit, local);
        String b = String::nullTerminatedGlobalView(AllocatedInit, local);
        CORRADE_COMPARE(a, local);
        CORRADE_COMPARE(b, local);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != local.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != local.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!a.deleter());
        CORRADE_VERIFY(!b.deleter());
    }

    /* For a local null-terminated only second does */
    StringView localNullTerminated = "Hello!";
    CORRADE_COMPARE(localNullTerminated.flags(), StringViewFlag::NullTerminated);
    {
        String a = String::nullTerminatedView(AllocatedInit, localNullTerminated);
        String b = String::nullTerminatedGlobalView(AllocatedInit, localNullTerminated);
        CORRADE_COMPARE(a, localNullTerminated);
        CORRADE_COMPARE(b, localNullTerminated);
        CORRADE_COMPARE(static_cast<void*>(a.data()), localNullTerminated.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != localNullTerminated.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(a.deleter());
        CORRADE_VERIFY(!b.deleter());
    }

    /* For a global null-terminated string, both keep a view */
    StringView globalNullTerminated = "Hello!"_s;
    CORRADE_COMPARE(globalNullTerminated.flags(), StringViewFlag::Global|StringViewFlag::NullTerminated);
    {
        String a = String::nullTerminatedView(AllocatedInit, globalNullTerminated);
        String b = String::nullTerminatedGlobalView(AllocatedInit, globalNullTerminated);
        CORRADE_COMPARE(a, globalNullTerminated);
        CORRADE_COMPARE(b, globalNullTerminated);
        CORRADE_COMPARE(static_cast<void*>(a.data()), globalNullTerminated.data());
        CORRADE_COMPARE(static_cast<void*>(b.data()), globalNullTerminated.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(a.deleter());
        CORRADE_VERIFY(b.deleter());
    }

    /* For a global non-null-terminated string, neither keeps a view */
    StringView global{"Hello!", 6, StringViewFlag::Global};
    CORRADE_COMPARE(global.flags(), StringViewFlag::Global);
    {
        String a = String::nullTerminatedView(AllocatedInit, global);
        String b = String::nullTerminatedGlobalView(AllocatedInit, global);
        CORRADE_COMPARE(a, global);
        CORRADE_COMPARE(b, global);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != global.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != global.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!a.deleter());
        CORRADE_VERIFY(!b.deleter());
    }

    /* A null view is a special case. It has the flags, but a non-owning String
       can't guarantee the null-termination so an owning empty instance has to
       be made instead. */
    StringView null;
    CORRADE_VERIFY(!null.data());
    CORRADE_COMPARE(null.flags(), StringViewFlag::Global);
    {
        String a = String::nullTerminatedView(AllocatedInit, null);
        String b = String::nullTerminatedGlobalView(AllocatedInit, null);
        CORRADE_COMPARE(a, null);
        CORRADE_COMPARE(b, null);
        CORRADE_VERIFY(static_cast<void*>(a.data()) != null.data());
        CORRADE_VERIFY(static_cast<void*>(b.data()) != null.data());
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_VERIFY(!a.deleter());
        CORRADE_VERIFY(!b.deleter());
    }
}

void StringTest::convertStringView() {
    const String a = "Allocated hello\0for a verbose world"_s;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
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
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    StringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertStringViewSmallAllocatedInit() {
    const String a{AllocatedInit, "this\0world"_s};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    StringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertMutableStringView() {
    char aData[] = "Allocated hello\0for a verbose world";
    String a = MutableStringView{aData, 35};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
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
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    MutableStringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertMutableStringViewSmallAllocatedInit() {
    char aData[] = "this\0world";
    String a{AllocatedInit, MutableStringView{aData, 10}};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    MutableStringView aView = a;
    CORRADE_COMPARE(aView.flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());
}

void StringTest::convertArrayView() {
    const String a = arrayView("Allocated hello\0for a verbose world").exceptSuffix(1);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    ArrayView<const char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<const void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertArrayViewSmall() {
    const String a = arrayView("this\0world").exceptSuffix(1);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<const char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<const void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertArrayViewSmallAllocatedInit() {
    const String a{AllocatedInit, arrayView("this\0world").exceptSuffix(1)};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<const char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<const void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertMutableArrayView() {
    char aData[] = "Allocated hello\0for a verbose world";
    String a = ArrayView<char>{aData}.exceptSuffix(1);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 35);
    CORRADE_COMPARE(a[0], 'A');

    ArrayView<char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertMutableArrayViewSmall() {
    char aData[] = "this\0world";
    String a = ArrayView<char>{aData}.exceptSuffix(1);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertMutableArrayViewSmallAllocatedInit() {
    char aData[] = "this\0world";
    String a{AllocatedInit, ArrayView<char>{aData}.exceptSuffix(1)};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 10);
    CORRADE_COMPARE(a[0], 't');

    ArrayView<char> aView = a;
    CORRADE_COMPARE(aView.size(), a.size());
    CORRADE_COMPARE(static_cast<const void*>(aView.data()), a.data());

    ArrayView<void> aVoidView = a;
    CORRADE_COMPARE(aVoidView.size(), a.size());
    CORRADE_COMPARE(aVoidView.data(), a.data());
}

void StringTest::convertArray() {
    String a = "Allocated hello\0for a verbose world"_s;
    CORRADE_VERIFY(!a.isSmall());

    Array<char> array = std::move(a);
    CORRADE_COMPARE(StringView{array}, "Allocated hello\0for a verbose world"_s);
    CORRADE_COMPARE(array.deleter(), nullptr);
    /* The original allocation includes a null terminator, it should be here as
       well */
    CORRADE_COMPARE(array[array.size()], '\0');

    /* State should be the same as with release(), so of a default-constructed
       instance -- with zero size, but a non-null null-terminated data */
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::convertArraySmall() {
    String a = "this\0world"_s;
    CORRADE_VERIFY(a.isSmall());

    Array<char> array = std::move(a);
    CORRADE_COMPARE(StringView{array}, "this\0world"_s);
    CORRADE_COMPARE(array.deleter(), nullptr);
    /* The newly allocated array should include the null terminator */
    CORRADE_COMPARE(array[array.size()], '\0');

    /* State should be the same as with release(), so of a default-constructed
       instance -- with zero size, but a non-null null-terminated data */
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::convertArraySmallAllocatedInit() {
    String a{AllocatedInit, "this\0world"_s};
    Array<char> array = std::move(a);
    CORRADE_COMPARE(StringView{array}, "this\0world"_s);
    CORRADE_COMPARE(array.deleter(), nullptr);

    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
}

void StringTest::convertArrayCustomDeleter() {
    const char data[] = "Statically allocated hello\0for a verbose world";
    auto deleter = [](char*, std::size_t){};

    String a{data, Containers::arraySize(data) - 1, deleter};
    Array<char> array = std::move(a);
    CORRADE_COMPARE(StringView{array}, "Statically allocated hello\0for a verbose world"_s);
    CORRADE_COMPARE(array.deleter(), deleter);

    /* State should be the same as with release(), so of a default-constructed
       instance -- with zero size, but a non-null null-terminated data */
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], '\0');
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
    CORRADE_VERIFY(String{"hello\0world"_s} == (String{"hello\0world!", 11}));
    CORRADE_VERIFY(String{"hello\0wOrld"_s} != (String{"hello\0world!", 11}));

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

void StringTest::copyConstructLargeAllocatedInit() {
    /* Same as above, for already-large strings it should have no difference */

    char aData[] = "Allocated hello for a verbose world";

    {
        String a{aData, sizeof(aData) - 1, [](char* data, std::size_t){
            ++data[0];
        }};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        /* A copy is made using a default deleter */
        String b{AllocatedInit, a};
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

void StringTest::copyConstructSmallAllocatedInit() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    /* A copy is made using a default deleter */
    String b{AllocatedInit, a};
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(!b.isSmall());
    CORRADE_VERIFY(!b.deleter());
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
        auto deleter = [](char* data, std::size_t){ ++data[0]; };
        String a{aData, sizeof(aData) - 1, deleter};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        /* Everything including the deleter is moved */
        String b = Utility::move(a);
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() == aData);
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_COMPARE(b.deleter(), deleter);
    }

    /* b is deallocated just once */
    CORRADE_COMPARE(aData[0], 'B');

    CORRADE_VERIFY(std::is_nothrow_move_constructible<String>::value);
}

void StringTest::moveConstructLargeAllocatedInit() {
    /* Same as above, for already-large strings it should have no difference */

    char aData[] = "Allocated hello for a verbose world";

    {
        auto deleter = [](char* data, std::size_t){ ++data[0]; };
        String a{aData, sizeof(aData) - 1, deleter};
        CORRADE_VERIFY(!a.isSmall());
        CORRADE_VERIFY(a.deleter());

        /* Everything including the deleter is moved */
        String b{AllocatedInit, Utility::move(a)};
        CORRADE_COMPARE(b, "Allocated hello for a verbose world"_s);
        CORRADE_VERIFY(b.data() == aData);
        CORRADE_VERIFY(!b.isSmall());
        CORRADE_COMPARE(b.deleter(), deleter);
    }

    /* b is deallocated just once */
    CORRADE_COMPARE(aData[0], 'B');

    CORRADE_VERIFY(std::is_nothrow_move_constructible<String>::value);
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
        b = Utility::move(a);
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

    CORRADE_VERIFY(std::is_nothrow_move_assignable<String>::value);
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
        b = Utility::move(a);
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
    String b = Utility::move(a);
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(b.isSmall());
}

void StringTest::moveConstructSmallAllocatedInit() {
    String a = "hello";
    CORRADE_VERIFY(a.isSmall());

    /* A copy is made using a default deleter */
    String b{AllocatedInit, Utility::move(a)};
    CORRADE_COMPARE(b, "hello"_s);
    CORRADE_VERIFY(b.data() != a.data());
    CORRADE_VERIFY(!b.isSmall());
    CORRADE_VERIFY(!b.deleter());
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
        b = Utility::move(a);
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
    CORRADE_COMPARE(a.front(), 'A');
    CORRADE_COMPARE(*(a.end() - 1), 'd');
    CORRADE_COMPARE(*(a.cend() - 1), 'd');
    CORRADE_COMPARE(a.back(), 'd');

    a[14] = '!';
    *a.begin() = 'N';
    ++a.front();
    *(a.end() - 1) = 's';
    ++a.back();
    CORRADE_COMPARE(a, "Ollocated hell! for a verbose worlt");

    const String ca = "Allocated hello for a verbose world";
    CORRADE_VERIFY(!ca.isSmall());
    CORRADE_COMPARE(*ca.begin(), 'A');
    CORRADE_COMPARE(*ca.cbegin(), 'A');
    CORRADE_COMPARE(ca.front(), 'A');
    CORRADE_COMPARE(*(ca.end() - 1), 'd');
    CORRADE_COMPARE(*(ca.cend() - 1), 'd');
    CORRADE_COMPARE(ca.back(), 'd');
    CORRADE_COMPARE(ca[14], 'o');
}

void StringTest::accessSmall() {
    String a = "hello!";
    CORRADE_VERIFY(a.isSmall());
    CORRADE_COMPARE(*a.begin(), 'h');
    CORRADE_COMPARE(*a.cbegin(), 'h');
    CORRADE_COMPARE(a.front(), 'h');
    CORRADE_COMPARE(*(a.end() - 1), '!');
    CORRADE_COMPARE(*(a.cend() - 1), '!');
    CORRADE_COMPARE(a.back(), '!');

    a[4] = '!';
    *(a.end() - 1) = '?';
    *a.begin() = 'J';
    ++a.front();
    ++a.back();
    CORRADE_COMPARE(a, "Kell!@");
}

void StringTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    /* Use a SSO instance to test we're not checking the members directly */
    String a;
    CORRADE_VERIFY(a.isSmall());

    a.front();
    a.back();
    CORRADE_COMPARE(out.str(),
        "Containers::String::front(): string is empty\n"
        "Containers::String::back(): string is empty\n");
}

void StringTest::slice() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    String a = "hello";
    CORRADE_COMPARE(a.slice(1, 4), "ell"_s);
    CORRADE_COMPARE(a.prefix(3), "hel"_s);
    CORRADE_COMPARE(a.prefix(2).flags(), StringViewFlags{});
    CORRADE_COMPARE(a.exceptPrefix(2), "llo"_s);
    CORRADE_COMPARE(a.exceptPrefix(2).flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(a.exceptSuffix(2), "hel"_s);

    const String ca = "hello";
    CORRADE_COMPARE(ca.slice(1, 4), "ell"_s);
    CORRADE_COMPARE(ca.prefix(3), "hel"_s);
    CORRADE_COMPARE(ca.prefix(2).flags(), StringViewFlags{});
    CORRADE_COMPARE(ca.exceptPrefix(2), "llo"_s);
    CORRADE_COMPARE(ca.exceptPrefix(2).flags(), StringViewFlag::NullTerminated);
    CORRADE_COMPARE(ca.exceptSuffix(2), "hel"_s);
}

void StringTest::slicePointer() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

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

void StringTest::split() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    const String ca = "ab//c/def";
    {
        Array<StringView> s = ca.split('/');
        CORRADE_COMPARE_AS(s, arrayView({"ab"_s, ""_s, "c"_s, "def"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(s[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[2].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[3].flags(), StringViewFlag::NullTerminated);
    } {
        Array<StringView> s = ca.splitWithoutEmptyParts('/');
        CORRADE_COMPARE_AS(s, arrayView({"ab"_s, "c"_s, "def"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(s[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[2].flags(), StringViewFlag::NullTerminated);
    }

    String a = "ab//c/def";
    {
        /** @todo any chance this could get done better? */
        String s1 = "ab";
        String s2 = "c";
        String s3 = "def";
        CORRADE_COMPARE_AS(a.split('/'),
            array<MutableStringView>({s1, {}, s2, s3}),
            TestSuite::Compare::Container);
    } {
        String s1 = "ab";
        String s2 = "c";
        String s3 = "def";
        CORRADE_COMPARE_AS(a.splitWithoutEmptyParts('/'),
            array<MutableStringView>({s1, s2, s3}),
            TestSuite::Compare::Container);
    }
}

void StringTest::splitOnAny() {
    constexpr StringView delimiters = ".:;"_s;

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    const String ca = "ab.:c;def";
    {
        Array<StringView> s = ca.splitOnAnyWithoutEmptyParts(delimiters);
        CORRADE_COMPARE_AS(s, arrayView({"ab"_s, "c"_s, "def"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(s[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[2].flags(), StringViewFlag::NullTerminated);
    }

    String a = "ab.:c;def";
    {
        /** @todo any chance this could get done better? */
        String s1 = "ab";
        String s2 = "c";
        String s3 = "def";
        CORRADE_COMPARE_AS(a.splitOnAnyWithoutEmptyParts(delimiters),
            array<MutableStringView>({s1, s2, s3}),
            TestSuite::Compare::Container);
    }
}

void StringTest::splitOnWhitespace() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    const String ca = "ab\n  c\t\rdef";
    {
        Array<StringView> s = ca.splitOnWhitespaceWithoutEmptyParts();
        CORRADE_COMPARE_AS(s, arrayView({"ab"_s, "c"_s, "def"_s}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(s[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(s[2].flags(), StringViewFlag::NullTerminated);
    }

    String a = "ab\n  c\t\rdef";
    {
        /** @todo any chance this could get done better? */
        String s1 = "ab";
        String s2 = "c";
        String s3 = "def";
        CORRADE_COMPARE_AS(a.splitOnWhitespaceWithoutEmptyParts(),
            array<MutableStringView>({s1, s2, s3}),
            TestSuite::Compare::Container);
    }
}

void StringTest::partition() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    const String ca = "ab=c";
    {
        Array3<StringView> p = ca.partition('=');
        CORRADE_COMPARE_AS(p, (Array3<StringView>{"ab", "=", "c"}),
            TestSuite::Compare::Container);
        CORRADE_COMPARE(p[0].flags(), StringViewFlags{});
        CORRADE_COMPARE(p[1].flags(), StringViewFlags{});
        CORRADE_COMPARE(p[2].flags(), StringViewFlag::NullTerminated);
    }

    String a = "ab=c";
    {
        /** @todo any chance this could get done better? */
        String p1 = "ab";
        String p2 = "=";
        String p3 = "c";
        CORRADE_COMPARE_AS(a.partition('='),
            (Array3<MutableStringView>{p1, p2, p3}),
            TestSuite::Compare::Container);
    }
}

void StringTest::add() {
    /* Tests the StringView API as it's better to do it here instead of in
       StringViewTest where we would need to include String */

    CORRADE_COMPARE(""_s + ""_s, "");
    CORRADE_COMPARE("hello"_s + ""_s, "hello");
    CORRADE_COMPARE(""_s + "hello"_s, "hello");
    CORRADE_COMPARE("hello"_s + "world"_s, "helloworld");
}

void StringTest::addNullViews() {
    /* Test that these don't trigger bullying from UBSan (memcpy called with
       null pointers) */

    CORRADE_COMPARE(StringView{} + StringView{}, "");
    CORRADE_COMPARE("hello"_s + nullptr, "hello");
    CORRADE_COMPARE(nullptr + "hello"_s, "hello");
}

void StringTest::join() {
    /* Tests also the StringView API as it's better to do it here instead of in
       StringViewTest where we would need to include String */

    /* Empty */
    CORRADE_COMPARE(", "_s.join({}), "");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({}), "");

    /* One empty value */
    CORRADE_COMPARE(", "_s.join({""}), "");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({""}), "");

    /* Two empty values */
    CORRADE_COMPARE(", "_s.join({"", ""}), ", ");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({"", ""}), "");

    /* One value */
    CORRADE_COMPARE(", "_s.join({"abcdef"}), "abcdef");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({"abcdef"}), "abcdef");

    /* Common case */
    CORRADE_COMPARE(", "_s.join({"ab", "c", "def"}),
        "ab, c, def");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({"ab", "c", "def"}),
        "ab, c, def");

    /* Empty parts, also the overload directly on a String */
    CORRADE_COMPARE(", "_s.join({"ab", "", "c", "def", "", ""}),
        "ab, , c, def, , ");
    CORRADE_COMPARE(String{", "}.join({"ab", "", "c", "def", "", ""}),
        "ab, , c, def, , ");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({"ab", "", "c", "def", "", ""}),
        "ab, c, def");
    CORRADE_COMPARE(String{", "}.joinWithoutEmptyParts({"ab", "", "c", "def", "", ""}),
        "ab, c, def");
}

void StringTest::joinNullViews() {
    /* Test that these don't trigger bullying from UBSan (memcpy called with
       null pointers) */

    /* Null values */
    CORRADE_COMPARE(", "_s.join({nullptr, nullptr}), ", ");
    CORRADE_COMPARE(", "_s.joinWithoutEmptyParts({nullptr, nullptr}), "");

    /* Null joiner */
    CORRADE_COMPARE(StringView{nullptr}.join({"ab", "c", "def"}),
        "abcdef");
    CORRADE_COMPARE(StringView{nullptr}.joinWithoutEmptyParts({"ab", "c", "def"}),
        "abcdef");
}

void StringTest::hasPrefix() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics */

    CORRADE_VERIFY(String{"overcomplicated"}.hasPrefix("over"));
    CORRADE_VERIFY(!String{"overcomplicated"}.hasPrefix("oven"));

    CORRADE_VERIFY(String{"hello"}.hasPrefix('h'));
    CORRADE_VERIFY(!String{"hello"}.hasPrefix('e'));
}

void StringTest::hasSuffix() {
    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics */

    CORRADE_VERIFY(String{"overcomplicated"}.hasSuffix("complicated"));
    CORRADE_VERIFY(!String{"overcomplicated"}.hasSuffix("somplicated"));

    CORRADE_VERIFY(String{"hello"}.hasSuffix('o'));
    CORRADE_VERIFY(!String{"hello"}.hasSuffix('l'));
}

template<class T> void StringTest::exceptPrefix() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    T a{"overcomplicated"};

    typename ConstTraits<T>::ViewType b = a.exceptPrefix("over");
    CORRADE_COMPARE(b, "complicated"_s);
    CORRADE_COMPARE(b.flags(), StringViewFlag::NullTerminated);
}

void StringTest::exceptPrefixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    String a{"overcomplicated"};
    const String ca{"overcomplicated"};

    std::ostringstream out;
    Error redirectOutput{&out};
    a.exceptPrefix("complicated");
    ca.exceptPrefix("complicated");
    /* Assert is coming from StringView */
    CORRADE_COMPARE(out.str(),
        "Containers::StringView::exceptPrefix(): string doesn't begin with complicated\n"
        "Containers::StringView::exceptPrefix(): string doesn't begin with complicated\n");
}

CORRADE_HAS_TYPE(CanExceptPrefixBeCalledWith, decltype(String{}.exceptPrefix(std::declval<T>())));

void StringTest::exceptPrefixDisabledOverloads() {
    /* I thought I could use std::is_invocable (or a C++11 backport of it) to
       easily test this. Boy I was wrong, that API is absolutely useless, while
       the CORRADE_HAS_TYPE() macro is the best thing ever.

       Same as exceptSuffixDisabledOverloads(), please keep consistent */

    CORRADE_VERIFY(CanExceptPrefixBeCalledWith<std::size_t>::value);
    /* Doing exceptPrefix(1) should work */
    CORRADE_VERIFY(CanExceptPrefixBeCalledWith<int>::value);
    /* Borderline dangerous, but decltype('a') == char, so this should be ok */
    CORRADE_VERIFY(CanExceptPrefixBeCalledWith<signed char>::value);
    /* exceptPrefix('a') should be disallowed because it's too easy to misuse
       e.g. as exceptPrefix(std::toupper('a')), resulting in exceptPrefix(65) */
    CORRADE_VERIFY(!CanExceptPrefixBeCalledWith<char>::value);
}

template<class T> void StringTest::exceptSuffix() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    String a{"overcomplicated"};

    typename ConstTraits<T>::ViewType b = a.exceptSuffix("complicated");
    CORRADE_COMPARE(b, "over"_s);
    CORRADE_COMPARE(b.flags(), StringViewFlags{});

    CORRADE_COMPARE(a.exceptSuffix("").flags(), StringViewFlag::NullTerminated);
}

void StringTest::exceptSuffixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    String a{"overcomplicated"};
    const String ca{"overcomplicated"};

    std::ostringstream out;
    Error redirectOutput{&out};
    a.exceptSuffix("over");
    ca.exceptSuffix("over");
    /* Assert is coming from StringView */
    CORRADE_COMPARE(out.str(),
        "Containers::StringView::exceptSuffix(): string doesn't end with over\n"
        "Containers::StringView::exceptSuffix(): string doesn't end with over\n");
}

CORRADE_HAS_TYPE(CanExceptSuffixBeCalledWith, decltype(String{}.exceptSuffix(std::declval<T>())));

void StringTest::exceptSuffixDisabledOverloads() {
    /* I thought I could use std::is_invocable (or a C++11 backport of it) to
       easily test this. Boy I was wrong, that API is absolutely useless, while
       the CORRADE_HAS_TYPE() macro is the best thing ever.

       Same as exceptPrefixDisabledOverloads(), please keep consistent */

    CORRADE_VERIFY(CanExceptSuffixBeCalledWith<std::size_t>::value);
    /* Ding exceptSuffix(1) should work */
    CORRADE_VERIFY(CanExceptSuffixBeCalledWith<int>::value);
    /* Borderline dangerous, but decltype('a') == char, so this should be ok */
    CORRADE_VERIFY(CanExceptSuffixBeCalledWith<signed char>::value);
    /* exceptPrefix('a') should be disallowed because it's too easy to misuse
       e.g. as exceptPrefix(std::toupper('a')), resulting in exceptPrefix(65) */
    CORRADE_VERIFY(!CanExceptSuffixBeCalledWith<char>::value);
}

template<class T> void StringTest::trimmed() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    T a{" \t abc \n "};
    {
        typename ConstTraits<T>::ViewType trimmed = a.trimmedPrefix();
        CORRADE_COMPARE(trimmed, "abc \n "_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType trimmed = a.trimmedSuffix();
        CORRADE_COMPARE(trimmed, " \t abc"_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType trimmed = a.trimmed();
        CORRADE_COMPARE(trimmed, "abc"_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlags{});
    }

    T b{"oubya"};
    {
        typename ConstTraits<T>::ViewType trimmed = b.trimmedPrefix("aeiyou");
        CORRADE_COMPARE(trimmed, "bya"_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType trimmed = b.trimmedSuffix("aeiyou");
        CORRADE_COMPARE(trimmed, "oub"_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType trimmed = b.trimmed("aeiyou");
        CORRADE_COMPARE(trimmed, "b"_s);
        CORRADE_COMPARE(trimmed.flags(), StringViewFlags{});
    }
}

template<class T> void StringTest::find() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    /* Duplicated word to test that it's not delegated to findLast() */
    T a{"hello hello world"};
    {
        CORRADE_VERIFY(a.contains("hello"));

        typename ConstTraits<T>::ViewType found = a.find("hello");
        CORRADE_COMPARE(found, "hello"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        CORRADE_VERIFY(a.contains("world"));

        typename ConstTraits<T>::ViewType found = a.find("world");
        CORRADE_COMPARE(found, "world"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 12));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        CORRADE_VERIFY(!a.contains("cursed"));

        typename ConstTraits<T>::ViewType found = a.find("cursed");
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(nullptr));
    } {
        CORRADE_VERIFY(a.contains('h'));

        typename ConstTraits<T>::ViewType found = a.find('h');
        CORRADE_COMPARE(found, "h"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        CORRADE_VERIFY(a.contains('d'));

        typename ConstTraits<T>::ViewType found = a.find('d');
        CORRADE_COMPARE(found, "d"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 16));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        CORRADE_VERIFY(!a.contains('c'));

        typename ConstTraits<T>::ViewType found = a.find('c');
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(nullptr));
    }
}

template<class T> void StringTest::findOr() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* Mostly the same as find(), except that we expect a different pointer in
       case of failure. Non-failure cases are kept to verify it's not
       propagated to findLastOr() by accident. */

    T a{"hello hello world"};
    {
        typename ConstTraits<T>::ViewType found = a.findOr("hello", a.end());
        CORRADE_COMPARE(found, "hello"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findOr("world", a.end());
        CORRADE_COMPARE(found, "world"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 12));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findOr("cursed", a.end());
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.end()));
    } {
        typename ConstTraits<T>::ViewType found = a.findOr('h', a.end());
        CORRADE_COMPARE(found, "h"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findOr('d', a.end());
        CORRADE_COMPARE(found, "d"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 16));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findOr('c', a.end());
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.end()));
    }
}

template<class T> void StringTest::findLast() {
    /* Mostly the same as find(), except that we don't test contains() which is
       implemented with the same algorithm as find() */

    setTestCaseTemplateName(ConstTraits<T>::name());

    /* These rely on StringView conversion and then delegate there so we don't
       need to verify SSO behavior, only the basics and flag propagation */

    /* Duplicated word to test that it's not delegated to find() */
    T a{"hello world world"};
    {
        typename ConstTraits<T>::ViewType found = a.findLast("hello");
        CORRADE_COMPARE(found, "hello"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findLast("world");
        CORRADE_COMPARE(found, "world"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 12));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findLast("cursed");
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(nullptr));
    } {
        typename ConstTraits<T>::ViewType found = a.findLast('h');
        CORRADE_COMPARE(found, "h"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findLast('d');
        CORRADE_COMPARE(found, "d"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 16));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findLast('c');
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(nullptr));
    }
}

template<class T> void StringTest::findLastOr() {
    setTestCaseTemplateName(ConstTraits<T>::name());

    /* Mostly the same as findLast(), except that we expect a different pointer
       in case of failure. Non-failure cases are kept to verify it's not
       propagated to findOr() by accident. */

    T a{"hello world world"};
    {
        typename ConstTraits<T>::ViewType found = a.findLastOr("hello", a.end());
        CORRADE_COMPARE(found, "hello"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findLastOr("world", a.end());
        CORRADE_COMPARE(found, "world"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 12));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findLastOr("cursed", a.end());
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.end()));
    } {
        typename ConstTraits<T>::ViewType found = a.findLastOr('h', a.end());
        CORRADE_COMPARE(found, "h"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(found.flags(), StringViewFlags{});
    } {
        typename ConstTraits<T>::ViewType found = a.findLastOr('d', a.end());
        CORRADE_COMPARE(found, "d"_s);
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.data() + 16));
        CORRADE_COMPARE(found.flags(), StringViewFlag::NullTerminated);
    } {
        typename ConstTraits<T>::ViewType found = a.findLastOr('c', a.end());
        CORRADE_VERIFY(found.isEmpty());
        CORRADE_COMPARE(found.data(), static_cast<const void*>(a.end()));
    }
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

void StringTest::defaultDeleter() {
    String a{Corrade::ValueInit, 50};
    CORRADE_VERIFY(!a.isSmall());
    CORRADE_VERIFY(a.deleter() == nullptr);
}

int CustomDeleterCallCount = 0;

void StringTest::customDeleter() {
    CustomDeleterCallCount = 0;
    char data[26]{'\xfc'};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        String a{data, 25, [](char* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], '\xfc');
            CORRADE_COMPARE(size, 25);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a.data() == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void StringTest::customDeleterZeroSize() {
    CustomDeleterCallCount = 0;
    /* Zero size forces us to have data[0] a null terminator, so use the second
       element for an "expected content" check */
    char data[26]{0, '\xfc'};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        String a{data, 0, [](char* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[1], '\xfc');
            CORRADE_COMPARE(size, 0);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a.data() == data);
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter should unconditionally get called here as well, consistently
       with what Array does */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void StringTest::customDeleterMovedOutInstance() {
    CustomDeleterCallCount = 0;
    char data[26]{};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        String a{data, 25, [](char*, std::size_t) {
            ++CustomDeleterCallCount;
        }};
        CORRADE_COMPARE(CustomDeleterCallCount, 0);

        String b = std::move(a);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter got reset to nullptr in a, which means the function gets
       called only once, consistently with what Array does */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringTest)
