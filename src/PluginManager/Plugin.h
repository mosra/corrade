#ifndef Map2X_PluginManager_AbstractPlugin_h
#define Map2X_PluginManager_AbstractPlugin_h
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
 * @brief Class Map2X::PluginManager::AbstractPlugin and plugin registering macros
 *
 * Macros PLUGIN_INTERFACE(), PLUGIN_REGISTER_STATIC(), PLUGIN_REGISTER(),
 * ::PLUGIN_FINISH, PLUGIN_SET_NAME(), PLUGIN_SET_DESCRIPTION(),
 * PLUGIN_ADD_DEPENDENCY(), PLUGIN_ADD_CONFLICT(), PLUGIN_ADD_REPLACED(),
 * PLUGIN_IMPORT_STATIC().
 */

#include <string>

namespace Map2X { namespace PluginManager {

class AbstractPluginManager;

/**
 * @brief Base class for plugin interfaces
 *
 * Connects every plugin instance to parent plugin manager to ensure the
 * plugin can be unloaded only if there are no active instances.
 * @todo Make constructors/destructors private
 */
class Plugin {
    public:
        /**
         * @brief Constructor
         * @param manager       Parent plugin manager
         * @param plugin        Plugin name
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

    private:
        AbstractPluginManager* _manager;
        std::string _plugin;
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
 * @param _interface    Interface name (the same as is defined with
 *      PLUGIN_INTERFACE() in plugin base class)
 * @hideinitializer
 *
 * Registers plugin so it can be dynamically loaded via PluginManager by
 * supplying a filename of the plugin module. Macro PLUGIN_FINISH must be
 * called too.
 */
#define PLUGIN_REGISTER(className, _interface) \
    extern "C" int pluginVersion() { return PLUGIN_VERSION; } \
    extern "C" void* pluginInstancer(Map2X::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); } \
    extern "C" void pluginMetadataCreator(Map2X::PluginManager::PluginMetadata* metadata) { \
        metadata->interface = _interface;

/**
 * @brief Register static plugin
 * @param name          Name of static plugin (equivalent of dynamic plugin
 *      filename)
 * @param className     Plugin class name
 * @param _interface    Interface name (the same as is defined with
 *      PLUGIN_INTERFACE() in plugin base class)
 * @hideinitializer
 *
 * Registers static plugin. It will be loaded automatically when PluginManager
 * instance with corresponding interface is created. Macro PLUGIN_FINISH
 * must be called too.
 */
#define PLUGIN_REGISTER_STATIC(name, className, _interface) \
    inline int name##Version() { return PLUGIN_VERSION; } \
    inline void* name##Instancer(Map2X::PluginManager::AbstractPluginManager* manager, const std::string& plugin) \
        { return new className(manager, plugin); } \
    inline void name##MetadataCreator(Map2X::PluginManager::PluginMetadata* metadata) { \
        metadata->interface = _interface;

/** @{ @name Plugin metadata
 * These macros are optional, but must be called between PLUGIN_REGISTER() or
 * PLUGIN_REGISTER_STATIC() and ::PLUGIN_FINISH macro.
 */

/** @hideinitializer @brief Set plugin name */
#define PLUGIN_SET_NAME(_name)                  metadata->name = _name;

/** @hideinitializer @brief Set plugin description */
#define PLUGIN_SET_DESCRIPTION(_description)    metadata->description = _description;

/** @hideinitializer @brief Add plugin dependency */
#define PLUGIN_ADD_DEPENDENCY(_dependency)      metadata->depends.push_back(_dependency);

/** @hideinitializer @brief Add plugin conflict */
#define PLUGIN_ADD_CONFLICT(_conflict)          metadata->conflicts.push_back(_conflict);

/** @hideinitializer @brief Add replaced plugin */
#define PLUGIN_ADD_REPLACED(_replaced)          metadata->replaces.push_back(_replaced);

/*@}*/

/**
 * @brief Finish plugin registration
 *
 * Finishes plugin registration started with PLUGIN_REGISTER()
 * or PLUGIN_REGISTER_STATIC().
 */
#define PLUGIN_FINISH }

}}

#endif
