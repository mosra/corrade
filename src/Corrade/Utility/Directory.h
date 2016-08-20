#ifndef Corrade_Utility_Directory_h
#define Corrade_Utility_Directory_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

        #if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
        /**
         * @brief Memory-mapped file deleter
         *
         * @partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix"
         *      and non-RT @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
         * @see @ref map(), @ref mapRead()
         */
        class MapDeleter;
        #endif

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
         * @brief Convert path from native separators
         *
         * On Windows converts backward slashes to forward slashes, on all
         * other platforms returns the input argument untouched.
         */
        static std::string fromNativeSeparators(std::string path);

        /**
         * @brief Convert path to native separators
         *
         * On Windows converts forward slashes to backward slashes, on all
         * other platforms returns the input argument untouched.
         */
        static std::string toNativeSeparators(std::string path);

        /**
         * @brief Extract path from filename
         *
         * Returns everything before first slash. If the filename doesn't
         * contain any path, returns empty string, if the filename is already a
         * path (ends with slash), returns whole string without trailing slash.
         * @attention The implementation expects forward slashes as directory
         *      separators. Use @ref fromNativeSeparators() to convert from
         *      platform-specific format.
         */
        static std::string path(const std::string& filename);

        /**
         * @brief Extract filename (without path) from filename
         *
         * If the filename doesn't contain any slash, returns whole string,
         * otherwise returns everything after last slash.
         * @attention The implementation expects forward slashes as directory
         *      separators. Use @ref fromNativeSeparators() to convert from
         *      platform-specific format.
         */
        static std::string filename(const std::string& filename);

        /**
         * @brief Join path and filename
         *
         * If the path is empty or the filename is absolute (with leading
         * slash), returns @p filename.
         * @attention The implementation expects forward slashes as directory
         *      separators. Use @ref fromNativeSeparators() to convert from
         *      platform-specific format.
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
         * @brief Whether the application runs in a sandboxed environment
         *
         * Returns `true` if running on @ref CORRADE_TARGET_IOS "iOS",
         * @ref CORRADE_TARGET_ANDROID "Android", as a
         * @ref CORRADE_TARGET_APPLE "Mac OS X" app bundle,
         * @ref CORRADE_TARGET_WINDOWS_RT "Windows Phone/Store" application or
         * in a browser through @ref CORRADE_TARGET_NACL "NaCl" or
         * @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", `false` otherwise.
         */
        static bool isSandboxed();

        /**
         * @brief Executable location
         *
         * Returns location of the executable on Linux, Windows, non-sandboxed
         * and sandboxed OSX and iOS. On other systems or if the directory
         * can't be found, empty string is returned.
         * @note The path is returned with forward slashes on all platforms.
         *      Use @ref toNativeSeparators() to convert it to
         *      platform-specific format, if needed.
         */
        static std::string executableLocation();

        /**
         * @brief Current user's home directory
         *
         * On Unix and non-sandboxed OSX, the directory is equivalent to
         * `${HOME}` environment variable. On sandboxed OSX and iOS the
         * directory is equivalent to what's returned by `NSHomeDirectory()`.
         * On Windows the directory is equivalent to `%USERPROFILE%/Documents`
         * or similar. On other systems or if the directory can't be found,
         * empty string is returned.
         * @note The path is returned with forward slashes on all platforms.
         *      Use @ref toNativeSeparators() to convert it to
         *      platform-specific format, if needed.
         */
        static std::string home();

        /**
         * @brief Application configuration dir
         * @param name              Application name
         *
         * On Unix (except for OSX), the configuration dir is
         * `${XDG_CONFIG_HOME}/name` or `${HOME}/.config/name` (@p name is
         * lowercased), on Windows the configuration dir is in `%APPDATA%/`name`
         * (@p name is left as is). On OSX and iOS the configuration dir
         * is `${HOME}/Library/Application Support/name`. On other systems or
         * if the directory can't be found, empty string is returned.
         * @note The path is returned with forward slashes on all platforms.
         *      Use @ref toNativeSeparators() to convert it to
         *      platform-specific format, if needed.
         */
        static std::string configurationDir(const std::string& name);

        /**
         * @brief Temporary dir
         *
         * On Unix and non-sandboxed OSX, the directory is equivalent to `/tmp`.
         * On sandboxed OSX and iOS the directory is the `/tmp` subfolder of
         * the app sandbox. On non-RT Windows the directory is equivalent to
         * `%TEMP%`. On other systems or if the directory can't be found, empty
         * string is returned.
         * @note The path is returned with forward slashes on all platforms.
         *      Use @ref toNativeSeparators() to convert it to
         *      platform-specific format, if needed.
         */
        static std::string tmp();

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
         * @see @ref readString(), @ref fileExists(), @ref write(),
         *      @ref mapRead()
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
         * @see @ref writeString(), @ref read(), @ref map()
         */
        static bool write(const std::string& filename, Containers::ArrayView<const void> data);

        /**
         * @brief Write string into file
         *
         * Convenience overload for @ref write().
         * @see @ref write(), @ref readString()
         */
        static bool writeString(const std::string& filename, const std::string& data);

        #if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
        /**
         * @brief Map file for reading and writing
         *
         * Maps the file as read-write memory and enlarges it to @p size. If
         * the file does not exist yet, it is created, if it exists, it's
         * truncated. The array deleter takes care of unmapping, however the
         * file is not deleted after unmapping. If an error occurs, `nullptr`
         * is returned and an error message is printed to output.
         * @see @ref mapRead(), @ref read(), @ref write()
         * @partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix"
         *      and non-RT @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
         */
        static Containers::Array<char, MapDeleter> map(const std::string& filename, std::size_t size);

        /**
         * @brief Map file for reading
         *
         * Maps the file as read-only memory. The array deleter takes care of
         * unmapping. If the file doesn't exist or an error occurs while
         * mapping, `nullptr` is returned and an error message is printed to
         * output.
         * @see @ref map(), @ref read()
         * @partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix"
         *      and non-RT @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
         */
        static Containers::Array<const char, MapDeleter> mapRead(const std::string& filename);
        #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifdef CORRADE_TARGET_UNIX
class CORRADE_UTILITY_EXPORT Directory::MapDeleter {
    public:
        constexpr explicit MapDeleter(): _fd{} {}
        constexpr explicit MapDeleter(int fd) noexcept: _fd{fd} {}
        void operator()(const char* data, std::size_t size);
    private:
        int _fd;
};
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
class CORRADE_UTILITY_EXPORT Directory::MapDeleter {
    public:
        constexpr explicit MapDeleter(): _hFile{}, _hMap{} {}
        constexpr explicit MapDeleter(void* hFile, void* hMap) noexcept: _hFile{hFile}, _hMap{hMap} {}
        void operator()(const char* data, std::size_t size);
    private:
        void* _hFile;
        void* _hMap;
};
#endif
#endif

CORRADE_ENUMSET_OPERATORS(Directory::Flags)

}}

#endif
