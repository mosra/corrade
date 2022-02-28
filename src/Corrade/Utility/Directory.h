#ifndef Corrade_Utility_Directory_h
#define Corrade_Utility_Directory_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
 * @brief Namespace @ref Corrade::Utility::Directory
 */

#include <initializer_list>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#endif

namespace Corrade { namespace Utility {

/**
@brief Filesystem utilities

This library is built if `WITH_UTILITY` is enabled when building Corrade. To
use this library with CMake, request the `Utility` component of the `Corrade`
package and link to the `Corrade::Utility` target:

@code{.cmake}
find_package(Corrade REQUIRED Utility)

# ...
target_link_libraries(your-app PRIVATE Corrade::Utility)
@endcode

See also @ref building-corrade and @ref corrade-cmake for more information.
*/
namespace Directory {

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
class MapDeleter;
#endif

/**
@brief Listing flag

@see @ref Flags, @ref list()
*/
enum class Flag: unsigned char {
    /** Skip `.` and `..` directories */
    SkipDotAndDotDot = 1 << 0,

    /**
     * Skip regular files
     * @partialsupport On @ref CORRADE_TARGET_WINDOWS "Windows" and
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" skips everything except
     *      directories, as there's no concept of a special file.
     */
    SkipFiles = 1 << 1,

    /** Skip directories (including `.` and `..`) */
    SkipDirectories = 1 << 2,

    /**
     * Skip everything that is not a file or directory
     * @partialsupport Has no effect on @ref CORRADE_TARGET_WINDOWS "Windows"
     *      and @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", as these platforms
     *      don't have a concept of a special file.
     */
    SkipSpecial = 1 << 3,

    /**
     * Sort items in ascending order. If both @ref Flag::SortAscending and
     * @ref Flag::SortDescending is specified, ascending order is used.
     */
    SortAscending = (1 << 4) | (1 << 5),

    /**
     * Sort items in descending order. If both @ref Flag::SortAscending and
     * @ref Flag::SortDescending is specified, ascending order is used.
     */
    SortDescending = 1 << 5
};

/**
@brief Listing flags

@see @ref list()
*/
typedef Containers::EnumSet<Flag> Flags;

CORRADE_ENUMSET_OPERATORS(Flags)

/**
@brief Convert path from native separators

On Windows converts backward slashes to forward slashes, on all other platforms
returns the input argument untouched.
*/
CORRADE_UTILITY_EXPORT std::string fromNativeSeparators(std::string path);

/**
@brief Convert path to native separators

On Windows converts forward slashes to backward slashes, on all other platforms
returns the input argument untouched.
*/
CORRADE_UTILITY_EXPORT std::string toNativeSeparators(std::string path);

/**
@brief Extract path from filename

Returns everything before the last slash. If the filename doesn't contain any
path, returns empty string, if the filename is already a path (ends with
slash), returns whole string without trailing slash.
@attention The implementation expects forward slashes as directory separators.
    Use @ref fromNativeSeparators() to convert from a platform-specific format.
@see @ref filename(), @ref splitExtension()
*/
CORRADE_UTILITY_EXPORT std::string path(const std::string& filename);

/**
@brief Extract filename (without path) from filename

If the filename doesn't contain any slash, returns whole string, otherwise
returns everything after last slash.
@attention The implementation expects forward slashes as directory separators.
    Use @ref fromNativeSeparators() to convert from a platform-specific format.
@see @ref path(), @ref splitExtension()
*/
CORRADE_UTILITY_EXPORT std::string filename(const std::string& filename);

/**
@brief Split basename and extension
@m_since{2019,10}

Returns a pair `(root, ext)` where @cpp root + ext == path @ce, and ext is
empty or begins with a period and contains at most one period. Leading periods
on the filename are ignored, @cpp splitExtension("/home/.bashrc") @ce returns
@cpp ("/home/.bashrc", "") @ce. Behavior equivalent to Python's
@cb{.py} os.path.splitext() @ce.
@attention The implementation expects forward slashes as directory separators.
    Use @ref fromNativeSeparators() to convert from a platform-specific format.
@see @ref path(), @ref filename(), @ref String::partition()
*/
CORRADE_UTILITY_EXPORT std::pair<std::string, std::string> splitExtension(const std::string& path);

/**
@brief Join path and filename

If the path is empty or the filename is absolute (with leading slash), returns
@p filename.
@attention The implementation expects forward slashes as directory separators.
    Use @ref fromNativeSeparators() to convert from a platform-specific format.
*/
CORRADE_UTILITY_EXPORT std::string join(const std::string& path, const std::string& filename);

/**
@brief Join paths
@m_since{2019,10}

Convenience overload to @ref join(const std::string&, const std::string&) when
joining the path from more than two parts. When @p paths is empty, returns
empty string, when it's just a single path, returns it verbatim.
@attention The implementation expects forward slashes as directory separators.
    Use @ref fromNativeSeparators() to convert from a platform-specific format.
*/
CORRADE_UTILITY_EXPORT std::string join(std::initializer_list<std::string> paths);

/**
@brief List directory contents

If the directory can't be opened, prints a message to @ref Error and returns an
empty vector. Expects that the path is in UTF-8.
@partialsupport On @ref CORRADE_TARGET_UNIX "Unix" platforms and
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", symlinks are followed and
    @ref Flag::SkipFiles and @ref Flag::SkipDirectories affects the link
    target, not the link itself. This behavior is not implemented on Windows at
    the moment.
@see @ref isDirectory(), @ref exists()
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> list(const std::string& path, Flags flags = Flags());

/**
@brief Create a path

If any component of @p path doesn't exist already and can't be created, prints
a message to @ref Error and returns @cpp false @ce. Creating an empty path
always succeeds. Expects that the path is in UTF-8.
*/
CORRADE_UTILITY_EXPORT bool mkpath(const std::string& path);

/**
@brief Remove a file or a directory

If @p path doesn't exist, is a non-empty directory or can't be removed for some
other reason, prints a message to @ref Error and returns @cpp false @ce.
Expects that the path is in UTF-8.
*/
CORRADE_UTILITY_EXPORT bool rm(const std::string& path);

/**
@brief Move given file or directory

If @p from doesn't exist, can't be read, or @p to can't be written, prints a
message to @ref Error and returns @cpp false @ce. Expects that the paths are in
UTF-8.
@see @ref read(), @ref write()
*/
CORRADE_UTILITY_EXPORT bool move(const std::string& from, const std::string& to);

/**
@brief Whether the application runs in a sandboxed environment

Returns @cpp true @ce if running on @ref CORRADE_TARGET_IOS "iOS",
@ref CORRADE_TARGET_ANDROID "Android", as a
@ref CORRADE_TARGET_APPLE "macOS" app bundle,
@ref CORRADE_TARGET_WINDOWS_RT "Windows Phone/Store" application or in a
browser through @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", @cpp false @ce
otherwise.
*/
CORRADE_UTILITY_EXPORT bool isSandboxed();

/**
@brief Current directory
@m_since{2019,10}

Returns current working directory on Unix systems (equivalent to the value of
shell builtin @cb{.sh} pwd @ce), non-RT Windows and
@ref CORRADE_TARGET_EMSCRIPTEN "Emscripten". On other systems or if the current
directory doesn't exist prints a message to @ref Error and returns an empty
string. Returned value is encoded in UTF-8.

On Unix systems and Emscripten the function is thread-safe. on Windows, the
current directory is stored as a global state and thus modifying its value in
multithreaded applications may have bad consequences. See the
@m_class{m-doc-external} [GetCurrentDirectory()](https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectory)
documentation for details.
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
@see @ref executableLocation()
*/
CORRADE_UTILITY_EXPORT std::string current();

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Shared library location containing given address
@m_since{2019,10}

Like @ref executableLocation() but instead of the main executable returns
location of a shared library that contains @p address. If the address is not
resolvable, prints a message to @ref Error and returns an empty string.
Returned value is encoded in UTF-8.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT std::string libraryLocation(const void* address);

/**
@overload
@m_since{2019,10}
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class R, class ...Args> std::string libraryLocation(R(*address)(Args...));
#else
/* Can't do a template because that would mean including <string>. NOPE NOPE */
namespace Implementation {

/** @todo make a reusable type in Containers for this once there's more
    than one use case? */
struct FunctionPointer {
    /* Assuming both POSIX and Windows allow this (otherwise dlsym() or
       GetProcAddress() wouldn't work either). GCC 4.8 complains that "ISO C++
       forbids casting between pointer-to-function and pointer-to-object", GCC9
       doesn't. Unfortunately __extension__ that's used inside PluginManager
       doesn't work here (it apparently works only when going from void* to a
       function pointer). */
    #if defined(__GNUC__) && __GNUC__ < 5
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    template<class R, class ...Args> /*implicit*/ FunctionPointer(R(*address)(Args...)): address{reinterpret_cast<const void*>(address)} {}
    #ifdef __GNUC__
    #pragma GCC diagnostic pop
    #endif
    const void* address;
};

}
/* Unfortunately, on MSVC (and clang-cl), the const void* overload is picked
   instead, as MSVC implements implicit cast from function pointers to void*
   (while GCC doesn't). On clang-cl it prints a -Wmicrosoft-cast warning and there's no way to make it go to this overload first without using a
   template, so the warning is disabled globally in UseCorrade.cmake. More
   info here: https://bugs.chromium.org/p/chromium/issues/detail?id=550065 */
CORRADE_UTILITY_EXPORT std::string libraryLocation(Implementation::FunctionPointer address);
#endif
#endif

/**
@brief Executable location

Returns location of the executable on Linux, Windows, non-sandboxed and
sandboxed macOS and iOS. On other systems or if the directory can't be found,
prints a message to @ref Error and returns an empty string. Returned value is
encoded in UTF-8.
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
@see @ref current(), @ref libraryLocation()
*/
CORRADE_UTILITY_EXPORT std::string executableLocation();

/**
@brief Current user's home directory

On Unix and non-sandboxed macOS, the directory is equivalent to
@cb{.sh} ${HOME} @ce environment variable. On sandboxed macOS and iOS the
directory is equivalent to what's returned by @cpp NSHomeDirectory() @ce. On
Windows the directory is equivalent to @cb{.bat} %USERPROFILE%/Documents @ce
or similar. On other systems or if the directory can't be found, prints a
message to @ref Error and returns an empty string. Returned value is encoded in
UTF-8.
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT std::string home();

/**
@brief Application configuration dir
@param name     Application name

On Unix (except for macOS), the configuration dir is
@cb{.sh} ${XDG_CONFIG_HOME}/name @ce or @cb{.sh} ${HOME}/.config/name @ce
(@p name is lowercased), on Windows the configuration dir is in
@cb{.bat} %APPDATA%/name @ce (@p name is left as is). On macOS and iOS the
configuration dir is @cb{.sh} ${HOME}/Library/Application Support/name @ce. On
other systems or if the directory can't be found, prints a message to
@ref Error and returns an empty string. Returned value is encoded in UTF-8.
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT std::string configurationDir(const std::string& name);

/**
@brief Temporary dir

On Unix and non-sandboxed macOS, the directory is equivalent to `/tmp`. On
sandboxed macOS and iOS the directory is the `/tmp` subfolder of the app
sandbox. On non-RT Windows the directory is equivalent to @cb{.bat} %TEMP% @ce.
On other systems or if the directory can't be found, prints a message to
@ref Error and returns an empty string. Returned value is encoded in UTF-8.
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT std::string tmp();

/**
@brief Check if the file or directory exists
@m_since{2019,10}

Returns @cpp true @ce if the file exists, @cpp false @ce otherwise.
Inaccessible files may still return @cpp true @ce even if reading them will
subsequently fail. Checking for an empty filename always fails, checking
@cpp "." @ce always succeeds, even in case current working directory doesn't
exist. Expects that the filename is in UTF-8.

Unlike other APIs such as @ref read() or @ref list(), this function doesn't
have any failure state nor it produces any error message --- and if it returns
@cpp true @ce, it doesn't necessarily mean the file can be opened or the
directory listed, that's only possible to know once such operation is
attempted.
@see @ref isDirectory()
*/
CORRADE_UTILITY_EXPORT bool exists(const std::string& filename);

/**
@brief Check if given path is a directory
@m_since{2019,10}

Returns @cpp true @ce if the path exists and is a directory, @cpp false @ce
otherwise. Inaccessible directories may still return @cpp true @ce even if
listing them will subsequently fail. Expects that the filename is in UTF-8.

Unlike other APIs such as @ref list(), this function doesn't have any failure
state nor it produces any error message --- and if it returns @cpp true @ce, it
doesn't necessarily mean the directory can be listed, that's only possible to
know once such operation is attempted.
@partialsupport On @ref CORRADE_TARGET_UNIX "Unix" platforms and
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", symlinks are followed, so this
    function will return @cpp true @ce for a symlink that points to a
    directory. This behavior is not implemented on Windows at the moment.
@see @ref exists(), @ref list()
*/
CORRADE_UTILITY_EXPORT bool isDirectory(const std::string& path);

#ifdef CORRADE_BUILD_DEPRECATED
/** @brief @copybrief exists()
 * @m_deprecated_since{2019,10} Use @ref exists() instead.
 */
inline CORRADE_DEPRECATED("use exists() instead") bool fileExists(const std::string& filename) {
    return exists(filename);
}
#endif

/**
@brief File size
@m_since{2020,06}

If the file can't be read, is a directory or is not seekable, prints a message
to @ref Error and returns @ref Containers::NullOpt. Note that some special
files on Unix platforms may either be non-seekable or report more bytes than
they actually have, in which case using @ref read() is a more reliable way to
get the size along with the contents. Expects that the @p filename is in UTF-8.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<std::size_t> fileSize(const std::string& filename);

/**
@brief Read a file into an array

Reads the whole file in a binary mode (i.e., without newline conversion). If
the file can't be read, prints a message to @ref Error and returns a
@cpp nullptr @ce array. If the file is non-seekable, it's read in 4 kB chunks
and the returned array is growable. For seekable files the returned array has a
default deleter but the size may get shortened from what the system reported to
what was actually read. Expects that the @p filename is in UTF-8.
@see @ref readString(), @ref exists(), @ref write(), @ref append(),
    @ref copy(), @ref mapRead(), @ref fileSize()
*/
CORRADE_UTILITY_EXPORT Containers::Array<char> read(const std::string& filename);

/**
@brief Read a file into a string

Convenience overload for @ref read().
@see @ref exists(), @ref writeString(), @ref append(), @ref copy()
*/
CORRADE_UTILITY_EXPORT std::string readString(const std::string& filename);

/**
@brief Write an array into a file

Writes the file as binary (i.e., without newline conversion). Existing files
are overwritten, use @ref append() to append instead. Prints a message to
@ref Error and returns @cpp false @ce if the file can't be written. Expects
that the @p filename is in UTF-8.
@see @ref writeString(), @ref read(), @ref map()
*/
CORRADE_UTILITY_EXPORT bool write(const std::string& filename, Containers::ArrayView<const void> data);

/**
@brief Write a string into a file

Convenience overload for @ref write().
@see @ref readString(), @ref appendString(), @ref copy()
*/
CORRADE_UTILITY_EXPORT bool writeString(const std::string& filename, const std::string& data);

/**
@brief Append an array to a file
@m_since{2019,10}

Appends to the file as binary (i.e., without newline conversion). Prints a
message to @ref Error and returns @cpp false @ce if the file can't be written.
Expects that the @p filename is in UTF-8.
@see @ref appendString(), @ref write(), @ref read(), @ref copy(), @ref map()
*/
CORRADE_UTILITY_EXPORT bool append(const std::string& filename, Containers::ArrayView<const void> data);

/**
@brief Append a string to a file
@m_since{2019,10}

Convenience overload for @ref append().
@see @ref writeString() @ref readString(), @ref copy()
*/
CORRADE_UTILITY_EXPORT bool appendString(const std::string& filename, const std::string& data);

/**
@brief Copy a file
@m_since{2019,10}

Zero-allocation file copy with 128 kB block size. Works only on single files,
i.e., it can't be used to recursively copy whole directories. Prints a message
to @ref Error and returns @cpp false @ce if @p from can't be read or @p to
can't be written. Expects that @p from and @p to are in UTF-8.

Note that the following might be slightly faster on some systems where
memory-mapping is supported and virtual memory is large enough for given file
size:

@snippet Utility.cpp Directory-copy-mmap

@see @ref read(), @ref write(), @ref mapRead()
*/
CORRADE_UTILITY_EXPORT bool copy(const std::string& from, const std::string& to);

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Map a file for reading and writing
@m_since{2020,06}

Maps the file as a read-write memory. The array deleter takes care of
unmapping. If the file doesn't exist or an error occurs while mapping, prints a
message to @ref Error and returns a @cpp nullptr @ce array. Consistently with
@ref read(), if the file is empty it's only opened but not mapped and a
zero-sized @cpp nullptr @ce array is returned, with the deleter containing the
open file handle. Expects that the @p filename is in UTF-8.
@see @ref mapRead(), @ref mapWrite(), @ref read(), @ref write()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Array<char, MapDeleter> map(const std::string& filename);

/**
@brief Map a file for reading

Maps the file as a read-only memory. The array deleter takes care of unmapping.
If the file doesn't exist or an error occurs while mapping, prints a message to
@ref Error and returns a @cpp nullptr @ce array. Consistently with @ref read(),
if the file is empty it's only opened but not mapped and a zero-sized
@cpp nullptr @ce array is returned, with the deleter containing the open file
handle. Expects that the @p filename is in UTF-8.
@see @ref map(), @ref mapWrite(), @ref read()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Array<const char, MapDeleter> mapRead(const std::string& filename);

/**
@brief Map a file for writing
@m_since{2020,06}

Maps the file as a read-write memory and enlarges it to @p size. If the file
does not exist yet, it is created, if it exists, it's truncated --- thus no
data is preserved. The array deleter takes care of unmapping. If an error
occurs while mapping, prints a message to @ref Error and returns a
@cpp nullptr @ce array. Consistently with @ref map() and @ref mapRead(), if
@p size is @cpp 0 @ce the file is only opened but not mapped and a zero-sized
@cpp nullptr @ce array is returned, with the deleter containing the open file
handle. Expects that the @p filename is in UTF-8.
@see @ref map(), @ref mapRead(), @ref read(), @ref write()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Array<char, MapDeleter> mapWrite(const std::string& filename, std::size_t size);

#ifdef CORRADE_BUILD_DEPRECATED
/**
 * @copybrief mapWrite()
 * @m_deprecated_since{2020,06} Use @ref mapWrite() instead.
 */
CORRADE_DEPRECATED("use mapWrite() instead") CORRADE_UTILITY_EXPORT Containers::Array<char, MapDeleter> map(const std::string& filename, std::size_t size);
#endif

/**
@brief Memory-mapped file deleter

@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
@see @ref map(), @ref mapRead()
*/
class CORRADE_UTILITY_EXPORT MapDeleter {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    #ifdef CORRADE_TARGET_UNIX
    public:
        constexpr explicit MapDeleter(): _fd{} {}
        constexpr explicit MapDeleter(int fd) noexcept: _fd{fd} {}
        void operator()(const char* data, std::size_t size);
    private:
        int _fd;
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    public:
        constexpr explicit MapDeleter(): _hFile{}, _hMap{} {}
        constexpr explicit MapDeleter(void* hFile, void* hMap) noexcept: _hFile{hFile}, _hMap{hMap} {}
        void operator()(const char* data, std::size_t size);
    private:
        void* _hFile;
        void* _hMap;
    #endif
    #endif
};
#endif

}}}

#endif
