#ifndef Corrade_PluginManager_PluginManagerVisibility_h
#define Corrade_PluginManager_PluginManagerVisibility_h
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

#ifndef DOXYGEN_GENERATING_OUTPUT

#ifdef _WIN32
    #ifdef CorradePluginManager_EXPORTS
        #define PLUGINMANAGER_EXPORT __declspec(dllexport)
    #else
        #define PLUGINMANAGER_EXPORT __declspec(dllimport)
    #endif
    #define PLUGINMANAGER_LOCAL
    #define PLUGIN_EXPORT
#else
    #define PLUGINMANAGER_EXPORT __attribute__ ((visibility ("default")))
    #define PLUGINMANAGER_LOCAL __attribute__ ((visibility ("hidden")))
    #define PLUGIN_EXPORT __attribute__ ((visibility ("default")))
#endif

#endif

#endif
