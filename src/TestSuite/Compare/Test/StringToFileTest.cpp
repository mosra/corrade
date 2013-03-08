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
#include "TestSuite/Compare/StringToFile.h"
#include "Utility/Directory.h"

#include "configure.h"

using Corrade::Utility::Directory;

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

class StringToFileTest: public Tester {
    public:
        StringToFileTest();

        void same();
        void empty();

        void notFound();

        void outputActualSmaller();
        void outputExpectedSmaller();
        void output();
};

StringToFileTest::StringToFileTest() {
    addTests({&StringToFileTest::same,
              &StringToFileTest::empty,
              &StringToFileTest::notFound,
              &StringToFileTest::outputActualSmaller,
              &StringToFileTest::outputExpectedSmaller,
              &StringToFileTest::output});
}

void StringToFileTest::same() {
    CORRADE_VERIFY(Comparator<Compare::StringToFile>()("Hello World!", Directory::join(FILETEST_DIR, "base.txt")));
}

void StringToFileTest::empty() {
    CORRADE_VERIFY(Comparator<Compare::StringToFile>()("", Directory::join(FILETEST_DIR, "empty.txt")));
}

void StringToFileTest::notFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello World!", "inexistent"));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "File a (inexistent) cannot be read.\n");
}

void StringToFileTest::outputActualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello W", Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void StringToFileTest::outputExpectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello World!", Directory::join(FILETEST_DIR, "smaller.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

void StringToFileTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello world?", Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::StringToFileTest)
