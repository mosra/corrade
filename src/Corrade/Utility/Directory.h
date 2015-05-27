#ifndef Corrade_Utility_Directory_h
#define Corrade_Utility_Directory_h
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

/** @file
 * @brief Class @ref Corrade::Utility::Directory
 */

#include <string>
#include <vector>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Directory utilities
@todo Unicode <-> UTF8 path conversion for Windows
*/
class CORRADE_UTILITY_EXPORT Directory {
    public:
        Directory() = delete;

        /**
         * @brief Listing flag
         *
         * @see @ref Flags, @ref list()
         */
        enum class Flag: unsigned char {
            /** Skip `.` and `..` directories */
            SkipDotAndDotDot = 1 << 0,

            /**
             * Skip regular files
             * @partialsupport Has no effect in
             *      @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib". On
             *      @ref CORRADE_TARGET_WINDOWS "Windows" and in
             *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" skips
             *      everything except directories.
             */
            SkipFiles = 1 << 1,

            /**
             * Skip directories (including `.` and `..`)
             * @partialsupport Has no effect in
             *      @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib".
             */
            SkipDirectories = 1 << 2,

            /**
             * Skip everything what is not a file or directory
             * @partialsupport Has no effect on @ref CORRADE_TARGET_WINDOWS "Windows"
             *      and in @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib". In
             *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" skips
             *      everything except directories.
             */
            SkipSpecial = 1 << 3,

            /**
             * Sort items in ascending order. If specified both @ref Flag "Flag::SortAscending"
             * and @ref Flag "Flag::SortDescending", ascending order is used.
             */
            SortAscending = (1 << 4) | (1 << 5),

            /**
             * Sort items in descending order. If specified both @ref Flag "Flag::SortAscending"
             * and @ref Flag "Flag::SortDescending", ascending order is used.
             */
            SortDescending = 1 << 5
        };

        /**
         * @brief Listing flags
         *
         * @see @ref list()
         */
        typedef Containers::EnumSet<Flag> Flags;

        /**
         * @brief Extract path from filename
         *
         * Returns everything before first slash. If the filename doesn't
         * contain any path, returns empty string, if the filename is already a
         * path (ends with slash), returns whole string without trailing slash.
         */
        static std::string path(const std::string& filename);

        /**
         * @brief Extract filename (without path) from filename
         *
         * If the filename doesn't contain any slash, returns whole string,
         * otherwise returns everything after last slash.
         */
        static std::string filename(const std::string& filename);

        /**
         * @brief Join path and filename
         *
         * If the path is empty or the filename is absolute (with leading
         * slash), returns @p filename.
         */
        static std::string join(const std::string& path, const std::string& filename);

        /**
         * @brief List directory contents
         *
         * On failure returns empty vector.
         */
        static std::vector<std::string> list(const std::string& path, Flags flags = Flags());

        #ifndef CORRADE_TARGET_NACL_NEWLIB
        /**
         * @brief Create path
         *
         * Returns `true` if path was successfully created, `false` otherwise.
         * @partialsupport Not available in @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib".
         */
        static bool mkpath(const std::string& path);

        /**
         * @brief Remove file or directory
         *
         * Returns `true` if path is file or empty directory and was
         * successfully removed, `false` otherwise.
         * @partialsupport Not available in @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib".
         */
        static bool rm(const std::string& path);

        /**
         * @brief Move given file or directory
         *
         * Returns `true` on success, `false` otherwise.
         * @partialsupport Not available in @ref CORRADE_TARGET_NACL_NEWLIB "NaCl newlib".
         */
        static bool move(const std::string& oldPath, const std::string& newPath);
        #endif

        /**
         * @brief Current user's home directory
         *
         * On Unix, the directory is equivalent to `${HOME}` environment
         * variable, on Windows to `%USERPROFILE%/Documents` or similar. If the
         * directory can't be found, empty string is returned.
         * @partialsupport In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" and
         *      @ref CORRADE_TARGET_NACL returns empty string.
         */
        static std::string home();

        /**
         * @brief Application configuration dir
         * @param name              Application name
         *
         * On Unix, the configuration dir is `${XDG_CONFIG_HOME}/name` or
         * `${HOME}/.config/name` (@p name is lowercased), on Windows the
         * configuration dir is in `%APPDATA%/`name` (@p name is left as is).
         * If the directory can't be found, empty string is returned.
         * @partialsupport In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" and
         *      @ref CORRADE_TARGET_NACL returns empty string.
         */
        static std::string configurationDir(const std::string& name);

        /**
         * @brief Check if the file exists
         *
         * Returns `true` if the file exists and is accessible (e.g. user has
         * permission to open it), `false` otherwise.
         */
        static bool fileExists(const std::string& filename);

        /**
         * @brief Read file into array
         *
         * Reads whole file as binary (i.e. without newline conversion).
         * Returns `nullptr` if the file can't be read.
         * @see @ref readString(), @ref fileExists(), @ref write()
         */
        static Containers::Array<char> read(const std::string& filename);

        /**
         * @brief Read file into string
         *
         * Convenience overload for @ref read().
         * @see @ref fileExists(), @ref writeString()
         */
        static std::string readString(const std::string& filename);

        /**
         * @brief Write array into file
         *
         * Writes the file as binary (i.e. without newline conversion). Returns
         * `false` if the file can't be written, `true` otherwise.
         * @see @ref writeString(), @ref read()
         */
        static bool write(const std::string& filename, Containers::ArrayView<const void> data);

        /**
         * @brief Write string into file
         *
         * Convenience overload for @ref write().
         * @see @ref write(), @ref readString()
         */
        static bool writeString(const std::string& filename, const std::string& data);
};

CORRADE_ENUMSET_OPERATORS(Directory::Flags)

}}

#endif
