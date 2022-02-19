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

#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StringStlTest: TestSuite::Tester {
    explicit StringStlTest();

    void convertToStlString();
    void convertToStlStringEmpty();
    void convertFromStlString();
    void convertFromStlStringEmpty();

    void convertViewToStlString();
    void convertViewToStlStringEmpty();
    void convertMutableViewToStlString();
    void convertMutableViewToStlStringEmpty();
    void convertViewFromStlString();
    void convertViewFromStlStringEmpty();
    void convertMutableViewFromStlString();
    void convertMutableViewFromStlStringEmpty();
};

StringStlTest::StringStlTest() {
    addTests({&StringStlTest::convertToStlString,
              &StringStlTest::convertToStlStringEmpty,
              &StringStlTest::convertFromStlString,
              &StringStlTest::convertFromStlStringEmpty,

              &StringStlTest::convertViewToStlString,
              &StringStlTest::convertViewToStlStringEmpty,
              &StringStlTest::convertMutableViewToStlString,
              &StringStlTest::convertMutableViewToStlStringEmpty,
              &StringStlTest::convertViewFromStlString,
              &StringStlTest::convertViewFromStlStringEmpty,
              &StringStlTest::convertMutableViewFromStlString,
              &StringStlTest::convertMutableViewFromStlStringEmpty});
}

using namespace Literals;

void StringStlTest::convertToStlString() {
    String a = "hello\0!!!"_s;
    std::string b = a;
    CORRADE_COMPARE(b, (std::string{"hello\0!!!", 9}));
}

void StringStlTest::convertToStlStringEmpty() {
    String a;
    std::string b = a;
    CORRADE_COMPARE(b, std::string{});
}

void StringStlTest::convertFromStlString() {
    const std::string a{"hello\0!!!", 9};
    String b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
}

void StringStlTest::convertFromStlStringEmpty() {
    const std::string a;
    String b = a;
    CORRADE_COMPARE(b, ""_s);
}

void StringStlTest::convertViewToStlString() {
    StringView a = "hello\0!!!"_s;
    std::string b = a;
    CORRADE_COMPARE(b, (std::string{"hello\0!!!", 9}));
}

void StringStlTest::convertViewToStlStringEmpty() {
    StringView a;
    std::string b = a;
    CORRADE_COMPARE(b, std::string{});
}

void StringStlTest::convertMutableViewToStlString() {
    char data[] = "hello\0!!!";
    MutableStringView a{data, 9};
    std::string b = a;
    CORRADE_COMPARE(b, (std::string{"hello\0!!!", 9}));
}

void StringStlTest::convertMutableViewToStlStringEmpty() {
    MutableStringView a;
    std::string b = a;
    CORRADE_COMPARE(b, std::string{});
}

void StringStlTest::convertViewFromStlString() {
    const std::string a{"hello\0!!!", 9};
    StringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlTest::convertViewFromStlStringEmpty() {
    const std::string a;
    StringView b = a;
    CORRADE_COMPARE(b, ""_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlTest::convertMutableViewFromStlString() {
    std::string a{"hello\0!!!", 9};
    MutableStringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));

    /* Only a mutable string instance should be convertible to a mutable view */
    CORRADE_VERIFY(std::is_convertible<std::string&, MutableStringView>::value);
    CORRADE_VERIFY(!std::is_convertible<const std::string&, MutableStringView>::value);
}

void StringStlTest::convertMutableViewFromStlStringEmpty() {
    std::string a;
    MutableStringView b = a;
    CORRADE_COMPARE(b, ""_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringStlTest)
