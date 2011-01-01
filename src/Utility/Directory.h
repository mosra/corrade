#ifndef Kompas_Utility_Directory_h
#define Kompas_Utility_Directory_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Kompas::Utility::Directory
 */

#include <string>
#include <vector>

#include "utilities.h"

namespace Kompas { namespace Utility {

/**
 * @brief %Directory listing
 *
 * Provides list of items in directory as a standard @c vector<string> class.
 * The list is not modifiable. Provides filtering of certain item types,
 * sorting of the list is available.
 * @todo Make it usable on windoze without mingw :-)
 */
class UTILITY_EXPORT Directory: public std::vector<std::string> {
    private:
        /* Hiding edit functions and types to private scope */
        vector::iterator;
        vector::assign;
        vector::push_back;
        vector::pop_back;
        vector::insert;
        vector::erase;
        vector::swap;
        vector::clear;
        vector::get_allocator;
        vector::resize;
        vector::reserve;

        bool _isLoaded;
    public:
        /** @brief Listing flags */
        enum Flags {
            SkipDotAndDotDot = 0x01,    /**< @brief Skip @c '.' and @c '..' directories */
            SkipFiles = 0x02,           /**< @brief Skip regular files */
            SkipDirectories = 0x04,     /**< @brief Skip directories (including @c '.' and @c '..') */
            SkipSpecial = 0x08,         /**< @brief Skip everything what is not a file or directory */
            SortAscending = 0x10,       /**< @brief Sort items in ascending order */
            SortDescending = 0x20       /**< @brief Sort items in descending order */
        };

        /**
         * @brief Extract path from filename
         * @param filename  Filename
         * @return Path (everything before first slash). If the filename
         * doesn't contain any path, returns empty string, if the filename is
         * already a path (ends with slash), returns whole string without
         * trailing slash.
         */
        static std::string path(const std::string& filename);

        /**
         * @brief Extract filename (without path) from filename
         * @param filename  Filename
         * @return File name without path. If the filename doesn't contain any
         * slash, returns whole string, otherwise returns everything after last
         * slash.
         */
        static std::string filename(const std::string& filename);

        /**
         * @brief Join path and filename
         * @param path      Path
         * @param filename  Filename
         * @return Joined path and filename. If the path is empty or the
         * filename is absolute (with leading slash), returns filename.
         */
        static std::string join(const std::string& path, const std::string& filename);

        /**
         * @brief Create given path
         * @param path      Path
         * @return True if path was successfully created or false if an error
         * occured.
         */
        static bool mkpath(const std::string& path);

        /** @brief Get current user's home directory */
        static std::string home();

        /**
         * @brief Get application configuration dir
         * @param name              Application name
         * @param createIfNotExists Create the directory, if not exists already
         */
        static std::string configurationDir(const std::string& name, bool createIfNotExists = true);

        /**
         * @brief Check if the file exists
         * @param filename          Filename
         * @return Whether the file exists and is accessible (e.g. user has
         *      permission to open it).
         */
        static bool fileExists(const std::string& filename);

        /**
         * @brief Constructor
         * @param path      %Directory path
         * @param flags     Listing flags. See Directory::Flags. If no flag is
         *      specified, everything will be loaded.
         *
         * Tries to load items from given directory. Directory::isLoaded()
         * should be used to determine whether the load was successful or not.
         */
        Directory(const std::string& path, int flags = 0);

        /** @brief Whether the directory is successfully loaded */
        inline bool isLoaded() const { return _isLoaded; }
};

}}

#endif
