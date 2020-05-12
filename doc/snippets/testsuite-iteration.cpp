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

#include <cctype>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/DebugStl.h>

using namespace Corrade;

struct NameTest: TestSuite::Tester {
    explicit NameTest();

    void noYellingAllowed();
};

NameTest::NameTest() {
    addTests({&NameTest::noYellingAllowed});
}

/** [0] */
void NameTest::noYellingAllowed() {
    for(std::string name: {"Lucy", "JOHN", "Ed"}) {
        CORRADE_ITERATION(name);
        for(std::size_t i = 1; i != name.size(); ++i) {
            CORRADE_ITERATION(i);
            CORRADE_VERIFY(!std::isupper(name[i]));
        }
    }
}
/** [0] */

CORRADE_TEST_MAIN(NameTest)
