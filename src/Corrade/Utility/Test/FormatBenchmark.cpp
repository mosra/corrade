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

#include <sstream>

#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Format.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct FormatBenchmark: TestSuite::Tester {
    explicit FormatBenchmark();

    void format();
    void snprintf();
    void sstream();
    void debugSstream();
    void debugString();

    void floatFormat();
    void floatSnprintf();
    void floatSstream();
    void floatDebugSstream();
    void floatDebugString();
};

FormatBenchmark::FormatBenchmark() {
    addBenchmarks({&FormatBenchmark::format,
                   &FormatBenchmark::snprintf,
                   &FormatBenchmark::sstream,
                   &FormatBenchmark::debugSstream,
                   &FormatBenchmark::debugString,

                   &FormatBenchmark::floatFormat,
                   &FormatBenchmark::floatSnprintf,
                   &FormatBenchmark::floatSstream,
                   &FormatBenchmark::floatDebugSstream,
                   &FormatBenchmark::floatDebugString}, 50);
}

void FormatBenchmark::format() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        formatInto(buffer, "hello, {}! {1} + {2} = {} = {2} + {1}", "people", 42, 1337, 42 + 1337);

    CORRADE_COMPARE(Containers::StringView{buffer}, "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatBenchmark::snprintf() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        std::snprintf(buffer, 1024, "hello, %s! %i + %i = %i = %i + %i",
            "people", 42, 1337, 42 + 1337, 1337, 42);

    CORRADE_COMPARE(Containers::StringView{buffer}, "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatBenchmark::sstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        out << "hello, " << "people" << "! " << 42 << " + " << 1337 << " = "
            << 42 + 1337 << " = " << 1337 << " + " << 42;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatBenchmark::debugSstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 42 << "+"
            << 1337 << "=" << 42 + 1337 << "=" << 1337 << "+" << 42;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatBenchmark::debugString() {
    Containers::String out;

    CORRADE_BENCHMARK(1000) {
        out = {};
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 42 << "+"
            << 1337 << "=" << 42 + 1337 << "=" << 1337 << "+" << 42;
    }

    CORRADE_COMPARE(out, "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatBenchmark::floatFormat() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        formatInto(buffer, "hello, {}! {1} + {2} = {} = {2} + {1}", "people", 4.2, 13.37, 4.2 + 13.37);

    CORRADE_COMPARE(Containers::StringView{buffer}, "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatBenchmark::floatSnprintf() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        std::snprintf(buffer, 1024, "hello, %s! %g + %g = %g = %g + %g",
            "people", 4.2, 13.37, 4.2 + 13.37, 13.37, 4.2);

    CORRADE_COMPARE(Containers::StringView{buffer}, "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatBenchmark::floatSstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        out << "hello, " << "people" << "! " << 4.2 << " + " << 13.37 << " = "
            << 4.2 + 13.37 << " = " << 13.37 << " + " << 4.2;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatBenchmark::floatDebugSstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 4.2 << "+"
            << 13.37 << "=" << 4.2 + 13.37 << "=" << 13.37 << "+" << 4.2;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatBenchmark::floatDebugString() {
    Containers::String out;

    CORRADE_BENCHMARK(1000) {
        out = {};
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 4.2 << "+"
            << 13.37 << "=" << 4.2 + 13.37 << "=" << 13.37 << "+" << 4.2;
    }

    CORRADE_COMPARE(out, "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::FormatBenchmark)
