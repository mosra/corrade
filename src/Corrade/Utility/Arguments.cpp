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

/* For Arguments::environment() */
#if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include <cstdio>
extern char **environ;
#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <emscripten.h>
#endif
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include <windows.h>
#include "Corrade/Utility/Unicode.h"
using Corrade::Utility::Unicode::widen;
using Corrade::Utility::Unicode::narrow;
#endif

namespace Corrade { namespace Utility {

namespace {
    enum class Type: std::uint8_t {
        Argument,
        NamedArgument,
        Option,
        BooleanOption
    };

    inline std::string uppercaseKey(std::string key) {
        for(char& i: key) {
            if(i >= 'a' && i <= 'z')
                i = 'A' + i - 'a';
            else if(i == '-')
                i = '_';
        }

        return key;
    }
}

struct Arguments::Entry {
    Entry(Type type, char shortKey, std::string key, std::string helpKey, std::string defaultValue, std::size_t id);

    Type type;
    char shortKey;
    std::string key, help, helpKey, defaultValue;
    #ifndef CORRADE_TARGET_WINDOWS_RT
    std::string environment;
    #endif
    std::size_t id;
};

Arguments::Entry::Entry(Type type, char shortKey, std::string key, std::string helpKey, std::string defaultValue, std::size_t id): type(type), shortKey(shortKey), key(std::move(key)), defaultValue(std::move(defaultValue)), id(id) {
    if(type == Type::NamedArgument || type == Type::Option)
        this->helpKey = this->key + ' ' + uppercaseKey(helpKey);
    else this->helpKey = std::move(helpKey);

    CORRADE_INTERNAL_ASSERT(type == Type::Option || this->defaultValue.empty());
}

std::vector<std::string> Arguments::environment() {
    std::vector<std::string> list;

    /* Standard Unix and local Emscripten environment */
    #if defined(CORRADE_TARGET_UNIX) || defined(CORRADE_TARGET_EMSCRIPTEN)
    for(char** e = environ; *e; ++e)
        list.emplace_back(*e);

    /* System environment provided by Node.js. Hopefully nobody uses \b in
       environment variables. (Can't use \0 because Emscripten chokes on it.) */
    #ifdef CORRADE_TARGET_EMSCRIPTEN
    #ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Winvalid-pp-token"
    #endif
    char* const env = reinterpret_cast<char*>(EM_ASM_INT_V({
        var env = '';
        if(typeof process !== 'undefined') for(var key in process.env)
            env += key + '=' + process.env[key] + '\b';
        env += '\b';
        return allocate(intArrayFromString(env), 'i8', ALLOC_NORMAL);
    }));
    #ifdef __clang__
    #pragma GCC diagnostic pop
    #endif
    char* e = env;
    while(*e != '\b') {
        char* end = std::strchr(e, '\b');
        list.push_back({e, std::size_t(end - e)});
        e = end + 1;
    }
    std::free(env);
    #endif

    /* Windows (not RT) */
    #elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    wchar_t* const env = GetEnvironmentStringsW();
    for(wchar_t* e = env; *e; e += std::wcslen(e) + 1)
        list.push_back(narrow(e));
    FreeEnvironmentStrings(env);

    /* Other platforms not implemented */
    #else
    #endif

    return list;
}

Arguments::Arguments(const std::string& prefix): _prefix{prefix + '-'} {
    /* Add help option */
    addBooleanOption("help");
    setHelp("help", "display this help message and exit");
}

Arguments::Arguments() {
    /* Add help option */
    addBooleanOption('h', "help");
    setHelp("help", "display this help message and exit");
}

Arguments::~Arguments() = default;

Arguments& Arguments::addArgument(std::string key) {
    CORRADE_ASSERT(_prefix.empty(),
        "Utility::Arguments::addArgument(): argument" << key << "not allowed in prefixed version", *this);

    CORRADE_ASSERT(!key.empty(), "Utility::Arguments::addArgument(): key must not be empty", *this);

    /* Verify that the argument has unique key */
    CORRADE_ASSERT(find(key) == _entries.end(), "Utility::Arguments::addArgument(): the key" << key << "is already used", *this);

    std::string helpKey = key;
    _entries.emplace_back(Type::Argument, '\0', std::move(key), std::move(helpKey), std::string(), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addNamedArgument(char shortKey, std::string key) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addNamedArgument(): invalid key" << key << "or its short variant", *this);

    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(_prefix + key) == _entries.end(),
        "Utility::Arguments::addNamedArgument(): the key" << key << "or its short version is already used", *this);

    CORRADE_ASSERT(_prefix.empty(),
        "Utility::Arguments::addNamedArgument(): argument" << key << "not allowed in prefixed version", *this);

    std::string helpKey = key;
    _entries.emplace_back(Type::NamedArgument, shortKey, std::move(key), std::move(helpKey), std::string(), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addOption(char shortKey, std::string key, std::string defaultValue) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addOption(): invalid key" << key << "or its short variant", *this);

    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(_prefix + key) == _entries.end(),
        "Utility::Arguments::addOption(): the key" << key << "or its short version is already used", *this);

    CORRADE_ASSERT(_prefix.empty() || shortKey == '\0',
        "Utility::Arguments::addOption(): short option" << std::string{shortKey} << "not allowed in prefixed version", *this);
    CORRADE_ASSERT(!skippedPrefix(key),
        "Utility::Arguments::addOption(): key" << key << "conflicts with skipped prefixes", *this);

    std::string helpKey;
    if(_prefix.empty())
        helpKey = key;
    else {
        std::string tmp = std::move(key);
        key = _prefix + tmp;
        helpKey = std::move(tmp);
    }
    _entries.emplace_back(Type::Option, shortKey, std::move(key), std::move(helpKey), std::move(defaultValue), _values.size());
    _values.emplace_back();
    return *this;
}

Arguments& Arguments::addBooleanOption(char shortKey, std::string key) {
    CORRADE_ASSERT(verifyKey(shortKey) && verifyKey(key),
        "Utility::Arguments::addBooleanOption(): invalid key" << key << "or its short variant", *this);

    CORRADE_ASSERT((!shortKey || find(shortKey) == _entries.end()) && find(key) == _entries.end(),
        "Utility::Arguments::addBooleanOption(): the key" << key << "or its short version is already used", *this);

    CORRADE_ASSERT(_prefix.empty() || key == "help",
        "Utility::Arguments::addBooleanOption(): boolean option" << key << "not allowed in prefixed version", *this);
    CORRADE_ASSERT(!skippedPrefix(key),
        "Utility::Arguments::addBooleanOption(): key" << key << "conflicts with skipped prefixes", *this);

    /* The prefix addition is here only for --prefix-help, which is the only
       allowed boolean option */
    std::string helpKey;
    if(_prefix.empty())
        helpKey = key;
    else
        helpKey = key = _prefix + std::move(key);
    _entries.emplace_back(Type::BooleanOption, shortKey, std::move(key), std::move(helpKey), std::string(), _booleans.size());
    _booleans.push_back(false);
    return *this;
}

namespace {
    inline bool keyHasPrefix(const std::string& key, const std::string& prefix) {
        if(key.size() < prefix.size()) return false;
        return std::equal(prefix.begin(), prefix.end(), key.begin());
    }
}

Arguments& Arguments::addSkippedPrefix(std::string prefix, std::string help) {
    CORRADE_ASSERT(!skippedPrefix(prefix),
        "Utility::Arguments::addSkippedPrefix(): prefix" << prefix << "already added", *this);

    /* Verify that no already added option conflicts with this */
    #if !defined(CORRADE_NO_ASSERT) || defined(CORRADE_GRACEFUL_ASSERT)
    for(const Entry& entry: _entries)
        CORRADE_ASSERT(!keyHasPrefix(entry.key, prefix),
            "Utility::Arguments::addSkippedPrefix(): skipped prefix" << prefix << "conflicts with existing keys", *this);
    #endif

    /* Add `-` to the end so we always compare with `--prefix-` and not just
       `--prefix` */
    prefix += '-';

    _skippedPrefixes.emplace_back(std::move(prefix), std::move(help));
    return *this;
}

#ifndef CORRADE_TARGET_WINDOWS_RT
Arguments& Arguments::setFromEnvironment(const std::string& key, std::string environmentVariable) {
    auto found = find(_prefix + key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::setFromEnvironment(): key" << key << "doesn't exist", *this);
    CORRADE_ASSERT(found->type == Type::Option || found->type == Type::BooleanOption,
        "Utility::Arguments::setFromEnvironment(): only options can be set from environment", *this);

    found->environment = std::move(environmentVariable);
    return *this;
}

Arguments& Arguments::setFromEnvironment(const std::string& key) {
    return setFromEnvironment(key, uppercaseKey(_prefix + key));
}
#endif

Arguments& Arguments::setCommand(std::string name) {
    _command = std::move(name);
    return *this;
}

Arguments& Arguments::setHelp(std::string help) {
    CORRADE_ASSERT(_prefix.empty(),
        "Utility::Arguments::setHelp(): global help text only allowed in unprefixed version", *this);

    _help = std::move(help);
    return *this;
}

Arguments& Arguments::setHelp(const std::string& key, std::string help, std::string helpKey) {
    auto found = find(_prefix + key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::setHelp(): key" << key << "doesn't exist", *this);

    found->help = std::move(help);

    if(!helpKey.empty()) {
        CORRADE_ASSERT(found->type != Type::BooleanOption,
            "Utility::Arguments::setHelpKey(): help key can't be set for boolean option", *this);

        if(found->type == Type::NamedArgument || found->type == Type::Option)
            found->helpKey = _prefix + key + ' ' + std::move(helpKey);
        else found->helpKey = std::move(helpKey);
    }

    return *this;
}

#ifdef CORRADE_BUILD_DEPRECATED
Arguments& Arguments::setHelpKey(const std::string& key, std::string helpKey) {
    auto found = find(_prefix + key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::setHelp(): key" << key << "doesn't exist", *this);
    return setHelp(key, found->help, std::move(helpKey));
}
#endif

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
    /* Save command name */
    if(_command.empty() && argv && argc >= 1) _command = argv[0];

    /* Clear previously parsed values */
    for(auto&& boolean: _booleans) boolean = false;
    for(const Entry& entry: _entries) {
        if(entry.type == Type::BooleanOption) continue;

        CORRADE_INTERNAL_ASSERT(entry.id < _values.size());
        _values[entry.id] = entry.defaultValue;
    }

    /* Get options from environment */
    #ifndef CORRADE_TARGET_WINDOWS_RT
    for(const Entry& entry: _entries) {
        if(entry.environment.empty()) continue;

        /* UTF-8 handling on sane platforms */
        #ifndef CORRADE_TARGET_WINDOWS
        const char* const env = std::getenv(entry.environment.data());
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        #ifdef __clang__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
        #endif
        char* const systemEnv = reinterpret_cast<char*>(EM_ASM_INT({
            var name = UTF8ToString($0);
            return typeof process !== 'undefined' && name in process.env ? allocate(intArrayFromString(process.env[name]), 'i8', ALLOC_NORMAL) : 0;
        }, entry.environment.data()));
        #ifdef __clang__
        #pragma GCC diagnostic pop
        #endif
        #endif

        #ifndef CORRADE_TARGET_EMSCRIPTEN
        if(!env) continue;
        #else
        if(!env && !systemEnv) continue;
        #endif

        /* Mess with UTF-16 on Windows */
        #else
        const wchar_t* const wenv = _wgetenv(widen(entry.environment).data());
        if(!wenv) continue;
        std::string env{narrow(wenv)};
        #endif

        if(entry.type == Type::BooleanOption) {
            CORRADE_INTERNAL_ASSERT(entry.id < _booleans.size());
            _booleans[entry.id] = String::uppercase(
                #ifndef CORRADE_TARGET_EMSCRIPTEN
                env
                #else
                env ? env : systemEnv
                #endif
                ) == "ON";
        } else {
            CORRADE_INTERNAL_ASSERT(entry.id < _values.size());
            _values[entry.id] =
                #ifndef CORRADE_TARGET_EMSCRIPTEN
                env
                #else
                env ? env : systemEnv;
                #endif
                ;
        }

        #ifdef CORRADE_TARGET_EMSCRIPTEN
        std::free(systemEnv);
        #endif
    }
    #endif

    std::vector<Entry>::iterator valueFor = _entries.end();
    bool optionsAllowed = true;
    std::vector<Entry>::iterator nextArgument = _entries.begin();
    std::vector<bool> parsedArguments(_entries.size());

    for(int i = 1; i < argc; ++i) {
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
                /* Ignore if this is the prefixed version */
                if(!_prefix.empty()) continue;

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

                    /* If this is prefixed version and the option does not have
                       the prefix, ignore */
                    if(!_prefix.empty() && !keyHasPrefix(key, _prefix))
                        continue;

                    /* If skipped prefix, ignore the option and its value */
                    bool ignore = false;
                    for(auto&& prefix: _skippedPrefixes) {
                        if(!keyHasPrefix(key, prefix.first)) continue;

                        /* Ignore the option and also its value (except for
                           help, which is the only allowed boolean option) */
                        ignore = true;
                        if(key != prefix.first + "help") ++i;
                        break;
                    }
                    if(ignore) continue;

                    /* Find the option */
                    found = find(key);
                    if(found == _entries.end()) {
                        Error() << "Unknown command-line argument" << std::string("--") + key;
                        return false;
                    }

                /* Typo (long option with only one dash). Ignore them if this
                   is prefixed version because they can actually be values of
                   some long options. */
                } else {
                    if(!_prefix.empty()) continue;

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
            /* Ignore if this is the prefixed version */
            if(!_prefix.empty()) continue;

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

    /* Print all skipped prefixes */
    for(auto&& prefix: _skippedPrefixes)
        out << " [--" << prefix.first << "...]";

    /* Print all options and named argument */
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

    /* Print ellipsis for main application arguments, if this is an prefixed
       version */
    if(!_prefix.empty()) out << " ...";

    out << '\n';

    return out.str();
}

std::string Arguments::help() const {
    std::ostringstream out;
    out << usage();

    /* Global help text */
    if(!_help.empty())
        out << '\n' << _help << '\n';

    /* Calculate key column width. Minimal is to display `-h, --help` */
    constexpr std::size_t maxKeyColumnWidth = 26;
    std::size_t keyColumnWidth = 10;
    for(auto&& prefix: _skippedPrefixes) {
        /* Add space for `--` at the beginning and `...` at the end */
        keyColumnWidth = std::max(prefix.first.size() + 5, keyColumnWidth);

        /* If the key width is larger than maximum, cut it. Also no need to
           process more entries, as no key width can be larger than this */
        if(keyColumnWidth >= maxKeyColumnWidth) {
            keyColumnWidth = maxKeyColumnWidth;
            break;
        }
    }

    /* If prefixes are already long enough, no need to go through the entries */
    if(keyColumnWidth != maxKeyColumnWidth) for(const Entry& entry: _entries) {
        /* Skip arguments and options without default value, environment or
           help text (won't be printed, so they shouldn't contribute to the
           width) */
        if(entry.type == Type::Argument || (entry.defaultValue.empty() && entry.help.empty()
            #ifndef CORRADE_TARGET_WINDOWS_RT
            && entry.environment.empty()
            #endif
        ))
            continue;

        /* Compute size of current key column */
        std::size_t currentKeyColumnWidth = entry.helpKey.size();
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

    /* If prefixed, print the info about unprefixed arguments */
    if(!_prefix.empty()) {
        out << "  " << std::left << std::setw(keyColumnWidth) << "..." << "  main application arguments\n"
            << std::string(keyColumnWidth + 4, ' ') << "(see -h or --help for details)\n";
    }

    /* Print all arguments first */
    for(const Entry& entry: _entries) {
        /* Skip non-arguments and arguments without help text */
        if(entry.type != Type::Argument || entry.help.empty()) continue;

        out << "  " << std::left << std::setw(keyColumnWidth) << entry.helpKey << "  " << entry.help << '\n';
    }

    /* Print all named arguments and options second */
    for(const Entry& entry: _entries) {
        /* Skip arguments and options without default value, environment or
           help text (no additional info to show) */
        if(entry.type == Type::Argument || (entry.defaultValue.empty() && entry.help.empty()
            #ifndef CORRADE_TARGET_WINDOWS_RT
            && entry.environment.empty()
            #endif
        ))
            continue;

        /* Key name */
        out << "  ";
        if(entry.shortKey)
            out << '-' << entry.shortKey << ", ";
        out << "--" << std::left << std::setw(keyColumnWidth - (entry.shortKey ? 6 : 2)) << entry.helpKey << "  ";

        /* Help text */
        if(!entry.help.empty()) out << entry.help << '\n';

        /* Value taken from environment */
        #ifndef CORRADE_TARGET_WINDOWS_RT
        if(!entry.environment.empty()) {
            if(!entry.help.empty()) out << std::string(keyColumnWidth + 4, ' ');
            out << "(environment: " << entry.environment;
            if(entry.type == Type::BooleanOption) out << "=ON|OFF";
            out << ")\n";
        }
        #endif

        /* Default value, put it on new indented line (two spaces from the
           left and one from the right additionaly to key column width), if
           help text is also present */
        if(!entry.defaultValue.empty()) {
            if(!entry.help.empty()) out << std::string(keyColumnWidth + 4, ' ');
            out << "(default: " << entry.defaultValue << ")\n";
        }
    }

    /* Print references to skipped prefies last */
    for(auto&& prefix: _skippedPrefixes) {
        out << "  --" << std::left << std::setw(keyColumnWidth) << prefix.first + "...  ";
        if(!prefix.second.empty()) out << prefix.second << '\n' << std::string(keyColumnWidth + 4, ' ');
        out << "(see --" << prefix.first << "help for details)\n";
    }

    return out.str();
}

std::string Arguments::valueInternal(const std::string& key) const {
    const auto found = find(_prefix + key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::value(): key" << key << "not found", {});
    CORRADE_ASSERT(found->type != Type::BooleanOption,
        "Utility::Arguments::value(): cannot use this function for boolean option" << key, {});
    CORRADE_INTERNAL_ASSERT(found->id < _values.size());
    return _values[found->id];
}

bool Arguments::isSet(const std::string& key) const {
    const auto found = find(_prefix + key);
    CORRADE_ASSERT(found != _entries.end(), "Utility::Arguments::value(): key" << key << "not found", false);
    CORRADE_ASSERT(found->type == Type::BooleanOption,
        "Utility::Arguments::isSet(): cannot use this function for non-boolean value" << key, false);
    CORRADE_INTERNAL_ASSERT(found->id < _booleans.size());
    return _booleans[found->id];
}

bool Arguments::skippedPrefix(const std::string& key) const {
    for(auto&& prefix: _skippedPrefixes)
        if(keyHasPrefix(key, prefix.first)) return true;

    return false;
}

bool Arguments::verifyKey(const std::string& key) const {
    static constexpr const char allowed[] { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-" };

    return key.size() > 1 && key.find_first_not_of(allowed) == std::string::npos;
}

bool Arguments::verifyKey(char shortKey) const {
    static constexpr const char allowedShort[] { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" };

    return !shortKey || std::strchr(allowedShort, shortKey) != nullptr;
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

auto Arguments::findNextArgument(const std::vector<Entry>::iterator start) -> std::vector<Entry>::iterator {
    for(auto it = start; it != _entries.end(); ++it)
        if(it->type == Type::Argument) return it;

    return _entries.end();
}

inline std::string Arguments::keyName(const Entry& entry) const {
    return entry.type == Type::Argument ? entry.helpKey : "--" + entry.key;
}

}}
