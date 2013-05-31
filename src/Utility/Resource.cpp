/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include <fstream>
#include <iomanip>
#include <sstream>
#include <tuple>
#include <vector>

#include "Utility/Configuration.h"
#include "Utility/Debug.h"
#include "Utility/Directory.h"

namespace Corrade { namespace Utility {

std::map<std::string, std::map<std::string, Resource::ResourceData>>& Resource::resources() {
    static std::map<std::string, std::map<std::string, Resource::ResourceData>> resources;
    return resources;
}

void Resource::registerData(const char* group, unsigned int count, const unsigned char* positions, const unsigned char* filenames, const unsigned char* data) {
    auto groupData = resources().find(group);
    if(groupData == resources().end())
        groupData = resources().emplace(group, std::map<std::string, ResourceData>()).first;

    /* Cast to type which can be eaten by std::string constructor */
    const char* _positions = reinterpret_cast<const char*>(positions);
    const char* _filenames = reinterpret_cast<const char*>(filenames);

    const unsigned int size = sizeof(unsigned int);
    unsigned int oldFilenamePosition = 0, oldDataPosition = 0;

    /* Every 2*sizeof(unsigned int) is one data */
    for(unsigned int i = 0; i != count*2*size; i += 2*size) {
        unsigned int filenamePosition = *reinterpret_cast<const unsigned int*>(_positions+i);
        unsigned int dataPosition = *reinterpret_cast<const unsigned int*>(_positions+i+size);

        ResourceData res{
            oldDataPosition,
            dataPosition-oldDataPosition,
            data};

        groupData->second.emplace(std::string(_filenames+oldFilenamePosition, filenamePosition-oldFilenamePosition), res);

        oldFilenamePosition = filenamePosition;
        oldDataPosition = dataPosition;
    }
}

void Resource::unregisterData(const char* group, const unsigned char* data) {
    if(resources().find(group) == resources().end()) return;

    /* Positions which to remove */
    std::vector<std::string> positions;

    for(auto it = resources()[group].begin(); it != resources()[group].end(); ++it) {
        if(it->second.data == data)
            positions.push_back(it->first);
    }

    /** @todo wtf? this doesn't crash?? */
    for(auto it = positions.cbegin(); it != positions.cend(); ++it)
        resources()[group].erase(*it);

    if(resources()[group].empty()) resources().erase(group);
}

std::string Resource::compileFrom(const std::string& name, const std::string& configurationFile) {
    const std::string path = Directory::path(configurationFile);
    const Configuration conf(configurationFile);

    /* Group name */
    const std::string group = conf.value("group");

    /* Load all files */
    std::vector<const ConfigurationGroup*> files = conf.groups("file");
    std::vector<std::pair<std::string, std::string>> fileData;
    fileData.reserve(files.size());
    for(const auto file: files) {
        Debug() << "Reading file" << fileData.size()+1 << "of" << files.size() << "in group" << '\'' + group + '\'';

        const std::string filename = file->value("filename");
        const std::string alias = file->keyExists("alias") ? file->value("alias") : filename;
        if(filename.empty() || alias.empty()) {
            Error() << "    Error: empty filename or alias!";
            return {};
        }

        Debug() << "   " << filename;
        if(alias != filename) Debug() << " ->" << alias;

        bool success;
        std::string contents;
        std::tie(success, contents) = fileContents(Directory::join(path, filename));
        if(!success) return {};
        fileData.emplace_back(std::move(alias), std::move(contents));
    }

    return compile(name, group, fileData);
}

std::string Resource::compile(const std::string& name, const std::string& group, const std::vector<std::pair<std::string, std::string>>& files) {
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

        positions += hexcode(numberToString(filenamesLen));
        positions += hexcode(numberToString(dataLen));

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
       -Wmissing-declarations in GCC) */
    return "/* Compiled resource file. DO NOT EDIT! */\n\n"
        "#include \"Utility/utilities.h\"\n"
        "#include \"Utility/Resource.h\"\n\n"
        "static const unsigned char resourcePositions[] = {" +
        positions + "\n};\n\n"
        "static const unsigned char resourceFilenames[] = {" +
        filenames + "\n};\n\n"
        "static const unsigned char resourceData[] = {" +
        data +      "\n};\n\n"
        "int resourceInitializer_" + name + "();\n"
        "int resourceInitializer_" + name + "() {\n"
        "    Corrade::Utility::Resource::registerData(\"" + group + "\", " + std::to_string(files.size()) + ", resourcePositions, resourceFilenames, resourceData);\n"
        "    return 1;\n"
        "} AUTOMATIC_INITIALIZER(resourceInitializer_" + name + ")\n\n"
        "int resourceFinalizer_" + name + "();\n"
        "int resourceFinalizer_" + name + "() {\n"
        "    Corrade::Utility::Resource::unregisterData(\"" + group + "\", resourceData);\n"
        "    return 1;\n"
        "} AUTOMATIC_FINALIZER(resourceFinalizer_" + name + ")\n";
}

Resource::Resource(const std::string& group) {
    _group = resources().find(group);
    if(_group == resources().end())
        Error() << "Resource: group" << '\'' + group + '\'' << "was not found";
}

std::pair<const unsigned char*, unsigned int> Resource::getRaw(const std::string& filename) const {
    /* No-op, error already emitted in constructor */
    if(_group == resources().end()) return {};

    /* If the filename doesn't exist, return empty string */
    const auto it = _group->second.find(filename);
    if(it == _group->second.end()) {
        Error() << "Resource: file" << '\'' + filename + '\'' << "was not found in group" << '\'' + _group->first + '\'';
        return {};
    }

    return {it->second.data+it->second.position, it->second.size};
}

std::string Resource::get(const std::string& filename) const {
    const unsigned char* data;
    unsigned int size;
    std::tie(data, size) = getRaw(filename);
    return data ? std::string(reinterpret_cast<const char*>(data), size) : std::string();
}

std::pair<bool, std::string> Resource::fileContents(const std::string& filename) {
    std::ifstream file(filename.data(), std::ifstream::binary);

    if(!file.good()) {
        Error() << "Cannot open file " << filename;
        return {false, std::string()};
    }

    file.seekg(0, std::ios::end);
    if(file.tellg() == 0) return {true, std::string()};
    std::string data(file.tellg(), '\0');
    file.seekg(0, std::ios::beg);
    file.read(&data[0], data.size());

    return {true, std::move(data)};
}

std::string Resource::comment(const std::string& comment) {
    return "\n    /* " + comment + " */";
}

std::string Resource::hexcode(const std::string& data) {
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

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> std::string Resource::numberToString(const T& number) {
    return std::string(reinterpret_cast<const char*>(&number), sizeof(T));
}
#endif

}}
