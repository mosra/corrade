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
#include "Utility/Resource.h"

/** @brief Plugin version */
#define PLUGIN_VERSION 1

namespace Map2X { namespace PluginManager {

class Plugin;

/**
 * @brief Base class of PluginManager
 *
 * Base abstract class for all PluginManager templated classes. See also
 * @ref PluginManagement.
 * @todo Resolving dependecies, updating PluginMetadata with reversed deps
 * @todo Casting pointer-to-object to pointer-to-function is not ISO C++ (see
 *      C++ Standard Core Language Active Issue #195,
 *      http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#195 )
 * @todo Print out errors to stderr
 * @todo Provide destructing function with plugin (new/delete overloads, segfaults...)
 */
class AbstractPluginManager {
    friend class Plugin;

    public:
        /**
         * @brief Load state
         *
         * Describes state of the plugin. States before Unknown state are used
         * when loading plugins, states after are used when unloading plugins.
         * Static plugins are loaded at first, they have always state
         * PluginMetadataStatic::IsStatic. Dynamic plugins have at first state
         * NotLoaded, after first attempt to load the state is changed.
         */
        enum LoadState {
            /** %Plugin cannot be found */
            NotFound = 0x0001,

            /**
             * The plugin is build with different version of PluginManager and
             * cannot be loaded. This means the PluginMetadata are inaccessible.
             */
            WrongPluginVersion = 0x0002,

            /**
             * The plugin uses different interface than the interface
             * used by PluginManager and cannot be loaded.
             */
            WrongInterfaceVersion = 0x0004,

            /**
             * The plugin depends on another plugin, which cannot be loaded
             * (e.g. not found, conflict, wrong version).
             */
            UnresolvedDependency = 0x0010,

            /** %Plugin failed to load */
            LoadFailed = 0x0020,

            /** %Plugin is successfully loaded */
            LoadOk = 0x0040,

            /**
             * %Plugin is not loaded. %Plugin can be unloaded only
             * if is dynamic and is not required by any other plugin.
             */
            NotLoaded = 0x0100,

            /** %Plugin failed to unload */
            UnloadFailed = 0x0200,

            /**
             * %Plugin cannot be unloaded because another plugin is depending on
             * it. Unload that plugin first and try again.
             */
            IsRequired = 0x0400,

            /** %Plugin is static (and cannot be unloaded) */
            IsStatic = 0x0800,

            /**
             * %Plugin has active instance and cannot be unloaded. Destroy all
             * instances and try again.
             */
            IsUsed = 0x1000
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
        static void importStaticPlugin(const std::string& name, int _version, std::string (*interface)(), void* (*instancer)(AbstractPluginManager*, const std::string&));

        /**
         * @brief Constructor
         * @param pluginDirectory   Directory where plugins will be searched,
         *      with tailing slash. No recursive processing is done.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this PluginManager instance. The gets list of
         * all dynamic plugins in given directory.
         * @see PluginManager::nameList()
         * @todo Plugin dir without trailing slash.
         * @todo Make static plugin have higher priority than dynamic
         */
        AbstractPluginManager(const std::string& pluginDirectory);

        /**
         * @brief Destructor
         *
         * Destroys all plugin instances and unload all plugins.
         */
        virtual ~AbstractPluginManager();

        /** @brief Plugin directory */
        inline std::string pluginDirectory() const { return _pluginDirectory; }

        /** @brief List of all available plugin names */
        std::vector<std::string> nameList() const;

        /**
         * @brief Try to load all plugins
         *
         * Alphabetically goes through list and tries to load plugins. Doesn't
         * do any conflict resolving, whichever plugin was first, that plugin
         * will be loaded and any conflicting plugins found after will be
         * skipped.
         * @see PluginManager::load()
         */
        virtual void loadAll();

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
        inline LoadState loadState(const std::string& name) const {
            /* Plugin with given name doesn't exist */
            if(plugins.find(name) == plugins.end()) return NotFound;

            return plugins.at(name).loadState;
        }

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
        virtual LoadState load(const std::string& name);

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
        virtual LoadState unload(const std::string& name);

    protected:
        #ifndef DOXYGEN_GENERATING_OUTPUT
        struct StaticPlugin {
            std::string name;
            std::string interface;
            void* (*instancer)(AbstractPluginManager*, const std::string&);
        };

        struct PluginObject {
            LoadState loadState;
            PluginMetadata metadata;
            void* (*instancer)(AbstractPluginManager*, const std::string&);
            void* handle;
            PluginObject(const Utility::Configuration& _metadata):
                loadState(NotLoaded), metadata(_metadata), handle(0) {}
        };

        static std::vector<StaticPlugin> staticPlugins;
        std::string _pluginDirectory;
        std::map<std::string, PluginObject> plugins;
        #endif

        /** @brief Plugin interface used by the plugin manager */
        virtual std::string pluginInterface() const = 0;

    private:
        std::map<std::string, std::vector<Plugin*> > instances;

        void registerInstance(const std::string& name, Plugin* instance);
        void unregisterInstance(const std::string& name, Plugin* instance);
};

/**
 * @brief Import static plugin
 * @param name      Static plugin name (defined with PLUGIN_REGISTER_STATIC())
 * @hideinitializer
 *
 * If static plugins are compiled into dynamic library or directly into the
 * executable, they should be automatically loaded at startup thanks to
 * AUTOMATIC_INITALIZER() and AUTOMATIC_FINALIZER() macros.
 *
 * If static plugins are compiled into static library, they are not
 * automatically loaded at startup, so you need to load them explicitly by
 * calling PLUGIN_IMPORT() at the beggining of main() function. You can also
 * wrap these macro calls into another function (which will then be compiled
 * into dynamic library or main executable) and use AUTOMATIC_INITIALIZER()
 * macro for automatic call.
 * @attention This macro should be called outside of any namespace. If you are
 * running into linker errors with @c pluginInitializer_*, this could be the
 * problem. See RESOURCE_INITIALIZE() documentation for more information.
 */
#define PLUGIN_IMPORT(name)                                                   \
    extern int pluginInitializer_##name();                                    \
    pluginInitializer_##name();                                               \
    RESOURCE_INITIALIZE(name)

}}

#endif
