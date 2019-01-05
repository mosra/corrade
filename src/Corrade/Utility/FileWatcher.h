#ifndef Corrade_Utility_FileWatcher_h
#define Corrade_Utility_FileWatcher_h
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

/** @file
 * @brief Class @ref Corrade::Utility::FileWatcher
 */

#include <string>

#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
/**
@brief File watcher

Provides a non-blocking interface to watch a single file for changes. Example
usage:

@snippet Utility.cpp FileWatcher

@section Utility-FileWatcher-behavior Behavior

The generic implementation (currently used on all supported systems) checks for
file modification time and reports a change if the modification time changes.
Deleting a file and immediately recreating it with the same name will behave
the same as simply updating that file, unless the file status is checked during
the short time when it was deleted --- in that case @ref isValid() will return
@cpp false @ce and monitoring is stopped.

Different OSes and filesystems have different granularity of filesystem
modification time:

-   Most native Linux filesystems (such as ext4) will report file modification
    time in millisecond precision (usually tens of milliseconds)
-   Windows, macOS and Emscripten file modification time APIs return the value
    in seconds, FAT filesystems have two-second precision

@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms and on
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten". On Emscripten it woeks on the
    virtual filesystem.
*/
class CORRADE_UTILITY_EXPORT FileWatcher {
    public:
        /** @brief Constructor */
        explicit FileWatcher(const std::string& filename);

        /** @brief Copying is not allowed */
        FileWatcher(const FileWatcher&) = delete;

        /** @brief Move constructor */
        FileWatcher(FileWatcher&&) noexcept;

        /** @brief Copying is not allowed */
        FileWatcher& operator=(const FileWatcher&) = delete;

        /** @brief Move assignment */
        FileWatcher& operator=(FileWatcher&&)
            /* std::string move assignment in libstdc++ before 5.5 is not
               noexcept: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58265
               It went through on GCC 4.7, but apparently only because it
               doesn't care; it causes build failure on Clang in case it's
               using libstdc++ < 5.5. Since it's impossible to check for a
               libstdc++ version when using Clang (the version number is A
               RELEASE DATE, wtf!), I'm checking for it this way. */
            #ifdef __GNUC__
            noexcept(std::is_nothrow_move_assignable<std::string>::value)
            #else
            noexcept
            #endif
            ;

        ~FileWatcher();

        /**
         * @brief Whether the file watcher is valid
         *
         * Returns @cpp true @ce if the watcher was valid the last time
         * @ref hasChanged() was called (or, if not called yet, on
         * construction). For example, a file could get deleted in the meantime
         * or a filesystem unmounted. Note that it's also possible for an
         * invalid watch to become valid later, for example if the file under
         * watch gets recreated again.
         */
        bool isValid() const { return _valid; }

        /**
         * @brief Whether the file has changed
         *
         * Returns @cpp true @ce if the file modification time was updated
         * since the previous call, @cpp false @ce otherwise.
         */
        bool hasChanged();

    private:
        #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
        std::string _filename;
        #elif defined(CORRADE_TARGET_WINDOWS)
        std::wstring _filename;
        #else
        #error
        #endif
        bool _valid = true;
        std::uint64_t _time;
};
#else
#error this header is available only on Unix, non-RT Windows and Emscripten
#endif

}}

#endif
