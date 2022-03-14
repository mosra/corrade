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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/PluginManager/Manager.h"
#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/PluginManager/Manager.hpp"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Path.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

#define CMAKE_INSTALL_PREFIX "/usr"
/* [AbstractPlugin] */
class AbstractFilesystem: public PluginManager::AbstractPlugin {
    public:
        static Containers::StringView pluginInterface() {
            using namespace Containers::Literals;
            return "cz.mosra.corrade.AbstractFilesystem/1.0"_s;
        }

        static Containers::Array<Containers::String> pluginSearchPaths() {
            return {InPlaceInit, {
                "corrade/filesystems",
                Utility::Path::join(CMAKE_INSTALL_PREFIX, "lib/corrade/filesystems")
            }};
        }

        explicit AbstractFilesystem(PluginManager::AbstractManager& manager, const Containers::StringView& plugin):
            PluginManager::AbstractPlugin{manager, plugin} {}

        explicit AbstractFilesystem() = default;

        // the actual plugin interface goes here
};
/* [AbstractPlugin] */

/* [CORRADE_PLUGIN_IMPORT] */
static int corradeZipFilesystemStaticImport() {
    CORRADE_PLUGIN_IMPORT(ZipFilesystem)
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(corradeZipFilesystemStaticImport)
/* [CORRADE_PLUGIN_IMPORT] */

int main() {
/* Needed to verify the AbstractFilesystem definition is actually usable */
PluginManager::Manager<AbstractFilesystem> manager;

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
{
PluginManager::LoadState loadState{};
/* [LoadStates] */
if(loadState & (PluginManager::LoadState::WrongPluginVersion|
                PluginManager::LoadState::WrongInterfaceVersion)) {
    // ...
}
/* [LoadStates] */
}
#endif
}

/* The include is already above, so doing it again here should be harmless */
/* [Manager-explicit-template-instantiation] */
#include <Corrade/PluginManager/Manager.hpp>

namespace MyNamespace {
    class MyAbstractPlugin: public PluginManager::AbstractPlugin {
        DOXYGEN_ELLIPSIS()
    };
}

namespace Corrade { namespace PluginManager {
    template class Manager<MyNamespace::MyAbstractPlugin>;
}}
/* [Manager-explicit-template-instantiation] */
