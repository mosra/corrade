#ifndef Corrade_Utility_FileWatcher_h
#define Corrade_Utility_FileWatcher_h
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

/** @file
 * @brief Class @ref Corrade::Utility::FileWatcher
 */

#include <string>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/Utility.h"
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
@cpp false @ce and monitoring is stopped. Pass @ref Flag::IgnoreErrors to
the constructor to disable this behavior. Similarly, in some cases a file
update might first empty the contents, update modification timestamp and only
then populate it with updated data but without a second timestamp update.
Reacting to the update when the file is still empty might be counterproductive
as well, enable @ref Flag::IgnoreChangeIfEmpty to detect and ignore this case
as well.

Different OSes and filesystems have different granularity of filesystem
modification time:

-   Most native Linux filesystems (such as ext4) will report file modification
    time in millisecond precision (usually tens of milliseconds)
-   Windows, macOS and Emscripten file modification time APIs return the value
    in seconds, FAT filesystems have two-second precision

@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms and on
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten". On Emscripten it works on the
    virtual filesystem.
*/
class CORRADE_UTILITY_EXPORT FileWatcher {
    public:
        /**
         * @brief Watch behavior flag
         * @m_since{2019,10}
         *
         * @see @ref Flags, @ref FileWatcher(const std::string&, Flags),
         *      @ref flags()
         */
        enum class Flag: std::uint8_t {
            /**
             * Don't abort the watch on errors. Useful if the watched file is
             * being updated by deleting it first and then creating a new one.
             */
            IgnoreErrors = 1 << 0,

            /**
             * Don't signal a file change if it's currently empty. Useful if
             * the watched file is being updated by first clearing its contents
             * together with updating the modification time and then populating
             * it without updating the modifcation time again.
             *
             * @note @ref CORRADE_TARGET_IOS "iOS" seems to be always reporting
             *      file sizes as 0 which would make @ref FileWatcher
             *      absolutely useless with this flag. This flag is thus
             *      ignored there.
             */
            IgnoreChangeIfEmpty = 1 << 1
        };

        /**
         * @brief Watch behavior flags
         * @m_since{2019,10}
         *
         * @see @ref FileWatcher(const std::string&, Flags), @ref flags()
         */
        typedef Containers::EnumSet<Flag> Flags;

        /** @brief Constructor */
        explicit FileWatcher(const std::string& filename, Flags flags = {});

        /** @brief Copying is not allowed */
        FileWatcher(const FileWatcher&) = delete;

        /** @brief Move constructor */
        FileWatcher(FileWatcher&&)
            #ifdef __GNUC__
            noexcept(std::is_nothrow_move_constructible<std::string>::value)
            #else
            noexcept
            #endif
            ;

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

        /** @brief Watch behavior flags */
        Flags flags() const;

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
        bool isValid() const;

        /**
         * @brief Whether the file has changed
         *
         * Returns @cpp true @ce if the file modification time was updated
         * since the previous call, @cpp false @ce otherwise.
         */
        bool hasChanged();

    private:
        enum class InternalFlag: std::uint8_t;
        typedef Containers::EnumSet<InternalFlag> InternalFlags;
        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
        std::string _filename;
        #elif defined(CORRADE_TARGET_WINDOWS)
        std::wstring _filename;
        #else
        #error
        #endif
        InternalFlags _flags;
        std::uint64_t _time;
};

CORRADE_ENUMSET_OPERATORS(FileWatcher::Flags)

/** @debugoperatorclassenum{FileWatcher,FileWatcher::Flag} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, FileWatcher::Flag value);

/** @debugoperatorclassenum{FileWatcher,FileWatcher::Flags} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, FileWatcher::Flags value);
#else
#error this header is available only on Unix, non-RT Windows and Emscripten
#endif

}}

#endif
