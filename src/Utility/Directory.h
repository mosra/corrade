#ifndef Map2X_Utility_Directory_h
#define Map2X_Utility_Directory_h
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

/** @file
 * @brief Class Map2X::Utility::Directory
 */

#include <string>
#include <vector>

namespace Map2X { namespace Utility {

/**
 * @brief %Directory listing
 *
 * Provides list of items in directory as a standard @c vector<string> class.
 * The list is not modifiable. Provides filtering of certain item types,
 * sorting of the list is available.
 * @todo Make it usable on windoze without mingw :-)
 */
class Directory: public std::vector<std::string> {
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
