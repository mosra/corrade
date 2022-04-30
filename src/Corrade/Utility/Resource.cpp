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
       static variable from there. The keys point to names of existing groups,
       thus don't need to be allocated. */
    std::map<Containers::StringView, Containers::String>* overrideGroups;
};

/* What the hell is going on here with the #ifdefs?! */
#if !defined(CORRADE_BUILD_STATIC) || !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || (defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS)) || defined(CORRADE_TARGET_WINDOWS_RT)
#ifdef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
/* On static builds that get linked to multiple shared libraries and then used
   in a single app we want to ensure there's just one global symbol. On Linux
   it's apparently enough to just export, macOS needs the weak attribute. */
CORRADE_VISIBILITY_EXPORT
    #ifdef CORRADE_TARGET_GCC
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
    /* Here the key is again pointing to names of existing files, thus no need
       to be allocated */
    std::map<Containers::StringView, Containers::Array<char>> data;

    explicit OverrideData(const Containers::StringView filename): conf(filename) {}
};

void Resource::registerData(Implementation::ResourceGroup& resource) {
    Containers::Implementation::forwardListInsert(resourceGlobals.groups, resource);
}

void Resource::unregisterData(Implementation::ResourceGroup& resource) {
    Containers::Implementation::forwardListRemove(resourceGlobals.groups, resource);
}

namespace {
    Implementation::ResourceGroup* findGroup(const Containers::StringView name) {
        for(Implementation::ResourceGroup* group = resourceGlobals.groups; group; group = Containers::Implementation::forwardListNext(*group)) {
            if(group->name == name) return group;
        }

        return nullptr;
    }
}

void Resource::overrideGroup(const Containers::StringView group, const Containers::StringView configurationFile) {
    if(!resourceGlobals.overrideGroups) {
        static std::map<Containers::StringView, Containers::String> overrideGroups;
        resourceGlobals.overrideGroups = &overrideGroups;
    }

    CORRADE_ASSERT(findGroup(group),
        "Utility::Resource::overrideGroup(): group '" << Debug::nospace << group << Debug::nospace << "' was not found", );
    /* This group can be already overridden from before, so insert if not there
       yet and then update the filename */
    resourceGlobals.overrideGroups->emplace(group, Containers::String{}).first->second = Containers::String::nullTerminatedGlobalView(configurationFile);
}

bool Resource::hasGroup(const Containers::StringView group) {
    return findGroup(group);
}

Resource::Resource(const Containers::StringView group): _group{findGroup(group)}, _overrideGroup(nullptr) {
    CORRADE_ASSERT(_group, "Utility::Resource: group '" << Debug::nospace << group << Debug::nospace << "' was not found", );

    if(resourceGlobals.overrideGroups) {
        auto overridden = resourceGlobals.overrideGroups->find(group);
        if(overridden != resourceGlobals.overrideGroups->end()) {
            Debug{}
                << "Utility::Resource: group '" << Debug::nospace << group << Debug::nospace << "' overridden with '" << Debug::nospace << overridden->second << Debug::nospace << "\'";
            _overrideGroup = new OverrideData(overridden->second);

            if(_overrideGroup->conf.value<Containers::StringView>("group") != group) Warning{}
                << "Utility::Resource: overridden with different group, found '"
                << Debug::nospace << _overrideGroup->conf.value<Containers::StringView>("group")
                << Debug::nospace << "' but expected '" << Debug::nospace
                << group << Debug::nospace << "'";
        }
    }
}

Resource::~Resource() {
    delete _overrideGroup;
}

Containers::Array<Containers::StringView> Resource::list() const {
    CORRADE_INTERNAL_ASSERT(_group);

    Containers::Array<Containers::StringView> out{NoInit, _group->count};
    for(std::size_t i = 0; i != _group->count; ++i)
        new(&out[i]) Containers::StringView{Implementation::resourceFilenameAt(_group->positions, _group->filenames, i)};
    return out;
}

Containers::ArrayView<const char> Resource::getRaw(const Containers::StringView filename) const {
    /* Going this way instead of the other way around because the StringView
       can hold information about null termination or global lifetime, which
       would be painful to query in a subsequent step */
    return getString(filename);
}

Containers::StringView Resource::getString(const Containers::StringView filename) const {
    CORRADE_INTERNAL_ASSERT(_group);

    /* Look for the file in compiled-in resources. This is done before looking
       into an overriden group configuration file to prevent retrieving files
       that aren't compiled in. */
    const unsigned int i = Implementation::resourceLookup(_group->count, _group->positions, _group->filenames, filename);
    CORRADE_ASSERT(i != _group->count,
        "Utility::Resource::get(): file '" << Debug::nospace << filename << Debug::nospace << "' was not found in group '" << Debug::nospace << _group->name << Debug::nospace << "'", {});

    /* The group is overridden with live data */
    if(_overrideGroup) {
        /* The file is already loaded */
        auto it = _overrideGroup->data.find(filename);
        if(it != _overrideGroup->data.end())
            return Containers::ArrayView<const char>{it->second};

        /* Load the file and save it for later use. Linear search is not an
           issue, as this shouldn't be used in production code anyway. */
        std::vector<const ConfigurationGroup*> files = _overrideGroup->conf.groups("file");
        for(auto file: files) {
            const Containers::StringView name = file->hasValue("alias") ? file->value<Containers::StringView>("alias") : file->value<Containers::StringView>("filename");
            if(name != filename) continue;

            /* Load the file */
            Containers::Optional<Containers::Array<char>> data = Path::read(Path::join(Path::split(_overrideGroup->conf.filename()).first(), file->value("filename")));
            if(!data) {
                Error{} << "Utility::Resource::get(): cannot open file" << file->value<Containers::StringView>("filename") << "from overridden group";
                break;
            }

            /* Save the file for later use and return. Use a filename from the
               compiled-in resources which is guaranteed to be global to avoid
               allocating a new string */
            it = _overrideGroup->data.emplace(Implementation::resourceFilenameAt(_group->positions, _group->filenames, i), *std::move(data)).first;
            return Containers::ArrayView<const char>{it->second};
        }

        /* The file was not found, fallback to compiled-in ones */
        Warning{} << "Utility::Resource::get(): file '" << Debug::nospace
            << filename << Debug::nospace << "' was not found in overridden group, fallback to compiled-in resources";
    }

    return Implementation::resourceDataAt(_group->positions, _group->data, i);
}

#ifdef CORRADE_BUILD_DEPRECATED
std::string Resource::get(const std::string& filename) const {
    return getString(filename);
}
#endif

}}
