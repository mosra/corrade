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
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Path.h"
#ifdef CORRADE_TARGET_EMSCRIPTEN
#include "Corrade/Utility/Test/nodeJsVersionHelpers.h"
#endif

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

class StringToFileTest: public Tester {
    public:
        StringToFileTest();

        void same();
        void empty();
        void utf8Filename();

        void notFound();

        void differentContents();
        void actualSmaller();
        void expectedSmaller();
};

StringToFileTest::StringToFileTest() {
    addTests({&StringToFileTest::same,
              &StringToFileTest::empty,
              &StringToFileTest::utf8Filename,

              &StringToFileTest::notFound,

              &StringToFileTest::differentContents,
              &StringToFileTest::actualSmaller,
              &StringToFileTest::expectedSmaller});
}

void StringToFileTest::same() {
    CORRADE_COMPARE_AS("Hello World!", Utility::Path::join(FILETEST_DIR, "base.txt"), Compare::StringToFile);
}

void StringToFileTest::empty() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
    /* Emscripten 2.0.26+ has a problem in the file embedder, where zero-size
       files are reported as having 3 bytes. The changelog between 2.0.25 and
       2.0.26 doesn't mention anything related, the only related change I found
       was https://github.com/emscripten-core/emscripten/pull/14526, going into
       2.0.25 already, and I suspect it's something related to padding in
       base64 decode. This problem is gone in 3.1.3, where they replace the
       base64 file embedding with putting a binary directly to wasm in
       https://github.com/emscripten-core/emscripten/pull/16050. Which then
       however breaks UTF-8 paths, see the CORRADE_SKIP() below.

       Also seems to happen only with Node.js 14 that's bundled with emsdk, not
       with external version 18. Node.js 15+ is only bundled with emsdk 3.1.35+
       which doesn't suffer from this 3-byte bug anymore. */
    CORRADE_EXPECT_FAIL_IF(Utility::Test::nodeJsVersionLess(18),
        "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
    #endif
    CORRADE_COMPARE_AS("", Utility::Path::join(FILETEST_DIR, "empty.txt"), Compare::StringToFile);
}

void StringToFileTest::utf8Filename() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_COMPARE_AS("Hello World!", Utility::Path::join(FILETEST_DIR, "hýždě.txt"), Compare::StringToFile);
}

void StringToFileTest::notFound() {
    std::stringstream out;

    Comparator<Compare::StringToFile> compare;
    ComparisonStatusFlags flags = compare("Hello World!", "nonexistent.txt");
    /* Should return Diagnostic even though we can't find the expected file
        as it doesn't matter */
    CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);

    {
        Debug redirectOutput{&out};
        compare.printMessage(flags, redirectOutput, "a", "file");
    }

    CORRADE_COMPARE(out.str(), "File file (nonexistent.txt) cannot be read.\n");

    /* Create the output dir if it doesn't exist, but avoid stale files making
       false positives */
    CORRADE_VERIFY(Utility::Path::make(FILETEST_SAVE_DIR));
    Containers::String filename = Utility::Path::join(FILETEST_SAVE_DIR, "nonexistent.txt");
    if(Utility::Path::exists(filename))
        CORRADE_VERIFY(Utility::Path::remove(filename));

    {
        out.str({});
        Debug redirectOutput{&out};
        compare.saveDiagnostic(flags, redirectOutput, FILETEST_SAVE_DIR);
    }

    /* Extreme dogfooding, eheh. We expect the *actual* contents, but under the
       *expected* filename */
    CORRADE_COMPARE(out.str(), Utility::formatString("-> {}\n", filename));
    CORRADE_COMPARE_AS(filename, "Hello World!", FileToString);
}

void StringToFileTest::differentContents() {
    std::stringstream out;

    Comparator<Compare::StringToFile> compare;
    /* The filename is referenced as a string view as the assumption is that
       the whole comparison and diagnostic printing gets done in a single
       expression. Thus don't pass it as a temporary to avoid random failures. */
    Containers::String filenameIn = Utility::Path::join(FILETEST_DIR, "base.txt");
    ComparisonStatusFlags flags = compare("Hello world?", filenameIn);
    CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);

    {
        Debug redirectOutput{&out};
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different contents. Actual character w but W expected on position 6.\n");

    /* Create the output dir if it doesn't exist, but avoid stale files making
       false positives */
    CORRADE_VERIFY(Utility::Path::make(FILETEST_SAVE_DIR));
    Containers::String filenameOut = Utility::Path::join(FILETEST_SAVE_DIR, "base.txt");
    if(Utility::Path::exists(filenameOut))
        CORRADE_VERIFY(Utility::Path::remove(filenameOut));

    {
        out.str({});
        Debug redirectOutput{&out};
        compare.saveDiagnostic(flags, redirectOutput, FILETEST_SAVE_DIR);
    }

    /* Extreme dogfooding, eheh. We expect the *actual* contents, but under the
       *expected* filename */
    CORRADE_COMPARE(out.str(), Utility::formatString("-> {}\n", filenameOut));
    CORRADE_COMPARE_AS(filenameOut, "Hello world?", FileToString);
}

void StringToFileTest::actualSmaller() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::StringToFile> compare;
        /* The filename is referenced as a string view as the assumption is
           that the whole comparison and diagnostic printing gets done in a
           single expression. Thus don't pass it as a temporary to avoid
           dangling views. */
        Containers::String filename = Utility::Path::join(FILETEST_DIR, "base.txt");
        ComparisonStatusFlags flags = compare("Hello W", filename);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);
        compare.printMessage(flags, redirectOutput, "a", "b");
        /* not testing diagnostic as differentContents() tested this code path
           already */
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 7 but 12 expected. Expected has character o on position 7.\n");
}

void StringToFileTest::expectedSmaller() {
    std::stringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<Compare::StringToFile> compare;
        /* The filename is referenced as a string view as the assumption is
           that the whole comparison and diagnostic printing gets done in a
           single expression. Thus don't pass it as a temporary to avoid
           dangling views. */
        Containers::String filename = Utility::Path::join(FILETEST_DIR, "smaller.txt");
        ComparisonStatusFlags flags = compare("Hello World!", filename);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic);
        compare.printMessage(flags, redirectOutput, "a", "b");
        /* not testing diagnostic as differentContents() tested this code path
           already */
    }

    CORRADE_COMPARE(out.str(), "Files a and b have different size, actual 12 but 7 expected. Actual has character o on position 7.\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::StringToFileTest)
