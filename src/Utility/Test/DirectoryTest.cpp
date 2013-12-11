/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "Containers/Array.h"
#include "TestSuite/Tester.h"
#include "TestSuite/Compare/Container.h"
#include "TestSuite/Compare/File.h"
#include "Utility/Directory.h"

#include "testConfigure.h"

namespace Corrade { namespace Utility { namespace Test {

class DirectoryTest: public Corrade::TestSuite::Tester {
    public:
        DirectoryTest();

        void path();
        void filename();
        void join();
        #ifdef _WIN32
        void joinWindows();
        #endif
        void fileExists();
        void remove();
        void moveFile();
        void moveDirectory();
        void mkpath();
        void home();
        void configurationDir();
        void list();
        void listSortPrecedence();
        void read();
        void readNonSeekable();
        void write();
};

DirectoryTest::DirectoryTest() {
    addTests({&DirectoryTest::path,
              &DirectoryTest::filename,
              &DirectoryTest::join,
              #ifdef _WIN32
              &DirectoryTest::joinWindows,
              #endif
              &DirectoryTest::fileExists,
              &DirectoryTest::remove,
              &DirectoryTest::moveFile,
              &DirectoryTest::moveDirectory,
              &DirectoryTest::mkpath,
              &DirectoryTest::home,
              &DirectoryTest::configurationDir,
              &DirectoryTest::list,
              &DirectoryTest::listSortPrecedence,
              &DirectoryTest::read,
              &DirectoryTest::readNonSeekable,
              &DirectoryTest::write});
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

#ifdef _WIN32
void DirectoryTest::joinWindows() {
    /* Drive letter */
    CORRADE_COMPARE(Directory::join("/foo/bar", "X:/path/file.txt"), "X:/path/file.txt");
}
#endif

void DirectoryTest::fileExists() {
    /* File */
    CORRADE_VERIFY(Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "file")));

    /* Directory */
    CORRADE_VERIFY(Directory::fileExists(DIRECTORY_TEST_DIR));

    /* Nonexistent file */
    CORRADE_VERIFY(!Directory::fileExists(Directory::join(DIRECTORY_TEST_DIR, "nonexistentFile")));
}

void DirectoryTest::remove() {
    /* Directory */
    std::string directory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "directory");
    CORRADE_VERIFY(Directory::mkpath(directory));
    CORRADE_VERIFY(Directory::fileExists(directory));
    CORRADE_VERIFY(Directory::rm(directory));
    CORRADE_VERIFY(!Directory::fileExists(directory));

    /* File */
    std::string file = Directory::join(DIRECTORY_WRITE_TEST_DIR, "file.txt");
    CORRADE_VERIFY(Directory::writeString(file, "a"));
    CORRADE_VERIFY(Directory::fileExists(file));
    CORRADE_VERIFY(Directory::rm(file));
    CORRADE_VERIFY(!Directory::fileExists(file));

    /* Nonexistent file */
    std::string nonexistent = Directory::join(DIRECTORY_WRITE_TEST_DIR, "nonexistent");
    CORRADE_VERIFY(!Directory::fileExists(nonexistent));
    CORRADE_VERIFY(!Directory::rm(nonexistent));
}

void DirectoryTest::moveFile() {
    /* Old file */
    std::string oldFile = Directory::join(DIRECTORY_WRITE_TEST_DIR, "oldFile.txt");
    CORRADE_VERIFY(Directory::writeString(oldFile, "a"));

    /* New file, remove if exists */
    std::string newFile = Directory::join(DIRECTORY_WRITE_TEST_DIR, "newFile.txt");
    Directory::rm(newFile);

    CORRADE_VERIFY(Directory::fileExists(oldFile));
    CORRADE_VERIFY(!Directory::fileExists(newFile));
    CORRADE_VERIFY(Directory::move(oldFile, newFile));
    CORRADE_VERIFY(!Directory::fileExists(oldFile));
    CORRADE_VERIFY(Directory::fileExists(newFile));
}

void DirectoryTest::moveDirectory() {
    /* Old directory, create if not exists */
    std::string oldDirectory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "oldDirectory");
    if(!Directory::fileExists(oldDirectory))
        CORRADE_VERIFY(Directory::mkpath(oldDirectory));

    /* New directory, remove if exists */
    std::string newDirectory = Directory::join(DIRECTORY_WRITE_TEST_DIR, "newDirectory");
    if(Directory::fileExists(newDirectory))
        CORRADE_VERIFY(Directory::rm(newDirectory));

    CORRADE_VERIFY(Directory::move(oldDirectory, newDirectory));
    CORRADE_VERIFY(!Directory::fileExists(oldDirectory));
    CORRADE_VERIFY(Directory::fileExists(newDirectory));
}

void DirectoryTest::mkpath() {
    /* Existing */
    CORRADE_VERIFY(Directory::fileExists(DIRECTORY_WRITE_TEST_DIR));
    CORRADE_VERIFY(Directory::mkpath(DIRECTORY_WRITE_TEST_DIR));

    /* Leaf */
    std::string leaf = Directory::join(DIRECTORY_WRITE_TEST_DIR, "leaf");
    if(Directory::fileExists(leaf)) CORRADE_VERIFY(Directory::rm(leaf));
    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));

    /* Path */
    std::string path = Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new/dir");
    if(Directory::fileExists(path)) CORRADE_VERIFY(Directory::rm(path));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to/new")));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path/to")));
    if(Directory::fileExists(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path")))
        CORRADE_VERIFY(Directory::rm(Directory::join(DIRECTORY_WRITE_TEST_DIR, "path")));

    CORRADE_VERIFY(Directory::mkpath(leaf));
    CORRADE_VERIFY(Directory::fileExists(leaf));
}

void DirectoryTest::home() {
    const std::string home = Directory::home();
    Debug() << "Home dir found as:" << home;

    /* On Linux verify that the home dir contains `.local` directory. Ugly and
       hacky, but it's the best I came up with. Can't test for e.g. `/home/`
       substring, as that can be overriden. */
    #ifdef __linux__
    CORRADE_VERIFY(Directory::fileExists(Directory::join(home, ".local")));

    /* On Windows verify that the home dir contains `desktop.ini` file. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overriden. */
    #elif defined(_WIN32)
    CORRADE_VERIFY(Directory::fileExists(Directory::join(home, "desktop.ini")));

    /* No idea elsewhere */
    #else
    CORRADE_EXPECT_FAIL("Not implemented yet.");
    CORRADE_COMPARE(home, "");
    #endif
}

void DirectoryTest::configurationDir() {
    const std::string dir = Directory::configurationDir("Corrade");
    Debug() << "Configuration dir found as:" << dir;

    /* On Linux verify that the parent dir contains `autostart` directory. Ugly
       and hacky, but it's the best I came up with. Can't test for e.g.
       `/home/` substring, as that can be overriden. */
    #if __linux__
    CORRADE_COMPARE(dir.substr(dir.size()-7), "corrade");
    CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(dir), "autostart")));

    /* On Windows verify that the parent dir contains `Microsoft` subdirectory.
       Ugly and hacky, but it's the best I came up with. Can't test for e.g.
       `/Users/` substring, as that can be overriden. */
    #elif defined(_WIN32)
    CORRADE_COMPARE(dir.substr(dir.size()-7), "Corrade");
    CORRADE_VERIFY(Directory::fileExists(Directory::join(Directory::path(dir), "Microsoft")));

    /* No idea elsewhere */
    #else
    CORRADE_EXPECT_FAIL("Not implemented yet.");
    CORRADE_COMPARE(dir, "");
    #endif
}

void DirectoryTest::list() {
    /* All */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* Skip special */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipSpecial),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* All, sorted ascending */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SortAscending),
        (std::vector<std::string>{".", "..", "dir", "file"}),
        TestSuite::Compare::Container);

    /* All, sorted descending */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SortDescending),
        (std::vector<std::string>{"file", "dir", "..", "."}),
        TestSuite::Compare::Container);

    /* Skip . and .. */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipDotAndDotDot),
        (std::vector<std::string>{"dir", "file"}),
        TestSuite::Compare::SortedContainer);

    /* Skip directories */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipDirectories),
        std::vector<std::string>{"file"},
        TestSuite::Compare::SortedContainer);

    /* Skip files */
    CORRADE_COMPARE_AS(Directory::list(DIRECTORY_TEST_DIR, Directory::Flag::SkipFiles),
        (std::vector<std::string>{".", "..", "dir"}),
        TestSuite::Compare::SortedContainer);

}

void DirectoryTest::listSortPrecedence() {
    CORRADE_VERIFY((Directory::Flag::SortAscending|Directory::Flag::SortDescending) == Directory::Flag::SortAscending);
}

void DirectoryTest::read() {
    /* Existing file, check if we are reading it as binary (CR+LF is not
       converted to LF) and nothing after \0 gets lost */
    const auto data = Directory::read(Directory::join(DIRECTORY_TEST_DIR, "file"));
    CORRADE_COMPARE(std::vector<unsigned char>(data.begin(), data.end()),
        (std::vector<unsigned char>{0xCA, 0xFE, 0xBA, 0xBE, 0x0D, 0x0A, 0x00, 0xDE, 0xAD, 0xBE, 0xEF}));

    /* Nonexistent file */
    const auto none = Directory::read("nonexistent");
    CORRADE_VERIFY(!none);

    /* Read into string */
    CORRADE_COMPARE(Directory::readString(Directory::join(DIRECTORY_TEST_DIR, "file")),
        std::string("\xCA\xFE\xBA\xBE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11));
}

void DirectoryTest::readNonSeekable() {
    #ifdef __unix__
    /** @todo Test more thoroughly than this */
    const auto data = Directory::read("/proc/loadavg");
    CORRADE_VERIFY(!data.empty());
    #else
    CORRADE_SKIP("Not implemented on this platform.");
    #endif
}

void DirectoryTest::write() {
    constexpr unsigned char data[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x0D, 0x0A, 0x00, 0xDE, 0xAD, 0xBE, 0xEF};
    CORRADE_VERIFY(Directory::write(Directory::join(DIRECTORY_WRITE_TEST_DIR, "file"), data));
    CORRADE_COMPARE_AS(Directory::join(DIRECTORY_WRITE_TEST_DIR, "file"),
        Directory::join(DIRECTORY_TEST_DIR, "file"),
        TestSuite::Compare::File);

    CORRADE_VERIFY(Directory::writeString(Directory::join(DIRECTORY_WRITE_TEST_DIR, "file"),
        std::string("\xCA\xFE\xBA\xBE\x0D\x0A\x00\xDE\xAD\xBE\xEF", 11)));
    CORRADE_COMPARE_AS(Directory::join(DIRECTORY_WRITE_TEST_DIR, "file"),
        Directory::join(DIRECTORY_TEST_DIR, "file"),
        TestSuite::Compare::File);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DirectoryTest)
