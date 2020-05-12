#ifndef Corrade_PluginManager_visibility_h
#define Corrade_PluginManager_visibility_h
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

#include "Corrade/configure.h"
#include "Corrade/Utility/VisibilityMacros.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef CORRADE_BUILD_STATIC
    #if defined(CorradePluginManager_EXPORTS) || defined(CorradePluginManagerObjects_EXPORTS) || defined(CorradePluginManagerTestLib_EXPORTS)
        #define CORRADE_PLUGINMANAGER_EXPORT CORRADE_VISIBILITY_EXPORT
    #else
        #define CORRADE_PLUGINMANAGER_EXPORT CORRADE_VISIBILITY_IMPORT
    #endif
#else
    #define CORRADE_PLUGINMANAGER_EXPORT CORRADE_VISIBILITY_STATIC
#endif
#define CORRADE_PLUGINMANAGER_LOCAL CORRADE_VISIBILITY_LOCAL
#define CORRADE_PLUGIN_EXPORT CORRADE_VISIBILITY_EXPORT
#else
#define CORRADE_PLUGINMANAGER_EXPORT
#define CORRADE_PLUGINMANAGER_LOCAL
#define CORRADE_PLUGIN_EXPORT
#endif

#endif
