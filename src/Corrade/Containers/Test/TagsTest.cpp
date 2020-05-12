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

#include "Corrade/Containers/Tags.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct TagsTest: TestSuite::Tester {
    explicit TagsTest();

    void noDefaultConstructor();
    void inlineDefinition();
};

TagsTest::TagsTest() {
    addTests({&TagsTest::noDefaultConstructor,
              &TagsTest::inlineDefinition});
}

void TagsTest::noDefaultConstructor() {
    CORRADE_VERIFY(!std::is_default_constructible<DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<ValueInitT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<NoInitT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<NoCreateT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<DirectInitT>::value);
}

void TagsTest::inlineDefinition() {
    CORRADE_VERIFY((std::is_same<decltype(DefaultInit), const DefaultInitT>::value));
    CORRADE_VERIFY((std::is_same<decltype(ValueInit), const ValueInitT>::value));
    CORRADE_VERIFY((std::is_same<decltype(NoInit), const NoInitT>::value));
    CORRADE_VERIFY((std::is_same<decltype(NoCreate), const NoCreateT>::value));
    CORRADE_VERIFY((std::is_same<decltype(DirectInit), const DirectInitT>::value));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TagsTest)
