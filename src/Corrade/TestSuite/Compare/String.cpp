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

    /* If the strings are different, we can print them both in a verbose
       message */
    if(!actual.hasPrefix(expectedPrefix)) return ComparisonStatusFlag::Failed;
    if(actual != expectedPrefix) return ComparisonStatusFlag::Verbose;
    return {};
}

void Comparator<Compare::StringHasPrefix>::printMessage(const ComparisonStatusFlags flags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    if(flags == ComparisonStatusFlag::Failed)
        out << "String" << actual << "isn't prefixed with" << expected
            << Utility::Debug::nospace << ", actual is\n       " << _actualValue
            << Utility::Debug::newline << "        but expected prefix\n       "
            << _expectedPrefixValue;
    else if(flags == ComparisonStatusFlag::Verbose)
        out << "String" << actual << "is prefixed with" << expected
            << Utility::Debug::nospace << ", the actual string\n       " << _actualValue
            << Utility::Debug::newline << "        has expected prefix\n       "
            << _expectedPrefixValue;
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

ComparisonStatusFlags Comparator<Compare::StringHasSuffix>::operator()(const Containers::StringView actual, const Containers::StringView expectedSuffix) {
    _actualValue = actual;
    _expectedSuffixValue = expectedSuffix;

    /* If the strings are different, we can print them both in a verbose
       message */
    if(!actual.hasSuffix(expectedSuffix)) return ComparisonStatusFlag::Failed;
    if(actual != expectedSuffix) return ComparisonStatusFlag::Verbose;
    return {};
}

void Comparator<Compare::StringHasSuffix>::printMessage(const ComparisonStatusFlags flags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    if(flags == ComparisonStatusFlag::Failed)
        out << "String" << actual << "isn't suffixed with" << expected
            << Utility::Debug::nospace << ", actual is\n       " << _actualValue
            << Utility::Debug::newline << "        but expected suffix\n       "
            << _expectedSuffixValue;
    else if(flags == ComparisonStatusFlag::Verbose)
        out << "String" << actual << "is suffixed with" << expected
            << Utility::Debug::nospace << ", the actual string\n       " << _actualValue
            << Utility::Debug::newline << "        has expected suffix\n       "
            << _expectedSuffixValue;
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

ComparisonStatusFlags Comparator<Compare::StringContains>::operator()(const Containers::StringView actual, const Containers::StringView expectedToContain) {
    _actualValue = actual;
    _expectedToContainValue = expectedToContain;

    /* If the strings are different, we can print them both in a verbose
       message */
    if(!actual.contains(expectedToContain)) return ComparisonStatusFlag::Failed;
    if(actual != expectedToContain) return ComparisonStatusFlag::Verbose;
    return {};
}

void Comparator<Compare::StringContains>::printMessage(const ComparisonStatusFlags flags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    if(flags == ComparisonStatusFlag::Failed)
        out << "String" << actual << "doesn't contain" << expected
            << Utility::Debug::nospace << ", actual is\n       " << _actualValue
            << Utility::Debug::newline << "        but expected to contain\n       "
            << _expectedToContainValue;
    else if(flags == ComparisonStatusFlag::Verbose)
        out << "String" << actual << "contains" << expected << "at position"
            << (_actualValue.find(_expectedToContainValue).begin() - _actualValue.begin())
            << Utility::Debug::nospace << ", the actual string\n       " << _actualValue
            << Utility::Debug::newline << "        expectedly contains\n       "
            << _expectedToContainValue;
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

ComparisonStatusFlags Comparator<Compare::StringNotContains>::operator()(const Containers::StringView actual, const Containers::StringView expectedToNotContain) {
    _actualValue = actual;
    _expectedToNotContainValue = expectedToNotContain;

    /* Unlike the other comparators, here it can't pass if the strings are the
       same, meaning we report the verbose message always */
    if(actual.contains(expectedToNotContain)) return ComparisonStatusFlag::Failed;
    return ComparisonStatusFlag::Verbose;
}

void Comparator<Compare::StringNotContains>::printMessage(const ComparisonStatusFlags flags, Utility::Debug& out, const char* const actual, const char* const expected) const {
    if(flags == ComparisonStatusFlag::Failed)
        out << "String" << actual << "contains" << expected << "at position"
            << (_actualValue.find(_expectedToNotContainValue).begin() - _actualValue.begin())
            << Utility::Debug::nospace << ", actual is\n       " << _actualValue
            << Utility::Debug::newline << "        but expected to not contain\n       "
            << _expectedToNotContainValue;
    else if(flags == ComparisonStatusFlag::Verbose)
        out << "String" << actual << "doesn't contain" << expected
            << Utility::Debug::nospace << ", the actual string\n       " << _actualValue
            << Utility::Debug::newline << "        expectedly doesn't contain\n       "
            << _expectedToNotContainValue;
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

}}
