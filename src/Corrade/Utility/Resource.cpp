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

#include "Resource.h"

#ifdef _MSC_VER
#include <algorithm> /* std::max() */
#endif
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Implementation/RawForwardList.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/ConfigurationGroup.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"
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

    /* Overriden groups. This is only allocated if the user calls
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

std::pair<bool, Containers::Array<char>> fileContents(const std::string& filename) {
    if(!Directory::exists(filename)) return {false, nullptr};
    return {true, Directory::read(filename)};
}

std::string comment(const std::string& comment) {
    return "\n    /* " + comment + " */";
}

std::string hexcode(const std::string& data) {
    std::ostringstream out;
    out << std::hex;

    /* Each row is indented by four spaces and has newline at the end */
    for(std::size_t row = 0; row < data.size(); row += 15) {
        out << "\n    ";

        /* Convert all characters on a row to hex "0xab,0x01,..." */
        for(std::size_t end = std::min(row + 15, data.size()), i = row; i != end; ++i) {
            out << "0x" << std::setw(2) << std::setfill('0')
                << static_cast<unsigned int>(static_cast<unsigned char>(data[i]))
                << ",";
        }
    }

    return out.str();
}

inline bool lessFilename(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
    return a.first < b.first;
}


}

std::string Resource::compileFrom(const std::string& name, const std::string& configurationFile) {
    /* Resource file existence */
    if(!Directory::exists(configurationFile)) {
        Error() << "    Error: file" << configurationFile << "does not exist";
        return {};
    }

    const std::string path = Directory::path(configurationFile);
    const Configuration conf(configurationFile, Configuration::Flag::ReadOnly);

    /* Group name */
    if(!conf.hasValue("group")) {
        Error() << "    Error: group name is not specified";
        return {};
    }
    const std::string group = conf.value("group");

    /* Load all files */
    std::vector<const ConfigurationGroup*> files = conf.groups("file");
    std::vector<std::pair<std::string, std::string>> fileData;
    fileData.reserve(files.size());
    for(const auto file: files) {
        const std::string filename = file->value("filename");
        const std::string alias = file->hasValue("alias") ? file->value("alias") : filename;
        if(filename.empty() || alias.empty()) {
            Error() << "    Error: filename or alias of file" << fileData.size()+1 << "in group" << group << "is empty";
            return {};
        }

        std::pair<bool, Containers::Array<char>> contents = fileContents(Directory::join(path, filename));
        if(!contents.first) {
            Error() << "    Error: cannot open file" << filename << "of file" << fileData.size()+1 << "in group" << group;
            return {};
        }
        fileData.emplace_back(alias, std::string{contents.second, contents.second.size()});
    }

    /* The list has to be sorted before passing it to compile() */
    std::sort(fileData.begin(), fileData.end(), lessFilename);

    return compile(name, group, fileData);
}

std::string Resource::compile(const std::string& name, const std::string& group, const std::vector<std::pair<std::string, std::string>>& files) {
    CORRADE_ASSERT(std::is_sorted(files.begin(), files.end(), lessFilename),
        "Utility::Resource::compile(): the file list is not sorted", {});

    /* Special case for empty file list */
    if(files.empty()) {
        return formatString(R"(/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {{

Corrade::Utility::Implementation::ResourceGroup resource;

}}

int resourceInitializer_{0}();
int resourceInitializer_{0}() {{
    resource.name = "{1}";
    resource.count = 0;
    resource.positions = nullptr;
    resource.filenames = nullptr;
    resource.data = nullptr;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
}} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_{0})

int resourceFinalizer_{0}();
int resourceFinalizer_{0}() {{
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
}} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_{0})
)", name, group);
    }

    std::string positions, filenames, data;
    unsigned int filenamesLen = 0, dataLen = 0;

    /* Convert data to hexacodes */
    for(auto it = files.cbegin(); it != files.cend(); ++it) {
        filenamesLen += it->first.size();
        dataLen += it->second.size();

        if(it != files.begin()) {
            filenames += '\n';
            data += '\n';
        }

        positions += Utility::formatString("\n    0x{:.8x},0x{:.8x},", filenamesLen, dataLen);

        filenames += comment(it->first);
        filenames += hexcode(it->first);

        data += comment(it->first);
        data += hexcode(it->second);
    }

    /* Remove last comma from positions and filenames array */
    positions.resize(positions.size()-1);
    filenames.resize(filenames.size()-1);

    /* Remove last comma from data array only if the last file is not empty */
    if(!files.back().second.empty())
        data.resize(data.size()-1);

    /* Return C++ file. The functions have forward declarations to avoid warning
       about functions which don't have corresponding declarations (enabled by
       -Wmissing-declarations in GCC). If we don't have any data, we don't
       create the resourceData array, as zero-length arrays are not allowed. */
    return formatString(R"(/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

namespace {{

const unsigned int resourcePositions[] = {{{0}
}};

const unsigned char resourceFilenames[] = {{{1}
}};

{2}const unsigned char resourceData[] = {{{3}
{2}}};

Corrade::Utility::Implementation::ResourceGroup resource;

}}

int resourceInitializer_{4}();
int resourceInitializer_{4}() {{
    resource.name = "{5}";
    resource.count = {6};
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = {7};
    Corrade::Utility::Resource::registerData(resource);
    return 1;
}} CORRADE_AUTOMATIC_INITIALIZER(resourceInitializer_{4})

int resourceFinalizer_{4}();
int resourceFinalizer_{4}() {{
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
}} CORRADE_AUTOMATIC_FINALIZER(resourceFinalizer_{4})
)",
        positions,                              // 0
        filenames,                              // 1
        dataLen ? "" : "// ",                   // 2
        data,                                   // 3
        name,                                   // 4
        group,                                  // 5
        files.size(),                           // 6
        dataLen ? "resourceData" : "nullptr"    // 7
    );
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
    /* This group can be already overriden from before, so insert if not there
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
        auto overriden = resourceGlobals.overrideGroups->find(groupString);
        if(overriden != resourceGlobals.overrideGroups->end()) {
            Debug{}
                << "Utility::Resource: group '" << Debug::nospace << groupString << Debug::nospace << "' overriden with '" << Debug::nospace << overriden->second << Debug::nospace << "\'";
            _overrideGroup = new OverrideData(overriden->second);

            if(_overrideGroup->conf.value("group") != groupString) Warning{}
                << "Utility::Resource: overriden with different group, found '"
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

    /* The group is overriden with live data */
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
            bool success;
            Containers::Array<char> data;
            std::tie(success, data) = fileContents(Directory::join(Directory::path(_overrideGroup->conf.filename()), file->value("filename")));
            if(!success) {
                Error() << "Utility::Resource::get(): cannot open file" << file->value("filename") << "from overriden group";
                break;
            }

            /* Save the file for later use and return */
            it = _overrideGroup->data.emplace(filenameString, std::move(data)).first;
            return it->second;
        }

        /* The file was not found, fallback to compiled-in ones */
        Warning() << "Utility::Resource::get(): file '" << Debug::nospace
            << filenameString << Debug::nospace << "' was not found in overriden group, fallback to compiled-in resources";
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
