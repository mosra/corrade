/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include <sstream>

#include "TestSuite/Tester.h"
#include "TestSuite/Compare/File.h"
#include "Utility/Directory.h"

#include "configure.h"

using namespace Corrade::Utility;

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

class FileTest: public Tester {
    public:
        FileTest();

        void same();
        void empty();

        void actualNotFound();
        void expectedNotFound();

        void outputActualSmaller();
        void outputExpectedSmaller();
        void output();
};

FileTest::FileTest() {
    addTests(&FileTest::same,
             &FileTest::empty,
             &FileTest::actualNotFound,
             &FileTest::expectedNotFound,
             &FileTest::outputActualSmaller,
             &FileTest::outputExpectedSmaller,
             &FileTest::output);
}

void FileTest::same() {
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("base.txt", "base.txt"));
}

void FileTest::empty() {
    CORRADE_VERIFY(Comparator<Compare::File>(FILETEST_DIR)("empty.txt", "empty.txt"));
}

void FileTest::actualNotFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare;
        CORRADE_VERIFY(!compare("inexistent.txt", Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Actual file a (inexistent.txt) cannot be read.\n");
}

void FileTest::expectedNotFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare;
        CORRADE_VERIFY(!compare(Directory::join(FILETEST_DIR, "base.txt"), "inexistent.txt"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Expected file b (inexistent.txt) cannot be read.\n");
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
