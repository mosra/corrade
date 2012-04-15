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

#include "NativePluginAccessor.h"

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#undef interface
#define dlsym GetProcAddress
#define dlerror GetLastError
#define dlclose FreeLibrary
#endif

#include "Utility/Directory.h"
#include "NativePluginAccessorConfigure.h"

using namespace std;
using namespace Corrade::Utility;

namespace Corrade { namespace PluginManager { namespace Plugins {

AbstractPluginManager::LoadState NativePluginAccessor::load() {
    string filename = Directory::join(pluginManager->pluginDirectory(), pluginName + PLUGIN_FILENAME_SUFFIX);

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    #ifndef _WIN32
    void* module = dlopen(filename.c_str(), RTLD_NOW|RTLD_GLOBAL);
    #else
    HMODULE module = LoadLibraryA(filename.c_str());
    #endif
    if(!module) {
        Error() << "PluginManager: cannot open plugin file"
                << '"' + pluginManager->pluginDirectory() + pluginName + PLUGIN_FILENAME_SUFFIX + "\":"
                << dlerror();
        loadState = AbstractPluginManager::LoadFailed;
        return loadState;
    }

    /* Check plugin version */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    int (*_version)(void) = reinterpret_cast<int(*)()>(dlsym(module, "pluginVersion"));
    if(_version == 0) {
        Error() << "PluginManager: cannot get version of plugin" << '\'' + pluginName + "':" << dlerror();
        dlclose(module);
        loadState = AbstractPluginManager::LoadFailed;
        return loadState;
    }
    if(_version() != AbstractPluginManager::version) {
        Error() << "PluginManager: wrong plugin version, expected" << _version() << "got" << version;
        dlclose(module);
        loadState = AbstractPluginManager::WrongPluginVersion;
        return loadState;
    }

    /* Check interface string */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    string (*interface)() = reinterpret_cast<string (*)()>(dlsym(module, "pluginInterface"));
    if(interface == 0) {
        Error() << "PluginManager: cannot get interface string of plugin" << '\'' + pluginName + "':" << dlerror();
        dlclose(module);
        loadState = AbstractPluginManager::LoadFailed;
        return loadState;
    }
    if(interface() != pluginManager->pluginInterface()) {
        Error() << "PluginManager: wrong plugin interface, expected" << '\'' + pluginManager->pluginInterface() + ", got '" + interface() + "'";
        dlclose(module);
        loadState = AbstractPluginManager::WrongInterfaceVersion;
        return loadState;
    }

    /* Load plugin instancer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    instancer = reinterpret_cast<void* (*)(AbstractPluginManager*, const std::string&)>(dlsym(module, "pluginInstancer"));
    if(instancer == 0) {
        Error() << "PluginManager: cannot get instancer of plugin" << '\'' + pluginName + "':" << dlerror();
        dlclose(module);
        loadState = AbstractPluginManager::LoadFailed;
        return loadState;
    }

    loadState = AbstractPluginManager::LoadOk;
    return loadState;
}

AbstractPluginManager::LoadState NativePluginAccessor::unload() {
    #ifndef _WIN32
    if(dlclose(module) != 0) {
    #else
    if(!FreeLibrary(module)) {
    #endif
        Error() << "PluginManager: cannot unload plugin" << '\'' + pluginName + "':" << dlerror();
        loadState = AbstractPluginManager::UnloadFailed;
        return loadState;
    }

    loadState = AbstractPluginManager::NotLoaded;
    return loadState;
}

void* NativePluginAccessor::instance() {
    instancer(pluginManager, pluginName);
}

}
