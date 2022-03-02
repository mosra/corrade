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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/System.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct SystemTest: TestSuite::Tester {
    explicit SystemTest();

    void isSandboxed();
    void sleep();
};

SystemTest::SystemTest() {
    addTests({&SystemTest::isSandboxed,
              &SystemTest::sleep});
}

void SystemTest::isSandboxed() {
    #if defined(CORRADE_TARGET_ANDROID) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_VERIFY(System::isSandboxed());
    #else
    CORRADE_VERIFY(!System::isSandboxed());
    #endif
}

void SystemTest::sleep() {
    System::sleep(5);

    /* Just test that it doesn't crash, can't test much else */
    CORRADE_VERIFY(true);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::SystemTest)
