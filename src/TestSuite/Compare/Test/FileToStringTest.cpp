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

#include "FileToStringTest.h"

#include <sstream>

#include "TestSuite/Compare/FileToString.h"
#include "Utility/Directory.h"

#include "configure.h"

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileToStringTest)

using namespace Corrade::Utility;

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

FileToStringTest::FileToStringTest() {
    addTests(&FileToStringTest::same,
             &FileToStringTest::empty,
             &FileToStringTest::notFound,
             &FileToStringTest::outputActualSmaller,
             &FileToStringTest::outputExpectedSmaller,
             &FileToStringTest::output);
}

void FileToStringTest::same() {
    CORRADE_VERIFY(Comparator<Compare::FileToString>()(Directory::join(FILETEST_DIR, "base.txt"), "Hello World!"));
}

void FileToStringTest::empty() {
    CORRADE_VERIFY(Comparator<Compare::FileToString>()(Directory::join(FILETEST_DIR, "empty.txt"), ""));
}

void FileToStringTest::notFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare("inexistent", "Hello World!"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "File a (inexistent) cannot be read.\n");
}

void FileToStringTest::outputActualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Directory::join(FILETEST_DIR, "smaller.txt"), "Hello World!"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileToStringTest::outputExpectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Directory::join(FILETEST_DIR, "base.txt"), "Hello W"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

void FileToStringTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::FileToString> compare;
        CORRADE_VERIFY(!compare(Directory::join(FILETEST_DIR, "different.txt"), "Hello World!"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

}}}}
