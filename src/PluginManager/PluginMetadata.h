#ifndef Map2X_Plugins_PluginMetadata_h
#define Map2X_Plugins_PluginMetadata_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Map2X::PluginManager::PluginMetadata
 */

#include <string>
#include <vector>

namespace Map2X { namespace PluginManager {

/** @brief Plugin version */
#define PLUGIN_VERSION 1

/**
 * @brief Plugin metadata
 *
 * This class stores metadata about particular plugin, mainly the plugin
 * interface name.
 * See also @ref PluginManagement.
 */
struct PluginMetadata {
        /**
         * @brief Plugin interface
         *
         * Plugin must have the same interface name as interface used by
         * PluginManager, otherwise it cannot be loaded.
         */
        std::string interface;

        /**
         * @brief Plugin name
         *
         * Descriptive name of plugin. Not to be confused with name under which
         * the plugin is loaded.
         */
        std::string name;

        /**
         * @brief Plugin description
         *
         * More detailed description of plugin.
         */
        std::string description;

        /** @brief Plugin dependencies */
        std::vector<std::string> depends;

        /**
         * @brief Replaced plugins
         *
         * Plugins which are replaced with this plugin. Plugins which depends on
         * them can be used with this plugin. The plugin cannot be loaded when
         * any of the replaced plugins are loaded.
         */
        std::vector<std::string> replaces;

        /**
         * @brief Conflicting plugins
         *
         * The plugin cannot be loaded when any of the conflicting plugins are
         * loaded.
         */
        std::vector<std::string> conflicts;
};

}}

#endif
