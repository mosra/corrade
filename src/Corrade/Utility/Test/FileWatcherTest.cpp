/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/FileWatcher.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/System.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct FileWatcherTest: TestSuite::Tester {
    explicit FileWatcherTest();

    void nonexistent();

    void setup();
    void teardown();

    void changedRead();
    void changedWrite();
    void changedWriteUtf8();
    void changedDeleted();
    void changedRecreatedImmediately();
    void changedRecreatedLate();
    void changedRecreatedLateIgnoreErrors();
    void changedCleared();
    void changedClearedIgnoreEmpty();

    void debugFlag();
    void debugFlags();

    private:
        Containers::String _filename;
};

FileWatcherTest::FileWatcherTest() {
    addTests({&FileWatcherTest::nonexistent});

    addTests({&FileWatcherTest::changedRead,
              &FileWatcherTest::changedWrite},
             &FileWatcherTest::setup, &FileWatcherTest::teardown);

    addTests({&FileWatcherTest::changedWriteUtf8});

    addTests({&FileWatcherTest::changedDeleted,
              &FileWatcherTest::changedRecreatedImmediately,
              &FileWatcherTest::changedRecreatedLate,
              &FileWatcherTest::changedRecreatedLateIgnoreErrors,
              &FileWatcherTest::changedCleared,
              &FileWatcherTest::changedClearedIgnoreEmpty},
             &FileWatcherTest::setup, &FileWatcherTest::teardown);

    addTests({&FileWatcherTest::debugFlag,
              &FileWatcherTest::debugFlags});

    Path::make(FILEWATCHER_WRITE_TEST_DIR);
    _filename = Path::join(FILEWATCHER_WRITE_TEST_DIR, "file.txt");
}

using namespace Containers::Literals;

void FileWatcherTest::nonexistent() {
    std::ostringstream out;
    {
        Error redirectError{&out};
        FileWatcher watcher{"nonexistent"};
        CORRADE_COMPARE(watcher.flags(), FileWatcher::Flags{});
        CORRADE_VERIFY(!watcher.isValid());
        CORRADE_VERIFY(!watcher.hasChanged());
    }

    /* Error reported only once, hasChanged() is a no-op when not valid */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* Emscripten uses a different errno for "No such file or directory" */
    CORRADE_COMPARE_AS(out.str(),
        "Utility::FileWatcher: can't stat nonexistent, aborting watch: error 44 (",
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE_AS(out.str(),
        "Utility::FileWatcher: can't stat nonexistent, aborting watch: error 2 (",
        TestSuite::Compare::StringHasPrefix);
    #endif
}

void FileWatcherTest::setup() {
    Path::write(_filename, "hello"_s);
}

void FileWatcherTest::teardown() {
    Path::remove(_filename);
}

void FileWatcherTest::changedRead() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_COMPARE(watcher.flags(), FileWatcher::Flags{});
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    /* So we don't write at the same nanosecond. Linux gives us 10-millisecond
       precision, HFS+ on macOS has second precision (even though the API has
       nanoseconds), on Windows the API itself has second granularity.
       https://developer.apple.com/library/archive/technotes/tn/tn1150.html#HFSPlusDates
       https://github.com/kripken/emscripten/blob/52ff847187ee30fba48d611e64b5d10e2498fe0f/src/library_syscall.js#L66 */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_COMPARE(Path::readString(_filename), Containers::String{"hello"});

    CORRADE_VERIFY(!watcher.hasChanged());
}

void FileWatcherTest::changedWrite() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::write(_filename, "ahoy"_s));

    CORRADE_VERIFY(watcher.hasChanged());
    CORRADE_VERIFY(!watcher.hasChanged()); /* Nothing changed second time */
}

void FileWatcherTest::changedWriteUtf8() {
    Containers::String filenameUtf8 = Path::join(FILEWATCHER_WRITE_TEST_DIR, "šňůra.txt");
    CORRADE_VERIFY(Path::write(filenameUtf8, "hýždě"_s));

    FileWatcher watcher{filenameUtf8};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::write(filenameUtf8, "půlky"_s));

    CORRADE_VERIFY(watcher.hasChanged());
    CORRADE_VERIFY(!watcher.hasChanged()); /* Nothing changed second time */
}

void FileWatcherTest::changedDeleted() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    CORRADE_VERIFY(Path::remove(_filename));
    CORRADE_VERIFY(!watcher.hasChanged());
    CORRADE_VERIFY(!watcher.isValid());
}

void FileWatcherTest::changedRecreatedImmediately() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    CORRADE_VERIFY(Path::remove(_filename));

    /* Not checking here otherwise it would invalidate the watcher */

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    Path::write(_filename, "hello again"_s);

    CORRADE_VERIFY(watcher.hasChanged());
    CORRADE_VERIFY(watcher.isValid());
}

void FileWatcherTest::changedRecreatedLate() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    CORRADE_VERIFY(Path::remove(_filename));

    /* Checking here will invalidate the watcher */
    CORRADE_VERIFY(!watcher.hasChanged());
    CORRADE_VERIFY(!watcher.isValid());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    Path::write(_filename, "hello again"_s);

    /* And it won't recover from it */
    CORRADE_VERIFY(!watcher.hasChanged());
    CORRADE_VERIFY(!watcher.isValid());
}

void FileWatcherTest::changedRecreatedLateIgnoreErrors() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename, FileWatcher::Flag::IgnoreErrors};
    CORRADE_COMPARE(watcher.flags(), FileWatcher::Flag::IgnoreErrors);
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    CORRADE_VERIFY(Path::remove(_filename));

    /* File is gone, but that gets ignored */
    CORRADE_VERIFY(!watcher.hasChanged());
    CORRADE_VERIFY(watcher.isValid());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    Path::write(_filename, "hello again"_s);

    CORRADE_VERIFY(watcher.hasChanged());
    CORRADE_VERIFY(watcher.isValid());
}

void FileWatcherTest::changedCleared() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename};
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif
    CORRADE_VERIFY(Path::write(_filename, {}));
    CORRADE_VERIFY(watcher.hasChanged());

    /* A change right after should not get detected, since it's too soon */
    CORRADE_VERIFY(Path::write(_filename, "some content again"_s));
    bool changed = watcher.hasChanged();
    {
        #if !defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        /* Started happening on CircleCI macOS as well after an update to XCode
        10.2 image on 2021-06-15, previously this was linux-specific. Maybe the
        timer resolution got better there? Needs further investigation. */
        CORRADE_EXPECT_FAIL_IF(changed, "Gah! Your system is too fast.");
        #endif
        CORRADE_VERIFY(!changed); /* Nothing changed second time */
    }
}

void FileWatcherTest::changedClearedIgnoreEmpty() {
    CORRADE_VERIFY(Path::exists(_filename));

    FileWatcher watcher{_filename, FileWatcher::Flag::IgnoreChangeIfEmpty};
    CORRADE_COMPARE(watcher.flags(), FileWatcher::Flag::IgnoreChangeIfEmpty);
    CORRADE_VERIFY(watcher.isValid());
    CORRADE_VERIFY(!watcher.hasChanged());

    /* See above for details */
    /** @todo get rid of this once proper FS inode etc. watching is implemented */
    #if defined(CORRADE_TARGET_APPLE) || defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_EMSCRIPTEN)
    System::sleep(1100);
    #else
    System::sleep(10);
    #endif

    /* Change to an empty file is ignored */
    CORRADE_VERIFY(Path::write(_filename, {}));
    {
        #ifdef CORRADE_TARGET_IOS
        CORRADE_EXPECT_FAIL("iOS seems to be reporting all file sizes to be 0, so the IgnoreChangeIfEmpty flag is ignored there.");
        #endif
        CORRADE_VERIFY(!watcher.hasChanged());
    }

    /* When the file becomes non-empty again, the change is signalled */
    CORRADE_VERIFY(Path::write(_filename, "some content again"_s));
    {
        /* This used to fail on iOS back when Travis CI was used, but on
           CircleCI emulator it passes. Since the whole thing needs to be
           reworked to use OS filesystem watchers (and it's quite useless on
           iOS anyway), I don't really care. */
        CORRADE_VERIFY(watcher.hasChanged());
    }
}

void FileWatcherTest::debugFlag() {
    std::ostringstream out;

    Debug(&out) << FileWatcher::Flag::IgnoreChangeIfEmpty << FileWatcher::Flag(0xde);
    CORRADE_COMPARE(out.str(), "Utility::FileWatcher::Flag::IgnoreChangeIfEmpty Utility::FileWatcher::Flag(0xde)\n");
}

void FileWatcherTest::debugFlags() {
    std::ostringstream out;

    Debug(&out) << (FileWatcher::Flag::IgnoreChangeIfEmpty|FileWatcher::Flag::IgnoreErrors) << FileWatcher::Flags{};
    CORRADE_COMPARE(out.str(), "Utility::FileWatcher::Flag::IgnoreErrors|Utility::FileWatcher::Flag::IgnoreChangeIfEmpty Utility::FileWatcher::Flags{}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::FileWatcherTest)
