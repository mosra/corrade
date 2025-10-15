/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"
#include "Corrade/Utility/Algorithms.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/System.h"
#ifdef CORRADE_TARGET_APPLE
#include "Corrade/Utility/System.h" /* isSandboxed() */
#endif
#ifdef CORRADE_TARGET_EMSCRIPTEN
#include "Corrade/Utility/Test/nodeJsVersionHelpers.h"
#endif

#ifdef CORRADE_TARGET_UNIX
/* Needed for chdir() in currentInvalid() */
#include <unistd.h>
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

namespace Corrade { namespace Utility { namespace Test { namespace {

struct PathTest: TestSuite::Tester {
    explicit PathTest();

    void fromNativeSeparators();
    void fromNativeSeparatorsRvalue();
    void toNativeSeparators();
    void toNativeSeparatorsRvalue();

    /* Tests also path() and filename() */
    void split();
    void splitFlags();
    void splitExtension();
    void splitExtensionFlags();

    void join();
    #ifdef CORRADE_TARGET_WINDOWS
    void joinWindows();
    #endif
    void joinMultiple();
    void joinMultipleAbsolute();
    void joinMultipleOneEmpty();
    void joinMultipleJustOne();
    void joinMultipleNone();

    void exists();
    void existsNoPermission();
    void existsNonNullTerminated();
    void existsUtf8();

    void isDirectory();
    void isDirectorySymlink();
    void isDirectoryNoPermission();
    void isDirectoryNonNullTerminated();
    void isDirectoryUtf8();

    void make();
    void makeExistsAsAFile();
    void makeExistsAsADirectorySymlink();
    void makeDotDotDot();
    void makeNoPermission();
    void makeNonNullTerminated();
    void makeUtf8();

    void removeFile();
    void removeDirectory();
    void removeFileNonexistent();
    void removeDirectoryNonEmpty();
    void removeFileNonNullTerminated();
    void removeDirectoryNonNullTerminated();
    void removeFileUtf8();
    void removeDirectoryUtf8();

    void moveFile();
    void moveDirectory();
    void moveSourceNonexistent();
    void moveDestinationNoPermission();
    void moveDestinationExists();
    void moveNonNullTerminated();
    void moveUtf8();

    void currentDirectory();
    void currentDirectoryNonexistent();
    void currentDirectoryUtf8();

    #ifndef CORRADE_BUILD_STATIC
    void libraryLocation();
    #else
    void libraryLocationStatic();
    #endif
    void libraryLocationNull();
    void libraryLocationInvalid();
    void libraryLocationUtf8();

    void executableLocation();
    void executableLocationInvalid();
    void executableLocationUtf8();

    void homeDirectory();
    void homeDirectoryInvalid();
    void homeDirectoryUtf8();

    void configurationDirectory();
    void configurationDirectoryInvalid();
    void configurationDirectoryUtf8();

    void temporaryDirectory();
    void temporaryDirectoryInvalid();
    void temporaryDirectoryUtf8();

    void list();
    void listIterateRangeFor();
    void listEmptyDirectory();
    void listSkipDirectories();
    void listSkipDirectoriesSymlinks();
    void listSkipFiles();
    void listSkipFilesSymlinks();
    void listSkipSpecial();
    void listSkipSpecialSymlink();
    void listSkipDotAndDotDot();
    void listSkipEverything();
    void listSort();
    void listNonexistent();
    void listNonNullTerminated();
    void listTrailingSlash();
    void listUtf8Result();
    void listUtf8Path();

    void size();
    void sizeEmpty();
    void sizeNonSeekable();
    void sizeEarlyEof();
    void sizeSymlink();
    void sizeDirectory();
    void sizeNonexistent();
    void sizeNonNullTerminated();
    void sizeUtf8();

    void lastModification();
    void lastModificationNonexistent();
    void lastModificationNonNullTerminated();
    void lastModificationUtf8();

    void read();
    void readString();
    void readEmpty();
    void readEmptyString();
    void readNonSeekable();
    void readNonSeekableString();
    void readEarlyEof();
    void readEarlyEofString();
    void readDirectory();
    void readNonexistent();
    void readNonNullTerminated();
    void readUtf8();

    void write();
    void writeDisabledOverloads();
    void writeEmpty();
    void writeDirectory();
    void writeNoPermission();
    void writeNonNullTerminated();
    void writeUtf8();

    void append();
    void appendDisabledOverloads();
    void appendToNonexistent();
    void appendEmpty();
    void appendDirectory();
    void appendNoPermission();
    void appendNonNullTerminated();
    void appendUtf8();

    void prepareFileToCopy();
    void copy();
    void copyEmpty();
    void copyDirectory();
    void copyReadNonexistent();
    void copyWriteNoPermission();
    void copyNonNullTerminated();
    void copyUtf8();

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void prepareFileToBenchmarkCopy();
    void copy100MReadWrite();
    void copy100MReadWriteString();
    void copy100MCopy();
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    void copy100MMap();
    #endif
    #endif

    void map();
    void mapEmpty();
    void mapDirectory();
    void mapNonexistent();
    void mapNonNullTerminated();
    void mapUtf8();

    void mapRead();
    void mapReadEmpty();
    void mapReadDirectory();
    void mapReadNonexistent();
    void mapReadNonNullTerminated();
    void mapReadUtf8();

    void mapWrite();
    void mapWriteEmpty();
    void mapWriteDirectory();
    void mapWriteNoPermission();
    void mapWriteNonNullTerminated();
    void mapWriteUtf8();

    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    void readWriteWhileMapped();
    #endif

    Containers::String _testDir,
        _testDirSymlink,
        _testDirUtf8,
        _writeTestDir;
};

using namespace Containers::Literals;

const struct {
    const char* name;
    const char* path;
    Containers::Pair<Containers::StringView, Containers::StringView> expected;
} SplitData[]{
    /* In case you're not sure about the behavior, cross-check with Python's
       os.path.split(). */
    {"empty", "",
        {"", ""}},
    {"no path", "foo.txt",
        {"", "foo.txt"}},
    {"no filename", ".config/corrade/",
        {".config/corrade", ""}},
    {"common case", "foo/bar/map.conf",
        {"foo/bar", "map.conf"}},
    {"absolute path", "/foo/bar/map.conf",
        {"/foo/bar", "map.conf"}},
    {"absolute network path", "//computer/foo/bar/map.conf",
        {"//computer/foo/bar", "map.conf"}},
    {"absolute path with no filename", "/root",
        {"/", "root"}},
    {"root slash alone doesn't get dropped", "/",
        {"/", ""}},
    {"absolute network path with no filename", "//computer",
        {"//", "computer"}},
    {"double root slash alone doesn't get dropped either", "//",
        {"//", ""}},
};

const struct {
    const char* name;
    Containers::StringView path;
    Containers::Pair<Containers::StringView, Containers::StringView> expected;
    Containers::StringViewFlags expectedFlagsPath;
    Containers::StringViewFlags expectedFlagsFilename;
} SplitFlagsData[]{
    {"empty should preserve both null-terminated flags",
        ""_s, {"", ""},
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated,
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated},
    {"filename only the second",
        "/path"_s, {"/", "path"},
        Containers::StringViewFlag::Global,
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated},
    {"path alone only the second as / gets dropped",
        "path/"_s, {"path", ""},
        Containers::StringViewFlag::Global,
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated},
    {"root alone both",
        "/"_s, {"/", ""},
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated,
        Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated},
    {"non-literal should not become global",
        "path/file.txt", {"path", "file.txt"},
        {},
        Containers::StringViewFlag::NullTerminated},
    {"non-null-terminated should not become such",
        "path/file.txt!"_s.exceptSuffix(1), {"path", "file.txt"},
        Containers::StringViewFlag::Global,
        Containers::StringViewFlag::Global},
};

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
Containers::Optional<Containers::Array<const char, Path::MapDeleter>> writeWhileMappedMap(Containers::StringView filename, std::size_t) {
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mapped = Path::map(filename);
    if(!mapped)
        return {};
    /* The compiler may execute the expressions in {} in an arbitrary order,
       potentially leading to release() clearing size or deleter before they'd
       be queried, explicitly get them first */
    std::size_t size = mapped->size();
    Path::MapDeleter deleter = mapped->deleter();
    return Containers::Optional<Containers::Array<const char, Path::MapDeleter>>{InPlaceInit, mapped->release(), size, deleter};
}

Containers::Optional<Containers::Array<const char, Path::MapDeleter>> writeWhileMappedMapRead(Containers::StringView filename, std::size_t) {
    return Path::mapRead(filename);
}

Containers::Optional<Containers::Array<const char, Path::MapDeleter>> writeWhileMappedMapWrite(Containers::StringView filename, std::size_t size) {
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mapped = Path::mapWrite(filename, size);
    if(!mapped)
        return {};
    /* The compiler may execute the expressions in {} in an arbitrary order,
       potentially leading to release() clearing deleter before it'd be
       queried, explicitly get it first. Size is same as passed. */
    Path::MapDeleter deleter = mapped->deleter();
    return Containers::Optional<Containers::Array<const char, Path::MapDeleter>>{InPlaceInit, mapped->release(), size, deleter};
}

const struct {
    const char* name;
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>>(*map)(Containers::StringView, std::size_t);
    bool canModify;
    Containers::StringView expectedBefore;
    Containers::StringView write;
    Containers::StringView expectedAfter;
    Containers::StringView expectedFileAfterModification;
} ReadWriteWhileMappedData[]{
    {"same size, map", writeWhileMappedMap, true,
        "hello this is a file",
        "HELLO THIS IS A FILE",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FILE"
        #else
        "Yello this is a file"
        #endif
    },
    {"same size, mapRead", writeWhileMappedMapRead, false,
        "hello this is a file",
        "HELLO THIS IS A FILE",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "HELLO THIS IS A FILE"
        #else
        "hello this is a file"
        #endif
    },
    {"same size, mapWrite", writeWhileMappedMapWrite, true,
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s,
        "HELLO THIS IS A FILE",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FILE"
        #else
        "Y\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s
        #endif
    },
    {"shorter, map", writeWhileMappedMap, true,
        "hello this is a file",
        "HELLO THIS IS A FI",
        "HELLO THIS IS A FI\0\0"_s,
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FI"
        #else
        "Yello this is a file"
        #endif
    },
    {"shorter, mapRead", writeWhileMappedMapRead, false,
        "hello this is a file",
        "HELLO THIS IS A FI",
        "HELLO THIS IS A FI\0\0"_s,
        #ifndef CORRADE_TARGET_WINDOWS
        "HELLO THIS IS A FI"
        #else
        "hello this is a file"
        #endif
    },
    {"shorter, mapWrite", writeWhileMappedMapWrite, true,
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s,
        "HELLO THIS IS A FI",
        "HELLO THIS IS A FI\0\0"_s,
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FI"
        #else
        "Y\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s
        #endif
    },
    {"longer, map", writeWhileMappedMap, true,
        "hello this is a file",
        "HELLO THIS IS A FILE!!",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FILE!!"
        #else
        "Yello this is a file"
        #endif
    },
    {"longer, mapRead", writeWhileMappedMapRead, false,
        "hello this is a file",
        "HELLO THIS IS A FILE!!",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "HELLO THIS IS A FILE!!"
        #else
        "hello this is a file"
        #endif
    },
    {"longer, mapWrite", writeWhileMappedMapWrite, true,
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s,
        "HELLO THIS IS A FILE!!",
        "HELLO THIS IS A FILE",
        #ifndef CORRADE_TARGET_WINDOWS
        "YELLO THIS IS A FILE!!"
        #else
        "Y\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_s
        #endif
    },
};
#endif

PathTest::PathTest() {
    addTests({&PathTest::fromNativeSeparators,
              &PathTest::fromNativeSeparatorsRvalue,
              &PathTest::toNativeSeparators,
              &PathTest::toNativeSeparatorsRvalue});

    addInstancedTests({&PathTest::split},
        Containers::arraySize(SplitData));

    addInstancedTests({&PathTest::splitFlags},
        Containers::arraySize(SplitFlagsData));

    addTests({&PathTest::splitExtension,
              &PathTest::splitExtensionFlags,

              &PathTest::join,
              #ifdef CORRADE_TARGET_WINDOWS
              &PathTest::joinWindows,
              #endif
              &PathTest::joinMultiple,
              &PathTest::joinMultipleAbsolute,
              &PathTest::joinMultipleOneEmpty,
              &PathTest::joinMultipleJustOne,
              &PathTest::joinMultipleNone,

              &PathTest::exists,
              &PathTest::existsNoPermission,
              &PathTest::existsNonNullTerminated,
              &PathTest::existsUtf8,

              &PathTest::isDirectory,
              &PathTest::isDirectorySymlink,
              &PathTest::isDirectoryNoPermission,
              &PathTest::isDirectoryNonNullTerminated,
              &PathTest::isDirectoryUtf8,

              &PathTest::make,
              &PathTest::makeExistsAsAFile,
              &PathTest::makeExistsAsADirectorySymlink,
              &PathTest::makeDotDotDot,
              &PathTest::makeNoPermission,
              &PathTest::makeNonNullTerminated,
              &PathTest::makeUtf8,

              &PathTest::removeFile,
              &PathTest::removeDirectory,
              &PathTest::removeFileNonexistent,
              &PathTest::removeDirectoryNonEmpty,
              &PathTest::removeFileNonNullTerminated,
              &PathTest::removeDirectoryNonNullTerminated,
              &PathTest::removeFileUtf8,
              &PathTest::removeDirectoryUtf8,

              &PathTest::moveFile,
              &PathTest::moveDirectory,
              &PathTest::moveSourceNonexistent,
              &PathTest::moveDestinationNoPermission,
              &PathTest::moveDestinationExists,
              &PathTest::moveNonNullTerminated,
              &PathTest::moveUtf8,

              /* These don't pass any strings to system APIs, so no need to
                 verify non-null-terminated variants */
              &PathTest::currentDirectory,
              &PathTest::currentDirectoryNonexistent,
              &PathTest::currentDirectoryUtf8,

              #ifndef CORRADE_BUILD_STATIC
              &PathTest::libraryLocation,
              #else
              &PathTest::libraryLocationStatic,
              #endif
              &PathTest::libraryLocationNull,
              &PathTest::libraryLocationInvalid,
              &PathTest::libraryLocationUtf8,

              &PathTest::executableLocation,
              &PathTest::executableLocationInvalid,
              &PathTest::executableLocationUtf8,

              &PathTest::homeDirectory,
              &PathTest::homeDirectoryInvalid,
              &PathTest::homeDirectoryUtf8,

              &PathTest::configurationDirectory,
              &PathTest::configurationDirectoryInvalid,
              &PathTest::configurationDirectoryUtf8,

              &PathTest::temporaryDirectory,
              &PathTest::temporaryDirectoryInvalid,
              &PathTest::temporaryDirectoryUtf8,

              &PathTest::list,
              &PathTest::listIterateRangeFor,
              &PathTest::listEmptyDirectory,
              &PathTest::listSkipDirectories,
              &PathTest::listSkipDirectoriesSymlinks,
              &PathTest::listSkipFiles,
              &PathTest::listSkipFilesSymlinks,
              &PathTest::listSkipSpecial,
              &PathTest::listSkipSpecialSymlink,
              &PathTest::listSkipDotAndDotDot,
              &PathTest::listSkipEverything,
              &PathTest::listSort,
              &PathTest::listNonexistent,
              &PathTest::listNonNullTerminated,
              &PathTest::listTrailingSlash,
              &PathTest::listUtf8Result,
              &PathTest::listUtf8Path,

              &PathTest::size,
              &PathTest::sizeEmpty,
              &PathTest::sizeNonSeekable,
              &PathTest::sizeEarlyEof,
              &PathTest::sizeSymlink,
              &PathTest::sizeDirectory,
              &PathTest::sizeNonexistent,
              &PathTest::sizeNonNullTerminated,
              &PathTest::sizeUtf8,

              &PathTest::lastModification,
              &PathTest::lastModificationNonexistent,
              &PathTest::lastModificationNonNullTerminated,
              &PathTest::lastModificationUtf8,

              &PathTest::read,
              &PathTest::readString,
              &PathTest::readEmpty,
              &PathTest::readEmptyString,
              &PathTest::readNonSeekable,
              &PathTest::readNonSeekableString,
              &PathTest::readEarlyEof,
              &PathTest::readEarlyEofString,
              &PathTest::readDirectory,
              &PathTest::readNonexistent,
              &PathTest::readNonNullTerminated,
              &PathTest::readUtf8,

              &PathTest::write,
              &PathTest::writeDisabledOverloads,
              &PathTest::writeEmpty,
              &PathTest::writeDirectory,
              &PathTest::writeNoPermission,
              &PathTest::writeNonNullTerminated,
              &PathTest::writeUtf8,

              &PathTest::append,
              &PathTest::appendDisabledOverloads,
              &PathTest::appendToNonexistent,
              &PathTest::appendEmpty,
              &PathTest::appendDirectory,
              &PathTest::appendNoPermission,
              &PathTest::appendNonNullTerminated,
              &PathTest::appendUtf8});

    addTests({&PathTest::copy},
        &PathTest::prepareFileToCopy,
        &PathTest::prepareFileToCopy);

    addTests({&PathTest::copyEmpty,
              &PathTest::copyDirectory,
              &PathTest::copyReadNonexistent,
              &PathTest::copyWriteNoPermission});

    addTests({&PathTest::copyNonNullTerminated},
        &PathTest::prepareFileToCopy,
        &PathTest::prepareFileToCopy);

    addTests({&PathTest::copyUtf8});

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    addBenchmarks({
        &PathTest::copy100MReadWrite,
        &PathTest::copy100MReadWriteString,
        &PathTest::copy100MCopy,
        #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
        &PathTest::copy100MMap
        #endif
        }, 5,
        &PathTest::prepareFileToBenchmarkCopy,
        &PathTest::prepareFileToBenchmarkCopy);
    #endif

    addTests({&PathTest::map,
              &PathTest::mapEmpty,
              &PathTest::mapDirectory,
              &PathTest::mapNonexistent,
              &PathTest::mapNonNullTerminated,
              &PathTest::mapUtf8,

              &PathTest::mapRead,
              &PathTest::mapReadEmpty,
              &PathTest::mapReadDirectory,
              &PathTest::mapReadNonexistent,
              &PathTest::mapReadNonNullTerminated,
              &PathTest::mapReadUtf8,

              &PathTest::mapWrite,
              &PathTest::mapWriteEmpty,
              &PathTest::mapWriteDirectory,
              &PathTest::mapWriteNoPermission,
              &PathTest::mapWriteNonNullTerminated,
              &PathTest::mapWriteUtf8});

    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    addInstancedTests({&PathTest::readWriteWhileMapped},
        Containers::arraySize(ReadWriteWhileMappedData));
    #endif

    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        /** @todo Fix this once I persuade CMake to run XCTest tests properly */
        && std::getenv("SIMULATOR_UDID")
        #endif
    ) {
        _testDir = Path::join(Path::path(*Path::executableLocation()), "PathTestFiles");
        _testDirSymlink = Path::join(Path::path(*Path::executableLocation()), "PathTestFilesSymlink");
        _testDirUtf8 = Path::join(Path::path(*Path::executableLocation()), "PathTestFilesUtf8");
        _writeTestDir = Path::join(*Path::homeDirectory(), "Library/Caches");
    } else
    #endif
    {
        _testDir = Containers::String::nullTerminatedView(PATH_TEST_DIR);
        _testDirSymlink = Containers::String::nullTerminatedView(PATH_TEST_DIR_SYMLINK);
        _testDirUtf8 = Containers::String::nullTerminatedView(PATH_TEST_DIR_UTF8);
        _writeTestDir = Containers::String::nullTerminatedView(PATH_WRITE_TEST_DIR);
    }

    /* Delete the file for copy tests to avoid using a stale version */
    if(Path::exists(Path::join(_writeTestDir, "copySource.dat")))
        Path::remove(Path::join(_writeTestDir, "copySource.dat"));
    if(Path::exists(Path::join(_writeTestDir, "copyBenchmarkSource.dat")))
        Path::remove(Path::join(_writeTestDir, "copyBenchmarkSource.dat"));
}

void PathTest::fromNativeSeparators() {
    Containers::String nativeSeparators = Path::fromNativeSeparators("put\\ that/somewhere\\ else");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "put/ that/somewhere/ else");
    #else
    CORRADE_COMPARE(nativeSeparators, "put\\ that/somewhere\\ else");
    #endif
}

void PathTest::fromNativeSeparatorsRvalue() {
    /* Make sure the string is not SSO'd, as that would also not preserve the
       same pointer */
    Containers::String input{Containers::AllocatedInit, "foo\\bar/"};
    const void* inputData = input.data();
    CORRADE_VERIFY(!input.isSmall());

    /* On Windows it's passing through a String instance, elsewhere it's a
       StringView (and by using String we'd get a copy) */
    #ifdef CORRADE_TARGET_WINDOWS
    Containers::String nativeSeparators = Path::fromNativeSeparators(Utility::move(input));
    CORRADE_COMPARE(nativeSeparators, "foo/bar/");
    #else
    Containers::StringView nativeSeparators = Path::fromNativeSeparators(input);
    CORRADE_COMPARE(nativeSeparators, "foo\\bar/");
    #endif

    /* It should pass the data through if possible */
    CORRADE_COMPARE(nativeSeparators.data(), inputData);
}

void PathTest::toNativeSeparators() {
    Containers::String nativeSeparators = Path::toNativeSeparators("this\\is a weird/system\\right");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird\\system\\right");
    #else
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird/system\\right");
    #endif
}

void PathTest::toNativeSeparatorsRvalue() {
    /* Make sure the string is not SSO'd, as that would also not preserve the
       same pointer */
    Containers::String input{Containers::AllocatedInit, "foo\\bar/"};
    const void* inputData = input.data();
    CORRADE_VERIFY(!input.isSmall());

    /* On Windows it's passing through a String instance, elsewhere it's a
       StringView (and by using String we'd get a copy) */
    #ifdef CORRADE_TARGET_WINDOWS
    Containers::String nativeSeparators = Path::toNativeSeparators(Utility::move(input));
    CORRADE_COMPARE(nativeSeparators, "foo\\bar\\");
    #else
    Containers::StringView nativeSeparators = Path::toNativeSeparators(input);
    CORRADE_COMPARE(nativeSeparators, "foo\\bar/");
    #endif
    /* It should pass the data through if possible */
    CORRADE_COMPARE(nativeSeparators.data(), inputData);
}

void PathTest::split() {
    auto&& data = SplitData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE(Path::split(data.path), data.expected);
    CORRADE_COMPARE(Path::path(data.path), data.expected.first());
    CORRADE_COMPARE(Path::filename(data.path), data.expected.second());
}

void PathTest::splitFlags() {
    auto&& data = SplitFlagsData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Pair<Containers::StringView, Containers::StringView> a = Path::split(data.path);
    CORRADE_COMPARE(a, data.expected);
    CORRADE_COMPARE(a.first().flags(), data.expectedFlagsPath);
    CORRADE_COMPARE(a.second().flags(), data.expectedFlagsFilename);

    Containers::StringView path = Path::path(data.path);
    CORRADE_COMPARE(path, data.expected.first());
    CORRADE_COMPARE(path.flags(), data.expectedFlagsPath);

    Containers::StringView filename = Path::filename(data.path);
    CORRADE_COMPARE(filename, data.expected.second());
    CORRADE_COMPARE(filename.flags(), data.expectedFlagsFilename);
}

void PathTest::splitExtension() {
    /* In case you're not sure about the behavior, cross-check with Python's
       os.path.splitext(). */

    /* Empty */
    CORRADE_COMPARE(Path::splitExtension(""),
        Containers::pair(""_s, ""_s));

    /* Common case */
    CORRADE_COMPARE(Path::splitExtension("file.txt"),
        Containers::pair("file"_s, ".txt"_s));

    /* Double extension */
    CORRADE_COMPARE(Path::splitExtension("file.tar.gz"),
        Containers::pair("file.tar"_s, ".gz"_s));

    /* No extension */
    CORRADE_COMPARE(Path::splitExtension("/etc/passwd"),
        Containers::pair("/etc/passwd"_s, ""_s));

    /* Dot not a part of the file */
    CORRADE_COMPARE(Path::splitExtension("/etc/rc.d/file"),
        Containers::pair("/etc/rc.d/file"_s, ""_s));

    /* Dot at the end */
    CORRADE_COMPARE(Path::splitExtension("/home/no."),
        Containers::pair("/home/no"_s, "."_s));

    /* Dotfile, prefixed or not */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/.bashrc"),
        Containers::pair("/home/mosra/.bashrc"_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension(".bashrc"),
        Containers::pair(".bashrc"_s, ""_s));

    /* One level up, prefixed or not */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/Code/.."),
        Containers::pair("/home/mosra/Code/.."_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension(".."),
        Containers::pair(".."_s, ""_s));

    /* This directory */
    CORRADE_COMPARE(Path::splitExtension("/home/mosra/."),
        Containers::pair("/home/mosra/."_s, ""_s));
    CORRADE_COMPARE(Path::splitExtension("."),
        Containers::pair("."_s, ""_s));

    /* More dots at the start */
    CORRADE_COMPARE(Path::splitExtension("... And Justice For All.mp3"),
        Containers::pair("... And Justice For All"_s, ".mp3"_s));
    CORRADE_COMPARE(Path::splitExtension("... And Justice For All"),
        Containers::pair("... And Justice For All"_s, ""_s));
}

void PathTest::splitExtensionFlags() {
    /* Empty should preserve both null-terminated flags */
    {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension(""_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Filename with extension only the second */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file.txt"_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Extension-less file both */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file"_s);
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Non-literal should not be global */
    } {
        Containers::Pair<Containers::StringView, Containers::StringView> a = Path::splitExtension("file.txt");
        CORRADE_COMPARE(a.first().flags(), Containers::StringViewFlags{});
        CORRADE_COMPARE(a.second().flags(), Containers::StringViewFlag::NullTerminated);
    }
}

void PathTest::join() {
    /* Empty path */
    CORRADE_COMPARE(Path::join("", "/foo.txt"), "/foo.txt");

    /* Empty all */
    CORRADE_COMPARE(Path::join("", ""), "");

    /* Absolute filename */
    CORRADE_COMPARE(Path::join("/foo/bar", "/file.txt"), "/file.txt");

    /* Trailing slash */
    CORRADE_COMPARE(Path::join("/foo/bar/", "file.txt"), "/foo/bar/file.txt");

    /* Common case */
    CORRADE_COMPARE(Path::join("/foo/bar", "file.txt"), "/foo/bar/file.txt");
}

#ifdef CORRADE_TARGET_WINDOWS
void PathTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Path::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void PathTest::joinMultiple() {
    CORRADE_COMPARE(Path::join({"foo", "bar", "file.txt"}), "foo/bar/file.txt");
}

void PathTest::joinMultipleAbsolute() {
    CORRADE_COMPARE(Path::join({"foo", "/bar", "file.txt"}), "/bar/file.txt");
}

void PathTest::joinMultipleOneEmpty() {
    CORRADE_COMPARE(Path::join({"foo", "", "file.txt"}), "foo/file.txt");
}

void PathTest::joinMultipleJustOne() {
    CORRADE_COMPARE(Path::join({"file.txt"}), "file.txt");
}

void PathTest::joinMultipleNone() {
    CORRADE_COMPARE(Path::join({}), "");
}

void PathTest::exists() {
    /* File */
    CORRADE_VERIFY(Path::exists(Path::join(_testDir, "file")));

    /* Directory */
    CORRADE_VERIFY(Path::exists(_testDir));

    /* Nonexistent file */
    CORRADE_VERIFY(!Path::exists(Path::join(_testDir, "nonexistentFile")));

    /* Current directory, empty */
    CORRADE_VERIFY(Path::exists("."));
    CORRADE_VERIFY(!Path::exists(""));
}

void PathTest::existsNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is accessible under Emscripten.");
    #else
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /* Assuming there's no real possibility to run as root on Apple so this
       checks only other Unix systems */
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists, PID 1 is the "root" process and should always
       exist -- thus exists() reporting true, but any attempts to read it
       should fail. */
    CORRADE_VERIFY(Path::exists("/proc/self/mem"));
    CORRADE_VERIFY(Path::exists("/proc/1"));
    CORRADE_VERIFY(Path::exists("/proc/1/mem"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Path::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Path::size("/proc/1/mem"));
    #else
    /** @todo find something that actually exists, to test the same case as on
        Unix. No idea what in C:/Program Files/WindowsApps could be guaranteed
        to exist. */
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void PathTest::existsNonNullTerminated() {
    CORRADE_VERIFY(Path::exists(Path::join(_testDir, "fileX").exceptSuffix(1)));
}

void PathTest::existsUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_VERIFY(Path::exists(Path::join(_testDirUtf8, "hýždě")));
}

void PathTest::isDirectory() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDir, "dir")));
    }

    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "file")));

    /* Nonexistent file */
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "nonexistentFile")));
}

void PathTest::isDirectorySymlink() {
    CORRADE_VERIFY(Path::exists(Path::join(_testDirSymlink, "file-symlink")));
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDirSymlink, "file-symlink")));

    CORRADE_VERIFY(Path::exists(Path::join(_testDirSymlink, "dir-symlink")));
    {
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* See PathTest::sizeSymlink() for details */
        CORRADE_EXPECT_FAIL_IF(Path::size(Path::join(_testDirSymlink, "file-symlink")) != 11,
            "Symlinks not preserved in the source tree, can't test.");
        #endif
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDirSymlink, "dir-symlink")));
    }
}

void PathTest::isDirectoryNoPermission() {
    /* Similar to existsNoPermission(), but with isDirectory() being tested */

    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is accessible under Emscripten.");
    #else
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /* Assuming there's no real possibility to run as root on Apple so this
       checks only other Unix systems */
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");

    /* /proc/self/fd exists and is a directory, PID 1 is the "root" process and
       should always exist, but we shouldn't be able to peek into it */
    CORRADE_VERIFY(Path::isDirectory("/proc/self/fd"));
    CORRADE_VERIFY(Path::isDirectory("/proc/1"));
    CORRADE_VERIFY(Path::isDirectory("/proc/1/fd"));
    /* Just to be sure we're not giving back bullshit -- a random file in the
       same inaccessible directory should fail, opening that inacessible file
       should fail */
    CORRADE_VERIFY(!Path::exists("/proc/1/nonexistent"));
    CORRADE_VERIFY(!Path::size("/proc/1/fd"));
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* This file doesn't exist, but the whole contents of the WindowsApps
       directory is accessible only by the TrustedInstaller system user so this
       should fail */
    CORRADE_VERIFY(Path::isDirectory("C:/Program Files/WindowsApps"));
    CORRADE_VERIFY(!Path::isDirectory("C:/Program Files/WindowsApps/someDir"));
    #else
    CORRADE_SKIP("Not sure how to test this.");
    #endif
    #endif
}

void PathTest::isDirectoryNonNullTerminated() {
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDir, "dirX").exceptSuffix(1)));
    }
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDir, "fileX").exceptSuffix(1)));
}

void PathTest::isDirectoryUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "iOS (in a simulator) has no idea about file types.");
        #endif
        CORRADE_VERIFY(Path::isDirectory(Path::join(_testDirUtf8, "šňůra")));
    }
    CORRADE_VERIFY(!Path::isDirectory(Path::join(_testDirUtf8, "hýždě")));
}

void PathTest::make() {
    /* Existing */
    CORRADE_VERIFY(Path::exists(_testDir));
    /* Well... gotta make the test dir first to avoid failures later */
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Leaf */
    Containers::String leaf = Path::join(_writeTestDir, "leaf");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));

    /* Path */
    Containers::String path = Path::join(_writeTestDir, "path/to/new/dir");
    if(Path::exists(path))
        CORRADE_VERIFY(Path::remove(path));
    if(Path::exists(Path::join(_writeTestDir, "path/to/new")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path/to/new")));
    if(Path::exists(Path::join(_writeTestDir, "path/to")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path/to")));
    if(Path::exists(Path::join(_writeTestDir, "path")))
        CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "path")));

    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));

    /* Empty should be just a no-op without checking anything. Not like
       in Python, where `os.makedirs('', exist_ok=True)` stupidly fails with
        FileNotFoundError: [Errno 2] No such file or directory: '' */
    CORRADE_VERIFY(Path::make(""));
}

void PathTest::makeExistsAsAFile() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(!Path::isDirectory(file));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::make(file));
    /* It should fail also if some parent part of the path is a file */
    CORRADE_VERIFY(!Path::make(Path::join(file, "sub/dir")));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten says "Permission denied" instead of "Not a directory", for
       some reason */
    CORRADE_COMPARE_AS(out, format(
        "Utility::Path::make(): {0} exists but is not a directory\n"
        "Utility::Path::make(): can't create {0}/sub: error 2 (",
        file), TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code */
    CORRADE_COMPARE_AS(out, format(
        "Utility::Path::make(): {0} exists but is not a directory\n"
        "Utility::Path::make(): can't create {0}/sub: error 3 (",
        file), TestSuite::Compare::StringHasPrefix);
    #else
    /* Usual Unix */
    CORRADE_COMPARE_AS(out, format(
        "Utility::Path::make(): {0} exists but is not a directory\n"
        "Utility::Path::make(): can't create {0}/sub: error 20 (",
        file), TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::makeExistsAsADirectorySymlink() {
    Containers::String dirSymlink = Path::join(_testDirSymlink, "dir-symlink");
    Containers::String fileSymlink = Path::join(_testDirSymlink, "file-symlink");
    CORRADE_VERIFY(Path::exists(dirSymlink));
    CORRADE_VERIFY(Path::exists(fileSymlink));

    /* If symlinks are preserved in the source tree, the file should have size
       of the actual contents, not being empty or having some textual path in
       it */
    if(Path::size(fileSymlink) != 11)
        CORRADE_SKIP("Symlinks not preserved in the source tree, can't test.");

    /* If symlinks are preserved, this should succeed always, on all
       platforms */
    CORRADE_VERIFY(Path::make(dirSymlink));

    /* This will be a false positive on Windows */
    {
        #ifdef CORRADE_TARGET_WINDOWS
        CORRADE_EXPECT_FAIL("Path::make() returns true for symlinks always, even if they're not actually directories");
        #endif
        Containers::String out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::make(fileSymlink));
        CORRADE_COMPARE(out, format("Utility::Path::make(): {} exists but is not a directory\n", fileSymlink));
    }
}

void PathTest::makeDotDotDot() {
    /* Creating current directory should be a no-op because it exists */
    CORRADE_VERIFY(Path::exists("."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\".\") but fails instead.");
        #endif
        CORRADE_VERIFY(Path::make("."));
    }

    /* Parent as well */
    CORRADE_VERIFY(Path::exists(".."));
    {
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_EXPECT_FAIL("Emscripten doesn't return EEXIST on mdkir(\"..\") but fails instead.");
        #endif
        CORRADE_VERIFY(Path::make(".."));
    }
}

void PathTest::makeNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView prefix = "/var/root";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView prefix = "/data/local";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView prefix = "/root";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView prefix = "C:/Program Files/WindowsApps";
    #else
    Containers::StringView prefix;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::make(Path::join(prefix, "nope/never")));
    #ifndef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::make(): can't create {}/nope: error 13 (", prefix),
        TestSuite::Compare::StringHasPrefix);
    #else
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::make(): can't create {}/nope: error 5 (", prefix),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #endif
}

void PathTest::makeNonNullTerminated() {
    Containers::String leaf = Path::join(_writeTestDir, "leaf");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(Path::join(_writeTestDir, "leafX").exceptSuffix(1)));
    CORRADE_VERIFY(Path::exists(leaf));
}

void PathTest::makeUtf8() {
    Containers::String leaf = Path::join(_writeTestDir, "šňůra");
    if(Path::exists(leaf))
        CORRADE_VERIFY(Path::remove(leaf));
    CORRADE_VERIFY(Path::make(leaf));
    CORRADE_VERIFY(Path::exists(leaf));

    /* Verify it behaves correctly also if the directory exists already, in
       particular that the subsequent isDirectory()-like check in the internals
       properly handles UTF-8 as well */
    CORRADE_VERIFY(Path::make(leaf));
}

void PathTest::removeFile() {
    Containers::String file = Path::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectory() {
    Containers::String directory = Path::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(directory));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::removeFileNonexistent() {
    CORRADE_VERIFY(!Path::exists("nonexistent"));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::remove("nonexistent"));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::remove(): can't remove nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::remove(): can't remove nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::removeDirectoryNonEmpty() {
    Containers::String directory = Path::join(_writeTestDir, "nonEmptyDirectory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::write(Path::join(directory, "file.txt"), "a"_s));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::remove(directory));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "Directory not empty",
       also there's a dedicated code path (and message) for directories */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::remove(): can't remove directory {}: error 55 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Windows also have a dedicated code path and message for dirs,
       "The directory is not empty." */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::remove(): can't remove directory {}: error 145 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_APPLE)
    /* Otherwise there's common handling for files and dirs, however Apple has
       to be special and also have a different error code for the same thing */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::remove(): can't remove {}: error 66 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::remove(): can't remove {}: error 39 (", directory),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::removeFileNonNullTerminated() {
    Containers::String file = Path::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "file.txtX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectoryNonNullTerminated() {
    Containers::String directory = Path::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(Path::join(_writeTestDir, "directoryX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::removeFileUtf8() {
    Containers::String file = Path::join(_writeTestDir, "hýždě.txt");
    CORRADE_VERIFY(Path::make(_writeTestDir));
    CORRADE_VERIFY(Path::write(file, "a"_s));
    CORRADE_VERIFY(Path::exists(file));
    CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(!Path::exists(file));
}

void PathTest::removeDirectoryUtf8() {
    Containers::String directory = Path::join(_writeTestDir, "složka");
    CORRADE_VERIFY(Path::make(directory));
    CORRADE_VERIFY(Path::exists(directory));
    CORRADE_VERIFY(Path::remove(directory));
    CORRADE_VERIFY(!Path::exists(directory));
}

void PathTest::moveFile() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "newFile.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::move(oldFile, newFile));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::moveDirectory() {
    /* Old directory, create if not exists */
    Containers::String oldDirectory = Path::join(_writeTestDir, "oldDirectory");
    if(!Path::exists(oldDirectory))
        CORRADE_VERIFY(Path::make(oldDirectory));

    /* New directory, remove if exists */
    Containers::String newDirectory = Path::join(_writeTestDir, "newDirectory");
    if(Path::exists(newDirectory))
        CORRADE_VERIFY(Path::remove(newDirectory));

    CORRADE_VERIFY(Path::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Path::exists(oldDirectory));
    CORRADE_VERIFY(Path::exists(newDirectory));
}

void PathTest::moveSourceNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String to = Path::join(_writeTestDir, "empty");

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::move("nonexistent", to));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::move(): can't move nonexistent to {}: error 44 (", to),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::move(): can't move nonexistent to {}: error 2 (", to),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::moveDestinationNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    Containers::String from = Path::join(_testDir, "dir/dummy");
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView to = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView to = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView to = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView to = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView to;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::move(from, to));
    #ifndef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::move(): can't move {} to {}: error 13 (", from, to),
        TestSuite::Compare::StringHasPrefix);
    #else
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access is denied.") */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::move(): can't move {} to {}: error 5 (", from, to),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #endif
}

void PathTest::moveDestinationExists() {
    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, should get overwritten */
    Containers::String newFile = Path::join(_writeTestDir, "newFile.txt");
    CORRADE_VERIFY(Path::write(newFile, "b"_s));

    CORRADE_VERIFY(Path::move(oldFile, newFile));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_COMPARE_AS(newFile, "a", TestSuite::Compare::FileToString);
}

void PathTest::moveNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "newFile.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::move(
        Path::join(_writeTestDir, "oldFile.txtX").exceptSuffix(1),
        Path::join(_writeTestDir, "newFile.txtX").exceptSuffix(1)));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::moveUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    /* Old file */
    Containers::String oldFile = Path::join(_writeTestDir, "starý hýždě.txt");
    CORRADE_VERIFY(Path::write(oldFile, "a"_s));

    /* New file, remove if exists */
    Containers::String newFile = Path::join(_writeTestDir, "nový hýždě.txt");
    if(Path::exists(newFile))
        CORRADE_VERIFY(Path::remove(newFile));

    CORRADE_VERIFY(Path::exists(oldFile));
    CORRADE_VERIFY(!Path::exists(newFile));
    CORRADE_VERIFY(Path::move(oldFile, newFile));
    CORRADE_VERIFY(!Path::exists(oldFile));
    CORRADE_VERIFY(Path::exists(newFile));
}

void PathTest::currentDirectory() {
    Containers::Optional<Containers::String> current = Path::currentDirectory();
    CORRADE_VERIFY(current);
    CORRADE_VERIFY(*current);
    CORRADE_INFO("Current directory found as:" << current);

    /* Ensure the test is not accidentally false positive due to stale files */
    if(Path::exists("currentPathTestDir.mark"))
        CORRADE_VERIFY(Path::remove("currentPathTestDir.mark"));

    /* Create a file on a relative path. If current directory is correctly
       queried, it should exist there */
    CORRADE_VERIFY(Path::write("currentPathTestDir.mark",
        "hi, i'm testing Utility::Path::current()"_s));
    CORRADE_VERIFY(Path::exists(Path::join(*current, "currentPathTestDir.mark")));

    /* Clean up after ourselves */
    CORRADE_VERIFY(Path::remove("currentPathTestDir.mark"));

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*current, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*current->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*current, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::currentDirectoryNonexistent() {
    #ifdef CORRADE_TARGET_UNIX
    Containers::Optional<Containers::String> currentDirectory = Path::currentDirectory();
    CORRADE_VERIFY(currentDirectory);
    CORRADE_VERIFY(*currentDirectory);

    /* On Android the write dir is relative, so append it to current */
    Containers::String newCurrent = Path::join({*currentDirectory, _writeTestDir, "cwd"});
    CORRADE_VERIFY(Path::make(newCurrent));

    {
        /** @todo remove (and <unistd.h>) once Path::setCurrent() exists */
        Containers::ScopeGuard resetCurrent{&*currentDirectory, [](Containers::String* current) {
            CORRADE_COMPARE(chdir(current->data()), 0);
        }};
        CORRADE_COMPARE(chdir(newCurrent.data()), 0);
        CORRADE_COMPARE(Path::currentDirectory(), newCurrent);

        CORRADE_VERIFY(Path::exists("."));
        CORRADE_VERIFY(Path::remove(newCurrent));

        /* Interestingly, this doesn't fail */
        CORRADE_VERIFY(Path::exists("."));

        Containers::String out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::currentDirectory());
        CORRADE_COMPARE_AS(out,
            "Utility::Path::currentDirectory(): error 2 (",
            TestSuite::Compare::StringHasPrefix);
    }

    /* Verify that we're back to the original directory so other tests relying
       on it keep working. Should be also done in case anything above fails. */
    CORRADE_COMPARE(Path::currentDirectory(), currentDirectory);
    #else
    CORRADE_SKIP("Known to fail only on UNIX, not sure how to test elsewhere.");
    #endif
}

void PathTest::currentDirectoryUtf8() {
    /** @todo test once setCurrent() exists -- otherwise we'd have to implement
        UTF-16 conversion for Windows by hand here to pass to
        SetCurrentDirectory() and ... ew */
    CORRADE_SKIP("Not sure how to test this.");
}

#ifndef CORRADE_BUILD_STATIC
void PathTest::libraryLocation() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::String> libraryLocation = Path::libraryLocation(&Path::remove);
    CORRADE_VERIFY(libraryLocation);
    CORRADE_VERIFY(*libraryLocation);
    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    {
        CORRADE_VERIFY(libraryLocation != Path::executableLocation());

        /* There should be a TestSuite library next to this one */
        Containers::StringView testSuiteLibraryName =
            #ifdef CORRADE_TARGET_WINDOWS
            #ifdef __MINGW32__
            "lib"
            #endif
            #ifdef CORRADE_IS_DEBUG_BUILD
            "CorradeTestSuite-d.dll"
            #else
            "CorradeTestSuite.dll"
            #endif
            #elif defined(CORRADE_TARGET_APPLE)
            #ifdef CORRADE_IS_DEBUG_BUILD
            "libCorradeTestSuite-d.dylib"
            #else
            "libCorradeTestSuite.dylib"
            #endif
            #else
            #ifdef CORRADE_IS_DEBUG_BUILD
            "libCorradeTestSuite-d.so"
            #else
            "libCorradeTestSuite.so"
            #endif
            #endif
            ;
        CORRADE_VERIFY(Path::exists(Path::join(Path::path(*libraryLocation), testSuiteLibraryName)));
    }

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*libraryLocation, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*libraryLocation->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*libraryLocation, "\\", TestSuite::Compare::StringNotContains);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#else
void PathTest::libraryLocationStatic() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::String> libraryLocation = Path::libraryLocation(&Path::remove);
    CORRADE_VERIFY(libraryLocation);
    CORRADE_VERIFY(*libraryLocation);
    CORRADE_INFO("Corrade::Utility library location found as:" << libraryLocation);

    /* No libraries in a static build, so this will print the final executable
       instead */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*libraryLocation,
        "UtilityPathTest.exe",
        TestSuite::Compare::StringHasSuffix);
    #else
    CORRADE_COMPARE_AS(*libraryLocation,
        "UtilityPathTest",
        TestSuite::Compare::StringHasSuffix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}
#endif

void PathTest::libraryLocationNull() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::libraryLocation(nullptr));
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out,
        "Utility::Path::libraryLocation(): can't get library location: error 87 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out, "Utility::Path::libraryLocation(): can't get library location\n");
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::libraryLocationInvalid() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::libraryLocation(reinterpret_cast<const void*>(0xbadcafe)));
    #ifdef CORRADE_TARGET_WINDOWS
    /* "The specified module could not be found." */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::libraryLocation(): can't get library location: error 126 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out, "Utility::Path::libraryLocation(): can't get library location\n");
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::libraryLocationUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_SKIP("Not sure how to test this.");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::executableLocation() {
    Containers::Optional<Containers::String> executableLocation = Path::executableLocation();
    CORRADE_VERIFY(executableLocation);
    CORRADE_VERIFY(*executableLocation);
    CORRADE_INFO("Executable location found as:" << executableLocation);

    /* On sandboxed macOS and iOS verify that the directory contains Info.plist
       file */
    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()) {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif

        CORRADE_VERIFY(Path::exists(Path::join(Path::path(*executableLocation), "Info.plist")));
    } else
    #endif

    /* On Emscripten we should have access to the bundled files */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_VERIFY(Path::exists(Path::join(Path::path(*executableLocation), "PathTestFiles")));

    /* On Android we can't be sure about anything, so just test that the
       executable exists and it has access to the bundled files */
    #elif defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Path::exists(*executableLocation));
    CORRADE_COMPARE_AS(*executableLocation,
        "UtilityPathTest",
        TestSuite::Compare::StringContains);
    CORRADE_VERIFY(Path::exists(Path::join(Path::path(*executableLocation), "PathTestFiles")));

    /* Otherwise it should contain other executables and libraries as we put
       all together */
    #else
    {
        #ifndef CORRADE_TARGET_WINDOWS
        CORRADE_VERIFY(Path::exists(Path::join(Path::path(*executableLocation), "corrade-rc")));
        #else
        CORRADE_VERIFY(Path::exists(Path::join(Path::path(*executableLocation), "corrade-rc.exe")));
        #endif
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*executableLocation, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*executableLocation->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*executableLocation, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::executableLocationInvalid() {
    /** @todo on Linux it fails if /proc/self/exe is inacessible, which might
        happen with some overly-restrictive VMs, how to test that? */
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::executableLocationUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::homeDirectory() {
    Containers::Optional<Containers::String> homeDirectory = Path::homeDirectory();

    #ifdef CORRADE_TARGET_WINDOWS
    /* Inverse of the check in homeDirectoryInvalid() below, see there for more
       information */
    if(!std::getenv("HOMEPATH") || !Path::exists(std::getenv("HOMEPATH"))) {
        CORRADE_VERIFY(!homeDirectory);
        CORRADE_SKIP("%HOMEPATH% doesn't exist, can't test.");
    }
    #endif

    CORRADE_VERIFY(homeDirectory);
    CORRADE_VERIFY(*homeDirectory);
    CORRADE_INFO("Home directory found as:" << homeDirectory);

    /* On macOS and iOS verify that the home dir contains `Library` directory */
    #ifdef CORRADE_TARGET_APPLE
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, "Library")));

    /* On other Unixes (except Android, which is shit) verify that the home dir
       contains `.local` directory or is /root. Ugly and hacky, but it's the
       best I came up with. Can't test for e.g. `/home/` substring, as that can
       be overridden. */
    #elif defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_VERIFY(Path::exists(*homeDirectory));
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, ".local")) || homeDirectory == "/root");

    /* On Emscripten verify that the directory exists (it's empty by default) */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_VERIFY(Path::exists(*homeDirectory));

    /* On Windows verify that the home dir contains `desktop.ini` file. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_VERIFY(Path::exists(Path::join(*homeDirectory, "desktop.ini")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(*homeDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*homeDirectory, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*homeDirectory->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*homeDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::homeDirectoryInvalid() {
    #ifdef CORRADE_TARGET_WINDOWS
    /* The query fails for system accounts, and system accounts apparently have
       no access to environment, so checking if %HOMEPATH% is missing:
       https://serverfault.com/questions/292040/win-service-running-under-localservice-account-cannot-access-environment-variabl

       Additionally this can be reproduced by temporarily removing / renaming
       the user directory, such as with `ren C:\Users\appveyor\Documents Doc`.
       Then the environment variable is set, but the query fails. */
    /** @todo Note that, however, %HOMEPATH% returns just `C:/Users/appveyor`,
        so this check *isn't* enough. And there's no alternative way to query
        the location other than what homeDirectory() itself does, so... */
    if(std::getenv("HOMEPATH") && Path::exists(std::getenv("HOMEPATH")))
        CORRADE_SKIP("%HOMEPATH% exists, can't test.");

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::homeDirectory());
    /** @todo It produces 1008 (An attempt was made to reference a token that
        does not exist.) for the second failure, but the first failure in
        homeDirectory() above is error 2 (The system cannot find the file
        specified.). Am I triggering some other error in between, or does the
        first lookup trigger some OS-internal update that makes it fail earlier
        or differently the second time? */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::homeDirectory(): can't retrieve FOLDERID_Documents: error 1008 (",
        TestSuite::Compare::StringHasPrefix);

    #else
    /* On Unix could be tested by temporarily removing $HOME, but ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
    #endif
}

void PathTest::homeDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::configurationDirectory() {
    Containers::Optional<Containers::String> configurationDirectory = Path::configurationDirectory("Corrade");

    #ifdef CORRADE_TARGET_WINDOWS
    /* Inverse of the check in configurationDirectoryInvalid() below, see there
       for more information */
    if(!std::getenv("APPDATA") || !Path::exists(std::getenv("APPDATA"))) {
        CORRADE_VERIFY(!configurationDirectory);
        CORRADE_SKIP("%APPDATA% doesn't exist, can't test.");
    }
    #endif

    CORRADE_VERIFY(configurationDirectory);
    CORRADE_VERIFY(*configurationDirectory);
    CORRADE_INFO("Configuration dir found as:" << configurationDirectory);

    #ifdef CORRADE_TARGET_APPLE
    CORRADE_COMPARE_AS(*configurationDirectory,
        "Corrade",
        TestSuite::Compare::StringHasSuffix);
    if(System::isSandboxed())
        CORRADE_VERIFY(Path::exists(Path::join(Path::path(Path::path(*configurationDirectory)), "Caches")));
    else
        /* App Store is not present on *some* Travis VMs since 2018-08-05.
           CrashReporter is. */
        CORRADE_VERIFY(
            Path::exists(Path::join(Path::path(*configurationDirectory), "App Store")) ||
            Path::exists(Path::join(Path::path(*configurationDirectory), "CrashReporter")));

    /* On Linux verify that the parent dir contains `autostart` directory,
       something from GTK or something from Qt. Ugly and hacky, but it's the
       best I could come up with. Can't test for e.g. `/home/` substring, as
       that can be overridden. */
    #elif defined(__linux__) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE_AS(*configurationDirectory,
        "corrade",
        TestSuite::Compare::StringHasSuffix);
    CORRADE_VERIFY(
        Path::exists(Path::join(Path::path(*configurationDirectory), "autostart")) ||
        Path::exists(Path::join(Path::path(*configurationDirectory), "dconf")) ||
        Path::exists(Path::join(Path::path(*configurationDirectory), "Trolltech.conf")));

    /* Emscripten -- just compare to hardcoded value */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(*configurationDirectory, "/home/web_user/.config/corrade");

    /* On Windows verify that the parent dir contains `Microsoft` subdirectory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overridden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_COMPARE_AS(*configurationDirectory,
        "Corrade",
        TestSuite::Compare::StringHasSuffix);
    CORRADE_VERIFY(Path::exists(Path::join(Path::path(*configurationDirectory), "Microsoft")));

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(*configurationDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*configurationDirectory, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*configurationDirectory->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*configurationDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif
}

void PathTest::configurationDirectoryInvalid() {
    #ifdef CORRADE_TARGET_WINDOWS
    /* The query fails for system accounts, and system accounts apparently have
       no access to environment, so checking if %HOMEPATH% is missing:
       https://serverfault.com/questions/292040/win-service-running-under-localservice-account-cannot-access-environment-variabl

       Additionally, similarly to homeDirectoryInvalid(), this can be
       *theoretically* reproduced by temporarily removing / renaming the dir,
       such as with `ren C:/Users/appveyor/AppData/Roaming Roam` (although in
       my test that failed with "Access denied"). Then the environment variable
       is set, but the query fails. */
    if(std::getenv("APPDATA") && Path::exists(std::getenv("APPDATA")))
        CORRADE_SKIP("%APPDATA% exists, can't test.");

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::configurationDirectory("Corrade"));
    /** @todo Assuming the same repro as with homeDirectoryInvalid(), i.e., a
        rename, it should also produce an error 1008 for the second failure,
        but the first failure in configurationDirectory() above would be error
        2. Like with homeDirectory() failing, figure out what's going on. */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::configurationDirectory(): can't retrieve FOLDERID_RoamingAppData: error 1008 (",
        TestSuite::Compare::StringHasPrefix);

    #else
    /* On Unix could be tested by temporarily removing $XDG_CONFIG_HOME and
       $HOME, but ... ahem */
    CORRADE_SKIP("Not sure how to test this.");
    #endif
}

void PathTest::configurationDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::temporaryDirectory() {
    Containers::Optional<Containers::String> temporaryDirectory = Path::temporaryDirectory();
    CORRADE_VERIFY(temporaryDirectory);
    CORRADE_VERIFY(*temporaryDirectory);
    CORRADE_INFO("Temporary dir found as:" << temporaryDirectory);

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Path::exists(*temporaryDirectory));
    }
    CORRADE_COMPARE_AS(*temporaryDirectory,
        "tmp",
        TestSuite::Compare::StringContains);

    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    CORRADE_VERIFY(Path::exists(*temporaryDirectory));
    #ifndef __MINGW32__
    CORRADE_COMPARE_AS(*temporaryDirectory,
        "Temp",
        TestSuite::Compare::StringContains);
    #else
    /* MinGW shell maps temp to a different directory, e.g. C:/msys64/tmp, so
       check for both */
    /** @todo needs some StringContainsAny, taking a list */
    if(temporaryDirectory->contains("tmp"))
        CORRADE_COMPARE_AS(*temporaryDirectory,
            "tmp",
            TestSuite::Compare::StringContains);
    else
        CORRADE_COMPARE_AS(*temporaryDirectory,
            "Temp",
            TestSuite::Compare::StringContains);
    #endif

    /* No idea elsewhere */
    #else
    {
        CORRADE_EXPECT_FAIL("Not implemented yet.");
        CORRADE_COMPARE(temporaryDirectory, "(not implemented)");
    }
    #endif

    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*temporaryDirectory, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*temporaryDirectory->end(), '\0');

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(*temporaryDirectory, "\\", TestSuite::Compare::StringNotContains);
    #endif

    /* Verify that it's possible to write stuff there */
    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_VERIFY(Path::write(Path::join(*temporaryDirectory, "a"), "hello"_s));
        CORRADE_VERIFY(Path::remove(Path::join(*temporaryDirectory, "a")));
    }
}

void PathTest::temporaryDirectoryInvalid() {
    CORRADE_SKIP("Not known to fail on any known system.");
}

void PathTest::temporaryDirectoryUtf8() {
    CORRADE_SKIP("Not sure how to test this.");
}

void PathTest::list() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listIterateRangeFor() {
    /* Verify that the directory can be listed to make the unconditional
       dereference in the for() below unlikely to assert */
    CORRADE_VERIFY(Path::list(_testDir));

    /* It should not happen that the original Optional somehow gets out of
       scope before we get to iterating the array contained in it. This is a
       yet-unsolved problem in C++ with std::optional and other STL containers:
       https://josuttis.com/download/std/D2012R0_fix_rangebasedfor_201029.pdf

       However, in this case, and unlike with std::make, the operator*()
       returns a T instead of T&&, and the reference lifetime extension takes
       care of the rest. See also OptionalTest::accessRvalueLifetimeExtension(). */
    Containers::Array<Containers::String> out;
    for(Containers::String& a: *Path::list(_testDir))
        arrayAppend(out, a);

    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif
    CORRADE_COMPARE_AS(out, Containers::array<Containers::String>({
        ".", "..", "dir", "file"
    }), TestSuite::Compare::SortedContainer);
}

void PathTest::listEmptyDirectory() {
    /* Create an empty directory. To be sure it's empty, recreate it. */
    Containers::String emptyDir = Path::join(_writeTestDir, "EmptyDir");
    if(Path::exists(emptyDir))
        CORRADE_VERIFY(Path::remove(emptyDir));
    CORRADE_VERIFY(Path::make(emptyDir));

    /* It shouldn't fail if there's nothing inside */
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(emptyDir);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", ".."
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDirectories() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDirectoriesSymlinks() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* See PathTest::sizeSymlink() for details */
        CORRADE_EXPECT_FAIL_IF(Path::size(Path::join(_testDirSymlink, "file-symlink")) != 11,
            "Symlinks not preserved in the source tree, can't test.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file", "file-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipFiles() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipFiles);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipFilesSymlinks() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipFiles);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        #if !defined(CORRADE_TARGET_UNIX) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* See PathTest::sizeSymlink() for details */
        CORRADE_EXPECT_FAIL_IF(Path::size(Path::join(_testDirSymlink, "file-symlink")) != 11,
            "Symlinks not preserved in the source tree, can't test.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "dir-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipSpecial() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipSpecial);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipSpecialSymlink() {
    /* Symlinks should not be treated as special files, so they're shown */

    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDirSymlink, Path::ListFlag::SkipSpecial);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "dir-symlink", "file", "file-symlink"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipDotAndDotDot() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipDotAndDotDot);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSkipEverything() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SkipFiles|Path::ListFlag::SkipDirectories);
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listSort() {
    {
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortAscending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::Container);
    } {
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortDescending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            "file", "dir", "..", "."
        }), TestSuite::Compare::Container);
    } {
        /* Ascending and descending together will pick ascending */
        Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir, Path::ListFlag::SortAscending|Path::ListFlag::SortDescending);
        CORRADE_VERIFY(list);

        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::Container);
    }
}

void PathTest::listNonexistent() {
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::list("nonexistent"));
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows has its own code path and thus different errors */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::list(): can't list nonexistent: error 3 (",
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::list(): can't list nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::list(): can't list nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::listNonNullTerminated() {
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list((_testDir + "X").exceptSuffix(1));
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listTrailingSlash() {
    /* Should have the same result as without */
    Containers::Optional<Containers::Array<Containers::String>> list = Path::list(_testDir + "/");
    CORRADE_VERIFY(list);

    {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif
        CORRADE_COMPARE_AS(*list, Containers::array<Containers::String>({
            ".", "..", "dir", "file"
        }), TestSuite::Compare::SortedContainer);
    }
}

void PathTest::listUtf8Result() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    const Containers::String list[]{".", "..", "hýždě", "šňůra"};

    Containers::Optional<Containers::Array<Containers::String>> actual = Path::list(_testDirUtf8, Path::ListFlag::SortAscending);
    CORRADE_VERIFY(actual);

    #ifdef CORRADE_TARGET_APPLE
    /* Apple HFS+ stores filenames in a decomposed normalized form to avoid
       e.g. `e` + `ˇ` and `ě` being treated differently. That makes sense. I
       wonder why neither Linux nor Windows do this. */
    const Containers::String listDecomposedUnicode[]{".", "..", "hýždě", "šňůra"};
    CORRADE_COMPARE_AS(list[3],
        listDecomposedUnicode[3],
        TestSuite::Compare::NotEqual);
    #endif

    #ifdef CORRADE_TARGET_APPLE
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    /* However, Apple systems still can use filesystems other than HFS+, so
       be prepared that it can compare to either */
    if((*actual)[3] == listDecomposedUnicode[3]) {
        CORRADE_COMPARE_AS(*actual,
            Containers::arrayView(listDecomposedUnicode),
            TestSuite::Compare::Container);
    } else
    #endif
    {
        CORRADE_COMPARE_AS(*actual,
            Containers::arrayView(list),
            TestSuite::Compare::Container);
    }
}

void PathTest::listUtf8Path() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    const Containers::String list[]{".", "..", "dummy", "klíče"};

    Containers::Optional<Containers::Array<Containers::String>> actual = Path::list(Path::join(_testDirUtf8, "šňůra"), Path::ListFlag::SortAscending);
    CORRADE_VERIFY(actual);

    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif
    CORRADE_COMPARE_AS(*actual,
        Containers::arrayView(list),
        TestSuite::Compare::Container);
}

/* Checks if we are reading it as binary (CR+LF is not converted to LF),
   nothing after \0 gets lost, and invalid UTF-8 chars (over 0x80) also cause
   no issues */
constexpr const char Data[]{'\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

void PathTest::size() {
    /* Existing file, containing the above data */
    CORRADE_COMPARE(Path::size(Path::join(_testDir, "file")),
        Containers::arraySize(Data));
}

void PathTest::sizeEmpty() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));

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
    CORRADE_COMPARE(Path::size(empty), 0);
}

void PathTest::sizeNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    Containers::String out;
    Error redirectError{&out};
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    CORRADE_VERIFY(!Path::size("/proc/zoneinfo"));
    CORRADE_COMPARE(out, "Utility::Path::size(): /proc/zoneinfo is not seekable\n");
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::sizeEarlyEof() {
    #ifdef __linux__
    Containers::StringView file = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    if(!Path::exists(file))
        CORRADE_SKIP(file << "doesn't exist, can't test");
    Containers::Optional<std::size_t> size = Path::size(file);
    Containers::Optional<Containers::Array<char>> data = Path::read(file);
    CORRADE_VERIFY(size);
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*size, data->size(), TestSuite::Compare::Greater);
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::sizeSymlink() {
    Containers::String fileSymlink = Path::join(_testDirSymlink, "file-symlink");
    CORRADE_VERIFY(Path::exists(fileSymlink));

    Containers::Optional<std::size_t> size = Path::size(fileSymlink);
    CORRADE_VERIFY(size);

    /* This is also used elsewhere as a CORRADE_EXPECT_FAIL_IF() condition, to
       expect a symlink treatment failure if the files are not symlinks in the
       first place */
    if(size != 11)
        CORRADE_SKIP("Symlinks not preserved in the source tree, can't test.");

    CORRADE_COMPARE(size, 11);
}

void PathTest::sizeDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_COMPARE(Path::size(_testDir), Containers::NullOpt);

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::size(): can't open {}: error 13 (", _testDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out, format("Utility::Path::size(): {} is a directory\n", _testDir));
    #endif
}

void PathTest::sizeNonexistent() {
    Containers::String out;
    Error redirectError{&out};
    CORRADE_COMPARE(Path::size("nonexistent"), Containers::NullOpt);
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::size(): can't open nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::size(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::sizeNonNullTerminated() {
    /* Existing file, containing the above data */
    CORRADE_COMPARE(Path::size(Path::join(_testDir, "fileX").exceptSuffix(1)),
        Containers::arraySize(Data));
}

void PathTest::sizeUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_COMPARE(Path::size(Path::join(_testDirUtf8, "hýždě")),
        Containers::arraySize(Data));
}

void PathTest::lastModification() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "a"_s));
    Containers::Optional<std::int64_t> time1 = Path::lastModification(file);
    CORRADE_VERIFY(time1);
    /* Unix time 1234567890 was in 2009. The reported timestamp should
       definitely be larger than that. */
    CORRADE_COMPARE_AS(*time1,
        1234567890ll*1000ll*1000ll*1000ll,
        TestSuite::Compare::Greater);

    /* So we don't write at the same nanosecond. Linux gives us 10-millisecond
       precision, HFS+ on macOS has second precision (even though the API has
       nanoseconds), on Windows the API itself has second granularity,
       Emscripten sets the nanosecond part of the API to zero.
        https://developer.apple.com/library/archive/technotes/tn/tn1150.html#HFSPlusDates
        https://github.com/kripken/emscripten/blob/52ff847187ee30fba48d611e64b5d10e2498fe0f/src/library_syscall.js#L66 */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::append(file, "b"_s));

    Containers::Optional<std::int64_t> time2 = Path::lastModification(file);
    CORRADE_VERIFY(time2);
    CORRADE_COMPARE_AS(*time2, *time1,
        TestSuite::Compare::Greater);
}

void PathTest::lastModificationNonexistent() {
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::lastModification("nonexistent"));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::lastModification(): can't stat nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::lastModification(): can't stat nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::lastModificationNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "a"_s));
    Containers::Optional<std::int64_t> time1 = Path::lastModification(Path::join(_writeTestDir, "file!!!").exceptSuffix(3));
    CORRADE_VERIFY(time1);
    /* Unix time 1234567890 was in 2009. The reported timestamp should
       definitely be larger than that. */
    CORRADE_COMPARE_AS(*time1,
        1234567890ll*1000ll*1000ll*1000ll,
        TestSuite::Compare::Greater);

    /* So we don't write at the same nanosecond. Linux gives us 10-millisecond
       precision, HFS+ on macOS has second precision (even though the API has
       nanoseconds), on Windows the API itself has second granularity,
       Emscripten sets the nanosecond part of the API to zero.
        https://developer.apple.com/library/archive/technotes/tn/tn1150.html#HFSPlusDates
        https://github.com/kripken/emscripten/blob/52ff847187ee30fba48d611e64b5d10e2498fe0f/src/library_syscall.js#L66 */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::append(file, "b"_s));

    Containers::Optional<std::int64_t> time2 = Path::lastModification(Path::join(_writeTestDir, "file!!!").exceptSuffix(3));
    CORRADE_VERIFY(time2);
    CORRADE_COMPARE_AS(*time2, *time1,
        TestSuite::Compare::Greater);
}

void PathTest::lastModificationUtf8() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "a"_s));
    Containers::Optional<std::int64_t> time1 = Path::lastModification(file);
    CORRADE_VERIFY(time1);
    /* Unix time 1234567890 was in 2009. The reported timestamp should
       definitely be larger than that. */
    CORRADE_COMPARE_AS(*time1,
        1234567890ll*1000ll*1000ll*1000ll,
        TestSuite::Compare::Greater);

    /* So we don't write at the same nanosecond. Linux gives us 10-millisecond
       precision, HFS+ on macOS has second precision (even though the API has
       nanoseconds), on Windows the API itself has second granularity,
       Emscripten sets the nanosecond part of the API to zero.
        https://developer.apple.com/library/archive/technotes/tn/tn1150.html#HFSPlusDates
        https://github.com/kripken/emscripten/blob/52ff847187ee30fba48d611e64b5d10e2498fe0f/src/library_syscall.js#L66 */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::append(file, "b"_s));

    Containers::Optional<std::int64_t> time2 = Path::lastModification(file);
    CORRADE_VERIFY(time2);
    CORRADE_COMPARE_AS(*time2, *time1,
        TestSuite::Compare::Greater);
}

void PathTest::read() {
    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDir, "file"));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::readString() {
    Containers::Optional<Containers::String> string = Path::readString(Path::join(_testDir, "file"));
    CORRADE_VERIFY(string);
    /* Data is a char array *without* a null terminator, so take the whole
       size */
    CORRADE_COMPARE(*string, (Containers::StringView{Data, Containers::arraySize(Data)}));
    /* There should be a null terminator at the end. With assertions enabled
       the String constructor checks for this on its own, but let's double
       check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
}

void PathTest::readEmpty() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));

    /* The optional is set, but the array is empty */
    Containers::Optional<Containers::Array<char>> data = Path::read(empty);
    CORRADE_VERIFY(data);

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
    CORRADE_VERIFY(data->isEmpty());
}

void PathTest::readEmptyString() {
    Containers::String empty = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(empty));

    /* The optional is set, but the string is empty */
    Containers::Optional<Containers::String> string = Path::readString(empty);
    CORRADE_VERIFY(string);

    {
        #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 20026 && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ < 30103
        /* Emscripten 2.0.26+ has a problem in the file embedder, where
           zero-size files are reported as having 3 bytes. The changelog
           between 2.0.25 and 2.0.26 doesn't mention anything related, the only
           related change I found was
            https://github.com/emscripten-core/emscripten/pull/14526
           going into 2.0.25 already, and I suspect it's something related to
           padding in base64 decode. This problem is gone in 3.1.3, where they
           replace the base64 file embedding with putting a binary directly to
           wasm in
            https://github.com/emscripten-core/emscripten/pull/16050
           Which then however breaks UTF-8 paths, see the CORRADE_SKIP()
           elsewhere.

           Also seems to happen only with Node.js 14 that's bundled with emsdk,
           not with external version 18. Node.js 15+ is only bundled with emsdk
           3.1.35+ which doesn't suffer from this 3-byte bug anymore. */
        CORRADE_EXPECT_FAIL_IF(nodeJsVersionLess(18),
            "Emscripten 2.0.26 to 3.1.3 with Node.js < 18 reports empty files as having 3 bytes.");
        #endif
        CORRADE_VERIFY(!*string);
    }
    /* There should be a null terminator at the end. With assertions enabled
       the String constructor checks for this on its own, but let's double
       check here as well. */
    CORRADE_COMPARE(*string->end(), '\0');
}

void PathTest::readNonSeekable() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /** @todo Test more thoroughly than this */
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    Containers::Optional<Containers::Array<char>> data = Path::read("/proc/zoneinfo");
    CORRADE_VERIFY(data);
    CORRADE_VERIFY(!data->isEmpty());
    /* The array is growable */
    CORRADE_VERIFY(data->deleter());
    /* But it shouldn't contain null bytes anywhere (which would point to
       issues with growing the array) */
    CORRADE_COMPARE_AS(Containers::StringView{*data}, "\0"_s, TestSuite::Compare::StringNotContains);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::readNonSeekableString() {
    /* macOS or BSD doesn't have /proc */
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) && \
        !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && \
        !defined(__NetBSD__) && !defined(__DragonFly__)
    /** @todo Test more thoroughly than this */
    /* /proc/loadavg works on Android Emulator but not on a real device;
       /proc/zoneinfo works everywhere */
    Containers::Optional<Containers::String> string = Path::readString("/proc/zoneinfo");
    CORRADE_VERIFY(string);
    CORRADE_VERIFY(*string);
    /* The array is growable and the deleter should be preserved in the string
       as well */
    CORRADE_VERIFY(string->deleter());
    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there, not overwritten by some garbage. */
    CORRADE_COMPARE_AS(*string, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*string->end(), '\0');
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::readEarlyEof() {
    #ifdef __linux__
    if(!Path::exists("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"))
        CORRADE_SKIP("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor doesn't exist, can't test");
    Containers::Optional<Containers::Array<char>> data = Path::read("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CORRADE_VERIFY(data);
    CORRADE_VERIFY(!data->isEmpty());
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::readEarlyEofString() {
    #ifdef __linux__
    if(!Path::exists("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"))
        CORRADE_SKIP("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor doesn't exist, can't test");
    Containers::Optional<Containers::String> string = Path::readString("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CORRADE_VERIFY(string);
    CORRADE_VERIFY(*string);
    /* It shouldn't contain null bytes anywhere, especially not at the end. But
       the final \0 should be there and not overwritten by some garbage in this
       case as well, not just when the array is growable or when we know the
       correct size in advance. */
    CORRADE_COMPARE_AS(*string, "\0"_s, TestSuite::Compare::StringNotContains);
    CORRADE_COMPARE(*string->end(), '\0');
    #else
    CORRADE_SKIP("Not sure how to test on this platform.");
    #endif
}

void PathTest::readDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::read(_testDir));

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::read(): can't open {}: error 13 (", _testDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out, format(
        "Utility::Path::read(): {} is a directory\n", _testDir));
    #endif

    /* String variant should return a NullOpt as well, not testing the message
       as it should be coming from read() */
    CORRADE_VERIFY(!Path::readString(_testDir));
}

void PathTest::readNonexistent() {
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::read("nonexistent"));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::read(): can't open nonexistent: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::read(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif

    /* String variant should return a NullOpt as well, not testing the message
       as it should be coming from read() */
    CORRADE_VERIFY(!Path::readString("nonexistent"));
}

void PathTest::readNonNullTerminated() {
    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDir, "fileX").exceptSuffix(1));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::readUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(data);
    CORRADE_COMPARE_AS(*data,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
}

void PathTest::write() {
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "file");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDir, "file"),
        TestSuite::Compare::File);
}

CORRADE_HAS_TYPE(CanWriteBeCalledWith, decltype(Path::write("", std::declval<T>())));

void PathTest::writeDisabledOverloads() {
    /* I thought I could use std::is_invocable (or a C++11 backport of it) to
       easily test this. Boy I was wrong, that API is absolutely useless, while
       the CORRADE_HAS_TYPE() macro is the best thing ever.

       Same as appendDisabledOverloads(), please keep consistent */
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::ArrayView<char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::ArrayView<const char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::StringView>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::Array<char>>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<Containers::String>::value);
    /* This is desirable for writing empty files */
    CORRADE_VERIFY(CanWriteBeCalledWith<std::nullptr_t>::value);
    /* This is desirable for writing arbitrary arrays */
    CORRADE_VERIFY(CanWriteBeCalledWith<const float(&)[4]>::value);
    CORRADE_VERIFY(CanWriteBeCalledWith<float(&)[4]>::value);

    /* None of these should work as it may lead to the null terminator getting
       written by accident */
    CORRADE_VERIFY(!CanWriteBeCalledWith<const char*>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<char*>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<const char(&)[4]>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<char(&)[4]>::value);
    CORRADE_VERIFY(!CanWriteBeCalledWith<void*>::value);
}

void PathTest::writeEmpty() {
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "empty");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(file, nullptr));
    CORRADE_COMPARE_AS(file, "",
        TestSuite::Compare::FileToString);
}

void PathTest::writeDirectory() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::write(_writeTestDir, nullptr));
    /* Fortunately enough, opening the directory for writing fails already,
       without having to do anything special */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs use "Permission denied" instead of "Is a directory" */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::write(): can't open {}: error 13 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "Is a directory" */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::write(): can't open {}: error 31 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::write(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::writeNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::write(filename, nullptr));
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::write(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::writeNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "file");

    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));
    CORRADE_VERIFY(Path::write(Path::join(_writeTestDir, "fileX").exceptSuffix(1), Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDir, "file"),
        TestSuite::Compare::File);
}

void PathTest::writeUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void PathTest::append() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    constexpr const char expected[]{'h', 'e', 'l', 'l', 'o', '\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file,
        Containers::arrayView(expected),
        TestSuite::Compare::FileToString);
}

CORRADE_HAS_TYPE(CanAppendBeCalledWith, decltype(Path::append("", std::declval<T>())));

void PathTest::appendDisabledOverloads() {
    /* Same as writeDisabledOverloads(), please keep consistent */
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::ArrayView<char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::ArrayView<const char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::StringView>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::Array<char>>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<Containers::String>::value);
    /* This is desirable for writing empty files */
    CORRADE_VERIFY(CanAppendBeCalledWith<std::nullptr_t>::value);
    /* This is desirable for writing arbitrary arrays */
    CORRADE_VERIFY(CanAppendBeCalledWith<const float(&)[4]>::value);
    CORRADE_VERIFY(CanAppendBeCalledWith<float(&)[4]>::value);

    /* None of these should work as it may lead to the null terminator getting
       written by accident */
    CORRADE_VERIFY(!CanAppendBeCalledWith<const char*>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<char*>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<const char(&)[4]>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<char(&)[4]>::value);
    CORRADE_VERIFY(!CanAppendBeCalledWith<void*>::value);
}

void PathTest::appendToNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "empty");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::append(file, "hello"_s));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void PathTest::appendEmpty() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "empty");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(file, nullptr));
    CORRADE_COMPARE_AS(file, "hello",
        TestSuite::Compare::FileToString);
}

void PathTest::appendDirectory() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::append(_writeTestDir, nullptr));
    /* Fortunately enough, opening the directory for writing fails already,
       without having to do anything special */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs use "Permission denied" instead of "Is a directory" */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::append(): can't open {}: error 13 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Emscripten uses a different errno for "Is a directory" */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::append(): can't open {}: error 31 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::append(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::appendNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::append(filename, nullptr));
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::append(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::appendNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    constexpr const char expected[]{'h', 'e', 'l', 'l', 'o', '\xCA', '\xFE', '\xBA', '\xBE', '\x0D', '\x0A', '\x00', '\xDE', '\xAD', '\xBE', '\xEF'};

    Containers::String file = Path::join(_writeTestDir, "file");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::write(file, "hello"_s));
    CORRADE_VERIFY(Path::append(Path::join(_writeTestDir, "fileX").exceptSuffix(1), Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file,
        Containers::arrayView(expected),
        TestSuite::Compare::FileToString);
}

void PathTest::appendUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(file))
        CORRADE_VERIFY(Path::remove(file));

    CORRADE_VERIFY(Path::append(file, Containers::arrayView(Data)));
    CORRADE_COMPARE_AS(file, Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

void PathTest::prepareFileToCopy() {
    Path::make(_writeTestDir);

    if(Path::exists(Path::join(_writeTestDir, "copySource.dat")))
        return;

    Containers::Array<int> data{NoInit, 150000};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    Path::write(Path::join(_writeTestDir, "copySource.dat"), data);
}

void PathTest::copy() {
    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(source, destination));
    CORRADE_COMPARE_AS(source, destination, TestSuite::Compare::File);
}

void PathTest::copyEmpty() {
    Containers::String source = Path::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "empty");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(source, destination));

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
    CORRADE_COMPARE_AS(destination, "",
        TestSuite::Compare::FileToString);
}

void PathTest::copyDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    CORRADE_VERIFY(Path::exists(source));

    {
        Containers::String out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::copy(source, _writeTestDir));
        /* Opening a directory for writing fails on its own, so there's no need
           for a special message */
        #ifdef CORRADE_TARGET_WINDOWS
        /* Windows APIs use "Permission denied" instead of "Is a directory" */
        CORRADE_COMPARE_AS(out,
            format("Utility::Path::copy(): can't open {} for writing: error 13 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #elif defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Emscripten uses a different errno for "Is a directory" */
        CORRADE_COMPARE_AS(out,
            format("Utility::Path::copy(): can't open {} for writing: error 31 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #else
        CORRADE_COMPARE_AS(out,
            format("Utility::Path::copy(): can't open {} for writing: error 21 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #endif
    } {
        Containers::String out;
        Error redirectError{&out};
        CORRADE_VERIFY(!Path::copy(_writeTestDir, destination));

        /* On Windows the opening itself fails, on Unix we have an explicit
           check. On other systems no idea, so let's say we expect the same
           message as on Unix. */
        #ifdef CORRADE_TARGET_WINDOWS
        CORRADE_COMPARE_AS(out,
            format("Utility::Path::copy(): can't open {} for reading: error 13 (", _writeTestDir),
            TestSuite::Compare::StringHasPrefix);
        #else
        CORRADE_COMPARE(out, format("Utility::Path::copy(): can't read from {} which is a directory\n", _writeTestDir));
        #endif
    }
}

void PathTest::copyReadNonexistent() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::copy("nonexistent", Path::join(_writeTestDir, "empty")));
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out,
        "Utility::Path::copy(): can't open nonexistent for reading: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        "Utility::Path::copy(): can't open nonexistent for reading: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::copyWriteNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writable under Emscripten.");
    #else
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/writtenFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/writtenFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/writtenFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/writtenFile";
    #else
    Containers::StringView filename;
    CORRADE_SKIP("Not sure how to test on this system.");
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::copy(Path::join(_testDir, "dir/dummy"), filename));
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::copy(): can't open {} for writing: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void PathTest::copyNonNullTerminated() {
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String source = Path::join(_writeTestDir, "copySource.dat");
    CORRADE_VERIFY(Path::exists(source));

    Containers::String destination = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(destination))
        CORRADE_VERIFY(Path::remove(destination));

    CORRADE_VERIFY(Path::copy(Path::join(_writeTestDir, "copySource.datX").exceptSuffix(1), Path::join(_writeTestDir, "copyDestination.datX").exceptSuffix(1)));
    CORRADE_COMPARE_AS(source, destination, TestSuite::Compare::File);
}

void PathTest::copyUtf8() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) && __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30103
    /* Emscripten 3.1.3 changed the way files are bundled, putting them
       directly to WASM instead of Base64'd to the JS file. However, it broke
       UTF-8 handling, causing both a compile error (due to a syntax error in
       the assembly file) and if that's patched, also runtime errors later.
        https://github.com/emscripten-core/emscripten/pull/16050 */
    /** @todo re-enable once a fix is made */
    CORRADE_SKIP("Emscripten 3.1.3+ has broken UTF-8 handling in bundled files.");
    #endif

    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String output = Path::join(_writeTestDir, "hýždě");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_VERIFY(Path::copy(Path::join(_testDirUtf8, "hýždě"), output));
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "hýždě"),
        Path::join(_testDirUtf8, "hýždě"),
        TestSuite::Compare::File);
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
void PathTest::prepareFileToBenchmarkCopy() {
    Path::make(_writeTestDir);

    if(Path::exists(Path::join(_writeTestDir, "copyBenchmarkSource.dat")))
        return;

    /* Append a megabyte file 50 times to create a 50MB file */
    Containers::Array<int> data{ValueInit, 256*1024};
    for(std::size_t i = 0; i != data.size(); ++i) data[i] = 4678641 + i;

    for(std::size_t i = 0; i != 50; ++i)
        Path::append(Path::join(_writeTestDir, "copyBenchmarkSource.dat"), data);
}

void PathTest::copy100MReadWrite() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::read(input));
}

void PathTest::copy100MReadWriteString() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::readString(input));
}

void PathTest::copy100MCopy() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::copy(input, output);
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
void PathTest::copy100MMap() {
    Containers::String input = Path::join(_writeTestDir, "copyBenchmarkSource.dat");
    CORRADE_VERIFY(Path::exists(input));

    Containers::String output = Path::join(_writeTestDir, "copyDestination.dat");
    if(Path::exists(output))
        CORRADE_VERIFY(Path::remove(output));

    CORRADE_BENCHMARK(1)
        Path::write(output, *Path::mapRead(input));
}
#endif
#endif

void PathTest::map() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "mappedFile");
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(file);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE_AS(*mappedFile,
            Containers::arrayView(Data),
            TestSuite::Compare::Container);

        /* Write a thing there */
        (*mappedFile)[2] = '\xCA';
        (*mappedFile)[3] = '\xFE';

        /* Implicit unmap */
    }

    /* Here --------------------vv--vv- the file should be changed */
    CORRADE_COMPARE_AS(file,
        "\xCA\xFE\xCA\xFE\x0D\x0A\x00\xDE\xAD\xBE\xEF"_s,
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));
    Containers::String file = Path::join(_writeTestDir, "mappedEmpty");
    CORRADE_VERIFY(Path::write(file, nullptr));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(file);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE_AS(*mappedFile,
            Containers::arrayView<char>({}),
            TestSuite::Compare::Container);

        /* Implicit unmap */
    }

    /* The file should be still as empty as before */
    CORRADE_COMPARE_AS(file,
        "",
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::map(_writeTestDir));
    /* Opening a directory for R+W fails on its own, so there's no need for a
       special message */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::map(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::map(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::map("nonexistent"));
    CORRADE_COMPARE_AS(out,
        "Utility::Path::map(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String file = Path::join(_writeTestDir, "mappedFile");
    CORRADE_VERIFY(Path::write(file, Containers::arrayView(Data)));

    /* Enough to just verify that the file got read, no need to test writing as
       well */
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(Path::join(_writeTestDir, "mappedFileX").exceptSuffix(1));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::map(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapRead() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "file"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "dir/dummy"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView<char>({}),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadDirectory() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(!std::getenv("SIMULATOR_UDID"))
        CORRADE_SKIP("iOS (in a simulator) has no idea about file types, can't test.");
    #endif

    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapRead(_writeTestDir));
    /* On Windows the opening itself fails, on Unix we have an explicit check */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::mapRead(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out, format("Utility::Path::mapRead(): {} is a directory\n", _writeTestDir));
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapRead("nonexistent"));
    CORRADE_COMPARE_AS(out,
        "Utility::Path::mapRead(): can't open nonexistent: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDir, "fileX").exceptSuffix(1));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapReadUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = Path::mapRead(Path::join(_testDirUtf8, "hýždě"));
    CORRADE_VERIFY(mappedFile);
    CORRADE_COMPARE_AS(*mappedFile,
        Containers::arrayView(Data),
        TestSuite::Compare::Container);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWrite() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteFile"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteFile"),
        Containers::arrayView(Data),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteEmpty() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteEmpty"), 0);
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), 0);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteEmpty"),
        "",
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapWrite(_writeTestDir, 64));
    /* Opening a directory for R+W fails on its own, so there's no need for a
       special message */
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code ("Access denied") */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::mapWrite(): can't open {}: error 5 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::mapWrite(): can't open {}: error 21 (", _writeTestDir),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteNoPermission() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    #ifdef CORRADE_TARGET_APPLE
    /* Assuming there's no real possibility to run as root on Apple */
    Containers::StringView filename = "/var/root/mappedFile";
    #elif defined(CORRADE_TARGET_ANDROID)
    /* Same here, would need a rooted device */
    Containers::StringView filename = "/data/local/mappedFile";
    #elif defined(CORRADE_TARGET_UNIX)
    Containers::StringView filename = "/root/mappedFile";
    if(Path::homeDirectory() == "/root")
        CORRADE_SKIP("Running under root, can't test for permissions.");
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Only the TrustedInstaller system user is supposed to have access into
       WindowsApps */
    Containers::StringView filename = "C:/Program Files/WindowsApps/mappedFile";
    #else
    #error
    #endif

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Path::mapWrite(filename, 64));
    #ifdef CORRADE_TARGET_WINDOWS
    /* Windows APIs fill GetLastError() instead of errno, leading to a
       different code */
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::mapWrite(): can't open {}: error 5 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out,
        format("Utility::Path::mapWrite(): can't open {}: error 13 (", filename),
        TestSuite::Compare::StringHasPrefix);
    #endif
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteNonNullTerminated() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "mappedWriteFileX").exceptSuffix(1), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "mappedWriteFile"),
        Containers::arrayView(Data),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void PathTest::mapWriteUtf8() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    CORRADE_VERIFY(Path::make(_writeTestDir));

    {
        Containers::Optional<Containers::Array<char, Path::MapDeleter>> mappedFile = Path::mapWrite(Path::join(_writeTestDir, "hýždě chlípníka"), Containers::arraySize(Data));
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), Containers::arraySize(Data));
        Utility::copy(Data, *mappedFile);
    }
    CORRADE_COMPARE_AS(Path::join(_writeTestDir, "hýždě chlípníka"),
        Containers::arrayView(Data),
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
void PathTest::readWriteWhileMapped() {
    auto&& data = ReadWriteWhileMappedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::String filename = Path::join(_writeTestDir, "writeWhileMapped");
    Containers::StringView initial = "hello this is a file"_s;
    CORRADE_VERIFY(Path::write(filename, initial));

    {
        Containers::Optional<Containers::Array<const char, Path::MapDeleter>> mappedFile = data.map(filename, initial.size());
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile->size(), data.expectedBefore.size());
        CORRADE_COMPARE(Containers::StringView{*mappedFile}, data.expectedBefore);

        {
            #ifdef CORRADE_TARGET_WINDOWS
            CORRADE_EXPECT_FAIL("It's not possible to write to a file that's currently mapped on Windows.");
            #endif
            CORRADE_VERIFY(Path::write(filename, data.write));
            #ifndef CORRADE_TARGET_WINDOWS
            CORRADE_COMPARE(Containers::StringView{*mappedFile}, data.expectedAfter);
            #endif
        }

        /* Reading the file while it's mapped should work and give back exactly
           what was written, not the potentially cut / extended memory that's
           exposed to the mapped file. On Windows it gives back the original
           contents since the write failed. */
        #ifndef CORRADE_TARGET_WINDOWS
        CORRADE_COMPARE_AS(filename,
            data.write,
            TestSuite::Compare::FileToString);
        #else
        CORRADE_COMPARE_AS(filename,
            data.expectedBefore,
            TestSuite::Compare::FileToString);
        #endif

        /* Modify the mapping, if it's not mapped for just reading */
        if(data.canModify)
            const_cast<char&>(mappedFile->front()) = 'Y';

        /* Reading again after modification should reflect the modification. On
           Windows it's the original contents modified, reflected in the test
           instance data already */
        CORRADE_COMPARE_AS(filename,
            data.expectedFileAfterModification,
            TestSuite::Compare::FileToString);
    }

    /* Reading the file after unmapping should work the same way */
    CORRADE_COMPARE_AS(filename,
        data.expectedFileAfterModification,
        TestSuite::Compare::FileToString);
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::PathTest)
