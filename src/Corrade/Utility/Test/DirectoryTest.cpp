/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/Utility/Directory.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test {

struct DirectoryTest: TestSuite::Tester {
    explicit DirectoryTest();

    void fromNativeSeparators();
    void toNativeSeparators();
    void path();
    void filename();
    void join();
    #ifdef CORRADE_TARGET_WINDOWS
    void joinWindows();
    #endif
    void fileExists();
    void removeFile();
    void removeDirectory();
    void moveFile();
    void moveDirectory();
    void mkpath();
    void mkpathNoPermission();
    void isSandboxed();
    void executableLocation();
    void home();
    void configurationDir();

    void list();
    void listSkipDirectories();
    void listSkipFiles();
    void listSkipSpecial();
    void listSkipDotAndDotDot();
    void listSort();
    void listSortPrecedence();

    void read();
    void readEmpty();
    void readNonSeekable();
    void write();

    void map();
    void mapNoPermission();
    void mapRead();
    void mapReadNonexistent();

    std::string _testDir,
        _writeTestDir;
};

DirectoryTest::DirectoryTest() {
    addTests({&DirectoryTest::fromNativeSeparators,
              &DirectoryTest::toNativeSeparators,
              &DirectoryTest::path,
              &DirectoryTest::filename,
              &DirectoryTest::join,
              #ifdef CORRADE_TARGET_WINDOWS
              &DirectoryTest::joinWindows,
              #endif
              &DirectoryTest::fileExists,
              &DirectoryTest::removeFile,
              &DirectoryTest::removeDirectory,
              &DirectoryTest::moveFile,
              &DirectoryTest::moveDirectory,
              &DirectoryTest::mkpath,
              &DirectoryTest::mkpathNoPermission,
              &DirectoryTest::isSandboxed,
              &DirectoryTest::executableLocation,
              &DirectoryTest::home,
              &DirectoryTest::configurationDir,

              &DirectoryTest::list,
              &DirectoryTest::listSkipDirectories,
              &DirectoryTest::listSkipFiles,
              &DirectoryTest::listSkipSpecial,
              &DirectoryTest::listSkipDotAndDotDot,
              &DirectoryTest::listSort,
              &DirectoryTest::listSortPrecedence,

              &DirectoryTest::read,
              &DirectoryTest::readEmpty,
              &DirectoryTest::readNonSeekable,
              &DirectoryTest::write,

              &DirectoryTest::map,
              &DirectoryTest::mapNoPermission,
              &DirectoryTest::mapRead,
              &DirectoryTest::mapReadNonexistent});

    #ifdef CORRADE_TARGET_APPLE
    if(Directory::isSandboxed()
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        /** @todo Fix this once I persuade CMake to run XCTest tests properly */
        && std::getenv("SIMULATOR_UDID")
        #endif
    ) {
        _testDir = Directory::join(Directory::path(Directory::executableLocation()), "DirectoryTestFiles");
        _writeTestDir = Directory::join(Directory::home(), "Library/Caches");
    } else
    #endif
    {
        _testDir = DIRECTORY_TEST_DIR;
        _writeTestDir = DIRECTORY_WRITE_TEST_DIR;
    }
}

void DirectoryTest::fromNativeSeparators() {
    const std::string nativeSeparators = Directory::fromNativeSeparators("put\\ that/somewhere\\ else");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "put/ that/somewhere/ else");
    #else
    CORRADE_COMPARE(nativeSeparators, "put\\ that/somewhere\\ else");
    #endif
}

void DirectoryTest::toNativeSeparators() {
    const std::string nativeSeparators = Directory::toNativeSeparators("this\\is a weird/system\\right");
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird\\system\\right");
    #else
    CORRADE_COMPARE(nativeSeparators, "this\\is a weird/system\\right");
    #endif
}

void DirectoryTest::path() {
    /* No path */
    CORRADE_COMPARE(Directory::path("foo.txt"), "");

    /* No filename */
    CORRADE_COMPARE(Directory::path(".corrade/configuration/"), ".corrade/configuration");

    /* Common case */
    CORRADE_COMPARE(Directory::path("package/map.conf"), "package");
}

void DirectoryTest::filename() {
    /* Path only */
    CORRADE_COMPARE(Directory::filename("foo/bar/"), "");

    /* File only */
    CORRADE_COMPARE(Directory::filename("file.txt"), "file.txt");

    /* Common case */
    CORRADE_COMPARE(Directory::filename("foo/bar/map.conf"), "map.conf");
}

void DirectoryTest::join() {
    /* Empty path */
    CORRADE_COMPARE(Directory::join("", "/foo.txt"), "/foo.txt");

    /* Empty all */
    CORRADE_COMPARE(Directory::join("", ""), "");

    /* Absolute filename */
    CORRADE_COMPARE(Directory::join("/foo/bar", "/file.txt"), "/file.txt");

    /* Trailing slash */
    CORRADE_COMPARE(Directory::join("/foo/bar/", "file.txt"), "/foo/bar/file.txt");

    /* Common case */
    CORRADE_COMPARE(Directory::join("/foo/bar", "file.txt"), "/foo/bar/file.txt");
}

#ifdef CORRADE_TARGET_WINDOWS
void DirectoryTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Directory::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void DirectoryTest::fileExists() {
    /* File */
    CORRADE_VERIFY(Directory::fileExists(Directory::join(_testDir, "file")));

    /* Directory */
    CORRADE_VERIFY(Directory::fileExists(_testDir));

    /* Nonexistent file */
    CORRADE_VERIFY(!Directory::fileExists(Directory::join(_testDir, "nonexistentFile")));
}

void DirectoryTest::removeFile() {
    /* File */
    std::string file = Directory::join(_writeTestDir, "file.txt");
    CORRADE_VERIFY(Directory::mkpath(_writeTestDir));
    CORRADE_VERIFY(Directory::writeString(file, "a"));
    CORRADE_VERIFY(Directory::fileExists(file));
    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(!Directory::fileExists(file));

    /* Nonexistent file */
    std::string nonexistent = Directory::join(_writeTestDir, "nonexistent");
    CORRADE_VERIFY(!Directory::fileExists(nonexistent));
    CORRADE_VERIFY(!Directory::rm(nonexistent));
}

void DirectoryTest::removeDirectory() {
    /* Directory */
    std::string directory = Directory::join(_writeTestDir, "directory");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::fileExists(directory));
    CORRADE_VERIFY(Directory::rm(directory));
    CORRADE_VERIFY(!Directory::fileExists(directory));
}

void DirectoryTest::moveFile() {
    /* Old file */
    std::string oldFile = Directory::join(_writeTestDir, "oldFile.txt");
    CORRADE_VERIFY(Directory::writeString(oldFile, "a"));

    /* New file, remove if exists */
    std::string newFile = Directory::join(_writeTestDir, "newFile.txt");
    Directory::rm(newFile);

    CORRADE_VERIFY(Directory::fileExists(oldFile));
    CORRADE_VERIFY(!Directory::fileExists(newFile));
    CORRADE_VERIFY(Directory::move(oldFile, newFile));
    CORRADE_VERIFY(!Directory::fileExists(oldFile));
    CORRADE_VERIFY(Directory::fileExists(newFile));
}

void DirectoryTest::moveDirectory() {
    /* Old directory, create if not exists */
    std::string oldDirectory = Directory::join(_writeTestDir, "oldDirectory");
    if(!Directory::fileExists(oldDirectory))
        CORRADE_VERIFY(Directory::mkpath(oldDirectory));

    /* New directory, remove if exists */
    std::string newDirectory = Directory::join(_writeTestDir, "newDirectory");
    if(Directory::fileExists(newDirectory))
        CORRADE_VERIFY(Directory::rm(newDirectory));

    CORRADE_VERIFY(Directory::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Directory::fileExists(oldDirectory));
    CORRADE_VERIFY(Directory::fileExists(newDirectory));
}

void DirectoryTest::mkpath() {
    /* Existing */
    CORRADE_VERIFY(Directory::fileExists(_writeTestDir));
    CORRADE_VERIFY(Directory::mkpath(_writeTestDir));

    /* Leaf */
    std::string leaf = Directory::join(_writeTestDir, "leaf");
    if(Directory::fileExists(leaf)) CORRADE_VERIFY(Directory::rm(leaf));
    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));

    /* Path */
    std::string path = Directory::join(_writeTestDir, "path/to/new/dir");
    if(Directory::fileExists(path)) CORRADE_VERIFY(Directory::rm(path));
    if(Directory::fileExists(Directory::join(_writeTestDir, "path/to/new")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path/to/new")));
    if(Directory::fileExists(Directory::join(_writeTestDir, "path/to")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path/to")));
    if(Directory::fileExists(Directory::join(_writeTestDir, "path")))
        CORRADE_VERIFY(Directory::rm(Directory::join(_writeTestDir, "path")));

    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));
}

void DirectoryTest::mkpathNoPermission() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_SKIP("Everything is writeable under Emscripten");
    #elif !defined(CORRADE_TARGET_WINDOWS)
    if(Directory::fileExists("/nope"))
        CORRADE_SKIP("Can't test because the destination might be writeable");

    CORRADE_VERIFY(!Directory::mkpath("/nope/never"));
    #else
    if(Directory::fileExists("W:/"))
        CORRADE_SKIP("Can't test because the destination might be writeable");

    CORRADE_VERIFY(!Directory::mkpath("W:/nope"));
    #endif
}

void DirectoryTest::isSandboxed() {
    #if defined(CORRADE_TARGET_ANDROID) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_NACL) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_VERIFY(Directory::isSandboxed());
    #else
    CORRADE_VERIFY(!Directory::isSandboxed());
    #endif
}

void DirectoryTest::executableLocation() {
    const std::string executableLocation = Directory::executableLocation();
    Debug() << "Executable location found as:" << executableLocation;

    /* On sandboxed OSX and iOS verify that the directory contains Info.plist
       file */
    #ifdef CORRADE_TARGET_APPLE
    if(Directory::isSandboxed()) {
        #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
        CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
            "CTest is not able to run XCTest executables properly in the simulator.");
        #endif

        CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(executableLocation), "Info.plist")));
    } else
    #endif

    /* On Emscripten we should have access to the bundled files */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(executableLocation), "DirectoryTestFiles")));

    /* Otherwise it should contain CMake build files */
    #else
    {
        #ifdef CMAKE_CFG_INTDIR
        CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(Directory::path(executableLocation)), "CMakeFiles")));
        #else
        CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(executableLocation), "CMakeFiles")));
        #endif
    }
    #endif

    /* On Windows it shouldn't contain backslashes */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE(executableLocation.find('\\'), std::string::npos);
    #endif
}

void DirectoryTest::home() {
    const std::string home = Directory::home();
    Debug() << "Home dir found as:" << home;

    /* On OSX and iOS verify that the home dir contains `Library` directory */
    #ifdef CORRADE_TARGET_APPLE
    CORRADE_VERIFY(Directory::fileExists(Directory::join(home, "Library")));

    /* On other Unixes verify that the home dir contains `.local` directory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/home/` substring, as that can be overriden. */
    #elif defined(CORRADE_TARGET_UNIX)
    CORRADE_VERIFY(Directory::fileExists(Directory::join(home, ".local")));

    /* On Emscripten verify that the directory exists (it's empty by default) */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_VERIFY(Directory::fileExists(home));

    /* On Windows verify that the home dir contains `desktop.ini` file. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overriden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_VERIFY(Directory::fileExists(Directory::join(home, "desktop.ini")));

    /* On Windows it also shouldn't contain backslashes */
    CORRADE_COMPARE(home.find('\\'), std::string::npos);

    /* No idea elsewhere */
    #else
    CORRADE_EXPECT_FAIL("Not implemented yet.");
    CORRADE_COMPARE(home, "(not implemented)");
    #endif
}

void DirectoryTest::configurationDir() {
    const std::string dir = Directory::configurationDir("Corrade");
    Debug() << "Configuration dir found as:" << dir;

    #ifdef CORRADE_TARGET_APPLE
    CORRADE_COMPARE(dir.substr(dir.size() - 7), "Corrade");
    if(Directory::isSandboxed())
        CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(Directory::path(dir)), "Caches")));
    else
        CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(dir), "App Store")));

    /* On Linux verify that the parent dir contains `autostart` directory,
       something from GTK or something from Qt. Ugly and hacky, but it's the
       best I could come up with. Can't test for e.g. `/home/` substring, as
       that can be overriden. */
    #elif defined(__linux__) && !defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE(dir.substr(dir.size()-7), "corrade");
    CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(dir), "autostart")) ||
                   Directory::fileExists(Directory::join(Directory::path(dir), "dconf")) ||
                   Directory::fileExists(Directory::join(Directory::path(dir), "Trolltech.conf")));

    /* Emscripten -- just compare to hardcoded value */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(Directory::path(dir), "/home/web_user/.config");

    /* On Windows verify that the parent dir contains `Microsoft` subdirectory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overriden. */
    #elif defined(CORRADE_TARGET_WINDOWS)
    CORRADE_COMPARE(dir.substr(dir.size()-7), "Corrade");
    CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(dir), "Microsoft")));

    /* On Windows it also shouldn't contain backslashes */
    CORRADE_COMPARE(dir.find('\\'), std::string::npos);

    /* No idea elsewhere */
    #else
    CORRADE_EXPECT_FAIL("Not implemented yet.");
    CORRADE_COMPARE(dir, "(not implemented)");
    #endif
}

void DirectoryTest::list() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipDirectories() {
    #ifdef TRAVIS_CI_HAS_CRAZY_FILESYSTEM_ON_LINUX
    CORRADE_EXPECT_FAIL("Travis CI has crazy filesystem on Linux.");
    #endif
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipDirectories),
        std::vector<std::string>{"file"},
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipFiles() {
    #ifdef TRAVIS_CI_HAS_CRAZY_FILESYSTEM_ON_LINUX
    CORRADE_EXPECT_FAIL("Travis CI has crazy filesystem on Linux.");
    #endif
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipFiles),
        (std::vector<std::string>{".", "..", "dir"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipSpecial() {
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_EXPECT_FAIL("Files are treated as special in Emscripten.");
    #endif
    #ifdef TRAVIS_CI_HAS_CRAZY_FILESYSTEM_ON_LINUX
    CORRADE_EXPECT_FAIL("Travis CI has crazy filesystem on Linux.");
    #endif
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipSpecial),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSkipDotAndDotDot() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SkipDotAndDotDot),
        (std::vector<std::string>{"dir", "file"}),
        TestSuite::Compare::SortedContainer);
}

void DirectoryTest::listSort() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    CORRADE_EXPECT_FAIL_IF(!std::getenv("SIMULATOR_UDID"),
        "CTest is not able to run XCTest executables properly in the simulator.");
    #endif

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SortAscending),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::Container);

    CORRADE_COMPARE_AS(Directory::list(_testDir, Directory::Flag::SortDescending),
        (std::vector<std::string>{"file", "dir", "..", "."}),
        TestSuite::Compare::Container);
}

void DirectoryTest::listSortPrecedence() {
    CORRADE_VERIFY((Directory::Flag::SortAscending|Directory::Flag::SortDescending) == Directory::Flag::SortAscending);
}

void DirectoryTest::read() {
    /* Existing file, check if we are reading it as binary (CR+LF is not
       converted to LF) and nothing after \0 gets lost */
    CORRADE_COMPARE_AS(Directory::read(Directory::join(_testDir, "file")),
        (Containers::Array<char>::from(0xCA, 0xFE, 0xBA, 0xBE, 0x0D, 0x0A, 0x00, 0xDE, 0xAD, 0xBE, 0xEF)),
        TestSuite::Compare::Container);

    /* Nonexistent file */
    const auto none = Directory::read("nonexistent");
    CORRADE_VERIFY(!none);

    /* Read into string */
    CORRADE_COMPARE(Directory::readString(Directory::join(_testDir, "file")),
        std::string("\xCA\xFE\xBA\xBE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11));
}

void DirectoryTest::readEmpty() {
    const std::string empty = Directory::join(_testDir, "dir/dummy");
    CORRADE_VERIFY(Directory::fileExists(empty));
    CORRADE_VERIFY(!Directory::read(empty));
}

void DirectoryTest::readNonSeekable() {
    #if defined(__unix__) && !defined(CORRADE_TARGET_EMSCRIPTEN) /* (OS X doesn't have /proc) */
    /** @todo Test more thoroughly than this */
    const auto data = Directory::read("/proc/loadavg");
    CORRADE_VERIFY(!data.empty());
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::write() {
    constexpr unsigned char data[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x0D, 0x0A, 0x00, 0xDE, 0xAD, 0xBE, 0xEF};
    CORRADE_VERIFY(Directory::write(Directory::join(_writeTestDir, "file"), data));
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "file"),
        Directory::join(_testDir, "file"),
        TestSuite::Compare::File);

    CORRADE_VERIFY(Directory::writeString(Directory::join(_writeTestDir, "file"),
        std::string("\xCA\xFE\xBA\xBE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11)));
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "file"),
        Directory::join(_testDir, "file"),
        TestSuite::Compare::File);
}

void DirectoryTest::map() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    std::string data{"\xCA\xFE\xBA\xBE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11};
    {
        auto mappedFile = Directory::map(Directory::join(_writeTestDir, "mappedFile"), data.size());
        CORRADE_VERIFY(mappedFile);
        CORRADE_COMPARE(mappedFile.size(), data.size());
        std::copy(std::begin(data), std::end(data), mappedFile.begin());
    }
    CORRADE_COMPARE_AS(Directory::join(_writeTestDir, "mappedFile"),
        data,
        TestSuite::Compare::FileToString);
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapNoPermission() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        std::ostringstream out;
        Error err{&out};
        auto mappedFile = Directory::map("/root/mappedFile", 64);
        CORRADE_VERIFY(!mappedFile);
        CORRADE_COMPARE(out.str(), "Utility::Directory::map(): can't open the file\n");
    }
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapRead() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        const auto mappedFile = Directory::mapRead(Directory::join(_testDir, "file"));
        CORRADE_COMPARE_AS(Containers::ArrayView<const char>(mappedFile),
            (Containers::Array<char>::from(0xCA, 0xFE, 0xBA, 0xBE, 0x0D, 0x0A, 0x00, 0xDE, 0xAD, 0xBE, 0xEF)),
            TestSuite::Compare::Container);
    }
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::mapReadNonexistent() {
    #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    {
        std::ostringstream out;
        Error err{&out};
        const auto mappedFile = Directory::mapRead(Directory::join(_testDir, "nonexistentFile"));
        CORRADE_VERIFY(!mappedFile);
        CORRADE_COMPARE(out.str(), "Utility::Directory::mapRead(): can't open the file\n");
    }
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DirectoryTest)
