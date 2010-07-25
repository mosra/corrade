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

#include "PluginMetadata.h"
#include "AbstractPluginManager.h"

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
 * @todo Print out errors to stderr
 */
template<class T> class PluginManager: public AbstractPluginManager {
    private:
        std::string pluginDirectory;

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

        LoadState load(const std::string& name);
        LoadState unload(const std::string& name);

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
