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

#include "Directory.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

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

#include "Corrade/Utility/Implementation/WindowsError.h"
#endif

#include "Corrade/configure.h"
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/String.h"

/* Unicode helpers for Windows */
#ifdef CORRADE_TARGET_WINDOWS
#include "Corrade/Utility/Unicode.h"
using Corrade::Utility::Unicode::widen;
using Corrade::Utility::Unicode::narrow;
#endif

namespace Corrade { namespace Utility { namespace Directory {

std::string fromNativeSeparators(std::string path) {
    #ifdef CORRADE_TARGET_WINDOWS
    std::replace(path.begin(), path.end(), '\\', '/');
    #endif
    return path;
}

std::string toNativeSeparators(std::string path) {
    #ifdef CORRADE_TARGET_WINDOWS
    std::replace(path.begin(), path.end(), '/', '\\');
    #endif
    return path;
}

std::string path(const std::string& filename) {
    /* If filename is already a path, return it */
    if(!filename.empty() && filename.back() == '/')
        return filename.substr(0, filename.size()-1);

    std::size_t pos = filename.find_last_of('/');

    /* Filename doesn't contain any slash (no path), return empty string */
    if(pos == std::string::npos) return {};

    /* Return everything to last slash */
    return filename.substr(0, pos);
}

std::string filename(const std::string& filename) {
    std::size_t pos = filename.find_last_of('/');

    /* Return whole filename if it doesn't contain slash */
    if(pos == std::string::npos) return filename;

    /* Return everything after last slash */
    return filename.substr(pos+1);
}

std::pair<std::string, std::string> splitExtension(const std::string& filename) {
    /* Find the last dot and the last slash -- for file.tar.gz we want just
       .gz as an extension; for /etc/rc.conf/bak we don't want to split at the
       folder name. */
    const std::size_t pos = filename.find_last_of('.');
    const std::size_t lastSlash = filename.find_last_of('/');

    /* Empty extension if there's no dot or if the dot is not inside the
       filename */
    if(pos == std::string::npos || (lastSlash != std::string::npos && pos < lastSlash))
        return {filename, {}};

    /* If the dot at the start of the filename (/root/.bashrc), it's also an
       empty extension. Multiple dots at the start (/home/mosra/../..) classify
       as no extension as well. */
    std::size_t prev = pos;
    while(prev && filename[prev - 1] == '.') --prev;
    CORRADE_INTERNAL_ASSERT(pos < filename.size());
    if(prev == 0 || filename[prev - 1] == '/') return {filename, {}};

    /* Otherwise it's a real extension */
    return {filename.substr(0, pos), filename.substr(pos)};
}

std::string join(const std::string& path, const std::string& filename) {
    /* Empty path */
    if(path.empty()) return filename;

    #ifdef CORRADE_TARGET_WINDOWS
    /* Absolute filename on Windows */
    if(filename.size() > 2 && filename[1] == ':' && filename[2] == '/')
        return filename;
    #endif

    /* Absolute filename */
    if(!filename.empty() && filename[0] == '/')
        return filename;

    /* Add trailing slash to path, if not present */
    if(path.back() != '/')
        return path + '/' + filename;

    return path + filename;
}

std::string join(const std::initializer_list<std::string> paths) {
    if(paths.size() == 0) return {};

    auto it = paths.begin();
    std::string path = *it;
    ++it;
    for(; it != paths.end(); ++it)
        path = join(path, *it);

    return path;
}

bool mkpath(const std::string& path) {
    if(path.empty()) return true;

    /* If path contains trailing slash, strip it */
    if(path.back() == '/')
        return mkpath(path.substr(0, path.size()-1));

    /* If parent directory doesn't exist, create it */
    const std::string parentPath = Directory::path(path);
    if(!parentPath.empty() && !exists(parentPath) && !mkpath(parentPath)) return false;

    /* Create directory, return true if successfully created or already exists */

    /* Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const int ret = mkdir(path.data(), 0777);
    if(ret != 0 && errno != EEXIST) {
        Error{} << "Utility::Directory::mkpath(): error creating" << path << Debug::nospace << ":" << strerror(errno);
        return false;
    }
    return true;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    if(CreateDirectoryW(widen(path).data(), nullptr) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
        Error{} << "Utility::Directory::mkpath(): error creating"
            << path << Debug::nospace << ":"
            << Utility::Implementation::windowsErrorString(GetLastError());
        return false;
    }
    return true;

    /* Not implemented elsewhere */
    #else
    Warning() << "Utility::Directory::mkdir(): not implemented on this platform";
    return false;
    #endif
}

bool rm(const std::string& path) {
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* std::remove() can't remove directories on Windows */
    auto wpath = widen(path);
    if(GetFileAttributesW(wpath.data()) & FILE_ATTRIBUTE_DIRECTORY)
        return RemoveDirectoryW(wpath.data());

    /* Need to use nonstandard _wremove in order to handle Unicode properly */
    return _wremove(wpath.data()) == 0;

    #else
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    /* std::remove() can't remove directories on Emscripten */
    struct stat st;
    /* using lstat() and not stat() as we care about the symlink, not the
       file/dir it points to */
    if(lstat(path.data(), &st) == 0 && S_ISDIR(st.st_mode))
        return rmdir(path.data()) == 0;
    #endif

    return std::remove(path.data()) == 0;
    #endif
}

bool move(const std::string& oldPath, const std::string& newPath) {
    return
        #ifndef CORRADE_TARGET_WINDOWS
        std::rename(oldPath.data(), newPath.data())
        #else
        _wrename(widen(oldPath).data(), widen(newPath).data())
        #endif
        == 0;
}

bool exists(const std::string& filename) {
    /* Sane platforms */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    return access(filename.data(), F_OK) == 0;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    return GetFileAttributesW(widen(filename).data()) != INVALID_FILE_ATTRIBUTES;

    /* Windows Store/Phone not implemented */
    #else
    static_cast<void>(filename);
    Warning() << "Utility::Directory::exists(): not implemented on this platform";
    return false;
    #endif
}

namespace {

/* Used by fileSize() and read(). Returns NullOpt if the file is not seekable
   (as file existence is already checked when opening the FILE*). */
Containers::Optional<std::size_t> fileSize(std::FILE* const f) {
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

Containers::Optional<std::size_t> fileSize(const std::string& filename) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(filename.data(), "rb");
    #else
    std::FILE* const f = _wfopen(widen(filename).data(), L"rb");
    #endif
    if(!f) {
        Error{} << "Utility::Directory::fileSize(): can't open" << filename;
        return {};
    }

    Containers::ScopeGuard exit{f, std::fclose};
    Containers::Optional<std::size_t> size = fileSize(f);
    if(!size)
        Error{} << "Utility::Directory::fileSize():" << filename << "is not seekable";
    return size;
}

bool isDirectory(const std::string& path) {
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /** @todo symlink support */
    const DWORD fileAttributes = GetFileAttributesW(widen(path).data());
    return fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    #elif defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* using stat() instead of lstat() as that follows symlinks and that's what
       is desired in most cases */
    struct stat st;
    return stat(path.data(), &st) == 0 && S_ISDIR(st.st_mode);
    #else
    static_cast<void>(path);
    Warning() << "Utility::Directory::isDirectory(): not implemented on this platform";
    return false;
    #endif
}

bool isSandboxed() {
    #if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID) || defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT)
    return true;
    #elif defined(CORRADE_TARGET_APPLE)
    return std::getenv("APP_SANDBOX_CONTAINER_ID");
    #else
    return false;
    #endif
}

std::string current() {
    /* POSIX. Needs a shitty loop because ... ugh. */
    #ifdef CORRADE_TARGET_UNIX
    std::string path(4, '\0');
    char* success;
    while(!(success = getcwd(&path[0], path.size() + 1))) {
        /* Unexpected error, exit */
        if(errno != ERANGE) {
            Error{} << "Utility::Directory::current(): error:" << strerror(errno);
            return {};
        }

        /* Otherwise try again with larger buffer */
        path.resize(path.size()*2);
    }

    /* Success, cut the path to correct size */
    path.resize(std::strlen(&path[0]));
    return path;
    CORRADE_INTERNAL_ASSERT_UNREACHABLE();

    /* Windows (not RT) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    const std::size_t sizePlusOne = GetCurrentDirectoryW(0, nullptr);
    CORRADE_INTERNAL_ASSERT(sizePlusOne);
    /* std::string is always 0-terminated meaning we can ask it to have size
       only for what we need */
    std::wstring path(sizePlusOne - 1, '\0');
    CORRADE_INTERNAL_ASSERT_OUTPUT(GetCurrentDirectoryW(sizePlusOne, &path[0]) == sizePlusOne - 1);
    return fromNativeSeparators(narrow(path));

    /* Use the root path on Emscripten */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    return "/";

    /* No clue elsewhere (and on Windows RT) */
    #else
    Warning() << "Utility::Directory::current(): not implemented on this platform";
    return {};
    #endif
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
std::string libraryLocation(const void* address) {
    /* Linux (and macOS as well, even though Linux man pages don't mention that) */
    #ifdef CORRADE_TARGET_UNIX
    /* Otherwise GCC 4.8 loudly complains about missing initializers */
    Dl_info info{nullptr, nullptr, nullptr, nullptr};
    if(!dladdr(address, &info)) {
        Error e;
        e << "Utility::Directory::libraryLocation(): can't get library location";
        const char* const error = dlerror();
        if(error)
            e << Debug::nospace << ":" << error;
        return {};
    }

    return info.dli_fname;
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    HMODULE module{};
    if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<const char*>(address), &module)) {
        Error{} << "Utility::Directory::libraryLocation(): can't get library location:"
            << Utility::Implementation::windowsErrorString(GetLastError());
        return {};
    }

    /** @todo get rid of MAX_PATH */
    std::wstring path(MAX_PATH, L'\0');
    std::size_t size = GetModuleFileNameW(module, &path[0], path.size());
    path.resize(size);
    return fromNativeSeparators(narrow(path));
    #endif
}

#ifndef DOXYGEN_GENERATING_OUTPUT
std::string libraryLocation(Implementation::FunctionPointer address) {
    return libraryLocation(address.address);
}
#endif
#endif

std::string executableLocation() {
    /* Linux */
    #if defined(__linux__)
    /* Reallocate like hell until we have enough place to store the path. Can't
       use lstat because the /proc/self/exe symlink is not a real symlink and
       so stat::st_size returns 0. POSIX, WHAT THE HELL. */
    constexpr const char self[]{"/proc/self/exe"};
    std::string path(4, '\0');
    ssize_t size;
    while((size = readlink(self, &path[0], path.size())) == ssize_t(path.size()))
        path.resize(path.size()*2);

    CORRADE_INTERNAL_ASSERT(size > 0);

    path.resize(size);
    return path;

    /* OSX, iOS */
    #elif defined(CORRADE_TARGET_APPLE)
    /* Get path size (need to set it to 0 to avoid filling nullptr with random
       data and crashing) */
    std::uint32_t size = 0;
    CORRADE_INTERNAL_ASSERT_OUTPUT(_NSGetExecutablePath(nullptr, &size) == -1);

    /* Allocate proper size and get the path */
    std::string path(size, '\0');
    CORRADE_INTERNAL_ASSERT_OUTPUT(_NSGetExecutablePath(&path[0], &size) == 0);
    return path;

    /* Windows (not RT) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    std::wstring path(MAX_PATH, L'\0');
    std::size_t size = GetModuleFileNameW(nullptr, &path[0], path.size());
    path.resize(size);
    return fromNativeSeparators(narrow(path));

    /* hardcoded for Emscripten */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    return "/app.js";

    /* Not implemented */
    #else
    Warning() << "Utility::Directory::executableLocation(): not implemented on this platform";
    return std::string{};
    #endif
}

std::string home() {
    /* Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    if(const char* const h = std::getenv("HOME"))
        return h;
    return std::string{};

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    wchar_t h[MAX_PATH];
    if(!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, h)))
        return {};
    return fromNativeSeparators(narrow(h));

    /* Other */
    #else
    Warning() << "Utility::Directory::home(): not implemented on this platform";
    return {};
    #endif
}

std::string configurationDir(const std::string& applicationName) {
    /* OSX, iOS */
    #ifdef CORRADE_TARGET_APPLE
    return join(home(), "Library/Application Support/" + applicationName);

    /* XDG-compliant Unix (not using CORRADE_TARGET_UNIX, because that is a
       superset), Emscripten */
    #elif defined(__unix__) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const std::string lowercaseApplicationName = String::lowercase(applicationName);
    if(const char* const config = std::getenv("XDG_CONFIG_HOME"))
        return join(config, lowercaseApplicationName);

    const std::string home = Directory::home();
    return home.empty() ? std::string{} : join(home, ".config/" + lowercaseApplicationName);

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    wchar_t path[MAX_PATH];
    if(!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
        return {};
    const std::string appdata{fromNativeSeparators(narrow(path))};
    return appdata.empty() ? std::string{} : join(appdata, applicationName);

    /* Other not implemented */
    #else
    static_cast<void>(applicationName);
    Warning() << "Utility::Directory::configurationDir(): not implemented on this platform";
    return {};
    #endif
}

std::string tmp() {
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    /* Sandboxed OSX, iOS */
    #ifdef CORRADE_TARGET_APPLE
    if(isSandboxed()) return join(home(), "tmp");
    #endif

    /* Android, you had to be special, right? */
    #ifdef CORRADE_TARGET_ANDROID
    return "/data/local/tmp";
    #endif

    /* Common Unix, Emscripten */
    return "/tmp";

    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* Windows */

    /* Get path size */
    wchar_t c;
    const std::size_t size = GetTempPathW(1, &c);

    /* Get the path, remove the trailing slash (and zero terminator) */
    std::wstring path(size, '\0');
    GetTempPathW(size, &path[0]);
    if(path.size()) path.resize(path.size() - 2);

    /* Convert to forward slashes */
    return fromNativeSeparators(narrow(path));
    #else
    Warning() << "Utility::Directory::tmp(): not implemented on this platform";
    return {};
    #endif
}

std::vector<std::string> list(const std::string& path, Flags flags) {
    std::vector<std::string> list;

    /* POSIX-compliant Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    DIR* directory = opendir(path.data());
    if(!directory) return list;

    dirent* entry;
    while((entry = readdir(directory)) != nullptr) {
        if((flags >= Flag::SkipDirectories) && entry->d_type == DT_DIR)
            continue;
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        if((flags >= Flag::SkipFiles) && entry->d_type == DT_REG)
            continue;
        if((flags >= Flag::SkipSpecial) && entry->d_type != DT_DIR && entry->d_type != DT_REG && entry->d_type != DT_LNK)
            continue;
        #else
        /* Emscripten doesn't set DT_REG for files, so we treat everything
           that's not a DT_DIR as a file. SkipSpecial has no effect here. */
        if(flags >= Flag::SkipFiles && entry->d_type != DT_DIR)
            continue;
        #endif

        /* For symlinks we have to deref the link and ask there again. If that
           fails for whatever reason, we leave the file in the list -- it can
           be thought of as "neither a file nor directory" and we're told to
           skip files/directories, not "include only files/directories".

           Also do this only if we're told to skip certain entry types, for a
           plain list this is unnecessary overhead. */
        if((flags & (Flag::SkipDirectories|Flag::SkipFiles|Flag::SkipSpecial)) && entry->d_type == DT_LNK) {
            /* stat() follows the symlink, lstat() doesn't */
            struct stat st;
            if(stat((join(path, entry->d_name)).data(), &st) == 0) {
                if(flags >= Flag::SkipDirectories && S_ISDIR(st.st_mode))
                    continue;
                if(flags >= Flag::SkipFiles && S_ISREG(st.st_mode))
                    continue;
                if(flags >= Flag::SkipSpecial && !S_ISDIR(st.st_mode) && !S_ISREG(st.st_mode))
                    continue;
            }
        }

        std::string file{entry->d_name};
        if((flags >= Flag::SkipDotAndDotDot) && (file == "." || file == ".."))
            continue;

        list.push_back(std::move(file));
    }

    closedir(directory);

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    WIN32_FIND_DATAW data;
    HANDLE hFile = FindFirstFileW(widen(join(path, "*")).data(), &data);
    if(hFile == INVALID_HANDLE_VALUE) return list;
    Containers::ScopeGuard closeHandle{hFile,
        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        /* MSVC 2015 is unable to cast the parameter for FindClose */
        [](HANDLE hFile){ FindClose(hFile); }
        #else
        FindClose
        #endif
    };

    /* Explicitly add `.` for compatibility with other systems */
    if(!(flags & (Flag::SkipDotAndDotDot|Flag::SkipDirectories))) list.push_back(".");

    while(FindNextFileW(hFile, &data) != 0 || GetLastError() != ERROR_NO_MORE_FILES) {
        if((flags >= Flag::SkipDirectories) && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        if((flags >= Flag::SkipFiles) && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        /** @todo symlink support */
        /** @todo are there any special files in WINAPI? */

        std::string file{narrow(data.cFileName)};
        /* Not testing for dot, as it is not listed on Windows */
        if((flags >= Flag::SkipDotAndDotDot) && file == "..")
            continue;

        list.push_back(std::move(file));
    }

    /* Other not implemented */
    #else
    Warning() << "Utility::Directory::list(): not implemented on this platform";
    static_cast<void>(path);
    #endif

    if(flags >= Flag::SortAscending)
        std::sort(list.begin(), list.end());
    else if(flags >= Flag::SortDescending)
        std::sort(list.rbegin(), list.rend());

    return list;
}

Containers::Array<char> read(const std::string& filename) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(filename.data(), "rb");
    #else
    std::FILE* const f = _wfopen(widen(filename).data(), L"rb");
    #endif
    if(!f) {
        Error{} << "Utility::Directory::read(): can't open" << filename;
        return nullptr;
    }

    Containers::ScopeGuard exit{f, std::fclose};
    Containers::Optional<std::size_t> size = fileSize(f);

    /* If the file is not seekable, read it in chunks */
    if(!size) {
        std::string data;
        char buffer[4096];
        std::size_t count;
        do {
            count = std::fread(buffer, 1, Containers::arraySize(buffer), f);
            data.append(buffer, count);
        } while(count);

        Containers::Array<char> out{data.size()};
        std::copy(data.begin(), data.end(), out.begin());
        return out;
    }

    /* Some special files report more bytes than they actually have (such as
       stuff in /sys). Clamp the returned array to what was reported. */
    Containers::Array<char> out{*size};
    const std::size_t realSize = std::fread(out, 1, *size, f);
    CORRADE_INTERNAL_ASSERT(realSize <= *size);
    return Containers::Array<char>{out.release(), realSize};
}

std::string readString(const std::string& filename) {
    const auto data = read(filename);

    return {data, data.size()};
}

bool write(const std::string& filename, const Containers::ArrayView<const void> data) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(filename.data(), "wb");
    #else
    std::FILE* const f = _wfopen(widen(filename).data(), L"wb");
    #endif
    if(!f) {
        Error{} << "Utility::Directory::write(): can't open" << filename;
        return false;
    }

    Containers::ScopeGuard exit{f, std::fclose};

    std::fwrite(data, 1, data.size(), f);
    return true;
}

bool writeString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return write(filename, {data.data(), data.size()});
}

bool append(const std::string& filename, const Containers::ArrayView<const void> data) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const f = std::fopen(filename.data(), "ab");
    #else
    std::FILE* const f = _wfopen(widen(filename).data(), L"ab");
    #endif
    if(!f) {
        Error{} << "Utility::Directory::append(): can't open" << filename;
        return false;
    }

    Containers::ScopeGuard exit{f, std::fclose};

    std::fwrite(data, 1, data.size(), f);
    return true;
}

bool appendString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return append(filename, {data.data(), data.size()});
}

bool copy(const std::string& from, const std::string& to) {
    /* Special case for "Unicode" Windows support */
    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const in = std::fopen(from.data(), "rb");
    #else
    std::FILE* const in = _wfopen(widen(from).data(), L"rb");
    #endif
    if(!in) {
        Error{} << "Utility::Directory::copy(): can't open" << from << "for reading";
        return false;
    }

    Containers::ScopeGuard exitIn{in, std::fclose};

    #ifndef CORRADE_TARGET_WINDOWS
    std::FILE* const out = std::fopen(to.data(), "wb");
    #else
    std::FILE* const out = _wfopen(widen(to).data(), L"wb");
    #endif
    if(!out) {
        Error{} << "Utility::Directory::copy(): can't open" << to << "for writing";
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
    char buffer[128*1024];
    std::size_t count;
    do {
        count = std::fread(buffer, 1, Containers::arraySize(buffer), in);
        std::fwrite(buffer, 1, count, out);
    } while(count);

    return true;
}

#ifdef CORRADE_TARGET_UNIX
void MapDeleter::operator()(const char* const data, const std::size_t size) {
    if(data && munmap(const_cast<char*>(data), size) == -1)
        Error() << "Utility::Directory: can't unmap memory-mapped file";
    if(_fd) close(_fd);
}

Containers::Array<char, MapDeleter> map(const std::string& filename) {
    /* Open the file for reading */
    const int fd = open(filename.data(), O_RDWR);
    if(fd == -1) {
        Error{} << "Utility::Directory::map(): can't open" << filename;
        return nullptr;
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
        close(fd);
        Error{} << "Utility::Directory::map(): can't map the file";
        return nullptr;
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{fd}};
}

Containers::Array<const char, MapDeleter> mapRead(const std::string& filename) {
    /* Open the file for reading */
    const int fd = open(filename.data(), O_RDONLY);
    if(fd == -1) {
        Error() << "Utility::Directory::mapRead(): can't open" << filename;
        return nullptr;
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
        close(fd);
        Error() << "Utility::Directory::mapRead(): can't map the file";
        return nullptr;
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{fd}};
}

Containers::Array<char, MapDeleter> mapWrite(const std::string& filename, const std::size_t size) {
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    const int fd = open(filename.data(), O_RDWR|O_CREAT|O_TRUNC, mode_t(0600));
    if(fd == -1) {
        Error{} << "Utility::Directory::mapWrite(): can't open" << filename;
        return nullptr;
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
            close(fd);
            Error{} << "Utility::Directory::mapWrite(): can't seek to resize the file";
            return nullptr;
        }

        /* And then writing a zero byte on that position */
        if(::write(fd, "", 1) != 1) {
            close(fd);
            Error{} << "Utility::Directory::mapWrite(): can't write to resize the file";
            return nullptr;
        }

        /* Map the file */
        if((data = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))) == MAP_FAILED) {
            close(fd);
            Error{} << "Utility::Directory::mapWrite(): can't map the file";
            return nullptr;
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{fd}};
}
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
void MapDeleter::operator()(const char* const data, const std::size_t) {
    if(data) UnmapViewOfFile(data);
    if(_hMap) CloseHandle(_hMap);
    if(_hFile) CloseHandle(_hFile);
}

Containers::Array<char, MapDeleter> map(const std::string& filename) {
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    HANDLE hFile = CreateFileW(widen(filename).data(),
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error() << "Utility::Directory::map(): can't open" << filename;
        return nullptr;
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
            Error{} << "Utility::Directory::map(): can't create the file mapping:" << GetLastError();
            CloseHandle(hFile);
            return nullptr;
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)))) {
            Error{} << "Utility::Directory::map(): can't map the file:" << GetLastError();
            CloseHandle(hMap);
            CloseHandle(hFile);
            return nullptr;
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
}

Containers::Array<const char, MapDeleter> mapRead(const std::string& filename) {
    /* Open the file for reading */
    HANDLE hFile = CreateFileW(widen(filename).data(),
        GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error() << "Utility::Directory::mapRead(): can't open" << filename;
        return nullptr;
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
            Error{} << "Utility::Directory::mapRead(): can't create the file mapping:" << GetLastError();
            CloseHandle(hFile);
            return nullptr;
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)))) {
            Error{} << "Utility::Directory::mapRead(): can't map the file:" << GetLastError();
            CloseHandle(hMap);
            CloseHandle(hFile);
            return nullptr;
        }
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
}

Containers::Array<char, MapDeleter> mapWrite(const std::string& filename, const std::size_t size) {
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    HANDLE hFile = CreateFileW(widen(filename).data(),
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        Error() << "Utility::Directory::mapWrite(): can't open" << filename;
        return nullptr;
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
            Error{} << "Utility::Directory::mapWrite(): can't create the file mapping:" << GetLastError();
            CloseHandle(hFile);
            return nullptr;
        }

        /* Map the file */
        if(!(data = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)))) {
            Error{} << "Utility::Directory::mapWrite(): can't map the file:" << GetLastError();
            CloseHandle(hMap);
            CloseHandle(hFile);
            return nullptr;
        }
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
}
#endif

#if defined(CORRADE_BUILD_DEPRECATED) && (defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)))
Containers::Array<char, MapDeleter> map(const std::string& filename, const std::size_t size) {
    return mapWrite(filename, size);
}
#endif

}}}
