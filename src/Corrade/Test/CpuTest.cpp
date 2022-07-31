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

#include <sstream>

#include "Corrade/Cpu.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

#ifdef CORRADE_ENABLE_SSE2
#include "Corrade/Utility/IntrinsicsSse2.h"
#endif
#ifdef CORRADE_ENABLE_SSE3
#include "Corrade/Utility/IntrinsicsSse3.h"
#endif
#ifdef CORRADE_ENABLE_SSSE3
#include "Corrade/Utility/IntrinsicsSsse3.h"
#endif
#if defined(CORRADE_ENABLE_SSE41) || defined(CORRADE_ENABLE_SSE42)
#include "Corrade/Utility/IntrinsicsSse4.h"
#endif
#if defined(CORRADE_ENABLE_AVX) || defined(CORRADE_ENABLE_AVX_F16C) || defined(CORRADE_ENABLE_AVX_FMA) || defined(CORRADE_ENABLE_AVX2) || defined(CORRADE_ENABLE_AVX512F)
#include "Corrade/Utility/IntrinsicsAvx.h"
#endif
#ifdef CORRADE_ENABLE_NEON
#include <arm_neon.h>
#endif
#ifdef CORRADE_ENABLE_SIMD128
#include <wasm_simd128.h>
#endif

#include "CpuTestExternalLibrary.h"

namespace Corrade { namespace Test { namespace {

struct CpuTest: TestSuite::Tester {
    explicit CpuTest();

    void tagNoDefaultConstructor();
    void tagInlineDefinition();
    void tagConstructTemplate();

    void typeTraits();

    /* Most of the operator tests is inherited from EnumSetTest, just replacing
       Feature::Fast with Cpu::Sse2, Feature::Cheap with Cpu::Sse3,
       Feature::Tested with Cpu::Ssse3 and Feature::Popular with Cpu::Sse41,
       and adjusting numeric values for inverse because there's no fullValue
       specified */

    void featuresConstructScalar();
    void featuresConstruct();
    void featuresConstructTemplate();
    void featuresOperatorOr();
    void featuresOperatorAnd();
    void featuresOperatorXor();
    void featuresOperatorBoolScalar();
    void featuresOperatorBool();
    void featuresOperatorInverse();
    void featuresCompare();

    void detectDefault();
    void detect();

    void bitIndex();
    void bitCount();
    void priority();

    void tagDispatch();
    void tagDispatchExtraExact();
    void tagDispatchExtraUnusedExtra();
    void tagDispatchExtraFallbackExtra();
    void tagDispatchExtraFallbackBase();
    void tagDispatchExtraFallbackBoth();
    void tagDispatchExtraPriority();

    void tagDispatchRuntime();
    void tagDispatchRuntimeExtra();
    void tagDispatchRuntimeExtraCombination();
    void tagDispatchRuntimeExtraZeroExtra();

    void tagDispatchedCompileTime();
    void tagDispatchedPointer();
    void tagDispatchedIfunc();

    void benchmarkTagDispatchedCompileTime();
    void benchmarkTagDispatchedPointer();
    void benchmarkTagDispatchedIfunc();
    void benchmarkTagDispatchedExternalLibraryCompileTime();
    void benchmarkTagDispatchedExternalLibraryPointer();
    void benchmarkTagDispatchedExternalLibraryIfunc();
    void benchmarkTagDispatchedExternalLibraryEveryCall();

    template<class T> void enableMacros();

    void enableMacrosMultiple();
    void enableMacrosMultipleAllEmpty();

    void enableMacrosLambda();
    void enableMacrosLambdaMultiple();

    void debug();
    void debugPacked();
};

const struct {
    const char* name;
    Cpu::Features(*function)();
} DetectData[]{
    {"compiled", Cpu::compiledFeatures},
    {"runtime", Cpu::runtimeFeatures}
};

CpuTest::CpuTest() {
    addTests({&CpuTest::tagNoDefaultConstructor,
              &CpuTest::tagInlineDefinition,
              &CpuTest::tagConstructTemplate,

              &CpuTest::typeTraits,

              &CpuTest::featuresConstructScalar,
              &CpuTest::featuresConstruct,
              &CpuTest::featuresConstructTemplate,
              &CpuTest::featuresOperatorOr,
              &CpuTest::featuresOperatorAnd,
              &CpuTest::featuresOperatorXor,
              &CpuTest::featuresOperatorBoolScalar,
              &CpuTest::featuresOperatorBool,
              &CpuTest::featuresOperatorInverse,
              &CpuTest::featuresCompare,

              &CpuTest::detectDefault});

    addInstancedTests({&CpuTest::detect},
        Containers::arraySize(DetectData));

    addTests({&CpuTest::bitIndex,
              &CpuTest::bitCount,
              &CpuTest::priority,

              &CpuTest::tagDispatch,
              &CpuTest::tagDispatchExtraExact,
              &CpuTest::tagDispatchExtraUnusedExtra,
              &CpuTest::tagDispatchExtraFallbackBase,
              &CpuTest::tagDispatchExtraFallbackExtra,
              &CpuTest::tagDispatchExtraFallbackBoth,
              &CpuTest::tagDispatchExtraPriority,

              &CpuTest::tagDispatchRuntime,
              &CpuTest::tagDispatchRuntimeExtra,
              &CpuTest::tagDispatchRuntimeExtraCombination,
              &CpuTest::tagDispatchRuntimeExtraZeroExtra,

              &CpuTest::tagDispatchedCompileTime,
              &CpuTest::tagDispatchedPointer,
              &CpuTest::tagDispatchedIfunc});

    addBenchmarks({&CpuTest::benchmarkTagDispatchedCompileTime,
                   &CpuTest::benchmarkTagDispatchedPointer,
                   &CpuTest::benchmarkTagDispatchedIfunc,
                   &CpuTest::benchmarkTagDispatchedExternalLibraryCompileTime,
                   &CpuTest::benchmarkTagDispatchedExternalLibraryPointer,
                   &CpuTest::benchmarkTagDispatchedExternalLibraryIfunc,
                   &CpuTest::benchmarkTagDispatchedExternalLibraryEveryCall}, 100);

    addTests({
              #ifdef CORRADE_TARGET_X86
              &CpuTest::enableMacros<Cpu::Sse2T>,
              &CpuTest::enableMacros<Cpu::Sse3T>,
              &CpuTest::enableMacros<Cpu::Ssse3T>,
              &CpuTest::enableMacros<Cpu::Sse41T>,
              &CpuTest::enableMacros<Cpu::Sse42T>,
              &CpuTest::enableMacros<Cpu::PopcntT>,
              &CpuTest::enableMacros<Cpu::LzcntT>,
              &CpuTest::enableMacros<Cpu::Bmi1T>,
              &CpuTest::enableMacros<Cpu::AvxT>,
              &CpuTest::enableMacros<Cpu::AvxF16cT>,
              &CpuTest::enableMacros<Cpu::AvxFmaT>,
              &CpuTest::enableMacros<Cpu::Avx2T>,
              &CpuTest::enableMacros<Cpu::Avx512fT>,
              #elif defined(CORRADE_TARGET_ARM)
              &CpuTest::enableMacros<Cpu::NeonT>,
              &CpuTest::enableMacros<Cpu::NeonFmaT>,
              &CpuTest::enableMacros<Cpu::NeonFp16T>,
              #elif defined(CORRADE_TARGET_WASM)
              &CpuTest::enableMacros<Cpu::Simd128T>,
              #endif

              &CpuTest::enableMacrosMultiple,
              &CpuTest::enableMacrosMultipleAllEmpty,

              &CpuTest::enableMacrosLambda,
              &CpuTest::enableMacrosLambdaMultiple,

              &CpuTest::debug,
              &CpuTest::debugPacked});
}

using namespace Containers::Literals;

void CpuTest::tagNoDefaultConstructor() {
    /* Isn't default constructible to prevent ambiguity when calling
       foo({}) if both foo(TagT) and foo(whatever) is available */
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::ScalarT>::value);
    #ifdef CORRADE_TARGET_X86
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Sse2T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Sse3T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Ssse3T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Sse41T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Sse42T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::AvxT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::AvxF16cT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::AvxFmaT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Avx2T>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Avx512fT>::value);
    #elif defined(CORRADE_TARGET_ARM)
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::NeonT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::NeonFmaT>::value);
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::NeonFp16T>::value);
    #elif defined(CORRADE_TARGET_WASM)
    CORRADE_VERIFY(!std::is_default_constructible<Cpu::Simd128T>::value);
    #endif
}

void CpuTest::tagInlineDefinition() {
    /* Just a sanity check that the types match */
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Scalar), const Cpu::ScalarT>::value);
    #ifdef CORRADE_TARGET_X86
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Sse2), const Cpu::Sse2T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Sse3), const Cpu::Sse3T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Ssse3), const Cpu::Ssse3T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Sse41), const Cpu::Sse41T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Sse42), const Cpu::Sse42T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Avx), const Cpu::AvxT>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::AvxF16c), const Cpu::AvxF16cT>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::AvxFma), const Cpu::AvxFmaT>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Avx2), const Cpu::Avx2T>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Avx512f), const Cpu::Avx512fT>::value);
    #elif defined(CORRADE_TARGET_ARM)
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Neon), const Cpu::NeonT>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::NeonFma), const Cpu::NeonFmaT>::value);
    CORRADE_VERIFY(std::is_same<decltype(Cpu::NeonFp16), const Cpu::NeonFp16T>::value);
    #elif defined(CORRADE_TARGET_WASM)
    CORRADE_VERIFY(std::is_same<decltype(Cpu::Simd128), const Cpu::Simd128T>::value);
    #endif
}

void CpuTest::tagConstructTemplate() {
    #ifdef CORRADE_TARGET_X86
    auto tag = Cpu::tag<Cpu::Sse3T>();
    constexpr auto cTag = Cpu::tag<Cpu::Sse3T>();
    CORRADE_VERIFY(std::is_same<decltype(tag), Cpu::Sse3T>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTag), const Cpu::Sse3T>::value);
    #elif defined(CORRADE_TARGET_ARM)
    auto tag = Cpu::tag<Cpu::NeonT>();
    constexpr auto cTag = Cpu::tag<Cpu::NeonT>();
    CORRADE_VERIFY(std::is_same<decltype(tag), Cpu::NeonT>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTag), const Cpu::NeonT>::value);
    #elif defined(CORRADE_TARGET_WASM)
    auto tag = Cpu::tag<Cpu::Simd128T>();
    constexpr auto cTag = Cpu::tag<Cpu::Simd128T>();
    CORRADE_VERIFY(std::is_same<decltype(tag), Cpu::Simd128T>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTag), const Cpu::Simd128T>::value);
    #else
    CORRADE_SKIP("No Cpu tags available on this platform");
    #endif
}

void CpuTest::typeTraits() {
    CORRADE_VERIFY(!Cpu::TypeTraits<Cpu::ScalarT>::Index);
    #ifdef CORRADE_TARGET_X86
    CORRADE_VERIFY(Cpu::TypeTraits<Cpu::Avx2T>::Index);
    CORRADE_COMPARE(Cpu::TypeTraits<Cpu::Avx2T>::name(), "Avx2"_s);
    #elif defined(CORRADE_TARGET_ARM)
    CORRADE_VERIFY(Cpu::TypeTraits<Cpu::NeonFp16T>::Index);
    CORRADE_COMPARE(Cpu::TypeTraits<Cpu::NeonFp16T>::name(), "NeonFp16"_s);
    #elif defined(CORRADE_TARGET_WASM)
    CORRADE_VERIFY(Cpu::TypeTraits<Cpu::Simd128T>::Index);
    CORRADE_COMPARE(Cpu::TypeTraits<Cpu::Simd128T>::name(), "Simd128"_s);
    #else
    CORRADE_SKIP("No Cpu tags available on this platform");
    #endif
}

void CpuTest::featuresConstructScalar() {
    Cpu::Features noFeatures1;
    Cpu::Features noFeatures2 = Cpu::Scalar;
    constexpr Cpu::Features cNoFeatures1;
    constexpr Cpu::Features cNoFeatures2 = Cpu::Scalar;
    CORRADE_COMPARE(std::uint32_t(noFeatures1), 0);
    CORRADE_COMPARE(std::uint32_t(noFeatures2), 0);
    CORRADE_COMPARE(std::uint32_t(cNoFeatures1), 0);
    CORRADE_COMPARE(std::uint32_t(cNoFeatures2), 0);

    CORRADE_VERIFY(std::is_nothrow_constructible<Cpu::Features>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Cpu::Features, Cpu::ScalarT>::value);
}

void CpuTest::featuresConstruct() {
    #ifdef CORRADE_TARGET_X86
    Cpu::Features features = Cpu::Sse3;
    constexpr Cpu::Features cFeatures = Cpu::Sse3;
    CORRADE_COMPARE(std::uint32_t(features), 2);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 2);
    CORRADE_VERIFY(std::is_nothrow_constructible<Cpu::Features, Cpu::Sse3T>::value);
    #elif defined(CORRADE_TARGET_ARM)
    Cpu::Features features = Cpu::Neon;
    constexpr Cpu::Features cFeatures = Cpu::Neon;
    CORRADE_COMPARE(std::uint32_t(features), 1);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 1);
    CORRADE_VERIFY(std::is_nothrow_constructible<Cpu::Features, Cpu::NeonT>::value);
    #elif defined(CORRADE_TARGET_WASM)
    Cpu::Features features = Cpu::Simd128;
    constexpr Cpu::Features cFeatures = Cpu::Simd128;
    CORRADE_COMPARE(std::uint32_t(features), 1);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 1);
    CORRADE_VERIFY(std::is_nothrow_constructible<Cpu::Features, Cpu::Simd128T>::value);
    #else
    CORRADE_SKIP("No Cpu tags available on this platform");
    #endif
}

void CpuTest::featuresConstructTemplate() {
    #ifdef CORRADE_TARGET_X86
    auto features = Cpu::features<Cpu::Sse3T>();
    constexpr auto cFeatures = Cpu::features<Cpu::Sse3T>();
    CORRADE_COMPARE(std::uint32_t(features), 2);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 2);
    #elif defined(CORRADE_TARGET_ARM)
    auto features = Cpu::features<Cpu::NeonT>();
    constexpr auto cFeatures = Cpu::features<Cpu::NeonT>();
    CORRADE_COMPARE(std::uint32_t(features), 1);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 1);
    #elif defined(CORRADE_TARGET_WASM)
    auto features = Cpu::features<Cpu::Simd128T>();
    constexpr auto cFeatures = Cpu::features<Cpu::Simd128T>();
    CORRADE_COMPARE(std::uint32_t(features), 1);
    CORRADE_COMPARE(std::uint32_t(cFeatures), 1);
    #else
    CORRADE_SKIP("No Cpu tags available on this platform");
    #endif
}

void CpuTest::featuresOperatorOr() {
    #ifdef CORRADE_TARGET_X86
    /* This is actually using the compile-time operation, producing Tags<bits>,
       which is tested explicitly below, but that should be completely
       transparent to the user and work as if it produced Features directly
       instead of going through Tags first */
    Cpu::Features features = Cpu::Sse3|Cpu::Sse2;
    CORRADE_COMPARE(std::uint32_t(features), 3);

    CORRADE_COMPARE(std::uint32_t(features|Cpu::Ssse3), 7);
    CORRADE_COMPARE(std::uint32_t(Cpu::Ssse3|features), 7);

    features |= Cpu::Ssse3;
    CORRADE_COMPARE(std::uint32_t(features), 7);

    constexpr Cpu::Features cFeatures = Cpu::Sse3|Cpu::Sse2;
    constexpr Cpu::Features cFeatures1 = cFeatures|Cpu::Ssse3;
    constexpr Cpu::Features cFeatures2 = Cpu::Ssse3|cFeatures;
    CORRADE_COMPARE(std::uint32_t(cFeatures), 3);
    CORRADE_COMPARE(std::uint32_t(cFeatures1), 7);
    CORRADE_COMPARE(std::uint32_t(cFeatures2), 7);

    /* Test also the compile-time operation, different values should be
       different types but same values should be same types */
    constexpr auto cTags = Cpu::Sse3|Cpu::Sse2;
    constexpr auto cTags1 = cTags|Cpu::Ssse3;
    constexpr auto cTags2 = Cpu::Ssse3|cTags;
    CORRADE_COMPARE(std::uint32_t(cTags), 3);
    CORRADE_COMPARE(std::uint32_t(cTags1), 7);
    CORRADE_COMPARE(std::uint32_t(cTags2), 7);
    CORRADE_VERIFY(!std::is_same<decltype(cTags), decltype(cTags1)>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTags1), decltype(cTags2)>::value);

    /* And also with Tags<> on both sides, check that it doesn't decay to an
       int or other horrible thing */
    constexpr auto cTags3 = Cpu::Ssse3|Cpu::Sse41;
    constexpr auto cTags4 = cTags3|cTags;
    CORRADE_COMPARE(std::uint32_t(cTags4), 15);
    CORRADE_VERIFY(std::is_same<decltype(cTags4), const Cpu::Implementation::Tags<15>>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorAnd() {
    #ifdef CORRADE_TARGET_X86
    /* This is actually using the compile-time operation, producing Tags<bits>,
       which is tested explicitly below, but that should be completely
       transparent to the user and work as if it produced Features directly
       instead of going through Tags first */
    CORRADE_COMPARE(std::uint32_t(Cpu::Sse3 & Cpu::Sse2), 0);

    Cpu::Features features = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    CORRADE_COMPARE(std::uint32_t(features & Cpu::Sse41), 8);
    CORRADE_COMPARE(std::uint32_t(Cpu::Sse41 & features), 8);

    CORRADE_COMPARE(std::uint32_t(features & Cpu::Ssse3), 0);

    Cpu::Features features2 = Cpu::Sse41|Cpu::Sse2|Cpu::Ssse3;
    CORRADE_COMPARE(std::uint32_t(features & features2), 9);

    features &= features2;
    CORRADE_COMPARE(std::uint32_t(features), 9);

    constexpr Cpu::Features cFeatures = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr Cpu::Features cFeatures1 = cFeatures & Cpu::Sse41;
    constexpr Cpu::Features cFeatures2 = Cpu::Sse41 & cFeatures;
    CORRADE_COMPARE(std::uint32_t(cFeatures1), 8);
    CORRADE_COMPARE(std::uint32_t(cFeatures2), 8);

    /* Test also the compile-time operation, different values should be
       different types but same values should be same types */
    constexpr auto cTags = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr auto cTags1 = cTags & Cpu::Sse41;
    constexpr auto cTags2 = Cpu::Sse41 & cTags;
    CORRADE_COMPARE(std::uint32_t(cTags1), 8);
    CORRADE_COMPARE(std::uint32_t(cTags2), 8);
    CORRADE_VERIFY(!std::is_same<decltype(cTags), decltype(cTags1)>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTags1), decltype(cTags2)>::value);

    /* And also with Tags<> on both sides, check that it doesn't decay to an
       int or other horrible thing */
    constexpr auto cTags3 = Cpu::Ssse3|Cpu::Sse41;
    constexpr auto cTags4 = cTags3 & cTags;
    CORRADE_COMPARE(std::uint32_t(cTags4), 8);
    CORRADE_VERIFY(std::is_same<decltype(cTags4), const Cpu::Implementation::Tags<8>>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorXor() {
    #ifdef CORRADE_TARGET_X86
    /* This is actually using the compile-time operation, producing Tags<bits>,
       which is tested explicitly below, but that should be completely
       transparent to the user and work as if it produced Features directly
       instead of going through Tags first */
    CORRADE_COMPARE(std::uint32_t(Cpu::Sse3 ^ Cpu::Sse3), 0);
    CORRADE_COMPARE(std::uint32_t(Cpu::Sse3 ^ Cpu::Sse2), 3);

    Cpu::Features features = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    CORRADE_COMPARE(std::uint32_t(features ^ Cpu::Sse2), 10);
    CORRADE_COMPARE(std::uint32_t(Cpu::Sse2 ^ features), 10);

    CORRADE_COMPARE(std::uint32_t(features ^ Cpu::Sse41), 3);

    Cpu::Features features2 = Cpu::Sse41|Cpu::Sse2|Cpu::Ssse3;
    CORRADE_COMPARE(std::uint32_t(features ^ features2), 6);

    features ^= features2;
    CORRADE_COMPARE(std::uint32_t(features), 6);

    constexpr Cpu::Features cFeatures = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr Cpu::Features cFeatures1 = cFeatures ^ Cpu::Sse2;
    constexpr Cpu::Features cFeatures2 = Cpu::Sse2 ^ cFeatures;
    CORRADE_COMPARE(std::uint32_t(cFeatures1), 10);
    CORRADE_COMPARE(std::uint32_t(cFeatures2), 10);

    /* Test also the compile-time operation, different values should be
       different types but same values should be same types */
    constexpr auto cTags = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr auto cTags1 = cTags ^ Cpu::Sse2;
    constexpr auto cTags2 = Cpu::Sse2 ^ cTags;
    CORRADE_COMPARE(std::uint32_t(cTags1), 10);
    CORRADE_COMPARE(std::uint32_t(cTags2), 10);
    CORRADE_VERIFY(!std::is_same<decltype(cTags), decltype(cTags1)>::value);
    CORRADE_VERIFY(std::is_same<decltype(cTags1), decltype(cTags2)>::value);

    /* And also with Tags<> on both sides, check that it doesn't decay to an
       int or other horrible thing */
    constexpr auto cTags3 = Cpu::Ssse3|Cpu::Sse41;
    constexpr auto cTags4 = cTags3 ^ cTags;
    CORRADE_COMPARE(std::uint32_t(cTags4), 7);
    CORRADE_VERIFY(std::is_same<decltype(cTags4), const Cpu::Implementation::Tags<7>>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorBoolScalar() {
    CORRADE_COMPARE(!!Cpu::Features{Cpu::Scalar}, false);

    constexpr bool cFeatures = !!Cpu::Features{Cpu::Scalar};
    CORRADE_VERIFY(!cFeatures);

    /* Have to use an implementation detail to create the Tags type here. Which
       is fine, people shouldn't need to do this directly. */
    constexpr bool cTags = !!Cpu::Implementation::tags(Cpu::Scalar);
    CORRADE_VERIFY(!cTags);
}

void CpuTest::featuresOperatorBool() {
    #ifdef CORRADE_TARGET_X86
    Cpu::Features features = Cpu::Sse3|Cpu::Sse2;
    CORRADE_COMPARE(!!(features & Cpu::Sse41), false);
    CORRADE_COMPARE(!!(features & Cpu::Sse3), true);

    constexpr Cpu::Features cFeatures = Cpu::Sse3|Cpu::Sse2;
    constexpr bool cFeatures1 = !!(cFeatures & Cpu::Sse41);
    constexpr bool cFeatures2 = !!(cFeatures & Cpu::Sse3);
    CORRADE_VERIFY(!cFeatures1);
    CORRADE_VERIFY(cFeatures2);

    /* Test also the compile-time operation */
    constexpr auto cTags = Cpu::Sse3|Cpu::Sse2;
    constexpr bool cTags1 = !!(cTags & Cpu::Sse41);
    constexpr bool cTags2 = !!(cTags & Cpu::Sse3);
    CORRADE_VERIFY(!cTags1);
    CORRADE_VERIFY(cTags2);
    CORRADE_VERIFY(!std::is_same<decltype(cTags), Cpu::Features>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorInverse() {
    #ifdef CORRADE_TARGET_X86
    /* This is actually using the compile-time operation, producing Tags<bits>,
       which is tested explicitly below, but that should be completely
       transparent to the user and work as if it produced Features directly
       instead of going through Tags first */
    CORRADE_COMPARE(std::uint32_t(~Cpu::Scalar), 0xffffffffu);
    CORRADE_COMPARE(std::uint32_t(~(Cpu::Sse41|Cpu::Sse3)), 4294967285u);
    CORRADE_COMPARE(std::uint32_t(~Cpu::Sse41), 4294967287u);

    constexpr Cpu::Features cFeatures1 = ~Cpu::Scalar;
    constexpr Cpu::Features cFeatures2 = ~(Cpu::Sse41|Cpu::Sse3);
    CORRADE_COMPARE(std::uint32_t(cFeatures1), 0xffffffffu);
    CORRADE_COMPARE(std::uint32_t(cFeatures2), 4294967285u);

    /* Test also the compile-time operation, different values should be
       different types but same values should be same types */
    constexpr auto cTags1 = ~Cpu::Scalar;
    constexpr auto cTags2 = ~(Cpu::Sse41|Cpu::Sse3);
    CORRADE_COMPARE(std::uint32_t(cTags1), 0xffffffffu);
    CORRADE_COMPARE(std::uint32_t(cTags2), 4294967285u);
    CORRADE_VERIFY(!std::is_same<decltype(cTags1), Cpu::Features>::value);
    CORRADE_VERIFY(!std::is_same<decltype(cTags1), decltype(cTags2)>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresCompare() {
    #ifdef CORRADE_TARGET_X86
    Cpu::Features features = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    CORRADE_VERIFY(features == features);
    CORRADE_VERIFY(!(features != features));
    CORRADE_VERIFY(Cpu::Sse3 == Cpu::Sse3);
    CORRADE_VERIFY(Cpu::Sse3 != Cpu::Sse41);

    CORRADE_VERIFY(Cpu::Scalar <= Cpu::Sse41);
    CORRADE_VERIFY(Cpu::Sse41 >= Cpu::Scalar);
    CORRADE_VERIFY(Cpu::Sse41 <= Cpu::Sse41);
    CORRADE_VERIFY(Cpu::Sse41 >= Cpu::Sse41);
    CORRADE_VERIFY(Cpu::Sse41 <= features);
    CORRADE_VERIFY(features >= Cpu::Sse41);
    CORRADE_VERIFY(features <= features);
    CORRADE_VERIFY(features >= features);

    CORRADE_VERIFY(features <= (Cpu::Sse41|Cpu::Sse2|Cpu::Sse3|Cpu::Ssse3));
    CORRADE_VERIFY(!(features >= (Cpu::Sse41|Cpu::Sse2|Cpu::Sse3|Cpu::Ssse3)));

    constexpr Cpu::Features cFeatures = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr bool cFeaturesEqual = cFeatures == cFeatures;
    constexpr bool cFeaturesNonEqual = cFeatures != cFeatures;
    constexpr bool cFeaturesLessEqual = cFeatures <= cFeatures;
    constexpr bool cFeaturesGreaterEqual = cFeatures >= cFeatures;
    CORRADE_VERIFY(cFeaturesEqual);
    CORRADE_VERIFY(!cFeaturesNonEqual);
    CORRADE_VERIFY(cFeaturesLessEqual);
    CORRADE_VERIFY(cFeaturesGreaterEqual);

    constexpr auto cTags = Cpu::Sse41|Cpu::Sse2|Cpu::Sse3;
    constexpr bool cTagsEqual = cTags == cTags;
    constexpr bool cTagsNonEqual = cTags != cTags;
    constexpr bool cTagsLessEqual = cTags <= cTags;
    constexpr bool cTagsGreaterEqual = cTags >= cTags;
    CORRADE_VERIFY(cTagsEqual);
    CORRADE_VERIFY(!cTagsNonEqual);
    CORRADE_VERIFY(cTagsLessEqual);
    CORRADE_VERIFY(cTagsGreaterEqual);
    CORRADE_VERIFY(!std::is_same<decltype(cTags), Cpu::Features>::value);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::detectDefault() {
    CORRADE_INFO("Detected:" << Debug::packed << Cpu::DefaultBase);

    /* There should be at least something if we have any of the defines
       present */
    #if defined(CORRADE_TARGET_SSE2) || defined(CORRADE_TARGET_SSE3) || defined(CORRADE_TARGET_SSSE3) || defined(CORRADE_TARGET_SSE41) || defined(CORRADE_TARGET_SSE42) || defined(CORRADE_TARGET_AVX) || defined(CORRADE_TARGET_AVX2) || defined(CORRADE_TARGET_NEON) || defined(CORRADE_TARGET_SIMD128)
    CORRADE_VERIFY(Cpu::Features{Cpu::DefaultBase});

    /* And nothing if we don't */
    #else
    CORRADE_VERIFY(!Cpu::Features{Cpu::DefaultBase});
    #endif
}

void CpuTest::detect() {
    auto&& data = DetectData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Cpu::Features features = data.function();
    CORRADE_INFO("Detected:" << Debug::packed << features);

    /* The compile-time feature should be listed among these as well, otherwise
       we wouldn't even be able to run the code. */
    CORRADE_VERIFY(features >= Cpu::DefaultBase);

    #ifdef CORRADE_TARGET_X86
    /* Test that for every feature, the subset is present as well */
    if(features & Cpu::Avx512f) CORRADE_VERIFY(features & Cpu::Avx2);
    if(features & Cpu::Avx2) CORRADE_VERIFY(features & Cpu::Avx);
    if(features & Cpu::Avx) CORRADE_VERIFY(features & Cpu::Sse42);
    if(features & Cpu::Sse42) CORRADE_VERIFY(features & Cpu::Sse41);
    if(features & Cpu::Sse41) CORRADE_VERIFY(features & Cpu::Ssse3);
    if(features & Cpu::Ssse3) CORRADE_VERIFY(features & Cpu::Sse3);
    if(features & Cpu::Sse3) CORRADE_VERIFY(features & Cpu::Sse2);
    #elif defined(CORRADE_TARGET_ARM)
    if(features & Cpu::NeonFp16) CORRADE_VERIFY(features & Cpu::NeonFma);
    if(features & Cpu::NeonFma) CORRADE_VERIFY(features & Cpu::Neon);
    #else
    /* WebAssembly currently has just one feature, so no subset testing applies
       on those */
    #endif
}

void CpuTest::bitIndex() {
    CORRADE_COMPARE(Cpu::Implementation::BitIndex<0>::Value, 0);
    CORRADE_COMPARE(Cpu::Implementation::BitIndex<1>::Value, 1);
    CORRADE_COMPARE(Cpu::Implementation::BitIndex<(1 << 7)>::Value, 8);
    CORRADE_COMPARE(Cpu::Implementation::BitIndex<(1 << 15)>::Value, 16);
}

void CpuTest::bitCount() {
    CORRADE_COMPARE(Cpu::Implementation::BitCount<0>::Value, 0);
    CORRADE_COMPARE(Cpu::Implementation::BitCount<(1 << 7)>::Value, 1);
    CORRADE_COMPARE(Cpu::Implementation::BitCount<12345>::Value, 6);
    CORRADE_COMPARE(Cpu::Implementation::BitCount<65432>::Value, 11);
    CORRADE_COMPARE(Cpu::Implementation::BitCount<0xffffu>::Value, 16);
}

template<unsigned int i> unsigned int priorityValue(Cpu::Implementation::Priority<i>) {
    return i;
}

void CpuTest::priority() {
    #ifdef CORRADE_TARGET_X86
    /* Extra tag alone is always 1 */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::AvxFma)), 1);
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Popcnt)), 1);

    /* More extra tags together is their count */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::AvxFma|Cpu::AvxF16c|Cpu::Lzcnt)), 3);

    /* Base tag alone is its BitIndex, where the Scalar is the lowest, thus
       zero, times the count of extra tags plus one */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Scalar)), 0);
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Sse2)), 1*6);
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Avx2)), 7*6);

    /* Base tag + extra tags is a sum of the two */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Avx2|Cpu::AvxFma|Cpu::AvxF16c)), 7*6 + 2);
    #elif defined(CORRADE_TARGET_ARM)
    /* Base tag alone is its BitIndex, where the Scalar is the lowest, thus
       zero, times one as there are no extra tags */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Scalar)), 0);
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Neon)), 1);
    #elif defined(CORRADE_TARGET_WASM)
    /* Base tag alone is its BitIndex, where the Scalar is the lowest, thus
       zero, times one as there are no extra tags */
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Scalar)), 0);
    CORRADE_COMPARE(priorityValue(Cpu::Implementation::priority(Cpu::Simd128)), 1);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#if defined(CORRADE_TARGET_X86) || defined(CORRADE_TARGET_ARM)
const char* dispatch(Cpu::ScalarT) { return "scalar"; }
#ifdef CORRADE_TARGET_X86
const char* dispatch(Cpu::Sse3T) { return "SSE3"; }
const char* dispatch(Cpu::Avx2T) { return "AVX2"; }
#elif defined(CORRADE_TARGET_ARM)
const char* dispatch(Cpu::NeonT) { return "NEON"; }
const char* dispatch(Cpu::NeonFmaT) { return "NEON FMA"; }
#else
#error
#endif
#endif

void CpuTest::tagDispatch() {
    #ifdef CORRADE_TARGET_X86
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatch(Cpu::Avx512f), "AVX2"_s);
    CORRADE_COMPARE(dispatch(Cpu::Sse42), "SSE3"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatch(Cpu::Sse3), "SSE3"_s);

    /* Anything below gets ... the scalar */
    CORRADE_COMPARE(dispatch(Cpu::Sse2), "scalar"_s);
    #elif defined(CORRADE_TARGET_ARM)
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatch(Cpu::NeonFp16), "NEON FMA"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatch(Cpu::Neon), "NEON"_s);
    CORRADE_COMPARE(dispatch(Cpu::Scalar), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraExact(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::AvxFma|Cpu::AvxF16c)) { return "AVX2+FMA+F16C"; }
const char* dispatchExtraExact(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Popcnt)) { return "SSE4.2+POPCNT"; }
const char* dispatchExtraExact(CORRADE_CPU_DECLARE(Cpu::Ssse3)) { return "SSSE3"; }
const char* dispatchExtraExact(CORRADE_CPU_DECLARE(Cpu::Popcnt|Cpu::Lzcnt)) { return "POPCNT+LZCNT"; }
#endif

void CpuTest::tagDispatchExtraExact() {
    #ifdef CORRADE_TARGET_X86
    /* For each there's an exact matching overload */
    CORRADE_COMPARE(dispatchExtraExact(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::AvxFma|Cpu::AvxF16c)), "AVX2+FMA+F16C"_s);
    CORRADE_COMPARE(dispatchExtraExact(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt)), "SSE4.2+POPCNT"_s);
    CORRADE_COMPARE(dispatchExtraExact(CORRADE_CPU_SELECT(Cpu::Ssse3)), "SSSE3"_s);
    /* The base tag doesn't even need to be there */
    CORRADE_COMPARE(dispatchExtraExact(CORRADE_CPU_SELECT(Cpu::Popcnt|Cpu::Lzcnt)), "POPCNT+LZCNT"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraUnusedExtra(CORRADE_CPU_DECLARE(Cpu::Avx2)) { return "AVX2"; }
const char* dispatchExtraUnusedExtra(CORRADE_CPU_DECLARE(Cpu::Sse42)) { return "SSE4.2"; }
const char* dispatchExtraUnusedExtra(CORRADE_CPU_DECLARE(Cpu::Ssse3)) { return "SSSE3"; }
const char* dispatchExtraUnusedExtra(CORRADE_CPU_DECLARE(Cpu::Scalar)) { return "scalar"; }
#endif

void CpuTest::tagDispatchExtraUnusedExtra() {
    #ifdef CORRADE_TARGET_X86
    /* The extra tags get ignored, only the base one will be used */
    CORRADE_COMPARE(dispatchExtraUnusedExtra(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::AvxFma|Cpu::AvxF16c)), "AVX2"_s);
    CORRADE_COMPARE(dispatchExtraUnusedExtra(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt)), "SSE4.2"_s);
    CORRADE_COMPARE(dispatchExtraUnusedExtra(CORRADE_CPU_SELECT(Cpu::Ssse3)), "SSSE3"_s);
    /* The base tag doesn't even need to be there */
    CORRADE_COMPARE(dispatchExtraUnusedExtra(CORRADE_CPU_SELECT(Cpu::Popcnt|Cpu::Lzcnt)), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraFallbackExtra(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::Popcnt|Cpu::Lzcnt)) { return "AVX2+POPCNT+LZCNT"; }
const char* dispatchExtraFallbackExtra(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Lzcnt)) { return "SSE4.2+LZCNT"; }
const char* dispatchExtraFallbackExtra(CORRADE_CPU_DECLARE(Cpu::Sse42)) { return "SSE4.2"; }
#endif

void CpuTest::tagDispatchExtraFallbackExtra() {
    #ifdef CORRADE_TARGET_X86
    /* The base tag stays the same, but the extra ones get dropped */
    CORRADE_COMPARE(dispatchExtraFallbackExtra(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::Popcnt|Cpu::Lzcnt|Cpu::AvxFma|Cpu::AvxF16c)), "AVX2+POPCNT+LZCNT"_s);
    CORRADE_COMPARE(dispatchExtraFallbackExtra(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt|Cpu::Lzcnt)), "SSE4.2+LZCNT"_s);
    CORRADE_COMPARE(dispatchExtraFallbackExtra(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt)), "SSE4.2"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraFallbackBase(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::Popcnt|Cpu::Lzcnt)) { return "AVX2+POPCNT+LZCNT"; }
const char* dispatchExtraFallbackBase(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Lzcnt)) { return "SSE42+LZCNT"; }
const char* dispatchExtraFallbackBase(CORRADE_CPU_DECLARE(Cpu::Ssse3)) { return "SSSE3"; }
const char* dispatchExtraFallbackBase(CORRADE_CPU_DECLARE(Cpu::Popcnt|Cpu::Lzcnt)) { return "POPCNT+LZCNT"; }
#endif

void CpuTest::tagDispatchExtraFallbackBase() {
    #ifdef CORRADE_TARGET_X86
    /* The extra tags stay the same, but the base one gets lowered */
    CORRADE_COMPARE(dispatchExtraFallbackBase(CORRADE_CPU_SELECT(Cpu::Avx512f|Cpu::Popcnt|Cpu::Lzcnt)), "AVX2+POPCNT+LZCNT"_s);
    CORRADE_COMPARE(dispatchExtraFallbackBase(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::Lzcnt)), "SSE42+LZCNT"_s);
    CORRADE_COMPARE(dispatchExtraFallbackBase(CORRADE_CPU_SELECT(Cpu::Sse42)), "SSSE3"_s);
    CORRADE_COMPARE(dispatchExtraFallbackBase(CORRADE_CPU_SELECT(Cpu::Sse3|Cpu::Popcnt|Cpu::Lzcnt)), "POPCNT+LZCNT"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraFallbackBoth(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::AvxFma)) { return "AVX2+FMA"; }
const char* dispatchExtraFallbackBoth(CORRADE_CPU_DECLARE(Cpu::Avx|Cpu::AvxF16c)) { return "AVX+F16C"; }
const char* dispatchExtraFallbackBoth(CORRADE_CPU_DECLARE(Cpu::Avx)) { return "AVX"; }
const char* dispatchExtraFallbackBoth(CORRADE_CPU_DECLARE(Cpu::AvxF16c|Cpu::AvxFma)) { return "F16C+FMA"; }
#endif

void CpuTest::tagDispatchExtraFallbackBoth() {
    #ifdef CORRADE_TARGET_X86
    /* Top-class HW, just pick AVX2 as it's the closest */
    CORRADE_COMPARE(dispatchExtraFallbackBoth(CORRADE_CPU_SELECT(Cpu::Avx512f|Cpu::AvxFma|Cpu::AvxF16c)), "AVX2+FMA"_s);

    /* We have one extra less than required for the AVX2 variant, fall back to
       AVX with F16C */
    CORRADE_COMPARE(dispatchExtraFallbackBoth(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::AvxF16c)), "AVX+F16C"_s);

    /* We have AVX2, but neither of the extra bits, just some irrelevant ones,
       take plain AVX */
    CORRADE_COMPARE(dispatchExtraFallbackBoth(CORRADE_CPU_SELECT(Cpu::Avx2|Cpu::Lzcnt|Cpu::Popcnt)), "AVX"_s);

    /* We have only SSE3 but both extra bits, fall back to the scalar version
       that has them. Yes, it's silly, but the scalar fallback needs to be
       verified. */
    CORRADE_COMPARE(dispatchExtraFallbackBoth(CORRADE_CPU_SELECT(Cpu::Sse3|Cpu::AvxF16c|Cpu::AvxFma)), "F16C+FMA"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
const char* dispatchExtraPriority(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Popcnt|Cpu::Lzcnt)) {
    return "SSE4.2+POPCNT+LZCNT";
}
const char* dispatchExtraPriority(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Popcnt)) {
    return "SSE4.2+POPCNT";
}
const char* dispatchExtraPriority(CORRADE_CPU_DECLARE(Cpu::Sse42|Cpu::Lzcnt)) {
    return "SSE4.2+LZCNT";
}
#endif

void CpuTest::tagDispatchExtraPriority() {
    #ifdef CORRADE_TARGET_X86
    /* The candidate which has the most tags gets picked. OTOH, if it wouldn't
       be there, this call would be ambiguous. */
    CORRADE_COMPARE(dispatchExtraPriority(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt|Cpu::Lzcnt)), "SSE4.2+POPCNT+LZCNT"_s);

    /* Both single-extra-tag candidates have the same calculated priority, the
       one for which we actually have the feature gets picked */
    CORRADE_COMPARE(dispatchExtraPriority(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Popcnt)), "SSE4.2+POPCNT"_s);
    CORRADE_COMPARE(dispatchExtraPriority(CORRADE_CPU_SELECT(Cpu::Sse42|Cpu::Lzcnt)), "SSE4.2+LZCNT"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

/* The lambda wrappers are marked with CORRADE_ALWAYS_INLINE to make them go
   away with optimizations */
#if defined(CORRADE_TARGET_X86) || defined(CORRADE_TARGET_ARM) || defined(CORRADE_TARGET_WASM)
typedef const char*(*DispatchRuntimeT)();

CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::ScalarT) {
    return []() -> const char* { return "scalar"; };
}
#ifdef CORRADE_TARGET_X86
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::Sse3T) {
    return []() -> const char* { return "SSE3"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::Avx2T) {
    return []() -> const char* { return "AVX2"; };
}
#elif defined(CORRADE_TARGET_ARM)
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::NeonT) {
    return []() -> const char* { return "NEON"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::NeonFmaT) {
    return []() -> const char* { return "NEON FMA"; };
}
#elif defined(CORRADE_TARGET_WASM)
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntime(Cpu::Simd128T) {
    return []() -> const char* { return "SIMD128"; };
}
#else
#error
#endif

CORRADE_CPU_DISPATCHER_BASE(dispatchRuntime)
#endif

void CpuTest::tagDispatchRuntime() {
    /* Explicitly casting to Cpu::Features here to ensure it's indeed going
       through the runtime switch and not directly. Otherwise it's the same as
       tagDispatch(). */

    #ifdef CORRADE_TARGET_X86
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Avx512f})(), "AVX2"_s);
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Sse42})(), "SSE3"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Sse3})(), "SSE3"_s);

    /* Anything below gets ... the scalar */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Sse2})(), "scalar"_s);
    #elif defined(CORRADE_TARGET_ARM)
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::NeonFp16})(), "NEON FMA"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Neon})(), "NEON"_s);
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Scalar})(), "scalar"_s);
    #elif defined(CORRADE_TARGET_WASM)
    /* Exact match. No other opportunity to test anything, but better than have
       the macro completely untested. */
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Simd128})(), "SIMD128"_s);
    CORRADE_COMPARE(dispatchRuntime(Cpu::Features{Cpu::Scalar})(), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtra(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
    return []() -> const char* { return "scalar"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtra(CORRADE_CPU_DECLARE(Cpu::AvxF16c|Cpu::AvxFma)) {
    return []() -> const char* { return "F16C+FMA"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtra(CORRADE_CPU_DECLARE(Cpu::Avx)) {
    return []() -> const char* { return "AVX"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtra(CORRADE_CPU_DECLARE(Cpu::Avx|Cpu::AvxF16c)) {
    return []() -> const char* { return "AVX+F16C"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtra(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::AvxFma)) {
    return []() -> const char* { return "AVX2+FMA"; };
}

CORRADE_CPU_DISPATCHER(dispatchRuntimeExtra, Cpu::AvxF16c, Cpu::AvxFma)
#endif

void CpuTest::tagDispatchRuntimeExtra() {
    /* Explicitly casting to Cpu::Features here to ensure it's indeed going
       through the runtime switch and not directly. Otherwise it's mostly the
       same as tagDispatchExtraFallbackBoth(). */

    #ifdef CORRADE_TARGET_X86
    /* Top-class HW, just pick AVX2 as it's the closest */
    CORRADE_COMPARE(dispatchRuntimeExtra(Cpu::Features{Cpu::Avx512f|Cpu::AvxFma|Cpu::AvxF16c})(), "AVX2+FMA"_s);

    /* We have one extra less than required for the AVX2 variant, fall back to
       AVX with F16C */
    CORRADE_COMPARE(dispatchRuntimeExtra(Cpu::Features{Cpu::Avx2|Cpu::AvxF16c})(), "AVX+F16C"_s);

    /* We have AVX2, but neither of the extra bits, just some irrelevant ones,
       take plain AVX */
    CORRADE_COMPARE(dispatchRuntimeExtra(Cpu::Features{Cpu::Avx2|Cpu::Lzcnt|Cpu::Popcnt})(), "AVX"_s);

    /* We have only SSE3 but both extra bits, fall back to the scalar version
       that has them. Yes, it's silly, but the scalar fallback needs to be
       verified. */
    CORRADE_COMPARE(dispatchRuntimeExtra(Cpu::Features{Cpu::Sse3|Cpu::AvxF16c|Cpu::AvxFma})(), "F16C+FMA"_s);

    /* Finally, this will fall back to the scalar version. */
    CORRADE_COMPARE(dispatchRuntimeExtra(Cpu::Features{Cpu::Sse3})(), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#ifdef CORRADE_TARGET_X86
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraCombination(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
    return []() -> const char* { return "scalar"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraCombination(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt)) {
    return []() -> const char* { return "SSE4.1+POPCNT+LZCNT"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraCombination(CORRADE_CPU_DECLARE(Cpu::Avx2|Cpu::Popcnt|Cpu::Lzcnt)) {
    return []() -> const char* { return "AVX2+POPCNT+LZCNT"; };
}

CORRADE_CPU_DISPATCHER(dispatchRuntimeExtraCombination, Cpu::Popcnt|Cpu::Lzcnt)
#endif

void CpuTest::tagDispatchRuntimeExtraCombination() {
    /* This verifies that the CORRADE_CPU_DISPATCHER() can accept also
       tag combinations, in case the variants only ever use them together */

    #ifdef CORRADE_TARGET_X86
    /* Verify the tag combinations don't get skipped if we have them all or
       more */
    CORRADE_COMPARE(dispatchRuntimeExtraCombination(Cpu::Features{Cpu::Avx512f|Cpu::Popcnt|Cpu::Lzcnt|Cpu::AvxFma})(), "AVX2+POPCNT+LZCNT"_s);
    CORRADE_COMPARE(dispatchRuntimeExtraCombination(Cpu::Features{Cpu::Avx|Cpu::Popcnt|Cpu::Lzcnt})(), "SSE4.1+POPCNT+LZCNT"_s);

    /* But also that they don't get picked if we don't have them all */
    CORRADE_COMPARE(dispatchRuntimeExtraCombination(Cpu::Features{Cpu::Avx512f|Cpu::Popcnt|Cpu::AvxFma})(), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

#if defined(CORRADE_TARGET_X86) || defined(CORRADE_TARGET_ARM) || defined(CORRADE_TARGET_WASM)
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
    return []() -> const char* { return "scalar"; };
}

#ifdef CORRADE_TARGET_X86
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::Sse2)) {
    return []() -> const char* { return "SSE2"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::Avx)) {
    return []() -> const char* { return "AVX"; };
}
CORRADE_CPU_DISPATCHER(dispatchRuntimeExtraZeroExtra)
#elif defined(CORRADE_TARGET_ARM)
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::Neon)) {
    return []() -> const char* { return "NEON"; };
}
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::NeonFma)) {
    return []() -> const char* { return "NEON FMA"; };
}

CORRADE_CPU_DISPATCHER(dispatchRuntimeExtraZeroExtra)
#elif defined(CORRADE_TARGET_WASM)
CORRADE_ALWAYS_INLINE DispatchRuntimeT dispatchRuntimeExtraZeroExtra(CORRADE_CPU_DECLARE(Cpu::Simd128)) {
    return []() -> const char* { return "SIMD128"; };
}

CORRADE_CPU_DISPATCHER(dispatchRuntimeExtraZeroExtra)
#else
#error
#endif
#endif

void CpuTest::tagDispatchRuntimeExtraZeroExtra() {
    /* Explicitly casting to Cpu::Features here to ensure it's indeed going
       through the runtime switch and not directly. Otherwise it's mostly the
       same as tagDispatchRuntime(). */

    #ifdef CORRADE_TARGET_X86
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Avx512f})(), "AVX"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Sse2})(), "SSE2"_s);
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Scalar})(), "scalar"_s);
    #elif defined(CORRADE_TARGET_ARM)
    /* If no match, gets the next highest available */
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::NeonFp16})(), "NEON FMA"_s);

    /* Exact match */
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Neon})(), "NEON"_s);
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Scalar})(), "scalar"_s);
    #elif defined(CORRADE_TARGET_WASM)
    /* Exact match. No other opportunity to test anything, but better than have
       the macro completely untested. */
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Simd128})(), "SIMD128"_s);
    CORRADE_COMPARE(dispatchRuntimeExtraZeroExtra(Cpu::Features{Cpu::Scalar})(), "scalar"_s);
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

using DispatchedT = Cpu::Features(*)();

CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
    return []() -> Cpu::Features { return Cpu::Scalar; };
}
#ifdef CORRADE_TARGET_X86
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Sse2)) {
    return []() -> Cpu::Features { return Cpu::Sse2; };
}
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Avx)) {
    return []() -> Cpu::Features { return Cpu::Avx; };
}
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Avx|Cpu::AvxFma)) {
    return []() -> Cpu::Features { return Cpu::Avx|Cpu::AvxFma; };
}

CORRADE_CPU_DISPATCHER(dispatchedImplementation, Cpu::AvxFma)
#elif defined(CORRADE_TARGET_ARM)
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::NeonFp16)) {
    return []() -> Cpu::Features { return Cpu::NeonFp16; };
}
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Neon)) {
    return []() -> Cpu::Features { return Cpu::Neon; };
}

CORRADE_CPU_DISPATCHER(dispatchedImplementation)
#elif defined(CORRADE_TARGET_WASM)
CORRADE_ALWAYS_INLINE DispatchedT dispatchedImplementation(CORRADE_CPU_DECLARE(Cpu::Simd128)) {
    return []() -> Cpu::Features { return Cpu::Simd128; };
}

CORRADE_CPU_DISPATCHER(dispatchedImplementation)
#endif

/* CORRADE_NEVER_INLINE to make it possible to look at the disassembly. The
   lambda body should be fully inlined here. */
CORRADE_NEVER_INLINE Cpu::Features dispatchedCompileTime() {
    /* While calling without CORRADE_CPU_SELECT() would work too, it'd go
       through the runtime dispatch. And then the call would *not* get inlined,
       no matter how many force inlines I put onto the lambdas. */
    return dispatchedImplementation(CORRADE_CPU_SELECT(Cpu::Default))();
}

void CpuTest::tagDispatchedCompileTime() {
    Cpu::Features dispatchedFeatures = dispatchedCompileTime();
    CORRADE_INFO("Dispatched to:" << dispatchedFeatures);

    Cpu::Features features = Cpu::compiledFeatures();
    #ifdef CORRADE_TARGET_X86
    if(features >= (Cpu::Avx|Cpu::AvxFma))
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx|Cpu::AvxFma);
    else if(features >= Cpu::Avx)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx);
    else if(features >= Cpu::Sse2)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Sse2);
    else
    #elif defined(CORRADE_TARGET_ARM)
    if(features >= Cpu::NeonFp16)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::NeonFp16);
    else if(features >= Cpu::Neon)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Neon);
    else
    #elif defined(CORRADE_TARGET_WASM)
    if(features >= Cpu::Simd128)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Simd128);
    else
    #endif
    {
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Scalar);
    }
}

CORRADE_CPU_DISPATCHED_POINTER(dispatchedImplementation, Cpu::Features (*dispatchedPointer)())

void CpuTest::tagDispatchedPointer() {
    Cpu::Features dispatchedFeatures = dispatchedPointer();
    CORRADE_INFO("Dispatched to:" << dispatchedFeatures);

    Cpu::Features features = Cpu::runtimeFeatures();
    #ifdef CORRADE_TARGET_X86
    if(features >= (Cpu::Avx|Cpu::AvxFma))
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx|Cpu::AvxFma);
    else if(features >= Cpu::Avx)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx);
    else if(features >= Cpu::Sse2)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Sse2);
    else
    #elif defined(CORRADE_TARGET_ARM)
    if(features >= Cpu::NeonFp16)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::NeonFp16);
    else if(features >= Cpu::Neon)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Neon);
    else
    #elif defined(CORRADE_TARGET_WASM)
    if(features >= Cpu::Simd128)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Simd128);
    else
    #endif
    {
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Scalar);
    }
}

#ifdef CORRADE_CPU_USE_IFUNC
CORRADE_CPU_DISPATCHED_IFUNC(dispatchedImplementation, Cpu::Features dispatchedIfunc())
#endif

void CpuTest::tagDispatchedIfunc() {
    #ifndef CORRADE_CPU_USE_IFUNC
    CORRADE_SKIP("CORRADE_CPU_USE_IFUNC not available");
    #else
    Cpu::Features dispatchedFeatures = dispatchedIfunc();
    CORRADE_INFO("Dispatched to:" << dispatchedFeatures);

    Cpu::Features features = Cpu::runtimeFeatures();
    #ifdef CORRADE_TARGET_X86
    if(features >= (Cpu::Avx|Cpu::AvxFma))
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx|Cpu::AvxFma);
    else if(features >= Cpu::Avx)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Avx);
    else if(features >= Cpu::Sse2)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Sse2);
    else
    #elif defined(CORRADE_TARGET_ARM)
    if(features >= Cpu::NeonFp16)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::NeonFp16);
    else if(features >= Cpu::Neon)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Neon);
    else
    #elif defined(CORRADE_TARGET_WASM)
    if(features >= Cpu::Simd128)
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Simd128);
    else
    #endif
    {
        CORRADE_COMPARE(dispatchedFeatures, Cpu::Scalar);
    }
    #endif
}

constexpr std::size_t BenchmarkDispatchedRepeats = 1000000;

/* Otherwise Clang inlines even through the function pointer */
#ifdef CORRADE_TARGET_CLANG
CORRADE_NEVER_INLINE
#endif
int benchmarkDispatchedImplementation(int a) {
    return a + 1;
}

CORRADE_NEVER_INLINE int benchmarkDispatchedCompileTime(int a) {
    /* Because benchmarkDispatchedImplementation() is marked as never inline
       on Clang, calling it from here would result in two deinlined calls,
       skewing the benchmark */
    #ifdef CORRADE_TARGET_CLANG
    return a + 1;
    #else
    return benchmarkDispatchedImplementation(a);
    #endif
}

void CpuTest::benchmarkTagDispatchedCompileTime() {
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedCompileTime(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
}

auto benchmarkDispatchedImplementation(Cpu::Features) -> int(*)(int) {
    return benchmarkDispatchedImplementation;
}

CORRADE_CPU_DISPATCHED_POINTER(benchmarkDispatchedImplementation, int(*benchmarkDispatchedPointer)(int))

void CpuTest::benchmarkTagDispatchedPointer() {
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedPointer(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
}

#ifdef CORRADE_CPU_USE_IFUNC
CORRADE_CPU_DISPATCHED_IFUNC(benchmarkDispatchedImplementation, int benchmarkDispatchedIfunc(int))
#endif

void CpuTest::benchmarkTagDispatchedIfunc() {
    #ifndef CORRADE_CPU_USE_IFUNC
    CORRADE_SKIP("CORRADE_CPU_USE_IFUNC not available");
    #else
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedIfunc(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
    #endif
}

void CpuTest::benchmarkTagDispatchedExternalLibraryCompileTime() {
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedExternalLibraryCompileTime(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
}

void CpuTest::benchmarkTagDispatchedExternalLibraryPointer() {
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedExternalLibraryPointer(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
}

void CpuTest::benchmarkTagDispatchedExternalLibraryIfunc() {
    #ifndef CORRADE_CPU_USE_IFUNC
    CORRADE_SKIP("CORRADE_CPU_USE_IFUNC not available");
    #else
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedExternalLibraryIfunc(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
    #endif
}

void CpuTest::benchmarkTagDispatchedExternalLibraryEveryCall() {
    int a = 0;
    CORRADE_BENCHMARK(BenchmarkDispatchedRepeats) {
        a = Test::benchmarkDispatchedExternalLibraryEveryCall(Cpu::compiledFeatures())(a);
    }

    CORRADE_COMPARE(a, BenchmarkDispatchedRepeats);
}

/* Not using an argument here since we *don't* want the overload delegating
   in this case -- it would hide errors when a certain instruction set doesn't
   have a corresponding overload, as it'd fall back to a parent one. I'm also
   defining a catch-all implementation with CORRADE_SKIP() instead of having an
   #ifdef CORRADE_ENABLE_* around every variant in addTests(), because this way
   it's clearly visible in the test output if any CORRADE_ENABLE_* macro isn't
   defined for whatever reason. */
template<class T> int callInstructionFor() {
    CORRADE_SKIP("No CORRADE_ENABLE_* macro for" << Cpu::features<T>() << "on this compiler");
}
/* The goal here is to use instructions that would make the compilation fail
   on GCC and default flags (i.e., no -march=native etc.) if the
   CORRADE_ENABLE_* macro is removed. While this is quite a lot of code, it's a
   good overview of how all the instructions look like... and it also uncovers
   a MASSIVE amount of platform-specific warts and compiler bugs that the API
   should take care of.

   All these are also marked with CORRADE_NEVER_INLINE to make it easier to see
   into what code they get actually compiled. Except for the catch-all variant,
   which isn't interesting for disassembly. */
#ifdef CORRADE_ENABLE_SSE2
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SSE2) int callInstructionFor<Cpu::Sse2T>() {
    __m128i a = _mm_set_epi32(0x80808080, 0, 0x80808080, 0);

    /* All instructions SSE2 */

    int mask = _mm_movemask_epi8(a);
    CORRADE_COMPARE(mask, 0xf0f0); /* 0b1111000011110000 */
    return mask;
}
#endif
#ifdef CORRADE_ENABLE_SSE3
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SSE3) int callInstructionFor<Cpu::Sse3T>() {
    const std::uint32_t a[]{0, 10, 20, 30, 40};

    /* SSE3 */
    union {
        __m128i v;
        int s[4];
    } b;
    b.v = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(a + 1));

    CORRADE_COMPARE(b.s[0], 10);
    CORRADE_COMPARE(b.s[1], 20);
    CORRADE_COMPARE(b.s[2], 30);
    CORRADE_COMPARE(b.s[3], 40);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_SSSE3
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SSSE3) int callInstructionFor<Cpu::Ssse3T>() {
    __m128i a = _mm_set_epi32(-10, 20, -30, 40);

    /* SSSE3 */
    union {
        __m128i v;
        int s[4];
    } b;
    b.v = _mm_abs_epi32(a);

    CORRADE_COMPARE(b.s[3], 10);
    CORRADE_COMPARE(b.s[2], 20);
    CORRADE_COMPARE(b.s[1], 30);
    CORRADE_COMPARE(b.s[0], 40);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_SSE41
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SSE41) int callInstructionFor<Cpu::Sse41T>() {
    __m128 a = _mm_set_ps(5.47f, 2.23f, 7.62f, 0.5f);

    /* SSE4.1 */
    union {
        __m128 v;
        float s[4];
    } b;
    b.v = _mm_ceil_ps(a);

    CORRADE_COMPARE(b.s[3], 6.0f);
    CORRADE_COMPARE(b.s[2], 3.0f);
    CORRADE_COMPARE(b.s[1], 8.0f);
    CORRADE_COMPARE(b.s[0], 1.0f);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_SSE42
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SSE42) int callInstructionFor<Cpu::Sse42T>() {
    __m128i a = _mm_set_epi64x(50, 60);
    __m128i b = _mm_set_epi64x(60, 50);

    /* SSE4.2 */
    union {
        __m128i v;
        std::int64_t s[2];
    } c;
    c.v = _mm_cmpgt_epi64(a, b);

    CORRADE_COMPARE(c.s[0], -1);
    CORRADE_COMPARE(c.s[1], 0);
    return c.s[0];
}
#endif
#ifdef CORRADE_ENABLE_POPCNT
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(POPCNT) int callInstructionFor<Cpu::PopcntT>() {
    /* Just pocnt alone; using volatile to prevent this from being folded into
       a constant */
    volatile unsigned int a = 0x0005c1a6;
    unsigned int count = _mm_popcnt_u32(a);
    CORRADE_COMPARE(count, 9);
    return count;
}
#endif
#ifdef CORRADE_ENABLE_LZCNT
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(LZCNT) int callInstructionFor<Cpu::LzcntT>() {
    /* Just lzcnt alone; using volatile to prevent this from being folded into
       a constant */
    volatile int a = 0x0005c1a6;
    unsigned int count = _lzcnt_u32(a);

    /* Also verify that it does the right thing for 0. If misdetected and the
       BSR fallback gets used, this would return something random here. */
    CORRADE_COMPARE(_lzcnt_u32(0), 32);

    CORRADE_COMPARE(count, 13);
    return count;
}
#endif
#ifdef CORRADE_ENABLE_BMI1
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(BMI1) int callInstructionFor<Cpu::Bmi1T>() {
    /* Just lzcnt alone; using volatile to prevent this from being folded into
       a constant */
    volatile int a = 0x6583a000; /* 0x0005c1a6 but bit-reversed */
    /* GCC and Clang have __tzcnt_u32() as well, but MSVC has only a single
       underscore. */
    unsigned int count = _tzcnt_u32(a);

    /* Also verify that it does the right thing for 0. If misdetected and the
       BSR fallback gets used, this would return something random here. */
    CORRADE_COMPARE(_tzcnt_u32(0), 32);

    CORRADE_COMPARE(count, 13);
    return count;
}
#endif
#ifdef CORRADE_ENABLE_AVX
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(AVX) int callInstructionFor<Cpu::AvxT>() {
    __m256d a = _mm256_set_pd(5.47, 2.23, 7.62, 0.5);

    /* All instructions AVX */

    union {
        __m256d v;
        double s[4];
    } b;
    b.v = _mm256_ceil_pd(a);

    CORRADE_COMPARE(b.s[3], 6.0);
    CORRADE_COMPARE(b.s[2], 3.0);
    CORRADE_COMPARE(b.s[1], 8.0);
    CORRADE_COMPARE(b.s[0], 1.0);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_AVX_F16C
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(AVX_F16C) int callInstructionFor<Cpu::AvxF16cT>() {
    /* Values from Magnum::Math::Test::HalfTest::pack() */
    __m128 a = _mm_set_ps(0.0f, 123.75f, -0.000351512f, 3.0f);

    /* F16C */
    union {
        __m128i v;
        std::uint16_t s[8];
    } b;
    b.v = _mm_cvtps_ph(a, 0);

    CORRADE_COMPARE(b.s[3], 0x0000);
    CORRADE_COMPARE(b.s[2], 0x57bc);
    CORRADE_COMPARE(b.s[1], 0x8dc2);
    CORRADE_COMPARE(b.s[0], 0x4200);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_AVX_FMA
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(AVX_FMA) int callInstructionFor<Cpu::AvxFmaT>() {
    /* Values from Magnum::Math::Test::FunctionsTest::fma() */
    __m128 a = _mm_set_ps(0.0f,  2.0f,  1.5f,  0.5f);
    __m128 b = _mm_set_ps(0.0f,  3.0f,  2.0f, -1.0f);
    __m128 c = _mm_set_ps(0.0f, 0.75f, 0.25f,  0.1f);

    /* FMA */
    union {
        __m128 v;
        float s[4];
    } d;
    d.v = _mm_fmadd_ps(a, b, c);

    CORRADE_COMPARE(d.s[3], 0.0f);
    CORRADE_COMPARE(d.s[2], 6.75f);
    CORRADE_COMPARE(d.s[1], 3.25f);
    CORRADE_COMPARE(d.s[0], -0.4f);
    return d.s[2];
}
#endif
#ifdef CORRADE_ENABLE_AVX2
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(AVX2) int callInstructionFor<Cpu::Avx2T>() {
    __m256i a = _mm256_set_epi64x(0x8080808080808080ull, 0, 0x8080808080808080ull, 0);

    /* Like callInstructionFor<Cpu::Sse2T>(), but expanded to AVX2 */
    int mask = _mm256_movemask_epi8(a);

    CORRADE_COMPARE(mask, 0xff00ff00);/* 0b11111111000000001111111100000000 */
    return mask;
}
#endif
#ifdef CORRADE_ENABLE_AVX512F
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(AVX512F) int callInstructionFor<Cpu::Avx512fT>() {
    __m128 a = _mm_set1_ps(5.47f);

    /* AVX512 */
    int ceil = _mm_cvt_roundss_si32(a, _MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC);

    CORRADE_COMPARE(ceil, 6);
    return ceil;
}
#endif
#ifdef CORRADE_ENABLE_NEON
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(NEON) int callInstructionFor<Cpu::NeonT>() {
    int32x4_t a{-10, 20, -30, 40};

    /* All instructions NEON */
    union {
        int32x4_t v;
        int s[8];
    } b;
    b.v = vabsq_s32(a);

    CORRADE_COMPARE(b.s[0], 10);
    CORRADE_COMPARE(b.s[1], 20);
    CORRADE_COMPARE(b.s[2], 30);
    CORRADE_COMPARE(b.s[3], 40);
    return b.s[0];
}
#endif
#ifdef CORRADE_ENABLE_NEON_FMA
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(NEON_FMA) int callInstructionFor<Cpu::NeonFmaT>() {
    /* Values from Magnum::Math::Test::FunctionsTest::fma() */
    float32x4_t a{0.0f,  2.0f,  1.5f,  0.5f};
    float32x4_t b{0.0f,  3.0f,  2.0f, -1.0f};
    float32x4_t c{0.0f, 0.75f, 0.25f,  0.1f};

    /* FMA */
    union {
        float32x4_t v;
        float s[4];
    } d;
    d.v = vfmaq_f32(c, b, a);

    CORRADE_COMPARE(d.s[0], 0.0f);
    CORRADE_COMPARE(d.s[1], 6.75f);
    CORRADE_COMPARE(d.s[2], 3.25f);
    CORRADE_COMPARE(d.s[3], -0.4f);
    return d.s[2];
}
#endif
#ifdef CORRADE_ENABLE_NEON_FP16
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(NEON_FP16) int callInstructionFor<Cpu::NeonFp16T>() {
    float32x4_t a{5.47f, 2.23f, 7.62f, 0.5f};
    float16x4_t b = vcvt_f16_f32(a);

    /* FP16 */
    float16x4_t c = vrndp_f16(b);

    union {
        float32x4_t v;
        float s[4];
    } d;
    d.v = vcvt_f32_f16(c);
    CORRADE_COMPARE(d.s[0], 6.0f);
    CORRADE_COMPARE(d.s[1], 3.0f);
    CORRADE_COMPARE(d.s[2], 8.0f);
    CORRADE_COMPARE(d.s[3], 1.0f);
    return d.s[3];
}
#endif
#ifdef CORRADE_ENABLE_SIMD128
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE(SIMD128) int callInstructionFor<Cpu::Simd128T>() {
    v128_t a = wasm_f32x4_make(5.47, 2.23, 7.62, 0.5);

    /* All instructions SIMD128. wasm_f32x4_ceil() is only available in the
       finalized wasm intrinsics that's since Clang 13 which is used since
       Emscripten 2.0.13, thus any older version should not have the
       CORRADE_ENABLE_SIMD128 macro defined:
        https://github.com/llvm/llvm-project/commit/502f54049d17f5a107f833596fb2c31297a99773
        https://github.com/emscripten-core/emscripten/commit/deab7783df407b260f46352ffad2a77ca8fb0a4c */
    union {
        v128_t v;
        float s[8];
    } b;
    b.v = wasm_f32x4_ceil(a);

    CORRADE_COMPARE(b.s[0], 6.0);
    CORRADE_COMPARE(b.s[1], 3.0);
    CORRADE_COMPARE(b.s[2], 8.0);
    CORRADE_COMPARE(b.s[3], 1.0);
    return b.s[0];
}
#endif

template<class T> void CpuTest::enableMacros() {
    setTestCaseTemplateName(Cpu::TypeTraits<T>::name());

    if(!(Cpu::runtimeFeatures() & Cpu::features<T>()))
        CORRADE_SKIP("CPU feature not supported");

    CORRADE_VERIFY(true); /* to capture correct function name */
    CORRADE_VERIFY(callInstructionFor<T>());
}

#if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_AVX)
/* If set to 1, should fail on GCC (unless CORRADE_TARGET_AVX is set) */
#if 0
CORRADE_ENABLE_AVX2 CORRADE_ENABLE_AVX
#else
/* If CORRADE_TARGET_SSE2 is set (on 64bit), it'll result in just "avx2,avx" */
CORRADE_ENABLE(AVX2,SSE2,AVX)
#endif
int callInstructionMultiple() {
    if(!(Cpu::runtimeFeatures() & Cpu::Avx2))
        CORRADE_SKIP("AVX2 feature not supported");

    /* Same as callInstructionFor<Cpu::Avx2T>() */
    __m256i a = _mm256_set_epi64x(0x8080808080808080ull, 0, 0x8080808080808080ull, 0);

    /* If the AVX2 instructions aren't enabled, this will fail to link */
    int mask = _mm256_movemask_epi8(a);

    CORRADE_COMPARE(mask, 0xff00ff00);
    return mask;
}
#elif defined(CORRADE_ENABLE_NEON_FMA) && defined(CORRADE_ENABLE_NEON)
/* If set to 1, should fail on GCC (unless CORRADE_TARGET_NEON is set) */
#if 0
CORRADE_ENABLE_NEON_FMA CORRADE_ENABLE_NEON
#else
CORRADE_ENABLE(NEON_FMA,NEON)
#endif
int callInstructionMultiple() {
    if(!(Cpu::runtimeFeatures() & Cpu::NeonFma))
        CORRADE_SKIP("NEON FMA feature not supported");

    /* Same as callInstructionFor<Cpu::NeonFmaT>() */
    float32x4_t a{0.0f,  2.0f,  1.5f,  0.5f};
    float32x4_t b{0.0f,  3.0f,  2.0f, -1.0f};
    float32x4_t c{0.0f, 0.75f, 0.25f,  0.1f};

    union {
        float32x4_t v;
        float s[4];
    } d;
    /* If the FMA instructions aren't enabled, this will fail to link */
    d.v = vfmaq_f32(c, b, a);

    CORRADE_COMPARE(d.s[0], 0.0f);
    CORRADE_COMPARE(d.s[1], 6.75f);
    CORRADE_COMPARE(d.s[2], 3.25f);
    CORRADE_COMPARE(d.s[3], -0.4f);
    return d.s[2];
}
#else
int callInstructionMultiple() {
    CORRADE_SKIP("Not enough CORRADE_ENABLE_ macros defined");
}
#endif

void CpuTest::enableMacrosMultiple() {
    CORRADE_VERIFY(callInstructionMultiple());
}

/* If the ENABLE_ macro is empty, it should not result in any __attribute__
   annotation */
#ifdef CORRADE_TARGET_SSE2
CORRADE_ENABLE(SSE2) int callInstructionMultipleAllEmpty() {
    return 1;
}
#elif defined(CORRADE_TARGET_NEON)
CORRADE_ENABLE(NEON) int callInstructionMultipleAllEmpty() {
    return 1;
}
#elif defined(CORRADE_TARGET_SIMD128)
CORRADE_ENABLE(SIMD128) int callInstructionMultipleAllEmpty() {
    return 1;
}
#else
int callInstructionMultipleAllEmpty() {
    CORRADE_SKIP("No suitable CORRADE_TARGET_ macro defined");
}
#endif

void CpuTest::enableMacrosMultipleAllEmpty() {
    CORRADE_COMPARE(callInstructionMultipleAllEmpty(), 1);
}

/* On Clang it's enough to have the ENABLE macro just on the wrapper function.
   On GCC it has to be attached to the lambda due to
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80439. However, older versions
   of Clang suffer from the inverse problem and ignore lambda attributes, so it
   has to stay on the function definition as well. Furthermore, if the trailing
   return type is uncommented, the code will fail to compile on GCC 9.1 to 9.3:
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90333 */
#ifdef CORRADE_ENABLE_AVX2
CORRADE_ENABLE_AVX2 int callInstructionLambda() {
  return []() CORRADE_ENABLE_AVX2 /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Avx2))
        CORRADE_SKIP("AVX2 feature not supported");

    /* Same as callInstructionFor<Cpu::Avx2T>() */
    __m256i a = _mm256_set_epi64x(0x8080808080808080ull, 0, 0x8080808080808080ull, 0);

    /* If the AVX2 instructions aren't enabled, this will fail to link */
    int mask = _mm256_movemask_epi8(a);

    CORRADE_COMPARE(mask, 0xff00ff00);
    return mask;
  }();
}
#elif defined(CORRADE_ENABLE_NEON)
CORRADE_ENABLE_NEON int callInstructionLambda() {
  return []() CORRADE_ENABLE_NEON /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Neon))
        CORRADE_SKIP("NEON feature not supported");

    /* Same as callInstructionFor<Cpu::NeonT>() */
    int32x4_t a{-10, 20, -30, 40};

    union {
        int32x4_t v;
        int s[8];
    } b;
    b.v = vabsq_s32(a);

    CORRADE_COMPARE(b.s[0], 10);
    CORRADE_COMPARE(b.s[1], 20);
    CORRADE_COMPARE(b.s[2], 30);
    CORRADE_COMPARE(b.s[3], 40);
    return b.s[0];
  }();
}
#elif defined(CORRADE_ENABLE_SIMD128)
CORRADE_ENABLE_SIMD128 int callInstructionLambda() {
  return []() CORRADE_ENABLE_SIMD128 /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Simd128))
        CORRADE_SKIP("SIMD128 feature not supported");

    /* Same as callInstructionFor<Cpu::Simd128T>() */
    v128_t a = wasm_f32x4_make(5.47, 2.23, 7.62, 0.5);

    union {
        v128_t v;
        float s[8];
    } b;
    b.v = wasm_f32x4_ceil(a);

    CORRADE_COMPARE(b.s[0], 6.0);
    CORRADE_COMPARE(b.s[1], 3.0);
    CORRADE_COMPARE(b.s[2], 8.0);
    CORRADE_COMPARE(b.s[3], 1.0);
    return b.s[0];
  }();
}
#else
int callInstructionLambda() {
    CORRADE_SKIP("No usable CORRADE_ENABLE_ macros defined");
}
#endif

void CpuTest::enableMacrosLambda() {
    /* Verifies that CORRADE_ENABLE_* can be applied also to lambdas. See the
       comment above the implementations for more information. */
    CORRADE_VERIFY(callInstructionLambda());
}

/* Same as callInstructionLambda(), just with CORRADE_ENABLE(*) instead of
   CORRADE_ENABLE_* */
#ifdef CORRADE_ENABLE_AVX2
CORRADE_ENABLE(AVX2) int callInstructionLambdaMultiple() {
  return []() CORRADE_ENABLE(AVX2) /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Avx2))
        CORRADE_SKIP("AVX2 feature not supported");

    /* Same as callInstructionFor<Cpu::Avx2T>() */
    __m256i a = _mm256_set_epi64x(0x8080808080808080ull, 0, 0x8080808080808080ull, 0);

    /* If the AVX2 instructions aren't enabled, this will fail to link */
    int mask = _mm256_movemask_epi8(a);

    CORRADE_COMPARE(mask, 0xff00ff00);
    return mask;
  }();
}
#elif defined(CORRADE_ENABLE_NEON)
CORRADE_ENABLE(NEON) int callInstructionLambdaMultiple() {
  return []() CORRADE_ENABLE(NEON) /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Neon))
        CORRADE_SKIP("NEON feature not supported");

    /* Same as callInstructionFor<Cpu::NeonT>() */
    int32x4_t a{-10, 20, -30, 40};

    union {
        int32x4_t v;
        int s[8];
    } b;
    b.v = vabsq_s32(a);

    CORRADE_COMPARE(b.s[0], 10);
    CORRADE_COMPARE(b.s[1], 20);
    CORRADE_COMPARE(b.s[2], 30);
    CORRADE_COMPARE(b.s[3], 40);
    return b.s[0];
  }();
}
#elif defined(CORRADE_ENABLE_SIMD128)
CORRADE_ENABLE(SIMD128) int callInstructionLambdaMultiple() {
  return []() CORRADE_ENABLE(SIMD128) /*-> int*/ {
    if(!(Cpu::runtimeFeatures() & Cpu::Simd128))
        CORRADE_SKIP("SIMD128 feature not supported");

    /* Same as callInstructionFor<Cpu::Simd128T>() */
    v128_t a = wasm_f32x4_make(5.47, 2.23, 7.62, 0.5);

    union {
        v128_t v;
        float s[8];
    } b;
    b.v = wasm_f32x4_ceil(a);

    CORRADE_COMPARE(b.s[0], 6.0);
    CORRADE_COMPARE(b.s[1], 3.0);
    CORRADE_COMPARE(b.s[2], 8.0);
    CORRADE_COMPARE(b.s[3], 1.0);
    return b.s[0];
  }();
}
#else
int callInstructionLambdaMultiple() {
    CORRADE_SKIP("No usable CORRADE_ENABLE_ macros defined");
}
#endif

void CpuTest::enableMacrosLambdaMultiple() {
    /* Verifies that CORRADE_ENABLE(*) (the function macro variant) can be
       applied to lambdas as well */
    CORRADE_VERIFY(callInstructionLambdaMultiple());
}

void CpuTest::debug() {
    /* If more flags get added, this might need to become even more zeros */
    const unsigned int dead = 0xde00ad00;

    /* Features{} are equivalent to Scalar */
    #ifdef CORRADE_TARGET_X86
    std::ostringstream out;
    Debug{&out} << Cpu::Scalar << (Cpu::Avx2|Cpu::Ssse3|Cpu::Sse41) << Cpu::Features{} << (reinterpret_cast<const Cpu::Features&>(dead)|Cpu::Avx512f) << reinterpret_cast<const Cpu::Features&>(dead);
    CORRADE_COMPARE(out.str(), "Cpu::Scalar Cpu::Ssse3|Cpu::Sse41|Cpu::Avx2 Cpu::Scalar Cpu::Avx512f|Cpu::Features(0xde00ad00) Cpu::Features(0xde00ad00)\n");
    #elif defined(CORRADE_TARGET_ARM)
    std::ostringstream out;
    Debug{&out} << Cpu::Scalar << (Cpu::NeonFp16|Cpu::NeonFma|Cpu::Neon) << Cpu::Features{} << (reinterpret_cast<const Cpu::Features&>(dead)|Cpu::NeonFma) << reinterpret_cast<const Cpu::Features&>(dead);
    CORRADE_COMPARE(out.str(), "Cpu::Scalar Cpu::Neon|Cpu::NeonFma|Cpu::NeonFp16 Cpu::Scalar Cpu::NeonFma|Cpu::Features(0xde00ad00) Cpu::Features(0xde00ad00)\n");
    #else
    static_cast<void>(dead);
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::debugPacked() {
    /* If more flags get added, this might need to become even more zeros */
    const unsigned int dead = 0xde00ad00;

    /* Features{} are equivalent to Scalar */
    #ifdef CORRADE_TARGET_X86
    std::ostringstream out;
    Debug{&out} << Debug::packed << Cpu::Scalar << Debug::packed << (Cpu::Avx2|Cpu::Ssse3|Cpu::Sse41) << Debug::packed << Cpu::Features{} << Debug::packed << (reinterpret_cast<const Cpu::Features&>(dead)|Cpu::Avx512f) << Debug::packed << reinterpret_cast<const Cpu::Features&>(dead) << Cpu::Avx;
    CORRADE_COMPARE(out.str(), "Scalar Ssse3|Sse41|Avx2 Scalar Avx512f|0xde00ad00 0xde00ad00 Cpu::Avx\n");
    #elif defined(CORRADE_TARGET_ARM)
    std::ostringstream out;
    Debug{&out} << Debug::packed << Cpu::Scalar << Debug::packed << (Cpu::NeonFp16|Cpu::NeonFma|Cpu::Neon) << Debug::packed << Cpu::Features{} << Debug::packed << (reinterpret_cast<const Cpu::Features&>(dead)|Cpu::NeonFma) << Debug::packed << reinterpret_cast<const Cpu::Features&>(dead) << Cpu::NeonFma;
    CORRADE_COMPARE(out.str(), "Scalar Neon|NeonFma|NeonFp16 Scalar NeonFma|0xde00ad00 0xde00ad00 Cpu::NeonFma\n");
    #else
    static_cast<void>(dead);
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

}}}

CORRADE_TEST_MAIN(Corrade::Test::CpuTest)
