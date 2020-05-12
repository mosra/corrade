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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Resource.h"

#include "GlobalStateAcrossLibrariesLibrary.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct GlobalStateAcrossLibrariesTest: TestSuite::Tester {
    explicit GlobalStateAcrossLibrariesTest();

    void debug();
    void resource();
};

GlobalStateAcrossLibrariesTest::GlobalStateAcrossLibrariesTest() {
    addTests({&GlobalStateAcrossLibrariesTest::debug,
              &GlobalStateAcrossLibrariesTest::resource});
}

void GlobalStateAcrossLibrariesTest::debug() {
    #if defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_BUILD_STATIC)
    CORRADE_VERIFY(!"CORRADE_BUILD_STATIC_UNIQUE_GLOBALS enabled but CORRADE_BUILD_STATIC not");
    #endif

    std::ostringstream out;

    std::ostream* current = Debug::output();
    CORRADE_COMPARE(debugOutputFromALibrary(), current);

    {
        Debug redirectOutput{&out};
        #ifndef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
        CORRADE_EXPECT_FAIL("CORRADE_BUILD_STATIC_UNIQUE_GLOBALS not enabled.");
        #endif
        CORRADE_COMPARE(debugOutputFromALibrary(), &out);
    }

    CORRADE_COMPARE(debugOutputFromALibrary(), current);
}

void GlobalStateAcrossLibrariesTest::resource() {
    #if defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_BUILD_STATIC)
    CORRADE_VERIFY(!"CORRADE_BUILD_STATIC_UNIQUE_GLOBALS enabled but CORRADE_BUILD_STATIC not");
    #endif

    /* The resource is complied into the the library, but the executable should
       see it too */
    CORRADE_VERIFY(libraryHasATestResourceGroup());

    #ifndef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
    CORRADE_EXPECT_FAIL("CORRADE_BUILD_STATIC_UNIQUE_GLOBALS not enabled.");
    #endif
    CORRADE_VERIFY(Utility::Resource::hasGroup("test"));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::GlobalStateAcrossLibrariesTest)
