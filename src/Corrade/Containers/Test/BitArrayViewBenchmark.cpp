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

#include <bitset>

#include "Corrade/Cpu.h"
#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Containers/Test/BitArrayViewTest.h"
#include "Corrade/TestSuite/Tester.h"
#ifdef CORRADE_ENABLE_POPCNT
#include "Corrade/Utility/IntrinsicsAvx.h"
#endif
#include "Corrade/Utility/Test/cpuVariantHelpers.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BitArrayViewBenchmark: TestSuite::Tester {
    explicit BitArrayViewBenchmark();

    void captureImplementations();
    void restoreImplementations();

    void countLessThan64();
    void countAligned64();
    void countUnaligned128();
    void countAligned128();
    void countUnaligned1024();
    void countNaive128();
    void countStlBitset1024();

    private:
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(Implementation::bitCountSet) bitCountSetImplementation;
        #endif
};

const struct {
    Cpu::Features features;
    std::size_t(*function)(const char*, std::size_t, std::size_t);
} CountData[]{
    {Cpu::Scalar, nullptr},
    /* The 64-bit variants of POPCNT and BMI1 instructions aren't exposed on
       32-bit systems, and no 32-bit fallback is implemented. See the source
       for details. */
    #if defined(CORRADE_ENABLE_POPCNT) && !defined(CORRADE_TARGET_32BIT)
    {Cpu::Popcnt, bitCountSetImplementationPopcnt},
    #endif
    #if defined(CORRADE_ENABLE_POPCNT) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Popcnt|Cpu::Bmi1, nullptr},
    #endif
};

BitArrayViewBenchmark::BitArrayViewBenchmark() {
    addInstancedBenchmarks({&BitArrayViewBenchmark::countLessThan64,
                            &BitArrayViewBenchmark::countAligned64,
                            &BitArrayViewBenchmark::countUnaligned128,
                            &BitArrayViewBenchmark::countAligned128,
                            &BitArrayViewBenchmark::countUnaligned1024}, 100,
        Utility::Test::cpuVariantCount(CountData),
        &BitArrayViewBenchmark::captureImplementations,
        &BitArrayViewBenchmark::restoreImplementations);

    addBenchmarks({&BitArrayViewBenchmark::countNaive128,
                   &BitArrayViewBenchmark::countStlBitset1024}, 100);
}

void BitArrayViewBenchmark::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    bitCountSetImplementation = Implementation::bitCountSet;
    #endif
}

void BitArrayViewBenchmark::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    Implementation::bitCountSet = bitCountSetImplementation;
    #endif
}

constexpr std::size_t CountRepeats = 100;

void BitArrayViewBenchmark::countLessThan64() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This uses the special-case branch that loads byte-by-byte and then
       performs a single popcnt */

    constexpr std::uint64_t bits[]{~0ull, ~0ull};
    Containers::BitArrayView view{reinterpret_cast<const char*>(bits) + 3, 5, 58};

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += view.count();
    }

    CORRADE_COMPARE(count, 58*CountRepeats);
}

void BitArrayViewBenchmark::countAligned64() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This *also* uses the special-case branch that loads byte-by-byte and
       then performs a single popcnt, due to this possibly needing masking off
       both initial and final bits */

    constexpr std::uint64_t bits[]{~0ull};
    Containers::BitArrayView view{bits};

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += view.count();
    }

    CORRADE_COMPARE(count, 64*CountRepeats);
}

void BitArrayViewBenchmark::countUnaligned128() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This uses two overlapping & masked 64-bit reads and two popcnt calls */

    constexpr std::uint64_t bits[]{~0ull, ~0ull};
    Containers::BitArrayView view{bits, 1, 126};

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += view.count();
    }

    CORRADE_COMPARE(count, 126*CountRepeats);
}

void BitArrayViewBenchmark::countAligned128() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This uses two non-overlapping masked 64-bit reads and two popcnt calls */

    constexpr std::uint64_t bits[]{~0ull, ~0ull};
    Containers::BitArrayView view{bits};

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += view.count();
    }

    CORRADE_COMPARE(count, 128*CountRepeats);
}

void BitArrayViewBenchmark::countUnaligned1024() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CountData[testCaseInstanceId()];
    Implementation::bitCountSet = data.function ? data.function : Implementation::bitCountSetImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(CountData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This uses two non-overlapping masked 64-bit reads, 14 unmasked reads
       and 16 popcnt calls */

    constexpr std::uint64_t bits[]{
        ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull,
        ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull, ~0ull
    };
    Containers::BitArrayView view = Containers::BitArrayView{bits}.exceptPrefix(1).exceptSuffix(1);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += view.count();
    }

    CORRADE_COMPARE(count, (16*64 - 2)*CountRepeats);
}

void BitArrayViewBenchmark::countNaive128() {
    constexpr std::uint64_t bits[]{~0ull, ~0ull};
    Containers::BitArrayView view{bits, 1, 126};

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        for(std::size_t i = 0; i != view.size(); ++i)
            if(view[i]) ++count;
    }

    CORRADE_COMPARE(count, 126*CountRepeats);
}

CORRADE_NEVER_INLINE std::size_t stlBitsetCount(const std::bitset<1024>& bits) {
    return bits.count();
}

void BitArrayViewBenchmark::countStlBitset1024() {
    std::bitset<1024> bits;
    bits.set(); /* Wow, STL. Why can't I just construct it like this?! */
    bits.reset(0);
    bits.reset(1023);

    /* Interestingly enough, when put into a deinlined function to match what
       BitArrayView::count() is doing, it's significantly slower than even the
       scalar variant (huh?!) */

    std::size_t count = 0;
    CORRADE_BENCHMARK(CountRepeats) {
        count += stlBitsetCount(bits);
    }

    CORRADE_COMPARE(count, 1022*CountRepeats);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BitArrayViewBenchmark)
