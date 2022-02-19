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

#include "Corrade/Containers/StringStlView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStlStringView.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StringStlViewTest: TestSuite::Tester {
    explicit StringStlViewTest();

    void convertToStlStringView();
    void convertToStlStringViewEmpty();
    void convertFromStlStringView();
    void convertFromStlStringViewEmpty();

    void convertViewToStlStringView();
    void convertViewToStlStringViewEmpty();
    void convertMutableViewToStlStringView();
    void convertMutableViewToStlStringViewEmpty();
    void convertViewFromStlStringView();
    void convertViewFromStlStringViewEmpty();

    void convertMutableViewFromStlStringView();
};

StringStlViewTest::StringStlViewTest() {
    addTests({&StringStlViewTest::convertToStlStringView,
              &StringStlViewTest::convertToStlStringViewEmpty,
              &StringStlViewTest::convertFromStlStringView,
              &StringStlViewTest::convertFromStlStringViewEmpty,

              &StringStlViewTest::convertViewToStlStringView,
              &StringStlViewTest::convertViewToStlStringViewEmpty,
              &StringStlViewTest::convertMutableViewToStlStringView,
              &StringStlViewTest::convertMutableViewToStlStringViewEmpty,
              &StringStlViewTest::convertViewFromStlStringView,
              &StringStlViewTest::convertViewFromStlStringViewEmpty,

              &StringStlViewTest::convertMutableViewFromStlStringView});
}

using namespace Literals;
using namespace std::string_view_literals;

void StringStlViewTest::convertToStlStringView() {
    String a = "hello\0!!!"_s;
    std::string_view b = a;
    CORRADE_COMPARE(b, "hello\0!!!"sv);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlViewTest::convertToStlStringViewEmpty() {
    String a;
    std::string_view b = a;
    CORRADE_COMPARE(b, std::string_view{});
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlViewTest::convertFromStlStringView() {
    const std::string_view a = "hello\0!!!"sv;
    String b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
}

void StringStlViewTest::convertFromStlStringViewEmpty() {
    const std::string_view a;
    String b = a;
    CORRADE_COMPARE(b, ""_s);
}

void StringStlViewTest::convertViewToStlStringView() {
    StringView a = "hello\0!!!"_s;
    std::string_view b = a;
    CORRADE_COMPARE(b, "hello\0!!!"sv);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlViewTest::convertViewToStlStringViewEmpty() {
    StringView a;
    std::string_view b = a;
    CORRADE_COMPARE(b, std::string_view{});
    CORRADE_COMPARE(b.data(), static_cast<const void*>(nullptr));
}

void StringStlViewTest::convertMutableViewToStlStringView() {
    char data[] = "hello\0!!!";
    MutableStringView a{data, 9};
    std::string_view b = a;
    CORRADE_COMPARE(b, "hello\0!!!"sv);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
}

void StringStlViewTest::convertMutableViewToStlStringViewEmpty() {
    MutableStringView a;
    std::string_view b = a;
    CORRADE_COMPARE(b, std::string_view{});
    CORRADE_COMPARE(b.data(), static_cast<const void*>(nullptr));
}

void StringStlViewTest::convertViewFromStlStringView() {
    const std::string_view a = "hello\0!!!"sv;
    StringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlViewTest::convertViewFromStlStringViewEmpty() {
    const std::string_view a = "hello\0!!!"sv;
    StringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
    CORRADE_COMPARE(b.data(), static_cast<const void*>(a.data()));
}

void StringStlViewTest::convertMutableViewFromStlStringView() {
    /* A string_view should never be convertible to a mutable view */
    CORRADE_VERIFY(std::is_convertible<std::string_view, StringView>::value);
    CORRADE_VERIFY(!std::is_convertible<std::string_view, MutableStringView>::value);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringStlViewTest)
