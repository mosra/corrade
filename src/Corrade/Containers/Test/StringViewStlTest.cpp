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

#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StringViewStlTest: TestSuite::Tester {
    explicit StringViewStlTest();

    void convertToStlString();
    void convertToStlStringEmpty();
    void convertMutableToStlString();
    void convertMutableToStlStringEmpty();
    void convertFromStlString();
    void convertFromStlStringEmpty();
    void convertMutableFromStlString();
    void convertMutableFromStlStringEmpty();
};

StringViewStlTest::StringViewStlTest() {
    addTests({&StringViewStlTest::convertToStlString,
              &StringViewStlTest::convertToStlStringEmpty,
              &StringViewStlTest::convertMutableToStlString,
              &StringViewStlTest::convertMutableToStlStringEmpty,
              &StringViewStlTest::convertFromStlString,
              &StringViewStlTest::convertFromStlStringEmpty,
              &StringViewStlTest::convertMutableFromStlString,
              &StringViewStlTest::convertMutableFromStlStringEmpty});
}

using namespace Literals;

void StringViewStlTest::convertToStlString() {
    StringView a = "hello\0!!!"_s;
    std::string b = a;
    CORRADE_COMPARE(b, (std::string{"hello\0!!!", 9}));
}

void StringViewStlTest::convertToStlStringEmpty() {
    StringView a;
    std::string b = a;
    CORRADE_COMPARE(b, std::string{});
}

void StringViewStlTest::convertMutableToStlString() {
    char data[] = "hello\0!!!";
    MutableStringView a{data, 9};
    std::string b = a;
    CORRADE_COMPARE(b, (std::string{"hello\0!!!", 9}));
}

void StringViewStlTest::convertMutableToStlStringEmpty() {
    MutableStringView a;
    std::string b = a;
    CORRADE_COMPARE(b, std::string{});
}

void StringViewStlTest::convertFromStlString() {
    const std::string a{"hello\0!!!", 9};
    StringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);
}

void StringViewStlTest::convertFromStlStringEmpty() {
    const std::string a;
    StringView b = a;
    CORRADE_COMPARE(b, ""_s);
}

void StringViewStlTest::convertMutableFromStlString() {
    std::string a{"hello\0!!!", 9};
    MutableStringView b = a;
    CORRADE_COMPARE(b, "hello\0!!!"_s);

    /* Only a mutable string instance should be convertible to a mutable view */
    CORRADE_VERIFY((std::is_convertible<std::string&, MutableStringView>::value));
    CORRADE_VERIFY(!(std::is_convertible<const std::string&, MutableStringView>::value));
}

void StringViewStlTest::convertMutableFromStlStringEmpty() {
    std::string a;
    MutableStringView b = a;
    CORRADE_COMPARE(b, ""_s);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringViewStlTest)
