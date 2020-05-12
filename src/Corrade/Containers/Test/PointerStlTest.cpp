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

#include "Corrade/Containers/PointerStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct PointerStlTest: TestSuite::Tester {
    explicit PointerStlTest();

    void convert();
};

PointerStlTest::PointerStlTest() {
    addTests({&PointerStlTest::convert});
}

void PointerStlTest::convert() {
    std::unique_ptr<int> a{new int{5}};
    int* ptr = a.get();
    CORRADE_VERIFY(ptr);
    CORRADE_COMPARE(*ptr, 5);

    Pointer<int> b = std::move(a); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(b.get(), ptr);
    CORRADE_COMPARE(*b, 5);
    CORRADE_VERIFY(!a);

    std::unique_ptr<int> c = std::move(b); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(c.get(), ptr);
    CORRADE_COMPARE(*c, 5);
    CORRADE_VERIFY(!b);

    auto d = pointer(std::unique_ptr<int>{new int{17}});
    CORRADE_VERIFY((std::is_same<decltype(d), Pointer<int>>::value));
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(*d, 17);

    /* Non-move conversion is not allowed */
    CORRADE_VERIFY((std::is_convertible<std::unique_ptr<int>&&, Pointer<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const std::unique_ptr<int>&, Pointer<int>>::value));
    CORRADE_VERIFY((std::is_convertible<Pointer<int>&&, std::unique_ptr<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Pointer<int>&, std::unique_ptr<int>>::value));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PointerStlTest)
