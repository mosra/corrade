/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

class StringToFileTest: public Tester {
    public:
        StringToFileTest();

        void same();
        void empty();
        void utf8Filename();

        void notFound();

        void outputActualSmaller();
        void outputExpectedSmaller();
        void output();

        void saveFailed();
};

StringToFileTest::StringToFileTest() {
    addTests({&StringToFileTest::same,
              &StringToFileTest::empty,
              &StringToFileTest::utf8Filename,

              &StringToFileTest::notFound,

              &StringToFileTest::outputActualSmaller,
              &StringToFileTest::outputExpectedSmaller,
              &StringToFileTest::output,

              &StringToFileTest::saveFailed});
}

void StringToFileTest::same() {
    CORRADE_COMPARE_AS("Hello World!", Utility::Directory::join(FILETEST_DIR, "base.txt"), Compare::StringToFile);
}

void StringToFileTest::empty() {
    CORRADE_COMPARE_AS("", Utility::Directory::join(FILETEST_DIR, "empty.txt"), Compare::StringToFile);
}

void StringToFileTest::utf8Filename() {
    CORRADE_COMPARE_AS("Hello World!", Utility::Directory::join(FILETEST_DIR, "hýždě.txt"), Compare::StringToFile);
}

void StringToFileTest::notFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello World!", "nonexistent.txt"));
        compare.printErrorMessage(e, "a", "file");
    }

    CORRADE_COMPARE(out.str(), "File file (nonexistent.txt) cannot be read.\n");
}

void StringToFileTest::outputActualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello W", Utility::Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void StringToFileTest::outputExpectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello World!", Utility::Directory::join(FILETEST_DIR, "smaller.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

void StringToFileTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello world?", Utility::Directory::join(FILETEST_DIR, "base.txt")));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

void StringToFileTest::saveFailed() {
    std::stringstream out;

    /* Create the output dir if it doesn't exist, but avoid stale files making
       false positives */
    CORRADE_VERIFY(Utility::Directory::mkpath(FILETEST_SAVE_DIR));
    std::string filename = Utility::Directory::join(FILETEST_SAVE_DIR, "base.txt");
    if(Utility::Directory::exists(filename))
        CORRADE_VERIFY(Utility::Directory::rm(filename));

    {
        Error e(&out);
        Comparator<Compare::StringToFile> compare;
        CORRADE_VERIFY(!compare("Hello world?", Utility::Directory::join(FILETEST_DIR, "base.txt")));
        compare.saveActualFile(e, FILETEST_SAVE_DIR);
    }

    CORRADE_COMPARE(out.str(), Utility::formatString("-> {}\n", filename));

    /* Extreme dogfooding, eheh. We expect the *actual* contents, but under the
       *expected* filename */
    CORRADE_COMPARE_AS(filename, "Hello world?", FileToString);
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::StringToFileTest)
