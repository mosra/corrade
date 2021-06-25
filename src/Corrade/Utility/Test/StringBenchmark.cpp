/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <cctype>
#include <algorithm>
#include <locale>
#include <string>

#include "Corrade/Containers/StringStl.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringBenchmark: TestSuite::Tester {
    explicit StringBenchmark();

    void lowercase();
    void lowercaseStl();
    void lowercaseStlFacet();

    void uppercase();
    void uppercaseStl();
    void uppercaseStlFacet();
};

using namespace Containers::Literals;

constexpr Containers::StringView loremIpsum =
    "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis viverra diam non justo. Integer pellentesque quam vel velit. Pellentesque pretium lectus id turpis. Fusce suscipit libero eget elit. Vestibulum fermentum tortor id mi. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Nullam sit amet magna in magna gravida vehicula. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos hymenaeos. Donec ipsum massa, ullamcorper in, auctor et, scelerisque sed, est. Nam sed tellus id magna elementum tincidunt.\n"
    "Aliquam erat volutpat. Vivamus ac leo pretium faucibus. Etiam commodo dui eget wisi. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos hymenaeos. Maecenas ipsum velit, consectetuer eu lobortis ut, dictum at dui. Integer imperdiet lectus quis justo. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Integer tempor. Integer rutrum, orci vestibulum ullamcorper ultricies, lacus quam ultricies odio, vitae placerat pede sem sit amet enim. Pellentesque pretium lectus id turpis.\n"
    "Etiam commodo dui eget wisi. Aenean id metus id velit ullamcorper pulvinar. Etiam commodo dui eget wisi. Nullam sit amet magna in magna gravida vehicula. Nulla est. Duis sapien nunc, commodo et, interdum suscipit, sollicitudin et, dolor. Nullam lectus justo, vulputate eget mollis sed, tempor sed magna. Aliquam erat volutpat. Integer rutrum, orci vestibulum ullamcorper ultricies, lacus quam ultricies odio, vitae placerat pede sem sit amet enim. Proin in tellus sit amet nibh dignissim sagittis. Cras elementum. In enim a arcu imperdiet malesuada. Nulla turpis magna, cursus sit amet, suscipit a, interdum id, felis. Nam sed tellus id magna elementum tincidunt. Et harum quidem rerum facilis est et expedita distinctio. Nunc auctor. Aliquam erat volutpat. Pellentesque sapien. Nulla quis diam. Pellentesque arcu.\n"
    "Fusce wisi. Mauris elementum mauris vitae tortor. Etiam bibendum elit eget erat. Curabitur sagittis hendrerit ante. Fusce tellus. Aenean vel massa quis mauris vehicula lacinia. Aenean id metus id velit ullamcorper pulvinar. Etiam posuere lacus quis dolor. Fusce tellus odio, dapibus id fermentum quis, suscipit id erat. Duis bibendum, lectus ut viverra rhoncus, dolor nunc faucibus libero, eget facilisis enim ipsum id lacus. Integer lacinia. Pellentesque sapien. Duis bibendum, lectus ut viverra rhoncus, dolor nunc faucibus libero, eget facilisis enim ipsum id lacus. Curabitur bibendum justo non orci.\n"
    "Praesent id justo in neque elementum ultrices. Proin mattis lacinia justo. Duis viverra diam non justo. Mauris dictum facilisis augue. Mauris elementum mauris vitae tortor. Integer malesuada. Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Praesent dapibus. Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Nam quis nulla.\n"_s;

StringBenchmark::StringBenchmark() {
    addBenchmarks({&StringBenchmark::lowercase,
                   &StringBenchmark::lowercaseStl,
                   &StringBenchmark::lowercaseStlFacet,

                   &StringBenchmark::uppercase,
                   &StringBenchmark::uppercaseStl,
                   &StringBenchmark::uppercaseStlFacet}, 100);
}

void StringBenchmark::lowercase() {
    Containers::String string = loremIpsum;

    CORRADE_BENCHMARK(1)
        String::lowercaseInPlace(string);

    CORRADE_VERIFY(!string.contains('L'));
}

void StringBenchmark::lowercaseStl() {
    std::string string = loremIpsum;

    /* C++ experts recommend using a lambda here, even, but that's even more
       stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    CORRADE_BENCHMARK(1)
        std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::tolower));

    CORRADE_VERIFY(!Containers::StringView{string}.contains('L'));
}

void StringBenchmark::lowercaseStlFacet() {
    std::string string = loremIpsum;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    CORRADE_BENCHMARK(1)
        std::use_facet<std::ctype<char>>(std::locale::classic()).tolower(&string[0], &string[string.size()]);

    CORRADE_VERIFY(!Containers::StringView{string}.contains('L'));
}

void StringBenchmark::uppercase() {
    Containers::String string = loremIpsum;

    CORRADE_BENCHMARK(1)
        String::uppercaseInPlace(string);

    CORRADE_VERIFY(!string.contains('a'));
}

void StringBenchmark::uppercaseStl() {
    std::string string = loremIpsum;

    /* C++ experts recommend using a lambda here, even, but that's even more
       stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    CORRADE_BENCHMARK(1)
        std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::toupper));

    CORRADE_VERIFY(!Containers::StringView{string}.contains('a'));
}

void StringBenchmark::uppercaseStlFacet() {
    std::string string = loremIpsum;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    CORRADE_BENCHMARK(1)
        std::use_facet<std::ctype<char>>(std::locale::classic()).toupper(&string[0], &string[string.size()]);

    CORRADE_VERIFY(!Containers::StringView{string}.contains('a'));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringBenchmark)
