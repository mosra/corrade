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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Resource.h"

#include "configure.h"

static void initialize() {
    CORRADE_RESOURCE_INITIALIZE(ResourceTestData)
}

static void initializeAnother() {
    CORRADE_RESOURCE_INITIALIZE(ResourceTestNothingData)
}

static void finalize() {
    CORRADE_RESOURCE_FINALIZE(ResourceTestData)
}

static void finalizeAnother() {
    CORRADE_RESOURCE_FINALIZE(ResourceTestNothingData)
}

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ResourceStaticTest: TestSuite::Tester {
    explicit ResourceStaticTest();

    /* These test that the linked list isn't getting overwritten in a bad way
       (such as clearing the `next` pointer before adding an item, which was
       the case before). Thorough tests for the linked list operations are in
       Containers/Test/RawForwardListTest.cpp */
    void initializeOnce();
    void initializeTwice();
    void initializeTwiceMixedWithAnother();
};

ResourceStaticTest::ResourceStaticTest() {
    addTests({&ResourceStaticTest::initializeOnce,
              &ResourceStaticTest::initializeTwice,
              &ResourceStaticTest::initializeTwiceMixedWithAnother});
}

void ResourceStaticTest::initializeOnce() {
    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    initialize();

    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    Resource r("test");
    CORRADE_COMPARE_AS(r.get("predisposition.bin"),
        Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(r.get("consequence.bin"),
        Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);

    finalize();

    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

void ResourceStaticTest::initializeTwice() {
    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    initialize();
    initialize();

    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    Resource r("test");
    CORRADE_COMPARE_AS(r.get("predisposition.bin"),
        Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(r.get("consequence.bin"),
        Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);

    finalize();
    finalize();

    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

void ResourceStaticTest::initializeTwiceMixedWithAnother() {
    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nothing"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    initialize();
    initializeAnother();
    initialize();

    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(Resource::hasGroup("nothing"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));

    Resource r("test");
    CORRADE_COMPARE_AS(r.get("predisposition.bin"),
        Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(r.get("consequence.bin"),
        Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);

    finalize();
    finalizeAnother();
    finalize();

    CORRADE_VERIFY(!Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nothing"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceStaticTest)
