#ifndef Corrade_Filesystem_AbstractFilesystem_h
#define Corrade_Filesystem_AbstractFilesystem_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 */

#include <future>

#include "Corrade/PluginManager/AbstractPlugin.h"

namespace Corrade { namespace Filesystem {

/**
@todo What we want:
- make this usable from multi-file importers, which would do the stuff synchronously ?! This won't be useful in Emscripten...
- have the sync versions at all? if yes, fire assert on them when the plugin is async?
*/
class AbstractFilesystem: public PluginManager::AbstractPlugin {
    CORRADE_PLUGIN_INTERFACE("cz.mosra.corrade.Filesystem.AbstractFilesystem/1.0")

    public:
        enum class Feature: std::uint32_t {
            OpenPath = 1 << 0,
            OpenData = 1 << 1,
            Write = 1 << 2,
            Hierarchic = 1 << 3, // ?
            ListDirectories = 1 << 4,
            CreateFiles = 1 << 5,
            Asynchronous = 1 << 6
        };

        typedef Containers::EnumSet<Feature> Features;

        std::string configurationDir() const; // w/o opening?
        std::string home() const; // w/o opening?

        Features features() const { return doFeatures(); }

        bool isOpened() const;

        bool openPath(const std::string& path);

        bool openData(Containers::ArrayView<const char> data, const std::string& path = {});

        void close();

        /** @{ @name Filesystem manipulation */

        std::vector<std::string> list(const std::string& path = {}) const;

        std::future<std::vector<std::string>> listAsync(const std::string& path = {}) const;

        bool fileExists(const std::string& file) const;

        std::future<bool> fileExistsAsync(const std::string& file) const;

        bool createPath(const std::string& path);

        std::future<bool> createPathAsync(const std::string& path);

        bool remove(const std::string& path);

        std::future<bool> removeAsync(const std::string& path);

        bool move(const std::string& oldPath, const std::string& newPath);

        std::future<bool> moveAsync(const std::string& oldPath, const std::string& newPath);

        /*@}*/

        /** @{ @name File manipulation */

        Containers::Array<char> read(const std::string& file);

        std::string readString(const std::string& file);

        bool write(const std::string& file, Containers::ArrayView<const char> data);

        bool writeString(const std::string& file, const std::string& data);

        /*@}*/

    private:
        virtual Features doFeatures() const = 0;

        virtual bool doIsOpened() = 0;

        virtual void doOpenPath(const std::string& path);

        virtual void doOpenData(Containers::ArrayView<const char> data, const std::string& path = {});

        virtual void doClose() = 0;

        virtual std::vector<std::string> doList(const std::string& path) const;

        virtual bool doFileExists(const std::string& file) const;

        virtual bool doCreatePath(const std::string& path);

        virtual bool doRemove(const std::string& path);

        virtual bool doMove(const std::string& oldPath, const std::string& newPath);

        virtual Containers::Array<char> read(const std::string& file);

        virtual bool write(const std::string& file, Containers::ArrayView<char> data);
};

CORRADE_ENUMSET_OPERATORS(AbstractFilesystem::Feature)

}}

#endif
