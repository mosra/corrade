/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

static void initializeStaticResource() {
    CORRADE_RESOURCE_INITIALIZE(ResourceTestData)
}

static void finalizeStaticResource() {
    CORRADE_RESOURCE_FINALIZE(ResourceTestData)
}

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ResourceStaticTest: TestSuite::Tester {
    explicit ResourceStaticTest();

    void test();
};

ResourceStaticTest::ResourceStaticTest() {
    addTests({&ResourceStaticTest::test});
}

void ResourceStaticTest::test() {
    CORRADE_VERIFY(!Resource::hasGroup("test"));

    initializeStaticResource();
    /* Initializing second time shouldn't cause any problems */
    initializeStaticResource();

    CORRADE_VERIFY(Resource::hasGroup("test"));
    Resource r("test");
    CORRADE_COMPARE_AS(r.get("predisposition.bin"),
        Directory::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE_AS(r.get("consequence.bin"),
        Directory::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);

    /* Finalizing should remove the group again */
    finalizeStaticResource();
    CORRADE_VERIFY(!Resource::hasGroup("test"));

    /* Finalizing second time shouldn't cause any problems */
    finalizeStaticResource();
    CORRADE_VERIFY(!Resource::hasGroup("test"));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceStaticTest)
