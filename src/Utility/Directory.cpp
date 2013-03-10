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

#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <algorithm>

#ifdef _WIN32
#include <shlobj.h>
#endif

#include "String.h"

#include "corradeCompatibility.h"

namespace Corrade { namespace Utility {

std::string Directory::path(const std::string& filename) {
    /* If filename is already a path, return it */
    if(!filename.empty() && filename[filename.size()-1] == '/')
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
    /* Absolute filename or empty path, return filename */
    if(path.empty() || (!filename.empty() && filename[0] == '/'))
        return filename;

    /* Add leading slash to path, if not present */
    if(path[path.size()-1] != '/')
        return path + '/' + filename;

    return path + filename;
}

bool Directory::mkpath(const std::string& _path) {
    if(_path.empty()) return false;

    /* If path contains trailing slash, strip it */
    if(_path[_path.size()-1] == '/')
        return mkpath(_path.substr(0, _path.size()-1));

    /* If parent directory doesn't exist, create it */
    std::string parentPath = path(_path);
    if(!parentPath.empty()) {
        DIR* directory = opendir(parentPath.c_str());
        if(directory == nullptr && !mkpath(parentPath)) return false;
        closedir(directory);
    }

    /* Create directory */
    #ifndef _WIN32
    int ret = mkdir(_path.c_str(), 0777);
    #else
    int ret = mkdir(_path.c_str());
    #endif

    /* Directory is successfully created or already exists */
    if(ret == 0 || ret == -1) return true;

    return false;
}

bool Directory::rm(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

bool Directory::move(const std::string& oldPath, const std::string& newPath) {
    return std::rename(oldPath.c_str(), newPath.c_str()) == 0;
}

bool Directory::fileExists(const std::string& filename) {
  struct stat fileInfo;

  if(stat(filename.c_str(), &fileInfo) == 0) return true;
  return false;
}

std::string Directory::home() {
    #ifndef _WIN32
    char* h = getenv("HOME");
    if(!h) return {};
    #else
    /** @bug Doesn't work at all */
    TCHAR h[MAX_PATH];
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, h)))
        return {};
    #pragma GCC diagnostic pop
    #endif

    return h;
}

std::string Directory::configurationDir(const std::string& applicationName, bool createIfNotExists) {
    #ifndef _WIN32
    std::string h = home();
    if(h.empty()) return {};
    std::string dir = join(h, '.' + String::lowercase(applicationName));
    #else
    TCHAR path[MAX_PATH];
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
        return {};
    #pragma GCC diagnostic pop
    std::string appdata = path;
    if(appdata.empty()) return {};
    std::string dir = join(appdata, applicationName);
    #endif

    if(createIfNotExists) mkpath(dir);
    return dir;
}

std::vector<std::string> Directory::list(const std::string& path, Flags flags) {
    DIR* directory;
    directory = opendir(path.c_str());
    if(directory == nullptr) return {};

    std::vector<std::string> list;
    dirent* entry;
    while((entry = readdir(directory)) != nullptr) {
        #ifndef _WIN32
        if((flags >= Flag::SkipDotAndDotDot) && (std::string(entry->d_name) == "." || std::string(entry->d_name) == ".."))
            continue;
        if((flags >= Flag::SkipDirectories) && entry->d_type == DT_DIR)
            continue;
        if((flags >= Flag::SkipFiles) && entry->d_type == DT_REG)
            continue;
        if((flags >= Flag::SkipSpecial) && entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;
        #endif

        /** @todo On some systems dirent returns DT_UNKNOWN for everything */
        list.push_back(entry->d_name);
    }

    closedir(directory);

    if(flags >= Flag::SortAscending)
        std::sort(list.begin(), list.end());
    else if(flags >= Flag::SortDescending)
        std::sort(list.rbegin(), list.rend());

    return std::move(list);
}

}}
