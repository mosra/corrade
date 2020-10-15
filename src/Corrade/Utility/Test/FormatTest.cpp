/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include <limits>
#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/FormatStl.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

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

    void octal();
    void decimal();
    void hexadecimal();
    void hexadecimalUppercase();
    void integerFloat();

    void integerPrecision();

    void floatingFloat();
    void floatingDouble();
    void floatingLongDouble();
    template<class T> void floatingPrecision();

    void floatGeneric();
    void floatGenericUppercase();
    void floatExponent();
    void floatExponentUppercase();
    void floatFixed();
    void floatFixedUppercase();
    void floatBase();

    void charArray();
    void stringView();
    void mutableStringView();
    void string();
    #ifdef CORRADE_BUILD_DEPRECATED
    void charArrayView();
    #endif
    void stlString();
    void stringPrecision();

    void enumConstant();

    void multiple();
    void numbered();
    void numberedType();
    void numberedPrecision();
    void numberedPrecisionBase();
    void mixed();

    void toBuffer();
    void toBufferNullTerminatorFromSnprintfAtTheEnd();
    void array();
    void arrayNullTerminatorFromSnprintfAtTheEnd();
    void appendToString();
    void insertToString();
    void file();
    void fileLongDouble();

    void tooLittlePlaceholders();
    void tooManyPlaceholders();
    void emptyFormat();

    void tooSmallBuffer();
    void mismatchedDelimiter();
    void unknownPlaceholderContent();
    void invalidPrecision();
    void typeForString();
    void invalidType();

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

              &FormatTest::octal,
              &FormatTest::decimal,
              &FormatTest::hexadecimal,
              &FormatTest::hexadecimalUppercase,
              &FormatTest::integerFloat,

              &FormatTest::integerPrecision,

              &FormatTest::floatingFloat,
              &FormatTest::floatingDouble,
              &FormatTest::floatingLongDouble,
              &FormatTest::floatingPrecision<float>,
              &FormatTest::floatingPrecision<double>,
              &FormatTest::floatingPrecision<long double>,

              &FormatTest::floatGeneric,
              &FormatTest::floatGenericUppercase,
              &FormatTest::floatExponent,
              &FormatTest::floatExponentUppercase,
              &FormatTest::floatFixed,
              &FormatTest::floatFixedUppercase,
              &FormatTest::floatBase,

              &FormatTest::charArray,
              &FormatTest::stringView,
              &FormatTest::mutableStringView,
              &FormatTest::string,
              #ifdef CORRADE_BUILD_DEPRECATED
              &FormatTest::charArrayView,
              #endif
              &FormatTest::stlString,
              &FormatTest::stringPrecision,

              &FormatTest::enumConstant,

              &FormatTest::multiple,
              &FormatTest::numbered,
              &FormatTest::numberedType,
              &FormatTest::numberedPrecision,
              &FormatTest::numberedPrecisionBase,
              &FormatTest::mixed,

              &FormatTest::toBuffer,
              &FormatTest::toBufferNullTerminatorFromSnprintfAtTheEnd,
              &FormatTest::array,
              &FormatTest::arrayNullTerminatorFromSnprintfAtTheEnd,
              &FormatTest::appendToString,
              &FormatTest::insertToString,
              &FormatTest::file,
              &FormatTest::fileLongDouble,

              &FormatTest::tooLittlePlaceholders,
              &FormatTest::tooManyPlaceholders,
              &FormatTest::emptyFormat,

              &FormatTest::tooSmallBuffer,
              &FormatTest::mismatchedDelimiter,
              &FormatTest::unknownPlaceholderContent,
              &FormatTest::invalidPrecision,
              &FormatTest::typeForString,
              &FormatTest::invalidType});

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
    CORRADE_COMPARE(formatString(""), "");
}

void FormatTest::textOnly() {
    CORRADE_COMPARE(formatString("hello"), "hello");
}

void FormatTest::escapes() {
    CORRADE_COMPARE(formatString("typedef struct {{ int a; }} Type;"),
        "typedef struct { int a; } Type;");
}

void FormatTest::integerChar() {
    if(std::is_signed<char>::value) {
        CORRADE_COMPARE(formatString<char>("{}", -15), "-15");
    } else {
        /* Android simulator does this. Huh? */
        CORRADE_COMPARE(formatString<char>("{}", -15), "241");
    }
    CORRADE_COMPARE(formatString<unsigned char>("{}", 230), "230");
}

void FormatTest::integerShort() {
    CORRADE_COMPARE(formatString<short>("{}", -32001), "-32001");
    CORRADE_COMPARE(formatString<unsigned short>("{}", 62750), "62750");
}

void FormatTest::integerInt() {
    CORRADE_COMPARE(formatString<int>("{}", -2000123), "-2000123");
    CORRADE_COMPARE(formatString<unsigned int>("{}", 4025136), "4025136");
}

void FormatTest::integerLong() {
    CORRADE_COMPARE(formatString<long>("{}", -2000123), "-2000123");
    CORRADE_COMPARE(formatString<unsigned long>("{}", 4025136), "4025136");
}

void FormatTest::integerLongLong() {
    CORRADE_COMPARE(formatString<long long>("{}", -12345678901234ll), "-12345678901234");
    CORRADE_COMPARE(formatString<unsigned long long>("{}", 24568780984912ull), "24568780984912");
}

void FormatTest::octal() {
    CORRADE_COMPARE(formatString("{:o}", 0777), "777");
    CORRADE_COMPARE(formatString("{:o}", 0777ull), "777");
}

void FormatTest::decimal() {
    CORRADE_COMPARE(formatString("{:d}", 1234), "1234");
    CORRADE_COMPARE(formatString("{:d}", 1234ull), "1234");
}

void FormatTest::hexadecimal() {
    CORRADE_COMPARE(formatString("{:x}", 0xdead), "dead");
    CORRADE_COMPARE(formatString("{:x}", 0xdeadbeefcafebabeull), "deadbeefcafebabe");
}

void FormatTest::hexadecimalUppercase() {
    CORRADE_COMPARE(formatString("{:X}", 0xDEAD), "DEAD");
    CORRADE_COMPARE(formatString("{:X}", 0xDEADBEEFCAFEBABEULL), "DEADBEEFCAFEBABE");
}

void FormatTest::integerFloat() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[128]{};
    formatInto(buffer, "{:g}", 123456);
    formatInto(buffer, "{:g}", 123456ull);
    CORRADE_COMPARE(out.str(),
        "Utility::format(): floating-point type used for an integral value\n"
        "Utility::format(): floating-point type used for an integral value\n");
}

void FormatTest::integerPrecision() {
    /* Default should preserve the zero */
    CORRADE_COMPARE(formatString("{}!", 0), "0!");
    CORRADE_COMPARE(formatString("{}!", 0u), "0!");
    CORRADE_COMPARE(formatString("{}!", 0ll), "0!");
    CORRADE_COMPARE(formatString("{}!", 0ull), "0!");

    /* Zero should not preserve the zero */
    CORRADE_COMPARE(formatString("{:.0}!", 0), "!");
    CORRADE_COMPARE(formatString("{:.0}!", 0u), "!");
    CORRADE_COMPARE(formatString("{:.0}!", 0ll), "!");
    CORRADE_COMPARE(formatString("{:.0}!", 0ull), "!");

    /* Smaller should overflow */
    CORRADE_COMPARE(formatString("{:.2}", 1536), "1536");
    CORRADE_COMPARE(formatString("{:.2}", 1536u), "1536");
    CORRADE_COMPARE(formatString("{:.2}", 1536ll), "1536");
    CORRADE_COMPARE(formatString("{:.2}", 1536ull), "1536");

    /* Larger should pad from left */
    CORRADE_COMPARE(formatString("{:.15}", 1536), "000000000001536");
    CORRADE_COMPARE(formatString("{:.15}", 1536u), "000000000001536");
    CORRADE_COMPARE(formatString("{:.15}", 1536ll), "000000000001536");
    CORRADE_COMPARE(formatString("{:.15}", 1536ull), "000000000001536");
}

void FormatTest::floatingFloat() {
    CORRADE_COMPARE(formatString("{}", 12.34f), "12.34");
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{}", -1.32e+07f), "-1.32e+07");
    #else
    CORRADE_COMPARE(formatString("{}", -1.32e+07f), "-1.32e+007");
    #endif
}

void FormatTest::floatingDouble() {
    CORRADE_COMPARE(formatString("{}", 12.3404), "12.3404");
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{}", -1.32e+37), "-1.32e+37");
    #else
    CORRADE_COMPARE(formatString("{}", -1.32e+37), "-1.32e+037");
    #endif
}

void FormatTest::floatingLongDouble() {
    CORRADE_COMPARE(formatString("{}", 12.3404l), "12.3404");
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{}", -1.32e+67l), "-1.32e+67");
    #else
    CORRADE_COMPARE(formatString("{}", -1.32e+67l), "-1.32e+067");
    #endif
}

template<class> struct FloatingPrecisionData;
template<> struct FloatingPrecisionData<float> {
    static const char* name() { return "float"; }
    static const char* expected() {
        #ifndef __MINGW32__
        return "3.14159 -12345.7 1.23457e-12 3.14159";
        #else
        return "3.14159 -12345.7 1.23457e-012 3.14159";
        #endif
    }
};
template<> struct FloatingPrecisionData<double> {
    static const char* name() { return "double"; }
    static const char* expected() {
        #ifndef __MINGW32__
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-12 3.14159";
        #else
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-012 3.14159";
        #endif
    }
};
template<> struct FloatingPrecisionData<long double> {
    static const char* name() { return "long double"; }
    static const char* expected() {
        #ifndef __MINGW32__
        #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
        return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-12 3.14159";
        #else
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-12 3.14159";
        #endif
        #else
        return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-012 3.14159";
        #endif
    }
};

template<class T> void FormatTest::floatingPrecision() {
    setTestCaseTemplateName(FloatingPrecisionData<T>::name());

    /* This test is shared with DebugTest to ensure consistency of output */

    /* The last float value is to verify that the precision gets reset back */
    {
        CORRADE_COMPARE(formatString("{} {} {} {}",
            T(3.1415926535897932384626l),
            T(-12345.67890123456789l),
            T(1.234567890123456789e-12l),
            3.141592653589793f), FloatingPrecisionData<T>::expected());
    }
}

void FormatTest::floatGeneric() {
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{}", 1234.0e5f), "1.234e+08");
    #else
    CORRADE_COMPARE(formatString("{}", 1234.0e5f), "1.234e+008");
    #endif
    CORRADE_COMPARE(formatString("{}", 1234.0e5), "123400000");
    CORRADE_COMPARE(formatString("{}", 1234.0e5l), "123400000");

    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:g}", 1234.0e5f), "1.234e+08");
    #else
    CORRADE_COMPARE(formatString("{:g}", 1234.0e5f), "1.234e+008");
    #endif
    CORRADE_COMPARE(formatString("{:g}", 1234.0e5), "123400000");
    CORRADE_COMPARE(formatString("{:g}", 1234.0e5l), "123400000");

    CORRADE_COMPARE(formatString("{:.3}", 1.0f), "1");
    CORRADE_COMPARE(formatString("{:.3}", 1.0), "1");
    CORRADE_COMPARE(formatString("{:.3}", 1.0l), "1");
    CORRADE_COMPARE(formatString("{:.3}", 12.34567f), "12.3");
    CORRADE_COMPARE(formatString("{:.3}", 12.34567), "12.3");
    CORRADE_COMPARE(formatString("{:.3}", 12.34567l), "12.3");

    CORRADE_COMPARE(formatString("{:.3g}", 1.0f), "1");
    CORRADE_COMPARE(formatString("{:.3g}", 1.0), "1");
    CORRADE_COMPARE(formatString("{:.3g}", 1.0l), "1");
    CORRADE_COMPARE(formatString("{:.3g}", 12.34567f), "12.3");
    CORRADE_COMPARE(formatString("{:.3g}", 12.34567), "12.3");
    CORRADE_COMPARE(formatString("{:.3g}", 12.34567l), "12.3");
}

void FormatTest::floatGenericUppercase() {
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:G}", 1234.0e5f), "1.234E+08");
    #else
    CORRADE_COMPARE(formatString("{:G}", 1234.0e5f), "1.234E+008");
    #endif
    CORRADE_COMPARE(formatString("{:G}", 1234.0e5), "123400000");
    CORRADE_COMPARE(formatString("{:G}", 1234.0e5l), "123400000");

    CORRADE_COMPARE(formatString("{:.3G}", 1.0f), "1");
    CORRADE_COMPARE(formatString("{:.3G}", 1.0), "1");
    CORRADE_COMPARE(formatString("{:.3G}", 1.0l), "1");
    CORRADE_COMPARE(formatString("{:.3G}", 12.34567f), "12.3");
    CORRADE_COMPARE(formatString("{:.3G}", 12.34567), "12.3");
    CORRADE_COMPARE(formatString("{:.3G}", 12.34567l), "12.3");
}

void FormatTest::floatExponent() {
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5f), "1.234000e+08");
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5), "1.234000000000000e+08");
    #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5l), "1.234000000000000000e+08");
    #else
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5l), "1.234000000000000e+08");
    #endif
    #else
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5f), "1.234000e+008");
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5), "1.234000000000000e+008");
    CORRADE_COMPARE(formatString("{:e}", 1234.0e5l), "1.234000000000000000e+008");
    #endif

    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:.3e}", 1.0f), "1.000e+00");
    CORRADE_COMPARE(formatString("{:.3e}", 1.0), "1.000e+00");
    CORRADE_COMPARE(formatString("{:.3e}", 1.0l), "1.000e+00");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567f), "1.235e+01");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567), "1.235e+01");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567l), "1.235e+01");
    #else
    CORRADE_COMPARE(formatString("{:.3e}", 1.0f), "1.000e+000");
    CORRADE_COMPARE(formatString("{:.3e}", 1.0), "1.000e+000");
    CORRADE_COMPARE(formatString("{:.3e}", 1.0l), "1.000e+000");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567f), "1.235e+001");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567), "1.235e+001");
    CORRADE_COMPARE(formatString("{:.3e}", 12.34567l), "1.235e+001");
    #endif
}

void FormatTest::floatExponentUppercase() {
    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5f), "1.234000E+08");
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5), "1.234000000000000E+08");
    #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5l), "1.234000000000000000E+08");
    #else
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5l), "1.234000000000000E+08");
    #endif
    #else
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5f), "1.234000E+008");
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5), "1.234000000000000E+008");
    CORRADE_COMPARE(formatString("{:E}", 1234.0e5l), "1.234000000000000000E+008");
    #endif

    #ifndef __MINGW32__
    CORRADE_COMPARE(formatString("{:.3E}", 1.0f), "1.000E+00");
    CORRADE_COMPARE(formatString("{:.3E}", 1.0), "1.000E+00");
    CORRADE_COMPARE(formatString("{:.3E}", 1.0l), "1.000E+00");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567f), "1.235E+01");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567), "1.235E+01");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567l), "1.235E+01");
    #else
    CORRADE_COMPARE(formatString("{:.3E}", 1.0f), "1.000E+000");
    CORRADE_COMPARE(formatString("{:.3E}", 1.0), "1.000E+000");
    CORRADE_COMPARE(formatString("{:.3E}", 1.0l), "1.000E+000");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567f), "1.235E+001");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567), "1.235E+001");
    CORRADE_COMPARE(formatString("{:.3E}", 12.34567l), "1.235E+001");
    #endif
}

void FormatTest::floatFixed() {
    CORRADE_COMPARE(formatString("{:f}", 1234.0e5f), "123400000.000000");
    CORRADE_COMPARE(formatString("{:f}", 1234.0e5), "123400000.000000000000000");
    #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
    CORRADE_COMPARE(formatString("{:f}", 1234.0e5l), "123400000.000000000000000000");
    #else
    CORRADE_COMPARE(formatString("{:f}", 1234.0e5l), "123400000.000000000000000");
    #endif
    CORRADE_COMPARE(formatString("{:f}", std::numeric_limits<float>::quiet_NaN()), "nan");

    CORRADE_COMPARE(formatString("{:.3f}", 1.0f), "1.000");
    CORRADE_COMPARE(formatString("{:.3f}", 1.0), "1.000");
    CORRADE_COMPARE(formatString("{:.3f}", 1.0l), "1.000");
    CORRADE_COMPARE(formatString("{:.3f}", 12.34567f), "12.346");
    CORRADE_COMPARE(formatString("{:.3f}", 12.34567), "12.346");
    CORRADE_COMPARE(formatString("{:.3f}", 12.34567l), "12.346");
}

void FormatTest::floatFixedUppercase() {
    CORRADE_COMPARE(formatString("{:F}", 1234.0e5f), "123400000.000000");
    CORRADE_COMPARE(formatString("{:F}", 1234.0e5), "123400000.000000000000000");
    #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
    CORRADE_COMPARE(formatString("{:F}", 1234.0e5l), "123400000.000000000000000000");
    #else
    CORRADE_COMPARE(formatString("{:F}", 1234.0e5l), "123400000.000000000000000");
    #endif
    CORRADE_COMPARE(formatString("{:F}", std::numeric_limits<float>::quiet_NaN()), "NAN");

    CORRADE_COMPARE(formatString("{:.3F}", 1.0f), "1.000");
    CORRADE_COMPARE(formatString("{:.3F}", 1.0), "1.000");
    CORRADE_COMPARE(formatString("{:.3F}", 1.0l), "1.000");
    CORRADE_COMPARE(formatString("{:.3F}", 12.34567f), "12.346");
    CORRADE_COMPARE(formatString("{:.3F}", 12.34567), "12.346");
    CORRADE_COMPARE(formatString("{:.3F}", 12.34567l), "12.346");
}

void FormatTest::floatBase() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[128]{};
    formatInto(buffer, "{:o}", 123456.0f);
    formatInto(buffer, "{:x}", 123456.0);
    formatInto(buffer, "{:d}", 123456.0l);
    CORRADE_COMPARE(out.str(),
        "Utility::format(): integral type used for a floating-point value\n"
        "Utility::format(): integral type used for a floating-point value\n"
        "Utility::format(): integral type used for a floating-point value\n");
}

void FormatTest::charArray() {
    /* Decays from const char[n] to char* (?), stuff after \0 ignored due to
       strlen */
    CORRADE_COMPARE(formatString("hello {}", "world\0, i guess?"), "hello world");

    /* Decays to const char* (?) */
    CORRADE_COMPARE(formatString("hello {}", false ? "world" : "nobody"), "hello nobody");
}

void FormatTest::stringView() {
    using namespace Containers::Literals;

    CORRADE_COMPARE(formatString("hello {}", "worlds"_s.except(1)),
        "hello world");
    CORRADE_COMPARE(formatString("hello {}", "world\0, i guess?"_s),
        (std::string{"hello world\0, i guess?", 22}));
}

void FormatTest::mutableStringView() {
    Containers::String a = "world";
    CORRADE_COMPARE(formatString("hello {}", Containers::MutableStringView{a}),
        "hello world");
}

void FormatTest::string() {
    CORRADE_COMPARE(formatString("hello {}", Containers::String{"world"}),
        "hello world");
}

#ifdef CORRADE_BUILD_DEPRECATED
void FormatTest::charArrayView() {
    CORRADE_COMPARE(formatString("hello {}", Containers::arrayView("worlds", 5)),
        "hello world");
}
#endif

void FormatTest::stlString() {
    CORRADE_COMPARE(formatString("hello {}", std::string{"worlds", 5}),
        "hello world");
    CORRADE_COMPARE(formatString("hello {}", std::string{"world\0, i guess?", 16}),
        (std::string{"hello world\0, i guess?", 22}));
}

void FormatTest::stringPrecision() {
    CORRADE_COMPARE(formatString("{:.4}", "hello world"), "hell");
}

enum: std::uint64_t { SomeValue = 12345678901234ull };
enum Enum { SomeDifferentValue };

}}

namespace Implementation {
    template<> struct Formatter<Test::Enum> {
        static std::size_t format(const Containers::ArrayView<char>& buffer, Test::Enum, int precision, FormatType type) {
            return Formatter<const char*>::format(buffer, "SomeDifferentValue", precision, type);
        }
    };
}

namespace Test { namespace {

void FormatTest::enumConstant() {
    CORRADE_COMPARE(formatString("value: {} but an enum: {}", SomeValue, SomeDifferentValue), "value: 12345678901234 but an enum: SomeDifferentValue");
}

void FormatTest::multiple() {
    CORRADE_COMPARE(formatString("so I got {} {}, {} and {} and all that for {}€!",
        2, "beers", "a goulash", "a soup", 8.70f),
        "so I got 2 beers, a goulash and a soup and all that for 8.7€!");
}

void FormatTest::numbered() {
    CORRADE_COMPARE(formatString("<{0}>HTML, <{1}>amirite</{1}>?</{0}>", "p", "strong"),
        "<p>HTML, <strong>amirite</strong>?</p>");
}

void FormatTest::numberedType() {
    CORRADE_COMPARE(formatString("{0:x}{1:X}{0:x}", 0xdead, 0xface),
        "deadFACEdead");
}

void FormatTest::numberedPrecision() {
    CORRADE_COMPARE(formatString("{0:.1}{:.6}{0:.1}", 5, 0),
        "50000005");
}

void FormatTest::numberedPrecisionBase() {
    CORRADE_COMPARE(formatString("{0:.1X}{:.6x}{0:.1X}", 0xb, 0),
        "B000000B");
}

void FormatTest::mixed() {
    CORRADE_COMPARE(formatString("this {1} {} {0}, {}", "wrong", "is", "certainly"),
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

void FormatTest::array() {
    Containers::Array<char> array = format("hello, {}!", "world");
    CORRADE_COMPARE((std::string{array, array.size()}), "hello, world!");
}

void FormatTest::arrayNullTerminatorFromSnprintfAtTheEnd() {
    Containers::Array<char> array = format("hello {}", 42);
    CORRADE_COMPARE((std::string{array, array.size()}), "hello 42");
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
    if(!Directory::exists(FORMAT_WRITE_TEST_DIR))
        CORRADE_VERIFY(Directory::mkpath(FORMAT_WRITE_TEST_DIR));
    if(Directory::exists(filename))
        CORRADE_VERIFY(Directory::rm(filename));

    {
        FILE* f = std::fopen(filename.data(), "w");
        CORRADE_VERIFY(f);
        Containers::ScopeGuard e{f, fclose};
        formatInto(f, "A {} {} {} {} {} {} + ({}) {}",
            "string", std::string{"file"}, -2000123, 4025136u, -12345678901234ll, 24568780984912ull, 12.3404f, 1.52);
    }
    CORRADE_COMPARE_AS(filename,
        "A string file -2000123 4025136 -12345678901234 24568780984912 + (12.3404) 1.52",
        TestSuite::Compare::FileToString);
}

void FormatTest::fileLongDouble() {
    const std::string filename = Directory::join(FORMAT_WRITE_TEST_DIR, "format-long-double.txt");
    if(!Directory::exists(FORMAT_WRITE_TEST_DIR))
        CORRADE_VERIFY(Directory::mkpath(FORMAT_WRITE_TEST_DIR));
    if(Directory::exists(filename))
        CORRADE_VERIFY(Directory::rm(filename));

    {
        FILE* f = std::fopen(filename.data(), "w");
        CORRADE_VERIFY(f);
        Containers::ScopeGuard e{f, fclose};
        formatInto(f, "{}", 12.3404l);
    }

    CORRADE_COMPARE_AS(filename, "12.3404", TestSuite::Compare::FileToString);
}

void FormatTest::tooLittlePlaceholders() {
    /* Not a problem */
    CORRADE_COMPARE(formatString("{}!", 42, "but this is", "not visible", 1337), "42!");
}

void FormatTest::tooManyPlaceholders() {
    /* Not a problem */
    CORRADE_COMPARE(formatString("{} + {} = {13}!", 42, "a"), "42 + a = {13}!");
}

void FormatTest::emptyFormat() {
    /* Not a problem */
    CORRADE_COMPARE(formatString("{0:}*9 = {:}", 6, 42), "6*9 = 42");
}

void FormatTest::tooSmallBuffer() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[256]{};
    formatInto(buffer, "{name}");
    formatInto(buffer, "{1oh}");
    formatInto(buffer, "{1:xe}");

    CORRADE_COMPARE(out.str(),
        "Utility::format(): unknown placeholder content: n\n"
        "Utility::format(): unknown placeholder content: o\n"
        "Utility::format(): unknown placeholder content: e\n");
}

void FormatTest::invalidPrecision() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[256]{};
    formatInto(buffer, "{:.}");
    formatInto(buffer, "{1:.x}");

    CORRADE_COMPARE(out.str(),
        "Utility::format(): invalid character in precision specifier: }\n"
        "Utility::format(): invalid character in precision specifier: x\n");
}

void FormatTest::typeForString() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[256]{};
    formatInto(buffer, "{:x}", "dead");

    CORRADE_COMPARE(out.str(),
        "Utility::format(): type specifier can't be used for a string value\n");
}

void FormatTest::invalidType() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    /* Using formatInto() instead of format() to avoid all errors being printed
       twice due to the extra pass with size calculation */
    char buffer[256]{};
    formatInto(buffer, "{:H}");

    CORRADE_COMPARE(out.str(),
        "Utility::format(): invalid type specifier: H\n");
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

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::FormatTest)
