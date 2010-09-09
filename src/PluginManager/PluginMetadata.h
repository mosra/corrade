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
         * @note This field is constant during whole plugin lifetime.
         */
        std::string name;

        /**
         * @brief Plugin description
         *
         * More detailed description of plugin.
         * @note This field is constant during whole plugin lifetime.
         */
        std::string description;

        /**
         * @brief Plugins on which this plugin depend
         *
         * List of plugins which must be loaded before this plugin can be
         * loaded. See also PluginMetadata::replaced.
         * @note Thus field is constant during whole plugin lifetime.
         */
        std::vector<std::string> depends;

        /**
         * @brief Plugins which depend on this plugin
         *
         * List of plugins which uses this plugin. This plugin cannot be
         * unloaded when any of these plugins are loaded.
         * @note This list is automatically created by plugin manager and can
         *      be changed in plugin lifetime.
         */
        std::vector<std::string> usedBy;

        /**
         * @brief Plugins which are replaced with this plugin
         *
         * Plugins which depends on them can be used with this plugin. The
         * plugin cannot be loaded when any of the replaced plugins are loaded.
         * @note Thus field is constant during whole plugin lifetime.
         */
        std::vector<std::string> replaces;

        /**
         * @brief Plugins which replaces this plugin
         *
         * List of plugins which can replace this plugin. Every plugin which
         * depends on this plugin would work also with these.
         * @note This list is automatically created by plugin manage and can
         *      change in plugin lifetime.
         */
        std::vector<std::string> replacedWith;

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
