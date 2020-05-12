/*
    This file is part of Corrade.

    Original authors — credit is appreciated but not required:

        2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
        2017, 2018, 2019, 2020 — Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cmath>
#include <list>
#include <vector>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/Endianness.h>

namespace Corrade { namespace Examples {

namespace {

struct MyTest: TestSuite::Tester {
    explicit MyTest();

    void commutativity();
    void associativity();
    void pi();
    void sin();
    void bigEndian();

    void prepend1kItemsVector();
    void prepend1kItemsList();
};

MyTest::MyTest() {
    addTests({&MyTest::commutativity,
              &MyTest::associativity,
              &MyTest::sin,
              &MyTest::pi,
              &MyTest::bigEndian});

    addBenchmarks({&MyTest::prepend1kItemsVector,
                   &MyTest::prepend1kItemsList}, 100);
}

void MyTest::commutativity() {
    double a = 5.0;
    double b = 3.0;

    CORRADE_VERIFY(a*b == b*a);
    CORRADE_VERIFY(a/b == b/a);
}

void MyTest::associativity() {
    CORRADE_COMPARE(2*(3 + 4), 14);
    CORRADE_COMPARE((2*3) + 4, 14);
}

void MyTest::sin() {
    CORRADE_COMPARE_AS(std::sin(0), 0.0f, float);
}

void MyTest::pi() {
    CORRADE_EXPECT_FAIL("Need better approximation.");
    double pi = 22/7.0;
    CORRADE_COMPARE(pi, 3.14159265);
}

void MyTest::bigEndian() {
    if(!Utility::Endianness::isBigEndian())
        CORRADE_SKIP("Need big-endian machine for this.");

    union {
        short a = 64;
        char data[2];
    } a;
    CORRADE_COMPARE(a.data[0], 0);
    CORRADE_COMPARE(a.data[1], 64);
}

void MyTest::prepend1kItemsVector() {
    double a{};
    CORRADE_BENCHMARK(100) {
        std::vector<double> container;
        for(std::size_t i = 0; i != 1000; ++i)
            container.insert(container.begin(), 1.0);
        a += container.back();
    }
    CORRADE_VERIFY(a); // to avoid the benchmark loop being optimized out
}

void MyTest::prepend1kItemsList() {
    double a{};
    CORRADE_BENCHMARK(100) {
        std::list<double> container;
        for(std::size_t i = 0; i != 1000; ++i)
            container.push_front(1.0);
        a += container.back();
    }
    CORRADE_VERIFY(a); // to avoid the benchmark loop being optimized out
}

}

}}

CORRADE_TEST_MAIN(Corrade::Examples::MyTest)
