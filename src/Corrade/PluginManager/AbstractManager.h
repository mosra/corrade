#ifndef Corrade_PluginManager_AbstractManager_h
#define Corrade_PluginManager_AbstractManager_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::PluginManager::AbstractManager
 */

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/PluginManager/PluginManager.h"
#include "Corrade/PluginManager/visibility.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Resource.h"

#ifdef CORRADE_TARGET_WINDOWS
/* I didn't find a better way to circumvent the need for including windows.h */
struct HINSTANCE__;
typedef struct HINSTANCE__* HMODULE;

/* combaseapi.h in Windows does this insane thing. Can be worked around by
   defining WIN32_LEAN_AND_MEAN but not everybody does that. */
#ifdef interface
#undef interface
#endif
#endif

namespace Corrade { namespace PluginManager {

/**
@brief Plugin load state

@see @ref LoadStates, @ref AbstractManager::loadState(),
    @ref AbstractManager::load(), @ref AbstractManager::unload()
*/
enum class LoadState: unsigned short {
    /**
     * The plugin cannot be found. Returned by @ref AbstractManager::loadState()
     * and @ref AbstractManager::load().
     */
    NotFound = 1 << 0,

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    /**
     * The plugin is build with different version of plugin manager and cannot
     * be loaded. Returned by @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    WrongPluginVersion = 1 << 1,

    /**
     * The plugin uses different interface than the interface used by plugin
     * manager and cannot be loaded. Returned by @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    WrongInterfaceVersion = 1 << 2,

    /**
     * The plugin doesn't have any metadata file or the metadata file contains
     * errors. Returned by @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    WrongMetadataFile = 1 << 3,

    /**
     * The plugin depends on another plugin, which cannot be loaded (e.g. not
     * found or wrong version). Returned by @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    UnresolvedDependency = 1 << 4,

    /**
     * The plugin failed to load for other reason (e.g. linking failure).
     * Returned by @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    LoadFailed = 1 << 5,
    #endif

    /**
     * The plugin is static. Returned by @ref AbstractManager::loadState() and
     * @ref AbstractManager::load().
     */
    Static = 1 << 6,

    /**
     * The plugin is successfully loaded. Returned by @ref AbstractManager::loadState()
     * and @ref AbstractManager::load(). The value includes value of @ref LoadState::Static,
     * see @ref LoadStates for more information. In @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     * @ref CORRADE_TARGET_WINDOWS_RT "Windows RT", @ref CORRADE_TARGET_IOS "iOS"
     * and @ref CORRADE_TARGET_ANDROID the value is equivalent to
     * @ref LoadState::Static.
     */
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    Loaded = (1 << 7) | LoadState::Static,
    #else
    Loaded = LoadState::Static,
    #endif

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    /**
     * The plugin is not loaded. Plugin can be unloaded only if is dynamic and
     * is not required by any other plugin. Returned by
     * @ref AbstractManager::loadState() and @ref AbstractManager::load().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    NotLoaded = 1 << 8,

    /**
     * The plugin failed to unload. Returned by @ref AbstractManager::unload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    UnloadFailed = 1 << 9,

    /**
     * The plugin cannot be unloaded because another plugin is depending on it.
     * Unload that plugin first and try again. Returned by @ref AbstractManager::unload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    Required = 1 << 10,
    #endif

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    /**
     * The plugin has active instance and cannot be unloaded. Destroy all
     * instances and try again. Returned by @ref AbstractManager::unload().
     * @partialsupport Only static plugins are supported in
     *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
     *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
     *      @ref CORRADE_TARGET_IOS "iOS" and
     *      @ref CORRADE_TARGET_ANDROID "Android.
     */
    Used = 1 << 11
    #endif
};

/** @debugoperatorenum{Corrade::PluginManager::LoadState} */
CORRADE_PLUGINMANAGER_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, PluginManager::LoadState value);

/**
@brief Plugin load states

Useful when checking whether @ref LoadState in in given set of values, for
example:
@code
if(loadState & (LoadState::WrongPluginVersion|LoadState::WrongInterfaceVersion)) {
    // ...
}
@endcode

Note that @ref LoadState::Loaded includes value of @ref LoadState::Static, so
you can use `loadState & LoadState::Loaded` instead of much more verbose
`state & (LoadState::Loaded|LoadState::Static)`.
@see @ref AbstractManager::loadState(), @ref AbstractManager::load(),
    @ref AbstractManager::unload()
*/
typedef Containers::EnumSet<LoadState> LoadStates;

CORRADE_ENUMSET_OPERATORS(LoadStates)

/**
@brief Non-templated base class of @ref Manager

See also @ref plugin-management.
 */
class CORRADE_PLUGINMANAGER_EXPORT AbstractManager {
    friend AbstractPlugin;

    public:
        /** @brief Plugin version */
        static const int Version;

        #ifndef DOXYGEN_GENERATING_OUTPUT
        typedef void* (*Instancer)(AbstractManager&, const std::string&);
        static void importStaticPlugin(std::string plugin, int _version, std::string interface, Instancer instancer, void(*initializer)(), void(*finalizer)());
        #endif

        /** @brief Copying is not allowed */
        AbstractManager(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager(AbstractManager&&) = delete;

        /** @brief Copying is not allowed */
        AbstractManager& operator=(const AbstractManager&) = delete;

        /** @brief Moving is not allowed */
        AbstractManager& operator=(AbstractManager&&) = delete;

        /**
         * @brief Plugin interface
         *
         * Only plugins with the same plugin interface string can be used
         * in this plugin manager.
         */
        std::string pluginInterface() const;

        #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
        /**
         * @brief Plugin directory
         *
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android.
         */
        std::string pluginDirectory() const;

        /**
         * @brief Set another plugin directory
         *
         * Keeps loaded plugins untouched, removes unloaded plugins which are
         * not existing anymore and adds newly found plugins. The directory is
         * expected to be in UTF-8.
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android.
         */
        void setPluginDirectory(std::string directory);

        /**
         * @brief Reload plugin directory
         *
         * Convenience equivalent to `setPluginDirectory(pluginDirectory())`.
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android.
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
         * @see @ref AbstractPlugin::metadata()
         */
        const PluginMetadata* metadata(const std::string& plugin) const;

        /**
         * @brief Load state of a plugin
         *
         * Returns @ref LoadState::Loaded if the plugin is loaded or
         * @ref LoadState::NotLoaded if not. For static plugins returns always
         * @ref LoadState::Static. On failure returns @ref LoadState::NotFound
         * or @ref LoadState::WrongMetadataFile.
         * @see @ref load(), @ref unload()
         */
        LoadState loadState(const std::string& plugin) const;

        /**
         * @brief Load a plugin
         *
         * Returns @ref LoadState::Loaded if the plugin is already loaded or if
         * loading succeeded. For static plugins returns always
         * @ref LoadState::Static. On failure returns @ref LoadState::NotFound,
         * @ref LoadState::WrongPluginVersion,
         * @ref LoadState::WrongInterfaceVersion,
         * @ref LoadState::UnresolvedDependency or @ref LoadState::LoadFailed.
         *
         * If the plugin has any dependencies, they are recursively processed
         * before loading given plugin.
         *
         * @see @ref unload(), @ref loadState(), @ref Manager::instance(),
         *      @ref Manager::loadAndInstantiate()
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android.
         */
        LoadState load(const std::string& plugin);

        /**
         * @brief Unload a plugin
         *
         * Returns @ref LoadState::NotLoaded if the plugin is not loaded or
         * unloading succeeded. For static plugins always returns
         * @ref LoadState::Static. On failure returns
         * @ref LoadState::UnloadFailed, @ref LoadState::Required or
         * @ref LoadState::Used.
         *
         * @see @ref load(), @ref loadState()
         * @partialsupport Only static plugins are supported in
         *      @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten",
         *      @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
         *      @ref CORRADE_TARGET_IOS "iOS" and
         *      @ref CORRADE_TARGET_ANDROID "Android.
         */
        LoadState unload(const std::string& plugin);

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
            std::string plugin;
            std::string interface;
            Instancer instancer;
            void(*initializer)();
            void(*finalizer)();
        };

        struct Plugin {
            #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
            LoadState loadState;
            #else
            const LoadState loadState; /* Always LoadState::Static */
            #endif
            Utility::Configuration configuration;
            PluginMetadata metadata;

            /* If set to nullptr, the plugin has not any associated plugin
               manager and cannot be loaded. */
            AbstractManager* manager;

            Instancer instancer;

            #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
            union {
                /* For static plugins */
                StaticPlugin* staticPlugin;

                /* For dynamic plugins */
                #ifndef CORRADE_TARGET_WINDOWS
                void* module;
                #else
                HMODULE module;
                #endif
            };
            #else
            StaticPlugin* staticPlugin;
            #endif

            #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
            /* Constructor for dynamic plugins */
            explicit Plugin(std::string name, const std::string& metadata, AbstractManager* manager);
            #endif

            /* Constructor for static plugins */
            explicit Plugin(std::string name, std::istream& metadata, StaticPlugin* staticPlugin);

            /* Ensure that we don't delete staticPlugin twice */
            Plugin(const Plugin&) = delete;
            Plugin(Plugin&&) = delete;
            Plugin& operator=(const Plugin&) = delete;
            Plugin& operator=(Plugin&&) = delete;

            ~Plugin();
        };

        struct GlobalPluginStorage {
            std::map<std::string, Plugin*> plugins;
            std::map<std::string, Plugin&> aliases;
        };

        explicit AbstractManager(std::string pluginInterface, std::string pluginDirectory);

        #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
        std::string _pluginDirectory;
        #endif

        /* Initialize global plugin map. On first run it creates the instance
           and fills it with entries from staticPlugins(). The reference is
           then in constructor stored in _plugins variable to avoid at least
           some issues with duplicated static variables on static builds. */
        static GlobalPluginStorage& initializeGlobalPluginStorage();

        GlobalPluginStorage& _plugins;

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

        CORRADE_PLUGINMANAGER_LOCAL void registerInstance(const std::string& plugin, AbstractPlugin& instance, const PluginMetadata*& metadata);
        CORRADE_PLUGINMANAGER_LOCAL void unregisterInstance(const std::string& plugin, AbstractPlugin& instance);

        CORRADE_PLUGINMANAGER_LOCAL Plugin* findWithAlias(const std::string& plugin);
        CORRADE_PLUGINMANAGER_LOCAL const Plugin* findWithAlias(const std::string& plugin) const;

        #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
        CORRADE_PLUGINMANAGER_LOCAL LoadState loadInternal(Plugin& plugin);
        CORRADE_PLUGINMANAGER_LOCAL LoadState unloadInternal(Plugin& plugin);
        CORRADE_PLUGINMANAGER_LOCAL LoadState unloadRecursive(const std::string& plugin);
        CORRADE_PLUGINMANAGER_LOCAL LoadState unloadRecursiveInternal(Plugin& plugin);
        #endif

        std::string _pluginInterface;
        std::map<std::string, std::vector<AbstractPlugin*>> _instances;
};

/** @hideinitializer
@brief Import static plugin
@param name      Static plugin name (the same as defined with
    @ref CORRADE_PLUGIN_REGISTER())

If static plugins are compiled into dynamic library or directly into the
executable, they should be automatically loaded at startup thanks to
@ref CORRADE_AUTOMATIC_INITIALIZER() and @ref CORRADE_AUTOMATIC_FINALIZER()
macros.

If static plugins are compiled into static library, they are not automatically
loaded at startup, so you need to load them explicitly by calling
@ref CORRADE_PLUGIN_IMPORT() at the beginning of `main()` function. You can
also wrap these macro calls into another function (which will then be compiled
into dynamic library or main executable) and use @ref CORRADE_AUTOMATIC_INITIALIZER()
macro for automatic call.
@attention This macro should be called outside of any namespace. If you are
    running into linker errors with `pluginImporter_*`, this could be the
    problem. See @ref CORRADE_RESOURCE_INITIALIZE() documentation for more
    information.
 */
#define CORRADE_PLUGIN_IMPORT(name)                                         \
    extern int pluginImporter_##name();                                     \
    pluginImporter_##name();                                                \
    CORRADE_RESOURCE_INITIALIZE(name)

}}

#endif
