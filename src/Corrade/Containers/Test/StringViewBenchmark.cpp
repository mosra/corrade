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

#include <cstring>
#include <string>

#include "Corrade/Cpu.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Test/cpuVariantHelpers.h"

#include "configure.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StringViewBenchmark: TestSuite::Tester {
    explicit StringViewBenchmark();

    void captureImplementations();
    void restoreImplementations();

    /* The "Common" variants test rather the call / preamble / postamble
       overhead, while the "Rare" variants test the actual vectorized
       implementation perf */

    void findCharacterCommon();
    void findCharacterCommonNaive();
    void findCharacterCommonMemchr();
    void findCharacterCommonStlString();

    void findCharacterCommonSmall();
    void findCharacterCommonSmallMemchr();
    /* No std::string variant as the overhead from slicing would make this
       useless (and no, find() has no end position) */

    void findCharacterRare();
    void findCharacterRareNaive();
    void findCharacterRareMemchr();
    void findCharacterRareStlString();

    void findLastCharacterCommon();
    void findLastCharacterCommonNaive();
    void findLastCharacterCommonMemrchr();
    void findLastCharacterCommonStlString();

    void findLastCharacterCommonSmall();
    void findLastCharacterCommonSmallMemrchr();
    /* No std::string variant as the overhead from slicing would make this
       useless (and no, rfind() has no end position) */

    void findLastCharacterRare();
    void findLastCharacterRareNaive();
    void findLastCharacterRareMemrchr();
    void findLastCharacterRareStlString();

    private:
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(Implementation::stringFindCharacter) findCharacterImplementation;
        #endif
};

using namespace Containers::Literals;

const struct {
    Cpu::Features features;
} FindCharacterData[]{
    {Cpu::Scalar},
    #if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Sse2|Cpu::Bmi1},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Avx2|Cpu::Bmi1},
    #endif
    #ifdef CORRADE_ENABLE_NEON
    {Cpu::Neon},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t size;
} FindCharacterSmallData[]{
    {Cpu::Scalar, 15},
    #if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Sse2|Cpu::Bmi1, 15},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Avx2|Cpu::Bmi1, 15},
    {Cpu::Avx2|Cpu::Bmi1, 31},
    #endif
    #ifdef CORRADE_ENABLE_NEON
    {Cpu::Neon, 15},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128, 15},
    #endif
};

StringViewBenchmark::StringViewBenchmark() {
    addInstancedBenchmarks({&StringViewBenchmark::findCharacterCommon}, 100,
        Utility::Test::cpuVariantCount(FindCharacterData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks({&StringViewBenchmark::findCharacterCommonNaive,
                   &StringViewBenchmark::findCharacterCommonMemchr,
                   &StringViewBenchmark::findCharacterCommonStlString}, 100);

    addInstancedBenchmarks({&StringViewBenchmark::findCharacterCommonSmall}, 100,
        Utility::Test::cpuVariantCount(FindCharacterSmallData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks({&StringViewBenchmark::findCharacterCommonSmallMemchr}, 100);

    addInstancedBenchmarks({&StringViewBenchmark::findCharacterRare}, 100,
        Utility::Test::cpuVariantCount(FindCharacterData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks({&StringViewBenchmark::findCharacterRareNaive,
                   &StringViewBenchmark::findCharacterRareMemchr,
                   &StringViewBenchmark::findCharacterRareStlString,

                   &StringViewBenchmark::findLastCharacterCommon,
                   &StringViewBenchmark::findLastCharacterCommonNaive,
                   &StringViewBenchmark::findLastCharacterCommonMemrchr,
                   &StringViewBenchmark::findLastCharacterCommonStlString,

                   &StringViewBenchmark::findLastCharacterCommonSmall,
                   &StringViewBenchmark::findLastCharacterCommonSmallMemrchr,

                   &StringViewBenchmark::findLastCharacterRare,
                   &StringViewBenchmark::findLastCharacterRareNaive,
                   &StringViewBenchmark::findLastCharacterRareMemrchr,
                   &StringViewBenchmark::findLastCharacterRareStlString}, 100);
}

void StringViewBenchmark::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    findCharacterImplementation = Implementation::stringFindCharacter;
    #endif
}

void StringViewBenchmark::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    Implementation::stringFindCharacter = findCharacterImplementation;
    #endif
}

constexpr std::size_t CommonCharacterCount = 500;
constexpr std::size_t RareCharacterCount = 90;
constexpr std::size_t CharacterRepeats = 100;

void StringViewBenchmark::findCharacterCommon() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = FindCharacterData[testCaseInstanceId()];
    Implementation::stringFindCharacter = Implementation::stringFindCharacterImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(FindCharacterData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        while(StringView found = a.find(' ')) {
            ++count;
            a = a.suffix(found.end());
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonNaive() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = text->data();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = a; i != text->end(); ++i) {
                if(*i == ' ') {
                    found = i;
                    break;
                }
            }
            if(!found) break;

            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonMemchr() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = text->data();
        while(const char* found = static_cast<const char*>(std::memchr(a, ' ', text->end() - a))) {
            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonStlString() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    std::string a = *text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t pos = 0;
        std::size_t found;
        while((found = a.find(' ', pos)) != std::string::npos) {
            ++count;
            pos = found + 1;
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = FindCharacterSmallData[testCaseInstanceId()];
    Implementation::stringFindCharacter = Implementation::stringFindCharacterImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(FindCharacterSmallData);
    #endif
    setTestCaseDescription(Utility::format("{}, {} bytes", Utility::Test::cpuVariantName(data), data.size));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        while(StringView found = a.prefix(Utility::min(data.size, a.size())).find(' ')) {
            ++count;
            a = a.suffix(found.end());
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonSmallMemchr() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = text->data();
        while(const char* found = static_cast<const char*>(std::memchr(a, ' ', Utility::min(std::ptrdiff_t{15}, text->end() - a)))) {
            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterRare() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = FindCharacterData[testCaseInstanceId()];
    Implementation::stringFindCharacter = Implementation::stringFindCharacterImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(FindCharacterData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        while(StringView found = a.find('\n')) {
            ++count;
            a = a.suffix(found.end());
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterRareNaive() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = text->data();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = a; i != text->end(); ++i) {
                if(*i == '\n') {
                    found = i;
                    break;
                }
            }
            if(!found) break;

            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterRareMemchr() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = text->data();
        while(const char* found = static_cast<const char*>(std::memchr(a, '\n', text->end() - a))) {
            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findCharacterRareStlString() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    std::string a = *text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t pos = 0;
        std::size_t found;
        while((found = a.find('\n', pos)) != std::string::npos) {
            ++count;
            pos = found + 1;
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommon() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        while(StringView found = a.findLast(' ')) {
            ++count;
            a = a.prefix(found.begin());
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonNaive() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = text->begin() + end; i != text->begin(); --i) {
                if(*(i - 1) == ' ') {
                    found = i - 1;
                    break;
                }
            }
            if(!found) break;

            ++count;
            end = found - text->begin();
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonMemrchr() {
    #if !defined(__GLIBC__) && !defined(__BIONIC__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("memrchr() not available");
    #else
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        while(const char* found = static_cast<const char*>(memrchr(text->begin(), ' ', end))) {
            ++count;
            end = found - text->begin();
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
    #endif
}

void StringViewBenchmark::findLastCharacterCommonStlString() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    std::string a = *text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        std::size_t found;
        while((found = a.rfind(' ', end)) != std::string::npos) {
            ++count;
            end = found - 1;
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonSmall() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        /** @todo use suffix() once it takes suffix size */
        while(StringView found = a.exceptPrefix(Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(a.size()) - 15)).findLast(' ')) {
            ++count;
            a = a.prefix(found.begin());
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonSmallMemrchr() {
    #if !defined(__GLIBC__) && !defined(__BIONIC__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("memrchr() not available");
    #else
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        while(const char* found = static_cast<const char*>(memrchr(text->begin() + Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(end) - 15), ' ', end - Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(end) - 15)))) {
            ++count;
            end = found - text->begin();
        }
    }

    CORRADE_COMPARE(count, CommonCharacterCount*CharacterRepeats);
    #endif
}

void StringViewBenchmark::findLastCharacterRare() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *text;
        while(StringView found = a.findLast('\n')) {
            ++count;
            a = a.prefix(found.begin());
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterRareNaive() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = text->begin() + end; i != text->begin(); --i) {
                if(*(i - 1) == '\n') {
                    found = i - 1;
                    break;
                }
            }
            if(!found) break;

            ++count;
            end = found - text->begin();
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterRareMemrchr() {
    #if !defined(__GLIBC__) && !defined(__BIONIC__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("memrchr() not available");
    #else
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        while(const char* found = static_cast<const char*>(memrchr(text->begin(), '\n', end))) {
            ++count;
            end = found - text->begin();
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
    #endif
}

void StringViewBenchmark::findLastCharacterRareStlString() {
    Containers::Optional<Containers::String> text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
    CORRADE_VERIFY(text);
    *text = *text*10;

    std::size_t count = 0;
    std::string a = *text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = text->size();
        std::size_t found;
        while((found = a.rfind('\n', end)) != std::string::npos) {
            ++count;
            end = found - 1;
        }
    }

    CORRADE_COMPARE(count, RareCharacterCount*CharacterRepeats);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringViewBenchmark)
