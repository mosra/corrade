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

#include <string>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"

using namespace Corrade;

class FileContents {};

namespace Corrade { namespace TestSuite {

template<> class Comparator<FileContents> {
    public:
        ComparisonStatusFlags operator()(const std::string&, const std::string& expected) {
            _expectedFilename = expected;
            return ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Files" << actual << "and" << expected << "are not the same, actual ABC but expected abc";
        }

        void saveDiagnostic(ComparisonStatusFlags, Utility::Debug& out, const std::string& path) {
            out << "->" << Utility::Directory::join(path, Utility::Directory::filename(_expectedFilename));
        }

    private:
        std::string _expectedFilename;
};

}}

struct MyTest: TestSuite::Tester {
    explicit MyTest();

    void generateFile();
};

MyTest::MyTest() {
    addTests({&MyTest::generateFile});
}

void MyTest::generateFile() {
    CORRADE_COMPARE_AS("a.txt", "b.txt", FileContents);
}

CORRADE_TEST_MAIN(MyTest)
