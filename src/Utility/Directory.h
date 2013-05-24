#ifndef Corrade_Utility_Directory_h
#define Corrade_Utility_Directory_h
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

/** @file
 * @brief Class Corrade::Utility::Directory
 */

#include <string>
#include <vector>

#include "Containers/EnumSet.h"
#include "Utility/corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/**
@brief %Directory utilities
@todo Make it usable on windoze without mingw :-)
@todo Unicode <-> UTF8 path conversion for Windows
*/
class CORRADE_UTILITY_EXPORT Directory {
    public:
        Directory() = delete;

        /**
         * @brief Listing flag
         *
         * @see Flags, list()
         */
        enum class Flag: unsigned char {
            /** Skip `.` and `..` directories */
            SkipDotAndDotDot = 1 << 0,

            /**
             * Skip regular files
             * @partialsupport Has no effect on Windows and in
             *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib". In
             *      @ref CORRADE_TARGET_EMSCRIPTEN_ "Emscripten" skips
             *      everything except directories.
             */
            SkipFiles = 1 << 1,

            /**
             * Skip directories (including `.` and `..`)
             * @partialsupport Has no effect on Windows and in
             *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
             */
            SkipDirectories = 1 << 2,

            /**
             * Skip everything what is not a file or directory
             * @partialsupport Has no effect on Windows and in
             *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib". In
             *      @ref CORRADE_TARGET_EMSCRIPTEN_ "Emscripten" skips
             *      everything except directories.
             */
            SkipSpecial = 1 << 3,

            /**
             * Sort items in ascending order. If specified both @ref Flag "Flag::SortAscending"
             * and @ref Flag "Flag::SortDescending", ascending order is used.
             */
            SortAscending = 3 << 4,

            /**
             * Sort items in descending order. If specified both @ref Flag "Flag::SortAscending"
             * and @ref Flag "Flag::SortDescending", ascending order is used.
             */
            SortDescending = 1 << 5
        };

        /**
         * @brief Listing flags
         *
         * @see list()
         */
        typedef Containers::EnumSet<Flag, unsigned char> Flags;

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
         * @brief List directory contents
         * @param path      Path
         * @param flags     %Flags
         *
         * On failure returns empty vector.
         */
        static std::vector<std::string> list(const std::string& path, Flags flags = Flags());

        /**
         * @brief Create given path
         * @param path      Path
         * @return True if path was successfully created or false if an error
         * occured.
         */
        static bool mkpath(const std::string& path);

        /**
         * @brief Remove given file or directory
         * @param path      Path
         * @return `True` if path is file or empty directory and was
         *      successfully removed, `false` otherwise.
         */
        static bool rm(const std::string& path);

        /**
         * @brief Move given file or directory
         * @param oldPath   Old path
         * @param newPath   New path
         * @return `True` on success, `false` otherwise.
         */
        static bool move(const std::string& oldPath, const std::string& newPath);

        /**
         * @brief Current user's home directory
         *
         * @partialsupport In @ref CORRADE_TARGET_EMSCRIPTEN_ "Emscripten"
         *      returns empty string.
         */
        static std::string home();

        /**
         * @brief Get application configuration dir
         * @param name              Application name
         * @param createIfNotExists Create the directory, if not exists already
         *
         * On Unix, the configuration dir is `~/.name` (*name* is lowercased),
         * on Windows the configuration dir is somewhere in
         * `C:/Document and Settings/user/Application Data/`name` (*name* is
         * left as is).
         */
        static std::string configurationDir(const std::string& name, bool createIfNotExists = true);

        /**
         * @brief Check if the file exists
         * @param filename          Filename
         * @return Whether the file exists and is accessible (e.g. user has
         *      permission to open it).
         */
        static bool fileExists(const std::string& filename);
};

CORRADE_ENUMSET_OPERATORS(Directory::Flags)

}}

#endif
