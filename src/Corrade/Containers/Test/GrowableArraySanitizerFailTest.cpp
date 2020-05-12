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

#include <string>
#include <vector>

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/Arguments.h"

/* No __has_feature on GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60512
   Using a dedicated macro instead: https://stackoverflow.com/a/34814667 */
#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif
#endif
#ifdef __SANITIZE_ADDRESS__
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
extern "C" void __sanitizer_set_death_callback(void (*)(void));
#endif

namespace Corrade { namespace Containers { namespace Test { namespace {

struct GrowableArraySanitizerFailTest: TestSuite::Tester {
    explicit GrowableArraySanitizerFailTest();

    void testString();
    void testVector();
    void test();
};

GrowableArraySanitizerFailTest::GrowableArraySanitizerFailTest(): TestSuite::Tester{TesterConfiguration{}.setSkippedArgumentPrefixes({"test-stl"})} {

    Utility::Arguments args{"test-stl"};
    args.addOption("container", "").setHelp("container", "test behavior on a specific STL container instead", "vector|string")
        .parse(arguments().first, arguments().second);

    if(args.value("container") == "vector")
        addTests({&GrowableArraySanitizerFailTest::testVector});
    else if(args.value("container") == "string")
        addTests({&GrowableArraySanitizerFailTest::testString});
    else if(args.value("container").empty())
        addTests({&GrowableArraySanitizerFailTest::test});
    else Utility::Fatal{} << "Invalid --test-stl-option passed";

    /* So we exit cleanly without generating a stack trace each time */
    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    __sanitizer_set_death_callback([]() { std::exit(0); });
    #endif
}

void GrowableArraySanitizerFailTest::testVector() {
    std::vector<int> vector;
    vector.reserve(100);
    vector.resize(80);
    CORRADE_COMPARE(vector.size(), 80);
    CORRADE_COMPARE(vector.capacity(), 100);

    #ifndef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    CORRADE_SKIP("ASan not enabled");
    #endif
    vector.data()[80] = 3;

    #ifdef CORRADE_TARGET_LIBSTDCXX
    /* https://github.com/google/sanitizers/wiki/AddressSanitizerClangVsGCC-(6.0-vs-8.1)#feature */
    CORRADE_EXPECT_FAIL("ASan didn't abort, you need to use a special build of libstdc++ with _GLIBCXX_SANITIZE_VECTOR enabled");
    #endif
    /* No mention of this at https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/,
       need to test to be sure it's not there. */
    CORRADE_VERIFY(!"This shouldn't be reached");
}

void GrowableArraySanitizerFailTest::testString() {
    std::string string;
    string.reserve(100);
    string.resize(80);
    CORRADE_COMPARE(string.size(), 80);
    /* libc++ on 64bit reports 111 */
    CORRADE_COMPARE_AS(string.capacity(), 100, TestSuite::Compare::GreaterOrEqual);

    #ifndef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    CORRADE_SKIP("ASan not enabled");
    #endif
    (&string[0])[80] = 3;
    /* https://gcc.gnu.org/ml/gcc-patches/2017-07/msg00458.html, nothing in the
       WIP GCC 10 changelog yet either, libc++ has it "under review" (but can't
       find the PR and master doesn't have it. Last checked: Dec 5 2019.
       https://github.com/google/sanitizers/wiki/AddressSanitizerClangVsGCC-(6.0-vs-8.1)#feature */
    #if defined(CORRADE_TARGET_LIBSTDCXX) || defined(CORRADE_TARGET_LIBCXX)
    CORRADE_EXPECT_FAIL("libc++ / libstdc++ doesn't have sanitized std::string yet");
    #endif
    /* No mention of this at https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/,
       need to test to be sure it's not there. */
    CORRADE_VERIFY(!"This shouldn't be reached");
}

void GrowableArraySanitizerFailTest::test() {
    Array<int> array;
    arrayReserve(array, 100);
    arrayResize(array, 80);
    CORRADE_COMPARE(array.size(), 80);
    CORRADE_COMPARE(arrayCapacity(array), 100);

    #ifndef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    CORRADE_SKIP("ASan not enabled");
    #endif
    /* Even though the memory *is* there, this should cause ASan to complain */
    array[80] = 5;
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::GrowableArraySanitizerFailTest)
