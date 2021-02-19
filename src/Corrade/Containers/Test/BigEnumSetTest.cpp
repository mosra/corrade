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

#include <sstream>

#include "Corrade/Containers/BigEnumSet.hpp"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct BigEnumSetTest: TestSuite::Tester {
    explicit BigEnumSetTest();

    /* Most of the test is copied from EnumSetTest, with enum values adapted to
       cover the full 256-bit range */

    void size();

    void constructDefault();
    void construct();
    void constructOutOfRange();
    void constructNoInit();

    void operatorOr();
    void operatorAnd();
    void operatorXor();
    void operatorBool();
    void operatorInverse();
    void compare();

    void templateFriendOperators();

    void debug();
};

BigEnumSetTest::BigEnumSetTest() {
    addTests({&BigEnumSetTest::size,

              &BigEnumSetTest::constructDefault,
              &BigEnumSetTest::construct,
              &BigEnumSetTest::constructOutOfRange,
              &BigEnumSetTest::constructNoInit,

              &BigEnumSetTest::operatorOr,
              &BigEnumSetTest::operatorAnd,
              &BigEnumSetTest::operatorXor,
              &BigEnumSetTest::operatorBool,
              &BigEnumSetTest::operatorInverse,
              &BigEnumSetTest::compare,

              &BigEnumSetTest::templateFriendOperators,

              &BigEnumSetTest::debug});
}

enum class Feature: std::uint16_t {
    Fast = 1,
    Cheap = 41,
    Tested = 66,
    Popular = 197
};

enum: std::uint64_t {
    FastBit = 1ull << 1,
    CheapBit = 1ull << 41,
    TestedBit = 1ull << 2, /* 66 % 64 */
    PopularBit = 1ull << 5, /* 197 % 64 */
};

typedef BigEnumSet<Feature, 4> Features;

CORRADE_ENUMSET_OPERATORS(Features)

void BigEnumSetTest::size() {
    enum class Byte: std::uint8_t {};
    CORRADE_COMPARE(sizeof(BigEnumSet<Byte>), 32);
    CORRADE_COMPARE(BigEnumSet<Byte>::Size, 4);

    enum class Short: std::uint16_t {};
    /* BigEnumSet<Short> won't compile, as its size is 8 kB, which is above the
       1 kB limit */
    CORRADE_COMPARE(sizeof(BigEnumSet<Short, 1>), 8);
    CORRADE_COMPARE(sizeof(BigEnumSet<Short, 128>), 1024);

    enum class Int: std::uint32_t {};
    CORRADE_COMPARE(sizeof(BigEnumSet<Int, 128>), 1024);

    enum class Long: std::uint64_t {};
    CORRADE_COMPARE(sizeof(BigEnumSet<Long, 128>), 1024);
}

constexpr Features CNoFeatures;

void BigEnumSetTest::constructDefault() {
    CORRADE_COMPARE(Features::Size, 4);

    Features noFeatures;
    for(std::size_t i = 0; i != Features::Size; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(noFeatures.data()[i], 0);
    }

    /* The variable has to have a global scope in order to access its internals
       via a pointer. Works everywhere except MSVC2017 (MSVC2015 too!), I'll
       just shake that off as some weird temporary bug. */
    #if defined(CORRADE_MSVC2015_COMPATIBILITY) || !defined(CORRADE_MSVC2017_COMPATIBILITY)
    constexpr
    #endif
    const std::uint64_t* cData = CNoFeatures.data();
    for(std::size_t i = 0; i != Features::Size; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(cData[i], 0);
    }

    /* Useful when BigEnumSet{} is a default value in some constructor -- in
       that case not having it noexcept means the constructor call isn't
       noexcept either */
    CORRADE_VERIFY(std::is_nothrow_default_constructible<Features>::value);
}

constexpr Features CFeatures = Feature::Tested;

void BigEnumSetTest::construct() {
    {
        Features features = Feature::Fast;
        CORRADE_COMPARE(features.data()[0], FastBit);
        CORRADE_COMPARE(features.data()[1], 0);
        CORRADE_COMPARE(features.data()[2], 0);
        CORRADE_COMPARE(features.data()[3], 0);
    } {
        Features features = Feature::Cheap;
        CORRADE_COMPARE(features.data()[0], CheapBit);
        CORRADE_COMPARE(features.data()[1], 0);
        CORRADE_COMPARE(features.data()[2], 0);
        CORRADE_COMPARE(features.data()[3], 0);
    } {
        Features features = Feature::Tested;
        CORRADE_COMPARE(features.data()[0], 0);
        CORRADE_COMPARE(features.data()[1], TestedBit);
        CORRADE_COMPARE(features.data()[2], 0);
        CORRADE_COMPARE(features.data()[3], 0);
    } {
        Features features = Feature::Popular;
        CORRADE_COMPARE(features.data()[0], 0);
        CORRADE_COMPARE(features.data()[1], 0);
        CORRADE_COMPARE(features.data()[2], 0);
        CORRADE_COMPARE(features.data()[3], PopularBit);
    }

    /* The variable has to have a global scope in order to access its internals
       via a pointer. Works everywhere except MSVC2017 (MSVC2015 too!), I'll
       just shake that off as some weird temporary bug. */
    #if defined(CORRADE_MSVC2015_COMPATIBILITY) || !defined(CORRADE_MSVC2017_COMPATIBILITY)
    constexpr
    #endif
    const std::uint64_t* cData = CFeatures.data();
    CORRADE_COMPARE(cData[0], 0);
    CORRADE_COMPARE(cData[1], TestedBit);
    CORRADE_COMPARE(cData[2], 0);
    CORRADE_COMPARE(cData[3], 0);

    /* Useful when a BigEnumSet is a default value in some constructor -- in
       that case not having it noexcept means the constructor call isn't
       noexcept either */
    /** @todo this should be for all the operators as well, sigh */
    CORRADE_VERIFY((std::is_nothrow_constructible<Features, Feature>::value));
}

void BigEnumSetTest::constructOutOfRange() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    /* These are fine, shouldn't warn */
    Features{Feature(255)};
    BigEnumSet<Feature, 7>{Feature(447)};

    std::ostringstream out;
    Error redirectError{&out};
    Features{Feature(0xdead)};
    BigEnumSet<Feature, 7>{Feature(448)};
    CORRADE_COMPARE(out.str(),
        "Containers::BigEnumSet: value 57005 too large for a 256-bit storage\n"
        "Containers::BigEnumSet: value 448 too large for a 448-bit storage\n");
}

void BigEnumSetTest::constructNoInit() {
    {
        Features features{Feature::Tested};
        new(&features) Features{};
        for(std::size_t i = 0; i != Features::Size; ++i) {
            CORRADE_ITERATION(i);
            CORRADE_COMPARE(features.data()[i], 0);
        }
    } {
        Features features{Feature::Tested};
        new(&features) Features{NoInit};
        #if defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        #endif
        CORRADE_COMPARE(features.data()[1], TestedBit);
        #if defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        #pragma GCC diagnostic pop
        #endif
    }
}

void BigEnumSetTest::operatorOr() {
    Features features = Feature::Cheap|Feature::Fast;
    CORRADE_COMPARE(features.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(features.data()[1], 0);
    CORRADE_COMPARE(features.data()[2], 0);
    CORRADE_COMPARE(features.data()[3], 0);

    Features features2a = features|Feature::Popular;
    CORRADE_COMPARE(features2a.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(features2a.data()[1], 0);
    CORRADE_COMPARE(features2a.data()[2], 0);
    CORRADE_COMPARE(features2a.data()[3], PopularBit);

    Features features2b = Feature::Popular|features;
    CORRADE_COMPARE(features2b.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(features2b.data()[1], 0);
    CORRADE_COMPARE(features2b.data()[2], 0);
    CORRADE_COMPARE(features2b.data()[3], PopularBit);

    features |= Feature::Popular;
    CORRADE_COMPARE(features.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(features.data()[1], 0);
    CORRADE_COMPARE(features.data()[2], 0);
    CORRADE_COMPARE(features.data()[3], PopularBit);

    constexpr Features cFeatures = Feature::Cheap|Feature::Fast;
    CORRADE_COMPARE(cFeatures.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(cFeatures.data()[1], 0);
    CORRADE_COMPARE(cFeatures.data()[2], 0);
    CORRADE_COMPARE(cFeatures.data()[3], 0);

    constexpr Features cFeatures2a = cFeatures|Feature::Popular;
    CORRADE_COMPARE(cFeatures2a.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(cFeatures2a.data()[1], 0);
    CORRADE_COMPARE(cFeatures2a.data()[2], 0);
    CORRADE_COMPARE(cFeatures2a.data()[3], PopularBit);

    constexpr Features cFeatures2b = Feature::Popular|cFeatures;
    CORRADE_COMPARE(cFeatures2b.data()[0], CheapBit|FastBit);
    CORRADE_COMPARE(cFeatures2b.data()[1], 0);
    CORRADE_COMPARE(cFeatures2b.data()[2], 0);
    CORRADE_COMPARE(cFeatures2b.data()[3], PopularBit);
}

void BigEnumSetTest::operatorAnd() {
    Features none = Feature::Cheap & Feature::Fast;
    CORRADE_COMPARE(none.data()[0], 0);
    CORRADE_COMPARE(none.data()[1], 0);
    CORRADE_COMPARE(none.data()[2], 0);
    CORRADE_COMPARE(none.data()[3], 0);

    Features features = Feature::Popular|Feature::Fast|Feature::Cheap;
    Features featuresAndPopularA = features & Feature::Popular;
    CORRADE_COMPARE(featuresAndPopularA.data()[0], 0);
    CORRADE_COMPARE(featuresAndPopularA.data()[1], 0);
    CORRADE_COMPARE(featuresAndPopularA.data()[2], 0);
    CORRADE_COMPARE(featuresAndPopularA.data()[3], PopularBit);

    Features featuresAndPopularB = Feature::Popular & features;
    CORRADE_COMPARE(featuresAndPopularB.data()[0], 0);
    CORRADE_COMPARE(featuresAndPopularB.data()[1], 0);
    CORRADE_COMPARE(featuresAndPopularB.data()[2], 0);
    CORRADE_COMPARE(featuresAndPopularB.data()[3], PopularBit);

    Features featuresAndTested = features & Feature::Tested;
    CORRADE_COMPARE(featuresAndTested.data()[0], 0);
    CORRADE_COMPARE(featuresAndTested.data()[1], 0);
    CORRADE_COMPARE(featuresAndTested.data()[2], 0);
    CORRADE_COMPARE(featuresAndTested.data()[3], 0);

    Features features2 = Feature::Popular|Feature::Fast|Feature::Tested;
    Features featuresAndFeatures2 = features & features2;
    CORRADE_COMPARE(featuresAndFeatures2.data()[0], FastBit);
    CORRADE_COMPARE(featuresAndFeatures2.data()[1], 0);
    CORRADE_COMPARE(featuresAndFeatures2.data()[2], 0);
    CORRADE_COMPARE(featuresAndFeatures2.data()[3], PopularBit);

    features &= features2;
    CORRADE_COMPARE(features.data()[0], FastBit);
    CORRADE_COMPARE(features.data()[1], 0);
    CORRADE_COMPARE(features.data()[2], 0);
    CORRADE_COMPARE(features.data()[3], PopularBit);

    constexpr Features cFeatures = Feature::Popular|Feature::Fast|Feature::Cheap;
    constexpr Features cFeaturesAndPopularA = cFeatures & Feature::Popular;
    CORRADE_COMPARE(cFeaturesAndPopularA.data()[0], 0);
    CORRADE_COMPARE(cFeaturesAndPopularA.data()[1], 0);
    CORRADE_COMPARE(cFeaturesAndPopularA.data()[2], 0);
    CORRADE_COMPARE(cFeaturesAndPopularA.data()[3], PopularBit);

    constexpr Features cFeaturesAndPopularB = Feature::Popular & cFeatures;
    CORRADE_COMPARE(cFeaturesAndPopularB.data()[0], 0);
    CORRADE_COMPARE(cFeaturesAndPopularB.data()[1], 0);
    CORRADE_COMPARE(cFeaturesAndPopularB.data()[2], 0);
    CORRADE_COMPARE(cFeaturesAndPopularB.data()[3], PopularBit);
}

void BigEnumSetTest::operatorXor() {
    Features none = Feature::Cheap ^ Feature::Cheap;
    CORRADE_COMPARE(none.data()[0], 0);
    CORRADE_COMPARE(none.data()[1], 0);
    CORRADE_COMPARE(none.data()[2], 0);
    CORRADE_COMPARE(none.data()[3], 0);

    Features cheapAndFast = Feature::Cheap ^ Feature::Fast;
    CORRADE_COMPARE(cheapAndFast.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(cheapAndFast.data()[1], 0);
    CORRADE_COMPARE(cheapAndFast.data()[2], 0);
    CORRADE_COMPARE(cheapAndFast.data()[3], 0);

    Features features = Feature::Popular|Feature::Fast|Feature::Cheap;
    Features featuresXorTestedA = features ^ Feature::Tested;
    CORRADE_COMPARE(featuresXorTestedA.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(featuresXorTestedA.data()[1], TestedBit);
    CORRADE_COMPARE(featuresXorTestedA.data()[2], 0);
    CORRADE_COMPARE(featuresXorTestedA.data()[3], PopularBit);

    Features featuresXorTestedB = Feature::Tested ^ features;
    CORRADE_COMPARE(featuresXorTestedB.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(featuresXorTestedB.data()[1], TestedBit);
    CORRADE_COMPARE(featuresXorTestedB.data()[2], 0);
    CORRADE_COMPARE(featuresXorTestedB.data()[3], PopularBit);

    Features featuresXorPopular = features ^ Feature::Popular;
    CORRADE_COMPARE(featuresXorPopular.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(featuresXorPopular.data()[1], 0);
    CORRADE_COMPARE(featuresXorPopular.data()[2], 0);
    CORRADE_COMPARE(featuresXorPopular.data()[3], 0);

    Features features2 = Feature::Popular|Feature::Fast|Feature::Tested;
    Features features3 = features ^ features2;
    CORRADE_COMPARE(features3.data()[0], CheapBit);
    CORRADE_COMPARE(features3.data()[1], TestedBit);
    CORRADE_COMPARE(features3.data()[2], 0);
    CORRADE_COMPARE(features3.data()[3], 0);

    features ^= features2;
    CORRADE_COMPARE(features.data()[0], CheapBit);
    CORRADE_COMPARE(features.data()[1], TestedBit);
    CORRADE_COMPARE(features.data()[2], 0);
    CORRADE_COMPARE(features.data()[3], 0);

    constexpr Features cFeatures = Feature::Popular|Feature::Fast|Feature::Cheap;
    constexpr Features cFeaturesXorTestedA = cFeatures ^ Feature::Tested;
    CORRADE_COMPARE(cFeaturesXorTestedA.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(cFeaturesXorTestedA.data()[1], TestedBit);
    CORRADE_COMPARE(cFeaturesXorTestedA.data()[2], 0);
    CORRADE_COMPARE(cFeaturesXorTestedA.data()[3], PopularBit);

    constexpr Features cFeaturesXorTestedB = Feature::Tested ^ cFeatures;
    CORRADE_COMPARE(cFeaturesXorTestedB.data()[0], FastBit ^ CheapBit);
    CORRADE_COMPARE(cFeaturesXorTestedB.data()[1], TestedBit);
    CORRADE_COMPARE(cFeaturesXorTestedB.data()[2], 0);
    CORRADE_COMPARE(cFeaturesXorTestedB.data()[3], PopularBit);
}

void BigEnumSetTest::operatorBool() {
    CORRADE_COMPARE(!!(Features()), false);

    Features features = Feature::Cheap|Feature::Fast;
    CORRADE_COMPARE(!!(features & Feature::Popular), false);
    CORRADE_COMPARE(!!(features & Feature::Cheap), true);

    constexpr Features cFeatures = Feature::Cheap|Feature::Fast;
    constexpr bool cFeatures1 = !!(cFeatures & Feature::Popular);
    constexpr bool cFeatures2 = !!(cFeatures & Feature::Cheap);
    CORRADE_VERIFY(!cFeatures1);
    CORRADE_VERIFY(cFeatures2);
}

void BigEnumSetTest::operatorInverse() {
    Features inverse = ~Features();
    CORRADE_COMPARE(inverse.data()[0], 0xffffffffffffffffull);
    CORRADE_COMPARE(inverse.data()[1], 0xffffffffffffffffull);
    CORRADE_COMPARE(inverse.data()[2], 0xffffffffffffffffull);
    CORRADE_COMPARE(inverse.data()[3], 0xffffffffffffffffull);

    Features popularCheapInverse = ~(Feature::Popular|Feature::Cheap);
    CORRADE_COMPARE(popularCheapInverse.data()[0], ~CheapBit);
    CORRADE_COMPARE(popularCheapInverse.data()[1], 0xffffffffffffffffull);
    CORRADE_COMPARE(popularCheapInverse.data()[2], 0xffffffffffffffffull);
    CORRADE_COMPARE(popularCheapInverse.data()[3], ~PopularBit);

    Features popularInverse = ~Feature::Popular;
    CORRADE_COMPARE(popularInverse.data()[0], 0xffffffffffffffffull);
    CORRADE_COMPARE(popularInverse.data()[1], 0xffffffffffffffffull);
    CORRADE_COMPARE(popularInverse.data()[2], 0xffffffffffffffffull);
    CORRADE_COMPARE(popularInverse.data()[3], ~PopularBit);

    constexpr Features cInverse = ~Features{};
    CORRADE_COMPARE(cInverse.data()[0], 0xffffffffffffffffull);
    CORRADE_COMPARE(cInverse.data()[1], 0xffffffffffffffffull);
    CORRADE_COMPARE(cInverse.data()[2], 0xffffffffffffffffull);
    CORRADE_COMPARE(cInverse.data()[3], 0xffffffffffffffffull);

    constexpr Features cPopularCheapInverse = ~(Feature::Popular|Feature::Cheap);
    CORRADE_COMPARE(cPopularCheapInverse.data()[0], ~CheapBit);
    CORRADE_COMPARE(cPopularCheapInverse.data()[1], 0xffffffffffffffffull);
    CORRADE_COMPARE(cPopularCheapInverse.data()[2], 0xffffffffffffffffull);
    CORRADE_COMPARE(cPopularCheapInverse.data()[3], ~PopularBit);
}

void BigEnumSetTest::compare() {
    Features features = Feature::Popular|Feature::Fast|Feature::Cheap;
    CORRADE_VERIFY(features == features);
    CORRADE_VERIFY(!(features != features));
    CORRADE_VERIFY(Feature::Cheap == Features(Feature::Cheap));
    CORRADE_VERIFY(Feature::Cheap != Features(Feature::Popular));

    CORRADE_VERIFY(Features() <= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular >= Features());
    CORRADE_VERIFY(Feature::Popular <= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular >= Feature::Popular);
    CORRADE_VERIFY(Feature::Popular <= features);
    CORRADE_VERIFY(features >= Feature::Popular);
    CORRADE_VERIFY(features <= features);
    CORRADE_VERIFY(features >= features);

    CORRADE_VERIFY(features <= (Feature::Popular|Feature::Fast|Feature::Cheap|Feature::Tested));
    CORRADE_VERIFY(!(features >= (Feature::Popular|Feature::Fast|Feature::Cheap|Feature::Tested)));

    constexpr Features cFeatures = Feature::Popular|Feature::Fast|Feature::Cheap;
    constexpr bool cFeaturesEqual = cFeatures == cFeatures;
    constexpr bool cFeaturesNonEqual = cFeatures != cFeatures;
    constexpr bool cFeaturesLessEqual = cFeatures <= cFeatures;
    constexpr bool cFeaturesGreaterEqual = cFeatures >= cFeatures;
    CORRADE_VERIFY(cFeaturesEqual);
    CORRADE_VERIFY(!cFeaturesNonEqual);
    CORRADE_VERIFY(cFeaturesLessEqual);
    CORRADE_VERIFY(cFeaturesGreaterEqual);
}

template<class T> struct Foo {
    enum class Flag: std::uint8_t {
        A = 25,
        B = 77
    };

    typedef BigEnumSet<Flag> Flags;
    CORRADE_ENUMSET_FRIEND_OPERATORS(Flags)
};

void BigEnumSetTest::templateFriendOperators() {
    Foo<int>::Flags a = Foo<int>::Flag::A & ~Foo<int>::Flag::B;
    CORRADE_COMPARE(a.data()[0], 1 << 25);
    CORRADE_COMPARE(a.data()[1], 0);
    CORRADE_COMPARE(a.data()[2], 0);
    CORRADE_COMPARE(a.data()[3], 0);
}

Utility::Debug& operator<<(Utility::Debug& debug, Feature value) {
    switch(value) {
        #define _c(value) case Feature::value: return debug << "Feature::" #value;
        _c(Fast)
        _c(Cheap)
        _c(Tested)
        _c(Popular)
        #undef _c
    }

    return debug << "Feature(" << Utility::Debug::nospace << reinterpret_cast<void*>(int(value)) << Utility::Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, Features value) {
    return bigEnumSetDebugOutput(debug, value, "Features{}");
}

void BigEnumSetTest::debug() {
    std::stringstream out;

    Utility::Debug{&out} << Features{} << (Feature::Fast|Feature::Cheap) << (Feature(0xfa)|Feature(0xcd)|Feature::Popular);
    CORRADE_COMPARE(out.str(), "Features{} Feature::Fast|Feature::Cheap Feature::Popular|Feature(0xcd)|Feature(0xfa)\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::BigEnumSetTest)
