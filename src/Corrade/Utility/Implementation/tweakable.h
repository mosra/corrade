#ifndef Corrade_Utility_Implementation_tweakable_h
#define Corrade_Utility_Implementation_tweakable_h
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

#include <string>
#include <set>
#include <tuple>
#include <vector>

#include "Corrade/Utility/FileWatcher.h"
#include "Corrade/Utility/Tweakable.h"

namespace Corrade { namespace Utility { namespace Implementation {

/* Needs to be exposed like this so we can test it */

struct TweakableVariable {
    /* Align so we can safely save 64bit types without worrying about unaligned
       access. */
    CORRADE_ALIGNAS(8) char storage[TweakableStorageSize]{};
    int line{};
    TweakableState(*parser)(Containers::StringView, Containers::StaticArrayView<TweakableStorageSize, char>);
    void(*scopeLambda)(void(*)(), void*){};
    void(*scopeUserCall)(){};
    void* scopeUserData{};
};

CORRADE_UTILITY_EXPORT std::string findTweakableAlias(const std::string& file);
CORRADE_UTILITY_EXPORT TweakableState parseTweakables(const std::string& name, const std::string& filename, const std::string& data, std::vector<TweakableVariable>& variables, std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>>& scopes);

}}}

#endif
