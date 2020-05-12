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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Directory.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct FileToStringTest: Tester {
    explicit FileToStringTest();

    void same();
    void empty();
    void utf8Filename();

    void notFound();

    void differentContents();
    void actualSmaller();
    void expectedSmaller();
};

FileToStringTest::FileToStringTest() {
    addTests({&FileToStringTest::same,
              &FileToStringTest::empty,
              &FileToStringTest::utf8Filename,

              &FileToStringTest::notFound,

              &FileToStringTest::differentContents,
              &FileToStringTest::actualSmaller,
              &FileToStringTest::expectedSmaller});
}

void FileToStringTest::same() {
    CORRADE_COMPARE_AS(Utility::Directory::join(FILETEST_DIR, "base.txt"), "Hello World!", Compare::FileToString);
}

void FileToStringTest::empty() {
    CORRADE_COMPARE_AS(Utility::Directory::join(FILETEST_DIR, "empty.txt"), "", Compare::FileToString);
}

void FileToStringTest::utf8Filename() {
    CORRADE_COMPARE_AS(Utility::Directory::join(FILETEST_DIR, "hýždě.txt"), "Hello World!", Compare::FileToString);
}

void FileToStringTest::notFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        ComparisonStatusFlags flags = compare("nonexistent.txt", "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "file", "b");
    }

    CORRADE_COMPARE(out.str(), "File file (nonexistent.txt) cannot be read.\n");
}

void FileToStringTest::differentContents() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        ComparisonStatusFlags flags = compare(Utility::Directory::join(FILETEST_DIR, "different.txt"), "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

void FileToStringTest::actualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        ComparisonStatusFlags flags = compare(Utility::Directory::join(FILETEST_DIR, "smaller.txt"), "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileToStringTest::expectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        ComparisonStatusFlags flags = compare(Utility::Directory::join(FILETEST_DIR, "base.txt"), "Hello W");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileToStringTest)
