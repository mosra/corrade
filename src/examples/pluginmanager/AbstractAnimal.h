#ifndef Corrade_Examples_AbstractAnimal_h
#define Corrade_Examples_AbstractAnimal_h
/*
    This file is part of Corrade.

    Original authors — credit is appreciated but not required:

        2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018
            — Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <string>
#include <vector>
#include <Corrade/PluginManager/AbstractPlugin.h>

namespace Corrade { namespace Examples {

class AbstractAnimal: public PluginManager::AbstractPlugin {
    public:
        static std::string pluginInterface() {
            return "cz.mosra.corrade.Examples.AbstractAnimal/1.0";
        }

        static std::vector<std::string> pluginSearchPaths() {
            return {""};
        }

        explicit AbstractAnimal(PluginManager::AbstractManager& manager, const std::string& plugin):
            AbstractPlugin{manager, plugin} {}

        virtual std::string name() const = 0;
        virtual int legCount() const = 0;
        virtual bool hasTail() const = 0;
};

}}

#endif
