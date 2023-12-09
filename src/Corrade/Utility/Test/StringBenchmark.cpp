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

#include <cctype> /* std::ctype */
#include <algorithm> /* std::transform() */
#include <locale> /* std::locale::classic() */

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/String.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringBenchmark: TestSuite::Tester {
    explicit StringBenchmark();

    void lowercase();
    void lowercaseBranchless32();
    void lowercaseNaive();
    void lowercaseStl();
    void lowercaseStlFacet();

    void uppercase();
    void uppercaseBranchless32();
    void uppercaseNaive();
    void uppercaseStl();
    void uppercaseStlFacet();

    private:
        Containers::Optional<Containers::String> _text;
};

using namespace Containers::Literals;

StringBenchmark::StringBenchmark() {
    addBenchmarks({&StringBenchmark::lowercase,
                   &StringBenchmark::lowercaseBranchless32,
                   &StringBenchmark::lowercaseNaive,
                   &StringBenchmark::lowercaseStl,
                   &StringBenchmark::lowercaseStlFacet,

                   &StringBenchmark::uppercase,
                   &StringBenchmark::uppercaseBranchless32,
                   &StringBenchmark::uppercaseNaive,
                   &StringBenchmark::uppercaseStl,
                   &StringBenchmark::uppercaseStlFacet}, 10);

    _text = Path::readString(Path::join(CONTAINERS_STRING_TEST_DIR, "lorem-ipsum.txt"));
}

void StringBenchmark::lowercase() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        String::lowercaseInPlace(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

/* Compared to the implementation in String::lowercaseInPlace(), this uses
   `unsigned` instead of `std::uint8_t`, making it almost 8x slower. Not sure
   why, heh. */
CORRADE_NEVER_INLINE void lowercaseInPlaceBranchless32(Containers::MutableStringView string) {
    for(char& c: string)
        c += (unsigned(c - 'A') < 26) << 5;
}

void StringBenchmark::lowercaseBranchless32() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        lowercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

/* This is the original implementation that used to be in
   String::lowercaseInPlace() */
CORRADE_NEVER_INLINE void lowercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string)
        if(c >= 'A' && c <= 'Z') c |= 0x20;
}

void StringBenchmark::lowercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        lowercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::tolower));
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).tolower(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::uppercase() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        String::uppercaseInPlace(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

/* Compared to the implementation in String::lowercaseInPlace(), this uses
   `unsigned` instead of `std::uint8_t`, making it almost 8x slower. Not sure
   why, heh. */
CORRADE_NEVER_INLINE void uppercaseInPlaceBranchless32(Containers::MutableStringView string) {
    for(char& c: string)
        c -= (unsigned(c - 'a') < 26) << 5;
}

void StringBenchmark::uppercaseBranchless32() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        uppercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

/* This is the original implementation that used to be in
   String::uppercaseInPlace() */
CORRADE_NEVER_INLINE void uppercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string)
        if(c >= 'a' && c <= 'z') c &= ~0x20;
}

void StringBenchmark::uppercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        uppercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::toupper));
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).toupper(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringBenchmark)
