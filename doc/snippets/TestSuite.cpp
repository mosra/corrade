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

#include <vector>

#include "Corrade/Containers/Pointer.h"
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/TestSuite/Comparator.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/StlMath.h"

using namespace Corrade;

#if defined(CORRADE_NO_ASSERT) && defined(CORRADE_TARGET_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* [Comparator-pseudotypes] */
class FileContents {};

namespace Corrade { namespace TestSuite { // the namespace is important

template<> class Comparator<FileContents> {
    public:
        ComparisonStatusFlags operator()(const std::string& actual, const std::string& expected) {
            _actualContents = Utility::Directory::readString(actual);
            _expectedContents = Utility::Directory::readString(expected);
            return _actualContents == _expectedContents ? ComparisonStatusFlags{} :
                ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const {
            CORRADE_INTERNAL_ASSERT(flags & ComparisonStatusFlag::Failed);
            out << "Files" << actual << "and" << expected << "are not the same, actual" << _actualContents << "but expected" << _expectedContents;
        }

    private:
        std::string _actualContents, _expectedContents;
};

}}
/* [Comparator-pseudotypes] */
#if defined(CORRADE_NO_ASSERT) && defined(CORRADE_TARGET_GCC)
#pragma GCC diagnostic pop
#endif

struct Foo: TestSuite::Tester {
Foo() {
/* [Comparator-pseudotypes-usage] */
CORRADE_COMPARE_AS("/path/to/actual.dat", "/path/to/expected.dat",
    FileContents);
/* [Comparator-pseudotypes-usage] */

{
/* [Compare-Container] */
std::vector<int> a, b;
CORRADE_COMPARE_AS(a, b, TestSuite::Compare::Container);
/* [Compare-Container] */

/* [Compare-SortedContainer] */
CORRADE_COMPARE_AS(a, b, TestSuite::Compare::SortedContainer);
/* [Compare-SortedContainer] */
}

/* [Compare-File] */
CORRADE_COMPARE_AS("actual.txt", "expected.txt", TestSuite::Compare::File);
/* [Compare-File] */

/* [Compare-File-prefix] */
CORRADE_COMPARE_WITH("actual.txt", "expected.txt",
    TestSuite::Compare::File{"/common/prefix"});
/* [Compare-File-prefix] */

/* [Compare-FileToString] */
CORRADE_COMPARE_AS("actual.txt", "expected file contents",
    TestSuite::Compare::FileToString);
/* [Compare-FileToString] */

/* [Compare-StringToFile] */
CORRADE_COMPARE_AS("actual file contents", "expected.txt",
    TestSuite::Compare::StringToFile);
/* [Compare-StringToFile] */

{
/* [Compare-Less] */
float a;
CORRADE_COMPARE_AS(a, 9.28f, TestSuite::Compare::Less);
/* [Compare-Less] */
}

{
/* [Compare-LessOrEqual] */
float a;
CORRADE_COMPARE_AS(a, 9.28f, TestSuite::Compare::LessOrEqual);
/* [Compare-LessOrEqual] */
}

{
/* [Compare-GreaterOrEqual] */
float a;
CORRADE_COMPARE_AS(a, 9.28f, TestSuite::Compare::GreaterOrEqual);
/* [Compare-GreaterOrEqual] */
}

{
/* [Compare-Greater] */
float a;
CORRADE_COMPARE_AS(a, 9.28f, TestSuite::Compare::Greater);
/* [Compare-Greater] */
}

{
/* [Compare-Around] */
float a;
CORRADE_COMPARE_WITH(a, 9.28f, TestSuite::Compare::Around<float>{0.1f});
/* [Compare-Around] */
}

{
/* [Compare-around] */
float a;
CORRADE_COMPARE_WITH(a, 9.28f, TestSuite::Compare::Around<float>{0.1f});
CORRADE_COMPARE_WITH(a, 9.28f, TestSuite::Compare::around(0.1f));
/* [Compare-around] */
}

{
/* [Compare-around-just-one] */
float a;
CORRADE_COMPARE_WITH(a, 9.28f, TestSuite::Compare::around(0.1f));
/* [Compare-around-just-one] */
}

{
/* [Compare-Divisible] */
int a;
CORRADE_COMPARE_AS(a, 4, TestSuite::Compare::Divisible);
/* [Compare-Divisible] */
}

{
/* [Compare-NotDivisible] */
int a;
CORRADE_COMPARE_AS(a, 4, TestSuite::Compare::NotDivisible);
/* [Compare-NotDivisible] */
}

{
/* [CORRADE_VERIFY] */
std::string s("hello");
CORRADE_VERIFY(!s.empty());
/* [CORRADE_VERIFY] */
}

{
/* [CORRADE_VERIFY-explicit] */
Containers::Pointer<int> i{new int};
CORRADE_VERIFY(i);
/* [CORRADE_VERIFY-explicit] */
}

{
/* [CORRADE_COMPARE] */
int a = 5 + 3;
CORRADE_COMPARE(a, 8);
/* [CORRADE_COMPARE] */
}

{
/* [CORRADE_COMPARE_AS] */
CORRADE_COMPARE_AS(std::sin(0.0), 0.0f, float);
/* [CORRADE_COMPARE_AS] */
}

{
/* [CORRADE_COMPARE_WITH] */
CORRADE_COMPARE_WITH("actual.txt", "expected.txt",
    TestSuite::Compare::File{"/common/path/prefix"});
/* [CORRADE_COMPARE_WITH] */
}

{
struct { bool operator()() { return false; } } isFutureClear;
/* [CORRADE_EXPECT_FAIL] */
{
    CORRADE_EXPECT_FAIL("Not implemented.");
    CORRADE_VERIFY(isFutureClear());
}

int i = 6*7;
CORRADE_COMPARE(i, 42);
/* [CORRADE_EXPECT_FAIL] */
}

{
int answer{};
/* [CORRADE_EXPECT_FAIL_IF-wrong] */
{
    if(answer != 42)
        CORRADE_EXPECT_FAIL("This is not our universe.");

    CORRADE_COMPARE(6*9, 42); // always fails
}
/* [CORRADE_EXPECT_FAIL_IF-wrong] */

/* [CORRADE_EXPECT_FAIL_IF] */
{
    CORRADE_EXPECT_FAIL_IF(answer != 42, "This is not our universe.");

    CORRADE_COMPARE(6*7, 49); // expect the failure if answer is not 42
}
/* [CORRADE_EXPECT_FAIL_IF] */
}

{
bool bigEndian = false;
/* [CORRADE_SKIP] */
if(!bigEndian) {
    CORRADE_SKIP("Big endian compatibility can't be tested on this system.");
}
/* [CORRADE_SKIP] */
}

{
/* [Tester-setTestCaseName] */
setTestCaseName(CORRADE_FUNCTION);
/* [Tester-setTestCaseName] */
}

{
const char* name{};
/* [Tester-setTestCaseTemplateName] */
setTestCaseName(Utility::formatString("{}<{}>", CORRADE_FUNCTION, name));
/* [Tester-setTestCaseTemplateName] */
}
}

/* [CORRADE_BENCHMARK] */
void benchmark() {
    std::string a = "hello", b = "world";
    CORRADE_BENCHMARK(1000) {
        volatile std::string c = a + b;
    }
}
/* [CORRADE_BENCHMARK] */

/* [Tester-Debug] */
void myTestCase() {
    int a = 4;
    Debug{} << a;
    CORRADE_COMPARE(a + a, 8);
}
/* [Tester-Debug] */
};

/* To prevent macOS ranlib complaining that there are no symbols */
int main() {}
