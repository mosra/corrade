#ifndef Corrade_TestSuite_Implementation_BenchmarkStats_h
#define Corrade_TestSuite_Implementation_BenchmarkStats_h
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

#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#ifdef _MSC_VER
#include <algorithm> /* std::max() */
#endif

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/StlMath.h"

namespace Corrade { namespace TestSuite { namespace Implementation {

inline std::tuple<double, double, Utility::Debug::Color> calculateStats(const Containers::ArrayView<const std::uint64_t> measurements, const std::size_t batchSize, const double yellowThreshold, const double redThreshold) {
    if(measurements.empty() || !batchSize)
        return std::make_tuple(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), Utility::Debug::Color::Red);

    /* Calculate mean */
    double mean{};
    for(std::uint64_t v: measurements)
        mean += double(v)/double(batchSize*measurements.size());

    /* Calculate sample variance */
    double variance{};
    for(std::uint64_t v: measurements) {
        double dev = double(v)/batchSize - mean;
        variance += dev*dev/(measurements.size() - 1);
    }

    /* Calculate sample standard deviation */
    const double stddev = std::sqrt(variance);

    /* If variance is above 50% of the mean, it's bad, if above 10%, it's not
       quite right */
    Utility::Debug::Color color;
    const double absavg = std::abs(mean);
    if(stddev >= absavg*redThreshold)
        color = Utility::Debug::Color::Red;
    else if(stddev >= absavg*yellowThreshold)
        color = Utility::Debug::Color::Yellow;
    else
        color = Utility::Debug::Color::Default;

    return std::make_tuple(mean, stddev, color);
}

inline void printValue(Utility::Debug& out, const double mean, const double stddev, const Utility::Debug::Color color, const double divisor, const char* const unitPrefix, const char* const unit) {
    std::ostringstream meanFormatter, stddevFormatter;
    meanFormatter << std::right << std::fixed << std::setprecision(2) << std::setw(6) << mean/divisor;
    stddevFormatter << std::left << std::fixed << std::setprecision(2) << std::setw(6) << stddev/divisor;

    /* No data */
    if(mean != mean) {
        out << Utility::Debug::boldColor(Utility::Debug::Color::Red)
            << "(no data)       " << Utility::Debug::resetColor << unit;

    /* Only a single sample, omit stddev */
    } else if(stddev != stddev) {
        out << Utility::Debug::boldColor(Utility::Debug::Color::Green)
            << meanFormatter.str() << Utility::Debug::resetColor << "        "
            << unitPrefix << Utility::Debug::nospace << unit;

    /* Mean + stddev */
    } else {
        out << Utility::Debug::boldColor(Utility::Debug::Color::Green)
            << meanFormatter.str() << "±" << Utility::Debug::boldColor(color)
            << stddevFormatter.str() << Utility::Debug::resetColor
            << unitPrefix << Utility::Debug::nospace << unit;
    }
}

inline void printTime(Utility::Debug& out, const double mean, const double stddev, const Utility::Debug::Color color) {
    const double max = std::max(mean, stddev);

    if(max >= 1000000000.0)
        printValue(out, mean, stddev, color, 1000000000.0, " ", "s");
    else if(max >= 1000000.0)
        printValue(out, mean, stddev, color, 1000000.0, "m", "s");
    else if(max >= 1000.0)
        printValue(out, mean, stddev, color, 1000.0, "µ", "s");
    else
        printValue(out, mean, stddev, color, 1.0, "n", "s");
}

inline void printCount(Utility::Debug& out, const double mean, const double stddev, const Utility::Debug::Color color, double multiplier, const char* const unit) {
    double max = std::max(mean, stddev);

    if(max >= multiplier*multiplier*multiplier)
        printValue(out, mean, stddev, color, multiplier*multiplier*multiplier, "G", unit);
    else if(max >= multiplier*multiplier)
        printValue(out, mean, stddev, color, multiplier*multiplier, "M", unit);
    else if(max >= multiplier)
        printValue(out, mean, stddev, color, multiplier, "k", unit);
    else
        printValue(out, mean, stddev, color, 1.0, " ", unit);
}

inline void printStats(Utility::Debug& out, const double mean, const double stddev, const Utility::Debug::Color color, const Tester::BenchmarkUnits unit) {
    switch(unit) {
        case Tester::BenchmarkUnits::Nanoseconds:
            printTime(out, mean, stddev, color);
            return;
        case Tester::BenchmarkUnits::Cycles:
            printCount(out, mean, stddev, color, 1000.0, "C");
            return;
        case Tester::BenchmarkUnits::Instructions:
            printCount(out, mean, stddev, color, 1000.0, "I");
            return;
        case Tester::BenchmarkUnits::Bytes:
            printCount(out, mean, stddev, color, 1024.0, "B");
            return;
        case Tester::BenchmarkUnits::Count:
            printCount(out, mean, stddev, color, 1000.0, " ");
            return;
    }

    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

}}}

#endif
