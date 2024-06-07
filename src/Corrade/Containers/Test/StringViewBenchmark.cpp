/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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
#include "Corrade/Containers/StaticArray.h"
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

    template<char character> void findCharacter();
    template<char character> void findCharacterNaive();
    template<char character> void findCharacterMemchr();
    template<char character> void findCharacterStlString();

    void findCharacterCommonSmall();
    void findCharacterCommonSmallMemchr();
    /* No std::string variant as the overhead from slicing would make this
       useless (and no, find() has no end position) */

    template<char character> void findLastCharacter();
    template<char character> void findLastCharacterNaive();
    template<char character> void findLastCharacterMemrchr();
    template<char character> void findLastCharacterStrrchr();
    template<char character> void findLastCharacterStlString();

    void findLastCharacterCommonSmall();
    void findLastCharacterCommonSmallMemrchr();
    /* No std::string variant as the overhead from slicing would make this
       useless (and no, rfind() has no end position) */

    private:
        Containers::Optional<Containers::String> _text;
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(Implementation::stringFindCharacter) _findCharacterImplementation;
        #endif
};

using namespace Containers::Literals;

template<char> struct CharacterTraits;
template<> struct CharacterTraits<' '> {
    enum: std::size_t { Count = 500 };
    static const char* name() { return "common"; }
};
template<> struct CharacterTraits<'\n'> {
    enum: std::size_t { Count = 9 };
    static const char* name() { return "rare"; }
};

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
    /* The code uses ARM64 NEON instructions. 32-bit ARM isn't that important
       nowadays, so there it uses scalar code */
    #if defined(CORRADE_ENABLE_NEON) && !defined(CORRADE_TARGET_32BIT)
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
    /* This should fall back to the scalar case */
    {Cpu::Sse2|Cpu::Bmi1, 15},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Sse2|Cpu::Bmi1, 16},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Sse2|Cpu::Bmi1, 17},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    /* This should fall back to the SSE2 and then the scalar case */
    {Cpu::Avx2|Cpu::Bmi1, 15},
    /* This should fall back to the SSE2 case */
    {Cpu::Avx2|Cpu::Bmi1, 31},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Avx2|Cpu::Bmi1, 32},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Avx2|Cpu::Bmi1, 33},
    #endif
    /* The code uses ARM64 NEON instructions. 32-bit ARM isn't that important
       nowadays, so there it uses scalar code */
    #if defined(CORRADE_ENABLE_NEON) && !defined(CORRADE_TARGET_32BIT)
    /* This should fall back to the scalar case */
    {Cpu::Neon, 15},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Neon, 16},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Neon, 17},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    /* This should fall back to the scalar case */
    {Cpu::Simd128, 15},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Simd128, 16},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Simd128, 17},
    #endif
    /** @todo also the cases with either one aligned four-vector block or four
        aligned single-vector postambles, needs to figure out how it would
        behave re alignment tho */
};

StringViewBenchmark::StringViewBenchmark() {
    addInstancedBenchmarks({&StringViewBenchmark::findCharacter<' '>}, 100,
        Utility::Test::cpuVariantCount(FindCharacterData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks<StringViewBenchmark>({
        &StringViewBenchmark::findCharacterNaive<' '>,
        &StringViewBenchmark::findCharacterMemchr<' '>,
        &StringViewBenchmark::findCharacterStlString<' '>}, 20);

    addInstancedBenchmarks({&StringViewBenchmark::findCharacterCommonSmall}, 100,
        Utility::Test::cpuVariantCount(FindCharacterSmallData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks({&StringViewBenchmark::findCharacterCommonSmallMemchr}, 20);

    addInstancedBenchmarks({&StringViewBenchmark::findCharacter<'\n'>}, 100,
        Utility::Test::cpuVariantCount(FindCharacterData),
        &StringViewBenchmark::captureImplementations,
        &StringViewBenchmark::restoreImplementations);

    addBenchmarks({&StringViewBenchmark::findCharacterNaive<'\n'>,
                   &StringViewBenchmark::findCharacterMemchr<'\n'>,
                   &StringViewBenchmark::findCharacterStlString<'\n'>,

                   &StringViewBenchmark::findLastCharacter<' '>,
                   &StringViewBenchmark::findLastCharacterNaive<' '>,
                   &StringViewBenchmark::findLastCharacterMemrchr<' '>,
                   &StringViewBenchmark::findLastCharacterStrrchr<' '>,
                   &StringViewBenchmark::findLastCharacterStlString<' '>,

                   &StringViewBenchmark::findLastCharacterCommonSmall,
                   &StringViewBenchmark::findLastCharacterCommonSmallMemrchr,

                   &StringViewBenchmark::findLastCharacter<'\n'>,
                   &StringViewBenchmark::findLastCharacterNaive<'\n'>,
                   &StringViewBenchmark::findLastCharacterMemrchr<'\n'>,
                   &StringViewBenchmark::findLastCharacterStrrchr<'\n'>,
                   &StringViewBenchmark::findLastCharacterStlString<'\n'>}, 20);

    _text = Utility::Path::readString(Utility::Path::join(CONTAINERS_TEST_DIR, "StringTestFiles/lorem-ipsum.txt"));
}

void StringViewBenchmark::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    _findCharacterImplementation = Implementation::stringFindCharacter;
    #endif
}

void StringViewBenchmark::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    Implementation::stringFindCharacter = _findCharacterImplementation;
    #endif
}

constexpr std::size_t CharacterRepeats = 100;

template<char character> void StringViewBenchmark::findCharacter() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = FindCharacterData[testCaseInstanceId()];
    Implementation::stringFindCharacter = Implementation::stringFindCharacterImplementation(data.features);
    #else
    auto&& data = Utility::Test::cpuVariantCompiled(FindCharacterData);
    #endif
    setTestCaseDescription(Utility::format("{}, {}",
        CharacterTraits<character>::name(),
        Utility::Test::cpuVariantName(data)));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *_text;
        while(StringView found = a.find(character)) {
            ++count;
            a = a.suffix(found.end());
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findCharacterNaive() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = _text->data();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = a; i != _text->end(); ++i) {
                if(*i == character) {
                    found = i;
                    break;
                }
            }
            if(!found) break;

            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findCharacterMemchr() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = _text->data();
        while(const char* found = static_cast<const char*>(std::memchr(a, character, _text->end() - a))) {
            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findCharacterStlString() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    std::string a = *_text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t pos = 0;
        std::size_t found;
        while((found = a.find(character, pos)) != std::string::npos) {
            ++count;
            pos = found + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
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

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *_text;
        while(StringView found = a.prefix(Utility::min(data.size, a.size())).find(' ')) {
            ++count;
            a = a.suffix(found.end());
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
}

void StringViewBenchmark::findCharacterCommonSmallMemchr() {
    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        const char* a = _text->data();
        while(const char* found = static_cast<const char*>(std::memchr(a, ' ', Utility::min(std::ptrdiff_t{15}, _text->end() - a)))) {
            ++count;
            a = found + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findLastCharacter() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *_text;
        while(StringView found = a.findLast(character)) {
            ++count;
            a = a.prefix(found.begin());
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findLastCharacterNaive() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = _text->size();
        for(;;) {
            const char* found = nullptr;
            for(const char* i = _text->begin() + end; i != _text->begin(); --i) {
                if(*(i - 1) == character) {
                    found = i - 1;
                    break;
                }
            }
            if(!found) break;

            ++count;
            end = found - _text->begin();
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findLastCharacterMemrchr() {
    setTestCaseDescription(CharacterTraits<character>::name());

    #if !defined(__GLIBC__) && !defined(__BIONIC__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("memrchr() not available");
    #else
    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = _text->size();
        while(const char* found = static_cast<const char*>(memrchr(_text->begin(), character, end))) {
            ++count;
            end = found - _text->begin();
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
    #endif
}

template<char character> void StringViewBenchmark::findLastCharacterStrrchr() {
    setTestCaseDescription(CharacterTraits<character>::name());

    /* Just for laughs -- as there isn't really a way for strrchr to start
       looking at the *end* of the string, it has to go through the whole
       string every time. To actually end up finding all occurences, every time
       a occurence is found, it's converted to a null terminator, which means
       we need a new copy of the string for every benchmark iteration.

       It's funny how this function ended up being in the standard C but
       memrchr not. */

    CORRADE_VERIFY(_text);

    StaticArray<CharacterRepeats, String> strings{Corrade::DirectInit, *_text};

    std::size_t count = 0;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        while(char* found = strrchr(strings[i].begin(), character)) {
            ++count;
            *found = '\0';
        }
        ++i;
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringViewBenchmark::findLastCharacterStlString() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    std::string a = *_text;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = _text->size();
        std::size_t found;
        while((found = a.rfind(character, end)) != std::string::npos) {
            ++count;
            end = found - 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonSmall() {
    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        StringView a = *_text;
        /** @todo use suffix() once it takes suffix size */
        while(StringView found = a.exceptPrefix(Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(a.size()) - 15)).findLast(' ')) {
            ++count;
            a = a.prefix(found.begin());
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
}

void StringViewBenchmark::findLastCharacterCommonSmallMemrchr() {
    #if !defined(__GLIBC__) && !defined(__BIONIC__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("memrchr() not available");
    #else
    CORRADE_VERIFY(_text);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        std::size_t end = _text->size();
        while(const char* found = static_cast<const char*>(memrchr(_text->begin() + Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(end) - 15), ' ', end - Utility::max(std::ptrdiff_t{0}, std::ptrdiff_t(end) - 15)))) {
            ++count;
            end = found - _text->begin();
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StringViewBenchmark)
