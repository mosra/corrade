/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include "AbstractPlugin.h"

#include "Corrade/PluginManager/AbstractManager.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/Utility/ConfigurationGroup.h"
#include "Corrade/Utility/Directory.h"

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#include "Corrade/PluginManager/configure.h"
#endif

namespace Corrade { namespace PluginManager {

struct AbstractPlugin::State {
    AbstractManager* manager{};
    std::string plugin;
    const PluginMetadata* metadata{};
    Utility::ConfigurationGroup configuration{};
};

std::string AbstractPlugin::pluginInterface() { return {}; }

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
std::vector<std::string> AbstractPlugin::pluginSearchPaths() { return {}; }

std::string AbstractPlugin::pluginSuffix() { return PLUGIN_FILENAME_SUFFIX; }
#endif

std::string AbstractPlugin::pluginMetadataSuffix() { return ".conf"; }

void AbstractPlugin::initialize() {}

void AbstractPlugin::finalize() {}

AbstractPlugin::AbstractPlugin(): _state{Containers::InPlaceInit} {}

AbstractPlugin::AbstractPlugin(AbstractManager& manager, const std::string& plugin): _state{Containers::InPlaceInit} {
    _state->manager = &manager;
    _state->plugin = plugin;
    manager.registerInstance(plugin, *this, _state->metadata);
    _state->configuration = _state->metadata->configuration();
}

AbstractPlugin::AbstractPlugin(AbstractManager& manager): _state{Containers::InPlaceInit} {
    _state->manager = &manager;
}

AbstractPlugin::AbstractPlugin(AbstractPlugin&& other) noexcept: _state{std::move(other._state)} {
    /* Reregister the instance if the plugin was instantiated through a plugin
       manager. Note that instantiating using
       AbstractManagingPlugin::AbstractManagingPlugin(AbstractManager&) is
       *not* instantiating through the manager, in that case the _metadata
       field would be nullptr. */
    if(_state && _state->manager && _state->metadata)
        _state->manager->reregisterInstance(_state->plugin, other, this);
}

AbstractPlugin::~AbstractPlugin() {
    /* Unregister the instance only if the plugin was instantiated through a
       plugin manager. Note that instantiating using
       AbstractManagingPlugin::AbstractManagingPlugin(AbstractManager&) is
       *not* instantiating through the manager, in that case the _metadata
       field would be nullptr. */
    if(_state && _state->manager && _state->metadata)
        _state->manager->reregisterInstance(_state->plugin, *this, nullptr);
}

bool AbstractPlugin::canBeDeleted() { return false; }

const std::string& AbstractPlugin::plugin() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::plugin(): can't be called on a moved-out plugin", _state->plugin);
    return _state->plugin;
}

const PluginMetadata* AbstractPlugin::metadata() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::metadata(): can't be called on a moved-out plugin", {});
    return _state->metadata;
}

Utility::ConfigurationGroup& AbstractPlugin::configuration() {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::configuration(): can't be called on a moved-out plugin", _state->configuration);
    return _state->configuration;
}
const Utility::ConfigurationGroup& AbstractPlugin::configuration() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractPlugin::configuration(): can't be called on a moved-out plugin", _state->configuration);
    return _state->configuration;
}

/* These asserts refer AbstractManagingPlugin because it's public only there */
AbstractManager* AbstractPlugin::manager() {
    CORRADE_ASSERT(_state, "PluginManager::AbstractManagingPlugin::manager(): can't be called on a moved-out plugin", {});
    return _state->manager;
}
const AbstractManager* AbstractPlugin::manager() const {
    CORRADE_ASSERT(_state, "PluginManager::AbstractManagingPlugin::manager(): can't be called on a moved-out plugin", {});
    return _state->manager;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
std::vector<std::string> implicitPluginSearchPaths(const std::string& libraryLocation, const std::string& hardcodedPath, const char* const relativePath) {
    std::vector<std::string> out;
    #ifdef CORRADE_TARGET_APPLE
    out.reserve(5);
    #elif !defined(CORRADE_TARGET_WINDOWS)
    out.reserve(4);
    #else
    out.reserve(3);
    #endif

    if(!hardcodedPath.empty()) out.push_back(hardcodedPath);
    #ifdef CORRADE_TARGET_APPLE
    out.push_back(Utility::Directory::join("../PlugIns", relativePath));
    #endif
    if(!libraryLocation.empty())
        out.push_back(Utility::Directory::join(Utility::Directory::path(libraryLocation), relativePath));
    #ifndef CORRADE_TARGET_WINDOWS
    out.push_back(Utility::Directory::join("../lib", relativePath));
    #endif
    out.push_back(relativePath);

    return out;
}
#endif

}}
