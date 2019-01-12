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

#include "FileWatcher.h"

#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

#include "Corrade/Utility/DebugStl.h"

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Unicode.h"
#endif

namespace Corrade { namespace Utility {

FileWatcher::FileWatcher(const std::string& filename):
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    _filename{filename},
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    _filename{Unicode::widen(filename)},
    #else
    #error
    #endif
    _time{~std::uint64_t{}}
{
    /* Initialize the time value for the first time */
    hasChanged();
}

FileWatcher::FileWatcher(FileWatcher&&) noexcept = default;

FileWatcher::~FileWatcher() = default;

FileWatcher& FileWatcher::operator=(FileWatcher&&)
    /* See the header for details */
    #ifdef __GNUC__
    noexcept(std::is_nothrow_move_assignable<std::string>::value)
    #else
    noexcept
    #endif
    = default;

bool FileWatcher::hasChanged() {
    if(!_valid) return false;

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
        Error{} << "Utility::FileWatcher: can't stat"
            #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
            << _filename
            #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
            << Unicode::narrow(_filename)
            #else
            #error
            #endif
            << Debug::nospace << ":" << std::strerror(errno) << Debug::nospace << ", aborting watch";
        _valid = false;
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

    /* Modification time changed, update and report change */
    if(_time != time) {
        _time = time;
        return true;
    }

    return false;
}

}}
