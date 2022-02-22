/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include "String.h"

namespace Corrade { namespace TestSuite {

ComparisonStatusFlags Comparator<Compare::StringHasPrefix>::operator()(const Containers::StringView actual, const Containers::StringView expectedPrefix) {
    _actualValue = actual;
    _expectedPrefixValue = expectedPrefix;
    return actual.hasPrefix(expectedPrefix) ?
        ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
}

void Comparator<Compare::StringHasPrefix>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    out << "String" << actual << "isn't prefixed with" << expected
        << Utility::Debug::nospace << ", actual is\n       " << _actualValue
        << Utility::Debug::newline << "        but expected prefix\n       "
        << _expectedPrefixValue;
}

ComparisonStatusFlags Comparator<Compare::StringHasSuffix>::operator()(const Containers::StringView actual, const Containers::StringView expectedSuffix) {
    _actualValue = actual;
    _expectedSuffixValue = expectedSuffix;
    return actual.hasSuffix(expectedSuffix) ?
        ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
}

void Comparator<Compare::StringHasSuffix>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    out << "String" << actual << "isn't suffixed with" << expected
        << Utility::Debug::nospace << ", actual is\n       " << _actualValue
        << Utility::Debug::newline << "        but expected suffix\n       "
        << _expectedSuffixValue;
}

}}
