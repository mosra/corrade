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

#include <atomic>
#include <thread>
#include <Corrade/TestSuite/Tester.h>

using namespace Corrade;

/** [0] */
struct RaceTest: TestSuite::Tester {
    explicit RaceTest();

    template<class T> void threadedIncrement();
};

RaceTest::RaceTest() {
    addRepeatedTests<RaceTest>({
        &RaceTest::threadedIncrement<int>,
        &RaceTest::threadedIncrement<std::atomic_int>}, 10000);
}

template<class T> void RaceTest::threadedIncrement() {
    setTestCaseTemplateName(std::is_same<T, int>::value ?
        "int" : "std::atomic_int");

    T x{0};
    int y = 1;
    auto fun = [&x, &y] {
        for(std::size_t i = 0; i != 500; ++i) x += y;
    };
    std::thread a{fun}, b{fun}, c{fun};

    a.join();
    b.join();
    c.join();

    CORRADE_COMPARE(x, 1500);
}
/** [0] */

CORRADE_TEST_MAIN(RaceTest)
