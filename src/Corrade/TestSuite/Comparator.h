#ifndef Corrade_TestSuite_Comparator_h
#define Corrade_TestSuite_Comparator_h
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

/** @file
 * @brief Class @ref Corrade::TestSuite::Comparator, enum @ref Corrade::TestSuite::ComparisonStatusFlag, enum set @ref Corrade::TestSuite::ComparisonStatusFlags
 */

#include "Corrade/TestSuite/visibility.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/StlForwardString.h"

namespace Corrade { namespace TestSuite {

/**
@brief Comparison status flag

@see @ref ComparisonStatusFlags, @ref Comparator
*/
enum class ComparisonStatusFlag: std::uint8_t {
    /**
     * The comparison failed. Absence of this flag indicates success.
     * If this is returned from @ref Comparator::operator()(), the @ref Tester
     * then calls @ref Comparator::printMessage().
     */
    Failed = 1 << 0,

    /**
     * The comparison wants to print a warning. If this is returned from
     * @ref Comparator::operator()(), @ref Tester then calls
     * @ref Comparator::printMessage().
     */
    Warning = 1 << 1,

    /**
     * The comparison wants to print a message. If this is returned from
     * @ref Comparator::operator()(), @ref Tester then calls
     * @ref Comparator::printMessage(). Should be used only seldomly to avoid
     * spamming the output, prefer to use @ref ComparisonStatusFlag::Verbose
     * instead.
     */
    Message = 1 << 2,

    /**
     * The comparison can print a verbose message. If this is returned
     * from @ref Comparator::operator()() and the `--verbose`
     * @ref TestSuite-Tester-command-line "command-line option" is specified,
     * @ref Tester then calls @ref Comparator::printMessage().
     */
    Verbose = 1 << 3,

    /**
     * The comparison can save a comparison diagnostic to a file.
     * If this is returned from @ref Comparator::operator()(), the comparator
     * needs to implement an additional @ref Comparator::saveDiagnostic()
     * function, which is called in case the `--save-diagnostic`
     * @ref TestSuite-Tester-command-line "command-line option" is specified.
     * See @ref TestSuite-Comparator-save-diagnostic for more information.
     */
    Diagnostic = 1 << 4,

    /**
     * The comparison can save a verbose comparison diagnostic to a file. If
     * this is is returned from @ref Comparator::operator()(), the comparator
     * needs to implement an additional @ref Comparator::saveDiagnostic()
     * function. This function gets called in case both the `--save-diagnostic`
     * and `--verbose` @ref TestSuite-Tester-command-line "command-line options"
     * are specified. See @ref TestSuite-Comparator-save-diagnostic for more
     * information.
     */
    VerboseDiagnostic = 1 << 5
};

/**
@brief Comparison status flags

@see @ref Comparator
*/
typedef Containers::EnumSet<ComparisonStatusFlag> ComparisonStatusFlags;

CORRADE_ENUMSET_OPERATORS(ComparisonStatusFlags)

/** @debugoperatorenum{ComparisonStatusFlag} */
CORRADE_TESTSUITE_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ComparisonStatusFlag value);

/** @debugoperatorenum{ComparisonStatusFlags} */
CORRADE_TESTSUITE_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ComparisonStatusFlags value);

/**
@brief Default comparator implementation

See @ref CORRADE_COMPARE_AS(), @ref CORRADE_COMPARE_WITH() for more information
and the @ref Compare namespace for additional comparator implementations.

@section TestSuite-Comparator-subclassing Subclassing

You can reimplement this class for your own data types to provide additional
means of comparison. At the very least you need to provide the @ref operator()()
comparing two values of arbitrary types, returning empty
@ref ComparisonStatusFlags on success and @ref ComparisonStatusFlag::Failed
when the comparison fails. Then, @ref printMessage() gets called in case of a
comparison failure to print a detailed message. It receives stringified
"actual" and "expected" expressions from the comparison macro and is expected
to provide further details from its internal state saved by @ref operator()().

@section TestSuite-Comparator-pseudo-types Comparing with pseudo-types

Imagine you have two filenames and you want to compare their contents instead
of comparing the filename strings. Because you want to also compare strings
elsewhere, you cannot override the default behavior. The solution is to have
some *pseudo-type*, for which you create a @ref Comparator template
specialization, but the actual comparison operator will still take strings as
parameters:

@snippet TestSuite.cpp Comparator-pseudotypes

The actual use in the unit test would be like this:

@snippet TestSuite.cpp Comparator-pseudotypes-usage

@attention Due to implementation limitations, it's not possible to have
    multiple overloads for @cpp operator()() @ce in one class (for example to
    compare file contents with both a filename and a @ref std::istream), you
    have to create a different pseudo-type for that. An alternative advanced
    option is providing a specialization of the `ComparatorTraits` example, see
    source for details.

@section TestSuite-Comparator-parameters Passing parameters to comparators

Sometimes you need to pass additional parameters to the comparator class so you
can then use it in the @ref CORRADE_COMPARE_WITH() macro. In that case you need
to implement the constructor and a @cpp comparator() @ce function in your
pseudo-type. The @cpp comparator() @ce function returns a reference to a
pre-configured @ref Comparator instance. Don't forget to allow default
construction of @ref Comparator, if you want to be able to use it also with
@ref CORRADE_COMPARE_AS().Example:

@snippet TestSuite2.cpp Comparator-parameters

The actual use in a test would be like this:

@snippet TestSuite2.cpp Comparator-parameters-usage

@section TestSuite-Comparator-messages Printing additional messages

By default, the comparator is asked to print a message using @ref printMessage()
only in case the comparison fails. In some cases it's desirable to provide
extended info also in case the comparison *doesn't* fail. That can be done by
returning @ref ComparisonStatusFlag::Warning, @ref ComparisonStatusFlag::Message
or @ref ComparisonStatusFlag::Verbose from @ref operator()() in addition to the
actual comparison status. The @ref printMessage() then gets passed those flags
to know what to print.

The @ref printMessage() is always called at most once per comparison, with the
@ref ComparisonStatusFlag::Verbose flag filtered out when the `--verbose`
@ref TestSuite-Tester-command-line "command-line option" is not passed.

@section TestSuite-Comparator-save-diagnostic Saving diagnostic files

In addition to messages, the comparison can also save diagnostic files. This is
achieved by returning either @ref ComparisonStatusFlag::Diagnostic or
@ref ComparisonStatusFlag::VerboseDiagnostic from @ref operator()(). The
comparator is then required to implement the @ref saveDiagnostic() function
(which doesn't need to be present otherwise). This function gets called when
the `--save-diagnostic` @ref TestSuite-Tester-save-diagnostic "command-line option",
is specified, in case of @ref ComparisonStatusFlag::VerboseDiagnostic the flag
only when `--verbose` is enabled as well.

The function receives a path to where the diagnostic files should be be saved
and is free to do about anything -- for example a file comparator can write
both the actual file and a diff to the original. The message is printed right
after test case name and the comparator has a full freedom in its formatting as
well.

@snippet TestSuite3.cpp Comparator-save-diagnostic

In the above case, the message will look for example like this:

@include testsuite-save-diagnostic.ansi
*/
template<class T> class Comparator {
    public:
        explicit Comparator();

        /**
         * @brief Compare two values
         *
         * If the comparison fails, @ref ComparisonStatusFlag::Failed should be
         * returned. In addition, if the comparison desires to print additional
         * messages or save diagnostic file, it can include other flags.
         */
        ComparisonStatusFlags operator()(const T& actual, const T& expected);

        /**
         * @brief Print a message
         *
         * This function gets called only if @ref operator()() returned one of
         * @ref ComparisonStatusFlag::Failed, @ref ComparisonStatusFlag::Warning,
         * @ref ComparisonStatusFlag::Message or
         * @ref ComparisonStatusFlag::Verbose. The @p status is a result of
         * that call. The @ref ComparisonStatusFlag::Verbose flag gets filtered
         * out in case the `--verbose`
         * @ref TestSuite-Tester-command-line "command-line option" is not set
         * (and the function not being called at all if none of the other
         * above-mentioned flags are present).
         */
        void printMessage(ComparisonStatusFlags status, Utility::Debug& out, const char* actual, const char* expected);

        /**
         * @brief Save a diagnostic
         *
         * This function only needs to be present in the comparator
         * implementation if @ref operator()() *can* return either
         * @ref ComparisonStatusFlag::Diagnostic or
         * @ref ComparisonStatusFlag::VerboseDiagnostic, doesn't need to be
         * implemented at all otherwise. This function gets called only if
         * @ref operator()() returned one of those two flags *and* the
         * `--save-diagnostic` @ref TestSuite-Tester-command-line "command-line option"
         * is present. The @p status is a result of that call. The
         * @ref ComparisonStatusFlag::VerboseDiagnostic flag gets filtered out
         * in case the `--verbose`
         * @ref TestSuite-Tester-command-line "command-line option" is not set
         * (and the function not being called at all if
         * @ref ComparisonStatusFlag::Diagnostic is not present as well).
         */
        void saveDiagnostic(ComparisonStatusFlags status, Utility::Debug& out, const std::string& path);

    private:
        const T* actualValue;
        const T* expectedValue;
};

template<class T> Comparator<T>::Comparator(): actualValue(), expectedValue() {}

template<class T> ComparisonStatusFlags Comparator<T>::operator()(const T& actual, const T& expected) {
    if(actual == expected) return {};

    actualValue = &actual;
    expectedValue = &expected;
    return ComparisonStatusFlag::Failed;
}

template<class T> void Comparator<T>::printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) {
    CORRADE_INTERNAL_ASSERT(actualValue && expectedValue);
    out << "Values" << actual << "and" << expected << "are not the same, actual is\n       "
      << *actualValue << Utility::Debug::newline << "        but expected\n       " << *expectedValue;
}

/* LCOV_EXCL_START */
template<class T> void Comparator<T>::saveDiagnostic(ComparisonStatusFlags, Utility::Debug&, const std::string&) {
    CORRADE_INTERNAL_ASSERT_UNREACHABLE();
}
/* LCOV_EXCL_STOP */

namespace Implementation {

template<class T> struct ComparatorOperatorTraits;

template<class T, class U, class V> struct ComparatorOperatorTraits<ComparisonStatusFlags(T::*)(U, V)> {
    typedef typename std::decay<U>::type ActualType;
    typedef typename std::decay<V>::type ExpectedType;
};

/* The second and third parameters are ignored in the default implementation
   because for overloaded operators I don't see a general way to figure out
   what overload gets taken for particular argument types in combination with
   implicit conversions taking place. For particular types this could be fixed
   by providing an explicit specialization of ComparatorTraits, but the
   specialization has to be provided for all types that can be converted to
   it. */
template<class T, class, class> struct ComparatorTraits: ComparatorOperatorTraits<decltype(&Comparator<T>::operator())> {};

CORRADE_HAS_TYPE(CanSaveDiagnostic, decltype(std::declval<T>().saveDiagnostic({}, std::declval<Utility::Debug&>(), {})));

template<class T> auto diagnosticSaver(typename std::enable_if<CanSaveDiagnostic<Comparator<T>>::value>::type* = nullptr) -> void(*)(void*, ComparisonStatusFlags, Utility::Debug& out, const std::string&) {
    return [](void* comparator, ComparisonStatusFlags flags, Utility::Debug& out, const std::string& path) {
        static_cast<Comparator<T>*>(comparator)->saveDiagnostic(flags, out, path);
    };
}
template<class T> auto diagnosticSaver(typename std::enable_if<!CanSaveDiagnostic<Comparator<T>>::value>::type* = nullptr) -> void(*)(void*, ComparisonStatusFlags, Utility::Debug& out, const std::string&) {
    return nullptr;
}

#ifdef CORRADE_BUILD_DEPRECATED
/* Support for old signatures (operator() returning bool, printErrorMessage()
   instead of printMessage()) */
template<class T, class U, class V> struct ComparatorOperatorTraits<bool(T::*)(U, V)> {
    typedef typename std::decay<U>::type ActualType;
    typedef typename std::decay<V>::type ExpectedType;
};
constexpr CORRADE_DEPRECATED("return ComparisonStatusFlags in custom Comparator implementations instead") ComparisonStatusFlags comparisonStatusFlags(bool value) {
    return value ? ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
}
constexpr ComparisonStatusFlags comparisonStatusFlags(ComparisonStatusFlags value) {
    return value;
}
CORRADE_HAS_TYPE(HasOldPrintErrorMessage, decltype(std::declval<T>().printErrorMessage(std::declval<Utility::Error&>(), {}, {})));
template<class T> CORRADE_DEPRECATED("use printMessage() in custom Comparator implementations instead") void printMessage(typename std::enable_if<HasOldPrintErrorMessage<T>::value, T&>::type comparator, ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) {
    comparator.printErrorMessage(static_cast<Utility::Error&>(out), actual, expected);
}
template<class T> void printMessage(typename std::enable_if<!HasOldPrintErrorMessage<T>::value, T&>::type comparator, ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) {
    comparator.printMessage(flags, out, actual, expected);
}
#endif

}

}}

#endif
