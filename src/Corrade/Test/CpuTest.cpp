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

    void tagDispatch();
    template<class T> void enableMacros();

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

    addTests({&CpuTest::tagDispatch,
              #ifdef CORRADE_TARGET_X86
              &CpuTest::enableMacros<Cpu::Sse2T>,
              &CpuTest::enableMacros<Cpu::Sse3T>,
              &CpuTest::enableMacros<Cpu::Ssse3T>,
              &CpuTest::enableMacros<Cpu::Sse41T>,
              &CpuTest::enableMacros<Cpu::Sse42T>,
              &CpuTest::enableMacros<Cpu::AvxT>,
              &CpuTest::enableMacros<Cpu::Avx2T>,
              &CpuTest::enableMacros<Cpu::Avx512fT>,
              #endif

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
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorAnd() {
    #ifdef CORRADE_TARGET_X86
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
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorXor() {
    #ifdef CORRADE_TARGET_X86
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
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorBoolScalar() {
    CORRADE_COMPARE(!!Cpu::Features{Cpu::Scalar}, false);

    constexpr bool cFeatures = !!Cpu::Features{Cpu::Scalar};
    CORRADE_VERIFY(!cFeatures);
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
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::featuresOperatorInverse() {
    #ifdef CORRADE_TARGET_X86
    CORRADE_COMPARE(std::uint32_t(~Cpu::Scalar), 0xffffffffu);
    CORRADE_COMPARE(std::uint32_t(~(Cpu::Sse41|Cpu::Sse3)), 4294967285u);
    CORRADE_COMPARE(std::uint32_t(~Cpu::Sse41), 4294967287u);

    constexpr Cpu::Features cFeatures1 = ~Cpu::Scalar;
    constexpr Cpu::Features cFeatures2 = ~(Cpu::Sse41|Cpu::Sse3);
    CORRADE_COMPARE(std::uint32_t(cFeatures1), 0xffffffffu);
    CORRADE_COMPARE(std::uint32_t(cFeatures2), 4294967285u);
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
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_SSE2 int callInstructionFor<Cpu::Sse2T>() {
    __m128i a = _mm_set_epi32(0x80808080, 0, 0x80808080, 0);

    /* All instructions SSE2 */

    int mask = _mm_movemask_epi8(a);
    CORRADE_COMPARE(mask, 0xf0f0); /* 0b1111000011110000 */
    return mask;
}
#endif
#ifdef CORRADE_ENABLE_SSE3
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_SSE3 int callInstructionFor<Cpu::Sse3T>() {
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
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_SSSE3 int callInstructionFor<Cpu::Ssse3T>() {
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
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_SSE41 int callInstructionFor<Cpu::Sse41T>() {
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
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_SSE42 int callInstructionFor<Cpu::Sse42T>() {
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
#ifdef CORRADE_ENABLE_AVX
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_AVX int callInstructionFor<Cpu::AvxT>() {
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
#ifdef CORRADE_ENABLE_AVX2
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_AVX2 int callInstructionFor<Cpu::Avx2T>() {
    __m256i a = _mm256_set_epi64x(0x8080808080808080ull, 0, 0x8080808080808080ull, 0);

    /* Like callInstructionFor<Cpu::Sse2T>(), but expanded to AVX2 */
    int mask = _mm256_movemask_epi8(a);

    CORRADE_COMPARE(mask, 0xff00ff00);/* 0b11111111000000001111111100000000 */
    return mask;
}
#endif
#ifdef CORRADE_ENABLE_AVX512F
template<> CORRADE_NEVER_INLINE CORRADE_ENABLE_AVX512F int callInstructionFor<Cpu::Avx512fT>() {
    __m128 a = _mm_set1_ps(5.47f);

    /* AVX512 */
    int ceil = _mm_cvt_roundss_si32(a, _MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC);

    CORRADE_COMPARE(ceil, 6);
    return ceil;
}
#endif

template<class T> void CpuTest::enableMacros() {
    setTestCaseTemplateName(Cpu::TypeTraits<T>::name());

    if(!(Cpu::runtimeFeatures() & Cpu::features<T>()))
        CORRADE_SKIP("CPU feature not supported");

    CORRADE_VERIFY(true); /* to capture correct function name */
    CORRADE_VERIFY(callInstructionFor<T>());
}

void CpuTest::debug() {
    /* Features{} are equivalent to Scalar */
    #ifdef CORRADE_TARGET_X86
    std::ostringstream out;
    Debug{&out} << Cpu::Scalar << (Cpu::Avx2|Cpu::Ssse3|Cpu::Sse41) << Cpu::Features{};
    CORRADE_COMPARE(out.str(), "Cpu::Scalar Cpu::Ssse3|Cpu::Sse41|Cpu::Avx2 Cpu::Scalar\n");
    #elif defined(CORRADE_TARGET_ARM)
    std::ostringstream out;
    Debug{&out} << Cpu::Scalar << (Cpu::NeonFp16|Cpu::NeonFma|Cpu::Neon) << Cpu::Features{};
    CORRADE_COMPARE(out.str(), "Cpu::Scalar Cpu::Neon|Cpu::NeonFma|Cpu::NeonFp16 Cpu::Scalar\n");
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

void CpuTest::debugPacked() {
    /* Features{} are equivalent to Scalar */
    #ifdef CORRADE_TARGET_X86
    std::ostringstream out;
    Debug{&out} << Debug::packed << Cpu::Scalar << Debug::packed << (Cpu::Avx2|Cpu::Ssse3|Cpu::Sse41) << Debug::packed << Cpu::Features{} << Cpu::Avx;
    CORRADE_COMPARE(out.str(), "Scalar Ssse3|Sse41|Avx2 Scalar Cpu::Avx\n");
    #elif defined(CORRADE_TARGET_ARM)
    std::ostringstream out;
    Debug{&out} << Debug::packed << Cpu::Scalar << Debug::packed << (Cpu::NeonFp16|Cpu::NeonFma|Cpu::Neon) << Debug::packed << Cpu::Features{} << Cpu::NeonFma;
    CORRADE_COMPARE(out.str(), "Scalar Neon|NeonFma|NeonFp16 Scalar Cpu::NeonFma\n");
    #else
    CORRADE_SKIP("Not enough Cpu tags available on this platform, can't test");
    #endif
}

}}}

CORRADE_TEST_MAIN(Corrade::Test::CpuTest)
