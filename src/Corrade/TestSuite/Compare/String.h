#ifndef Corrade_TestSuite_Compare_String_h
#define Corrade_TestSuite_Compare_String_h
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

/** @file
 * @brief Class @ref Corrade::TestSuite::Compare::StringHasPrefix, @ref Corrade::TestSuite::Compare::StringHasSuffix
 * @m_since_latest
 */

/* Theoretically, if there would be C string overloads and PIMPL'd internals,
   this include would not be needed... but practically a string comparison
   almost never involves just C string literals, so I don't see a point. */
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace TestSuite {

namespace Compare {

/**
@brief Pseudo-type for verifying that a string has given prefix
@m_since_latest

Prints both strings if the actual string doesn't have the expected prefix.
Example usage is below, see @ref TestSuite-Comparator-pseudo-types for more
information.

@snippet TestSuite.cpp Compare-StringHasPrefix

If the `--verbose` @ref TestSuite-Tester-command-line "command-line option" is
specified, passed comparisons where the strings are different will print an
@cb{.ansi} [1;39mINFO @ce message with the full string content for detailed
inspection.
@see @ref StringHasSuffix, @ref StringContains, @ref StringNotContains
*/
class StringHasPrefix {};

/**
@brief Pseudo-type for verifying that a string has given suffix
@m_since_latest

Prints both strings if the actual string doesn't have the expected suffix.
Example usage is below, see @ref TestSuite-Comparator-pseudo-types for more
information.

@snippet TestSuite.cpp Compare-StringHasSuffix

If the `--verbose` @ref TestSuite-Tester-command-line "command-line option" is
specified, passed comparisons where the strings are different will print an
@cb{.ansi} [1;39mINFO @ce message with the full string content for detailed
inspection.
@see @ref StringHasPrefix, @ref StringContains, @ref StringNotContains
*/
class StringHasSuffix {};

/**
@brief Pseudo-type for verifying that a string contains given substring
@m_since_latest

Prints both strings if the actual string doesn't contain the expected
substring. Example usage is below, see @ref TestSuite-Comparator-pseudo-types
for more information.

@snippet TestSuite.cpp Compare-StringContains

If the `--verbose` @ref TestSuite-Tester-command-line "command-line option" is
specified, passed comparisons where the strings are different will print an
@cb{.ansi} [1;39mINFO @ce message with the full string content for detailed
inspection.
@see @ref StringNotContains, @ref StringHasPrefix, @ref StringHasSuffix
*/
class StringContains {};

/**
@brief Pseudo-type for verifying that a string does not contain given substring
@m_since_latest

Prints both strings if the actual string does contain the expected substring.
Example usage is below, see @ref TestSuite-Comparator-pseudo-types for more
information.

@snippet TestSuite.cpp Compare-StringNotContains

If the `--verbose` @ref TestSuite-Tester-command-line "command-line option" is
specified, passed comparisons where the strings are different will print an
@cb{.ansi} [1;39mINFO @ce message with the full string content for detailed
inspection.
@see @ref StringContains, @ref StringHasPrefix, @ref StringHasSuffix
*/
class StringNotContains {};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringHasPrefix> {
    public:
        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expectedPrefix);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        Containers::StringView _actualValue;
        Containers::StringView _expectedPrefixValue;
};

template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringHasSuffix> {
    public:
        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expectedSuffix);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        Containers::StringView _actualValue;
        Containers::StringView _expectedSuffixValue;
};

template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringContains> {
    public:
        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expectedToContain);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        Containers::StringView _actualValue;
        Containers::StringView _expectedToContainValue;
};

template<> class CORRADE_TESTSUITE_EXPORT Comparator<Compare::StringNotContains> {
    public:
        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expectedToNotContain);

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const;

    private:
        Containers::StringView _actualValue;
        Containers::StringView _expectedToNotContainValue;
};
#endif

}}

#endif
