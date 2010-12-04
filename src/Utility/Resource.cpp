/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Resource.h"

#include <sstream>
#include <vector>

using namespace std;

namespace Kompas { namespace Utility {

map<string, map<string, Resource::ResourceData> > Resource::resources;

void Resource::registerData(const char* group, unsigned int count, const unsigned char* positions, const unsigned char* filenames, const unsigned char* data) {
    if(resources.find(group) == resources.end()) resources.insert(pair<string, map<string, ResourceData> >(group, map<string, ResourceData>()));

    /* Cast to type which can be eaten by std::string constructor */
    const char* _positions = reinterpret_cast<const char*>(positions);
    const char* _filenames = reinterpret_cast<const char*>(filenames);

    unsigned int size = sizeof(unsigned int);
    unsigned int oldFilenamePosition = 0, oldDataPosition = 0;

    /* Every 2*sizeof(unsigned int) is one data */
    for(unsigned int i = 0; i != count*2*size; i=i+2*size) {
        unsigned int filenamePosition = numberFromString<unsigned int>(string(_positions+i, size));
        unsigned int dataPosition = numberFromString<unsigned int>(string(_positions+i+size, size));

        ResourceData res;
        res.data = data;
        res.position = oldDataPosition;
        res.size = dataPosition-oldDataPosition;

        string filename = string(_filenames+oldFilenamePosition, filenamePosition-oldFilenamePosition);
        resources[group].insert(pair<string, ResourceData>(filename, res));

        oldFilenamePosition = filenamePosition;
        oldDataPosition = dataPosition;
    }
}

void Resource::unregisterData(const char* group, const unsigned char* data) {
    if(resources.find(group) == resources.end()) return;

    /* Positions which to remove */
    vector<string> positions;

    for(map<string, ResourceData>::iterator it = resources[group].begin(); it != resources[group].end(); ++it) {
        if(it->second.data == data)
            positions.push_back(it->first);
    }

    for(vector<string>::const_iterator it = positions.begin(); it != positions.end(); ++it)
        resources[group].erase(*it);

    if(resources[group].empty()) resources.erase(group);
}

string Resource::compile(const string& name, const map<string, string>& files) const {
    string positions, filenames, data;
    unsigned int filenamesLen = 0, dataLen = 0;

    /* Convert data to hexacodes */
    for(map<string, string>::const_iterator it = files.begin(); it != files.end(); ++it) {
        filenamesLen += it->first.size();
        dataLen += it->second.size();

        positions += hexcode(numberToString(filenamesLen));
        positions += hexcode(numberToString(dataLen));

        filenames += hexcode(it->first, it->first);
        data += hexcode(it->second, it->first);
    }

    /* Remove last comma from data */
    positions = positions.substr(0, positions.size()-2);
    filenames = filenames.substr(0, filenames.size()-2);
    data = data.substr(0, data.size()-2);

    /* Resource count */
    ostringstream count;
    count << files.size();

    /* Return C++ file */
    return "/* Compiled resource file. DO NOT EDIT! */\n\n"
        "#include \"Utility/utilities.h\"\n"
        "#include \"Utility/Resource.h\"\n\n"
        "static const unsigned char resourcePositions[] = {\n" +
        positions + "\n};\n\n"
        "static const unsigned char resourceFilenames[] = {\n" +
        filenames + "\n};\n\n"
        "static const unsigned char resourceData[] = {\n" +
        data +      "\n};\n\n"
        "int resourceInitializer_" + name + "() {\n"
        "    Kompas::Utility::Resource::registerData(\"" + group + "\", " + count.str() + ", resourcePositions, resourceFilenames, resourceData);\n"
        "    return 1;\n"
        "} AUTOMATIC_INITIALIZER(resourceInitializer_" + name + ")\n\n"
        "int resourceFinalizer_" + name + "() {\n"
        "    Kompas::Utility::Resource::unregisterData(\"" + group + "\", resourceData);\n"
        "    return 1;\n"
        "} AUTOMATIC_FINALIZER(resourceFinalizer_" + name + ")\n";
}

string Resource::compile(const string& name, const string& filename, const string& data) const {
    std::map<std::string, std::string> files;
    files.insert(std::pair<std::string, std::string>(filename, data));
    return compile(name, files);
}

string Resource::get(const std::string& filename) const {
    /* If the group/filename doesn't exist, return empty string */
    if(resources.find(group) == resources.end() || resources[group].find(filename) == resources[group].end()) return "";

    const ResourceData& r = resources[group][filename];
    return string(reinterpret_cast<const char*>(r.data)+r.position, r.size);
}

string Resource::hexcode(const string& data, const string& comment) const {
    /* Add comment, if set */
    string output = "    ";
    if(!comment.empty()) output = "\n    /* " + comment + " */\n" + output;

    int row_len = 4;
    for(unsigned int i = 0; i != data.size(); ++i) {

        /* Every row is indented by four spaces and is max 80 characters long */
        if(row_len > 74) {
            output += "\n    ";
            row_len = 4;
        }

        /* Convert char to hex */
        ostringstream converter;
        converter << std::hex;
        converter << static_cast<unsigned int>(static_cast<unsigned char>(data[i]));

        /* Append to output */
        output += "0x" + converter.str() + ",";
        row_len += 3+converter.str().size();
    }

    return output + '\n';
}

template<class T> string Resource::numberToString(const T& number) {
    return string(reinterpret_cast<const char*>(&number), sizeof(T));
}

template<class T> T Resource::numberFromString(const std::string& number) {
    return *reinterpret_cast<const T*>(number.c_str());
}

}}
