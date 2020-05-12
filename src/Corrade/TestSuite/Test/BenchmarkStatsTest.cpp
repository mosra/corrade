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

#include "Corrade/TestSuite/Tester.h"

#include "Corrade/TestSuite/Implementation/BenchmarkStats.h"

namespace Corrade { namespace TestSuite { namespace Test { namespace {

struct BenchmarkStatsTest: Tester {
    explicit BenchmarkStatsTest();

    void calculateWhite();
    void calculateYellow();
    void calculateRed();
    void calculateNoValues();
    void calculateZeroBatchSize();
    void calculateSingleValue();

    void print();
};

enum: std::size_t { MultiplierDataCount = 14 };

constexpr const struct {
    const char* name;
    double multiplierMean;
    double multiplierStddev;
    TestSuite::Tester::BenchmarkUnits units;
    const char* expected;
} MultiplierData[MultiplierDataCount]{
    {"ones", 1.0, 1.0, TestSuite::Tester::BenchmarkUnits::Count,
        "153.70 ± 42.10    "},
    {"bytes", 1.0, 10.0, TestSuite::Tester::BenchmarkUnits::Bytes,
        "153.70 ± 421.00  B"},
    {"nanoseconds", 1.0, 10.0, TestSuite::Tester::BenchmarkUnits::Nanoseconds,
        "153.70 ± 421.00 ns"},
    {"thousands bytes mean", 1000.0, 10.0, TestSuite::Tester::BenchmarkUnits::Bytes,
        "150.10 ± 0.41   kB"},
    {"thousands cycles stddev", 10.0, 1000.0, TestSuite::Tester::BenchmarkUnits::Cycles,
        "  1.54 ± 42.10  kC"},
    {"microseconds", 1.0, 1000.0, TestSuite::Tester::BenchmarkUnits::Nanoseconds,
        "  0.15 ± 42.10  µs"},
    {"millions instructions mean", 1000000.0, 10000.0, TestSuite::Tester::BenchmarkUnits::Instructions,
        "153.70 ± 0.42   MI"},
    {"millions bytes stddev", 10000.0, 1000000.0, TestSuite::Tester::BenchmarkUnits::Bytes,
        "  1.47 ± 40.15  MB"},
    {"milliseconds", 1000000.0, 1000.0, TestSuite::Tester::BenchmarkUnits::Nanoseconds,
        "153.70 ± 0.04   ms"},
    {"billions bytes mean", 1000000000.0, 10000000.0, TestSuite::Tester::BenchmarkUnits::Bytes,
        "143.14 ± 0.39   GB"},
    {"billions stddev", 10000000.0, 1000000000.0, TestSuite::Tester::BenchmarkUnits::Count,
        "  1.54 ± 42.10  G "},
    {"seconds", 1000000.0, 100000000.0, TestSuite::Tester::BenchmarkUnits::Nanoseconds,
        "  0.15 ± 4.21    s"},
    {"no count", std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
        TestSuite::Tester::BenchmarkUnits::Instructions,
        "(no data)        I"},
    {"single time", 1000.0, std::numeric_limits<double>::quiet_NaN(),
        TestSuite::Tester::BenchmarkUnits::Nanoseconds,
        "153.70          µs"}
};

BenchmarkStatsTest::BenchmarkStatsTest() {
    addTests({&BenchmarkStatsTest::calculateWhite,
              &BenchmarkStatsTest::calculateYellow,
              &BenchmarkStatsTest::calculateRed,
              &BenchmarkStatsTest::calculateNoValues,
              &BenchmarkStatsTest::calculateZeroBatchSize,
              &BenchmarkStatsTest::calculateSingleValue});

    addInstancedTests({&BenchmarkStatsTest::print}, MultiplierDataCount);
}

/* Stolen from https://en.wikipedia.org/wiki/Standard_deviation */
constexpr const std::uint64_t Measurements[] = { 20, 40, 40, 40, 50, 50, 70, 90 };

void BenchmarkStatsTest::calculateWhite() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats(Measurements, 10, 1.0, 2.0);

    CORRADE_COMPARE(mean, 5.0);
    /* Not 2, because we're dividing by N-1 */
    CORRADE_COMPARE(stddev, 2.138089935299395);
    CORRADE_COMPARE(color, Debug::Color::Default);
}

void BenchmarkStatsTest::calculateYellow() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats(Measurements, 10, 0.4, 2.0);

    CORRADE_COMPARE(mean, 5.0);
    CORRADE_COMPARE(stddev, 2.138089935299395);
    CORRADE_COMPARE(color, Debug::Color::Yellow);
}

void BenchmarkStatsTest::calculateRed() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats(Measurements, 10, 0.05, 0.4);

    CORRADE_COMPARE(mean, 5.0);
    CORRADE_COMPARE(stddev, 2.138089935299395);
    CORRADE_COMPARE(color, Debug::Color::Red);
}

void BenchmarkStatsTest::calculateNoValues() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats({}, 10, 0.05, 0.25);

    CORRADE_COMPARE(mean, std::numeric_limits<double>::quiet_NaN());
    CORRADE_COMPARE(stddev, std::numeric_limits<double>::quiet_NaN());
    CORRADE_COMPARE(color, Debug::Color::Red);
}

void BenchmarkStatsTest::calculateZeroBatchSize() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats(Measurements, 0, 0.05, 0.4);

    CORRADE_COMPARE(mean, std::numeric_limits<double>::quiet_NaN());
    CORRADE_COMPARE(stddev, std::numeric_limits<double>::quiet_NaN());
    CORRADE_COMPARE(color, Debug::Color::Red);
}

void BenchmarkStatsTest::calculateSingleValue() {
    double mean, stddev;
    Debug::Color color;
    std::tie(mean, stddev, color) = Implementation::calculateStats(Containers::ArrayView<const std::uint64_t>{Measurements}.slice(4, 5), 10, 0.05, 0.25);

    CORRADE_COMPARE(mean, 5.0);
    CORRADE_COMPARE(stddev, std::numeric_limits<double>::quiet_NaN());
    CORRADE_COMPARE(color, Debug::Color::Default);
}

void BenchmarkStatsTest::print() {
    setTestCaseDescription(MultiplierData[testCaseInstanceId()].name);

    std::ostringstream str;
    Debug out{&str, Debug::Flag::DisableColors};

    Implementation::printStats(out, 153.70*MultiplierData[testCaseInstanceId()].multiplierMean,
        42.10*MultiplierData[testCaseInstanceId()].multiplierStddev, Utility::Debug::Color::Default, MultiplierData[testCaseInstanceId()].units);

    CORRADE_COMPARE(str.str(),
        MultiplierData[testCaseInstanceId()].expected);
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::BenchmarkStatsTest)
