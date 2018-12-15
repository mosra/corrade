/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Utility {

namespace {

struct NoDefaultConstructor {
    explicit NoDefaultConstructor(std::size_t a): a{a} {}
    std::size_t a;
};

}

template<> struct ConfigurationValue<NoDefaultConstructor> {
    ConfigurationValue() = delete;

    static std::string toString(NoDefaultConstructor value, ConfigurationValueFlags) {
        return std::string(value.a, 'a');
    }
    static NoDefaultConstructor fromString(const std::string& stringValue, ConfigurationValueFlags) {
        return NoDefaultConstructor{stringValue.size()};
    }
};

namespace Test { namespace {

struct ConfigurationValueTest: TestSuite::Tester {
    explicit ConfigurationValueTest();

    void string();
    void unsignedInteger();
    void signedInteger();
    void integerFlags();
    void floatingPoint();
    void floatingPointScientific();
    void unicodeCharLiteral();
    void boolean();

    void custom();
};

ConfigurationValueTest::ConfigurationValueTest() {
    addTests({&ConfigurationValueTest::string,
              &ConfigurationValueTest::unsignedInteger,
              &ConfigurationValueTest::signedInteger,
              &ConfigurationValueTest::integerFlags,
              &ConfigurationValueTest::floatingPoint,
              &ConfigurationValueTest::floatingPointScientific,
              &ConfigurationValueTest::unicodeCharLiteral,
              &ConfigurationValueTest::boolean,

              &ConfigurationValueTest::custom});
}

void ConfigurationValueTest::string() {
    Configuration c;

    /* It should not change any whitespace */
    std::string spaces{" value\t "};
    c.setValue("spaces", spaces);
    CORRADE_COMPARE(c.value("spaces"), spaces);

    std::string newline{"hello\nworld"};
    c.setValue("newline", newline);
    CORRADE_COMPARE(c.value("newline"), newline);

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value("empty"), "");
}

void ConfigurationValueTest::unsignedInteger() {
    Configuration c;

    std::uint32_t a = 5;
    std::string value{"5"};

    c.setValue("uint", a);
    CORRADE_COMPARE(c.value("uint"), value);
    CORRADE_COMPARE(c.value<std::uint32_t>("uint"), a);

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value<std::uint32_t>("empty"), 0);
}

void ConfigurationValueTest::signedInteger() {
    Configuration c;

    std::int32_t a = -10;
    std::string value{"-10"};

    c.setValue("int", a);
    CORRADE_COMPARE(c.value("int"), value);
    CORRADE_COMPARE(c.value<std::int32_t>("int"), a);

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value<std::int32_t>("empty"), 0);
}

void ConfigurationValueTest::integerFlags() {
    Configuration c;

    {
        int a = 0773;
        std::string value{"773"};

        c.setValue("oct", "0773");
        CORRADE_COMPARE(c.value<int>("oct", ConfigurationValueFlag::Oct), a);

        c.setValue("oct", a, ConfigurationValueFlag::Oct);
        CORRADE_COMPARE(c.value("oct"), value);
        CORRADE_COMPARE(c.value<int>("oct", ConfigurationValueFlag::Oct), a);
    } {
        int a = 0x6ecab;
        std::string value{"6ecab"};

        c.setValue("hex", "0x6ecab");
        CORRADE_COMPARE(c.value<int>("hex", ConfigurationValueFlag::Hex), a);

        c.setValue("hex", a, ConfigurationValueFlag::Hex);
        CORRADE_COMPARE(c.value("hex"), value);
        CORRADE_COMPARE(c.value<int>("hex", ConfigurationValueFlag::Hex), a);
    } {
        int a = 0x5462FF;
        std::string value{"5462FF"};

        c.setValue("hexUpper", "0x5462FF");
        CORRADE_COMPARE(c.value<int>("hexUpper", ConfigurationValueFlag::Hex), a);

        c.setValue("hexUpper", a, ConfigurationValueFlag::Hex|ConfigurationValueFlag::Uppercase);
        CORRADE_COMPARE(c.value("hexUpper"), value);
        CORRADE_COMPARE(c.value<int>("hexUpper", ConfigurationValueFlag::Hex), a);
    }
}

void ConfigurationValueTest::floatingPoint() {
    Configuration c;

    {
        float a = 3.78f;
        std::string value{"3.78"};

        c.setValue("float", a);
        CORRADE_COMPARE(c.value("float"), value);
        CORRADE_COMPARE(c.value<float>("float"), a);
    } {
        double a = -2.14;
        std::string value{"-2.14"};

        c.setValue("double", a);
        CORRADE_COMPARE(c.value("double"), value);
        CORRADE_COMPARE(c.value<double>("double"), a);
    }

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    {
        long double a = 0.125l;
        std::string value{"0.125"};

        c.setValue("ld", a);
        CORRADE_COMPARE(c.value("ld"), value);
        CORRADE_COMPARE(c.value<long double>("ld"), a);
    }
    #endif

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value<double>("empty"), 0);
}

void ConfigurationValueTest::floatingPointScientific() {
    Configuration c;

    {
        double a = 2.1e7;
        std::string value{
            #ifndef __MINGW32__
            "2.1e+07"
            #else
            "2.1e+007"
            #endif
        };

        c.setValue("exp", "2.1e7");
        CORRADE_COMPARE(c.value<double>("exp"), a);

        c.setValue("exp", a);
        CORRADE_COMPARE(c.value("exp"), value);
        CORRADE_COMPARE(c.value<double>("exp"), a);
        CORRADE_COMPARE(c.value<double>("exp", ConfigurationValueFlag::Scientific), a);
    } {
        double a = 2.1e+7;
        std::string value{
            #ifndef __MINGW32__
            "2.1e+07"
            #else
            "2.1e+007"
            #endif
        };
        std::string valueSci{
            #ifndef __MINGW32__
            "2.100000e+07"
            #else
            "2.100000e+007"
            #endif
        };

        c.setValue("expPos", "2.1e7");
        CORRADE_COMPARE(c.value<double>("expPos"), a);

        c.setValue("expPos", value);
        CORRADE_COMPARE(c.value("expPos"), value);
        CORRADE_COMPARE(c.value<double>("expPos"), a);

        c.setValue("expPos", a, ConfigurationValueFlag::Scientific);
        CORRADE_COMPARE(c.value("expPos"), valueSci);
        CORRADE_COMPARE(c.value<double>("expPos"), a);
    } {
        double a = -2.1e7;
        std::string value{
            #ifndef __MINGW32__
            "-2.1e+07"
            #else
            "-2.1e+007"
            #endif
        };

        c.setValue("expNeg", "-2.1e7");
        CORRADE_COMPARE(c.value<double>("expNeg"), a);

        c.setValue("expNeg", a);
        CORRADE_COMPARE(c.value("expNeg"), value);
        CORRADE_COMPARE(c.value<double>("expNeg"), a);
    } {
        double a = 2.1e-7;
        std::string value{
            #ifndef __MINGW32__
            "2.1e-07"
            #else
            "2.1e-007"
            #endif
        };

        c.setValue("expNeg2", "2.1e-7");
        CORRADE_COMPARE(c.value<double>("expNeg2"), a);

        c.setValue("expNeg2", a);
        CORRADE_COMPARE(c.value("expNeg2"), value);
        CORRADE_COMPARE(c.value<double>("expNeg2"), a);
    } {
        double a = 2.1E7;
        std::string value{
            #ifndef __MINGW32__
            "2.1E+07"
            #else
            "2.1E+007"
            #endif
        };
        std::string valueSci{
            #ifndef __MINGW32__
            "2.100000E+07"
            #else
            "2.100000E+007"
            #endif
        };

        c.setValue("expBig", "2.1E7");
        CORRADE_COMPARE(c.value<double>("expBig"), a);

        c.setValue("expBig", a, ConfigurationValueFlag::Uppercase);
        CORRADE_COMPARE(c.value("expBig"), value);
        CORRADE_COMPARE(c.value<double>("expBig"), a);

        c.setValue("expBig", a, ConfigurationValueFlag::Scientific|ConfigurationValueFlag::Uppercase);
        CORRADE_COMPARE(c.value("expBig"), valueSci);
        CORRADE_COMPARE(c.value<double>("expBig"), a);
    }
}

void ConfigurationValueTest::unicodeCharLiteral() {
    Configuration c;

    char32_t a = U'\xBEEF';
    std::string value{"BEEF"};

    c.setValue("unicode", "0xBEEF");
    CORRADE_COMPARE(c.value<char32_t>("unicode"), a);

    c.setValue("unicode", a);
    CORRADE_COMPARE(c.value("unicode"), value);
    CORRADE_COMPARE(c.value<char32_t>("unicode"), a);

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value<char32_t>("empty"), 0);
}

void ConfigurationValueTest::boolean() {
    Configuration c;

    bool a = true;
    bool b = false;
    c.setValue("bool", a, 0);
    c.setValue("bool", b, 1);
    CORRADE_COMPARE(c.value("bool", 0), "true");
    CORRADE_COMPARE(c.value<bool>("bool", 0), true);
    CORRADE_COMPARE(c.value("bool", 1), "false");
    CORRADE_COMPARE(c.value<bool>("bool", 1), false);

    /* Empty value is default-constructed */
    c.setValue("empty", "");
    CORRADE_COMPARE(c.value<bool>("empty"), false);
}

void ConfigurationValueTest::custom() {
    Configuration c;

    c.setValue("custom", NoDefaultConstructor{15});
    CORRADE_COMPARE(c.value("custom"), "aaaaaaaaaaaaaaa");
    CORRADE_COMPARE(c.value<NoDefaultConstructor>("custom").a, 15);

    c.setValue("empty", NoDefaultConstructor{0});
    CORRADE_COMPARE(c.value("empty"), "");
    CORRADE_COMPARE(c.value<NoDefaultConstructor>("empty").a, 0);

    c.addValue("more", NoDefaultConstructor{2});
    c.addValue("more", NoDefaultConstructor{5});
    c.addValue("more", NoDefaultConstructor{0});
    c.addValue("more", NoDefaultConstructor{7});

    std::vector<NoDefaultConstructor> values = c.values<NoDefaultConstructor>("more");
    CORRADE_COMPARE(values.size(), 4);
    CORRADE_COMPARE(values[0].a, 2);
    CORRADE_COMPARE(values[1].a, 5);
    CORRADE_COMPARE(values[2].a, 0);
    CORRADE_COMPARE(values[3].a, 7);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ConfigurationValueTest)
