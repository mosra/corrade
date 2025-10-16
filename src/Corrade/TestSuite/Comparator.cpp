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

#include "Comparator.h"

#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/StringView.h"

namespace Corrade { namespace TestSuite {

Utility::Debug& operator<<(Utility::Debug& debug, const ComparisonStatusFlag value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case ComparisonStatusFlag::value: return debug << "TestSuite::ComparisonStatusFlag::" #value;
        _c(Failed)
        _c(Warning)
        _c(Message)
        _c(Verbose)
        _c(Diagnostic)
        _c(VerboseDiagnostic)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "TestSuite::ComparisonStatusFlag(" << Utility::Debug::nospace << Utility::Debug::hex << static_cast<unsigned char>(value) << Utility::Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, const ComparisonStatusFlags value) {
    return Containers::enumSetDebugOutput(debug, value, "TestSuite::ComparisonStatusFlags{}", {
        ComparisonStatusFlag::Failed,
        ComparisonStatusFlag::Warning,
        ComparisonStatusFlag::Message,
        ComparisonStatusFlag::Verbose,
        ComparisonStatusFlag::Diagnostic,
        ComparisonStatusFlag::VerboseDiagnostic});
}

namespace Implementation {

void ComparatorBase::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* const actual, const char* const expected, void(*printer)(Utility::Debug&, const void*)) const {
    CORRADE_INTERNAL_ASSERT(actualValue && expectedValue);
    out << "Values" << actual << "and" << expected << "are not the same, actual is\n       ";
    printer(out, actualValue);
    out << Utility::Debug::newline << "        but expected\n       ";
    printer(out, expectedValue);
}

/* LCOV_EXCL_START */
void ComparatorBase::saveDiagnostic(ComparisonStatusFlags, Utility::Debug&, Containers::StringView) {
    CORRADE_INTERNAL_ASSERT_UNREACHABLE();
}
/* LCOV_EXCL_STOP */

}

}}
