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

#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct ScopeGuardTest: TestSuite::Tester {
    explicit ScopeGuardTest();

    void constructNoCreate();
    void constructMove();

    void pointer();
    void value();
    void lambda();
    void returningLambda();
    void noHandle();
    void release();
};

ScopeGuardTest::ScopeGuardTest() {
    addTests({&ScopeGuardTest::constructNoCreate,
              &ScopeGuardTest::constructMove,

              &ScopeGuardTest::pointer,
              &ScopeGuardTest::value,
              &ScopeGuardTest::lambda,
              &ScopeGuardTest::returningLambda,
              &ScopeGuardTest::noHandle,
              &ScopeGuardTest::release});
}

void ScopeGuardTest::constructNoCreate() {
    {
        ScopeGuard e{NoCreate};
    }

    /* Implicit construction from NoCreateT is not allowed, neither should be
       default construction (because such instance is too easy to create by
       accident but makes no sense, so prevent that) */
    CORRADE_VERIFY(!(std::is_convertible<NoCreateT, ScopeGuard>::value));
    CORRADE_VERIFY(!std::is_default_constructible<ScopeGuard>::value);
}

void increment(int* value) { ++*value; }

void ScopeGuardTest::constructMove() {
    int v = 0;

    {
        ScopeGuard a{&v, increment};
        CORRADE_COMPARE(v, 0);

        ScopeGuard b = std::move(a);
        CORRADE_COMPARE(v, 0);

        ScopeGuard c{NoCreate};
        CORRADE_COMPARE(v, 0);

        c = std::move(a);
        CORRADE_COMPARE(v, 0);
    }

    /* The deleter should be only called once */
    CORRADE_COMPARE(v, 1);
}

int fd;
void close(float* value) { *value = 3.14f; }
int closeInt(int) { fd = 42; return 5; }

void ScopeGuardTest::pointer() {
    float v = 0.0f;
    {
        ScopeGuard e{&v, close};
    }
    CORRADE_COMPARE(v, 3.14f);
}

void ScopeGuardTest::value() {
    {
        fd = 1337;
        ScopeGuard e{fd, closeInt};
    }
    CORRADE_COMPARE(fd, 42);
}

void ScopeGuardTest::lambda() {
    {
        fd = 0;
        ScopeGuard{&fd, [](int* handle) {
            *handle = 7;
        }};
    }
    CORRADE_COMPARE(fd, 7);
}

void ScopeGuardTest::returningLambda() {
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    CORRADE_SKIP("Lambdas with non-void return type are not supported on MSVC2015 due to a compiler limitation.");
    #else
    {
        fd = 0;
        ScopeGuard{&fd, [](int* handle) {
            *handle = 7;
            return true;
        }};
    }
    CORRADE_COMPARE(fd, 7);
    #endif
}

int globalThingy;

void ScopeGuardTest::noHandle() {
    globalThingy = 42;
    {
        ScopeGuard e{[]() {
            globalThingy = 1337;
        }};
    }
    CORRADE_COMPARE(globalThingy, 1337);
}

void ScopeGuardTest::release() {
    float v = 1.234f;
    {
        ScopeGuard e{&v, close};
        e.release();
    }
    CORRADE_COMPARE(v, 1.234f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ScopeGuardTest)
