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

#include "Resource.h"

#include <map> /* overrideGroups :( */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/Implementation/RawForwardList.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/ConfigurationGroup.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Implementation/Resource.h"

#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Implementation/WindowsWeakSymbol.h"
#endif

namespace Corrade { namespace Utility {

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
/* (Of course) can't be in an unnamed namespace in order to export it below
   (except for Windows, where we do extern "C" so this doesn't matter, but we
   don't want to expose the ResourceGlobals symbols if not needed) */
namespace {
#endif

struct ResourceGlobals {
    /* A linked list of resources. Managed using utilities from
       Containers/Implementation/RawForwardList.h, look there for more info. */
    Implementation::ResourceGroup* groups;

    /* Overridden groups. This is only allocated if the user calls
       Resource::overrideGroup() and stores a pointer to a function-local
       static variable from there. */
    std::map<std::string, std::string>* overrideGroups;
};

/* What the hell is going on here with the #ifdefs?! */
#if !defined(CORRADE_BUILD_STATIC) || !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || (defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS)) || defined(CORRADE_TARGET_WINDOWS_RT)
#ifdef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
/* On static builds that get linked to multiple shared libraries and then used
   in a single app we want to ensure there's just one global symbol. On Linux
   it's apparently enough to just export, macOS needs the weak attribute. */
CORRADE_VISIBILITY_EXPORT
    #ifdef __GNUC__
    __attribute__((weak))
    #else
    /* uh oh? the test will fail, probably */
    #endif
#endif
/* The value of this variable is guaranteed to be zero-filled even before any
   resource initializers are executed, which means we don't hit any static
   initialization order fiasco. */
ResourceGlobals resourceGlobals{nullptr, nullptr};
#else
/* On Windows the symbol is exported unmangled and then fetched via
   GetProcAddress() to emulate weak linking. Using an extern "C" block instead
   of just a function annotation because otherwise MinGW prints a warning:
   '...' initialized and declared 'extern' (uh?) */
extern "C" {
    CORRADE_VISIBILITY_EXPORT ResourceGlobals corradeUtilityUniqueWindowsResourceGlobals{nullptr, nullptr};
}
#endif

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
}
#endif

/* Windows don't have any concept of weak symbols, instead GetProcAddress() on
   GetModuleHandle(nullptr) "emulates" the weak linking as it's guaranteed to
   pick up the same symbol of the final exe independently of the DLL it was
   called from. To avoid #ifdef hell in code below, the resourceGlobals are
   redefined to return a value from this uniqueness-ensuring function. */
#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
namespace {

ResourceGlobals& windowsResourceGlobals() {
    /* A function-local static to ensure it's only initialized once without any
       race conditions among threads */
    static ResourceGlobals* uniqueGlobals = reinterpret_cast<ResourceGlobals*>(Implementation::windowsWeakSymbol("corradeUtilityUniqueWindowsResourceGlobals", &corradeUtilityUniqueWindowsResourceGlobals));
    return *uniqueGlobals;
}

}

#define resourceGlobals windowsResourceGlobals()
#endif

struct Resource::OverrideData {
    const Configuration conf;
    std::map<std::string, Containers::Array<char>> data;

    explicit OverrideData(const std::string& filename): conf(filename) {}
};

void Resource::registerData(Implementation::ResourceGroup& resource) {
    Containers::Implementation::forwardListInsert(resourceGlobals.groups, resource);
}

void Resource::unregisterData(Implementation::ResourceGroup& resource) {
    Containers::Implementation::forwardListRemove(resourceGlobals.groups, resource);
}

namespace {
    Implementation::ResourceGroup* findGroup(const Containers::ArrayView<const char> name) {
        for(Implementation::ResourceGroup* group = resourceGlobals.groups; group; group = Containers::Implementation::forwardListNext(*group)) {
            /* std::strncmp() would return equality also if name was just a
               prefix of group->name, so test that it ends with a null
               terminator */
            if(std::strncmp(group->name, name, name.size()) == 0 && group->name[name.size()] == '\0') return group;
        }

        return nullptr;
    }
}

void Resource::overrideGroup(const std::string& group, const std::string& configurationFile) {
    if(!resourceGlobals.overrideGroups) {
        static std::map<std::string, std::string> overrideGroups;
        resourceGlobals.overrideGroups = &overrideGroups;
    }

    CORRADE_ASSERT(findGroup({group.data(), group.size()}),
        "Utility::Resource::overrideGroup(): group" << '\'' + group + '\'' << "was not found", );
    /* This group can be already overridden from before, so insert if not there
       yet and then update the filename */
    resourceGlobals.overrideGroups->emplace(group, std::string{}).first->second = configurationFile;
}

bool Resource::hasGroup(const std::string& group) {
    return hasGroupInternal({group.data(), group.size()});
}

bool Resource::hasGroupInternal(const Containers::ArrayView<const char> group) {
    return findGroup(group);
}

Resource::Resource(const std::string& group): Resource{{group.data(), group.size()}, nullptr} {}

Resource::Resource(const Containers::ArrayView<const char> group, void*): _group{findGroup(group)}, _overrideGroup(nullptr) {
    CORRADE_ASSERT(_group, "Utility::Resource: group '" << Debug::nospace << (std::string{group, group.size()}) << Debug::nospace << "' was not found", );

    if(resourceGlobals.overrideGroups) {
        const std::string groupString{group.data(), group.size()};
        auto overridden = resourceGlobals.overrideGroups->find(groupString);
        if(overridden != resourceGlobals.overrideGroups->end()) {
            Debug{}
                << "Utility::Resource: group '" << Debug::nospace << groupString << Debug::nospace << "' overridden with '" << Debug::nospace << overridden->second << Debug::nospace << "\'";
            _overrideGroup = new OverrideData(overridden->second);

            if(_overrideGroup->conf.value("group") != groupString) Warning{}
                << "Utility::Resource: overridden with different group, found '"
                << Debug::nospace << _overrideGroup->conf.value("group")
                << Debug::nospace << "' but expected '" << Debug::nospace
                << groupString << Debug::nospace << "'";
        }
    }
}

Resource::~Resource() {
    delete _overrideGroup;
}

std::vector<std::string> Resource::list() const {
    CORRADE_INTERNAL_ASSERT(_group);

    std::vector<std::string> result;
    result.reserve(_group->count);
    for(std::size_t i = 0; i != _group->count; ++i) {
        Containers::ArrayView<const char> filename = Implementation::resourceFilenameAt(_group->positions, _group->filenames, i);
        result.push_back({filename.data(), filename.size()});
    }

    return result;
}

Containers::ArrayView<const char> Resource::getRaw(const std::string& filename) const {
    return getInternal({filename.data(), filename.size()});
}

Containers::ArrayView<const char> Resource::getInternal(const Containers::ArrayView<const char> filename) const {
    CORRADE_INTERNAL_ASSERT(_group);

    /* The group is overridden with live data */
    if(_overrideGroup) {
        const std::string filenameString{filename.data(), filename.size()};

        /* The file is already loaded */
        auto it = _overrideGroup->data.find(filenameString);
        if(it != _overrideGroup->data.end())
            return it->second;

        /* Load the file and save it for later use. Linear search is not an
           issue, as this shouldn't be used in production code anyway. */
        std::vector<const ConfigurationGroup*> files = _overrideGroup->conf.groups("file");
        for(auto file: files) {
            const std::string name = file->hasValue("alias") ? file->value("alias") : file->value("filename");
            if(name != filenameString) continue;

            /* Load the file */
            Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(Path::split(_overrideGroup->conf.filename()).first(), file->value("filename")));
            if(!data) {
                Error() << "Utility::Resource::get(): cannot open file" << file->value("filename") << "from overridden group";
                break;
            }

            /* Save the file for later use and return */
            it = _overrideGroup->data.emplace(filenameString, *std::move(data)).first;
            return it->second;
        }

        /* The file was not found, fallback to compiled-in ones */
        Warning() << "Utility::Resource::get(): file '" << Debug::nospace
            << filenameString << Debug::nospace << "' was not found in overridden group, fallback to compiled-in resources";
    }

    const unsigned int i = Implementation::resourceLookup(_group->count, _group->positions, _group->filenames, filename);
    CORRADE_ASSERT(i != _group->count,
        "Utility::Resource::get(): file '" << Debug::nospace << (std::string{filename, filename.size()}) << Debug::nospace << "' was not found in group '" << Debug::nospace << _group->name << Debug::nospace << "\'", nullptr);

    return Implementation::resourceDataAt(_group->positions, _group->data, i);
}

std::string Resource::get(const std::string& filename) const {
    Containers::ArrayView<const char> data = getRaw(filename);
    return data ? std::string{data, data.size()} : std::string{};
}

}}
