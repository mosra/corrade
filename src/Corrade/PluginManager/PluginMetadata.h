#ifndef Corrade_PluginManager_PluginMetadata_h
#define Corrade_PluginManager_PluginMetadata_h
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

/** @file
 * @brief Class @ref Corrade::PluginManager::PluginMetadata
 */

#include <string>
#include <vector>

#include "Corrade/PluginManager/PluginManager.h"
#include "Corrade/PluginManager/visibility.h"
#include "Corrade/Utility/Utility.h"

namespace Corrade { namespace PluginManager {

/**
@brief Plugin metadata

This class stores metadata about a particular plugin. The plugin metadata are
stored in a plugin metadata file, which resides either besides the dynamic
plugin binary in a filesystem or is compiled directly into executable with a
static plugin. See @ref plugin-management for a tutorial and brief introduction
to how plugins work.

The plugin metadata file has a simple INI-like syntax (see
@ref Utility::Configuration class documentation for full specification). The
file stores list of dependencies (if the plugin depends on another), list of
aliases and optionally plugin-specific data and configuration. Example
`Matrix.conf` file for a `Matrix` plugin:

@code{.ini}
# Dependencies
depends=SomeRandomJohnDoesPlugin
depends=BaseMatrixPlugin
depends=SkyNetPlugin

# Aliases
provides=RealWorld
provides=RealButSlightlyTwistedWorld

# Optional plugin-specific data
[data]
description=My first matrix without bugs

# Optional plugin-specific configuration
[configuration]
redPillOrBluePill=red
@endcode

According to the configuration file, the `Matrix` plugin can be loaded only if
`SomeRandomJohnDoesPlugin`, `BaseMatrixPlugin` and `SkyNetPlugin` are found can
be loaded. It will be also loaded when requesting `RealWorld` plugin, but only
if this is the first plugin providing it.

The @cb{.ini} [data] @ce section can contain read-only data that can be used
for example to provide additional info about the plugin in a user interface and
is accessible through @ref data().

The @cb{.ini} [configuration] @ce section contains initial configuration data
that can be modified by the user to setup a particular plugin instance beyond
what's possible via the plugin interface. The initial configuration is
accessible through @ref configuration(), while a modifiable copy is stored
in each plugin instance, accessible through @ref AbstractPlugin::configuration().
*/
class CORRADE_PLUGINMANAGER_EXPORT PluginMetadata {
    public:
        /** @brief Copying is not allowed */
        PluginMetadata(const PluginMetadata&) = delete;

        /** @brief Moving is not allowed */
        PluginMetadata(PluginMetadata&&) = delete;

        /** @brief Copying is not allowed */
        PluginMetadata& operator=(const PluginMetadata&) = delete;

        /** @brief Moving is not allowed */
        PluginMetadata& operator=(PluginMetadata&&) = delete;

        /** @brief Plugin name */
        std::string name() const;

        /**
         * @brief Plugins on which this plugin depend
         *
         * List of plugins which must be loaded before this plugin can be
         * loaded.
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
         * @brief Plugins which are provided by this plugin
         *
         * List of plugin names that are alias to this plugin when loading the
         * plugin by name (not as dependency) if there is no plugin with that
         * name. If there is more than one alias for given name, the first
         * found is used.
         * @note Thus field is constant during whole plugin lifetime.
         */
        const std::vector<std::string>& provides() const { return _provides; }

        /**
         * @brief Plugin-specific data
         *
         * Additional plugin-specific data, contained in the
         * @cb{.ini} [data] @ce group of plugin metadata. If the
         * @cb{.ini} [data] @ce group was not present in the metadata, the
         * returned group is empty.
         * @see @ref configuration()
         */
        const Utility::ConfigurationGroup& data() const { return *_data; }

        /**
         * @brief Initial plugin-specific configuration
         *
         * Initial plugin-specific configuration, contained in the
         * @cb{.ini} [configuration] @ce group of plugin configuration. A
         * plugin-local copy is stored in each plugin instance, available
         * through @ref AbstractPlugin::configuration(). Changing the global
         * configuration will affect all plugins instantiated with the same
         * manager, resetting the configuration back can be done by recreating
         * the manager.
         *
         * If the @cb{.ini} [configuration] @ce group was not present in the
         * metadata, the returned group is empty.
         * @see @ref data()
         */
        Utility::ConfigurationGroup& configuration() { return *_configuration; }
        const Utility::ConfigurationGroup& configuration() const { return *_configuration; } /**< @overload */

        #ifndef DOXYGEN_GENERATING_OUTPUT
        explicit PluginMetadata(std::string name, Utility::ConfigurationGroup& conf);
        #endif

    private:
        friend AbstractManager;

        std::string _name;

        std::vector<std::string> _depends,
            _usedBy,
            _provides;

        const Utility::ConfigurationGroup* _data;
        Utility::ConfigurationGroup*_configuration;
};

}}

#endif
