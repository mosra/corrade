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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StringView.h"
/* The hpp is needed by a static build with CORRADE_BUILD_DEPRECATED disabled,
   otherwise it gets pulled in by Manager.h for backwards compat  */
#include "Corrade/PluginManager/Manager.hpp"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"

#include "AbstractAnimal.h"

namespace Corrade { namespace PluginManager { namespace Test {

/* Defined in two separate dynamic libraries */
CORRADE_VISIBILITY_IMPORT int initialize1();
CORRADE_VISIBILITY_IMPORT int initialize2();

namespace {

struct DuplicateStaticPluginTest: TestSuite::Tester {
    explicit DuplicateStaticPluginTest();

    void test();
};

DuplicateStaticPluginTest::DuplicateStaticPluginTest() {
    addTests({&DuplicateStaticPluginTest::test});
}

void DuplicateStaticPluginTest::test() {
    #if defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_BUILD_STATIC)
    CORRADE_FAIL("CORRADE_BUILD_STATIC_UNIQUE_GLOBALS enabled but CORRADE_BUILD_STATIC not");
    #endif

    CORRADE_COMPARE(initialize1(), 42);
    CORRADE_COMPARE(initialize2(), 1337);

    Containers::String out;
    Warning redirectWarning{&out};
    PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};

    #ifndef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
    CORRADE_EXPECT_FAIL("CORRADE_BUILD_STATIC_UNIQUE_GLOBALS not enabled.");
    #endif
    CORRADE_COMPARE_AS(manager.pluginList(),
        Containers::arrayView<Containers::StringView>({"Canary", "Dird"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE(out, "PluginManager::Manager: duplicate static plugin Canary, ignoring\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::DuplicateStaticPluginTest)
