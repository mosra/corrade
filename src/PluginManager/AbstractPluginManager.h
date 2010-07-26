#ifndef Map2X_PluginManager_AbstractPluginManager_h
#define Map2X_PluginManager_AbstractPluginManager_h
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
 * @brief Class Map2X::PluginManager::AbstractPluginManager
 */

#include <vector>
#include <string>
#include <map>

#include "PluginMetadata.h"

namespace Map2X { namespace PluginManager {

/**
 * @brief Base class of PluginManager
 *
 * Base abstract class for all PluginManager templated classes. See also
 * @ref PluginManagement.
 * @todo Resolving dependecies, updating PluginMetadata with reversed deps
 * @todo Casting pointer-to-object to pointer-to-function is not ISO C++ (see
 *      C++ Standard Core Language Active Issue #195,
 *      http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#195 )
 * @todo Destructor, unloading at destroy
 * @todo Print out errors to stderr
 */
class AbstractPluginManager {
    public:
        /**
         * @brief Load state
         *
         * Describes state of the plugin. Values below zero are used when
         * loading plugin, values above zero when unloading plugin. Static
         * plugins are loaded at first, they have always state
         * PluginMetadataStatic::IsStatic. Dynamic plugins have at first state
         * NotLoaded, after first attempt to load the state is changed.
         */
        enum LoadState {
            /** %Plugin cannot be found */
            NotFound = -7,

            /**
             * The plugin is build with different version of PluginManager and
             * cannot be loaded. This means the PluginMetadata are inaccessible.
             */
            WrongPluginVersion = -6,

            /**
             * The plugin uses different interface than the interface
             * used by PluginManager and cannot be loaded.
             */
            WrongInterfaceVersion = -5,

            /**
             * The plugin is conflicting with another plugin that is already
             * loaded. Unload conflicting plugin and try it again.
             */
            Conflicts = -4,

            /**
             * The plugin depends on another plugin, which cannot be loaded
             * (e.g. not found, conflict, wrong version).
             */
            UnresolvedDependency = -3,

            /** %Plugin failed to load */
            LoadFailed = -2,

            /** %Plugin is successfully loaded */
            LoadOk = -1,

            /** %Plugin is not yet loaded and its state is unknown */
            Unknown = 0,

            /**
             * %Plugin is not loaded. %Plugin can be unloaded only
             * if is dynamic and is not required by any other plugin.
             */
            NotLoaded = 1,

            /** %Plugin failed to unload */
            UnloadFailed = 2,

            /**
             * %Plugin cannot be unloaded because another plugin is depending on
             * it. Unload that plugin first and try again.
             */
            IsRequired = 3,

            /** %Plugin is static (and cannot be unloaded) */
            IsStatic = 4
        };

        /** @brief %Plugin version */
        static const int version = PLUGIN_VERSION;

        /**
         * @brief Register static plugin
         * @param name              Static plugin name (defined with
         *      PLUGIN_REGISTER_STATIC())
         * @param _version          %Plugin version (must be the same as
         *      AbstractPluginManager::version)
         * @param metadataCreator   Pointer to metadata creator function
         * @param instancer         Pointer to plugin class instancer function
         *
         * Used internally by PLUGIN_IMPORT_STATIC() macro. There is absolutely
         * no need to use this directly.
         */
        static void importStaticPlugin(const std::string& name, int _version, void (*metadataCreator)(PluginMetadata*), void* (*instancer)());

        /**
         * @brief Constructor
         * @param pluginDirectory   Directory where plugins will be searched,
         *      with tailing slash. No recursive processing is done.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this PluginManager instance. The gets list of
         * all dynamic plugins in given directory.
         * @see PluginManager::nameList()
         */
        AbstractPluginManager(const std::string& pluginDirectory);

        /** @brief Plugin directory */
        std::string pluginDirectory() const { return _pluginDirectory; }

        /** @brief List of all available plugin names */
        std::vector<std::string> nameList() const;

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
        LoadState loadState(const std::string& name);

        /**
         * @brief Load a plugin
         * @param name              Plugin name
         * @return AbstractPluginManager::LoadOk on success,
         *      AbstractPluginManager::NotFound,
         *      AbstractPluginManager::WrongPluginVersion,
         *      AbstractPluginManager::WrongInterfaceVersion,
         *      AbstractPluginManager::Conflicts,
         *      AbstractPluginManager::UnresolvedDependency or
         *      AbstractPluginManager::LoadFailed  on failure.
         *
         * Checks whether a plugin is loaded, if not and loading is possible,
         * tries to load it. If the plugin has any dependencies, they are
         * recursively processed before loading given plugin.
         */
        LoadState load(const std::string& name);

        /**
         * @brief Unload a plugin
         * @param name              Plugin name
         * @return AbstractPluginManager::UnloadOk on success,
         *      AbstractPluginManager::UnloadFailed,
         *      AbstractPluginManager::IsRequired or
         *      AbstractPluginManager::IsStatic on failure.
         *
         * Checks whether a plugin is loaded, if yes, tries to unload it. If the
         * plugin is not loaded, returns its current load state.
         */
        LoadState unload(const std::string& name);

    protected:
        #ifndef DOXYGEN_GENERATING_OUTPUT
        struct StaticPlugin {
            std::string name;
            void (*metadataCreator)(PluginMetadata*);
            void* (*instancer)();
        };

        struct Plugin {
            LoadState loadState;
            PluginMetadata metadata;
            void* (*instancer)();
            void* handle;
            Plugin(): loadState(Unknown), handle(0) {}
        };

        static std::vector<StaticPlugin> staticPlugins;
        std::string _pluginDirectory;
        std::map<std::string, Plugin> plugins;
        #endif

        /** @brief Plugin interface used by the plugin manager */
        virtual std::string pluginInterface() const = 0;
};

/**
 * @brief Import static plugin
 * @param name      Static plugin name (defined with PLUGIN_REGISTER_STATIC())
 * @hideinitializer
 *
 * Imports statically linked plugin and makes it available in
 * PluginManager. The plugin must be registered with PLUGIN_STATIC_REGISTER()
 * macro, otherwise it will not be loaded.
 */
#define PLUGIN_IMPORT_STATIC(name) \
    Map2X::PluginManager::AbstractPluginManager::importStaticPlugin(#name, name##Version(), name##MetadataCreator, name##Instancer);

}}

#endif
