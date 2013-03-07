#ifndef Corrade_PluginManager_Plugin_h
#define Corrade_PluginManager_Plugin_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Corrade::PluginManager::Plugin and plugin registering macros
 *
 * Macros PLUGIN_INTERFACE(), PLUGIN_REGISTER().
 */

#include <string>

#include "Utility/utilities.h"
#include "AbstractPluginManager.h"

namespace Corrade { namespace PluginManager {

/**
 * @brief Base class for plugin interfaces
 *
 * Connects every plugin instance to parent plugin manager to ensure the
 * plugin can be unloaded only if there are no active instances.
 */
class CORRADE_PLUGINMANAGER_EXPORT Plugin {
    public:
        /**
         * @brief Default constructor
         *
         * Usable when using the plugin directly, without plugin manager. Define
         * this constructor in your subclass only if you want to allow using the
         * interface or plugin without plugin manager.
         */
        explicit Plugin();

        /**
         * @brief Plugin manager constructor
         *
         * Used by plugin manager. Don't forget to redefine this constructor in
         * all your subclasses.
         * @see plugin(), metadata(), configuration()
         */
        explicit Plugin(AbstractPluginManager* manager, std::string plugin);

        /**
         * @brief Destructor
         *
         * If instantiated through plugin manager, unregisters this instance
         * from it.
         */
        virtual ~Plugin();

        /**
         * @brief Whether the plugin can be deleted
         *
         * Called from PluginManager on all active instances before the plugin
         * is unloaded. Returns `true` if it is safe to delete the instance from
         * the manager, `false` if not. If any instance returns `false`, the
         * plugin is not unloaded.
         */
        virtual bool canBeDeleted() { return false; }

        /**
         * @brief Identifier string
         *
         * Name under which the plugin was instanced. If the plugin was not
         * instantiated via plugin manager, returns empty string.
         */
        inline std::string plugin() const { return _plugin; }

        /**
         * @brief Metadata
         *
         * Metadata associated with given plugin. If the plugin was not
         * instantiated through plugin manager, returns `nullptr`.
         */
        inline const PluginMetadata* metadata() const { return _metadata; }

    protected:
        /**
         * @brief Configuration
         *
         * Configuration associated with given plugin. If the plugin was not
         * instantiated through plugin manager, returns `nullptr`.
         * @todo Make use of this, change to pointer to ConfigurationGroup
         */
        inline const Utility::Configuration* configuration() const { return _configuration; }

    private:
        AbstractPluginManager* _manager;
        std::string _plugin;
        const Utility::Configuration* _configuration;
        const PluginMetadata* _metadata;
};

/** @brief Plugin version */
#define PLUGIN_VERSION 2

/**
@brief Define plugin interface
@param name          Interface name
@hideinitializer

This macro is called in class definition and makes that class usable as
plugin interface. Plugins using that interface must have exactly the same
interface name, otherwise they will not be loaded. A good practice
is to use "Java package name"-style syntax for version name, because this
makes the name as unique as possible. The interface name should also contain
version identifier to make sure the plugin will not be loaded with
incompatible interface version.
 */
#define PLUGIN_INTERFACE(name) \
    public: inline static std::string pluginInterface() { return name; } private:

/**
@brief Register static or dynamic lugin
@param name          Name of static plugin (equivalent of dynamic plugin
     filename)
@param className     Plugin class name
@param interface     Interface name (the same as is defined with
     PLUGIN_INTERFACE() in plugin base class)
@hideinitializer

If the plugin is built as **static** (using CMake command
`corrade_add_static_plugin`), registers it, so it will be loaded automatically
when PluginManager instance with corresponding interface is created. When
building as static plugin, `CORRADE_STATIC_PLUGIN` preprocessor directive is
defined.

If the plugin is built as **dynamic** (using CMake command
`corrade_add_plugin`), registers it, so it can be dynamically loaded via
PluginManager by supplying a name of the plugin. When building as dynamic
plugin, `CORRADE_DYNAMIC_PLUGIN` preprocessor directive is defined.

If the plugin is built as dynamic or static **library or executable** (not as
plugin, using e.g. CMake command `add_library` / `add_executable`), this macro
won't do anything to prevent linker issues when linking more plugins together.
No plugin-related preprocessor directive is defined.

See @ref plugin-management for more information about plugin compilation.

@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `pluginInitializer_`, this could be the
    problem.
*/
#ifdef CORRADE_STATIC_PLUGIN
#define PLUGIN_REGISTER(name, className, interface)                         \
    inline void* pluginInstancer_##name(Corrade::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); }                          \
    int pluginInitializer_##name() {                                        \
        Corrade::PluginManager::AbstractPluginManager::importStaticPlugin(#name, PLUGIN_VERSION, interface, pluginInstancer_##name); return 1; \
    }
#else
#ifdef CORRADE_DYNAMIC_PLUGIN
#define PLUGIN_REGISTER(name, className, interface)                         \
    extern "C" CORRADE_PLUGIN_EXPORT int pluginVersion() { return PLUGIN_VERSION; }               \
    extern "C" CORRADE_PLUGIN_EXPORT void* pluginInstancer(Corrade::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); }                          \
    extern "C" CORRADE_PLUGIN_EXPORT const char* pluginInterface() { return interface; }
#else
#define PLUGIN_REGISTER(name, className, interface)
#endif
#endif

}}

#endif
