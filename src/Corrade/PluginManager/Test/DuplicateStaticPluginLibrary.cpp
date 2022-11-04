/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include "Corrade/PluginManager/AbstractManager.h"

#ifdef BUILDING_LIBRARY1
static void importPlugin1() {
    CORRADE_PLUGIN_IMPORT(Canary)
}
#elif defined(BUILDING_LIBRARY2)
static void importPlugin2() {
    CORRADE_PLUGIN_IMPORT(Dird)
    CORRADE_PLUGIN_IMPORT(Canary)
}
#else
#error
#endif

namespace Corrade { namespace PluginManager { namespace Test {

#ifdef BUILDING_LIBRARY1
CORRADE_VISIBILITY_EXPORT int initialize1();
int initialize1() {
    importPlugin1();
    return 42;
}
#elif defined(BUILDING_LIBRARY2)
CORRADE_VISIBILITY_EXPORT int initialize2();
int initialize2() {
    importPlugin2();
    return 1337;
}
#endif

}}}
