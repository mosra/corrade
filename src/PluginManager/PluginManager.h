#ifndef Map2X_Plugins_PluginManager_h
#define Map2X_Plugins_PluginManager_h
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
 * @brief Class Map2X::PluginManager::PluginManager
 */

#include <string>
#include <vector>
#include <map>

#include "PluginMetadata.h"
#include "PluginManagerStatic.h"

namespace Map2X { namespace PluginManager {

/**
 * @brief Plugin manager
 *
 * Manages loading, instancing and unloading plugins.
 * See also @ref PluginManagement.
 * @todo Resolving dependecies, updating PluginMetadata with reversed deps
 * @todo Casting pointer-to-object to pointer-to-function is not ISO C++ (see
 *      C++ Standard Core Language Active Issue #195,
 *      http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#195 )
 * @todo Destructor, unloading at destroy
 */
template<class T> class PluginManager: public PluginManagerStatic {
    private:
        std::string pluginDirectory;
        std::map<std::string, Plugin> plugins;

    public:
        /**
         * @brief Constructor
         * @param _pluginDirectory  Directory where plugins will be searched,
         *      with tailing slash. No recursive processing is done.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this PluginManager instance. The gets list of
         * all dynamic plugins in given directory.
         * @see PluginManager::nameList()
         */
        PluginManager(const std::string& _pluginDirectory);

        /** @brief List of all available plugin names */
        inline std::vector<std::string> nameList() const;

        /**
         * @brief Try to load all plugins
         *
         * Alphabetically goes through list and tries to load plugins. Does not
         * any conflict resolving, whichever plugin was first, that plugin will
         * be loaded and any conflicting plugins loaded after will be skipped.
         * @see PluginManager::load()
         */
        void loadAll();

        /**
         * @brief Plugin metadata
         * @param name              Plugin name
         * @return Pointer to plugin metadata
         */
        const PluginMetadata* metadata(const std::string& name);

        /**
         * @brief Load state of a plugin
         * @param name              Plugin name
         * @return Load state of a plugin
         *
         * Static plugins always have PluginManangerStatic::Static state.
         */
        PluginManagerStatic::LoadState loadState(const std::string& name);

        /**
         * @brief Load a plugin
         * @param name              Plugin name
         * @return PluginManagerStatic::LoadOk on success,
         *      PluginManagerStatic::NotFound,
         *      PluginManagerStatic::WrongPluginVersion,
         *      PluginManagerStatic::WrongInterfaceVersion,
         *      PluginManagerStatic::Conflicts,
         *      PluginManagerStatic::UnresolvedDependency or
         *      PluginManagerStatic::LoadFailed  on failure.
         *
         * Checks whether a plugin is loaded, if not and loading is possible,
         * tries to load it. If the plugin has any dependencies, they are
         * recursively processed before loading given plugin.
         */
        PluginManagerStatic::LoadState load(const std::string& name);

        /**
         * @brief Unload a plugin
         * @param name              Plugin name
         * @return PluginManagerStatic::UnloadOk on success,
         *      PluginManagerStatic::UnloadFailed,
         *      PluginManagerStatic::IsRequired or
         *      PluginManagerStatic::IsStatic on failure.
         *
         * Checks whether a plugin is loaded, if yes, tries to unload it. If the
         * plugin is not loaded, returns its current load state.
         */
        PluginManagerStatic::LoadState unload(const std::string& name);

        /**
         * @brief Plugin class instance
         * @param name              Plugin name
         * @return Pointer to new instance of plugin class, zero on error
         *
         * Creates new instance of plugin class, if possible. If the plugin is
         * not successfully loaded, returns zero pointer.
         */
        T* instance(const std::string& name);
};

}}

#ifndef Map2X_PluginManager_PluginManager_cpp
#include "PluginManager.cpp"
#endif

#endif
