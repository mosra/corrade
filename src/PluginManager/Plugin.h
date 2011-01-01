#ifndef Kompas_PluginManager_AbstractPlugin_h
#define Kompas_PluginManager_AbstractPlugin_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Kompas::PluginManager::Plugin and plugin registering macros
 *
 * Macros PLUGIN_INTERFACE(), PLUGIN_REGISTER_STATIC(), PLUGIN_REGISTER().
 */

#include <string>

#include "Utility/utilities.h"
#include "AbstractPluginManager.h"

namespace Kompas { namespace PluginManager {

/**
 * @brief Base class for plugin interfaces
 *
 * Connects every plugin instance to parent plugin manager to ensure the
 * plugin can be unloaded only if there are no active instances.
 */
class PLUGINMANAGER_EXPORT Plugin {
    public:
        /**
         * @brief Constructor
         * @param manager       Parent plugin manager
         * @param plugin        %Plugin name
         *
         * Registers this plugin instance in plugin manager.
         */
        Plugin(AbstractPluginManager* manager = 0, const std::string& plugin = "");

        /**
         * @brief Destructor
         *
         * Unregisters this plugin instance in plugin manager.
         */
        virtual ~Plugin();

        /**
         * @brief Plugin identifier
         * @return String, under which the plugin was instanced. If the plugin
         *      was not instanced via plugin manager, returns empty string.
         */
        inline std::string plugin() const { return _plugin; }

    public:
        /**
         * @brief Plugin metadata
         * @return Metadata file associated with the plugin or 0, if no metadata
         *      is available.
         */
        inline const PluginMetadata* metadata() const { return _metadata; }

    protected:
        /**
         * @brief Plugin configuration
         * @return Configuration from file associated with the plugin or 0, if
         *      no configuration is available.
         */
        inline const Utility::Configuration* configuration() const { return _configuration; }

    private:
        AbstractPluginManager* _manager;
        std::string _plugin;
        const Utility::Configuration* _configuration;
        const PluginMetadata* _metadata;
};

/**
 * @brief Define plugin interface
 * @param name          Interface name
 * @hideinitializer
 *
 * This macro is called in class definition and makes that class usable as
 * plugin interface. Plugins using that interface must have exactly the same
 * interface name, otherwise they will not be loaded. A good practice
 * is to use "Java package name"-style syntax for version name, because this
 * makes the name as unique as possible. The interface name should also contain
 * version identifier to make sure the plugin will not be loaded with
 * incompatible interface version.
 */
#define PLUGIN_INTERFACE(name) \
    public: inline static std::string pluginInterface() { return name; } private:

/**
 * @brief Register dynamic plugin
 * @param className     Plugin class name
 * @param interface     Interface name (the same as is defined with
 *      PLUGIN_INTERFACE() in plugin base class)
 * @hideinitializer
 *
 * Registers plugin so it can be dynamically loaded via PluginManager by
 * supplying a filename of the plugin module.
 */
#define PLUGIN_REGISTER(className, interface) \
    extern "C" int pluginVersion() { return PLUGIN_VERSION; } \
    extern "C" void* pluginInstancer(Kompas::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); } \
    extern "C" std::string pluginInterface() { return interface; }

/**
 * @brief Register static plugin
 * @param name          Name of static plugin (equivalent of dynamic plugin
 *      filename)
 * @param className     Plugin class name
 * @param interface     Interface name (the same as is defined with
 *      PLUGIN_INTERFACE() in plugin base class)
 * @hideinitializer
 *
 * Registers static plugin. It will be loaded automatically when PluginManager
 * instance with corresponding interface is created.
 * @attention This macro should be called outside of any namespace. If you are
 * running into linker errors with @c pluginInitializer_*, this could be the
 * problem.
 * @todo Get rid of AUTOMATIC_INITIALIZER() -- PLUGIN_REGISTER_STATIC should be
 * only in *.cpp
 */
#define PLUGIN_REGISTER_STATIC(name, className, interface) \
    inline void* pluginInstancer_##name(Kompas::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); } \
    int pluginInitializer_##name() { \
        Kompas::PluginManager::AbstractPluginManager::importStaticPlugin(#name, PLUGIN_VERSION, interface, pluginInstancer_##name); return 1; \
    } AUTOMATIC_INITIALIZER(pluginInitializer_##name)

}}

#endif
