/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/ScopedExit.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Format.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test {

struct FormatTest: TestSuite::Tester {
    explicit FormatTest();

    void empty();
    void textOnly();
    void escapes();

    void integerChar();
    void integerShort();
    void integerInt();
    void integerLong();
    void integerLongLong();

    void floatingFloat();
    void floatingDouble();
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void floatingLongDouble();
    #endif
    template<class T> void floatingPrecision();

    void charArray();
    void charArrayView();
    void string();

    void multiple();
    void numbered();
    void mixed();

    void toBuffer();
    void toBufferNullTerminatorFromSnprintfAtTheEnd();
    void appendToString();
    void insertToString();
    void file();
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void fileLongDouble();
    #endif

    void tooLittlePlaceholders();
    void tooManyPlaceholders();

    void tooSmallBuffer();
    void mismatchedDelimiter();
    void unknownPlaceholderContent();

    void benchmarkFormat();
    void benchmarkSnprintf();
    void benchmarkSstream();
    void benchmarkDebug();

    void benchmarkFloatFormat();
    void benchmarkFloatSnprintf();
    void benchmarkFloatSstream();
    void benchmarkFloatDebug();
};

FormatTest::FormatTest() {
    addTests({&FormatTest::empty,
              &FormatTest::textOnly,
              &FormatTest::escapes,

              &FormatTest::integerChar,
              &FormatTest::integerShort,
              &FormatTest::integerInt,
              &FormatTest::integerLong,
              &FormatTest::integerLongLong,

              &FormatTest::floatingFloat,
              &FormatTest::floatingDouble,
              #ifndef CORRADE_TARGET_EMSCRIPTEN
              &FormatTest::floatingLongDouble,
              #endif
              &FormatTest::floatingPrecision<float>,
              &FormatTest::floatingPrecision<double>,
              #ifndef CORRADE_TARGET_EMSCRIPTEN
              &FormatTest::floatingPrecision<long double>,
              #endif

              &FormatTest::charArray,
              &FormatTest::charArrayView,
              &FormatTest::string,

              &FormatTest::multiple,
              &FormatTest::numbered,
              &FormatTest::mixed,

              &FormatTest::toBuffer,
              &FormatTest::toBufferNullTerminatorFromSnprintfAtTheEnd,
              &FormatTest::appendToString,
              &FormatTest::insertToString,
              &FormatTest::file,
              #ifndef CORRADE_TARGET_EMSCRIPTEN
              &FormatTest::fileLongDouble,
              #endif

              &FormatTest::tooLittlePlaceholders,
              &FormatTest::tooManyPlaceholders,

              &FormatTest::tooSmallBuffer,
              &FormatTest::mismatchedDelimiter,
              &FormatTest::unknownPlaceholderContent});

    addBenchmarks({&FormatTest::benchmarkFormat,
                   &FormatTest::benchmarkSnprintf,
                   &FormatTest::benchmarkSstream,
                   &FormatTest::benchmarkDebug,

                   &FormatTest::benchmarkFloatFormat,
                   &FormatTest::benchmarkFloatSnprintf,
                   &FormatTest::benchmarkFloatSstream,
                   &FormatTest::benchmarkFloatDebug}, 50);
}

void FormatTest::empty() {
    CORRADE_COMPARE(format(""), "");
}

void FormatTest::textOnly() {
    CORRADE_COMPARE(format("hello"), "hello");
}

void FormatTest::escapes() {
    CORRADE_COMPARE(format("typedef struct {{ int a; }} Type;"),
        "typedef struct { int a; } Type;");
}

void FormatTest::integerChar() {
    if(std::is_signed<char>::value) {
        CORRADE_COMPARE(format<char>("{}", -15), "-15");
    } else {
        /* Android simulator does this. Huh? */
        CORRADE_COMPARE(format<char>("{}", -15), "241");
    }
    CORRADE_COMPARE(format<unsigned char>("{}", 230), "230");
}

void FormatTest::integerShort() {
    CORRADE_COMPARE(format<short>("{}", -32001), "-32001");
    CORRADE_COMPARE(format<unsigned short>("{}", 62750), "62750");
}

void FormatTest::integerInt() {
    CORRADE_COMPARE(format<int>("{}", -2000123), "-2000123");
    CORRADE_COMPARE(format<unsigned int>("{}", 4025136), "4025136");
}

void FormatTest::integerLong() {
    CORRADE_COMPARE(format<long>("{}", -2000123), "-2000123");
    CORRADE_COMPARE(format<unsigned long>("{}", 4025136), "4025136");
}

void FormatTest::integerLongLong() {
    CORRADE_COMPARE(format<long long>("{}", -12345678901234ll), "-12345678901234");
    CORRADE_COMPARE(format<unsigned long long>("{}", 24568780984912ull), "24568780984912");
}

void FormatTest::floatingFloat() {
    CORRADE_COMPARE(format("{}", 12.34f), "12.34");
    #ifndef __MINGW32__
    CORRADE_COMPARE(format("{}", -1.32e+07f), "-1.32e+07");
    #else
    CORRADE_COMPARE(format("{}", -1.32e+07f), "-1.32e+007");
    #endif
}

void FormatTest::floatingDouble() {
    CORRADE_COMPARE(format("{}", 12.3404), "12.3404");
    #ifndef __MINGW32__
    CORRADE_COMPARE(format("{}", -1.32e+37), "-1.32e+37");
    #else
    CORRADE_COMPARE(format("{}", -1.32e+37), "-1.32e+037");
    #endif
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void FormatTest::floatingLongDouble() {
    /* That's the case for MSVC as well, source:
       https://msdn.microsoft.com/en-us/library/9cx8xs15.aspx */
    if(sizeof(double) == sizeof(long double))
        CORRADE_SKIP("long double is equivalent to double on this system.");

    #ifdef CORRADE_TARGET_ANDROID
    CORRADE_EXPECT_FAIL_IF(sizeof(void*) == 4,
        "Android has precision problems with long double on 32bit.");
    #endif
    CORRADE_COMPARE(format("{}", 12.3404l), "12.3404");
    #ifndef __MINGW32__
    CORRADE_COMPARE(format("{}", -1.32e+67l), "-1.32e+67");
    #else
    CORRADE_COMPARE(format("{}", -1.32e+67l), "-1.32e+067");
    #endif
}
#endif

namespace {
    template<class> struct FloatingPrecisionData;
    template<> struct FloatingPrecisionData<float> {
        static const char* name() { return "floatingPrecision<float>"; }
        static const char* expected() {
            #ifndef __MINGW32__
            return "3.14159 -12345.7 1.23457e-12 3.14159";
            #else
            return "3.14159 -12345.7 1.23457e-012 3.14159";
            #endif
        }
    };
    template<> struct FloatingPrecisionData<double> {
        static const char* name() { return "floatingPrecision<double>"; }
        static const char* expected() {
            #ifndef __MINGW32__
            return "3.14159265358979 -12345.6789012346 1.23456789012346e-12 3.14159";
            #else
            return "3.14159265358979 -12345.6789012346 1.23456789012346e-012 3.14159";
            #endif
        }
    };
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    template<> struct FloatingPrecisionData<long double> {
        static const char* name() { return "floatingPrecision<long double>"; }
        static const char* expected() {
            #ifndef __MINGW32__
            return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-12 3.14159";
            #else
            return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-012 3.14159";
            #endif
        }
    };
    #endif
}

template<class T> void FormatTest::floatingPrecision() {
    setTestCaseName(FloatingPrecisionData<T>::name());

    /* This test is shared with DebugTest to ensure consistency of output */

    /* That's the case for MSVC as well, source:
       https://msdn.microsoft.com/en-us/library/9cx8xs15.aspx */
    if(std::is_same<T, long double>::value && sizeof(double) == sizeof(long double))
        CORRADE_SKIP("long double is equivalent to double on this system.");

    /* The last float value is to verify that the precision gets reset back */
    {
        #ifdef CORRADE_TARGET_ANDROID
        CORRADE_EXPECT_FAIL_IF((std::is_same<T, long double>::value && sizeof(void*) == 4),
            "Android has precision problems with long double on 32bit.");
        #endif
        CORRADE_COMPARE(format("{} {} {} {}",
            T(3.1415926535897932384626l),
            T(-12345.67890123456789l),
            T(1.234567890123456789e-12l),
            3.141592653589793f), FloatingPrecisionData<T>::expected());
    }
}

void FormatTest::charArray() {
    /* Decays from const char[n] to char* (?), stuff after \0 ignored due to
       strlen */
    CORRADE_COMPARE(format("hello {}", "world\0, i guess?"), "hello world");

    /* Decays to const char* (?) */
    CORRADE_COMPARE(format("hello {}", false ? "world" : "nobody"), "hello nobody");
}

void FormatTest::charArrayView() {
    CORRADE_COMPARE(format("hello {}", Containers::arrayView("worlds", 5)),
        "hello world");
    CORRADE_COMPARE(format("hello {}", Containers::arrayView("world\0, i guess?", 16)),
        (std::string{"hello world\0, i guess?", 22}));
}

void FormatTest::string() {
    CORRADE_COMPARE(format("hello {}", std::string{"worlds", 5}),
        "hello world");
    CORRADE_COMPARE(format("hello {}", std::string{"world\0, i guess?", 16}),
        (std::string{"hello world\0, i guess?", 22}));
}

void FormatTest::multiple() {
    CORRADE_COMPARE(format("so I got {} {}, {} and {} and all that for {}€!",
        2, "beers", "a goulash", "a soup", 8.70f),
        "so I got 2 beers, a goulash and a soup and all that for 8.7€!");
}

void FormatTest::numbered() {
    CORRADE_COMPARE(format("<{0}>HTML, <{1}>amirite</{1}>?</{0}>", "p", "strong"),
        "<p>HTML, <strong>amirite</strong>?</p>");
}

void FormatTest::mixed() {
    CORRADE_COMPARE(format("this {1} {} {0}, {}", "wrong", "is", "certainly"),
        "this is certainly wrong, is");
}

void FormatTest::toBuffer() {
    char buffer[15]{};
    buffer[13] = '?'; /* to verify that a null terminator wasn't printed */
    CORRADE_COMPARE(formatInto(buffer, "hello, {}!", "world"), 13);
    CORRADE_COMPARE(std::string{buffer}, "hello, world!?");
}

void FormatTest::toBufferNullTerminatorFromSnprintfAtTheEnd() {
    char buffer[8];
    CORRADE_COMPARE(formatInto(buffer, "hello {}", 42), 8);
    {
        CORRADE_EXPECT_FAIL("snprintf() really wants to print a null terminator so the last character gets cut off. Need a better solution.");
        CORRADE_COMPARE((std::string{buffer, 8}), "hello 42");
    }
    CORRADE_COMPARE(std::string{buffer}, "hello 4");
}

void FormatTest::appendToString() {
    /* Returned size should be including start offset */
    std::string hello = "hello";
    CORRADE_COMPARE(formatInto(hello, hello.size(), ", {}!", "world"), 13);
    CORRADE_COMPARE(hello, "hello, world!");
}

void FormatTest::insertToString() {
    /* Returned size should be including start offset but be less than string size */
    std::string hello = "hello, __________! Happy to see you!";
    CORRADE_COMPARE(hello.size(), 36);
    CORRADE_COMPARE(formatInto(hello, 8, "Frank"), 13);
    CORRADE_COMPARE(hello, "hello, _Frank____! Happy to see you!");
    CORRADE_COMPARE(hello.size(), 36);
}

void FormatTest::file() {
    const std::string filename = Directory::join(FORMAT_WRITE_TEST_DIR, "format.txt");
    if(!Directory::fileExists(FORMAT_WRITE_TEST_DIR))
        CORRADE_VERIFY(Directory::mkpath(FORMAT_WRITE_TEST_DIR));
    if(Directory::fileExists(filename))
        CORRADE_VERIFY(Directory::rm(filename));

    {
        FILE* f = std::fopen(filename.data(), "w");
        CORRADE_VERIFY(f);
        Containers::ScopedExit e{f, fclose};
        formatInto(f, "A {} {} {} {} {} {} + ({})",
            "string", std::string{"file"}, -2000123, 4025136u, -12345678901234ll, 24568780984912ull, 12.3404f);
    }
    CORRADE_COMPARE_AS(filename,
        "A string file -2000123 4025136 -12345678901234 24568780984912 + (12.3404)",
        TestSuite::Compare::FileToString);
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void FormatTest::fileLongDouble() {
    const std::string filename = Directory::join(FORMAT_WRITE_TEST_DIR, "format-long-double.txt");
    if(!Directory::fileExists(FORMAT_WRITE_TEST_DIR))
        CORRADE_VERIFY(Directory::mkpath(FORMAT_WRITE_TEST_DIR));
    if(Directory::fileExists(filename))
        CORRADE_VERIFY(Directory::rm(filename));

    {
        FILE* f = std::fopen(filename.data(), "w");
        CORRADE_VERIFY(f);
        Containers::ScopedExit e{f, fclose};
        formatInto(f, "{}", 12.3404l);
    } {
        /* That's the case for MSVC as well, source:
           https://msdn.microsoft.com/en-us/library/9cx8xs15.aspx */
        if(sizeof(double) == sizeof(long double))
            CORRADE_SKIP("long double is equivalent to double on this system.");

        #ifdef CORRADE_TARGET_ANDROID
        CORRADE_EXPECT_FAIL_IF(sizeof(void*) == 4,
            "Android has precision problems with long double on 32bit.");
        #endif
        CORRADE_COMPARE_AS(filename, "12.3404",
            TestSuite::Compare::FileToString);
    }
}
#endif

void FormatTest::tooLittlePlaceholders() {
    CORRADE_COMPARE(format("{}!", 42, "but this is", "not visible", 1337), "42!");
}

void FormatTest::tooManyPlaceholders() {
    CORRADE_COMPARE(format("{} + {} = {13}!", 42, "a"), "42 + a = {13}!");
}

void FormatTest::tooSmallBuffer() {
    std::ostringstream out;
    Error redirectError{&out};

    /* The assertion doesn't quit the function, so it will continue with
       copying. Better have some sentinel space at the end. */
    char data[20];
    formatInto({data, 10}, "{}", "hello this is big");
    formatInto({data, 10}, "hello is {} big", "this");

    CORRADE_COMPARE(out.str(),
        "Utility::formatInto(): buffer too small, expected at least 17 but got 10\n"
        "Utility::formatInto(): buffer too small, expected at least 13 but got 10\n");
}

void FormatTest::mismatchedDelimiter() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[128]{};
    {
        formatInto(buffer, "{");
        formatInto(buffer, "{123545");
        formatInto(buffer, "struct { int a; } foo;");
        CORRADE_COMPARE(out.str(),
            "Utility::format(): unexpected end of format string\n"
            "Utility::format(): unexpected end of format string\n"
            "Utility::format(): unknown placeholder content:  \n");
    } {
        out.str({});
        formatInto(buffer, "}");
        formatInto(buffer, "a; } foo;");
        CORRADE_COMPARE(out.str(),
            "Utility::format(): mismatched }\n"
            "Utility::format(): mismatched }\n");
    }
}

void FormatTest::unknownPlaceholderContent() {
    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[256]{};
    formatInto(buffer, "{name}");
    formatInto(buffer, "{1oh}");

    CORRADE_COMPARE(out.str(),
        "Utility::format(): unknown placeholder content: n\n"
        "Utility::format(): unknown placeholder content: o\n");
}

void FormatTest::benchmarkFormat() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        formatInto(buffer, "hello, {}! {1} + {2} = {} = {2} + {1}", "people", 42, 1337, 42 + 1337);

    CORRADE_COMPARE(std::string{buffer}, "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatTest::benchmarkSnprintf() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        snprintf(buffer, 1024, "hello, %s! %i + %i = %i = %i + %i",
            "people", 42, 1337, 42 + 1337, 1337, 42);

    CORRADE_COMPARE(std::string{buffer}, "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatTest::benchmarkSstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        out << "hello, " << "people" << "! " << 42 << " + " << 1337 << " = "
            << 42 + 1337 << " = " << 1337 << " + " << 42;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatTest::benchmarkDebug() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 42 << "+"
            << 1337 << "=" << 42 + 1337 << "=" << 1337 << "+" << 42;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 42 + 1337 = 1379 = 1337 + 42");
}

void FormatTest::benchmarkFloatFormat() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        formatInto(buffer, "hello, {}! {1} + {2} = {} = {2} + {1}", "people", 4.2, 13.37, 4.2 + 13.37);

    CORRADE_COMPARE(std::string{buffer}, "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatTest::benchmarkFloatSnprintf() {
    char buffer[1024];

    CORRADE_BENCHMARK(1000)
        snprintf(buffer, 1024, "hello, %s! %g + %g = %g = %g + %g",
            "people", 4.2, 13.37, 4.2 + 13.37, 13.37, 4.2);

    CORRADE_COMPARE(std::string{buffer}, "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatTest::benchmarkFloatSstream() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        out << "hello, " << "people" << "! " << 4.2 << " + " << 13.37 << " = "
            << 4.2 + 13.37 << " = " << 13.37 << " + " << 4.2;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

void FormatTest::benchmarkFloatDebug() {
    std::ostringstream out;

    CORRADE_BENCHMARK(1000) {
        out.str({});
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd}
            << "hello," << "people" << Debug::nospace << "!" << 4.2 << "+"
            << 13.37 << "=" << 4.2 + 13.37 << "=" << 13.37 << "+" << 4.2;
    }

    CORRADE_COMPARE(out.str(), "hello, people! 4.2 + 13.37 = 17.57 = 13.37 + 4.2");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::FormatTest)
