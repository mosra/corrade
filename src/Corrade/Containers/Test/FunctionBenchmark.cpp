/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include <functional>

#include "Corrade/Containers/Function.h"
#ifdef CORRADE_MSVC2015_COMPATIBILITY
#include "Corrade/Containers/StaticArray.h"
#endif
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct FunctionBenchmark: TestSuite::Tester {
    explicit FunctionBenchmark();

    void baseline();
    void baselineInline();
    void baselineTemplate();
    void baselineTemplateInline();

    void functionPointer();
    void functionPointerInline();
    void functionMemberPointer();
    void functionMemberPointerInline();
    void functionLambda();
    void functionStatefulLambda();
    void functionLargeStatefulLambda();

    void stlFunctionPointer();
    void stlFunctionPointerInline();
    void stlFunctionMemberPointer();
    void stlFunctionMemberPointerInline();
    void stlFunctionLambda();
    void stlFunctionStatefulLambda();
    void stlFunctionLargeStatefulLambda();
};

FunctionBenchmark::FunctionBenchmark() {
    addBenchmarks({&FunctionBenchmark::baseline,
                   &FunctionBenchmark::baselineInline,
                   &FunctionBenchmark::baselineTemplate,
                   &FunctionBenchmark::baselineTemplateInline,

                   &FunctionBenchmark::functionPointer,
                   &FunctionBenchmark::functionPointerInline,
                   &FunctionBenchmark::functionMemberPointer,
                   &FunctionBenchmark::functionMemberPointerInline,
                   &FunctionBenchmark::functionLambda,
                   &FunctionBenchmark::functionStatefulLambda,
                   &FunctionBenchmark::functionLargeStatefulLambda,

                   &FunctionBenchmark::stlFunctionPointer,
                   &FunctionBenchmark::stlFunctionPointerInline,
                   &FunctionBenchmark::stlFunctionMemberPointer,
                   &FunctionBenchmark::stlFunctionMemberPointerInline,
                   &FunctionBenchmark::stlFunctionLambda,
                   &FunctionBenchmark::stlFunctionStatefulLambda,
                   &FunctionBenchmark::stlFunctionLargeStatefulLambda}, 100);
}

constexpr int Repeats = 100000;

/* These are all a single instruction but each time a different instruction to
   prevent the compiler from "optimizing" by deduplicating them (and making the
   inline variant the same as non-inline, etc.). Also it's never just ++a as
   the compiler may combine that with the benchmark loop in that case, skewing
   the numbers. */

CORRADE_NEVER_INLINE void increment2(int& a) {
    a += 2;
}

CORRADE_ALWAYS_INLINE void increment3Inline(int& a) {
    a += 3;
}

CORRADE_NEVER_INLINE void call(int& a, void(*function)(int&)) {
    function(a);
}

template<void(*function)(int&)> CORRADE_NEVER_INLINE void call(int& a) {
    function(a);
}

CORRADE_NEVER_INLINE void call(int& a, Function<void(int&)>& function) {
    function(a);
}

CORRADE_NEVER_INLINE void call(int& a, std::function<void(int&)>& function) {
    function(a);
}

void FunctionBenchmark::baseline() {
    int a = 0;
    CORRADE_BENCHMARK(Repeats)
        call(a, increment2);
    CORRADE_COMPARE(a, Repeats*2);
}

void FunctionBenchmark::baselineInline() {
    int a = 0;
    CORRADE_BENCHMARK(Repeats)
        call(a, increment3Inline);
    CORRADE_COMPARE(a, Repeats*3);
}

void FunctionBenchmark::baselineTemplate() {
    int a = 0;
    CORRADE_BENCHMARK(Repeats)
        call<increment2>(a);
    CORRADE_COMPARE(a, Repeats*2);
}

void FunctionBenchmark::baselineTemplateInline() {
    int a = 0;
    CORRADE_BENCHMARK(Repeats)
        call<increment3Inline>(a);
    CORRADE_COMPARE(a, Repeats*3);
}

void FunctionBenchmark::functionPointer() {
    int a = 0;
    Function<void(int&)> f = increment2;
    CORRADE_VERIFY(!f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*2);
}

void FunctionBenchmark::functionPointerInline() {
    int a = 0;
    Function<void(int&)> f = increment3Inline;
    CORRADE_VERIFY(!f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*3);
}

struct Incrementor {
    int a = 0;
    CORRADE_NEVER_INLINE void increment4() { a += 4; }
    CORRADE_ALWAYS_INLINE void increment5Inline() { a += 5; }
};

CORRADE_NEVER_INLINE void call(Function<void()>& function) {
    function();
}

CORRADE_NEVER_INLINE void call(Incrementor& incrementor, std::function<void(Incrementor&)>& function) {
    function(incrementor);
}

void FunctionBenchmark::functionMemberPointer() {
    Incrementor incrementor;
    Function<void()> f{incrementor, &Incrementor::increment4};
    CORRADE_VERIFY(!f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(incrementor.a, Repeats*4);
}

void FunctionBenchmark::functionMemberPointerInline() {
    Incrementor incrementor;
    Function<void()> f{incrementor, &Incrementor::increment5Inline};
    CORRADE_VERIFY(!f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(incrementor.a, Repeats*5);
}

void FunctionBenchmark::functionLambda() {
    int a = 0;
    Function<void(int&)> f = [](int& a) { a += 6; };
    CORRADE_VERIFY(!f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*6);
}

void FunctionBenchmark::functionStatefulLambda() {
    int a = 0;
    Function<void()> f = [&a]{ a += 7; };
    /* All lambdas are non-trivially-copyable on MSVC 2015 and 2017. Not using
       CORRADE_EXPECT_FAIL() as it'd print a message for all 100 iterations. */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    CORRADE_VERIFY(!f.isAllocated());
    #else
    CORRADE_VERIFY(f.isAllocated());
    #endif

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(a, Repeats*7);
}

void FunctionBenchmark::functionLargeStatefulLambda() {
    int a = 0, b = 0;
    /* MSVC 2015 cannot capture arrays by value, let's capture a wrapper struct
       instead */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    Containers::StaticArray<5, int*> ptrs
    #else
    int* ptrs[5]
    #endif
        {&b, &a, &b, nullptr, &a};
    /* Up to 3 pointers on 64bit and up to 4 on 32bit can fit inline, 5
       pointers will allocate */
    Function<void()> f = [ptrs]{ *ptrs[1] += 8; };
    CORRADE_VERIFY(f.isAllocated());

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(a, Repeats*8);
}

void FunctionBenchmark::stlFunctionPointer() {
    int a = 0;
    std::function<void(int&)> f = increment2;

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*2);
}

void FunctionBenchmark::stlFunctionPointerInline() {
    int a = 0;
    std::function<void(int&)> f = increment3Inline;

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*3);
}

void FunctionBenchmark::stlFunctionMemberPointer() {
    Incrementor incrementor;
    std::function<void(Incrementor&)> f = &Incrementor::increment4;

    CORRADE_BENCHMARK(Repeats)
        call(incrementor, f);
    CORRADE_COMPARE(incrementor.a, Repeats*4);
}

void FunctionBenchmark::stlFunctionMemberPointerInline() {
    Incrementor incrementor;
    std::function<void(Incrementor&)> f = &Incrementor::increment5Inline;

    CORRADE_BENCHMARK(Repeats)
        call(incrementor, f);
    CORRADE_COMPARE(incrementor.a, Repeats*5);
}

void FunctionBenchmark::stlFunctionLambda() {
    int a = 0;
    std::function<void(int&)> f = [](int& a) { a += 9; };

    CORRADE_BENCHMARK(Repeats)
        call(a, f);
    CORRADE_COMPARE(a, Repeats*9);
}

CORRADE_NEVER_INLINE void call(std::function<void()>& function) {
    function();
}

void FunctionBenchmark::stlFunctionStatefulLambda() {
    int a = 0;
    std::function<void()> f = [&a]{ a += 10; };

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(a, Repeats*10);
}

void FunctionBenchmark::stlFunctionLargeStatefulLambda() {
    int a = 0, b = 0;
    /* MSVC 2015 cannot capture arrays by value, let's capture a wrapper struct
       instead */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    Containers::StaticArray<5, int*> ptrs
    #else
    int* ptrs[5]
    #endif
        {&b, &a, &b, nullptr, &a};
    /* On libstdc++ the amount of captured data is 2 pointers, so 5 pointers
       should definitely cause it to allocate. Making it the same size as in
       functionLargeStatefulLambda() to not skew benchmark numbers because of
       that. */
    std::function<void()> f = [ptrs]{ *ptrs[1] += 11; };

    CORRADE_BENCHMARK(Repeats)
        call(f);
    CORRADE_COMPARE(a, Repeats*11);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::FunctionBenchmark)
