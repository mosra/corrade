/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include "Arguments.h"

#include <cstdlib>
#include <cstring>
#ifdef _MSC_VER
#include <algorithm> /* std::max() */
#endif
#include <iomanip>
#include <sstream>

#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility {

namespace {
    enum class Type: std::uint8_t {
        Argument,
        NamedArgument,
        Option,
        BooleanOption
    };
}

struct Arguments::Entry {
    Entry(Type type, char shortKey, std::string key, std::string defaultValue, std::size_t id);

    Type type;
    char shortKey;
    std::string key, help, helpKey, defaultValue;
    std::size_t id;
};

Arguments::Entry::Entry(Type type, char shortKey, std::string key, std::string defaultValue, std::size_t id): type(type), shortKey(shortKey), key(std::move(key)), defaultValue(std::move(defaultValue)), id(id) {
    if(type == Type::NamedArgument || type == Type::Option)
        helpKey = this->key + ' ' + String::uppercase(this->key);
    else helpKey = this->key;

    CORRADE_INTERNAL_ASSERT(type == Type::Option || this->defaultValue.empty());
}

Arguments::Arguments() {
    /* Add help option */
    addBooleanOption('h', "help");
    setHelp("help", "display this help message and exit");
}

Arguments::~Arguments() = default;

Arguments& Arguments::addArgument(std::string key) {
    CORRADE_ASSERT(!key.empty(), "Utility::Arguments::addArgument(): key must not be empty", *this);

    /* Verify that the argument has unique key */
    CORRADE_ASSERT(find(key) == _entries.end(), "Utility::Arguments::addArgument(): the key" << key << "is already used", *this);

    _entries.emplace_back(Type::Argument, '\0', std::move(key), std::string(), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addNamedArgument(char shortKey, std::string key) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addNamedArgument(): invalid key" << key << "or its short variant", *this);

    /* Verify that the option has unique key */
    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(key) == _entries.end(),
        "Utility::Arguments::addNamedArgument(): the key" << key << "or its short version is already used", *this);

    _entries.emplace_back(Type::NamedArgument, shortKey, std::move(key), std::string(), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addOption(char shortKey, std::string key, std::string defaultValue) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addOption(): invalid key" << key << "or its short variant", *this);

    /* Verify that the option has unique key */
    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(key) == _entries.end(),
        "Utility::Arguments::addOption(): the key" << key << "or its short version is already used", *this);

    _entries.emplace_back(Type::Option, shortKey, std::move(key), std::move(defaultValue), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addBooleanOption(char shortKey, std::string key) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addBooleanOption(): invalid key" << key << "or its short variant", *this);

    /* Verify that the option has unique key */
    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(key) == _entries.end(),
        "Utility::Arguments::addBooleanOption(): the key" << key << "or its short version is already used", *this);

    _entries.emplace_back(Type::BooleanOption, shortKey, std::move(key), std::string(), _booleans.size());
    _booleans.push_back(false);
    return *this;
}

Arguments& Arguments::setCommand(std::string name) {
    _command = std::move(name);
    return *this;
}

Arguments& Arguments::setHelp(std::string help) {
    _help = std::move(help);
    return *this;
}

Arguments& Arguments::setHelp(const std::string& key, std::string help) {
    auto found = find(key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::setHelp(): key" << key << "doesn't exist", *this);
    found->help = std::move(help);
    return *this;
}

Arguments& Arguments::setHelpKey(const std::string& key, std::string helpKey) {
    auto found = find(key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::setHelp(): key" << key << "doesn't exist", *this);
    CORRADE_ASSERT(found->type != Type::BooleanOption,
        "Utility::Arguments::setHelpKey(): help key can't be set for boolean option", *this);

    if(found->type == Type::NamedArgument || found->type == Type::Option)
        found->helpKey = key + ' ' + std::move(helpKey);
    else found->helpKey = std::move(helpKey);

    return *this;
}

void Arguments::parse(const int argc, const char** const argv) {
    const bool status = tryParse(argc, argv);

    if(isSet("help")) {
        Debug() << help();
        std::exit(0);
    }

    if(!status) {
        Error() << usage();
        std::exit(1);
    }
}

bool Arguments::tryParse(const int argc, const char** const argv) {
    CORRADE_INTERNAL_ASSERT(argc >= 1);

    /* Save command name */
    if(_command.empty()) _command = argv[0];

    /* Clear previously parsed values */
    for(auto it = _booleans.begin(); it != _booleans.end(); ++it) *it = false;
    for(const Entry& entry: _entries) {
        if(entry.type == Type::BooleanOption) continue;

        CORRADE_INTERNAL_ASSERT(entry.id < _values.size());
        _values[entry.id] = entry.defaultValue;
    }

    /* Check for --help first, exit if found */
    if(argc >= 2 && (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)) {
        CORRADE_INTERNAL_ASSERT(_entries[0].shortKey == 'h' && _entries[0].id == 0);
        _booleans[0] = true;
        return true;
    }

    std::vector<Entry>::iterator valueFor = _entries.end();
    bool optionsAllowed = true;
    std::vector<Entry>::iterator nextArgument = _entries.begin();
    std::vector<bool> parsedArguments(_entries.size());

    for(int i = 1; i != argc; ++i) {
        /* Value for given argument */
        if(valueFor != _entries.end()) {
            CORRADE_INTERNAL_ASSERT(valueFor->type != Type::BooleanOption);
            CORRADE_INTERNAL_ASSERT(valueFor->id < _values.size());
            _values[valueFor->id] = argv[i];
            parsedArguments[valueFor-_entries.begin()] = true;
            valueFor = _entries.end();
            continue;
        }

        const std::size_t len = std::strlen(argv[i]);

        /* Option or named argument */
        if(optionsAllowed && len != 0 && argv[i][0] == '-') {
            std::vector<Entry>::iterator found = _entries.end();
            /** @todo Option merging (-aBcd) */

            /* Short option */
            if(len == 2) {
                const char key = argv[i][1];

                /* Option / argument separator */
                if(key == '-') {
                    optionsAllowed = false;
                    continue;
                }

                if(!verifyKey(key)) {
                    Error() << "Invalid command-line argument" << std::string("-") + key;
                    return false;
                }

                /* Find the option */
                found = find(key);
                if(found == _entries.end()) {
                    Error() << "Unknown command-line argument" << std::string("-") + key;
                    return false;
                }

            /* Long option */
            } else if(len > 2) {
                if(std::strncmp(argv[i], "--", 2) == 0) {
                    const std::string key = argv[i]+2;
                    if(!verifyKey(key)) {
                        Error() << "Invalid command-line argument" << std::string("--") + key;
                        return false;
                    }

                    /* Find the option */
                    found = find(key);
                    if(found == _entries.end()) {
                        Error() << "Unknown command-line argument" << std::string("--") + key;
                        return false;
                    }

                /* Typo (long option with only one dash) */
                } else {
                    Error() << "Invalid command-line argument" << argv[i] << std::string("(did you mean -") + argv[i] + "?)";
                    return false;
                }
            }

            /* Boolean option */
            CORRADE_INTERNAL_ASSERT(found != _entries.end());
            if(found->type == Type::BooleanOption) {
                CORRADE_INTERNAL_ASSERT(found->id < _booleans.size());
                _booleans[found->id] = true;
                parsedArguments[found-_entries.begin()] = true;

            /* Value option, save in next cycle */
            } else valueFor = found;

        /* Argument */
        } else {
            /* Find next argument */
            const auto found = findNextArgument(nextArgument);
            if(found == _entries.end()) {
                Error() << "Superfluous command-line argument" << argv[i];
                return false;
            }

            _values[found->id] = argv[i];
            parsedArguments[found-_entries.begin()] = true;
            nextArgument = found+1;
        }
    }

    /* Expected value, but none given */
    if(valueFor != _entries.end()) {
        Error() << "Missing value for command-line argument" << keyName(*valueFor);
        return false;
    }

    bool success = true;

    /* Check missing options */
    for(std::size_t i = 0; i != _entries.size(); ++i) {
        const Entry& entry = _entries[i];

        /* Non-mandatory, nothing to do */
        if(entry.type == Type::BooleanOption || entry.type == Type::Option)
            continue;

        /* Argument was not parsed */
        if(parsedArguments[i] != true) {
            Error() << "Missing command-line argument" << keyName(_entries[i]);
            success = false;
        }
    }

    return success;
}

std::string Arguments::usage() const {
    std::ostringstream out;
    out << "Usage:\n  " << (_command.empty() ? "./app" : _command);

    /* Print all options and named argument first */
    bool hasArguments = false;
    for(const Entry& entry: _entries) {
        if(entry.type == Type::Argument) {
            hasArguments = true;
            continue;
        }

        out << ' ';

        /* Optional */
        if(entry.type == Type::Option || entry.type == Type::BooleanOption)
            out << '[';

        /* Key name (+ value) */
        if(entry.shortKey)
            out << '-' << entry.shortKey << '|';
        out << "--" << entry.helpKey;

        /* Optional */
        if(entry.type == Type::Option || entry.type == Type::BooleanOption)
            out << ']';
    }

    /* Separator between named arguments (options) and unnamed arguments. Help
       option is always present. */
    if(hasArguments) out << " [--]";

    /* Print all arguments second */
    for(const Entry& entry: _entries) {
        if(entry.type != Type::Argument) continue;

        out << ' ' << entry.helpKey;
    }

    out << '\n';

    return out.str();
}

std::string Arguments::help() const {
    std::ostringstream out;
    out << usage();

    /* Global help text */
    if(!_help.empty())
        out << '\n' << _help << '\n';

    /* Compute key column width. Minimal is to display `-h, --help` */
    constexpr std::size_t maxKeyColumnWidth = 27;
    std::size_t keyColumnWidth = 11;
    for(const Entry& entry: _entries) {
        /* Entry which will not be printed, skip */
        if(entry.help.empty() || (entry.type == Type::Option && entry.defaultValue.empty()))
            continue;

        /* Compute size of current key column. Make it so the key name is
           separated from the help text ideally with two spaces. If the key is
           too long, the width will be stripped to maximum and the help text
           will be separated with one space only to save horizontal space. */
        std::size_t currentKeyColumnWidth = 1 + entry.helpKey.size();
        if(entry.type != Type::Argument) {
            currentKeyColumnWidth += 2;
            if(entry.shortKey) currentKeyColumnWidth += 4;
        }

        keyColumnWidth = std::max(currentKeyColumnWidth, keyColumnWidth);

        /* If the key width is larger than maximum, cut it. Also no need to
           process more entries, as no key width can be larger than this */
        if(keyColumnWidth >= maxKeyColumnWidth) {
            keyColumnWidth = maxKeyColumnWidth;
            break;
        }
    }

    /* Argument and option list */
    out << "\nArguments:\n";

    /* Print all arguments first */
    for(const Entry& entry: _entries) {
        /* Skip non-arguments and arguments without help text */
        if(entry.type != Type::Argument || entry.help.empty()) continue;

        out << "  " << std::left << std::setw(keyColumnWidth) << entry.helpKey << ' ' << entry.help << '\n';
    }

    /* Print all named arguments and options second */
    for(const Entry& entry: _entries) {
        /* Skip arguments and options without default value/help text */
        if(entry.type == Type::Argument || (entry.defaultValue.empty() && entry.help.empty()))
            continue;

        /* Key name */
        out << "  ";
        if(entry.shortKey)
            out << '-' << entry.shortKey << ", ";
        out << "--" << std::left << std::setw(keyColumnWidth - (entry.shortKey ? 6 : 2)) << entry.helpKey << ' ';

        /* Help text */
        if(!entry.help.empty()) out << entry.help << '\n';

        /* Default value, put it on new indented line (two spaces from the
            left and one from the right additionaly to key column width), if
            help text is also present */
        if(!entry.defaultValue.empty()) {
            if(!entry.help.empty()) out << std::string(keyColumnWidth + 3, ' ');
            out << "(default: " << entry.defaultValue << ")\n";
        }
    }

    return out.str();
}

std::string Arguments::valueInternal(const std::string& key) const {
    const auto found = find(key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::value(): key" << key << "not found", {});
    CORRADE_ASSERT(found->type != Type::BooleanOption,
        "Utility::Arguments::value(): cannot use this function for boolean option" << key, {});
    CORRADE_INTERNAL_ASSERT(found->id < _values.size());
    return _values[found->id];
}

bool Arguments::isSet(const std::string& key) const {
    const auto found = find(key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::value(): key" << key << "not found", false);
    CORRADE_ASSERT(found->type == Type::BooleanOption,
        "Utility::Arguments::isSet(): cannot use this function for non-boolean value" << key, false);
    CORRADE_INTERNAL_ASSERT(found->id < _booleans.size());
    return _booleans[found->id];
}

bool Arguments::verifyKey(const std::string& key) const {
    static const std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";

    return key.size() > 1 && key.find_first_not_of(allowed) == std::string::npos;
}

bool Arguments::verifyKey(char shortKey) const {
    static const std::string allowedShort = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    return !shortKey || allowedShort.find(shortKey) != std::string::npos;
}

auto Arguments::find(const std::string& key) -> std::vector<Entry>::iterator {
    for(auto it = _entries.begin(); it != _entries.end(); ++it)
        if(it->key == key) return it;

    return _entries.end();
}

auto Arguments::find(const std::string& key) const -> std::vector<Entry>::const_iterator {
    for(auto it = _entries.begin(); it != _entries.end(); ++it)
        if(it->key == key) return it;

    return _entries.end();
}

auto Arguments::find(const char shortKey) -> std::vector<Entry>::iterator {
    for(auto it = _entries.begin(); it != _entries.end(); ++it)
        if(it->shortKey == shortKey) return it;

    return _entries.end();
}

auto Arguments::find(const char shortKey) const -> std::vector<Entry>::const_iterator {
    for(auto it = _entries.begin(); it != _entries.end(); ++it)
        if(it->shortKey == shortKey) return it;

    return _entries.end();
}

auto Arguments::findNextArgument(const std::vector<Entry>::iterator start) -> std::vector<Entry>::iterator {
    for(auto it = start; it != _entries.end(); ++it)
        if(it->type == Type::Argument) return it;

    return _entries.end();
}

inline std::string Arguments::keyName(const Entry& entry) const {
    return entry.type == Type::Argument ? entry.helpKey : "--" + entry.key;
}

}}
