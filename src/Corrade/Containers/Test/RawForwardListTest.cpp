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

#include "Corrade/Containers/Implementation/RawForwardList.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct RawForwardListTest: TestSuite::Tester {
    explicit RawForwardListTest();

    void insert();
    void remove();
    void next();
};

RawForwardListTest::RawForwardListTest() {
    addTests({&RawForwardListTest::insert,
              &RawForwardListTest::remove,
              &RawForwardListTest::next});
}

struct Item {
    Item* next{};
};

void RawForwardListTest::insert() {
    Item a, b, c;
    Item* list{};

    Implementation::forwardListInsert(list, a);
    CORRADE_COMPARE(list, &a);
    CORRADE_COMPARE(a.next, &a);

    Implementation::forwardListInsert(list, b);
    CORRADE_COMPARE(list, &b);
    CORRADE_COMPARE(b.next, &a);
    CORRADE_COMPARE(a.next, &a);

    Implementation::forwardListInsert(list, c);
    CORRADE_COMPARE(list, &c);
    CORRADE_COMPARE(c.next, &b);
    CORRADE_COMPARE(b.next, &a);
    CORRADE_COMPARE(a.next, &a);

    /* Inserting existing should be a no-op */
    Implementation::forwardListInsert(list, b);
    CORRADE_COMPARE(list, &c);
    CORRADE_COMPARE(c.next, &b);
    CORRADE_COMPARE(b.next, &a);
    CORRADE_COMPARE(a.next, &a);
}

void RawForwardListTest::remove() {
    Item a, b, c, d;

    Item* list{};
    Implementation::forwardListInsert(list, a);
    Implementation::forwardListInsert(list, b);
    Implementation::forwardListInsert(list, c);
    Implementation::forwardListInsert(list, d);
    CORRADE_COMPARE(list, &d);
    CORRADE_COMPARE(d.next, &c);
    CORRADE_COMPARE(c.next, &b);
    CORRADE_COMPARE(b.next, &a);
    CORRADE_COMPARE(a.next, &a);

    /* From the middle */
    Implementation::forwardListRemove(list, b);
    CORRADE_COMPARE(b.next, nullptr);
    CORRADE_COMPARE(list, &d);
    CORRADE_COMPARE(d.next, &c);
    CORRADE_COMPARE(c.next, &a);
    CORRADE_COMPARE(a.next, &a);

    /* From the end */
    Implementation::forwardListRemove(list, a);
    CORRADE_COMPARE(a.next, nullptr);
    CORRADE_COMPARE(list, &d);
    CORRADE_COMPARE(d.next, &c);
    CORRADE_COMPARE(c.next, &c);

    /* From the beginning */
    Implementation::forwardListRemove(list, d);
    CORRADE_COMPARE(d.next, nullptr);
    CORRADE_COMPARE(list, &c);
    CORRADE_COMPARE(c.next, &c);

    /* From the beginning and also the end */
    Implementation::forwardListRemove(list, c);
    CORRADE_COMPARE(c.next, nullptr);
    CORRADE_COMPARE(list, nullptr);

    /* Removing an item that isn't there should be a no-op */
    Implementation::forwardListInsert(list, a);
    CORRADE_COMPARE(list, &a);
    CORRADE_COMPARE(a.next, &a);
    Implementation::forwardListRemove(list, b);
    CORRADE_COMPARE(list, &a);
    CORRADE_COMPARE(a.next, &a);
    CORRADE_COMPARE(b.next, nullptr);
}

void RawForwardListTest::next() {
    Item a, b, c;

    Item* list{};
    Implementation::forwardListInsert(list, a);
    Implementation::forwardListInsert(list, b);
    Implementation::forwardListInsert(list, c);
    CORRADE_COMPARE(list, &c);
    CORRADE_COMPARE(c.next, &b);
    CORRADE_COMPARE(b.next, &a);
    CORRADE_COMPARE(a.next, &a);

    /* Second item */
    CORRADE_COMPARE(Implementation::forwardListNext(*list), &b);
    CORRADE_COMPARE(Implementation::forwardListNext(c), &b);

    /* Third / last item */
    CORRADE_COMPARE(Implementation::forwardListNext(b), &a);

    /* End */
    CORRADE_COMPARE(Implementation::forwardListNext(a), nullptr);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::RawForwardListTest)
