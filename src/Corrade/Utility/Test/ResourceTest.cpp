/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#include <map> /* for a lookup bechmark against std::map<std::string> */
#include <string> /* for a lookup bechmark against std::map<std::string> */

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Resource.h"
#include "Corrade/Utility/Implementation/Resource.h"
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

/* Compiled using corrade_add_resource(... SINGLE), tested with
   single() and singleEmpty() */
extern const unsigned int corradeResourceSize_ResourceTestSingleData;
extern const unsigned int corradeResourceSize_ResourceTestSingleEmptyData;
extern const unsigned char corradeResourceData_ResourceTestSingleData[];
extern const unsigned char corradeResourceData_ResourceTestSingleEmptyData[];

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ResourceTest: TestSuite::Tester {
    explicit ResourceTest();

    void resourceFilenameAt();
    void resourceDataAt();
    void resourceLookup();

    void benchmarkLookupInPlace();
    void benchmarkLookupStdMap();

    void hasGroup();
    void emptyGroup();
    void nonexistentGroup();

    void list();
    void listEmptyGroup();

    void hasFile();
    void getRaw();
    void getString();
    void getEmptyFileRaw();
    void getEmptyFileString();
    void getNonexistentFile();
    void filenameWithSpaces();

    void nullTerminatedAligned();
    void nullTerminatedLastFile();
    void alignmentLargerThanDataSize();

    void overrideGroup();
    void overrideGroupNonexistent();
    void overrideGroupDifferent();
    void overrideGroupFileNonexistent();
    void overrideGroupFileFallback();
    void overrideGroupFileFallbackReadError();

    void single();
    void singleEmpty();
};

ResourceTest::ResourceTest() {
    addTests({&ResourceTest::resourceFilenameAt,
              &ResourceTest::resourceDataAt,
              &ResourceTest::resourceLookup});

    addBenchmarks({&ResourceTest::benchmarkLookupInPlace,
                   &ResourceTest::benchmarkLookupStdMap}, 100);

    addTests({&ResourceTest::hasGroup,
              &ResourceTest::emptyGroup,
              &ResourceTest::nonexistentGroup,

              &ResourceTest::list,
              &ResourceTest::listEmptyGroup,

              &ResourceTest::hasFile,
              &ResourceTest::getRaw,
              &ResourceTest::getString,
              &ResourceTest::getEmptyFileRaw,
              &ResourceTest::getEmptyFileString,
              &ResourceTest::getNonexistentFile,
              &ResourceTest::filenameWithSpaces,

              &ResourceTest::nullTerminatedAligned,
              &ResourceTest::nullTerminatedLastFile,
              &ResourceTest::alignmentLargerThanDataSize,

              &ResourceTest::overrideGroup,
              &ResourceTest::overrideGroupNonexistent,
              &ResourceTest::overrideGroupDifferent,
              &ResourceTest::overrideGroupFileNonexistent,
              &ResourceTest::overrideGroupFileFallback,
              &ResourceTest::overrideGroupFileFallbackReadError,

              &ResourceTest::single,
              &ResourceTest::singleEmpty});
}

using namespace Containers::Literals;

constexpr unsigned int Positions[] {
    3, 6,
    11, 17,
    20 | (1 << 24), 22,
    30, 26,
    40, 45
};

constexpr unsigned char Filenames[] =
    "TOC"           // 3    3
    "data.txt"      // 8    11
    "image.png"     // 9    20
    "image2.png"    // 10   30
    "license.md"    // 10   40
    ;

constexpr unsigned char Data[] =
    "Don't."                    // 6    6
    "hello world"               // 11   17
    "!PNG\0"                    // 4    21 + 1 padding
    "!PnG"                      // 4    26
    "GPL?!\n#####\n\nDon't."    // 19   45
    ;

void ResourceTest::resourceFilenameAt() {
    /* Last position says how large the filenames are */
    CORRADE_COMPARE(sizeof(Filenames) - 1, Positions[4*2]);

    /* First is a special case */
    Containers::StringView toc = Implementation::resourceFilenameAt(Positions, Filenames, 0);
    CORRADE_COMPARE(toc, "TOC");
    CORRADE_COMPARE(toc.flags(), Containers::StringViewFlag::Global);

    /* Third has a one-byte padding, so second has to account for that and it
       shouldn't affect third at all */
    Containers::StringView data = Implementation::resourceFilenameAt(Positions, Filenames, 1);
    CORRADE_COMPARE(data, "data.txt");
    CORRADE_COMPARE(data.flags(), Containers::StringViewFlag::Global);

    Containers::StringView png = Implementation::resourceFilenameAt(Positions, Filenames, 2);
    CORRADE_COMPARE(png, "image.png");
    CORRADE_COMPARE(png.flags(), Containers::StringViewFlag::Global);

    /* Fourth is a regular case */
    Containers::StringView png2 = Implementation::resourceFilenameAt(Positions, Filenames, 3);
    CORRADE_COMPARE(png2, "image2.png");
    CORRADE_COMPARE(png2.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::resourceDataAt() {
    /* Last position says how large the filenames are */
    CORRADE_COMPARE(sizeof(Data) - 1, Positions[4*2 + 1]);

    /* First is a special case */
    Containers::StringView toc = Implementation::resourceDataAt(Positions, Data, 0);
    CORRADE_COMPARE(toc, "Don't.");
    CORRADE_COMPARE(toc.flags(), Containers::StringViewFlag::Global);

    /* Third has a one-byte padding, so second has to account for that */
    Containers::StringView data = Implementation::resourceDataAt(Positions, Data, 1);
    CORRADE_COMPARE(data, "hello world");
    CORRADE_COMPARE(data.flags(), Containers::StringViewFlag::Global);

    /* Third should be marked as null-terminated */
    Containers::StringView png = Implementation::resourceDataAt(Positions, Data, 2);
    CORRADE_COMPARE(png, "!PNG");
    CORRADE_COMPARE(png.flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Fourth is a regular case */
    Containers::StringView png2 = Implementation::resourceDataAt(Positions, Data, 3);
    CORRADE_COMPARE(png2, "!PnG");
    CORRADE_COMPARE(png2.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::resourceLookup() {
    /* The filenames should be sorted */
    for(std::size_t i = 0; i != 4; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE_AS(
            Implementation::resourceFilenameAt(Positions, Filenames, 0),
            Implementation::resourceFilenameAt(Positions, Filenames, 1),
            TestSuite::Compare::Less);
    }

    /* Those exist; third has a one-byte padding so it needs to account for
       that */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "TOC"), 0);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "data.txt"), 1);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "image.png"), 2);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "image2.png"), 3);
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames,
        "license.md"), 4);

    /* An extra null terminator won't match */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "TOC\0"_s), 5);

    /* Lower bound returns license.md, but filename match discards that */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "image3.png"), 5);

    /* Last name is license.md, this is after, so lower bound returns end */
    CORRADE_COMPARE(Implementation::resourceLookup(5, Positions, Filenames, "termcap.info"), 5);
}

CORRADE_NEVER_INLINE unsigned int lookupInPlace(Containers::StringView key) {
    return Implementation::resourceLookup(5, Positions, Filenames, key);
}

CORRADE_NEVER_INLINE unsigned int lookupStdMap(const std::map<std::string, unsigned int>& map, const std::string& key) {
    return map.at(key);
}

void ResourceTest::benchmarkLookupInPlace() {
    const Containers::StringView key = "license.md";
    unsigned int out = 0;
    CORRADE_BENCHMARK(10)
        out += lookupInPlace(key);

    CORRADE_COMPARE(out, 40);
}

void ResourceTest::benchmarkLookupStdMap() {
    std::map<std::string, unsigned int> map{
        {"TOC", 0},
        {"data.txt", 1},
        {"image.png", 2},
        {"image2.png", 3},
        {"license.md", 4},
    };

    std::string key = "license.md";
    unsigned int out = 0;
    CORRADE_BENCHMARK(10)
        out += lookupStdMap(map, key);

    CORRADE_COMPARE(out, 40);
}

void ResourceTest::hasGroup() {
    CORRADE_VERIFY(Resource::hasGroup("test"));
    CORRADE_VERIFY(!Resource::hasGroup("nonexistent"));
}

void ResourceTest::emptyGroup() {
    /* Should not print any error messages about anything */
    Containers::String out;
    Error redirectError{&out};
    Resource rs{"nothing"};
    CORRADE_COMPARE(out, "");
}

void ResourceTest::nonexistentGroup() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Resource rs{"nonexistentGroup"};
    CORRADE_COMPARE(out, "Utility::Resource: group 'nonexistentGroup' was not found\n");
}

void ResourceTest::list() {
    Resource rs{"test"};
    Containers::StringIterable list = rs.list();
    CORRADE_COMPARE_AS(list,
        (Containers::StringIterable{"consequence.bin", "predisposition.bin"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE(list[0].flags(), Containers::StringViewFlag::Global);
    CORRADE_COMPARE(list[1].flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::listEmptyGroup() {
    Resource rs{"nothing"};
    CORRADE_COMPARE_AS(rs.list(),
        Containers::ArrayView<const Containers::StringView>{},
        TestSuite::Compare::Container);
}

void ResourceTest::hasFile() {
    Resource rs{"test"};

    CORRADE_VERIFY(rs.hasFile("predisposition.bin"));
    CORRADE_VERIFY(rs.hasFile("consequence.bin"));
    CORRADE_VERIFY(!rs.hasFile("nonexistentFile"));
}

void ResourceTest::getRaw() {
    Resource rs{"test"};

    CORRADE_COMPARE_AS(rs.getRaw("predisposition.bin"),
        Path::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);

    CORRADE_COMPARE_AS(rs.getRaw("consequence.bin"),
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
}

void ResourceTest::getString() {
    Resource rs{"test"};

    Containers::StringView predisposition = rs.getString("predisposition.bin");
    CORRADE_COMPARE_AS(predisposition,
        Path::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(predisposition.flags(), Containers::StringViewFlag::Global);

    Containers::StringView consequence = rs.getString("consequence.bin");
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::getEmptyFileRaw() {
    Resource rs{"empty"};
    CORRADE_VERIFY(rs.hasFile("empty.bin"));

    Containers::ArrayView<const char> empty = rs.getRaw("empty.bin");
    CORRADE_VERIFY(!empty.data());
    CORRADE_VERIFY(!empty.size());
}

void ResourceTest::getEmptyFileString() {
    Resource rs{"empty"};
    CORRADE_VERIFY(rs.hasFile("empty.bin"));

    Containers::StringView empty = rs.getString("empty.bin");
    CORRADE_VERIFY(!empty.data());
    CORRADE_VERIFY(!empty.size());
    CORRADE_COMPARE(empty.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::getNonexistentFile() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Resource rs{"test"};
    rs.getString("nonexistentFile");
    rs.getRaw("nonexistentFile");
    /* The message is still get() as that's what eventually will getRaw() be
       renamed to; and getString() uses the same underlying code */
    CORRADE_COMPARE(out,
        "Utility::Resource::get(): file 'nonexistentFile' was not found in group 'test'\n"
        "Utility::Resource::get(): file 'nonexistentFile' was not found in group 'test'\n");
}

void ResourceTest::filenameWithSpaces() {
    Resource rs{"spaces"};

    /* Both of these should get compiled correctly as well as found by CMake
       for dependency tracking */
    CORRADE_VERIFY(rs.hasFile("name with spaces.txt"));
    CORRADE_COMPARE(rs.getString("name with spaces.txt"), "hello\n");
    CORRADE_COMPARE_AS(rs.getString("predisposition.bin"),
        Path::join(RESOURCE_TEST_DIR, "predisposition.bin"),
        TestSuite::Compare::StringToFile);
}

void ResourceTest::nullTerminatedAligned() {
    Resource rs{"nullTerminatedAligned"};

    {
        Containers::StringView file = rs.getString("0-null-terminated.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"),
            TestSuite::Compare::StringToFile);
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::NullTerminated|Containers::StringViewFlag::Global);
        CORRADE_COMPARE(file[file.size()], '\0');
    } {
        Containers::StringView file = rs.getString("1.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "17bytes-33.bin"),
            TestSuite::Compare::StringToFile);
        /* There's padding in order to align the next file so it *may* be
           null terminated as well. Don't rely on it tho. */
        CORRADE_COMPARE_AS(file.flags(),
            Containers::StringViewFlag::Global,
            TestSuite::Compare::GreaterOrEqual);
    } {
        Containers::StringView file = rs.getString("2-align16.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"),
            TestSuite::Compare::StringToFile);
        /* There's padding in order to align the next file so it *may* be
           null terminated as well. Don't rely on it tho. */
        CORRADE_COMPARE_AS(file.flags(),
            Containers::StringViewFlag::Global,
            TestSuite::Compare::GreaterOrEqual);
        CORRADE_COMPARE_AS(file.data(), 16, TestSuite::Compare::Aligned);
    } {
        Containers::StringView file = rs.getString("3-align4-empty.bin");
        {
            #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
            /* Emscripten 2.0.26+ has a problem in the file embedder, where
               zero-size files are reported as having 3 bytes. The changelog
               between 2.0.25 and 2.0.26 doesn't mention anything related, the
               only related change I found was
                https://github.com/emscripten-core/emscripten/pull/14526
               going into 2.0.25 already, and I suspect it's something related
               to padding in base64 decode. This problem is gone in 3.1.3,
               where they replace the base64 file embedding with putting a
               binary directly to wasm in
                https://github.com/emscripten-core/emscripten/pull/16050
               Which then however breaks UTF-8 paths, see the CORRADE_SKIP()
               elsewhere.

               Also seems to happen only with Node.js 14 that's bundled with
               emsdk, not with external version 18. Node.js 15+ is only bundled
               with emsdk 3.1.35+ which doesn't suffer from this 3-byte bug
               anymore. */
            CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
                "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
            #endif
            CORRADE_COMPARE_AS(file,
                Path::join(RESOURCE_TEST_DIR, "empty.bin"),
                TestSuite::Compare::StringToFile);
        }
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE_AS(file.data(), 4, TestSuite::Compare::Aligned);
    } {
        Containers::StringView file = rs.getString("4-null-terminated-empty.bin");
        {
            #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
            /* See above */
            CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
                "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
            #endif
            CORRADE_COMPARE_AS(file,
                Path::join(RESOURCE_TEST_DIR, "empty.bin"),
                TestSuite::Compare::StringToFile);
        }
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::NullTerminated|Containers::StringViewFlag::Global);
        CORRADE_COMPARE(file[file.size()], '\0');
    } {
        Containers::StringView file = rs.getString("5-null-terminated-align8-empty.bin");
        {
            #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
            /* See above */
            CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
                "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
            #endif
            CORRADE_COMPARE_AS(file,
                Path::join(RESOURCE_TEST_DIR, "empty.bin"),
                TestSuite::Compare::StringToFile);
        }
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::NullTerminated|Containers::StringViewFlag::Global);
        CORRADE_COMPARE(file[file.size()], '\0');
        CORRADE_COMPARE_AS(file.data(), 8, TestSuite::Compare::Aligned);
    } {
        Containers::StringView file = rs.getString("6-null-terminated-align64.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "64bytes-33.bin"),
            TestSuite::Compare::StringToFile);
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::NullTerminated|Containers::StringViewFlag::Global);
        CORRADE_COMPARE(file[file.size()], '\0');
        CORRADE_COMPARE_AS(file.data(), 64, TestSuite::Compare::Aligned);
    } {
        Containers::StringView file = rs.getString("7-align64.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "55bytes-66.bin"),
            TestSuite::Compare::StringToFile);
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE_AS(file.data(), 64, TestSuite::Compare::Aligned);
    } {
        Containers::StringView file = rs.getString("8.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "17bytes-33.bin"),
            TestSuite::Compare::StringToFile);
        CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::Global);
    }
}

void ResourceTest::nullTerminatedLastFile() {
    Resource rs{"nullTerminatedLastFile"};

    Containers::StringView file = rs.getString("0-null-terminated.bin");
    CORRADE_COMPARE_AS(file,
        Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(file.flags(), Containers::StringViewFlag::NullTerminated|Containers::StringViewFlag::Global);
    CORRADE_COMPARE(file[file.size()], '\0');
}

void ResourceTest::alignmentLargerThanDataSize() {
    Resource rs{"alignmentLargerThanDataSize"};

    {
        Containers::StringView file = rs.getString("0-align128.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "17bytes-66.bin"),
            TestSuite::Compare::StringToFile);
        /* There's padding in order to satisfy the alignment so it *may* be
           null terminated as well. Don't rely on it tho. */
        CORRADE_COMPARE_AS(file.flags(),
            Containers::StringViewFlag::Global,
            TestSuite::Compare::GreaterOrEqual);
        CORRADE_COMPARE_AS(file.data(), 128, TestSuite::Compare::Aligned);

        /* It should be possible to access all 128 bytes without triggering
           ASan or some page fault. Access the raw data directly because it'd
           trigger an OOB assertion in operator[] otherwise */
        CORRADE_COMPARE(file.data()[127], '\0');

    /* The remaining files should still have their data as usual even though
       overlapping with the first one's alignment */
    } {
        Containers::StringView file = rs.getString("1.bin");
        CORRADE_COMPARE_AS(file,
            Path::join(RESOURCE_TEST_DIR, "64bytes-33.bin"),
            TestSuite::Compare::StringToFile);
        /* There's padding in order to align the next file so it *may* be
           null terminated as well. Don't rely on it tho. */
        CORRADE_COMPARE_AS(file.flags(),
            Containers::StringViewFlag::Global,
            TestSuite::Compare::GreaterOrEqual);
    } {
        Containers::StringView file = rs.getString("2-align2-empty.bin");
        {
            #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
            /* Emscripten 2.0.26+ has a problem in the file embedder, where
               zero-size files are reported as having 3 bytes. The changelog
               between 2.0.25 and 2.0.26 doesn't mention anything related, the
               only related change I found was
                https://github.com/emscripten-core/emscripten/pull/14526
               going into 2.0.25 already, and I suspect it's something related
               to padding in base64 decode. This problem is gone in 3.1.3,
               where they replace the base64 file embedding with putting a
               binary directly to wasm in
                https://github.com/emscripten-core/emscripten/pull/16050
               Which then however breaks UTF-8 paths, see the CORRADE_SKIP()
               elsewhere.

               Also seems to happen only with Node.js 14 that's bundled with
               emsdk, not with external version 18. Node.js 15+ is only bundled
               with emsdk 3.1.35+ which doesn't suffer from this 3-byte bug
               anymore. */
            CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
                "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
            #endif
            CORRADE_COMPARE_AS(file,
                Path::join(RESOURCE_TEST_DIR, "empty.bin"),
                TestSuite::Compare::StringToFile);
        }
        /* There's padding in order to satisfy the alignment file so it *may*
           be null terminated as well. Don't rely on it tho. */
        CORRADE_COMPARE_AS(file.flags(),
            Containers::StringViewFlag::Global,
            TestSuite::Compare::GreaterOrEqual);
        CORRADE_COMPARE_AS(file.data(), 2, TestSuite::Compare::Aligned);
    }
}

void ResourceTest::overrideGroup() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden.conf"));

    Containers::String out;
    Debug redirectDebug{&out};
    Resource rs{"test"};
    CORRADE_COMPARE(out, format("Utility::Resource: group 'test' overridden with '{}'\n", Path::join(RESOURCE_TEST_DIR, "resources-overridden.conf")));

    /* Overriden files are not marked as global but are null-terminated */
    Containers::StringView predisposition = rs.getString("predisposition.bin");
    CORRADE_COMPARE(predisposition, "overridden predisposition\n");
    CORRADE_COMPARE(predisposition.flags(), Containers::StringViewFlag::NullTerminated);

    /* Two subsequent calls should point to the same location (the file doesn't
    get read again) */
    CORRADE_VERIFY(rs.getString("predisposition.bin").data() == predisposition.data());
}

void ResourceTest::overrideGroupNonexistent() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Resource::overrideGroup("nonexistentGroup", {});
    CORRADE_COMPARE(out, "Utility::Resource::overrideGroup(): group 'nonexistentGroup' was not found\n");
}

void ResourceTest::overrideGroupDifferent() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-different.conf"));

    Containers::String out;
    Warning redirectWarning{&out};
    Resource rs{"test"};
    CORRADE_COMPARE(out, "Utility::Resource: overridden with different group, found 'wat' but expected 'test'\n");
}

void ResourceTest::overrideGroupFileNonexistent() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-nonexistent-file.conf"));
    Resource rs{"test"};
    /* The file is in the overriden group, but not in the compiled-in data and
       thus it fails */
    CORRADE_VERIFY(!rs.hasFile("consequence2.bin"));

    Containers::String out;
    Error redirectError{&out};
    rs.getString("consequence2.txt");
    /* The file is in the overriden group, but not in the compiled-in data and
       thus it fails */
    CORRADE_COMPARE(out, "Utility::Resource::get(): file 'consequence2.txt' was not found in group 'test'\n");
}

void ResourceTest::overrideGroupFileFallback() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-none.conf"));
    Resource rs{"test"};

    Containers::String out;
    Warning redirectWarning{&out};
    Containers::StringView consequence = rs.getString("consequence.bin");
    CORRADE_COMPARE(out, "Utility::Resource::get(): file 'consequence.bin' was not found in overridden group, fallback to compiled-in resources\n");

    /* Original compiled-in file, global flag (but implicitly not
       null-terminated) */
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::overrideGroupFileFallbackReadError() {
    Resource::overrideGroup("test", Path::join(RESOURCE_TEST_DIR, "resources-overridden-nonexistent-file.conf"));
    Resource rs{"test"};

    Containers::String out;
    Error redirectError{&out};
    Warning redirectWarning(&out);
    Containers::StringView consequence = rs.getString("consequence.bin");
    /* There's an error message from Path::read() before */
    CORRADE_COMPARE_AS(out,
        "\nUtility::Resource::get(): cannot open file path/to/nonexistent.bin from overridden group\n"
        "Utility::Resource::get(): file 'consequence.bin' was not found in overridden group, fallback to compiled-in resources\n",
        TestSuite::Compare::StringHasSuffix);

    /* Original compiled-in file, global flag (but implicitly not
       null-terminated) */
    CORRADE_COMPARE_AS(consequence,
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
    CORRADE_COMPARE(consequence.flags(), Containers::StringViewFlag::Global);
}

void ResourceTest::single() {
    CORRADE_COMPARE_AS((Containers::StringView{reinterpret_cast<const char*>(corradeResourceData_ResourceTestSingleData), corradeResourceSize_ResourceTestSingleData}),
        Path::join(RESOURCE_TEST_DIR, "consequence.bin"),
        TestSuite::Compare::StringToFile);
}

void ResourceTest::singleEmpty() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
    /* Emscripten 2.0.26+ has a problem in the file embedder, where zero-size
       files are reported as having 3 bytes. The changelog between 2.0.25 and
       2.0.26 doesn't mention anything related, the only related change I found
       was https://github.com/emscripten-core/emscripten/pull/14526, going into
       2.0.25 already, and I suspect it's something related to padding in
       base64 decode. This problem is gone in 3.1.3, where they replace the
       base64 file embedding with putting a binary directly to wasm in
       https://github.com/emscripten-core/emscripten/pull/16050. Which then
       however breaks UTF-8 paths, see the CORRADE_SKIP() elsewhere.

       Also seems to happen only with Node.js 14 that's bundled with emsdk, not
       with external version 18. Node.js 15+ is only bundled with emsdk 3.1.35+
       which doesn't suffer from this 3-byte bug anymore. */
    CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
        "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
    #endif
    CORRADE_COMPARE_AS((Containers::StringView{reinterpret_cast<const char*>(corradeResourceData_ResourceTestSingleEmptyData), corradeResourceSize_ResourceTestSingleEmptyData}),
        Path::join(RESOURCE_TEST_DIR, "empty.bin"),
        TestSuite::Compare::StringToFile);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ResourceTest)
