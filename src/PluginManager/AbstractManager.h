#ifndef Corrade_PluginManager_AbstractManager_h
#define Corrade_PluginManager_AbstractManager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
              Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class Corrade::PluginManager::AbstractManager
 */

#include <vector>
#include <string>
#include <map>

#ifdef _WIN32
#include <windows.h>
#undef interface
#endif

#include "corradeCompatibility.h"
#include "Containers/EnumSet.h"
#include "Utility/Configuration.h"
#include "Utility/Debug.h"
#include "Utility/Resource.h"
#include "PluginManager/PluginMetadata.h"
#include "PluginManager/PluginManager.h"

namespace Corrade { namespace PluginManager {

/** @relates AbstractManager
@brief Plugin load state

@see LoadStates, AbstractManager::loadState(),
    AbstractManager::load(), AbstractManager::unload(),
    AbstractManager::reload().
*/
enum class LoadState: unsigned short {
    /**
     * The plugin cannot be found. Returned by AbstractManager::loadState(),
     * AbstractManager::load() and AbstractManager::reload().
     */
    NotFound = 1 << 0,

    #ifndef CORRADE_TARGET_NACL_NEWLIB
    /**
     * The plugin is build with different version of plugin manager and cannot
     * be loaded. Returned by AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    WrongPluginVersion = 1 << 1,

    /**
     * The plugin uses different interface than the interface used by plugin
     * manager and cannot be loaded. Returned by AbstractManager::load()
     * and AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    WrongInterfaceVersion = 1 << 2,

    /**
     * The plugin doesn't have any metadata file or the metadata file contains
     * errors. Returned by AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    WrongMetadataFile = 1 << 3,

    /**
     * The plugin depends on another plugin, which cannot be loaded (e.g. not
     * found or wrong version). Returned by AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    UnresolvedDependency = 1 << 4,

    /**
     * The plugin failed to load for other reason (e.g. linking failure).
     * Returned by AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    LoadFailed = 1 << 5,

    /**
     * The plugin is successfully loaded. Returned by
     * AbstractManager::loadState(), AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    Loaded = 1 << 6,

    /**
     * The plugin is not loaded. Plugin can be unloaded only if is dynamic and
     * is not required by any other plugin. Returned by
     * AbstractManager::loadState(), AbstractManager::load() and
     * AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    NotLoaded = 1 << 7,

    /**
     * The plugin failed to unload. Returned by AbstractManager::unload()
     * and AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    UnloadFailed = 1 << 8,

    /**
     * The plugin cannot be unloaded because another plugin is depending on it.
     * Unload that plugin first and try again. Returned by
     * AbstractManager::unload() and AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    Required = 1 << 9,
    #endif

    /**
     * The plugin is static. Returned by AbstractManager::loadState(),
     * AbstractManager::load(), AbstractManager::reload() and
     * AbstractManager::unload().
     */
    Static = 1 << 10

    #ifndef CORRADE_TARGET_NACL_NEWLIB
    ,
    /**
     * The plugin has active instance and cannot be unloaded. Destroy all
     * instances and try again. Returned by AbstractManager::unload()
     * and AbstractManager::reload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
     */
    Used = 1 << 11
    #endif
};

/** @relates AbstractManager
@brief Plugin load states

Useful when checking whether LoadState in in given set of values, for example:
@code
if(loadState & (LoadState::Loaded|LoadState::Static)) {
    // ...
}
@endcode
@see AbstractManager::loadState(), AbstractManager::load(),
    AbstractManager::unload(), AbstractManager::reload().
*/
typedef Containers::EnumSet<LoadState, unsigned short> LoadStates;

CORRADE_ENUMSET_OPERATORS(LoadStates)

/**
@brief Non-templated base class of Manager

See also @ref plugin-management.
 */
class CORRADE_PLUGINMANAGER_EXPORT AbstractManager {
    friend class AbstractPlugin;

    public:
        /** @brief Plugin version */
        static const int Version;

        #ifndef DOXYGEN_GENERATING_OUTPUT
        typedef void* (*Instancer)(AbstractManager*, const std::string&);
        static void importStaticPlugin(const std::string& plugin, int _version, const std::string& interface, Instancer instancer, void(*initializer)(), void(*finalizer)());
        #endif

        /**
         * @brief Constructor
         * @param pluginDirectory   Directory where plugins will be searched. No
         *      recursive processing is done.
         *
         * First goes through list of static plugins and finds ones that use
         * the same interface as this PluginManager instance. Then gets list of
         * all dynamic plugins in given directory.
         * @note Dependencies of static plugins are skipped, as static plugins
         *      should have all dependencies present. Also, dynamic plugins
         *      with the same name as another static plugin are skipped.
         * @see pluginList()
         * @partialsupport Parameter @p pluginDirectory has no effect on
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib" as only static
         *      plugins are supported.
         */
        explicit AbstractManager(std::string pluginDirectory);

        /** @brief Copying is not allowed */
        AbstractManager(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager(AbstractManager&&) = delete;

        /** @brief Copying is not allowed */
        AbstractManager& operator=(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager& operator=(AbstractManager&&) = delete;

        #ifndef CORRADE_TARGET_NACL_NEWLIB
        /**
         * @brief Plugin directory
         *
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
         */
        std::string pluginDirectory() const;

        /**
         * @brief Set another plugin directory
         *
         * Keeps loaded plugins untouched, removes unloaded plugins which are
         * not existing anymore and adds newly found plugins.
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
         */
        void setPluginDirectory(std::string directory);

        /**
         * @brief Reload plugin directory
         *
         * Convenience equivalent to `setPluginDirectory(pluginDirectory())`.
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
         */
        void reloadPluginDirectory();
        #endif

        /** @brief List of all available plugin names */
        std::vector<std::string> pluginList() const;

        /**
         * @brief Plugin metadata
         *
         * Returns pointer to plugin metadata or `nullptr`, if given plugin is
         * not found.
         */
        const PluginMetadata* metadata(const std::string& plugin) const;

        /**
         * @brief Load state of a plugin
         *
         * Returns @ref LoadState "LoadState::Loaded" if the plugin is loaded or
         * @ref LoadState "LoadState::NotLoaded" if not. For static plugins
         * returns always @ref LoadState "LoadState::Static". On failure returns
         * @ref LoadState "LoadState::NotFound" or @ref LoadState "LoadState::WrongMetadataFile".
         * @see load(), unload(), reload()
         */
        LoadState loadState(const std::string& plugin) const;

        /**
         * @brief Load a plugin
         *
         * Returns @ref LoadState "LoadState::Loaded" if the plugin is already
         * loaded or if loading succeeded. For static plugins returns always
         * @ref LoadState "LoadState::Static". On failure returns
         * @ref LoadState "LoadState::NotFound", @ref LoadState "LoadState::WrongPluginVersion",
         * @ref LoadState "LoadState::WrongInterfaceVersion", @ref LoadState "LoadState::UnresolvedDependency"
         * or @ref LoadState "LoadState::LoadFailed".
         *
         * If the plugin has any dependencies, they are recursively processed
         * before loading given plugin.
         *
         * @see unload(), reload(), loadState()
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
         */
        virtual LoadState load(const std::string& plugin);

        /**
         * @brief Unload a plugin
         *
         * Returns @ref LoadState "LoadState::NotLoaded" if the plugin is not
         * loaded or unloading succeeded. For static plugins always returns
         * @ref LoadState "LoadState::Static". On failure returns
         * @ref LoadState "LoadState::UnloadFailed", @ref LoadState "LoadState::Required"
         * or @ref LoadState "LoadState::Used".
         *
         * @see load(), reload(), loadState()
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_NACL_NEWLIB_ "NaCl newlib".
         */
        virtual LoadState unload(const std::string& plugin);

    protected:
        /**
         * @brief Destructor
         *
         * Destroys all plugin instances and unloads all plugins.
         */
        /* Nobody will need to have (and delete) AbstractManager*, thus this is
           faster than public virtual destructor */
        ~AbstractManager();

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    protected:
    #endif
        struct StaticPlugin {
            /* GCC 4.6 cannot create this via new and initializer list */
            #ifdef CORRADE_GCC46_COMPATIBILITY
            StaticPlugin(std::string plugin, std::string interface, Instancer instancer, void(*initializer)(), void(*finalizer)()): plugin(std::move(plugin)), interface(std::move(interface)), instancer(instancer), initializer(initializer), finalizer(finalizer) {}
            #endif

            std::string plugin;
            std::string interface;
            Instancer instancer;
            void(*initializer)();
            void(*finalizer)();
        };

        struct Plugin {
            #ifndef CORRADE_TARGET_NACL_NEWLIB
            LoadState loadState;
            #else
            const LoadState loadState;
            #endif
            const Utility::Configuration configuration;
            PluginMetadata metadata;

            /* If set to nullptr, the plugin has not any associated plugin
               manager and cannot be loaded. */
            AbstractManager* manager;

            Instancer instancer;

            #ifndef CORRADE_TARGET_NACL_NEWLIB
            union {
            #endif
                /* For static plugins */
                StaticPlugin* staticPlugin;

            #ifndef CORRADE_TARGET_NACL_NEWLIB
                /* For dynamic plugins */
                #ifndef _WIN32
                void* module;
                #else
                HMODULE module;
                #endif
            };
            #endif

            #ifndef CORRADE_TARGET_NACL_NEWLIB
            /* Constructor for dynamic plugins */
            explicit Plugin(const std::string& _metadata, AbstractManager* _manager);
            #endif

            /* Constructor for static plugins */
            explicit Plugin(std::istream& _metadata, StaticPlugin* staticPlugin);

            ~Plugin();
        };

        #ifndef CORRADE_TARGET_NACL_NEWLIB
        std::string _pluginDirectory;
        #endif

        /* Defined in PluginManager */
        virtual std::string pluginInterface() const = 0;

        /* Global storage of static, unloaded and loaded plugins. The map is
           accessible via function, not directly, because we need to fill it
           with data from staticPlugins() before first use. */
        static std::map<std::string, Plugin*>* plugins();

        /* Because the plugin manager must be noticed about adding the plugin to
           "used by" list, it must be done through this function. */
        virtual void addUsedBy(const std::string& plugin, std::string usedBy);

        /* Because the plugin manager must be noticed about removing the plugin
           from "used by" list, it must be done through this function. */
        virtual void removeUsedBy(const std::string& plugin, const std::string& usedBy);

        void* instanceInternal(const std::string& plugin);

    private:
        /* Temporary storage of all information needed to import static plugins.
           They are imported to plugins() map on first call to plugins(),
           because at that time it is safe to assume that all static resources
           (plugin configuration files) are already registered. After that, the
           storage is deleted and set to `nullptr` to indicate that static
           plugins have been already processed.

           The vector is accessible via function, not directly, because we don't
           know initialization order of static members and thus the vector could
           be uninitalized when accessed from CORRADE_PLUGIN_REGISTER(). */
        CORRADE_PLUGINMANAGER_LOCAL static std::vector<StaticPlugin*>*& staticPlugins();

        std::map<std::string, std::vector<AbstractPlugin*> > instances;

        CORRADE_PLUGINMANAGER_LOCAL void registerInstance(std::string plugin, AbstractPlugin* instance, const Utility::Configuration** configuration, const PluginMetadata** metadata);
        CORRADE_PLUGINMANAGER_LOCAL void unregisterInstance(const std::string& plugin, AbstractPlugin* instance);
};

/** @hideinitializer
@brief Import static plugin
@param name      Static plugin name (defined with CORRADE_PLUGIN_REGISTER())

If static plugins are compiled into dynamic library or directly into the
executable, they should be automatically loaded at startup thanks to
AUTOMATIC_INITALIZER() and CORRADE_AUTOMATIC_FINALIZER() macros.

If static plugins are compiled into static library, they are not automatically
loaded at startup, so you need to load them explicitly by calling CORRADE_PLUGIN_IMPORT()
at the beginning of `main()` function. You can also wrap these macro calls into
another function (which will then be compiled into dynamic library or main
executable) and use CORRADE_AUTOMATIC_INITIALIZER() macro for automatic call.
@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `pluginImporter_*`, this could be the
    problem. See CORRADE_RESOURCE_INITIALIZE() documentation for more
    information.
 */
#define CORRADE_PLUGIN_IMPORT(name)                                         \
    extern int pluginImporter_##name();                                     \
    pluginImporter_##name();                                                \
    CORRADE_RESOURCE_INITIALIZE(name)

} namespace Utility {

/** @debugoperator{Corrade::PluginManager::AbstractManager} */
Debug CORRADE_PLUGINMANAGER_EXPORT operator<<(Debug debug, PluginManager::LoadState value);

}}

#endif
