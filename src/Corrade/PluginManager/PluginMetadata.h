#ifndef Corrade_PluginManager_PluginMetadata_h
#define Corrade_PluginManager_PluginMetadata_h
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
 * @brief Class @ref Corrade::PluginManager::PluginMetadata
 */

#include <string>
#include <vector>

#include "Corrade/PluginManager/corradePluginManagerVisibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin metadata

This class stores metadata about particular plugin. The plugin metadata are
stored in plugin configuration file, which resides either besides the dynamic
plugin binary in a filesystem or is compiled directly into executable with an
static plugin. See @ref plugin-management for tutorial and brief introduction
into how plugins work.

The plugin configuration file has an simple syntax (see @ref Utility::Configuration
class documentation for full specification). The file stores list of
dependencies (if the plugin depends on another), list of replaced plugins (if
the plugin can replace plugin and provide the same or better functionality) and
optionally plugin-specific configuration. Example `Matrix.conf` file for
`Matrix` plugin:

    # Dependencies
    depends=SomeRandomJohnDoesPlugin
    depends=BaseMatrixPlugin
    depends=SkyNetPlugin

    # Replaced plugins
    replaces=CrashingMatrixPlugin
    replaces=AlphaMatrixPlugin

    # Optional plugin-specific data
    [data]
    description=My first matrix without bugs

*/
class CORRADE_PLUGINMANAGER_EXPORT PluginMetadata {
    friend class AbstractManager;

    public:
        /** @brief Plugin name */
        std::string name() const;

        /**
         * @brief Plugins on which this plugin depend
         *
         * List of plugins which must be loaded before this plugin can be
         * loaded. See also @ref PluginMetadata::replaces().
         * @note Thus field is constant during whole plugin lifetime.
         */
        const std::vector<std::string>& depends() const { return _depends; }

        /**
         * @brief Plugins which depend on this plugin
         *
         * List of plugins which uses this plugin. This plugin cannot be
         * unloaded when any of these plugins are loaded.
         * @note This list is automatically created by plugin manager and can
         *      be changed in plugin lifetime.
         */
        std::vector<std::string> usedBy() const;

        /**
         * @brief Plugins which are replaced with this plugin
         *
         * Plugins which depends on them can be used with this plugin. The
         * plugin cannot be loaded when any of the replaced plugins are loaded.
         * @note Thus field is constant during whole plugin lifetime.
         */
        const std::vector<std::string>& replaces() const { return _replaces; }

        /**
         * @brief Plugins which replaces this plugin
         *
         * List of plugins which can replace this plugin. Every plugin which
         * depends on this plugin would work also with these.
         * @note This list is automatically created by plugin manager and can
         *      change in plugin lifetime.
         */
        std::vector<std::string> replacedWith() const;

        /**
         * @brief Plugin-specific data
         *
         * Additional plugin-specific data, contained in `data` group of plugin
         * configuration.
         */
        const Utility::ConfigurationGroup& data() const { return *_data; }

    private:
        explicit PluginMetadata(std::string name, Utility::ConfigurationGroup& conf);

        std::string _name;

        std::vector<std::string> _depends,
            _usedBy,
            _replaces,
            _replacedWith;

        const Utility::ConfigurationGroup* _data;
};

}}

#endif
