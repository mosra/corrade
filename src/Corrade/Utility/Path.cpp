/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019, 2020 Jonathan Hale <squareys@googlemail.com>

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

#include "Corrade/configure.h"

/* Requests 64bit file offset on Linux. Has to be done before anything else is
   included, since we can't be sure that <string> doesn't include anything that
   this macro would affect. */
#ifdef CORRADE_TARGET_UNIX
#define _FILE_OFFSET_BITS 64
#endif

/* Otherwise _wrename() and _wremove() is not defined on TDM-GCC 5.1. This has
   to be undefined before including any other header or it doesn't work. */
#ifdef __MINGW32__
#undef __STRICT_ANSI__
#endif

#include "Path.h"

#include <cstdio> /* std::fopen(), FILE, std::remove(), std::rename() */
#include <algorithm> /* std::sort() */

/* Checking for API level on Android */
#ifdef CORRADE_TARGET_ANDROID
#include <android/api-level.h>
#endif

/* Unix memory mapping, library location */
#ifdef CORRADE_TARGET_UNIX
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h> /* dladdr(), needs also -ldl */
#endif

/* Unix, Emscripten file & directory access */
#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include <cerrno>
#include <cstdlib> /* std::getenv() */
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#ifdef CORRADE_TARGET_APPLE
#include <mach-o/dyld.h>
#endif
#endif

/* Windows */
/** @todo remove the superfluous includes when mingw is fixed (otherwise causes undefined EXTERN_C error) */
#ifdef CORRADE_TARGET_WINDOWS
#ifdef __MINGW32__
#include <wtypes.h>
#include <windef.h>
#include <wincrypt.h>
#include <ntdef.h>
#include <basetyps.h>
#endif
#include <shlobj.h>
#include <io.h>
#endif

#include "Corrade/configure.h"
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Utility/Debug.h"

#if defined(__unix__) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include "Corrade/Utility/String.h" /* lowercase() */
#endif

#ifdef CORRADE_TARGET_APPLE
#include "Corrade/Utility/System.h" /* isSandboxed() */
#endif

/* errno and GetLastError() stringifiers */
#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS)
#include "Corrade/Utility/Implementation/ErrorString.h"
#endif

/* Unicode helpers for Windows */
#ifdef CORRADE_TARGET_WINDOWS
#include "Corrade/Utility/Unicode.h"
#endif

namespace Corrade { namespace Utility { namespace Path {

using namespace Containers::Literals;

#ifdef CORRADE_TARGET_WINDOWS
Containers::String fromNativeSeparators(Containers::String path) {
    /* In the rare scenario where we'd get a non-owned string (such as
       String::nullTerminatedView() passed right into the function), make it
       owned first. Usually it'll get copied however, which already makes it
       owned. */
    if(!path.isSmall() && path.deleter()) path = Containers::String{path};

    /* Since this replaces just single bytes, I don't think we need any fancy
       library function to do the job */
    for(char& c: path) if(c == '\\') c = '/';
    return path;
}
#else
Containers::StringView fromNativeSeparators(const Containers::StringView path) {
    return path;
}
#endif

#ifdef CORRADE_TARGET_WINDOWS
Containers::String toNativeSeparators(Containers::String path) {
    /* In the rare scenario where we'd get a non-owned string (such as
       String::nullTerminatedView() passed right into the function), make it
       owned first. Usually it'll get copied however, which already makes it
       owned. */
    if(!path.isSmall() && path.deleter()) path = Containers::String{path};

    /* Since this replaces just single bytes, I don't think we need any fancy
       library function to do the job */
    for(char& c: path) if(c == '/') c = '\\';
    return path;
}
#else
Containers::StringView toNativeSeparators(const Containers::StringView path) {
    return path;
}
#endif

Containers::Pair<Containers::StringView, Containers::StringView> split(const Containers::StringView path) {
    const Containers::StringView found = path.findLastOr('/', path.begin());

    /* Strip the trailing / from path unless it's the root */
    const Containers::StringView head = path.prefix(found.end());
    return {path.prefix(head == "/"_s || head == "//"_s ? found.end() : found.begin()), path.suffix(found.end())};
}

Containers::Pair<Containers::StringView, Containers::StringView> splitExtension(const Containers::StringView path) {
    /* Take the suffix after the last slash as the filename, or the whole path
       if it's not there */
    const Containers::StringView filename = path.suffix(path.findLastOr('/', path.begin()).end());

    /* Find the last dot in the filename, or nothing if it's not there */
    const Containers::StringView foundDot = filename.findLastOr('.', filename.end());

    /* As a special case, if there's dots are at the start of the filename
       (/root/.bashrc, dir/..), it's also an empty extension */
    if(foundDot) {
        bool initialDots = true;
        for(char i: filename.prefix(foundDot.begin())) if(i != '.') {
            initialDots = false;
            break;
        }
        if(initialDots)
            return {path.prefix(filename.end()), path.suffix(filename.end())};
    }

    return {path.prefix(foundDot.begin()), path.suffix(foundDot.begin())};
}

Containers::String join(Containers::StringView path, const Containers::StringView filename) {
    if(
        /* Empty path */
        !path ||

        /* Absolute filename */
        filename.hasPrefix('/')

        #ifdef CORRADE_TARGET_WINDOWS
        /* Absolute filename on Windows */
        || (filename.size() > 2 && filename[1] == ':' && filename[2] == '/')
        #endif
    )
        return filename;

    /* Join with a slash in between. If it's already there, slice it away first
       so we have uniform handling. */
    if(path.hasSuffix('/')) path = path.exceptSuffix(1);
    return "/"_s.join({path, filename});
}

Containers::String join(const Containers::ArrayView<const Containers::StringView> paths) {
    if(paths.isEmpty()) return {};

    /** @todo once growable strings are a thing, do this in a loop instead of
        recursing and allocating once for every item! One possibility would be
        to find earliest absolute path (or begin, if none), strip trailing /
        from all paths after, and then "/"_s.join() everything. Unfortunately
        stripping of trailing /s would mean we have to store the modified views
        somewhere, which is an additional allocation. Wait with that until
        allocators with fixed memory are a thing? */
    Containers::String path = paths.front();
    for(std::size_t i = 1; i != paths.size(); ++i)
        path = join(path, paths[i]);

    return path;
}

Containers::String join(const std::initializer_list<Containers::StringView> paths) {
    return join(Containers::arrayView(paths));
}

bool exists(const Containers::StringView filename) {
    /* Sane platforms */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    return access(Containers::String::nullTerminatedView(filename).data(), F_OK) == 0;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    return GetFileAttributesW(Unicode::widen(filename)) != INVALID_FILE_ATTRIBUTES;

    /* Windows Store/Phone not implemented */
    #else
    static_cast<void>(filename);
    Error{} << "Utility::Path::exists(): not implemented on this platform";
    return false;
    #endif
}

namespace {

#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
/* Used by fileSize(), read(), copy() source and mapRead() to prevent really
   bad issues. For directories lseek() returns 9223372036854775807 (2^63 - 1,
   and thus causing an allocation failure) or maybe also 0 (and thus a silent
   error); fread() will always read 0 bytes no matter what lseek() reports.
   Such behavior is everything but useful. The lseek() value is also
   undocumented, so we can't just check the value to know we opened a
   directory: https://stackoverflow.com/a/65912203

   Thus a directory is explicitly checked on the file descriptor, and if it is,
   we fail. This doesn't need to be done when opening for writing (so write(),
   append(), copy() destination or map() / mapWrite), there the opening itself fails already. On Windows, opening directories fails in any case, so there
   we don't need to do anything either. */
bool isDirectory(const int fd) {
    struct stat st;
    return fstat(fd, &st) == 0 && S_ISDIR(st.st_mode);
}
#endif

}

bool isDirectory(const Containers::StringView path) {
    /* Compared to the internal isDirectory(std::FILE*) above, this calls the
       OS APIs directly with the filename and should be atomic and faster
       compared to first opening the file and then asking for attributes */

    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /** @todo symlink support */
    const DWORD fileAttributes = GetFileAttributesW(Unicode::widen(path));
    return fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    #elif defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* using stat() instead of lstat() as that follows symlinks and that's what
       is desired in most cases */
    struct stat st;
    return stat(Containers::String::nullTerminatedView(path).data(), &st) == 0 && S_ISDIR(st.st_mode);
    #else
    static_cast<void>(path);
    Error{} << "Utility::Path::isDirectory(): not implemented on this platform";
    return false;
    #endif
}

bool make(const Containers::StringView path) {
    if(!path) return true;

    /* If path contains trailing slash, strip it */
    if(path.hasSuffix('/'))
        return make(path.exceptSuffix(1));

    /* If parent directory doesn't exist, create it. That means two syscalls to
       create each parent (and two UTF-16 conversions on Windows). I could also
       directly call into make() without checking exists() first, relying on
       mkdir() failing with EEXIST instead -- while that would save one syscall
       per path component that doesn't exist, for long paths that already exist
       (which is supposedly the more common scenario) it would mean mkdir()
       gets called once for each component instead of just one existence check
       for the parent and one mkdir() for the leaf directory. */
    const Containers::StringView parentPath = split(path).first();
    if(parentPath && parentPath != "/"_s && !exists(parentPath) && !make(parentPath))
        return false;

    /* Create directory, return true if successfully created or already exists */

    /* Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const int ret = mkdir(Containers::String::nullTerminatedView(path).data(), 0777);
    if(ret != 0 && errno != EEXIST) {
        Error err;
        err << "Utility::Path::make(): can't create" << path << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }
    return true;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    if(CreateDirectoryW(Unicode::widen(path), nullptr) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
        Error err;
        err << "Utility::Path::make(): can't create" << path << Debug::nospace << ":";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return false;
    }
    return true;

    /* Not implemented elsewhere */
    #else
    Error{} << "Utility::Path::make(): not implemented on this platform";
    return false;
    #endif
}

bool remove(const Containers::StringView path) {
    /* Windows need special handling for Unicode, otherwise we can work with
       the standard std::remove() */
    #ifdef CORRADE_TARGET_WINDOWS
    Containers::Array<wchar_t> wpath = Unicode::widen(path);

    /* std::remove() can't remove directories on Windows */
    /** @todo how to implement this for RT? */
    #ifndef CORRADE_TARGET_WINDOWS_RT
    /* INVALID_FILE_ATTRIBUTES contain the FILE_ATTRIBUTE_DIRECTORY bit for
       some reason -- so if wouldn't check for INVALID_FILE_ATTRIBUTES,
       nonexistent files would be reported as "can't remove directory". */
    const DWORD attributes = GetFileAttributesW(wpath.data());
    if(attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if(!RemoveDirectoryW(wpath.data())) {
            Error err;
            err << "Utility::Path::remove(): can't remove directory" << path << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            return false;
        }

        return true;
    }
    #endif

    /* Need to use nonstandard _wremove in order to handle Unicode properly */
    if(_wremove(wpath.data()) != 0) {
        Error err;
        err << "Utility::Path::remove(): can't remove" << path << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    return true;
    #else
    /* std::remove() can't remove directories on Emscripten */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    struct stat st;
    /* using lstat() and not stat() as we care about the symlink, not the
       file/dir it points to */
    if(lstat(Containers::String::nullTerminatedView(path).data(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if(rmdir(Containers::String::nullTerminatedView(path).data()) != 0) {
            Error err;
            err << "Utility::Path::remove(): can't remove directory" << path << Debug::nospace << ":";
            Utility::Implementation::printErrnoErrorString(err, errno);
            return false;
        }

        return true;
    }
    #endif

    if(std::remove(Containers::String::nullTerminatedView(path).data()) != 0) {
        Error err;
        err << "Utility::Path::remove(): can't remove" << path << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    return true;
    #endif
}

bool move(Containers::StringView from, Containers::StringView to) {
    if(
        #ifndef CORRADE_TARGET_WINDOWS
        std::rename(Containers::String::nullTerminatedView(from).data(),
                    Containers::String::nullTerminatedView(to).data())
        #else
        _wrename(Unicode::widen(from), Unicode::widen(to))
        #endif
    != 0) {
        Error err;
        err << "Utility::Path::move(): can't move" << from << "to" << to << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    return true;
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
Containers::Optional<Containers::String> libraryLocation(const void* address) {
    /* Linux (and macOS as well, even though Linux man pages don't mention that) */
    #ifdef CORRADE_TARGET_UNIX
    /* Otherwise GCC 4.8 loudly complains about missing initializers */
    Dl_info info{nullptr, nullptr, nullptr, nullptr};
    if(!dladdr(address, &info)) {
        Error{} << "Utility::Path::libraryLocation(): can't get library location";
        /* According to manpages, the dlerror is *never* available, so just
           assert on that instead of branching */
        CORRADE_INTERNAL_ASSERT(!dlerror());
        return {};
    }

    return Containers::String{info.dli_fname};
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    HMODULE module{};
    if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<const char*>(address), &module)) {
        Error err;
        err << "Utility::Path::libraryLocation(): can't get library location:";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return {};
    }

    /** @todo get rid of MAX_PATH */
    wchar_t path[MAX_PATH + 1];
    /* Returns size *without* the null terminator */
    const std::size_t size = GetModuleFileNameW(module, path, Containers::arraySize(path));
    return fromNativeSeparators(Unicode::narrow(Containers::arrayView(path, size)));
    #endif
}

#ifndef DOXYGEN_GENERATING_OUTPUT
Containers::Optional<Containers::String> libraryLocation(Implementation::FunctionPointer address) {
    return libraryLocation(address.address);
}
#endif
#endif

Containers::Optional<Containers::String> executableLocation() {
    /* Linux */
    #if defined(__linux__)
    /* Reallocate like hell until we have enough place to store the path. Can't
       use lstat because the /proc/self/exe symlink is not a real symlink and
       so stat::st_size returns 0. POSIX, WHAT THE HELL. */
    constexpr const char self[]{"/proc/self/exe"};
    /** @todo use a String when it can grow on its own */
    Containers::Array<char> path;
    arrayResize(path, NoInit, 4);
    ssize_t size;
    while((size = readlink(self, path, path.size())) == ssize_t(path.size()))
        arrayResize(path, NoInit, path.size()*2);

    /* readlink() doesn't put the null terminator into the array, do it
       ourselves. The above loop guarantees that path.size() is always larger
       than size -- if it would be equal, we'd try once more with a larger
       buffer */
    CORRADE_INTERNAL_ASSERT(size && std::size_t(size) < path.size());
    path[size] = '\0';
    const auto deleter = path.deleter();
    return Containers::String{path.release(), std::size_t(size), deleter};

    /* OSX, iOS */
    #elif defined(CORRADE_TARGET_APPLE)
    /* Get path size (need to set it to 0 to avoid filling nullptr with random
       data and crashing, HAHA) */
    std::uint32_t size = 0;
    CORRADE_INTERNAL_ASSERT_OUTPUT(_NSGetExecutablePath(nullptr, &size) == -1);

    /* Allocate proper size and get the path. The size includes a null
       terminator which the String handles on its own, so subtract it */
    CORRADE_INTERNAL_ASSERT(size);
    Containers::String path{NoInit, size - 1};
    CORRADE_INTERNAL_ASSERT_OUTPUT(_NSGetExecutablePath(path.data(), &size) == 0);
    return path;

    /* Windows (not RT) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /** @todo get rid of MAX_PATH */
    wchar_t path[MAX_PATH + 1];
    /* Returns size *without* the null terminator */
    const std::size_t size = GetModuleFileNameW(nullptr, path, Containers::arraySize(path));
    return fromNativeSeparators(Unicode::narrow(Containers::arrayView(path, size)));

    /* hardcoded for Emscripten */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    return Containers::String{"/app.js"_s};

    /* Not implemented */
    #else
    Error{} << "Utility::Path::executableLocation(): not implemented on this platform";
    return {};
    #endif
}

Containers::Optional<Containers::String> currentDirectory() {
    /* POSIX. Needs a shitty loop because ... ugh. */
    #ifdef CORRADE_TARGET_UNIX
    /** @todo use a String when it can grow on its own, and then call getcwd()
        with path.size() + 1 again */
    Containers::Array<char> path;
    arrayResize(path, NoInit, 4);
    char* success;
    while(!(success = getcwd(path, path.size()))) {
        /* Unexpected error, exit. Can be for example ENOENT when current
           working directory gets deleted while the program is running. */
        if(errno != ERANGE) {
            Error err;
            err << "Utility::Path::currentDirectory():";
            Utility::Implementation::printErrnoErrorString(err, errno);
            return {};
        }

        /* Otherwise try again with larger buffer */
        arrayResize(path, NoInit, path.size()*2);
    }

    /* Success, transfer to a string with a growable deleter and an appropriate
       size, assuming getcwd() put the null terminator at the end */
    const auto deleter = path.deleter();
    const std::size_t size = std::strlen(path);
    CORRADE_INTERNAL_ASSERT(size < path.size());
    return Containers::String{path.release(), size, deleter};

    /* Windows (not RT) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* Querying without a buffer returns size with the null terminator ... */
    const std::size_t sizePlusOne = GetCurrentDirectoryW(0, nullptr);
    CORRADE_INTERNAL_ASSERT(sizePlusOne);
    Containers::Array<wchar_t> path{NoInit, sizePlusOne};
    /* ... but retrieving the data returns size without it */
    CORRADE_INTERNAL_ASSERT_OUTPUT(GetCurrentDirectoryW(sizePlusOne, path) == sizePlusOne - 1);
    return fromNativeSeparators(Unicode::narrow(path.exceptSuffix(1)));

    /* Use the root path on Emscripten */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    return Containers::String{"/"_s};

    /* No clue elsewhere (and on Windows RT) */
    #else
    Error{} << "Utility::Path::currentDirectory(): not implemented on this platform";
    return {};
    #endif
}

Containers::Optional<Containers::String> homeDirectory() {
    /* Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const char* const h = std::getenv("HOME");
    if(!h) {
        Error{} << "Utility::Path::homeDirectory(): $HOME not available";
        return {};
    }

    return Containers::String{h};

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /** @todo get rid of MAX_PATH */
    wchar_t h[MAX_PATH + 1];
    /* There doesn't seem to be any possibility how this could fail, so just
       assert */
    CORRADE_INTERNAL_ASSERT(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, h) == S_OK);
    return fromNativeSeparators(Unicode::narrow(h));

    /* Other */
    #else
    Error{} << "Utility::Path::homeDirectory(): not implemented on this platform";
    return {};
    #endif
}

Containers::Optional<Containers::String> configurationDirectory(const Containers::StringView applicationName) {
    /* OSX, iOS */
    #ifdef CORRADE_TARGET_APPLE
    /* Not delegating into homeDirectory() as the (admittedly rare) error
       message would have a confusing source */
    const char* const home = std::getenv("HOME");
    if(!home) {
        Error{} << "Utility::Path::configurationDirectory(): $HOME not available";
        return {};
    }

    return join({home, "Library/Application Support"_s, applicationName});

    /* XDG-compliant Unix (not using CORRADE_TARGET_UNIX, because that is a
       superset), Emscripten */
    #elif defined(__unix__) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const Containers::String lowercaseApplicationName = String::lowercase(applicationName);
    if(const char* const config = std::getenv("XDG_CONFIG_HOME"))
        return join(config, lowercaseApplicationName);

    /* Not delegating into homeDirectory() as the (admittedly rare) error
       message would have a confusing source */
    const char* const home = std::getenv("HOME");
    if(!home) {
        Error{} << "Utility::Path::configurationDirectory(): neither $XDG_CONFIG_HOME nor $HOME available";
        return {};
    }

    return join({home, ".config"_s, lowercaseApplicationName});

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /** @todo get rid of MAX_PATH */
    wchar_t path[MAX_PATH];
    /* There doesn't seem to be any possibility how this could fail, so just
       assert */
    CORRADE_INTERNAL_ASSERT(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path) == S_OK);
    if(path[0] == L'\0') {
        Error{} << "Utility::Path::configurationDirectory(): can't retrieve CSIDL_APPDATA";
        return {};
    }
    return join(fromNativeSeparators(Unicode::narrow(path)), applicationName);

    /* Other not implemented */
    #else
    static_cast<void>(applicationName);
    Error{} << "Utility::Path::configurationDirectory(): not implemented on this platform";
    return {};
    #endif
}

Containers::Optional<Containers::String> temporaryDirectory() {
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Sandboxed OSX, iOS */
    #ifdef CORRADE_TARGET_APPLE
    if(System::isSandboxed()) {
        /* Not delegating into homeDirectory() as the (admittedly rare) error
           message would have a confusing source */
        const char* const home = std::getenv("HOME");
        if(!home) {
            Error{} << "Utility::Path::temporaryDirectory(): $HOME not available";
            return {};
        }

        return join(home, "tmp"_s);
    }
    #endif

    /* Android, you had to be special, right? */
    #ifdef CORRADE_TARGET_ANDROID
    return Containers::String{"/data/local/tmp"_s};
    #endif

    /* Common Unix, Emscripten */
    return Containers::String{"/tmp"_s};

    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* Windows */

    /* Get path size. Includes a trailing slash and a zero terminator. */
    wchar_t c;
    const std::size_t size = GetTempPathW(1, &c);
    CORRADE_INTERNAL_ASSERT(size >= 2);

    /* Get the path, convert to forward slashes, strip the trailing slash and
       zero terminator */
    Containers::Array<wchar_t> path{NoInit, size};
    GetTempPathW(size, path);
    return fromNativeSeparators(Unicode::narrow(path.exceptSuffix(2)));
    #else
    Error{} << "Utility::Path::temporaryDirectory(): not implemented on this platform";
    return {};
    #endif
}

Containers::Optional<Containers::Array<Containers::String>> list(const Containers::StringView path, ListFlags flags) {
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
    /* POSIX-compliant Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    DIR* directory = opendir(Containers::String::nullTerminatedView(path).data());
    if(!directory) {
        Error err;
        err << "Utility::Path::list(): can't list" << path << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    Containers::Array<Containers::String> list;

    dirent* entry;
    /* readdir() should fail only if the directory pointer is invalid, which
       we already checked above -- so the error handling is done oly for stat()
       below */
    while((entry = readdir(directory)) != nullptr) {
        if((flags >= ListFlag::SkipDirectories) && entry->d_type == DT_DIR)
            continue;
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        if((flags >= ListFlag::SkipFiles) && entry->d_type == DT_REG)
            continue;
        if((flags >= ListFlag::SkipSpecial) && entry->d_type != DT_DIR && entry->d_type != DT_REG && entry->d_type != DT_LNK)
            continue;
        #else
        /* Emscripten doesn't set DT_REG for files, so we treat everything
           that's not a DT_DIR as a file. SkipSpecial has no effect here. */
        if(flags >= ListFlag::SkipFiles && entry->d_type != DT_DIR)
            continue;
        #endif

        /* For symlinks we have to deref the link and ask there again. If that
           fails for whatever reason, we leave the file in the list -- it can
           be thought of as "neither a file nor directory" and we're told to
           skip files/directories, not "include only files/directories".

           Also do this only if we're told to skip certain entry types, for a
           plain list this is unnecessary overhead. */
        if((flags & (ListFlag::SkipDirectories|ListFlag::SkipFiles|ListFlag::SkipSpecial)) && entry->d_type == DT_LNK) {
            /* stat() follows the symlink, lstat() doesn't */
            struct stat st;
            /** @todo once we figure out a way to test, handle errors from
                stat() (print warnings), add a to make those fail the whole
                operation instead of carrying on; same in glob() */
            if(stat((join(path, entry->d_name)).data(), &st) == 0) {
                if(flags >= ListFlag::SkipDirectories && S_ISDIR(st.st_mode))
                    continue;
                if(flags >= ListFlag::SkipFiles && S_ISREG(st.st_mode))
                    continue;
                if(flags >= ListFlag::SkipSpecial && !S_ISDIR(st.st_mode) && !S_ISREG(st.st_mode))
                    continue;
            }
        }

        const Containers::StringView file = entry->d_name;
        if((flags >= ListFlag::SkipDotAndDotDot) && (file == "."_s || file == ".."_s))
            continue;

        arrayAppend(list, file);
    }

    closedir(directory);

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    WIN32_FIND_DATAW data;
    /** @todo drop the StringView cast once widen(const std::string&) is
        removed */
    HANDLE hFile = FindFirstFileW(Unicode::widen(Containers::StringView{join(path, "*"_s)}), &data);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error err;
        err << "Utility::Path::list(): can't list" << path << Debug::nospace << ":";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return {};
    }
    Containers::ScopeGuard closeHandle{hFile,
        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        /* MSVC 2015 is unable to cast the parameter for FindClose */
        [](HANDLE hFile){ FindClose(hFile); }
        #else
        FindClose
        #endif
    };

    Containers::Array<Containers::String> list;

    /* Explicitly add `.` for compatibility with other systems */
    if(!(flags & (ListFlag::SkipDotAndDotDot|ListFlag::SkipDirectories)))
        arrayAppend(list, "."_s);

    while(FindNextFileW(hFile, &data) != 0 || GetLastError() != ERROR_NO_MORE_FILES) {
        if((flags >= ListFlag::SkipDirectories) && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        if((flags >= ListFlag::SkipFiles) && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        /** @todo symlink support */
        /** @todo are there any special files in WINAPI? */

        /* Not testing for dot, as it is not listed on Windows. Also it doesn't
           cause any unnecessary temporary allocation if SkipDotAndDotDot is
           used because `..` fits easily into SSO. */
        Containers::String file = Unicode::narrow(data.cFileName);
        if((flags >= ListFlag::SkipDotAndDotDot) && file == ".."_s)
            continue;

        arrayAppend(list, std::move(file));
    }
    #else
    #error
    #endif

    if(flags & (ListFlag::SortAscending|ListFlag::SortDescending))
        std::sort(list.begin(), list.end());
    /* We don't have rbegin() / rend() on Array (would require a custom
       iterator class or a StridedArrayView), so just reverse the result */
    if(flags >= ListFlag::SortDescending && !(flags >= ListFlag::SortAscending))
        std::reverse(list.begin(), list.end());

    /* GCC 4.8 and Clang 3.8 need extra help here */
    return Containers::optional(std::move(list));

    /* Other not implemented */
    #else
    Error{} << "Utility::Path::list(): not implemented on this platform";
    static_cast<void>(path);
    return {};
    #endif
}

namespace {

/* Used by size() and read(). Returns NullOpt if the file is not seekable
   (as file existence is already checked when opening the FILE*). */
Containers::Optional<std::size_t> size(std::FILE* const f) {
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS)
    /* If the file is not seekable, return NullOpt. On POSIX this is usually
       -1 when the file is non-seekable: https://stackoverflow.com/q/3238788
       It's undefined behavior on MSVC, tho (but possibly not on MinGW?):
       https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/lseek-lseeki64 */
    /** @todo find a reliable way on Windows */
    if(
        #ifndef CORRADE_TARGET_WINDOWS
        lseek(fileno(f), 0, SEEK_END) == -1
        #else
        _lseek(_fileno(f), 0, SEEK_END) == -1
        #endif
    ) return {};
    #else
    /** @todo implementation for non-seekable platforms elsewhere? */
    #endif

    std::fseek(f, 0, SEEK_END);
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const std::size_t size =
        /* 32-bit Android ignores _LARGEFILE_SOURCE and instead makes ftello()
           available always after API level 24 and never before that
           https://android.googlesource.com/platform/bionic/+/master/docs/32-bit-abi.md */
        #if defined(CORRADE_TARGET_ANDROID) && __SIZEOF_POINTER__ == 4 && __ANDROID_API__ < 24
        ftell(f)
        #else
        ftello(f)
        #endif
        ;
    #elif defined(CORRADE_TARGET_WINDOWS)
    const std::size_t size = _ftelli64(f);
    #else
    const std::size_t size = std::ftell(f);
    #endif

    /* Put the file handle back to its original state */
    std::rewind(f);

    return size;
}

}

Containers::Optional<std::size_t> size(const Containers::StringView filename) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(Containers::String::nullTerminatedView(filename).data(), "rb");
    #else
    std::FILE* const f = _wfopen(Unicode::widen(filename), L"rb");
    #endif
    if(!f) {
        Error err;
        err << "Utility::Path::size(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    Containers::ScopeGuard exit{f, std::fclose};

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Explicitly fail if opening directories for reading on Unix to prevent
       silent errors, see isDirectory(int) for details. On Windows the fopen()
       fails already. */
    if(isDirectory(fileno(f))) {
        Error{} << "Utility::Path::size():" << filename << "is a directory";
        return {};
    }
    #endif

    Containers::Optional<std::size_t> size_ = size(f);
    if(!size_)
        Error{} << "Utility::Path::size():" << filename << "is not seekable";
    return size_;
}

namespace {

/* The extra is set to 0 by read() and to 1 by readString(), to use for a null
   terminator */
Containers::Optional<Containers::Array<char>> readInternal(const Containers::StringView filename, std::size_t extra) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(Containers::String::nullTerminatedView(filename).data(), "rb");
    #else
    std::FILE* const f = _wfopen(Unicode::widen(filename), L"rb");
    #endif
    if(!f) {
        Error err;
        err << "Utility::Path::read(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    Containers::ScopeGuard exit{f, std::fclose};

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Explicitly fail if opening directories for reading on Unix to prevent
       allocation failures or silent errors, see isDirectory(int) for details.
       On Windows the fopen() fails already. */
    if(isDirectory(fileno(f))) {
        Error{} << "Utility::Path::read():" << filename << "is a directory";
        return {};
    }
    #endif

    Containers::Optional<std::size_t> size_ = size(f);

    /* If the file is not seekable, read it in chunks. Read directly into the
       output array --- each time add uninitialized 4 kB at the end, pass that
       view to fread(), and then strip away what didn't get filled. Hey, do you
       like the growable array API as much as I do? :D */
    /** @todo that the loop works is only verifiable by setting the condition
        to if(true) and setting chunkSize to 1 -- I don't know about any large
        non-seekable file that could be used for this */
    if(!size_) {
        Containers::Array<char> out;
        constexpr std::size_t chunkSize = 4096;

        std::size_t count;
        do {
            count = std::fread(arrayAppend(out, NoInit, chunkSize + extra), 1, chunkSize, f);
            arrayRemoveSuffix(out, chunkSize + extra - count);
        } while(count);

        /* GCC 4.8 and Clang 3.8 need extra help here */
        return Containers::optional(std::move(out));
    }

    /* Some special files report more bytes than they actually have (such as
       stuff in /sys). Clamp the returned array to what was reported. */
    Containers::Array<char> out{NoInit, *size_ + extra};
    const std::size_t realSize = std::fread(out, 1, *size_, f);
    CORRADE_INTERNAL_ASSERT(realSize <= *size_);
    return Containers::Array<char>{out.release(), realSize};
}

}

Containers::Optional<Containers::Array<char>> read(const Containers::StringView filename) {
    return readInternal(filename, 0);
}

Containers::Optional<Containers::String> readString(const Containers::StringView filename) {
    if(Containers::Optional<Containers::Array<char>> data = readInternal(filename, 1)) {
        const std::size_t size = data->size();
        const auto deleter = data->deleter();

        /* If the array is growable because we have read a non-seekable file
           and ASan is active, touching anything after the size will trigger an
           ASan report because we marked the area between size and capacity as
           untouchable "container overflow". However, touching the null
           terminator is generally desirable so it shouldn't cause ASan
           failures. Thus we first resize it to include the null terminator,
           which will update ASan container annotations. */
        if(arrayIsGrowable(*data)) arrayResize(*data, NoInit, size + 1);

        /* Now it's safe to set the null terminator. In case the array is not
           growable, the allocation doesn't have any ASan annotations, so it's
           fine. */
        (*data)[size] = '\0';

        /* For the returned String we'll always use the original size,
           excluding ignoring the size change from arrayResize(). */
        return Containers::String{data->release(), size, deleter};
    }

    return {};
}

bool write(const Containers::StringView filename, const Containers::ArrayView<const void> data) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(Containers::String::nullTerminatedView(filename).data(), "wb");
    #else
    std::FILE* const f = _wfopen(Unicode::widen(filename), L"wb");
    #endif
    if(!f) {
        Error err;
        err << "Utility::Path::write(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    Containers::ScopeGuard exit{f, std::fclose};

    std::fwrite(data, 1, data.size(), f);
    return true;
}

bool append(const Containers::StringView filename, const Containers::ArrayView<const void> data) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(Containers::String::nullTerminatedView(filename).data(), "ab");
    #else
    std::FILE* const f = _wfopen(Unicode::widen(filename), L"ab");
    #endif
    if(!f) {
        Error err;
        err << "Utility::Path::append(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    Containers::ScopeGuard exit{f, std::fclose};

    std::fwrite(data, 1, data.size(), f);
    return true;
}

bool copy(const Containers::StringView from, const Containers::StringView to) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const in = std::fopen(Containers::String::nullTerminatedView(from).data(), "rb");
    #else
    std::FILE* const in = _wfopen(Unicode::widen(from), L"rb");
    #endif
    if(!in) {
        Error err;
        err << "Utility::Path::copy(): can't open" << from << "for reading:";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    Containers::ScopeGuard exitIn{in, std::fclose};

    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Explicitly fail if opening directories for reading on Unix to prevent
       silent errors, see isDirectory(int) for details. On Windows the fopen()
       fails already. */
    if(isDirectory(fileno(in))) {
        Error{} << "Utility::Path::copy(): can't read from" << from << "which is a directory";
        return {};
    }
    #endif

    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const out = std::fopen(Containers::String::nullTerminatedView(to).data(), "wb");
    #else
    std::FILE* const out = _wfopen(Unicode::widen(to), L"wb");
    #endif
    if(!out) {
        Error err;
        err << "Utility::Path::copy(): can't open" << to << "for writing:";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return false;
    }

    Containers::ScopeGuard exitOut{out, std::fclose};

    #if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
    /* As noted in https://eklitzke.org/efficient-file-copying-on-linux, might
       make the file reading faster. Didn't make any difference in the 100 MB
       benchmark on my ultra-fast SSD, though. */
    posix_fadvise(fileno(in), 0, 0, POSIX_FADV_SEQUENTIAL);
    #endif

    /* 128 kB: https://eklitzke.org/efficient-file-copying-on-linux. The 100 MB
       benchmark agrees, going below is significantly slower and going above is
       not any faster. */
    /** @todo investigate if alignas(32) would make any practical difference
        on any system (on glibc, fwrite() calls into mempcpy_avx_unaligned
        always, regardless of the alignment) */
    char buffer[128*1024];
    std::size_t count;
    do {
        count = std::fread(buffer, 1, Containers::arraySize(buffer), in);
        std::fwrite(buffer, 1, count, out);
    } while(count);

    return true;
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
void MapDeleter::operator()(const char* const data, const std::size_t size) {
    #ifdef CORRADE_TARGET_UNIX
    if(data && munmap(const_cast<char*>(data), size) == -1)
        Error() << "Utility::Path: can't unmap memory-mapped file";
    if(_fd) close(_fd);
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    if(data) UnmapViewOfFile(data);
    if(_hMap) CloseHandle(_hMap);
    if(_hFile) CloseHandle(_hFile);
    static_cast<void>(size);
    #endif
}

Containers::Optional<Containers::Array<char, MapDeleter>> map(const Containers::StringView filename) {
    #ifdef CORRADE_TARGET_UNIX
    /* Open the file for reading */
    const int fd = open(Containers::String::nullTerminatedView(filename).data(), O_RDWR);
    if(fd == -1) {
        Error err;
        err << "Utility::Path::map(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    /* Get file size */
    const off_t currentPos = lseek(fd, 0, SEEK_CUR);
    const std::size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, currentPos, SEEK_SET);

    /* Map the file. Can't call mmap() with a zero size, so if the file is
       empty just set the pointer to null -- but for consistency keep the fd
       open and let it be handled by the deleter. Array guarantees that deleter
       gets called even in case of a null data. */
    char* data;
    if(!size) data = nullptr;
    else if((data = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))) == MAP_FAILED) {
        Error err;
        err << "Utility::Path::map(): can't map" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        close(fd);
        return {};
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{fd}};
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    HANDLE hFile = CreateFileW(Unicode::widen(filename),
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error err;
        err << "Utility::Path::map(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return {};
    }

    /* Get file size */
    const std::size_t size = GetFileSize(hFile, nullptr);

    /* Can't call CreateFileMapping() with a zero size, so if the file is empty
       just set the pointer to null -- but for consistency keep the handle open
       and let it be handled by the deleter. Array guarantees that deleter gets
       called even in case of a null data. */
    HANDLE hMap;
    char* data;
    if(!size) {
        hMap = {};
        data = nullptr;
    } else {
        /* Create the file mapping */
        if(!(hMap = CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr))) {
            Error err;
            err << "Utility::Path::map(): can't create file mapping for" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hFile);
            return {};
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)))) {
            Error err;
            err << "Utility::Path::map(): can't map" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hMap);
            CloseHandle(hFile);
            return {};
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
    #endif
}

Containers::Optional<Containers::Array<const char, MapDeleter>> mapRead(const Containers::StringView filename) {
    #ifdef CORRADE_TARGET_UNIX
    /* Open the file for reading */
    const int fd = open(Containers::String::nullTerminatedView(filename).data(), O_RDONLY);
    if(fd == -1) {
        Error err;
        err << "Utility::Path::mapRead(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    /* Explicitly fail if opening directories for reading on Unix to prevent
       silent errors, see isDirectory(int) for details */
    if(isDirectory(fd)) {
        Error{} << "Utility::Path::mapRead():" << filename << "is a directory";
        return {};
    }

    /* Get file size */
    const off_t currentPos = lseek(fd, 0, SEEK_CUR);
    const std::size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, currentPos, SEEK_SET);

    /* Map the file. Can't call mmap() with a zero size, so if the file is
       empty just set the pointer to null -- but for consistency keep the fd
       open and let it be handled by the deleter. Array guarantees that deleter
       gets called even in case of a null data. */
    const char* data;
    if(!size) data = nullptr;
    else if((data = reinterpret_cast<const char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0))) == MAP_FAILED) {
        Error err;
        err << "Utility::Path::mapRead(): can't map" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        close(fd);
        return {};
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{fd}};
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* Open the file for reading */
    HANDLE hFile = CreateFileW(Unicode::widen(filename),
        GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error err;
        err << "Utility::Path::mapRead(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return {};
    }

    /* Get file size */
    const std::size_t size = GetFileSize(hFile, nullptr);

    /* Can't call CreateFileMapping() with a zero size, so if the file is empty
       just set the pointer to null -- but for consistency keep the handle open
       and let it be handled by the deleter. Array guarantees that deleter gets
       called even in case of a null data. */
    HANDLE hMap;
    char* data;
    if(!size) {
        hMap = {};
        data = nullptr;
    } else {
        /* Create the file mapping */
        if(!(hMap = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr))) {
            Error err;
            err << "Utility::Path::mapRead(): can't create file mapping for" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hFile);
            return {};
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)))) {
            Error err;
            err << "Utility::Path::mapRead(): can't map" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hMap);
            CloseHandle(hFile);
            return {};
        }
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
    #endif
}

Containers::Optional<Containers::Array<char, MapDeleter>> mapWrite(const Containers::StringView filename, const std::size_t size) {
    #ifdef CORRADE_TARGET_UNIX
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    const int fd = open(filename.data(), O_RDWR|O_CREAT|O_TRUNC, mode_t(0600));
    if(fd == -1) {
        Error err;
        err << "Utility::Path::mapWrite(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printErrnoErrorString(err, errno);
        return {};
    }

    /* Can't seek, write or mmap() with a zero size, so if the file is empty
       just set the pointer to null -- but for consistency keep the fd open and
       let it be handled by the deleter. Array guarantees that deleter gets
       called even in case of a null data. */
    char* data;
    if(!size) {
         data = nullptr;
    } else {
        /* Resize the file to requested size by seeking one byte before */
        if(lseek(fd, size - 1, SEEK_SET) == -1) {
            Error err;
            err << "Utility::Path::mapWrite(): can't seek to resize" << filename << Debug::nospace << ":";
            Utility::Implementation::printErrnoErrorString(err, errno);
            close(fd);
            return {};
        }

        /* And then writing a zero byte on that position */
        if(::write(fd, "", 1) != 1) {
            Error err;
            err << "Utility::Path::mapWrite(): can't write to resize" << filename << Debug::nospace << ":";
            Utility::Implementation::printErrnoErrorString(err, errno);
            close(fd);
            return {};
        }

        /* Map the file */
        if((data = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))) == MAP_FAILED) {
            Error err;
            err << "Utility::Path::mapWrite(): can't map" << filename << Debug::nospace << ":";
            Utility::Implementation::printErrnoErrorString(err, errno);
            close(fd);
            return {};
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{fd}};
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    HANDLE hFile = CreateFileW(Unicode::widen(filename),
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error err;
        err << "Utility::Path::mapWrite(): can't open" << filename << Debug::nospace << ":";
        Utility::Implementation::printWindowsErrorString(err, GetLastError());
        return {};
    }

    /* Can't call CreateFileMapping() with a zero size, so if the file is empty
       just set the pointer to null -- but for consistency keep the handle open
       and let it be handled by the deleter. Array guarantees that deleter gets
       called even in case of a null data. */
    HANDLE hMap;
    char* data;
    if(!size) {
        hMap = {};
        data = nullptr;
    } else {
        /* Create the file mapping */
        if(!(hMap = CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, size, nullptr))) {
            Error err;
            err << "Utility::Path::mapWrite(): can't create file mapping for" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hFile);
            return {};
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)))) {
            Error err;
            err << "Utility::Path::mapWrite(): can't map" << filename << Debug::nospace << ":";
            Utility::Implementation::printWindowsErrorString(err, GetLastError());
            CloseHandle(hMap);
            CloseHandle(hFile);
            return {};
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
    #endif
}
#endif

}}}
