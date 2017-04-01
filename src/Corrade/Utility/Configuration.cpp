/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Configuration.h"

#include <sstream>
#include <utility>
#include <vector>

#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility {

Configuration::Configuration(const Flags flags): ConfigurationGroup(this), _flags(static_cast<InternalFlag>(std::uint32_t(flags))) {}

Configuration::Configuration(const std::string& filename, const Flags flags): ConfigurationGroup(this), _filename(flags & Flag::ReadOnly ? std::string() : filename), _flags(static_cast<InternalFlag>(std::uint32_t(flags))|InternalFlag::IsValid) {
    /* File doesn't exist yet, nothing to do */
    if(!Directory::fileExists(filename)) return;

    /* The user wants to truncate the file, mark it as changed and do nothing */
    if(flags & Flag::Truncate) {
        _flags |= InternalFlag::Changed;
        return;
    }

    /* Read full contents of a file and then pass it via stringstream to the
       parser. Doing it this way to avoid Unicode filename issues on Windows. */
    /** @todo get rid of streams altogether */
    if(!Directory::fileExists(filename))
        Error() << "Utility::Configuration::Configuration(): cannot open file" << filename;
    std::istringstream in{Directory::readString(filename)};
    if(parse(in)) return;

    /* Error, reset everything back */
    _filename = {};
    _flags &= ~InternalFlag::IsValid;
}

Configuration::Configuration(std::istream& in, const Flags flags): ConfigurationGroup(this), _flags(static_cast<InternalFlag>(std::uint32_t(flags))) {
    /* The user wants to truncate the file, mark it as changed and do nothing */
    if(flags & Flag::Truncate) {
        _flags |= (InternalFlag::Changed|InternalFlag::IsValid);
        return;
    }

    if(parse(in)) _flags |= InternalFlag::IsValid;
}

Configuration::Configuration(Configuration&& other): ConfigurationGroup{std::move(other)}, _filename{std::move(other._filename)}, _flags{other._flags} {
    /* Redirect configuration pointer to this instance */
    setConfigurationPointer(this);
}

Configuration::~Configuration() { if(_flags & InternalFlag::Changed) save(); }

Configuration& Configuration::operator=(Configuration&& other) {
    ConfigurationGroup::operator=(std::move(other));
    _filename = std::move(other._filename);
    _flags = other._flags;

    /* Redirect configuration pointer to this instance */
    setConfigurationPointer(this);

    return *this;
}

void Configuration::setConfigurationPointer(ConfigurationGroup* group) {
    group->_configuration = this;

    for(auto& g: group->_groups) setConfigurationPointer(g.group);
}

std::string Configuration::filename() const { return _filename; }

void Configuration::setFilename(std::string filename) {
    _filename = std::move(filename);
}

namespace {
    constexpr const char Bom[] = "\xEF\xBB\xBF";
}

bool Configuration::parse(std::istream& in) {
    try {
        /* It looks like BOM */
        if(in.peek() == Bom[0]) {
            char bom[4];
            in.get(bom, 4);

            /* This is not a BOM, rewind back */
            if(bom[0] != Bom[0] || bom[1] != Bom[1] || bom[2] != Bom[2]) in.seekg(0);

            /* Or set flag */
            else _flags |= InternalFlag::HasBom;
        }

        /* Parse file */
        CORRADE_INTERNAL_ASSERT_OUTPUT(parse(in, this, {}).empty());

    } catch(std::string e) {
        Error() << "Utility::Configuration::Configuration():" << e;
        clear();
        return false;
    }

    return true;
}

std::string Configuration::parse(std::istream& in, ConfigurationGroup* group, const std::string& fullPath) {
    std::string buffer;

    /* Parse file */
    bool multiLineValue = false;
    while(in.good()) {
        std::getline(in, buffer);

        /* Windows EOL */
        if(!buffer.empty() && buffer.back() == '\r')
            _flags |= InternalFlag::WindowsEol;

        /* Multi-line value */
        if(multiLineValue) {
            /* End of multi-line value */
            if(String::trim(buffer) == "\"\"\"") {
                /* Remove trailing newline, if present */
                if(!group->_values.back().value.empty()) {
                    CORRADE_INTERNAL_ASSERT(group->_values.back().value.back() == '\n');
                    group->_values.back().value.resize(group->_values.back().value.size()-1);
                }

                multiLineValue = false;
                continue;
            }

            /* Remove Windows EOL, if present */
            if(!buffer.empty() && buffer.back() == '\r') buffer.resize(buffer.size()-1);

            /* Append it (with newline) to current value */
            group->_values.back().value += buffer;
            group->_values.back().value += '\n';
            continue;
        }

        /* Trim buffer */
        buffer = String::trim(buffer);

        /* Empty line */
        if(buffer.empty()) {
            if(_flags & InternalFlag::SkipComments) continue;

            group->_values.emplace_back();

        /* Group header */
        } else if(buffer[0] == '[') {

            /* Check ending bracket */
            if(buffer[buffer.size()-1] != ']')
                throw std::string("missing closing bracket for group header");

            std::string nextGroup = String::trim(buffer.substr(1, buffer.size()-2));

            if(nextGroup.empty())
                throw std::string("empty group name");

            /* Next group is subgroup of current group, recursive call */
            while(!nextGroup.empty() && (fullPath.empty() || nextGroup.substr(0, fullPath.size()) == fullPath)) {
                ConfigurationGroup::Group g;
                g.name = nextGroup.substr(fullPath.size());
                g.group = new ConfigurationGroup(_configuration);
                /* Add the group before attempting any other parsing, as it
                   could throw an exception and the group would otherwise be
                   leaked */
                group->_groups.push_back(std::move(g));
                nextGroup = parse(in, g.group, nextGroup+'/');
            }

            return nextGroup;

        /* Comment */
        } else if(buffer[0] == '#' || buffer[0] == ';') {
            if(_flags & InternalFlag::SkipComments) continue;

            ConfigurationGroup::Value item;
            item.value = buffer;
            group->_values.push_back(item);

        /* Key/value pair */
        } else {
            const std::size_t splitter = buffer.find_first_of('=');
            if(splitter == std::string::npos)
                throw std::string("key/value pair without '=' character");

            ConfigurationGroup::Value item;
            item.key = String::trim(buffer.substr(0, splitter));
            item.value = String::trim(buffer.substr(splitter+1));

            /* Start of multi-line value */
            if(item.value == "\"\"\"") {
                item.value = "";
                multiLineValue = true;

            /* Remove quotes, if present */
            /** @todo Check `"` characters better */
            } else if(!item.value.empty() && item.value[0] == '"') {
                if(item.value.size() < 2 || item.value[item.value.size()-1] != '"')
                    throw std::string("missing closing quotes in value");

                item.value = item.value.substr(1, item.value.size()-2);
            }

            group->_values.push_back(item);
        }
    }

    /* Remove last empty line, if present (will be written automatically) */
    if(!group->_values.empty() && group->_values.back().key.empty() && group->_values.back().value.empty())
        group->_values.pop_back();

    /* This was the last group */
    return {};
}

bool Configuration::save(const std::string& filename) {
    /* Save to a stringstream and then write it as a string to the file. Doing
       it this way to avoid issues with Unicode filenames on Windows. */
    /** @todo get rid of streams altogether */
    std::ostringstream out;
    save(out);
    if(Directory::writeString(filename, out.str()))
        return true;

    Error() << "Utility::Configuration::save(): cannot open file" << filename;
    return false;
}

void Configuration::save(std::ostream& out) {
    /* BOM, if user explicitly wants that crap */
    if((_flags & InternalFlag::PreserveBom) && (_flags & InternalFlag::HasBom))
        out.write(Bom, 3);

    /* EOL character */
    std::string eol;
    if(_flags & (InternalFlag::ForceWindowsEol|InternalFlag::WindowsEol) && !(_flags & InternalFlag::ForceUnixEol)) eol = "\r\n";
    else eol = "\n";

    /** @todo Checking file.good() after every operation */
    /** @todo Backup file */

    /* Recursively save all groups */
    save(out, eol, this, {});
}

bool Configuration::save() {
    if(_filename.empty()) return false;
    return save(_filename);
}

namespace {
    constexpr bool isWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r' || c == '\n';
    }
}

void Configuration::save(std::ostream& out, const std::string& eol, ConfigurationGroup* group, const std::string& fullPath) const {
    CORRADE_INTERNAL_ASSERT(group->configuration() == this);
    std::string buffer;

    /* Foreach all items in the group */
    for(const Value& value: group->_values) {
        /* Key/value pair */
        if(!value.key.empty()) {
            /* Multi-line value */
            if(value.value.find_first_of('\n') != std::string::npos) {
                /* Replace \n with `eol` */
                /** @todo fixme: ugly and slow */
                std::string valueString = value.value;
                std::size_t pos = 0;
                while((pos = valueString.find_first_of('\n', pos)) != std::string::npos) {
                    valueString.replace(pos, 1, eol);
                    pos += eol.size();
                }

                buffer = value.key + "=\"\"\"" + eol + valueString + eol + "\"\"\"" + eol;

            /* Value with leading/trailing spaces */
            } else if(!value.value.empty() && (isWhitespace(value.value.front()) || isWhitespace(value.value.back()))) {
                buffer = value.key + "=\"" + value.value + '"' + eol;

            /* Value without spaces */
            } else buffer = value.key + '=' + value.value + eol;
        }

        /* Comment / empty line */
        else buffer = value.value + eol;

        out.write(buffer.data(), buffer.size());
    }

    /* Recursively process all subgroups */
    for(const Group& g: group->_groups) {
        /* Subgroup name */
        std::string name = g.name;
        if(!fullPath.empty()) name = fullPath + '/' + name;

        buffer = '[' + name + ']' + eol;
        out.write(buffer.data(), buffer.size());

        save(out, eol, g.group, name);
    }
}

}}
