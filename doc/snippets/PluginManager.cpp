/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include <string>
#include <vector>

#include "Corrade/PluginManager/AbstractManager.h"
#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Macros.h"

using namespace Corrade;

#define CMAKE_INSTALL_PREFIX "/usr"
/* [AbstractPlugin] */
class AbstractFilesystem: public PluginManager::AbstractPlugin {
    public:
        static std::string pluginInterface() {
            return "cz.mosra.corrade.AbstractFilesystem/1.0";
        }

        std::vector<std::string> pluginSearchPaths() {
            return {
                "corrade/filesystems",
                Utility::Directory::join(CMAKE_INSTALL_PREFIX, "lib/corrade/filesystems")
            };
        }

        explicit AbstractFilesystem(PluginManager::AbstractManager& manager, const std::string& plugin):
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
