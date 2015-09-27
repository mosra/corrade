/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include "Directory.h"

#include <cstdio>
#include <cstdlib>
#include <array>
#include <algorithm>
#include <fstream>

/* Unix */
#ifdef CORRADE_TARGET_UNIX
#include <sys/stat.h>
#include <dirent.h>

/* Windows */
/** @todo remove the superfluous includes when mingw is fixed (otherwise causes undefined EXTERN_C error) */
#elif defined(CORRADE_TARGET_WINDOWS)
#ifdef __MINGW32__
#include <wtypesbase.h>
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

namespace Corrade { namespace Utility {

std::string Directory::path(const std::string& filename) {
    /* If filename is already a path, return it */
    if(!filename.empty() && filename.back() == '/')
        return filename.substr(0, filename.size()-1);

    std::size_t pos = filename.find_last_of('/');

    /* Filename doesn't contain any slash (no path), return empty string */
    if(pos == std::string::npos) return {};

    /* Return everything to last slash */
    return filename.substr(0, pos);
}

std::string Directory::filename(const std::string& filename) {
    std::size_t pos = filename.find_last_of('/');

    /* Return whole filename if it doesn't contain slash */
    if(pos == std::string::npos) return filename;

    /* Return everything after last slash */
    return filename.substr(pos+1);
}

std::string Directory::join(const std::string& path, const std::string& filename) {
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

#ifndef CORRADE_TARGET_NACL_NEWLIB
bool Directory::mkpath(const std::string& path) {
    if(path.empty()) return false;

    /* If path contains trailing slash, strip it */
    if(path.back() == '/')
        return mkpath(path.substr(0, path.size()-1));

    /* If parent directory doesn't exist, create it */
    const std::string parentPath = Directory::path(path);
    if(!parentPath.empty() && !fileExists(parentPath) && !mkpath(parentPath)) return false;

    /* Create directory, return true if successfully created or already exists */

    /* Unix */
    #ifdef CORRADE_TARGET_UNIX
    const int ret = mkdir(path.data(), 0777);
    return ret == 0 || ret == -1;

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    return CreateDirectory(path.data(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;

    /* Not implemented elsewhere */
    #else
    return false;
    #endif
}

bool Directory::rm(const std::string& path) {
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    /* std::remove() can't remove directories on Windows */
    if(GetFileAttributes(path.data()) & FILE_ATTRIBUTE_DIRECTORY)
        return RemoveDirectory(path.data());
    #endif

    return std::remove(path.data()) == 0;
}

bool Directory::move(const std::string& oldPath, const std::string& newPath) {
    return std::rename(oldPath.data(), newPath.data()) == 0;
}
#endif

bool Directory::fileExists(const std::string& filename) {
    /* Sane platforms */
    #ifndef CORRADE_TARGET_WINDOWS
    return std::ifstream(filename).good();

    /* Windows (not Store/Phone) */
    #elif !defined(CORRADE_TARGET_WINDOWS_RT)
    return GetFileAttributes(filename.data()) != INVALID_FILE_ATTRIBUTES;

    /* Windows Store/Phone not implemented */
    #else
    static_cast<void>(filename);
    return false;
    #endif
}

std::string Directory::home() {
    /* Unix */
    #ifdef CORRADE_TARGET_UNIX
    if(const char* const h = std::getenv("HOME"))
        return h;
    return std::string{};

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    TCHAR h[MAX_PATH];
    if(!SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, 0, h)))
        return {};
    return h;

    /* Other */
    #else
    return {};
    #endif
}

std::string Directory::configurationDir(const std::string& applicationName) {
    /* XDG-compilant Unix */
    #ifdef __unix__
    const std::string lowercaseApplicationName = String::lowercase(applicationName);
    if(const char* const config = std::getenv("XDG_CONFIG_HOME"))
        return join(config, lowercaseApplicationName);

    const std::string home = Directory::home();
    return home.empty() ? std::string{} : join(home, ".config/" + lowercaseApplicationName);

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    TCHAR path[MAX_PATH];
    if(!SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
        return {};
    const std::string appdata(path);
    return appdata.empty() ? std::string{} : join(appdata, applicationName);

    /* Other not implemented */
    #else
    static_cast<void>(applicationName);
    return {};
    #endif
}

std::vector<std::string> Directory::list(const std::string& path, Flags flags) {
    std::vector<std::string> list;

    /* POSIX-compilant Unix */
    #ifdef CORRADE_TARGET_UNIX
    DIR* directory = opendir(path.data());
    if(!directory) return list;

    dirent* entry;
    while((entry = readdir(directory)) != nullptr) {
        #ifndef CORRADE_TARGET_NACL_NEWLIB
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
        #endif

        const std::string file{entry->d_name};
        if((flags >= Flag::SkipDotAndDotDot) && (file == "." || file == ".."))
            continue;

        list.push_back(file);
    }

    closedir(directory);

    /* Windows (not Store/Phone) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    WIN32_FIND_DATA data;
    HANDLE hFile = FindFirstFile(join(path, "*").data(), &data);
    if(hFile == INVALID_HANDLE_VALUE) return list;

    /* Explicitly add `.` for compatibility with other systems */
    if(!(flags & (Flag::SkipDotAndDotDot|Flag::SkipDirectories))) list.push_back(".");

    while(FindNextFile(hFile, &data) != 0 || GetLastError() != ERROR_NO_MORE_FILES) {
        if((flags >= Flag::SkipDirectories) && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        if((flags >= Flag::SkipFiles) && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        /** @todo are there any special files in WINAPI? */

        const std::string file{data.cFileName};
        if((flags >= Flag::SkipDotAndDotDot) && (file == "." || file == ".."))
            continue;

        list.push_back(file);
    }

    /* Other not implemented */
    #else
    static_cast<void>(path);
    #endif

    if(flags >= Flag::SortAscending)
        std::sort(list.begin(), list.end());
    else if(flags >= Flag::SortDescending)
        std::sort(list.rbegin(), list.rend());

    return list;
}

Containers::Array<char> Directory::read(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if(!file) return nullptr;

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

std::string Directory::readString(const std::string& filename) {
    const auto data = read(filename);

    return {data, data.size()};
}

bool Directory::write(const std::string& filename, const Containers::ArrayView<const void> data) {
    std::ofstream file(filename, std::ofstream::binary);
    if(!file) return false;

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

bool Directory::writeString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return write(filename, {data.data(), data.size()});
}

}}
