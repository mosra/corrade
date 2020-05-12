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
#include <Corrade/TestSuite/Tester.h>

using namespace Corrade;

/** [0] */
struct VectorBenchmark: TestSuite::Tester {
    explicit VectorBenchmark();

    void insert();

    void copyCountBegin();
    std::uint64_t copyCountEnd();
};

namespace {
    std::uint64_t count = 0;

    struct CopyCounter {
        CopyCounter() = default;
        CopyCounter(const CopyCounter&) {
            ++count;
        }
    };

    enum: std::size_t { InsertDataCount = 3 };

    constexpr const struct {
        const char* name;
        std::size_t count;
    } InsertData[InsertDataCount]{
        {"100", 100},
        {"1k", 1000},
        {"10k", 10000}
    };
}

VectorBenchmark::VectorBenchmark() {
    addCustomInstancedBenchmarks({&VectorBenchmark::insert}, 1, InsertDataCount,
        &VectorBenchmark::copyCountBegin,
        &VectorBenchmark::copyCountEnd,
        BenchmarkUnits::Count);
}

void VectorBenchmark::insert() {
    auto&& data = InsertData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::vector<CopyCounter> v;
    CORRADE_BENCHMARK(1)
        for(std::size_t i = 0; i != data.count; ++i)
            v.push_back({});
}

void VectorBenchmark::copyCountBegin() {
    setBenchmarkName("copy count");
    count = 0;
}

std::uint64_t VectorBenchmark::copyCountEnd() {
    return count;
}
/** [0] */

CORRADE_TEST_MAIN(VectorBenchmark)
