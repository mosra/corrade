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

#include <vector>
#include <string>

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Test { namespace {

struct MainTest: TestSuite::Tester {
    explicit MainTest();

    void utf8output();
    void colors();
    void arguments();
};

MainTest::MainTest(): TestSuite::Tester{TesterConfiguration{}.setSkippedArgumentPrefixes({"arg"})} {
    addTests({&MainTest::utf8output,
              &MainTest::colors,
              &MainTest::arguments});
}

void MainTest::utf8output() {
    Debug{} << "The lines below should have the same length, one with diacritics:";
    Debug{} << "hýždě šňůra";
    Debug{} << "hyzde snura";

    CORRADE_VERIFY(true);
}

void MainTest::colors() {
    #ifdef CORRADE_TARGET_WINDOWS
    #ifndef CORRADE_UTILITY_USE_ANSI_COLORS
    Debug{} << "CORRADE_UTILITY_USE_ANSI_COLORS not set, using WinAPI instead";
    #else
    Debug{} << "CORRADE_UTILITY_USE_ANSI_COLORS set";
    #endif
    #endif
    Debug{} << "Visual check:" << Debug::boldColor(Debug::Color::Blue) << "this is blue!" << Debug::resetColor << "and this is a grey square:" << Debug::color << std::uint8_t(0x77);

    CORRADE_VERIFY(true);
}

void MainTest::arguments() {
    #ifdef CORRADE_TESTSUITE_TARGET_XCTEST
    CORRADE_SKIP("Command-line arguments are currently ignored under XCTest.");
    #endif
    Debug{} << "Arguments expected: {--arg-utf, hýždě, --arg-another, šňůra}";
    Debug{} << "Arguments passed:  " << Containers::arrayView(
        Tester::arguments().second, Tester::arguments().first).suffix(1);

    CORRADE_COMPARE_AS((std::vector<std::string>{
        Tester::arguments().second + 1, Tester::arguments().second + Tester::arguments().first}),
        (std::vector<std::string>{"--arg-utf", "hýždě", "--arg-another", "šňůra"
        }),
        TestSuite::Compare::Container);
}

}}}

CORRADE_TEST_MAIN(Corrade::Test::MainTest)
