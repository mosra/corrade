/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Directory.h"

#include <dirent.h>
#include <algorithm>

using namespace std;

namespace Map2X { namespace Utility {

Directory::Directory(const string& path, int flags): _isLoaded(false) {
    DIR* directory;
    dirent* entry;

    directory = opendir(path.c_str());
    if(directory == 0) return;

    while((entry = readdir(directory)) != 0) {
        if((flags & SkipDotAndDotDot) && (string(entry->d_name) == "." || string(entry->d_name) == ".."))
            continue;
        if((flags & SkipDirectories) && entry->d_type == DT_DIR)
            continue;
        if((flags & SkipFiles) && entry->d_type == DT_REG)
            continue;
        if((flags & SkipSpecial) && entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;

        /** @todo @c PORTABILITY: on some systems dirent returns DT_UNKNOWN for everything */
        push_back(entry->d_name);
    }

    /** @todo @c MEMLEAK: not deleted entry pointer? */
    closedir(directory);

    if(flags & SortAscending) sort(begin(), end());
    if(flags & SortDescending) sort(rbegin(), rend());

    _isLoaded = true;
}

}}
