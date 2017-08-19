/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Directory.h"

#include <cstdio>
#include <cstdlib>
#include <array>
#include <algorithm>
#include <fstream>

/* Unix memory mapping */
#ifdef CORRADE_TARGET_UNIX
#include <fcntl.h>
#include <sys/mman.h>
#endif

/* Unix, Emscripten directory access */
#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
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
#endif

#include "Corrade/configure.h"
#include "Corrade/Containers/Array.h"
#include "Corrade/Utility/String.h"

/* Unicode helpers for Windows */
#ifdef CORRADE_TARGET_WINDOWS
#include "Corrade/Utility/Unicode.h"
#ifdef __MINGW32__
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#endif
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

bool mkpath(const std::string& path) {
    if(path.empty()) return false;

    /* If path contains trailing slash, strip it */
    if(path.back() == '/')
        return mkpath(path.substr(0, path.size()-1));

    /* If the directory exists, done */
    if(fileExists(path)) return true;

    /* If parent directory doesn't exist, create it */
    const std::string parentPath = Directory::path(path);
    if(!parentPath.empty() && !fileExists(parentPath) && !mkpath(parentPath)) return false;

    /* Create directory, return true if successfully created or already exists */

    /* Unix, Emscripten */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    const int ret = mkdir(path.data(), 0777);
    return ret == 0;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    return CreateDirectoryW(widen(path).data(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;

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

bool fileExists(const std::string& filename) {
    /* Sane platforms */
    #ifndef CORRADE_TARGET_WINDOWS
    return std::ifstream(filename).good();

    /* Windows (not Store/Phone) */
    #elif !defined(CORRADE_TARGET_WINDOWS_RT)
    return GetFileAttributesW(widen(filename).data()) != INVALID_FILE_ATTRIBUTES;

    /* Windows Store/Phone not implemented */
    #else
    static_cast<void>(filename);
    Warning() << "Utility::Directory::fileExists(): not implemented on this platform";
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
    HMODULE module = GetModuleHandle(nullptr);
    std::wstring path(MAX_PATH, L'\0');
    std::size_t size = GetModuleFileNameW(module, &path[0], path.size());
    path.resize(size);
    return fromNativeSeparators(narrow(path));

    /* hardcoded for Emscripten */
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    return "/app.js";

    /* Not implemented */
    #else
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
        if((flags >= Flag::SkipSpecial) && entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;
        #else
        if((flags >= Flag::SkipFiles || flags >= Flag::SkipSpecial) && entry->d_type != DT_DIR)
            continue;
        #endif

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

    /* Explicitly add `.` for compatibility with other systems */
    if(!(flags & (Flag::SkipDotAndDotDot|Flag::SkipDirectories))) list.push_back(".");

    while(FindNextFileW(hFile, &data) != 0 || GetLastError() != ERROR_NO_MORE_FILES) {
        if((flags >= Flag::SkipDirectories) && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        if((flags >= Flag::SkipFiles) && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
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
    /* Sane platforms */
    #ifndef CORRADE_TARGET_WINDOWS
    std::ifstream file{filename, std::ifstream::binary};

    /* MSVC */
    #elif !defined(__MINGW32__)
    std::ifstream file{widen(filename), std::ifstream::binary};

    /* MinGW */
    #else
    /* http://stackoverflow.com/q/6524821, http://stackoverflow.com/a/30263811,
       but I'm not sure if filebuf should be stack- or heap-allocated */
    /* https://gcc.gnu.org/onlinedocs/gcc-4.6.2/libstdc++/api/a00069.html#a1dcd5a3e751c566a4b9b3e851ce92b30
       says that the file descriptor returned by _wopen will be closed
       automatically on destruction */
    const int fd = _wopen(widen(filename).data(), _O_RDONLY|_O_BINARY, 0666);
    if(fd == -1) {
        Error{} << "Utility::Directory::read(): can't open" << filename;
        return nullptr;
    }
    __gnu_cxx::stdio_filebuf<char> filebuf{fd, std::ifstream::in|std::ifstream::binary};
    std::istream file{&filebuf};
    #endif

    if(!file) {
        Error{} << "Utility::Directory::read(): can't open" << filename;
        return nullptr;
    }
    file.seekg(0, std::ios::end);

    /** @todo Better solution for non-seekable files */

    /* Probably seekable file. GCC's libstdc++ returns (cast) -1 for
       non-seekable files and sets badbit, Clang's libc++ returns 0 and doesn't
       set badbit, thus zero-length files are indistinguishable from
       non-seekable ones. */
    if(file && file.tellg() != std::ios::pos_type{0}) {
        Containers::Array<char> data(std::size_t(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(data, data.size());
        return data;
    }

    /* Probably non-seekable (or empty) file, clear badbit and read by chunks.
       Hopefully this juggling won't cause any needless memory allocations for
       empty files. */
    file.clear();
    std::string data;
    std::array<char, 4096> buffer;
    do {
        file.read(&buffer[0], buffer.size());
        data.append(&buffer[0], std::size_t(file.gcount()));
    } while(file);

    Containers::Array<char> out(data.size());
    std::copy(data.begin(), data.end(), out.begin());

    return out;
}

std::string readString(const std::string& filename) {
    const auto data = read(filename);

    return {data, data.size()};
}

bool write(const std::string& filename, const Containers::ArrayView<const void> data) {
    /* Sane platforms */
    #ifndef CORRADE_TARGET_WINDOWS
    std::ofstream file{filename, std::ofstream::binary};

    /* MSVC */
    #elif !defined(__MINGW32__)
    std::ofstream file{widen(filename), std::ifstream::binary};

    /* MinGW */
    #else
    /* http://stackoverflow.com/q/6524821, http://stackoverflow.com/a/30263811,
       but I'm not sure if filebuf should be stack- or heap-allocated */
    /* https://gcc.gnu.org/onlinedocs/gcc-4.6.2/libstdc++/api/a00069.html#a1dcd5a3e751c566a4b9b3e851ce92b30
       says that the file descriptor returned by _wopen will be closed
       automatically on destruction */
    const int fd = _wopen(widen(filename).data(), _O_CREAT|_O_TRUNC|_O_WRONLY|_O_BINARY, 0666);
    if(fd == -1) {
        Error{} << "Utility::Directory::write(): can't open" << filename;
        return false;
    }
    __gnu_cxx::stdio_filebuf<char> filebuf{fd, std::ofstream::out|std::ofstream::binary};
    std::ostream file{&filebuf};
    #endif

    if(!file) {
        Error{} << "Utility::Directory::write(): can't open" << filename;
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

bool writeString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return write(filename, {data.data(), data.size()});
}

#ifdef CORRADE_TARGET_UNIX
void MapDeleter::operator()(const char* const data, const std::size_t size) {
    if(data && munmap(const_cast<char*>(data), size) == -1)
        Error() << "Utility::Directory: can't unmap memory-mapped file";
    if(_fd) close(_fd);
}

Containers::Array<char, MapDeleter> map(const std::string& filename, std::size_t size) {
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    const int fd = open(filename.data(), O_RDWR|O_CREAT|O_TRUNC, mode_t(0600));
    if(fd == -1) {
        Error() << "Utility::Directory::map(): can't open" << filename;
        return nullptr;
    }

    /* Resize the file to requested size by seeking one byte before */
    if(lseek(fd, size - 1, SEEK_SET) == -1) {
        close(fd);
        Error() << "Utility::Directory::map(): can't seek to resize the file";
        return nullptr;
    }

    /* And then writing a zero byte on that position */
    if(::write(fd, "", 1) != 1) {
        close(fd);
        Error() << "Utility::Directory::map(): can't write to resize the file";
        return nullptr;
    }

    /* Map the file */
    char* data = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if(data == MAP_FAILED) {
        close(fd);
        Error() << "Utility::Directory::map(): can't map the file";
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

    /* Map the file */
    const char* data = reinterpret_cast<const char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if(data == MAP_FAILED) {
        close(fd);
        Error() << "Utility::Directory::mapRead(): can't map the file";
        return nullptr;
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{fd}};
}
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
void MapDeleter::operator()(const char* const data, const std::size_t) {
    if(data) UnmapViewOfFile(data);
    if(_hMap) CloseHandle(_hMap);
    if(_hFile) CloseHandle(_hFile);
}

Containers::Array<char, MapDeleter> map(const std::string& filename, std::size_t size) {
    /* Open the file for writing. Create if it doesn't exist, truncate it if it
       does. */
    HANDLE hFile = CreateFileW(widen(filename).data(),
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        Error() << "Utility::Directory::map(): can't open" << filename;
        return nullptr;
    }

    /* Create the file mapping */
    HANDLE hMap = CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, size, nullptr);
    if (!hMap) {
        Error() << "Utility::Directory::map(): can't create the file mapping:" << GetLastError();
        CloseHandle(hFile);
        return nullptr;
    }

    /* Map the file */
    char* data = reinterpret_cast<char*>(::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    if(!data) {
        Error() << "Utility::Directory::map(): can't map the file:" << GetLastError();
        CloseHandle(hMap);
        CloseHandle(hFile);
        return nullptr;
    }

    return Containers::Array<char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
}

Containers::Array<const char, MapDeleter> mapRead(const std::string& filename) {
    /* Open the file for reading */
    HANDLE hFile = CreateFileW(widen(filename).data(),
        GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        Error() << "Utility::Directory::mapRead(): can't open" << filename;
        return nullptr;
    }

    /* Create the file mapping */
    HANDLE hMap = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMap) {
        Error() << "Utility::Directory::mapRead(): can't create the file mapping:" << GetLastError();
        CloseHandle(hFile);
        return nullptr;
    }

    /* Get file size */
    const size_t size = GetFileSize(hFile, nullptr);

    /* Map the file */
    char* data = reinterpret_cast<char*>(::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
    if(!data) {
        Error() << "Utility::Directory::mapRead(): can't map the file:" << GetLastError();
        CloseHandle(hMap);
        CloseHandle(hFile);
        return nullptr;
    }

    return Containers::Array<const char, MapDeleter>{data, size, MapDeleter{hFile, hMap}};
}
#endif

}}}
