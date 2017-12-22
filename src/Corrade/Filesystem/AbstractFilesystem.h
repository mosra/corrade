#ifndef Corrade_Filesystem_AbstractFilesystem_h
#define Corrade_Filesystem_AbstractFilesystem_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
 * @brief Class @ref Corrade::Filesystem::AbstractFilesystem
 * @m_since_latest
 */

#include <cstdint>

#include "Corrade/Filesystem/visibility.h"
#include "Corrade/PluginManager/AbstractPlugin.h"

// TODO argh! there's CORRADE_WITH_FILESYSTEM, and then corrade-filesystem should be named how? or corrade-fs? that's eh ... but maybe..., corrade-rc is a precedent already
// TODO and same would happen with this named Fs and corrade-fs...
namespace Corrade { namespace Filesystem {

/**
@brief Filesystem feature
@m_since_latest

@see @ref FilesystemFeatures, @ref AbstractFilesystem::features()
*/
enum class FilesystemFeature: std::uint8_t {
    // TODO finish docs for all

    // TODO distinguish reading and writing already? should probably name the features as such already
    OpenPath = 1 << 0,
    OpenData = 1 << 1,

    // TODO multiple files, not just e.g. a single compression stream
    Files = 1 << 2,

    // TODO has a notion of directories (e.g. Utility::Resource doesn't have)
    Directories = Files|(1 << 3),

    // TODO can use setCurrentDirectory()
    WorkingDirectory = Directories|(1 << 4),

    // TODO can use mapRead()
    Map = Files|(1 << 5)
};

/**
@brief Filesystem features
@m_since_latest

@see @ref AbstractFilesystem::features()
*/
typedef Containers::EnumSet<FilesystemFeature> FilesystemFeatures;

CORRADE_ENUMSET_OPERATORS(FilesystemFeatures)

/**
@debugoperatorenum{FilesystemFeature}
@m_since_latest
*/
CORRADE_FILESYSTEM_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, FilesystemFeature value);

/**
@debugoperatorenum{FilesystemFeatures}
@m_since_latest
*/
CORRADE_FILESYSTEM_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, FilesystemFeatures value);

/**
@brief Filesystem listing flag
@m_since_latest

@see @ref FilesystemListFlags, @ref AbstractFilesystem::list()
*/
enum class FilesystemListFlag: std::uint8_t {
    /**
     * Skip regular files
     */
    SkipFiles = 1 << 0,

    /** Skip directories */
    SkipDirectories = 1 << 1
};

/**
@brief Filesystem listing flags
@m_since_latest

@see @ref AbstractFilesystem::list()
*/
typedef Containers::EnumSet<FilesystemListFlag> FilesystemListFlags;

CORRADE_ENUMSET_OPERATORS(FilesystemListFlags)

/**
@debugoperatorenum{FilesystemListFlag}
@m_since_latest
*/
CORRADE_FILESYSTEM_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, FilesystemListFlag value);

/**
@debugoperatorenum{FilesystemListFlags}
@m_since_latest
*/
CORRADE_FILESYSTEM_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, FilesystemListFlags value);

/**
@brief Base for filesystem plugins
@m_since_latest

TODO mention why it covers both compression streams alone and filesystems (to handle tar.zst and such nicely)

@section Filesystem-AbstractFilesystem-usage Usage

Filesystems are implemented as plugins. For example, reading an image from a
ZIP filesystem using the @ref AnyFilesystem plugin could be done like this:

@snippet Filesystem.cpp AbstractFilesystem-usage

See `*Filesystem` classes in the @ref Fs namespace for available filesystem
plugins.

TODO expand similarly to other plugins

@subsection Filesystem-AbstractFilesystem-usage-zero-copy Zero-copy file loading

Some filesystems have a possibility to map files into virtual memory instead of
reading them fully into the physical memory, which can save unnecessary copying
of data. Files can be read using @ref mapRead(), which then returns a virtual
memory range that's kept alive at least until either the array deleter is
called or the filesystem is closed --- check particular plugin documentation
for details.

File-based filesystems (such as ZIP files) can be
*/
class CORRADE_FILESYSTEM_EXPORT AbstractFilesystem: public PluginManager::AbstractPlugin {
    public:
        /**
         * @brief Deleter for arrays returned from @ref mapRead()
         *
         * To be used internally by the plugin implementation.
         */
        struct MapDeleter {
            /** @brief Implementation-specific data pointer */
            void* userData;

            /** @brief Implementation-specific deleter implementation */
            void(*deleter)(void*, char*, std::size_t);

            /**
             * @brief Deleter
             *
             * Calls @ref deleter with @ref userData, @p data and @p size.
             */
            void operator()(char* data, std::size_t size) const {
                deleter(userData, data, size);
            }
        };

        /**
         * @brief Plugin interface
         *
         * @snippet Corrade/Filesystem/AbstractFilesystem.h interface
         *
         * @see @ref CORRADE_FILESYSTEM_ABSTRACTFILESYSTEM_PLUGIN_INTERFACE
         */
        static Containers::StringView pluginInterface();

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /**
         * @brief Plugin search paths
         *
         * Looks into `magnum/importers/` or `magnum-d/importers/` next to the
         * dynamic @ref Filesystem library, next to the executable and
         * elsewhere according to the rules documented in
         * @ref PluginManager::implicitPluginSearchPaths(). The search
         * directory can be also hardcoded using the `CORRADE_PLUGINS_DIR`
         * CMake variables, see @ref building for more information.
         *
         * Not defined on platforms without
         * @ref CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT "dynamic plugin support".
         */
        static Containers::Array<Containers::String> pluginSearchPaths();
        #endif

        /** @brief Default constructor */
        explicit AbstractFilesystem();

        /** @brief Constructor with access to plugin manager */
        explicit AbstractFilesystem(PluginManager::Manager<AbstractFilesystem>& manager);

        /** @brief Plugin manager constructor */
        /* The plugin name is passed as a const& to make it possible for people
           to implement plugins without even having to include the StringView
           header. */
        explicit AbstractFilesystem(PluginManager::AbstractManager& manager, const Containers::StringView& plugin);

        /** @brief Features supported by this filesystem */
        FilesystemFeatures features() const { return doFeatures(); }

        // TODO some builtin way to recognize path schemes such as file://? or is that a job for AnyFilesystem?

        /**
         * @brief Whether any filesystem is opened
         *
         * Returns @cpp true @ce if a file is opened with @ref openPath(),
         * @ref openData() or @ref openMemory() and @ref close() wasn't called
         * yet; @cpp false @ce otherwise.
         */
        bool isOpened() const;

        /**
         * @brief Open path as a filesystem
         *
         * Closes previous filesystem, if it was opened, and tries to open
         * given path. Available only if @ref FilesystemFeature::OpenPath is
         * supported. On failure prints a message to @ref Utility::Error and
         * returns @cpp false @ce.
         */
        // TODO distinguish read and modification? likely a separate api?
        bool openPath(Containers::StringView path);
        // TODO mention that the path can be interpreted as just anything, for example a name of a compiled-in resource, or a URL ...

        // TODO damn that means i cannot just simply load the data with Path as a fallback, right? siigh
        // TODO fuh no, just have an option to compile all that Path compat out
        // TODO because then it might make sense to open a zip file from FS on desktop but (maybe) not on Emscripten, and that has to be some compile-time switch

        /**
         * @brief Open data as a filesystem
         *
         * Closes previous filesystem, if it was opened, and tries to open
         * given raw data. Available only if @ref FilesystemFeature::OpenData
         * is supported. On failure prints a message to @ref Utility::Error and
         * returns @cpp false @ce.
         *
         * The @p data is not expected to be alive after the function exits.
         * Using @ref openMemory() instead as can avoid unnecessary copies in
         * exchange for stricter requirements on @p data lifetime.
         * @see @ref features(), @ref openPath()
         */
        // TODO distinguish read and modification? likely a separate api?
        bool openData(Containers::ArrayView<const void> data);

        /**
         * @brief Open memory as a filesystem
         *
         * Closes previous filesystem, if it was opened, and tries to open
         * given memory. Available only if @ref FilesystemFeature::OpenData
         * is supported. On failure prints a message to @ref Utility::Error and
         * returns @cpp false @ce.
         *
         * Unlike @ref openData(), this function expects @p memory to stay in
         * scope until the importer is destructed, @ref close() is called or
         * another file is opened. This allows the implementation to directly
         * operate on the provided memory, without having to allocate a local
         * copy to extend its lifetime.
         * @see @ref features(), @ref openPath()
         */
        bool openMemory(Containers::ArrayView<const void> data);

        /**
         * @brief Close the filesystem
         *
         * On some implementations an explicit call to this function may result
         * in freed memory and/or system resources. This call is also done
         * automatically when the importer gets destructed or when another
         * filesystem is opened.
         *
         * Note that files opened through @ref mapRead() might not be available
         * anymore once this function is called --- see documentation of the
         * particular plugin for more information.
         */
        // TODO variant of close that returns the modified data
        void close();

        /** @{ @name Metadata access */

        /**
         * @brief Current directory
         *
         * Available only if @ref FilesystemFeature::Directories is supported.
         * If @ref FilesystemFeature::CurrentDirectory is not supported, always
         * returns an empty string. Relative paths passed to @ref list(),
         * @ref exists(), @ref read(), @ref mapRead() as well as
         * @ref setCurrentDirectory() are taken relative to this path. Expects
         * that a filesystem is opened.
         * @see @ref setCurrentDirectory()
         */
        Containers::String currentDirectory();

        /**
         * @brief Change current directory
         *
         * Available only if @ref FilesystemFeature::CurrentDirectory is
         * supported. By default the current directory is an empty string,
         * which is equivalent to root of the filesytem opened with
         * @ref openFile(), @ref openData() or @ref openMemory(). Non-absolute
         * paths passed to @ref list(), @ref exists(),
         * @ref read(Containers::StringView),
         * @ref mapRead(Containers::StringView) as well as to this function
         * itself are taken relative to the current directory.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @cpp false @ce. Expects that a filesystem is opened.
         */
        bool setCurrentDirectory(Containers::StringView path);

        /**
         * @brief Check if given file or directory exists
         *
         * Available only if @ref FilesystemFeature::Files is supported.
         *
         * Unlike other APIs such as @ref read() or @ref list(), and
         * consistently with @ref Utility::Path::exists(), this function
         * doesn't have any failure state nor it produces any error message ---
         * and if it returns @cpp true @ce, it doesn't necessarily mean the
         * file can be opened or the directory listed, that's only possible to
         * know once such operation is attempted.
         * @see @ref isDirectory()
         */
        bool exists(Containers::StringView file);

        /**
         * @brief Check if given file or directory exists
         *
         * Available only if @ref FilesystemFeature::Directories is supported.
         *
         * Unlike other APIs such as @ref list(), and consistently with
         * @ref Utility::Path::isDirectory(), this function doesn't have any
         * failure state nor it produces any error message --- and if it
         * returns @cpp true @ce, it doesn't necessarily mean the file can be
         * opened or the directory listed, that's only possible to know once
         * such operation is attempted.
         * @see @ref isDirectory()
         */
        bool isDirectory(Containers::StringView file);

        /**
         * @brief List files in a directory
         *
         * Available only if @ref FilesystemFeature::Directories is supported.
         * Non-absolute @p path is interpreted relative to
         * @ref currentDirectory(), If @p path is empty, lists
         * @ref currentDirectory() itself. Returns always just the filenames,
         * doesn't recurse into subdirectories. Use
         * @ref list(FilesystemListFlags) on filesystems that support only
         * @ref FilesystemFeature::Files.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<Containers::String>> list(Containers::StringView path, FilesystemListFlags flags = {});

        /**
         * @brief List files in current or root directory
         *
         * Available only if @ref FilesystemFeature::Files is supported. If
         * @ref FilesystemFeature::Directories is supported as well, lists
         * files in @ref currentDirectory(), otherwise lists all files in the
         * filesystem.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<Containers::String>> list(FilesystemListFlags flags = {});

        /**
         * @brief Size of the whole filesystem contents
         * @m_since_latest
         *
         * Available only if @ref FilesystemFeature::Files is *not* supported,
         * i.e. the filesystem is for example just a compressed data stream.
         * Use @ref size(Containers::StringView) otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<std::size_t> size();

        /**
         * @brief File size
         * @m_since_latest
         *
         * Available only if @ref FilesystemFeature::Files is supported. Use
         * @ref size() otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<std::size_t> size(Containers::StringView filename);

        /*@}*/

        /** @{ @name Data access */

        /**
         * @brief Read the whole filesystem contents
         *
         * Available only if @ref FilesystemFeature::Files is *not* supported,
         * i.e. the filesystem is for example just a compressed data stream.
         * Use @ref read(Containers::StringView) otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<char>> read();

        /**
         * @brief Read a file
         *
         * Available only if @ref FilesystemFeature::Files is supported. Use
         * @ref read() otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<char>> read(Containers::StringView file);

        /**
         * @brief Map the whole filesystem contents
         *
         * Available only if @ref FilesystemFeature::Map is supported and
         * @ref FilesystemFeature::Files is *not* supported, i.e. the
         * filesystem is for example just a compressed data stream.
         * Use @ref mapRead(Containers::StringView) otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<const char, MapDeleter>> mapRead();

        /**
         * @brief Map a file
         *
         * Available only if @ref FilesystemFeature::Map and
         * @ref FilesystemFeature::Files is supported. If
         * @ref FilesystemFeature::Files is not supported, use @ref mapRead()
         * otherwise.
         *
         * On failure prints a message to @ref Utility::Error and returns
         * @ref Containers::NullOpt. Expects that a filesystem is opened.
         */
        Containers::Optional<Containers::Array<const char, MapDeleter>> mapRead(Containers::StringView file);
        // TODO document that MapDeleter likely relies on the plugin being loaded

        /*@}*/

    private:
        /** @brief Implementation for @ref features() */
        virtual FilesystemFeatures doFeatures() const = 0;

        /** @brief Implementation for @ref isOpened() */
        virtual bool doIsOpened() = 0;

        /** @brief Implementation for @ref openPath() */
        virtual void doOpenPath(Containers::StringView path);

        /** @brief Implementation for @ref openData() */
        virtual void doOpenData(Containers::Array<const void> data);

        /** @brief Implementation for @ref openMemory() */
        virtual void doOpenMemory(Containers::ArrayView<const void> data);

        /** @brief Implementation for @ref close() */
        virtual void doClose() = 0;

        /** @brief Implementation for @ref currentDirectory() */
        virtual Containers::String doCurrentDirectory();

        /** @brief Implementation for @ref setCurrentDirectory() */
        virtual bool doSetCurrentDirectory(Containers::StringView path);

        /** @brief Implementation for @ref exists() */
        virtual bool doExists(Containers::StringView file);

        /** @brief Implementation for @ref isDirectory() */
        virtual bool doIsDirectory(Containers::StringView file);

        /** @brief Implementation for @ref list(FilesystemListFlags) */
        virtual Containers::Optional<Containers::Array<Containers::String>> doList(FilesystemListFlags flags);

        /** @brief Implementation for @ref list(Containers::StringView, FilesystemListFlags) */
        virtual Containers::Optional<Containers::Array<Containers::String>> doList(Containers::StringView path, FilesystemListFlags flags);

        /** @brief Implementation for @ref size() */
        virtual Containers::Optional<std::size_t> doSize();

        /** @brief Implementation for @ref size(Containers::StringView) */
        virtual Containers::Optional<std::size_t> doSize(Containers::StringView);

        /** @brief Implementation for @ref read() */
        virtual Containers::Optional<Containers::Array<char>> doRead();

        /** @brief Implementation for @ref read(Containers::StringView) */
        virtual Containers::Optional<Containers::Array<char>> doRead(Containers::StringView file);

        /** @brief Implementation for @ref mapRead() */
        virtual Containers::Optional<Containers::Array<char, MapDeleter>> doMapRead();

        /** @brief Implementation for @ref mapRead(Containers::StringView) */
        virtual Containers::Optional<Containers::Array<char, MapDeleter>> doMapRead(Containers::StringView file);
};

/**
@brief Filesystem plugin interface
@m_since_latest

Same string as returned by
@relativeref{Corrade::Filesystem,AbstractFilesystem::pluginInterface()}, meant
to be used inside @ref CORRADE_PLUGIN_REGISTER() to avoid having to update the
interface string by hand every time the version gets bumped:

@snippet Filesystem.cpp CORRADE_FILESYSTEM_ABSTRACTFILESYSTEM_PLUGIN_INTERFACE

The interface string version gets increased on every ABI break to prevent
silent crashes and memory corruption. Plugins built against the previous
version will then fail to load, a subsequent rebuild will make them pick up the
updated interface string.
*/
/* Silly indentation to make the string appear in pluginInterface() docs */
#define CORRADE_FILESYSTEM_ABSTRACTFILESYSTEM_PLUGIN_INTERFACE /* [interface] */ \
"cz.mosra.corrade.Filesystem.AbstractFilesystem/0.1"
/* [interface] */

}}

#endif
