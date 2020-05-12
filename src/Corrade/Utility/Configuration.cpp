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

#include "Configuration.h"

#include <algorithm>
#include <sstream>
#include <utility>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility {

Configuration::Configuration(const Flags flags): ConfigurationGroup(this), _flags(static_cast<InternalFlag>(std::uint32_t(flags))) {}

Configuration::Configuration(const std::string& filename, const Flags flags): ConfigurationGroup(this), _filename(flags & Flag::ReadOnly ? std::string() : filename), _flags(static_cast<InternalFlag>(std::uint32_t(flags))|InternalFlag::IsValid) {
    /* File doesn't exist yet, nothing to do */
    if(!Directory::exists(filename)) return;

    /* The user wants to truncate the file, mark it as changed and do nothing */
    if(flags & Flag::Truncate) {
        _flags |= InternalFlag::Changed;
        return;
    }

    if(parse(Directory::read(filename))) return;

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

    /** @todo deprecate and remove completely */
    const std::string data{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    if(parse({data.data(), data.size()})) _flags |= InternalFlag::IsValid;
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

bool Configuration::parse(Containers::ArrayView<const char> in) {
    /* Oh, BOM, eww */
    if(in.size() >= 3 && in[0] == Bom[0] && in[1] == Bom[1] && in[2] == Bom[2]) {
        _flags |= InternalFlag::HasBom;
        in = in.suffix(3);
    }

    /* Parse file */
    std::pair<Containers::ArrayView<const char>, const char*> parsed = parse(in, this, {});
    if(parsed.second) {
        Error() << "Utility::Configuration::Configuration():" << parsed.second;
        clear();
        return false;
    }

    CORRADE_INTERNAL_ASSERT(parsed.first.empty());
    return true;
}

std::pair<Containers::ArrayView<const char>, const char*> Configuration::parse(Containers::ArrayView<const char> in, ConfigurationGroup* group, const std::string& fullPath) {
    CORRADE_INTERNAL_ASSERT(fullPath.empty() || String::endsWith(fullPath, '/'));

    std::string buffer;

    /* Parse file */
    bool multiLineValue = false;
    while(!in.empty()) {
        const Containers::ArrayView<const char> currentLine = in;

        /* Extract the line and ignore the newline character after it, if any */
        const char* end = std::find(in.begin(), in.end(), '\n');
        buffer.assign(in.begin(), end);
        in = in.suffix(end == in.end() ? end : end + 1);

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
        String::trimInPlace(buffer);

        /* Empty line */
        if(buffer.empty()) {
            if(_flags & InternalFlag::SkipComments) continue;

            /* Save it only if this is not the last one */
            if(in) group->_values.emplace_back();

        /* Group header */
        } else if(buffer[0] == '[') {

            /* Check ending bracket */
            if(buffer[buffer.size()-1] != ']')
                return {nullptr, "missing closing bracket for a group header"};

            const std::string nextGroup = String::trim(buffer.substr(1, buffer.size()-2));

            if(nextGroup.empty())
                return {nullptr, "empty group name"};

            /* This is a subgroup of this one, parse recursively */
            if(String::beginsWith(nextGroup, fullPath)) {
                ConfigurationGroup::Group g;

                /* If the subgroup has a shorthand for multiple nesting, call
                   parse() on this same line again but with nested group and
                   larger fullPath */
                const std::size_t groupEnd = nextGroup.find('/', fullPath.size());
                if(groupEnd != std::string::npos) {
                    if(groupEnd == fullPath.size())
                        return {nullptr, "empty subgroup name"};

                    g.name = nextGroup.substr(fullPath.size(), groupEnd - fullPath.size());
                    g.group = new ConfigurationGroup(_configuration);
                    /* Add the group before attempting any other parsing, as it
                       could throw an exception and the group would otherwise
                       be leaked */
                    group->_groups.push_back(std::move(g));
                    std::pair<Containers::ArrayView<const char>, const char*> parsed = parse(currentLine, g.group, nextGroup.substr(0, groupEnd + 1));
                    if(parsed.second) return parsed; /* Error, bubble up */
                    in = parsed.first;

                /* Otherwise call parse() on the next line */
                } else {
                    g.name = nextGroup.substr(fullPath.size());
                    g.group = new ConfigurationGroup(_configuration);
                    /* Add the group before attempting any other parsing, as it
                       could throw an exception and the group would otherwise
                       be leaked */
                    group->_groups.push_back(std::move(g));
                    std::pair<Containers::ArrayView<const char>, const char*> parsed = parse(in, g.group, nextGroup + '/');
                    if(parsed.second) return parsed; /* Error, bubble up */
                    in = parsed.first;
                }

            /* Otherwise it's a subgroup of some parent, return the control
               back to caller (again with this line) */
            } else return {currentLine, nullptr};

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
                return {nullptr, "missing equals for a value"};

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
                    return {nullptr, "missing closing quote for a value"};

                item.value = item.value.substr(1, item.value.size()-2);
            }

            group->_values.push_back(item);
        }
    }

    if(multiLineValue)
        return {nullptr, "missing closing quotes for a multi-line value"};

    /* This was the last group */
    return {in, nullptr};
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
    for(std::size_t i = 0; i != group->_groups.size(); ++i) {
        const Group& g = group->_groups[i];

        /* Subgroup name */
        std::string name = g.name;
        if(!fullPath.empty()) name = fullPath + '/' + name;

        /* Omit the name if the group is a first subgroup of given name, has no
           values and only subgroups */
        if(!((i == 0 || group->_groups[i - 1].name != g.name) && g.group->_values.empty() && !g.group->_groups.empty())) {
            buffer = '[' + name + ']' + eol;
            out.write(buffer.data(), buffer.size());
        }

        save(out, eol, g.group, name);
    }
}

}}
