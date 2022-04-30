#ifndef Corrade_Utility_Path_h
#define Corrade_Utility_Path_h
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
 * @brief Namespace @ref Corrade::Utility::Path
 * @m_since_latest
 */

#include <initializer_list>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Filesystem utilities
@m_since_latest

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
namespace Path {

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
class MapDeleter;
#endif

/**
@brief Convert path from native separators
@m_since_latest

On Windows converts backward slashes to forward slashes, on all other platforms
returns the input argument untouched.

On Windows the argument and return type is @ref Containers::String and the
operation is performed in-place if @p path is owned, transferring the data
ownership to the returned instance. Makes a owned copy first if not. On other
platforms it's a @ref Containers::StringView that simply gets passed through.
*/
#if defined(CORRADE_TARGET_WINDOWS) || defined(DOXYGEN_GENERATING_OUTPUT)
CORRADE_UTILITY_EXPORT Containers::String fromNativeSeparators(Containers::String path);
#else
CORRADE_UTILITY_EXPORT Containers::StringView fromNativeSeparators(Containers::StringView path);
#endif

/**
@brief Convert path to native separators
@m_since_latest

On Windows converts forward slashes to backward slashes, on all other platforms
returns the input argument untouched.

On Windows the argument and return type is @ref Containers::String and the
operation is performed in-place if @p path is owned, transferring the data
ownership to the returned instance. Makes a owned copy first if not. On other
platforms it's a @ref Containers::StringView that simply gets passed through.
*/
#if defined(CORRADE_TARGET_WINDOWS) || defined(DOXYGEN_GENERATING_OUTPUT)
CORRADE_UTILITY_EXPORT Containers::String toNativeSeparators(Containers::String path);
#else
CORRADE_UTILITY_EXPORT Containers::StringView toNativeSeparators(Containers::StringView path);
#endif

/**
@brief Split path and filename
@m_since_latest

Returns a pair of @cpp {head, tail} @ce where `head` is everything before the
last slash and `tail` is everything after. The `head` will never have a
trailing slash except if it's the root (one or two slashes). In all cases,
calling @ref join() on the result will give back the original argument.
Equivalent to Python @m_class{m-doc-external} [os.path.split()](https://docs.python.org/3/library/os.path.html#os.path.split).
For example:

-   @cpp "path/to/file" @ce results in @cpp {"path/to/", "file"} @ce
-   @cpp "file.txt" @ce results in @cpp {"", "file.txt"} @ce
-   @cpp "/home/user/ @ce results in @cpp {"/home/user/", ""} @ce
-   @cpp "/root" @ce results in @cpp {"/", "root"} @ce
-   @cpp "//" @ce results in @cpp {"//", ""} @ce

The implementation expects forward slashes as directory separators. Use
@ref fromNativeSeparators() to convert from a platform-specific format.
@ref Containers::StringViewFlags of the input are propagated the same way as
with @ref Containers::StringView::slice().
@see @ref join(), @ref splitExtension(),
    @ref Containers::StringView::partition()
*/
CORRADE_UTILITY_EXPORT Containers::Pair<Containers::StringView, Containers::StringView> split(Containers::StringView path);

/**
@brief Split basename and extension
@m_since_latest

Returns a pair of @cpp {root, ext} @ce such that @cpp root + ext == path @ce,
and `ext` is empty or begins with a period and contains at most one period.
Leading periods on the filename are ignored. Equivalent to Python
@m_class{m-doc-external} [os.path.splitext()](https://docs.python.org/3/library/os.path.html#os.path.splitext).
For example:

-   @cpp "path/to/file.txt" @ce results in @cpp {"path/to/file", ".txt"} @ce
-   @cpp "file" @ce results in @cpp {"file", ""} @ce
-   @cpp "/home/.bashrc" @ce results in @cpp {"/home/.bashrc", ""} @ce

The implementation expects forward slashes as directory separators. Use
@ref fromNativeSeparators() to convert from a platform-specific format.
@ref Containers::StringViewFlags of the input are propagated the same way as
with @ref Containers::StringView::slice().
@see @ref split(), @ref Containers::StringView::partition()
*/
CORRADE_UTILITY_EXPORT Containers::Pair<Containers::StringView, Containers::StringView> splitExtension(Containers::StringView path);

/**
@brief Join path and filename
@m_since_latest

If the @p path is empty or the @p filename is absolute (with a leading slash or
a drive letter on Windows), returns @p filename. Otherwise joins them together
with a forward slash, unless present in @p path already. Behavior is equivalent
to @m_class{m-doc-external} [os.path.split()](https://docs.python.org/3/library/os.path.html#os.path.split).
For example:

-   @cpp {"path/to", "file"} @ce results in @cpp "path/to/file" @ce
-   @cpp {"path/to/", "file"} @ce results in @cpp "path/to/file" @ce
-   @cpp {"", "file"} @ce results in @cpp "file" @ce
-   @cpp {"path", "/absolute/file"} @ce results in @cpp "/absolute/file" @ce

The implementation expects forward slashes as directory separators. Use
@ref fromNativeSeparators() to convert from a platform-specific format.
@see @ref split()
*/
CORRADE_UTILITY_EXPORT Containers::String join(Containers::StringView path, Containers::StringView filename);

/**
@brief Join paths
@m_since_latest

Result is equivalent to recursively calling
@ref join(Containers::StringView, Containers::StringView) on consecutive pairs
of input arguments. When @p paths is empty, returns an empty string, when it's
just a single path, returns it verbatim.

The implementation expects forward slashes as directory separators. Use
@ref fromNativeSeparators() to convert from a platform-specific format.
*/
CORRADE_UTILITY_EXPORT Containers::String join(Containers::ArrayView<const Containers::StringView> paths);
CORRADE_UTILITY_EXPORT Containers::String join(std::initializer_list<Containers::StringView> paths); /**< @overload */

/**
@brief Check if given file or directory exists
@m_since_latest

Returns @cpp true @ce if the file exists, @cpp false @ce otherwise.
Inaccessible files may still return @cpp true @ce even if reading them will
subsequently fail. Checking for an empty filename always fails, checking
@cpp "." @ce always succeeds, even in case current working directory doesn't
exist.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.

Unlike other APIs such as @ref read() or @ref list(), this function doesn't
have any failure state nor it produces any error message --- and if it returns
@cpp true @ce, it doesn't necessarily mean the file can be opened or the
directory listed, that's only possible to know once such operation is
attempted.
@see @ref isDirectory()
*/
/* Another reason for this not returning Optional<bool> (= not having any
   failure state) is that such return type would be overly error-prone and
   annoying to use -- `if(exists("nonexistent"))` would pass, and the proper
   way to check would be `if(exists(file) && *exists(file))` which is an
   unheard-of API crime. */
CORRADE_UTILITY_EXPORT bool exists(Containers::StringView filename);

/**
@brief Check if given path is a directory
@m_since_latest

Returns @cpp true @ce if the path exists and is a directory, @cpp false @ce
otherwise. Inaccessible directories may still return @cpp true @ce even if
listing them will subsequently fail.

Expects that the @p path is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.

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
/* Another reason for this not returning Optional<bool> (= not having any
   failure state) is that such return type would be overly error-prone and
   annoying to use -- same as with exists(). */
CORRADE_UTILITY_EXPORT bool isDirectory(Containers::StringView path);

/**
@brief Make a path
@m_since_latest

If any component of @p path doesn't exist already and can't be created, prints
a message to @ref Error and returns @cpp false @ce. Creating an empty path
always succeeds.

Expects that the @p path is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
*/
CORRADE_UTILITY_EXPORT bool make(Containers::StringView path);

/**
@brief Remove a file or a directory
@m_since_latest

If @p path doesn't exist, is a non-empty directory or can't be removed for some
other reason, prints a message to @ref Error and returns @cpp false @ce.

Expects that the @p path is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
*/
CORRADE_UTILITY_EXPORT bool remove(Containers::StringView path);

/**
@brief Move given file or directory
@m_since_latest

If @p from doesn't exist, can't be read, or @p to can't be written, prints a
message to @ref Error and returns @cpp false @ce.

Expects that @p from and @p to are in UTF-8. If either path is already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref read(), @ref write()
*/
CORRADE_UTILITY_EXPORT bool move(Containers::StringView from, Containers::StringView to);

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Shared library location containing given address
@m_since_latest

Like @ref executableLocation() but instead of the main executable returns
location of a shared library that contains @p address. If the address is not
resolvable, prints a message to @ref Error and returns @ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> libraryLocation(const void* address);

/**
@overload
@m_since_latest
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class R, class ...Args> Containers::Optional<Containers::String> libraryLocation(R(*address)(Args...));
#else
/* Doing a template conversion for the argument instead of making the whole
   function templates to avoid including Containers::String. */
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
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    template<class R, class ...Args> /*implicit*/ FunctionPointer(R(*address)(Args...)): address{reinterpret_cast<const void*>(address)} {}
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
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
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> libraryLocation(Implementation::FunctionPointer address);
#endif
#endif

/**
@brief Executable location
@m_since_latest

Returns location of the executable on Linux, Windows, non-sandboxed and
sandboxed macOS and iOS. On other systems or if the directory can't be found,
prints a message to @ref Error and returns @ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
@see @ref currentDirectory(), @ref libraryLocation()
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> executableLocation();

/**
@brief Current directory
@m_since_latest

Returns current working directory on Unix systems (equivalent to the value of
shell builtin @cb{.sh} pwd @ce), non-RT Windows and
@ref CORRADE_TARGET_EMSCRIPTEN "Emscripten". On other systems or if the current
directory doesn't exist prints a message to @ref Error and returns
@ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().

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
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> currentDirectory();

/**
@brief Current user's home directory
@m_since_latest

On Unix and non-sandboxed macOS, the directory is equivalent to
@cb{.sh} ${HOME} @ce environment variable. On sandboxed macOS and iOS the
directory is equivalent to what's returned by @cpp NSHomeDirectory() @ce. On
Windows the directory is equivalent to @cb{.bat} %USERPROFILE%/Documents @ce
or similar. On other systems or if the directory can't be found, prints a
message to @ref Error and returns @ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> homeDirectory();

/**
@brief Application configuration directory
@param name     Application name
@m_since_latest

On Unix (except for macOS), the configuration dir is
@cb{.sh} ${XDG_CONFIG_HOME}/name @ce or @cb{.sh} ${HOME}/.config/name @ce
(@p name is lowercased), on Windows the configuration dir is in
@cb{.bat} %APPDATA%/name @ce (@p name is left as is). On macOS and iOS the
configuration dir is @cb{.sh} ${HOME}/Library/Application Support/name @ce. On
other systems or if the directory can't be found, prints a message to
@ref Error and returns @ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> configurationDirectory(Containers::StringView name);

/**
@brief Temporary directory
@m_since_latest

On Unix and non-sandboxed macOS, the directory is equivalent to `/tmp`. On
sandboxed macOS and iOS the directory is the `/tmp` subfolder of the app
sandbox. On non-RT Windows the directory is equivalent to @cb{.bat} %TEMP% @ce.
On other systems or if the directory can't be found, prints a message to
@ref Error and returns @ref Containers::NullOpt.

Returned value is encoded in UTF-8, on Windows it's first converted from a
UTF-16 representation using @ref Unicode::narrow().
@note For consistency is the path returned with forward slashes on all
    platforms. Use @ref toNativeSeparators() to convert it to platform-specific
    format, if needed.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> temporaryDirectory();

/**
@brief Directory listing flag
@m_since_latest

@see @ref ListFlags, @ref list()
*/
enum class ListFlag: unsigned char {
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
     * Sort items in ascending order. If both @ref ListFlag::SortAscending and
     * @ref ListFlag::SortDescending is specified, ascending order is used.
     */
    SortAscending = (1 << 4) | (1 << 5),

    /**
     * Sort items in descending order. If both @ref ListFlag::SortAscending and
     * @ref ListFlag::SortDescending is specified, ascending order is used.
     */
    SortDescending = 1 << 5
};

/**
@brief Directory listing flags
@m_since_latest

@see @ref list()
*/
typedef Containers::EnumSet<ListFlag> ListFlags;

CORRADE_ENUMSET_OPERATORS(ListFlags)

/**
@brief List directory contents
@m_since_latest

If @p path is not a directory or it can't be opened, prints a message to
@ref Error and returns @ref Containers::NullOpt.

Expects that the @p path is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@partialsupport On @ref CORRADE_TARGET_UNIX "Unix" platforms and
    @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", symlinks are followed and
    @ref ListFlag::SkipFiles and @ref ListFlag::SkipDirectories affects the
    link target, not the link itself. This behavior is not implemented on
    Windows at the moment.
@see @ref isDirectory(), @ref exists()
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<Containers::String>> list(Containers::StringView path, ListFlags flags = {});

/**
@brief File size
@m_since_latest

If the file can't be read, is a directory or is not seekable, prints a message
to @ref Error and returns @ref Containers::NullOpt. Note that some special
files on Unix platforms may either be non-seekable or report more bytes than
they actually have, in which case using @ref read() is a more reliable way to
get the size along with the contents.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<std::size_t> size(Containers::StringView filename);

/**
@brief Read a file into an array
@m_since_latest

Reads the whole file in a binary mode (i.e., without newline conversion). If
the file can't be read, prints a message to @ref Error and returns
@ref Containers::NullOpt. If the file is empty, returns a zero-sized
@cpp nullptr @ce array. If the file is non-seekable, it's read in 4 kB chunks
and the returned array is growable. For seekable files the returned array has a
default deleter but the size may get shortened from what the system reported to
what was actually read.

Expects that the @p path is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref readString(), @ref write(), @ref append(), @ref copy(),
    @ref mapRead(), @ref exists(), @ref size()
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<char>> read(Containers::StringView filename);

/**
@brief Read a file into a string
@m_since_latest

Similar to @ref read() but returns a @ref Containers::String, which is
guaranteed to be null-terminated.
@see @ref write(), @ref append(), @ref copy(), @ref exists(), @ref size()
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::String> readString(Containers::StringView filename);

/**
@brief Write an array into a file
@m_since_latest

Writes the file as binary (i.e., without newline conversion). Existing files
are overwritten, use @ref append() to append instead. Prints a message to
@ref Error and returns @cpp false @ce if the file can't be written.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref read(), @ref mapWrite()
*/
CORRADE_UTILITY_EXPORT bool write(Containers::StringView filename, Containers::ArrayView<const void> data);

/**
@brief Writing a C string into a file is not allowed
@m_since_latest

To avoid accidentally writing C strings including the null terminator, please
wrap them in a @ref Containers::ArrayView or @ref Containers::StringView first,
depending on the intention. You can then directly pass the wrapped array to
@ref write(Containers::StringView, Containers::ArrayView<const void>).
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
bool write(Containers::StringView filename, const char* string) = delete;
#else
/* Without the const& it requires StringView to be included, even for a deleted
   function, huh. Not including it as eventually there may be functions that
   operate with file handles instead of filenames, thus not needing StringView.

   The template has to be here in order to avoid ArrayView<const char> to match
   this (because the pointer conversion has a precedence over conversion to
   ArrayView<const void>). The std::decay is here in order to match both char*
   and char[], and because const char[] decays to char* while const char*
   decays to const char* (FFS!!) i need both. */
template<class T, class = typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value || std::is_same<typename std::decay<T>::type, char*>::value>::type> bool write(const Containers::StringView& filename, T&& string) = delete;
#endif

/**
@brief Append an array to a file
@m_since_latest

Appends to the file as binary (i.e., without newline conversion). Prints a
message to @ref Error and returns @cpp false @ce if the file can't be written.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref write(), @ref read(), @ref copy(), @ref map()
*/
CORRADE_UTILITY_EXPORT bool append(Containers::StringView filename, Containers::ArrayView<const void> data);

/**
@brief Appending a C string to a file is not allowed
@m_since_latest

To avoid accidentally writing C strings including the null terminator, please
wrap them in a @ref Containers::ArrayView or @ref Containers::StringView first,
depending on the intention. You can then directly pass the wrapped array to
@ref append(Containers::StringView, Containers::ArrayView<const void>).
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
bool append(Containers::StringView filename, const char* string) = delete;
#else
/* Without the const& it requires StringView to be included, even for a deleted
   function, huh. Not including it as eventually there may be functions that
   operate with file handles instead of filenames, thus not needing StringView.

   The template has to be here in order to avoid ArrayView<const char> to match
   this (because the pointer conversion has a precedence over conversion to
   ArrayView<const void>). The std::decay is here in order to match both char*
   and char[], and because const char[] decays to char* while const char*
   decays to const char* (FFS!!) i need both. */
template<class T, class = typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value || std::is_same<typename std::decay<T>::type, char*>::value>::type> bool append(const Containers::StringView& filename, T&& string) = delete;
#endif

/**
@brief Copy a file
@m_since_latest

Zero-allocation file copy with 128 kB block size. Works only on single files,
i.e., it can't be used to recursively copy whole directories. Prints a message
to @ref Error and returns @cpp false @ce if @p from can't be read or @p to
can't be written.

Expects that @p from and @p to are in UTF-8. If either path is already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.

Note that the following might be slightly faster on some systems where
memory-mapping is supported and virtual memory is large enough for given file
size:

@snippet Utility.cpp Path-copy-mmap

@see @ref read(), @ref write(), @ref mapRead()
*/
CORRADE_UTILITY_EXPORT bool copy(Containers::StringView from, Containers::StringView to);

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
/**
@brief Map a file for reading and writing
@m_since_latest

Maps the file as a read-write memory. The array deleter takes care of
unmapping. If the file doesn't exist or an error occurs while mapping, prints a
message to @ref Error and returns @ref Containers::NullOpt. Consistently with
@ref read(), if the file is empty it's only opened but not mapped and a
zero-sized @cpp nullptr @ce array is returned, with the deleter containing the
open file handle.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref mapRead(), @ref mapWrite(), @ref read(), @ref write()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<char, MapDeleter>> map(Containers::StringView filename);

/**
@brief Map a file for reading
@m_since_latest

Maps the file as a read-only memory. The array deleter takes care of unmapping.
If the file doesn't exist or an error occurs while mapping, prints a message to
@ref Error and returns @ref Containers::NullOpt. Consistently with @ref read(),
if the file is empty it's only opened but not mapped and a zero-sized
@cpp nullptr @ce array is returned, with the deleter containing the open file
handle.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref map(), @ref mapWrite(), @ref read()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<const char, MapDeleter>> mapRead(Containers::StringView filename);

/**
@brief Map a file for writing
@m_since_latest

Maps the file as a read-write memory and enlarges it to @p size. If the file
does not exist yet, it is created, if it exists, it's truncated --- thus no
data is preserved. The array deleter takes care of unmapping. If an error
occurs while mapping, prints a message to @ref Error and returns
@ref Containers::NullOpt. Consistently with @ref map() and @ref mapRead(), if
@p size is @cpp 0 @ce the file is only opened but not mapped and a zero-sized
@cpp nullptr @ce array is returned, with the deleter containing the open file
handle.

Expects that the @p filename is in UTF-8. If it's already
@ref Containers::StringViewFlag::NullTerminated, it's passed to system APIs
directly, otherwise a null-terminated copy is allocated first. On Windows the
path is instead first converted to UTF-16 using @ref Unicode::widen() and then
passed to system APIs.
@see @ref map(), @ref mapRead(), @ref read(), @ref write()
@partialsupport Available only on @ref CORRADE_TARGET_UNIX "Unix" and non-RT
    @ref CORRADE_TARGET_WINDOWS "Windows" platforms.
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<char, MapDeleter>> mapWrite(Containers::StringView filename, std::size_t size);
#endif

#if defined(DOXYGEN_GENERATING_OUTPUT) || (defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)))
/**
@brief Memory-mapped file deleter
@m_since_latest

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

}

}}

#endif
