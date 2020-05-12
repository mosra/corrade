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

#include "FileWatcher.h"

#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Utility/DebugStl.h"

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Unicode.h"
#endif

namespace Corrade { namespace Utility {

#ifndef DOXYGEN_GENERATING_OUTPUT
enum class FileWatcher::InternalFlag: std::uint8_t {
    /* Keep in sync with Flag */
    IgnoreErrors = std::uint8_t(FileWatcher::Flag::IgnoreErrors),
    IgnoreChangeIfEmpty = std::uint8_t(FileWatcher::Flag::IgnoreChangeIfEmpty),

    Valid = 1 << 7
};
#endif

FileWatcher::FileWatcher(const std::string& filename, Flags flags):
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    _filename{filename},
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    _filename{Unicode::widen(filename)},
    #else
    #error
    #endif
    _flags{InternalFlag(std::uint8_t(flags))|InternalFlag::Valid},
    _time{~std::uint64_t{}}
{
    /* Initialize the time value for the first time */
    hasChanged();
}

FileWatcher::FileWatcher(FileWatcher&&)
    #ifdef __GNUC__
    noexcept(std::is_nothrow_move_constructible<std::string>::value)
    #else
    noexcept
    #endif
    = default;

FileWatcher::~FileWatcher() = default;

FileWatcher::Flags FileWatcher::flags() const {
    return Flag(std::uint8_t(_flags & ~InternalFlag::Valid));
}

bool FileWatcher::isValid() const {
    return _flags >= InternalFlag::Valid;
}

FileWatcher& FileWatcher::operator=(FileWatcher&&)
    /* See the header for details */
    #ifdef __GNUC__
    noexcept(std::is_nothrow_move_assignable<std::string>::value)
    #else
    noexcept
    #endif
    = default;

bool FileWatcher::hasChanged() {
    if(!(_flags & InternalFlag::Valid)) return false;

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* GCC 4.8 complains about missing initializers if {} is used. The struct
       is initialized by stat() anyway so it's okay to keep it uninitialized */
    struct stat result;
    if(stat(_filename.data(), &result) != 0)
    #elif defined(CORRADE_TARGET_WINDOWS)
    struct _stat result;
    if(_wstat(_filename.data(), &result) != 0)
    #else
    #error
    #endif
    {
        Error err;
        err << "Utility::FileWatcher: can't stat"
            #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
            << _filename
            #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
            << Unicode::narrow(_filename)
            #else
            #error
            #endif
            << Debug::nospace << ":" << std::strerror(errno) << Debug::nospace;

        /* Ignore the error if we are told so (but still warn) */
        if(_flags & InternalFlag::IgnoreErrors) {
            err << ", ignoring";
            return false;
        }

        err << ", aborting watch";
        _flags &= ~InternalFlag::Valid;
        return false;
    }

    /* Linux (and Android) has st_mtim (and st_mtime is a preprocessor alias to
       st_mtim.tv_sec), which offers nanosecond precision (though the actual
       granularity is ~10s of ms). macOS has the same in an (arguably
       nonstandard) st_mtimespec, but HFS+ has only second precision anyway:
       https://developer.apple.com/library/archive/technotes/tn/tn1150.html#HFSPlusDates
       Emscripten defines st_mtime but sets tv_nsec to zero:
       https://github.com/kripken/emscripten/blob/52ff847187ee30fba48d611e64b5d10e2498fe0f/src/library_syscall.js#L66
       Windows doesn't have either, we get seconds there at best. */
    const std::uint64_t time =
        #ifdef CORRADE_TARGET_APPLE
        std::uint64_t(result.st_mtimespec.tv_sec)*1000000000 + std::uint64_t(result.st_mtimespec.tv_nsec)
        #elif defined(st_mtime)
        std::uint64_t(result.st_mtim.tv_sec)*1000000000 + std::uint64_t(result.st_mtim.tv_nsec)
        #else
        std::uint64_t(result.st_mtime)*1000000000
        #endif
        ;

    /* Checking for the first time, report no change */
    if(_time == ~std::uint64_t{}) {
        _time = time;
        return false;
    }

    /* Modification time changed, update and report change -- unless the size
       is zero and we're told to ignore those */
    if(_time != time
        #ifndef CORRADE_TARGET_IOS
        /* iOS (or at least the simulator) reports all sizes to be always 0,
           which means this flag would make FileWatcher absolutely useless. So
           ignore it there. */
        && (!(_flags & InternalFlag::IgnoreChangeIfEmpty) || result.st_size != 0)
        #endif
    ) {
        _time = time;
        return true;
    }

    return false;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
Debug& operator<<(Debug& debug, FileWatcher::Flag value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case FileWatcher::Flag::value: return debug << "Utility::FileWatcher::Flag::" #value;
        _c(IgnoreErrors)
        _c(IgnoreChangeIfEmpty)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "Utility::FileWatcher::Flag(" << Debug::nospace << reinterpret_cast<void*>(std::uint8_t(value)) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, FileWatcher::Flags value) {
    return Containers::enumSetDebugOutput(debug, value, "Utility::FileWatcher::Flags{}", {
        FileWatcher::Flag::IgnoreErrors,
        FileWatcher::Flag::IgnoreChangeIfEmpty});
}
#endif

}}
