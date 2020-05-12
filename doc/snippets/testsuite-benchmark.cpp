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

#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/StlMath.h>

using namespace Corrade;

namespace {

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
/* Taken from https://en.wikipedia.org/wiki/Fast_inverse_square_root */
float fastinvsqrt(float number) {
    int i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number*0.5f;
    y = number;
    i = *reinterpret_cast<int*>(&y);
    i = 0x5f3759df - (i >> 1);
    y = *reinterpret_cast<float*>(&i);
    y = y*(threehalfs - (x2*y*y));
    return y;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

}

/** [0] */
struct InvSqrtBenchmark: TestSuite::Tester {
    explicit InvSqrtBenchmark();

    void naive();
    void fast();
};

InvSqrtBenchmark::InvSqrtBenchmark() {
    for(auto fn: {&InvSqrtBenchmark::naive, &InvSqrtBenchmark::fast}) {
        addBenchmarks({fn}, 500, BenchmarkType::WallTime);
        addBenchmarks({fn}, 500, BenchmarkType::CpuTime);
    }
}

void InvSqrtBenchmark::naive() {
    volatile float a; /* to avoid optimizers removing the benchmark code */
    CORRADE_BENCHMARK(1000000)
        a = 1.0f/std::sqrt(float(testCaseRepeatId()));
    CORRADE_VERIFY(a);
}

void InvSqrtBenchmark::fast() {
    volatile float a; /* to avoid optimizers removing the benchmark code */
    CORRADE_BENCHMARK(1000000)
        a = fastinvsqrt(float(testCaseRepeatId()));
    CORRADE_VERIFY(a);
}
/** [0] */

CORRADE_TEST_MAIN(InvSqrtBenchmark)
