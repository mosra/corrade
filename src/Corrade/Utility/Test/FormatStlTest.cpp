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

#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct FormatStlTest: TestSuite::Tester {
    explicit FormatStlTest();

    void string();
    void stringEmpty();
    void stringIntoAppend();
    void stringIntoInsert();

    void file();

    /* std::string_view is tested in FormatStlStringViewTest instead */
};

FormatStlTest::FormatStlTest() {
    addTests({&FormatStlTest::string,
              &FormatStlTest::stringEmpty,
              &FormatStlTest::stringIntoAppend,
              &FormatStlTest::stringIntoInsert,

              &FormatStlTest::file});
}

void FormatStlTest::string() {
    /* This tests both string input and string output, yes, lazy */
    CORRADE_COMPARE(formatString("hello {}", std::string{"worlds", 5}),
        "hello world");
    CORRADE_COMPARE(formatString("hello {}", std::string{"world\0, i guess?", 16}),
        (std::string{"hello world\0, i guess?", 22}));
}

void FormatStlTest::stringEmpty() {
    /* Empty string should not cause any issues with data access */
    CORRADE_COMPARE(formatString("hello{}!", std::string{}), "hello!");
}

void FormatStlTest::stringIntoAppend() {
    /* Returned size should be including start offset */
    std::string hello = "hello";
    CORRADE_COMPARE(formatInto(hello, hello.size(), ", {}!", "world"), 13);
    CORRADE_COMPARE(hello, "hello, world!");
}

void FormatStlTest::stringIntoInsert() {
    /* Returned size should be including start offset but be less than string size */
    std::string hello = "hello, __________! Happy to see you!";
    CORRADE_COMPARE(hello.size(), 36);
    CORRADE_COMPARE(formatInto(hello, 8, "Frank"), 13);
    CORRADE_COMPARE(hello, "hello, _Frank____! Happy to see you!");
    CORRADE_COMPARE(hello.size(), 36);
}

void FormatStlTest::file() {
    Containers::String filename = Path::join(FORMAT_WRITE_TEST_DIR, "format-stl.txt");
    CORRADE_VERIFY(Path::make(FORMAT_WRITE_TEST_DIR));
    if(Path::exists(filename))
        CORRADE_VERIFY(Path::remove(filename));

    {
        FILE* f = std::fopen(filename.data(), "w");
        CORRADE_VERIFY(f);
        Containers::ScopeGuard e{f, fclose};
        formatInto(f, "A {} {} {}", "string", std::string{"file"}, 27);
    }
    CORRADE_COMPARE_AS(filename,
        "A string file 27",
        TestSuite::Compare::FileToString);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::FormatStlTest)
