/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/Directory.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

struct FileToStringTest: Tester {
    explicit FileToStringTest();

    void same();
    void empty();
    void utf8Filename();

    void notFound();

    void outputActualSmaller();
    void outputExpectedSmaller();
    void output();
};

FileToStringTest::FileToStringTest() {
    addTests({&FileToStringTest::same,
              &FileToStringTest::empty,
              &FileToStringTest::utf8Filename,

              &FileToStringTest::notFound,

              &FileToStringTest::outputActualSmaller,
              &FileToStringTest::outputExpectedSmaller,
              &FileToStringTest::output});
}

void FileToStringTest::same() {
    CORRADE_VERIFY(Comparator<Compare::FileToString>()(Utility::Directory::join(FILETEST_DIR, "base.txt"), "Hello World!"));
}

void FileToStringTest::empty() {
    CORRADE_VERIFY(Comparator<Compare::FileToString>()(Utility::Directory::join(FILETEST_DIR, "empty.txt"), ""));
}

void FileToStringTest::utf8Filename() {
    CORRADE_VERIFY(Comparator<Compare::FileToString>()(Utility::Directory::join(FILETEST_DIR, "hýždě.txt"), "Hello World!"));
}

void FileToStringTest::notFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare("nonexistent.txt", "Hello World!"));
        compare.printErrorMessage(e, "file", "b");
    }

    CORRADE_COMPARE(out.str(), "File file (nonexistent.txt) cannot be read.\n");
}

void FileToStringTest::outputActualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Utility::Directory::join(FILETEST_DIR, "smaller.txt"), "Hello World!"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileToStringTest::outputExpectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Utility::Directory::join(FILETEST_DIR, "base.txt"), "Hello W"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

void FileToStringTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Utility::Directory::join(FILETEST_DIR, "different.txt"), "Hello World!"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileToStringTest)
