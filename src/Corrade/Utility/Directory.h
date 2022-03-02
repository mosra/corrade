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

#ifdef CORRADE_BUILD_DEPRECATED
/** @file
 * @brief Namespace @ref Corrade::Utility::Directory
 * @m_deprecated_since_latest Use @ref Corrade/Utility/Path.h and the
 *      @relativeref{Corrade,Utility::Path} namespace instead.
 */
#endif

#include "Corrade/configure.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/visibility.h"

#ifndef _CORRADE_NO_DEPRECATED_DIRECTORY
CORRADE_DEPRECATED_FILE("use Corrade/Utility/Path.h and APIs in the Path namespace instead")
#endif

namespace Corrade { namespace Utility {

/**
@brief Filesystem utilities
@m_deprecated_since_latest Use APIs in the @ref Path namespace instead.
*/
namespace CORRADE_DEPRECATED_NAMESPACE("use APIs in the Path namespace instead") Directory {

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Memory-mapped file deleter
@m_deprecated_since_latest Use @ref Path::MapDeleter instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
typedef CORRADE_DEPRECATED("use Path::MapDeleter instead") Path::MapDeleter MapDeleter;
#endif

/**
@brief Listing flag
@m_deprecated_since_latest Use @ref Path::ListFlag instead.
*/
typedef CORRADE_DEPRECATED("use Path::ListFlag instead") Path::ListFlag Flag;

/**
@brief Listing flags
@m_deprecated_since_latest Use @ref Path::ListFlags instead.
*/
typedef CORRADE_DEPRECATED("use Path::ListFlags instead") Path::ListFlags Flags;

/**
@brief Convert path from native separators
@m_deprecated_since_latest Use @ref Path::fromNativeSeparators() instead.
*/
CORRADE_DEPRECATED("use Path::fromNativeSeparators() instead") CORRADE_UTILITY_EXPORT std::string fromNativeSeparators(const std::string& path);

/**
@brief Convert path to native separators
@m_deprecated_since_latest Use @ref Path::toNativeSeparators() instead.
*/
CORRADE_DEPRECATED("use Path::toNativeSeparators() instead") CORRADE_UTILITY_EXPORT std::string toNativeSeparators(const std::string& path);

/**
@brief Extract path from filename
@m_deprecated_since_latest Use @ref Path::split() instead.
*/
CORRADE_DEPRECATED("use Path::split() instead") CORRADE_UTILITY_EXPORT std::string path(const std::string& filename);

/**
@brief Extract filename (without path) from filename
@m_deprecated_since_latest Use @ref Path::split() instead.
*/
CORRADE_DEPRECATED("use Path::split() instead") CORRADE_UTILITY_EXPORT std::string filename(const std::string& filename);

/**
@brief Split basename and extension
@m_deprecated_since_latest Use @ref Path::splitExtension() instead.
*/
CORRADE_DEPRECATED("use Path::splitExtension() instead") CORRADE_UTILITY_EXPORT std::pair<std::string, std::string> splitExtension(const std::string& path);

/**
@brief Join path and filename
@m_deprecated_since_latest Use @ref Path::join() instead.
*/
CORRADE_DEPRECATED("use Path::join() instead") CORRADE_UTILITY_EXPORT std::string join(const std::string& path, const std::string& filename);

/**
@brief Join paths
@m_deprecated_since_latest Use @ref Path::join(std::initializer_list<Containers::StringView>) instead.
*/
CORRADE_DEPRECATED("use Path::join() instead") CORRADE_UTILITY_EXPORT std::string join(std::initializer_list<std::string> paths);

/**
@brief List directory contents
@m_deprecated_since_latest Use @ref Path::list() instead.
*/
CORRADE_DEPRECATED("use Path::list() instead") CORRADE_UTILITY_EXPORT std::vector<std::string> list(const std::string& path,
    #ifdef DOXYGEN_GENERATING_OUTPUT
    Flags flags = {}
    #else
    Path::ListFlags flags = {} /* so it doesn't warn about deprecated Flags */
    #endif
);

/**
@brief Create a path
@m_deprecated_since_latest Use @ref Path::make() instead.
*/
CORRADE_DEPRECATED("use Path::make() instead") CORRADE_UTILITY_EXPORT bool mkpath(const std::string& path);

/**
@brief Remove a file or a directory
@m_deprecated_since_latest Use @ref Path::remove() instead.
*/
CORRADE_DEPRECATED("use Path::remove() instead") CORRADE_UTILITY_EXPORT bool rm(const std::string& path);

/**
@brief Move given file or directory
@m_deprecated_since_latest Use @ref Path::move() instead.
*/
CORRADE_DEPRECATED("use Path::move() instead") CORRADE_UTILITY_EXPORT bool move(const std::string& from, const std::string& to);

/**
@brief Whether the application runs in a sandboxed environment
@m_deprecated_since_latest Use @ref System::isSandboxed() instead.
*/
CORRADE_DEPRECATED("use System::isSandboxed() instead") CORRADE_UTILITY_EXPORT bool isSandboxed();

/**
@brief Current directory
@m_deprecated_since_latest Use @ref Path::currentDirectory() instead.
*/
CORRADE_DEPRECATED("use Path::currentDirectory() instead") CORRADE_UTILITY_EXPORT std::string current();

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Shared library location containing given address
@m_deprecated_since_latest Use @ref Path::libraryLocation() instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_DEPRECATED("use Path::libraryLocation() instead") CORRADE_UTILITY_EXPORT std::string libraryLocation(const void* address);

/**
@overload
@m_deprecated_since_latest Use @ref Path::libraryLocation() instead.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class R, class ...Args> std::string libraryLocation(R(*address)(Args...));
#else
/* Unfortunately, on MSVC (and clang-cl), the const void* overload is picked
   instead, as MSVC implements implicit cast from function pointers to void*
   (while GCC doesn't). On clang-cl it prints a -Wmicrosoft-cast warning and there's no way to make it go to this overload first without using a
   template, so the warning is disabled globally in UseCorrade.cmake. More
   info here: https://bugs.chromium.org/p/chromium/issues/detail?id=550065 */
CORRADE_DEPRECATED("use Path::libraryLocation() instead") CORRADE_UTILITY_EXPORT std::string libraryLocation(Path::Implementation::FunctionPointer address);
#endif
#endif

/**
@brief Executable location
@m_deprecated_since_latest Use @ref Path::executableLocation() instead.
*/
CORRADE_DEPRECATED("use Path::executableLocation() instead") CORRADE_UTILITY_EXPORT std::string executableLocation();

/**
@brief Current user's home directory
@m_deprecated_since_latest Use @ref Path::homeDirectory() instead.
*/
CORRADE_DEPRECATED("use Path::homeDirectory() instead") CORRADE_UTILITY_EXPORT std::string home();

/**
@brief Application configuration dir
@m_deprecated_since_latest Use @ref Path::configurationDirectory() instead.
*/
CORRADE_DEPRECATED("use Path::configurationDirectory() instead") CORRADE_UTILITY_EXPORT std::string configurationDir(const std::string& name);

/**
@brief Temporary dir
@m_deprecated_since_latest Use @ref Path::temporaryDirectory() instead.
*/
CORRADE_DEPRECATED("use Path::temporaryDirectory() instead") CORRADE_UTILITY_EXPORT std::string tmp();

/**
@brief Check if the file or directory exists
@m_deprecated_since_latest Use @ref Path::exists() instead.
*/
CORRADE_DEPRECATED("use Path::exists() instead") CORRADE_UTILITY_EXPORT bool exists(const std::string& filename);

/**
@brief Check if given path is a directory
@m_deprecated_since_latest Use @ref Path::isDirectory() instead.
*/
CORRADE_DEPRECATED("use Path::isDirectory() instead") CORRADE_UTILITY_EXPORT bool isDirectory(const std::string& path);

/** @brief @copybrief exists()
 * @m_deprecated_since{2019,10} Use @ref Path::exists() instead.
 */
inline CORRADE_DEPRECATED("use Path::exists() instead") bool fileExists(const std::string& filename) {
    CORRADE_IGNORE_DEPRECATED_PUSH
    return exists(filename);
    CORRADE_IGNORE_DEPRECATED_POP
}

/**
@brief File size
@m_deprecated_since_latest Use @ref Path::size() instead.
*/
CORRADE_DEPRECATED("use Path::size() instead") CORRADE_UTILITY_EXPORT Containers::Optional<std::size_t> fileSize(const std::string& filename);

/**
@brief Read a file into an array
@m_deprecated_since_latest Use @ref Path::read() instead.
*/
CORRADE_DEPRECATED("use Path::read() instead") CORRADE_UTILITY_EXPORT Containers::Array<char> read(const std::string& filename);

/**
@brief Read a file into a string
@m_deprecated_since_latest Use @ref Path::readString() instead.
*/
CORRADE_DEPRECATED("use Path::readString() instead") CORRADE_UTILITY_EXPORT std::string readString(const std::string& filename);

/**
@brief Write an array into a file
@m_deprecated_since_latest Use @ref Path::write() instead.
*/
CORRADE_DEPRECATED("use Path::write() instead") CORRADE_UTILITY_EXPORT bool write(const std::string& filename, Containers::ArrayView<const void> data);

/**
@brief Write a string into a file
@m_deprecated_since_latest Use @ref Path::write() instead,
    @ref Containers::StringView is implicitly convertible to
    @ref Containers::ArrayView<const void> so there's no need for a dedicated
    string overload.
*/
CORRADE_DEPRECATED("use Path::write() instead") CORRADE_UTILITY_EXPORT bool writeString(const std::string& filename, const std::string& data);

/**
@brief Append an array to a file
@m_deprecated_since_latest Use @ref Path::append() instead.
*/
CORRADE_DEPRECATED("use Path::append() instead") CORRADE_UTILITY_EXPORT bool append(const std::string& filename, Containers::ArrayView<const void> data);

/**
@brief Append a string to a file
@m_deprecated_since_latest Use @ref Path::append() instead,
    @ref Containers::StringView is implicitly convertible to
    @ref Containers::ArrayView<const void> so there's no need for a dedicated
    string overload.
*/
CORRADE_DEPRECATED("use Path::append() instead") CORRADE_UTILITY_EXPORT bool appendString(const std::string& filename, const std::string& data);

/**
@brief Copy a file
@m_deprecated_since_latest Use @ref Path::copy() instead.
*/
CORRADE_DEPRECATED("use Path::copy() instead") CORRADE_UTILITY_EXPORT bool copy(const std::string& from, const std::string& to);

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Map a file for reading and writing
@m_deprecated_since_latest Use @ref Path::map() instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_DEPRECATED("use Path::map() instead") CORRADE_UTILITY_EXPORT Containers::Array<char,
    #ifdef DOXYGEN_GENERATING_OUTPUT
    MapDeleter
    #else
    Path::MapDeleter /* so it doesn't warn about deprecated MapDeleter */
    #endif
> map(const std::string& filename);

/**
@brief Map a file for reading
@m_deprecated_since_latest Use @ref Path::mapRead() instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_DEPRECATED("use Path::mapRead() instead") CORRADE_UTILITY_EXPORT Containers::Array<const char,
    #ifdef DOXYGEN_GENERATING_OUTPUT
    MapDeleter
    #else
    Path::MapDeleter /* so it doesn't warn about deprecated MapDeleter */
    #endif
> mapRead(const std::string& filename);

/**
@brief Map a file for writing
@m_deprecated_since_latest Use @ref Path::mapWrite() instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_DEPRECATED("use Path::mapWrite() instead") CORRADE_UTILITY_EXPORT Containers::Array<char,
    #ifdef DOXYGEN_GENERATING_OUTPUT
    MapDeleter
    #else
    Path::MapDeleter /* so it doesn't warn about deprecated MapDeleter */
    #endif
> mapWrite(const std::string& filename, std::size_t size);

/**
@copybrief mapWrite()
@m_deprecated_since{2020,06} Use @ref Path::mapWrite() instead.
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
@ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_DEPRECATED("use Path::mapWrite() instead") CORRADE_UTILITY_EXPORT Containers::Array<char,
    #ifdef DOXYGEN_GENERATING_OUTPUT
    MapDeleter
    #else
    Path::MapDeleter /* so it doesn't warn about deprecated MapDeleter */
    #endif
> map(const std::string& filename, std::size_t size);
#endif

}}}
#else
#error use Corrade/Utility/Path.h and APIs in the Path namespace instead
#endif

#endif
