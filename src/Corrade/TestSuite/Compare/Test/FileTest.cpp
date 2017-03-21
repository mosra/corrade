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
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/Utility/Directory.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

struct FileTest: Tester {
    explicit FileTest();

    void same();
    void empty();
    void utf8Filename();

    void actualNotFound();
    void expectedNotFound();

    void outputActualSmaller();
    void outputExpectedSmaller();
    void output();
};

FileTest::FileTest() {
    addTests({&FileTest::same,
              &FileTest::empty,
              &FileTest::utf8Filename,

              &FileTest::actualNotFound,
              &FileTest::expectedNotFound,
              &FileTest::outputActualSmaller,
              &FileTest::outputExpectedSmaller,
              &FileTest::output});
}

void FileTest::same() {
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("base.txt", "base.txt"));
}

void FileTest::empty() {
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("empty.txt", "empty.txt"));
}

void FileTest::utf8Filename() {
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("hýždě.txt", "base.txt"));
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("base.txt", "hýždě.txt"));
}

void FileTest::actualNotFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare;
        CORRADE_VERIFY(!compare("nonexistent.txt", Utility::Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Actual file a (nonexistent.txt) cannot be read.\n");
}

void FileTest::expectedNotFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare;
        CORRADE_VERIFY(!compare(Utility::Directory::join(FILETEST_DIR, "base.txt"), "nonexistent.txt"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Expected file b (nonexistent.txt) cannot be read.\n");
}

void FileTest::outputActualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare(FILETEST_DIR);
        CORRADE_VERIFY(!compare("smaller.txt", "base.txt"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileTest::outputExpectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare(FILETEST_DIR);
        CORRADE_VERIFY(!compare("base.txt", "smaller.txt"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

void FileTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare(FILETEST_DIR);
        CORRADE_VERIFY(!compare("different.txt", "base.txt"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileTest)
