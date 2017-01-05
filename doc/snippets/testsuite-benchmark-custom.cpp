/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <Corrade/TestSuite/Tester.h>

using namespace Corrade;

/** [0] */
struct VectorBenchmark: TestSuite::Tester {
    explicit VectorBenchmark();

    void copyCountInsert10k();
    void copyCountInsert10kBegin();
    std::uint64_t copyCountInsert10kEnd();
};

VectorBenchmark::VectorBenchmark() {
    addCustomBenchmarks({&VectorBenchmark::copyCountInsert10k}, 1,
        &VectorBenchmark::copyCountInsert10kBegin,
        &VectorBenchmark::copyCountInsert10kEnd,
        BenchmarkUnits::Count);
}

namespace {
    std::uint64_t count = 0;

    struct CopyCounter {
        CopyCounter() = default;
        CopyCounter(const CopyCounter&) {
            ++count;
        }
    };
}

void VectorBenchmark::copyCountInsert10k() {
    std::vector<CopyCounter> data;
    CORRADE_BENCHMARK(1)
        for(std::size_t i = 0; i != 10000; ++i)
            data.push_back({});
}

void VectorBenchmark::copyCountInsert10kBegin() {
    count = 0;
}

std::uint64_t VectorBenchmark::copyCountInsert10kEnd() {
    return count;
}

CORRADE_TEST_MAIN(VectorBenchmark)
/** [0] */

