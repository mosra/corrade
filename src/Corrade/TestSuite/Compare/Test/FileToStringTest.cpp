/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
              Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Path.h"

/* The __EMSCRIPTEN_major__ etc macros used to be passed implicitly, version
   3.1.4 moved them to a version header and version 3.1.23 dropped the
   backwards compatibility. To work consistently on all versions, including the
   header only if the version macros aren't present.
   https://github.com/emscripten-core/emscripten/commit/f99af02045357d3d8b12e63793cef36dfde4530a
   https://github.com/emscripten-core/emscripten/commit/f76ddc702e4956aeedb658c49790cc352f892e4c */
#if defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(__EMSCRIPTEN_major__)
#include <emscripten/version.h>
#endif

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
    CORRADE_COMPARE_AS(Utility::Path::join(FILETEST_DIR, "base.txt"), "Hello World!", Compare::FileToString);
}

void FileToStringTest::empty() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
    /* Emscripten 2.0.26+ has a problem in the file embedder, where zero-size
       files are reported as having 3 bytes. The changelog between 2.0.25 and
       2.0.26 doesn't mention anything related, the only related change I found
       was https://github.com/emscripten-core/emscripten/pull/14526, going into
       2.0.25 already, and I suspect it's something related to padding in
       base64 decode. This problem is gone in 3.1.3, where they replace the
       base64 file embedding with putting a binary directly to wasm in
       https://github.com/emscripten-core/emscripten/pull/16050. Which then
       however breaks UTF-8 paths, see the CORRADE_SKIP() below. */
    CORRADE_EXPECT_FAIL("Emscripten 2.0.26 to 3.1.3 reports empty files as having 3 bytes.");
    #endif
    CORRADE_COMPARE_AS(Utility::Path::join(FILETEST_DIR, "empty.txt"), "", Compare::FileToString);
}

void FileToStringTest::utf8Filename() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_COMPARE_AS(Utility::Path::join(FILETEST_DIR, "hýždě.txt"), "Hello World!", Compare::FileToString);
}

void FileToStringTest::notFound() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::FileToString> compare;
        ComparisonStatusFlags flags = compare("nonexistent.txt", "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "file", "b");
    }

    CORRADE_COMPARE(out.str(), "File file (nonexistent.txt) cannot be read.\n");
}

void FileToStringTest::differentContents() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::FileToString> compare;
        /* The filename is referenced as a string view as the assumption is
           that the whole comparison and diagnostic printing gets done in a
           single expression. Thus don't pass it as a temporary to avoid
           dangling views. */
        Containers::String filename = Utility::Path::join(FILETEST_DIR, "different.txt");
        ComparisonStatusFlags flags = compare(filename, "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");
}

void FileToStringTest::actualSmaller() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::FileToString> compare;
        /* The filename is referenced as a string view as the assumption is
           that the whole comparison and diagnostic printing gets done in a
           single expression. Thus don't pass it as a temporary to avoid
           dangling views. */
        Containers::String filename = Utility::Path::join(FILETEST_DIR, "smaller.txt");
        ComparisonStatusFlags flags = compare(filename, "Hello World!");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void FileToStringTest::expectedSmaller() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::FileToString> compare;
        /* The filename is referenced as a string view as the assumption is
           that the whole comparison and diagnostic printing gets done in a
           single expression. Thus don't pass it as a temporary to avoid
           dangling views. */
        Containers::String filename = Utility::Path::join(FILETEST_DIR, "base.txt");
        ComparisonStatusFlags flags = compare(filename, "Hello W");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::FileToStringTest)
