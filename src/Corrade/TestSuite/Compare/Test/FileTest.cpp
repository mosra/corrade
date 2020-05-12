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
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct FileTest: Tester {
    explicit FileTest();

    void same();
    void empty();
    void utf8Filename();

    void actualNotFound();
    void expectedNotFound();

    void differentContents();
    void actualSmaller();
    void expectedSmaller();
};

FileTest::FileTest() {
    addTests({&FileTest::same,
              &FileTest::empty,
              &FileTest::utf8Filename,

              &FileTest::actualNotFound,
              &FileTest::expectedNotFound,

              &FileTest::differentContents,
              &FileTest::actualSmaller,
              &FileTest::expectedSmaller});
}

void FileTest::same() {
    CORRADE_COMPARE_WITH("base.txt", "base.txt", Compare::File{FILETEST_DIR});

    /* Should not return Diagnostic as everything is okay */
    CORRADE_COMPARE(Comparator<Compare::File>{FILETEST_DIR}("base.txt", "base.txt"), ComparisonStatusFlags{});
}

void FileTest::empty() {
    CORRADE_COMPARE_WITH("empty.txt", "empty.txt", Compare::File{FILETEST_DIR});
}

void FileTest::utf8Filename() {
    CORRADE_COMPARE_WITH("hýždě.txt", "base.txt", Compare::File{FILETEST_DIR});
    CORRADE_COMPARE_WITH("base.txt", "hýždě.txt", Compare::File{FILETEST_DIR});
}

void FileTest::actualNotFound() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare;
        ComparisonStatusFlags flags = compare("nonexistent.txt", Utility::Directory::join(FILETEST_DIR, "base.txt"));
        /* Should not return Diagnostic as there's no file to read from */
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Actual file a (nonexistent.txt) cannot be read.\n");
}

void FileTest::expectedNotFound() {
    std::stringstream out;

    Comparator<Compare::File> compare;
    ComparisonStatusFlags flags = compare(Utility::Directory::join(FILETEST_DIR, "base.txt"), "nonexistent.txt");
    /* Should return Diagnostic even though we can't find the expected file
       as it doesn't matter */
    CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);

    {
        Debug redirectOutput(&out);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Expected file b (nonexistent.txt) cannot be read.\n");

    /* Create the output dir if it doesn't exist, but avoid stale files making
       false positives */
    CORRADE_VERIFY(Utility::Directory::mkpath(FILETEST_SAVE_DIR));
    std::string filename = Utility::Directory::join(FILETEST_SAVE_DIR, "nonexistent.txt");
    if(Utility::Directory::exists(filename))
        CORRADE_VERIFY(Utility::Directory::rm(filename));

    {
        out.str({});
        Debug redirectOutput(&out);
        compare.saveDiagnostic(flags, redirectOutput, FILETEST_SAVE_DIR);
    }

    /* Extreme dogfooding, eheh. We expect the *actual* contents, but under the
       *expected* filename */
    CORRADE_COMPARE(out.str(), Utility::formatString("-> {}\n", filename));
    CORRADE_COMPARE_AS(filename,
        Utility::Directory::join(FILETEST_DIR, "base.txt"), File);
}

void FileTest::differentContents() {
    std::stringstream out;

    Comparator<Compare::File> compare{FILETEST_DIR};
    ComparisonStatusFlags flags = compare("different.txt", "base.txt");
    CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);

    {
        Debug redirectOutput(&out);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");

    /* Create the output dir if it doesn't exist, but avoid stale files making
       false positives */
    CORRADE_VERIFY(Utility::Directory::mkpath(FILETEST_SAVE_DIR));
    std::string filename = Utility::Directory::join(FILETEST_SAVE_DIR, "base.txt");
    if(Utility::Directory::exists(filename))
        CORRADE_VERIFY(Utility::Directory::rm(filename));

    {
        out.str({});
        Debug redirectOutput(&out);
        compare.saveDiagnostic(flags, redirectOutput, FILETEST_SAVE_DIR);
    }

    /* Extreme dogfooding, eheh. We expect the *actual* contents, but under the
       *expected* filename */
    CORRADE_COMPARE(out.str(), Utility::formatString("-> {}\n", filename));
    CORRADE_COMPARE_AS(filename,
        Utility::Directory::join(FILETEST_DIR, "different.txt"), File);
}

void FileTest::actualSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare(FILETEST_DIR);
        ComparisonStatusFlags flags = compare("smaller.txt", "base.txt");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);
        compare.printMessage(flags, e, "a", "b");
        /* not testing diagnostic as differentContents() tested this code path
           already */
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileTest::expectedSmaller() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<Compare::File> compare(FILETEST_DIR);
        ComparisonStatusFlags flags = compare("base.txt", "smaller.txt");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);
        compare.printMessage(flags, e, "a", "b");
        /* not testing diagnostic as differentContents() tested this code path
           already */
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileTest)
