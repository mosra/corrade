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

#include <iostream>
#include <map>
#include <set>
#include <sstream> /** @todo move to dedicated test file once Debug is fully stream-free */
#include <string>

#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/DebugStl.h" /** @todo move to dedicated test file once Debug is fully stream-free */
#include "Corrade/Utility/FormatStl.h" /** @todo remove once Debug is fully stream-free */

#ifndef CORRADE_TARGET_EMSCRIPTEN
#include <thread>
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

struct DebugTest: TestSuite::Tester {
    explicit DebugTest();

    void isTty();

    void debug();
    void string();
    /** @todo once we get rid of iostreams, move this and other STL stuff to
        dedicated DebugStlTest */
    void stringStl();
    template<class T> void ints();
    template<class T> void floats();
    void boolean();
    void chars();
    void pointer();
    void unicode();
    void custom();

    void flags();
    void nospace();
    void newline();
    void space();
    void noNewlineAtTheEnd();

    void colors();
    void colorsAutoReset();
    void colorsExplicitReset();
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    void colorsBoldInvertedReset();
    #endif
    void colorsDisabled();
    void colorsNospace();
    void colorsNoOutput();
    void colorsScoped();

    void hex();

    void valueAsColor();
    void valueAsColorColorsDisabled();

    void iterable();
    void iterableNested();
    void iterablePacked();
    void iterableNospace();
    void pair();
    void tuple();
    void iterablePairPacked();
    void iterableTuplePacked();

    void ostreamFallback();
    void ostreamFallbackPriority();

    void scopedOutput();

    void move();

    void stringOutput();
    void stringOutputNonEmpty();
    void stringOutputNonEmptySmall();
    void stringOutputScopedFlush();
    void stringOutputReuseGrowable();
    void stringOutputReuseCleared();
    void stringOutputReuseModified();
    void stringOutputReuseModifiedUnsynced();
    void stringOutputMove();

    void debugColor();
    void debugFlag();
    void debugFlags();

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void multithreaded();
    #endif

    void sourceLocation();
};

DebugTest::DebugTest() {
    addTests({
        &DebugTest::isTty,

        &DebugTest::debug,
        &DebugTest::string,
        &DebugTest::stringStl,
        &DebugTest::boolean,
        &DebugTest::ints<unsigned char>,
        &DebugTest::ints<char>,
        &DebugTest::ints<unsigned short>,
        &DebugTest::ints<short>,
        &DebugTest::ints<unsigned int>,
        &DebugTest::ints<int>,
        &DebugTest::ints<unsigned long>,
        &DebugTest::ints<long>,
        &DebugTest::ints<unsigned long long>,
        &DebugTest::ints<long long>,
        &DebugTest::floats<float>,
        &DebugTest::floats<double>,
        &DebugTest::floats<long double>,
        &DebugTest::chars,
        &DebugTest::pointer,
        &DebugTest::unicode,
        &DebugTest::custom,

        &DebugTest::flags,
        &DebugTest::nospace,
        &DebugTest::newline,
        &DebugTest::space,
        &DebugTest::noNewlineAtTheEnd});

    addInstancedTests({&DebugTest::colors}, 9);

    addTests({
        &DebugTest::colorsAutoReset,
        &DebugTest::colorsExplicitReset,
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        &DebugTest::colorsBoldInvertedReset,
        #endif
        &DebugTest::colorsDisabled,
        &DebugTest::colorsNospace,
        &DebugTest::colorsNoOutput,
        &DebugTest::colorsScoped,

        &DebugTest::hex,

        &DebugTest::valueAsColor,
        &DebugTest::valueAsColorColorsDisabled,

        &DebugTest::iterable,
        &DebugTest::iterableNested,
        &DebugTest::iterablePacked,
        &DebugTest::iterableNospace,
        &DebugTest::pair,
        &DebugTest::tuple,
        &DebugTest::iterablePairPacked,
        &DebugTest::iterableTuplePacked,

        &DebugTest::ostreamFallback,
        &DebugTest::ostreamFallbackPriority,

        &DebugTest::scopedOutput,

        &DebugTest::move,

        &DebugTest::stringOutput,
        &DebugTest::stringOutputNonEmpty,
        &DebugTest::stringOutputNonEmptySmall,
        &DebugTest::stringOutputScopedFlush,
        &DebugTest::stringOutputReuseGrowable,
        &DebugTest::stringOutputReuseCleared,
        &DebugTest::stringOutputReuseModified,
        &DebugTest::stringOutputReuseModifiedUnsynced,
        &DebugTest::stringOutputMove,

        &DebugTest::debugColor,
        &DebugTest::debugFlag,
        &DebugTest::debugFlags,

        #ifndef CORRADE_TARGET_EMSCRIPTEN
        &DebugTest::multithreaded,
        #endif

        &DebugTest::sourceLocation});
}

void DebugTest::debug() {
    std::ostringstream debug, warning, error;

    Debug(&debug) << "a" << 33 << 0.567f;
    Warning(&warning) << "w" << 42 << "meh";
    Error(&error) << "e";

    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");
    CORRADE_COMPARE(warning.str(), "w 42 meh\n");
    CORRADE_COMPARE(error.str(), "e\n");

    /* Multiple times used instance */
    debug.str("");
    {
        Debug d{&debug};
        d << "a";
        d << 33;
        d << 0.567f;
    }
    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");

    /* Don't add newline at the end of empty output */
    debug.str("");
    Debug{&debug};
    CORRADE_COMPARE(debug.str(), "");
}

void DebugTest::string() {
    using namespace Containers::Literals;

    Containers::String a = "mutable";

    std::ostringstream out;
    Debug{&out} << "hello\0world,"_s << Containers::String{"very\0well!"_s} << Containers::MutableStringView{a};
    CORRADE_COMPARE(out.str(), (std::string{"hello\0world, very\0well! mutable\n", 32}));
}

void DebugTest::stringStl() {
    std::ostringstream out;
    Debug{&out} << std::string{"hello\0world!", 12};
    CORRADE_COMPARE(out.str(), (std::string{"hello\0world!\n", 13}));
}

template<class> struct IntsData;
template<> struct IntsData<char> {
    static const char* name() { return "char"; }
    static char value() {
        if(std::is_signed<char>::value)
            return -123;
        /* Android has unsigned char */
        CORRADE_INFO("char is unsigned");
        /* static_cast is needed to avoid "cast truncates constant value" on
           (x86) MSVC, sigh */
        return char(static_cast<unsigned char>(223));
    }
    static const char* expected() {
        /* Android has unsigned char */
        return std::is_signed<char>::value ? "-123\n" : "223\n";
    }
};
template<> struct IntsData<unsigned char> {
    static const char* name() { return "unsigned char"; }
    static unsigned char value() { return 223; }
    static const char* expected() { return "223\n"; }
};
template<> struct IntsData<short> {
    static const char* name() { return "short"; }
    static short value() { return -31752; }
    static const char* expected() { return "-31752\n"; }
};
template<> struct IntsData<unsigned short> {
    static const char* name() { return "unsigned short"; }
    static unsigned short value() { return 48523; }
    static const char* expected() { return "48523\n"; }
};
template<> struct IntsData<int> {
    static const char* name() { return "int"; }
    static int value() { return -1423584221; }
    static const char* expected() { return "-1423584221\n"; }
};
template<> struct IntsData<unsigned int> {
    static const char* name() { return "unsigned int"; }
    static unsigned int value() { return 4214211824; }
    static const char* expected() { return "4214211824\n"; }
};
template<> struct IntsData<long long> {
    static const char* name() { return "long long"; }
    static long long value() { return -4214211824234535464ll; }
    static const char* expected() { return "-4214211824234535464\n"; }
};
template<> struct IntsData<unsigned long long> {
    static const char* name() { return "unsigned long long"; }
    static unsigned long long value() { return 14214211824234535464ull; }
    static const char* expected() { return "14214211824234535464\n"; }
};
template<int> struct IntsDataFor;
template<int> struct IntsDataForUnsigned;
template<> struct IntsDataFor<4>: IntsData<int> {};
template<> struct IntsDataFor<8>: IntsData<long long> {};
template<> struct IntsDataForUnsigned<4>: IntsData<unsigned int> {};
template<> struct IntsDataForUnsigned<8>: IntsData<unsigned long long> {};
template<> struct IntsData<long>: IntsDataFor<sizeof(long)> {
    static const char* name() { return "long"; }
};
template<> struct IntsData<unsigned long>: IntsDataForUnsigned<sizeof(long)> {
    static const char* name() { return "unsigned long"; }
};

template<class> struct FloatsData;
template<> struct FloatsData<float> {
    static const char* name() { return "float"; }
    static const char* expected() {
        #ifndef __MINGW32__
        return "3.14159 -12345.7 1.23457e-12 3.14159\n";
        #else
        return "3.14159 -12345.7 1.23457e-012 3.14159\n";
        #endif
    }
};
template<> struct FloatsData<double> {
    static const char* name() { return "double"; }
    static const char* expected() {
        #ifndef __MINGW32__
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-12 3.14159\n";
        #else
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-012 3.14159\n";
        #endif
    }
};
template<> struct FloatsData<long double> {
    static const char* name() { return "long double"; }
    static const char* expected() {
        #ifndef __MINGW32__
        #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
        return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-12 3.14159\n";
        #else
        return "3.14159265358979 -12345.6789012346 1.23456789012346e-12 3.14159\n";
        #endif
        #else
        return "3.14159265358979324 -12345.6789012345679 1.23456789012345679e-012 3.14159\n";
        #endif
    }
};

void DebugTest::isTty() {
    Debug{} << "Debug output is a TTY?  " << (Debug::isTty() ? "yes" : "no");
    Debug{} << "Warning output is a TTY?" << (Warning::isTty() ? "yes" : "no");
    Debug{} << "Error output is a TTY?  " << (Error::isTty() ? "yes" : "no");

    CORRADE_VERIFY(!Debug::isTty(nullptr));

    std::ostringstream o;
    CORRADE_VERIFY(!Debug::isTty(&o));
}

template<class T> void DebugTest::ints() {
    setTestCaseTemplateName(IntsData<T>::name());

    /* To capture correct function name */
    CORRADE_VERIFY(true);

    std::ostringstream out;
    Debug{&out} << IntsData<T>::value();
    CORRADE_COMPARE(out.str(), IntsData<T>::expected());
}

template<class T> void DebugTest::floats() {
    setTestCaseTemplateName(FloatsData<T>::name());

    /* This test is shared with Format to ensure consistency of output */

    /* That's the case for MSVC as well, source:
       https://msdn.microsoft.com/en-us/library/9cx8xs15.aspx */
    if(std::is_same<T, long double>::value && sizeof(double) == sizeof(long double))
        CORRADE_SKIP("long double is equivalent to double on this system.");

    std::ostringstream o;
    /* The last float value is to verify that the precision gets reset back */
    Debug(&o) << T(3.1415926535897932384626l) << T(-12345.67890123456789l) << T(1.234567890123456789e-12l) << 3.141592653589793f;
    {
        #ifdef CORRADE_TARGET_ANDROID
        CORRADE_EXPECT_FAIL_IF((std::is_same<T, long double>::value && sizeof(void*) == 4),
            "Android has precision problems with long double on 32bit.");
        #endif
        CORRADE_COMPARE(o.str(), FloatsData<T>::expected());
    }
}

void DebugTest::boolean() {
    std::ostringstream o;
    Debug(&o) << true << false;
    CORRADE_COMPARE(o.str(), "true false\n");
}

void DebugTest::chars() {
    std::ostringstream o;
    Debug(&o) << 'a';
    CORRADE_COMPARE(o.str(), "97\n");
}

void DebugTest::pointer() {
    std::ostringstream out;
    Debug{&out} << reinterpret_cast<void*>(std::size_t(0xdeadbabeu)) << nullptr;
    CORRADE_COMPARE(out.str(), "0xdeadbabe nullptr\n");
}

void DebugTest::unicode() {
    /* Four-character hex values */
    std::ostringstream o;
    Debug(&o) << U'a';
    CORRADE_COMPARE(o.str(), "U+0061\n");

    /* Longer hex values */
    o.str({});
    Debug(&o) << U'\xBEEF3';
    CORRADE_COMPARE(o.str(), "U+BEEF3\n");

    /* UTF-32 string */
    o.str({});
    Debug(&o) << U"abc";
    CORRADE_COMPARE(o.str(), "{U+0061, U+0062, U+0063}\n");
}

struct Foo {
    int value;
};

Debug& operator<<(Debug& debug, const Foo& value) {
    return debug << value.value;
}

void DebugTest::custom() {
    std::ostringstream out;

    Foo f = { 42 };
    {
        Debug(&out) << "The answer is" << f;
        Debug(&out) << f << "is the answer";
    }
    CORRADE_COMPARE(out.str(), "The answer is 42\n"
                               "42 is the answer\n");
}

void DebugTest::flags() {
    std::ostringstream out;

    {
        Debug d{&out};
        CORRADE_COMPARE(d.flags(), Debug::Flags{});
        CORRADE_COMPARE(d.immediateFlags(), Debug::Flag::NoSpace);

        d << "Hello";
        CORRADE_COMPARE(d.flags(), Debug::Flags{});
        CORRADE_COMPARE(d.immediateFlags(), Debug::Flags{});

        d.setFlags(Debug::Flag::NoNewlineAtTheEnd);
        CORRADE_COMPARE(d.flags(), Debug::Flag::NoNewlineAtTheEnd);
        CORRADE_COMPARE(d.immediateFlags(), Debug::Flag::NoNewlineAtTheEnd);

        d.setImmediateFlags(Debug::Flag::NoSpace);
        CORRADE_COMPARE(d.flags(), Debug::Flag::NoNewlineAtTheEnd);
        CORRADE_COMPARE(d.immediateFlags(), Debug::Flag::NoNewlineAtTheEnd|Debug::Flag::NoSpace);

        d << ", world!";
    }

    /* No space, no newline at the end */
    CORRADE_COMPARE(out.str(), "Hello, world!");
}

void DebugTest::nospace() {
    /* Local nospace modifier, applied once */
    {
        std::ostringstream out;

        {
            Debug d{&out};
            d << "Value:" << 16;
            CORRADE_VERIFY(!(d.flags() & Debug::Flag::NoSpace));
            CORRADE_VERIFY(!(d.immediateFlags() & Debug::Flag::NoSpace));

            d << Debug::nospace;
            CORRADE_VERIFY(!(d.flags() & Debug::Flag::NoSpace));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::NoSpace));

            d << "," << 24 << "and more";
        }

        CORRADE_COMPARE(out.str(), "Value: 16, 24 and more\n");
    }

    /* Global nospace modifier, applied always */
    {
        std::ostringstream out;
        Debug d{&out, Debug::Flag::NoSpace};
        CORRADE_VERIFY((d.flags() & Debug::Flag::NoSpace));
        CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::NoSpace));

        d << "a" << "b" << "c";
        CORRADE_VERIFY((d.flags() & Debug::Flag::NoSpace));
        CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::NoSpace));
        CORRADE_COMPARE(out.str(), "abc");
    }
}

void DebugTest::newline() {
    std::ostringstream out;
    Debug(&out) << "Value:" << Debug::newline << 16;

    CORRADE_COMPARE(out.str(), "Value:\n16\n");
}

void DebugTest::space() {
    std::ostringstream out;
    Debug(&out) << Debug::space << Debug::space << "Value:" << Debug::space << 16;
    CORRADE_COMPARE(out.str(), "  Value: 16\n");
}

void DebugTest::noNewlineAtTheEnd() {
    std::ostringstream out1, out2, out3;

    Debug(&out1) << "Ahoy";
    Debug{&out1, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    Warning(&out2) << "Ahoy";
    Warning{&out2, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    Error(&out3) << "Ahoy";
    Error{&out3, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    CORRADE_COMPARE(out1.str(), "Ahoy\nHello");
    CORRADE_COMPARE(out2.str(), "Ahoy\nHello");
    CORRADE_COMPARE(out3.str(), "Ahoy\nHello");
}

constexpr const struct {
    const char* desc;
    Debug::Color color;
    char c;
} ColorsData[] = {
    #define _c(color) {#color, Debug::Color::color, char('0' + char(Debug::Color::color))},
    _c(Black)
    _c(Red)
    _c(Green)
    _c(Yellow)
    _c(Blue)
    _c(Magenta)
    _c(Cyan)
    _c(White)
    _c(Default)
    #undef _c
};

void DebugTest::colors() {
    auto&& data = ColorsData[testCaseInstanceId()];
    setTestCaseDescription(data.desc);
    auto fn = [&data](std::ostream& out) {
        Debug{&out}
            << Debug::color(data.color) << data.desc
            << Debug::boldColor(data.color) << "bold"
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            << Debug::space << Debug::invertedColor(data.color) << "inverted"
            #endif
            ;
    };

    fn(std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(), formatString(
        "\033[0;3{0}m{1}\033[0;1;3{0}m bold \033[0;7;3{0}minverted\033[0m\n",
        Containers::StringView{&data.c, 1}, data.desc));
    #endif
}

void DebugTest::colorsAutoReset() {
    /* Auto-reset at the end */
    auto fn = [](std::ostream& out) {
        Debug{&out} << "Default" << Debug::color(Debug::Color::Green) << "Green";
        Debug{&out} << "Default" << Debug::boldColor(Debug::Color::Green) << "Bold green";
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        Debug{&out} << "Default" << Debug::space << Debug::invertedColor(Debug::Color::Green) << "Inverted green"
        #endif
        ;
    };

    /* Print it for visual verification */
    fn(std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(),
        "Default\033[0;32m Green\033[0m\n"
        "Default\033[0;1;32m Bold green\033[0m\n"
        "Default \033[0;7;32mInverted green\033[0m\n");
    #endif
}

void DebugTest::colorsExplicitReset() {
    /* Don't reset twice */
    auto fn = [](std::ostream& out) {
        Debug{&out}
            << Debug::color(Debug::Color::Red) << "Red"
            << Debug::resetColor << "Default";
        Debug{&out}
            << Debug::boldColor(Debug::Color::Red) << "Bold red"
            << Debug::resetColor << "Default";
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        Debug{&out}
            << Debug::invertedColor(Debug::Color::Red) << "Inverted red"
            << Debug::resetColor << "Default";
        #endif
    };

    /* Print it for visual verification */
    fn(std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(),
        "\033[0;31mRed\033[0m Default\n"
        "\033[0;1;31mBold red\033[0m Default\n"
        "\033[0;7;31mInverted red\033[0m Default\n");
    #endif
}

#if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
void DebugTest::colorsBoldInvertedReset() {
    /* The bold and inverted style shouldn't carry over but get reset as well */
    auto fn = [](std::ostream& out) {
        Debug{&out}
            << Debug::boldColor(Debug::Color::Red) << "Bold red"
            << Debug::space << Debug::invertedColor(Debug::Color::Green) << "Non-bold inverted green"
            << Debug::color(Debug::Color::Blue) << "Non-inverted blue";
        Debug{&out}
            << Debug::invertedColor(Debug::Color::Red) << "Inverted red"
            << Debug::boldColor(Debug::Color::Green) << "Non-inverted bold green"
            << Debug::color(Debug::Color::Blue) << "Non-bold blue";
    };

    /* Print it for visual verification */
    fn(std::cout);

    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(),
        "\033[0;1;31mBold red \033[0;7;32mNon-bold inverted green\033[0;34m Non-inverted blue\033[0m\n"
        "\033[0;7;31mInverted red\033[0;1;32m Non-inverted bold green\033[0;34m Non-bold blue\033[0m\n");
}
#endif

void DebugTest::colorsDisabled() {
    /* Disabled globally */
    auto fn = [](std::ostream& out) {
        Debug{&out, Debug::Flag::DisableColors}
            << Debug::color(Debug::Color::Default) << "Default"
            << Debug::color(Debug::Color::Cyan) << "Default"
            << Debug::boldColor(Debug::Color::Red) << "Default"
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            << Debug::invertedColor(Debug::Color::Red) << "Default"
            #endif
            << Debug::resetColor;
    };

    /* Print it for visual verification */
    fn(std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(), "Default Default Default Default\n");
    #endif
}

void DebugTest::colorsNoOutput() {
    {
        Debug out{nullptr, Debug::Flag::DisableColors};
        out << Debug::color(Debug::Color::Red);

        Debug{&std::cout} << "This shouldn't be red.";
    } {
        Debug out{nullptr, Debug::Flag::DisableColors};
        out << Debug::boldColor(Debug::Color::Red);

        Debug{&std::cout} << "This shouldn't be bold red.";
    }
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    {
        Debug out{nullptr, Debug::Flag::DisableColors};
        out << Debug::invertedColor(Debug::Color::Red);

        Debug{&std::cout} << "This shouldn't be inverted red.";
    }
    #endif

    CORRADE_SKIP("Only possible to test visually.");
}

void DebugTest::colorsNospace() {
    /* Order of nospace and color modifiers shouldn't matter and give the same
       output */
    auto fn = [](std::ostream& out1, std::ostream& out2) {
        Debug{&out1} << "H"
            << Debug::color(Debug::Color::Blue) << Debug::nospace << "e"
            << Debug::boldColor(Debug::Color::Yellow) << Debug::nospace << "l"
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            << Debug::invertedColor(Debug::Color::Green)
            #endif
            << Debug::nospace << "l" << Debug::resetColor << Debug::nospace << "o";
        Debug{&out2} << "H"
            << Debug::nospace << Debug::color(Debug::Color::Blue) << "e"
            << Debug::nospace << Debug::boldColor(Debug::Color::Yellow) << "l"
            << Debug::nospace
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            << Debug::invertedColor(Debug::Color::Green)
            #endif
            << "l" << Debug::nospace << Debug::resetColor << "o";
    };

    /* Print it for visual verification */
    fn(std::cout, std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out1, out2;
    fn(out1, out2);
    CORRADE_COMPARE(out1.str(), "H\033[0;34me\033[0;1;33ml\033[0;7;32ml\033[0mo\n");
    CORRADE_COMPARE(out2.str(), "H\033[0;34me\033[0;1;33ml\033[0;7;32ml\033[0mo\n");
    #endif
}

void DebugTest::colorsScoped() {
    auto fn = [](std::ostream& out) {
        Debug{&out} << "This should have default color.";

        {
            Debug d{&out, Debug::Flag::NoNewlineAtTheEnd};
            d << Debug::color(Debug::Color::Cyan) << "This should be cyan." << Debug::newline;

            Debug{&out} << "This also,"
                << Debug::boldColor(Debug::Color::Blue) << "this bold blue,"
                << Debug::resetColor << "this again cyan and" << Debug::space
                #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
                << Debug::invertedColor(Debug::Color::Green)
                #endif
                << "this inverted green.";

            Debug{&out} << "This should be cyan again.";

            {
                Debug a{&out};
                a << Debug::color(Debug::Color::Yellow) << "This is yellow,";
                Debug b = Utility::move(a);
                b << "and the move-constructed instance should still correctly reset to cyan.";
            }

            Debug{&out, Debug::Flag::DisableColors} << "Disabling colors shouldn't affect outer scope, so also cyan.";
        } {
            Debug d{&out, Debug::Flag::NoNewlineAtTheEnd};
            d << Debug::boldColor(Debug::Color::Magenta) << "This should be bold magenta." << Debug::newline;

            Debug{&out} << "This also,"
                << Debug::color(Debug::Color::Blue) << "this non-bold blue,"
                << Debug::resetColor << "this again magenta and" << Debug::space
                #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
                << Debug::invertedColor(Debug::Color::Green)
                #endif
                << "this inverted green.";

            Debug{&out} << "This should be bold magenta again.";

            Debug{&out, Debug::Flag::DisableColors} << "Disabling colors shouldn't affect outer scope, so also bold magenta.";
        }
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        {
            Debug d{&out, Debug::Flag::NoNewlineAtTheEnd};
            d << Debug::invertedColor(Debug::Color::Yellow) << "This should be inverted yellow." << Debug::newline;

            Debug{&out} << "This also,"
                << Debug::boldColor(Debug::Color::Blue) << "this bold blue,"
                << Debug::resetColor << "this again yellow and"
                << Debug::color(Debug::Color::Green) << "this non-inverted green.";

            Debug{&out} << "This should be inverted yellow again.";

            Debug{&out, Debug::Flag::DisableColors} << "Disabling colors shouldn't affect outer scope, so also inverted yellow.";
        }
        #endif

        Debug{&out} << "And this resets back to default color.";
    };

    /* Print it for visual verification */
    fn(std::cout);

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    CORRADE_SKIP("Only possible to test visually on Windows.");
    #else
    std::ostringstream out;
    fn(out);
    CORRADE_COMPARE(out.str(),
        "This should have default color.\n"

        "\033[0;36mThis should be cyan.\n"
        "This also,\033[0;1;34m this bold blue,\033[0;36m this again cyan and \033[0;7;32mthis inverted green.\033[0;36m\n"
        "This should be cyan again.\n"
        "\033[0;33mThis is yellow, and the move-constructed instance should still correctly reset to cyan.\033[0;36m\n"
        "Disabling colors shouldn't affect outer scope, so also cyan.\n"
        "\033[0m"

        "\033[0;1;35mThis should be bold magenta.\n"
        "This also,\033[0;34m this non-bold blue,\033[0;1;35m this again magenta and \033[0;7;32mthis inverted green.\033[0;1;35m\n"
        "This should be bold magenta again.\n"
        "Disabling colors shouldn't affect outer scope, so also bold magenta.\n"
        "\033[0m"

        "\033[0;7;33mThis should be inverted yellow.\n"
        "This also,\033[0;1;34m this bold blue,\033[0;7;33m this again yellow and\033[0;32m this non-inverted green.\033[0;7;33m\n"
        "This should be inverted yellow again.\n"
        "Disabling colors shouldn't affect outer scope, so also inverted yellow.\n"
        "\033[0m"

        "And this resets back to default color.\n");
    #endif
}

void DebugTest::hex() {
    /* Local hex modifier, applied once */
    {
        std::ostringstream out;

        {
            Debug d{&out};
            d << "Values";
            CORRADE_VERIFY(!(d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY(!(d.immediateFlags() & Debug::Flag::Hex));

            d << Debug::hex;
            CORRADE_VERIFY(!(d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::Hex));

            d << 0xc0ffee;
            CORRADE_VERIFY(!(d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY(!(d.immediateFlags() & Debug::Flag::Hex));

            d << "and" << 16;
        }

        CORRADE_COMPARE(out.str(), "Values 0xc0ffee and 16\n");
    }

    /* Global hex modifier, applied always */
    {
        std::ostringstream out;
        {
            Debug d{&out, Debug::Flag::Hex};
            CORRADE_VERIFY((d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::Hex));

            /* Should work for any integer type without truncating, 0x should
               be printed for 0 as well */
            d << 0xfedcba9876543210ull << 0xcdu << static_cast<signed char>(0x13) << 0x0;
            CORRADE_VERIFY((d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::Hex));

            /* Shouldn't be applied to non-integer types but should still stay
               present for any that may come after */
            d << "yes" << 3.5f << false << 0xabc << U'\xabc';
            CORRADE_VERIFY((d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::Hex));

            /* Printing pointers applies it implicitly, check it doesn't
               cause 0x to be printed twice or the flag reset after */
            d << nullptr << reinterpret_cast<const void*>(std::size_t{0xc0ffee}) << 0x356;
            CORRADE_VERIFY((d.flags() & Debug::Flag::Hex));
            CORRADE_VERIFY((d.immediateFlags() & Debug::Flag::Hex));
        }

        CORRADE_COMPARE(out.str(),
            "0xfedcba9876543210 0xcd 0x13 0x0 "
            "yes 3.5 false 0xabc U+0ABC "
            "nullptr 0xc0ffee 0x356\n");
    }

    /* Negative values should have - before the 0x. Well, ideally, if iostreams
       weren't irreparably broken in the first place, printing everything as
       unsigned. */
    {
        std::ostringstream out;
        Debug{&out, Debug::Flag::Hex} << -0x356 << -0x1ll;

        {
            CORRADE_EXPECT_FAIL("This doesn't work as expected with std::hex anyway, won't bother fixing until iostreams are dropped.");
            CORRADE_COMPARE(out.str(), "-0x356 -0x1\n");
        }

        CORRADE_COMPARE(out.str(), "0xfffffcaa 0xffffffffffffffff\n");
    }

    /* Nested values should be printed as hex too, but it should be reset
       after */
    {
        std::ostringstream out;
        Debug{&out} << Debug::hex << Containers::pair(0xab, Containers::arrayView({0xcd, 0x13})) << 1234;
        CORRADE_COMPARE(out.str(), "{0xab, {0xcd, 0x13}} 1234\n");
    }
}

void DebugTest::valueAsColor() {
    Debug{} << "The following should be shades of gray:";

    {
        Debug d{Debug::Flag::Color|Debug::Flag::NoSpace};
        /* *not* 255 as that would be an infinite loop */
        for(unsigned char i = 0; i < 250; i += 7) d << i;
    }

    /* The modifier should work only for the immediately following value */
    std::ostringstream out;
    Debug{&out} << Debug::color << static_cast<unsigned char>(93)
        << Debug::color << static_cast<unsigned char>(173)
        << static_cast<unsigned char>(224);
    CORRADE_COMPARE(out.str(),
        "\033[38;2;93;93;93m\033[48;2;93;93;93m░░\033[0m "
        "\033[38;2;173;173;173m\033[48;2;173;173;173m▓▓\033[0m 224\n");
}

void DebugTest::valueAsColorColorsDisabled() {
    Debug{} << "The following should be uncolored shades of gray:";

    {
        Debug d{Debug::Flag::Color|Debug::Flag::DisableColors|Debug::Flag::NoSpace};
        /* *not* 255 as that would be an infinite loop */
        for(unsigned char i = 0; i < 250; i += 7) d << i;
    }

    /* The modifier should work only for the immediately following value */
    std::ostringstream out;
    Debug{&out, Debug::Flag::DisableColors}
        << Debug::color << static_cast<unsigned char>(93)
        << Debug::color << static_cast<unsigned char>(173)
        << static_cast<unsigned char>(224);
    CORRADE_COMPARE(out.str(), "░░ ▓▓ 224\n");
}

void DebugTest::iterable() {
    std::ostringstream out;
    Debug(&out) << Containers::arrayView({1, 2, 3});
    CORRADE_COMPARE(out.str(), "{1, 2, 3}\n");

    out.str({});
    Debug(&out) << std::set<std::string>{"a", "b", "c"};
    CORRADE_COMPARE(out.str(), "{a, b, c}\n");

    out.str({});
    Debug(&out) << std::map<int, std::string>{{1, "a"}, {2, "b"}, {3, "c"}};
    CORRADE_COMPARE(out.str(), "{(1, a), (2, b), (3, c)}\n");
}

void DebugTest::iterableNested() {
    std::ostringstream out;
    Debug{&out} << Containers::arrayView({
        Containers::arrayView({1, 2, 3}),
        Containers::arrayView({4, 5}),
        Containers::arrayView({6, 7, 8})
    });
    CORRADE_COMPARE(out.str(),
        "{{1, 2, 3},\n"
        " {4, 5},\n"
        " {6, 7, 8}}\n");
}

void DebugTest::iterablePacked() {
    {
        std::ostringstream out;
        Debug{&out} << Debug::packed << Containers::arrayView({1, 2, 3});
        CORRADE_COMPARE(out.str(), "123\n");
    }

    /* Nested containers should be printed packed too */
    {
        std::ostringstream out;
        Debug{&out} << Debug::packed << Containers::arrayView({
            Containers::arrayView({"a", "b", "c"}),
            Containers::arrayView({"d", "e"}),
            Containers::arrayView({"f", "g", "h"})
        });
        CORRADE_COMPARE(out.str(),
            "abc\n"
            "de\n"
            "fgh\n");
    }

    /* It's affecting just the immediately next item, so the one after
       shouldn't be packed */
    {
        std::ostringstream out;
        Debug(&out) << Debug::packed << std::set<std::string>{"a", "b", "c"}
            << Containers::arrayView({1, 2, 3});
        CORRADE_COMPARE(out.str(), "abc {1, 2, 3}\n");
    }
}

void DebugTest::iterableNospace() {
    /* The immediate nospace specifier should not be set for nested types */
    {
        std::ostringstream out;
        Debug{&out} << "array" << Debug::nospace
            << Containers::arrayView<std::pair<int, int>>({{1, 2}, {3, 4}});
        CORRADE_COMPARE(out.str(), "array{(1, 2), (3, 4)}\n");
    }

    /* The global nospace specifier should be preserved for nested types */
    {
        std::ostringstream out;
        Debug{&out, Debug::Flag::NoSpace} << "array"
            << Containers::arrayView<std::pair<int, int>>({{1, 2}, {3, 4}});
        CORRADE_COMPARE(out.str(), "array{(1,2), (3,4)}\n");
    }
}

void DebugTest::pair() {
    std::ostringstream out;
    Debug(&out) << std::make_pair("hey", 42);
    CORRADE_COMPARE(out.str(), "(hey, 42)\n");
}

void DebugTest::tuple() {
    std::ostringstream out;

    Debug(&out) << std::make_tuple();
    CORRADE_COMPARE(out.str(), "()\n");

    out.str({});
    Debug(&out) << std::make_tuple(3, 4.56, std::string{"hello"});
    CORRADE_COMPARE(out.str(), "(3, 4.56, hello)\n");
}

void DebugTest::iterablePairPacked() {
    /* Nested container should be printed packed, but the flag should get reset
       back after */
    std::ostringstream out;
    Debug{&out} << Debug::packed
        << std::make_pair(42, Containers::arrayView({1, 2, 3}))
        << Containers::arrayView({1, 2, 3});
    CORRADE_COMPARE(out.str(), "(42, 123) {1, 2, 3}\n");
}

void DebugTest::iterableTuplePacked() {
    /* Nested container should be printed packed, but the flag should get reset
       back after */
    std::ostringstream out;
    Debug{&out} << Debug::packed
        << std::make_tuple("hey", 42, Containers::arrayView({1, 2, 3}))
        << Containers::arrayView({1, 2, 3});
    CORRADE_COMPARE(out.str(), "(hey, 42, 123) {1, 2, 3}\n");
}

struct Bar {};
struct Baz {};

inline std::ostream& operator<<(std::ostream& o, const Bar&) {
    return o << "bar";
}

inline std::ostream& operator<<(std::ostream& o, const Baz&) {
    return o << "baz from ostream";
}

inline Debug& operator<<(Debug& debug, const Baz&) {
    return debug << "baz from Debug";
}

void DebugTest::ostreamFallback() {
    std::ostringstream out;
    Debug(&out) << Bar{};
    CORRADE_COMPARE(out.str(), "bar\n");
}

void DebugTest::ostreamFallbackPriority() {
    /* Suppress warning about unused function operator<<(std::ostream&, const Baz&) */
    {
        std::ostringstream o;
        o << Baz{};
    }

    std::ostringstream out;
    Debug(&out) << Baz{};
    CORRADE_COMPARE(out.str(), "baz from Debug\n");
}

void DebugTest::scopedOutput() {
    std::ostringstream debug1, debug2, warning1, warning2, error1, error2;

    Debug muteD{nullptr};
    Warning muteW{nullptr};
    Error muteE{nullptr};

    CORRADE_VERIFY(!Debug::output());
    CORRADE_VERIFY(!Warning::output());
    CORRADE_VERIFY(!Error::output());

    {
        Debug redirectD1{&debug1};
        Warning redirectW1{&warning1};
        Error redirectE1{&error1};

        CORRADE_VERIFY(Debug::output() == &debug1);
        CORRADE_VERIFY(Warning::output() == &warning1);
        CORRADE_VERIFY(Error::output() == &error1);

        Debug() << "hello";
        Warning() << "crazy";
        Error() << "world";

        {
            Debug redirectD2{&debug2};
            Warning redirectW2{&warning2};
            Error redirectE2{&error2};

            CORRADE_VERIFY(Debug::output() == &debug2);
            CORRADE_VERIFY(Warning::output() == &warning2);
            CORRADE_VERIFY(Error::output() == &error2);

            Debug() << "well";
            Warning() << "that";
            Error() << "smells";
        }

        CORRADE_VERIFY(Debug::output() == &debug1);
        CORRADE_VERIFY(Warning::output() == &warning1);
        CORRADE_VERIFY(Error::output() == &error1);

        Debug() << "how";
        Warning() << "are";
        Error() << "you?";
    }

    CORRADE_VERIFY(!Debug::output());
    CORRADE_VERIFY(!Warning::output());
    CORRADE_VERIFY(!Error::output());

    Debug() << "anyone";
    Warning() << "hears";
    Error() << "me?";

    CORRADE_COMPARE(debug1.str(), "hello\nhow\n");
    CORRADE_COMPARE(warning1.str(), "crazy\nare\n");
    CORRADE_COMPARE(error1.str(), "world\nyou?\n");

    CORRADE_COMPARE(debug2.str(), "well\n");
    CORRADE_COMPARE(warning2.str(), "that\n");
    CORRADE_COMPARE(error2.str(), "smells\n");
}

void DebugTest::move() {
    std::stringstream out;
    {
        Debug a{&out, Debug::Flag::Hex};
        a << "hello," << Debug::space;
        CORRADE_COMPARE(out.str(), "hello, ");
        CORRADE_COMPARE(a.flags(), Debug::Flag::Hex);
        CORRADE_COMPARE(a.immediateFlags(), Debug::Flag::Hex|Debug::Flag::NoSpace);

        /* This should correctly transfer all intermediate state and make the
           other one empty so it doesn't print a second newline at the end
           etc. */
        Debug b = Utility::move(a);
        CORRADE_COMPARE(b.flags(), Debug::Flag::Hex);
        CORRADE_COMPARE(b.immediateFlags(), Debug::Flag::Hex|Debug::Flag::NoSpace);
        b << 0xfeed;
    }
    CORRADE_COMPARE(out.str(), "hello, 0xfeed\n");
}

void DebugTest::stringOutput() {
    Containers::String debug, warning, error;

    Debug{&debug} << "a" << 33 << 0.567f;
    Warning{&warning} << "wow" << 42 << "oh" << "hi";
    Error{&error} << "hello hello hello hello hello";

    CORRADE_COMPARE(debug, "a 33 0.567\n");
    CORRADE_COMPARE(warning, "wow 42 oh hi\n");
    CORRADE_COMPARE(error, "hello hello hello hello hello\n");
}

void DebugTest::stringOutputNonEmpty() {
    Containers::String out{"this should be long enough to not be SSOd;"};
    CORRADE_VERIFY(!out.isSmall());

    Debug{&out} << "hey";
    CORRADE_COMPARE(out, "this should be long enough to not be SSOd;hey\n");
}

void DebugTest::stringOutputNonEmptySmall() {
    Containers::String out{"hey;"};
    CORRADE_VERIFY(out.isSmall());

    {
        /* Right after printing the original SSO string stays as it was before
           (isn't cleared, isn't appended to) */
        Debug d{&out};
        d << "hey";
        CORRADE_VERIFY(out.isSmall());
        CORRADE_COMPARE(out, "hey;");
    }

    /* Only on destruction it's replaced */
    CORRADE_COMPARE(out, "hey;hey\n");
    CORRADE_VERIFY(!out.isSmall());
}

void DebugTest::stringOutputScopedFlush() {
    /* A temporary instance populates the string right upon destruction, even
       with no newline at the end */
    {
        Containers::String out;
        Debug{&out, Debug::Flag::NoNewlineAtTheEnd} << "yello";
        CORRADE_COMPARE(out, "yello");
    }

    Containers::String out;
    {
        Debug redirectOutput{&out};

        /* Without a newline at the end, nothing is flushed */
        Debug{Debug::Flag::NoNewlineAtTheEnd} << "hi";
        CORRADE_COMPARE(out, "");

        /* With a newline it is */
        Debug{} << "hey!";
        CORRADE_COMPARE(out, "hihey!\n");

        /* Without a newline at the end but an explicit newline call it is
           also */
        Debug{Debug::Flag::NoNewlineAtTheEnd} << "hello?" << Debug::newline;
        CORRADE_COMPARE(out, "hihey!\nhello?\n");

        /* This one will get flushed only once the `redirectOutput` instance is
           destructed. Until then, the string storage is moved out to a
           growable array internally. */
        /** @todo clean up once the string is capable of growing */
        Debug{Debug::Flag::NoNewlineAtTheEnd} << "?!";
        CORRADE_COMPARE(out, "");
    }
    CORRADE_COMPARE(out, "hihey!\nhello?\n?!");
}

void DebugTest::stringOutputReuseGrowable() {
    Containers::String out;

    Debug{&out} << "a";
    const void* pointer = out.data();
    CORRADE_COMPARE(out, "a\n");

    /* Assuming the growable allocator allocates at least a size of a single
       pointer, three characters + \0 should fit without another reallocation.
       Cannot verify any other way as currently if arrayReserve() would be
       called from the test, it's not guaranteed that the Debug class
       internally would use the same allocator. */
    Debug{&out, Debug::Flag::NoNewlineAtTheEnd} << "b";
    CORRADE_COMPARE(out, "a\nb");
    CORRADE_COMPARE(out.data(), pointer);
}

void DebugTest::stringOutputReuseCleared() {
    Containers::String out;

    Debug{&out} << "hey";
    CORRADE_COMPARE(out, "hey\n");

    /* Emptying the string (and thus freeing the existing growable storage)
       shouldn't cause the stream to write to a garbage location */
    out = {};

    Debug{&out} << "hello";
    CORRADE_COMPARE(out, "hello\n");
}

void DebugTest::stringOutputReuseModified() {
    Containers::String out;

    Debug{&out} << "a";
    CORRADE_COMPARE(out, "a\n");

    /* Modifying the string (and thus also replacing the existing growable
       storage) shouldn't cause the contents to be discarded after */
    out = out + "heh;";

    Debug{&out} << "hello";
    CORRADE_COMPARE(out, "a\nheh;hello\n");
}

void DebugTest::stringOutputReuseModifiedUnsynced() {
    Containers::String out;

    {
        /* This doesn't sync so the output is empty */
        Debug debug{&out, Debug::Flag::NoNewlineAtTheEnd};
        debug << "hey";
        CORRADE_COMPARE(out, "");

        /* In this case, modifying the string would cause the modification to
           be lost on next write */
        out = "voila";

        debug << "hello";
    }
    CORRADE_COMPARE(out, "hey hello");
}

void DebugTest::stringOutputMove() {
    /* Like move() but with a String output. The internal stream is owned, so
       it should transfer the ownership, resulting in exactly one deletion --
       not two, and not leaking it either. */

    Containers::String out;
    {
        Debug a{&out, Debug::Flag::Hex};
        a << "hello," << Debug::space;
        CORRADE_COMPARE(a.flags(), Debug::Flag::Hex);
        CORRADE_COMPARE(a.immediateFlags(), Debug::Flag::Hex|Debug::Flag::NoSpace);

        Debug b = Utility::move(a);
        CORRADE_COMPARE(b.flags(), Debug::Flag::Hex);
        CORRADE_COMPARE(b.immediateFlags(), Debug::Flag::Hex|Debug::Flag::NoSpace);
        b << 0xfeed;
    }
    CORRADE_COMPARE(out, "hello, 0xfeed\n");
}

void DebugTest::debugColor() {
    std::ostringstream out;

    /* static_cast is needed to avoid "cast truncates constant value" on (x86)
       MSVC, sigh */
    Debug(&out) << Debug::Color::White << Debug::Color(char(static_cast<unsigned char>(0xde)));
    CORRADE_COMPARE(out.str(), "Utility::Debug::Color::White Utility::Debug::Color(0xde)\n");
}

void DebugTest::debugFlag() {
    std::ostringstream out;

    Debug(&out) << Debug::Flag::NoNewlineAtTheEnd << Debug::Flag(0xfe);
    CORRADE_COMPARE(out.str(), "Utility::Debug::Flag::NoNewlineAtTheEnd Utility::Debug::Flag(0xfe)\n");
}

void DebugTest::debugFlags() {
    std::ostringstream out;

    /* Only bit 5 and 6 is currently unused */
    Debug(&out) << (Debug::Flag::NoNewlineAtTheEnd|Debug::Flag::Packed|Debug::Flag(0x60)) << Debug::Flags{};
    CORRADE_COMPARE(out.str(), "Utility::Debug::Flag::NoNewlineAtTheEnd|Utility::Debug::Flag::Packed|Utility::Debug::Flag(0x60) Utility::Debug::Flags{}\n");
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void DebugTest::multithreaded() {
    std::ostream* defaultOutput = Debug::output();

    std::ostream* threadOutput = nullptr;
    std::ostringstream another;
    {
        Debug out{&another};

        std::thread t{[](std::ostream*& output) {
            output = Debug::output();
        }, std::ref(threadOutput)};

        t.join();
    }

    Debug{} << "CORRADE_BUILD_MULTITHREADED defined:" <<
        #ifdef CORRADE_BUILD_MULTITHREADED
        true
        #else
        false
        #endif
        ;

    Debug{} << "Output redirection visible in another thread:" << (threadOutput == &another);

    #ifdef CORRADE_BUILD_MULTITHREADED
    CORRADE_COMPARE(threadOutput, defaultOutput);
    #else
    CORRADE_COMPARE(threadOutput, &another);
    #endif
}
#endif

void DebugTest::sourceLocation() {
    std::ostringstream out;

    std::size_t line;
    {
        Debug redirect{&out};

        !Debug{} << "hello"; line = __LINE__;

        /* Verify that the source location gets preserved on move as well */
        Debug{Utility::move(!Debug{})} << "and this is from another line, a move-constructed instance even";

        !Debug{};

        Debug{} << "this no longer";
    }

    #ifdef CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED
    #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL) && _MSC_VER == 1937
    /* There's several bugreports related to this, with some even claiming that
       this exact issue was broken in MSVC 2019 and "fixed in 2022 17.7".
       Additionally, some reports are saying the __builtin_LINE() only works
       correctly with /std:c++20, while C++14 and 17 not; some are saying it
       only works if not used in a constexpr context... What a mess. The last
       link is currently (2023-09-16) saying "pending release", so I'm assuming
       17.8 / 1938 will have it fixed again:
        https://developercommunity.visualstudio.com/t/std::source_location-started-reporting-w/10429484
        https://developercommunity.visualstudio.com/t/VS2022-177-_MSC_VER--1937-broke-__/10437659
        https://developercommunity.visualstudio.com/t/Problem-with-inlining-and-constexpr-and-/10417907
        https://developercommunity.visualstudio.com/t/__builtin_LINE-function-is-reporting-w/10439054 */
    /** @todo or maybe just undef this macro on 19.37? */
    CORRADE_EXPECT_FAIL("MSVC 2022 19.37 (17.7) reports broken file/line location.");
    #endif
    CORRADE_COMPARE(out.str(), Utility::formatString(
        __FILE__ ":{}: hello\n"
        __FILE__ ":{}: and this is from another line, a move-constructed instance even\n"
        __FILE__ ":{}\n"
        "this no longer\n", line, line + 3, line + 5));
    #else
    CORRADE_COMPARE(out.str(),
        "hello\n"
        "and this is from another line, a move-constructed instance even\n"
        "this no longer\n");
    CORRADE_SKIP("Source location builtins not available.");
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DebugTest)
