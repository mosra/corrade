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

#include "Directory.h"

#include <cstdio>
#include <cstdlib>
#include <array>
#include <algorithm>
#include <fstream>

#ifndef _WIN32
#include <sys/stat.h>
#include <dirent.h>
#else
#include <shlobj.h>
#endif

#include "Containers/Array.h"
#include "Utility/String.h"

#include "corradeConfigure.h"

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

    #ifdef _WIN32
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
    #ifndef _WIN32
    const int ret = mkdir(path.c_str(), 0777);
    return ret == 0 || ret == -1;
    #else
    return CreateDirectory(path.c_str(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
    #endif
}

bool Directory::rm(const std::string& path) {
    #ifdef _WIN32
    /* std::remove() can't remove directories on Windows */
    if(GetFileAttributes(path.data()) & FILE_ATTRIBUTE_DIRECTORY)
        return RemoveDirectory(path.data());
    #endif

    return std::remove(path.c_str()) == 0;
}

bool Directory::move(const std::string& oldPath, const std::string& newPath) {
    return std::rename(oldPath.c_str(), newPath.c_str()) == 0;
}
#endif

bool Directory::fileExists(const std::string& filename) {
    #ifndef _WIN32
    return std::ifstream(filename).good();
    #else
    return GetFileAttributes(filename.data()) != INVALID_FILE_ATTRIBUTES;
    #endif
}

std::string Directory::home() {
    /* Unix */
    #ifdef __unix__
    if(const char* const h = std::getenv("HOME"))
        return h;
    return std::string{};

    /* Windows */
    #elif defined(_WIN32)
    TCHAR h[MAX_PATH];
    #ifdef __MINGW32__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
    if(!SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, 0, h)))
        return {};
    #ifdef __MINGW32__
    #pragma GCC diagnostic pop
    #endif
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

    /* Windows */
    #elif defined(_WIN32)
    TCHAR path[MAX_PATH];
    #ifdef __MINGW32__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
    if(!SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
        return {};
    #ifdef __MINGW32__
    #pragma GCC diagnostic pop
    #endif
    const std::string appdata(path);
    return appdata.empty() ? std::string{} : join(appdata, applicationName);

    /* Other not implemented */
    #else
    return {};
    #endif
}

std::vector<std::string> Directory::list(const std::string& path, Flags flags) {
    std::vector<std::string> list;

    /* POSIX-compilant Unix */
    #ifdef __unix__
    DIR* directory = opendir(path.c_str());
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

    /* Windows */
    #elif defined(_WIN32)
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
    return list;
    #endif

    if(flags >= Flag::SortAscending)
        std::sort(list.begin(), list.end());
    else if(flags >= Flag::SortDescending)
        std::sort(list.rbegin(), list.rend());

    return std::move(list);
}

Containers::Array<unsigned char> Directory::read(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if(!file) return nullptr;

    file.seekg(0, std::ios::end);

    /* Seekable file */
    if(file) {
        Containers::Array<unsigned char> data(std::size_t(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(data.begin()), data.size());
        return data;
    }

    /* Non-seekable file, clear badbit and read by chunks */
    file.clear();
    std::string data;
    std::array<char, 4096> buffer;
    do {
        file.read(buffer.begin(), buffer.size());
        data.append(buffer.begin(), file.gcount());
    } while(file);

    Containers::Array<unsigned char> out(data.size());
    std::copy(data.begin(), data.end(), out.begin());

    return out;
}

bool Directory::write(const std::string& filename, const Containers::ArrayReference<const void> data) {
    std::ofstream file(filename, std::ofstream::binary);
    if(!file) return false;

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

}}
