/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Tweakable.h"

#include <cstring>
#include <set>
#include <unordered_map>

#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FileWatcher.h"
#include "Corrade/Utility/String.h"

#include "Corrade/Utility/Implementation/tweakable.h"

namespace Corrade { namespace Utility {

namespace {
    Tweakable* globalInstance = nullptr;

    struct File {
        std::string watchPath;
        FileWatcher watcher;
        std::vector<Implementation::TweakableVariable> variables;
    };
}

struct Tweakable::Data {
    explicit Data(const std::string& prefix, const std::string& replace): prefix{prefix}, replace{replace} {}

    std::string prefix, replace;
    std::unordered_map<std::string, File> files;

    void(*currentScopeLambda)(void(*)(), void*) = nullptr;
    void(*currentScopeUserCall)() = nullptr;
    void* currentScopeUserData = nullptr;
};

Tweakable& Tweakable::instance() {
    CORRADE_ASSERT(globalInstance, "Utility::Tweakable: no instance created", *globalInstance);
    return *globalInstance;
}

Tweakable::Tweakable() {
    CORRADE_ASSERT(!globalInstance, "Utility::Tweakable: another instance is already active", );
    globalInstance = this;
}

Tweakable::~Tweakable() {
    CORRADE_INTERNAL_ASSERT(globalInstance == this);
    globalInstance = nullptr;
}

void Tweakable::enable() { Tweakable::enable({}, {}); }

void Tweakable::enable(const std::string& prefix, const std::string& replace) {
    _data.reset(new Data{prefix, replace});
}

void Tweakable::scopeInternal(void(*lambda)(void(*)(), void*), void(*userCall)(), void* userData) {
    if(_data) {
        _data->currentScopeLambda = lambda;
        _data->currentScopeUserCall = userCall;
        _data->currentScopeUserData = userData;
    }

    lambda(userCall, userData);

    if(_data) {
        _data->currentScopeLambda = nullptr;
        _data->currentScopeUserCall = nullptr;
        _data->currentScopeUserData = nullptr;
    }
}

std::pair<bool, void*> Tweakable::registerVariable(const char* const file, const int line, const std::size_t variable, TweakableState(*parser)(Containers::ArrayView<const char>, Containers::StaticArrayView<Implementation::TweakableStorageSize, char>)) {
    CORRADE_INTERNAL_ASSERT(_data);

    /* Find the file in the map */
    /** @todo this allocates and copies the string. std::map::find() in C++14
        has an overload that allows a zero-allocation lookup, but
        std::unordered_map doesn't. */
    auto found = _data->files.find(file);
    if(found == _data->files.end()) {
        /* Strip the directory prefix from the file. If that means the filename
           would then start with a slash, strip that too so Directory::join()
           works correctly. */
        /** @todo maybe some Directory utility for this? */
        std::string stripped = String::stripPrefix(Directory::fromNativeSeparators(file), _data->prefix);
        if(!stripped.empty() && stripped.front() == '/')
            stripped.erase(0, 1);

        const std::string watchPath = Directory::join(_data->replace, stripped);

        Debug{} << "Utility::Tweakable: watching for changes in" << watchPath;
        found = _data->files.emplace(file, File{watchPath, FileWatcher{watchPath}, {}}).first;
    }

    /* Extend the variable list to contain this one as well */
    if(found->second.variables.size() <= variable)
        found->second.variables.resize(variable + 1);

    /* Save the variable, if not already */
    Implementation::TweakableVariable& v = found->second.variables[variable];
    bool initialized = true;
    if(!v.parser) {
        initialized = false;
        v.line = line;
        v.parser = parser;
        v.scopeLambda = _data->currentScopeLambda;
        v.scopeUserCall = _data->currentScopeUserCall;
        v.scopeUserData = _data->currentScopeUserData;
    }

    return {initialized, v.storage};
}

namespace Implementation {

namespace {
    /* This doesn't eat newlines ATM because it would break the line counter.
       Also, for findTweakableAlias(), it *can't* eat newlines. */
    void eatWhitespace(const std::string& data, std::size_t& pos) {
        while(pos < data.size() && (data[pos] == ' ' || data[pos] == '\t'))
            ++pos;
    }
}

std::string findTweakableAlias(const std::string& data) {
    std::string name = "CORRADE_TWEAKABLE";
    std::size_t pos = 0;
    while((pos = data.find("#define", pos)) != std::string::npos) {
        /* Eat all whitespace before */
        std::size_t prev = pos;
        while(prev && (data[prev - 1] == ' ' || data[prev - 1] == '\t'))
            --prev;

        /* Skip what we found, so `continue`s will not cause an infinite loop */
        pos += 7;

        /* If this is not at the start of a line (or first in the file), nope */
        if(prev && data[prev - 1] != '\n')
            continue;

        /* Get rid of whitespace */
        std::size_t beg = pos;
        eatWhitespace(data, beg);

        /* Consume the name */
        std::size_t end = beg;
        while((data[end] >= 'A' && data[end] <= 'Z') ||
              (data[end] >= 'a' && data[end] <= 'z') ||
              (data[end] >= '0' && data[end] <= '9' && end != pos) ||
              (data[end] == '_')) ++end;

        /* Get rid of whitespace after */
        pos = end;
        eatWhitespace(data, pos);

        /* If the rest doesn't read CORRADE_TWEAKABLE, nope */
        if(!String::viewBeginsWith({data.data() + pos, data.size() - pos}, "CORRADE_TWEAKABLE"))
            continue;

        /* Get rid of whitespace at the end of the line */
        pos += name.size();
        eatWhitespace(data, pos);

        /* If there is something else than a newline or EOF, nope */
        if(pos < data.size() && data[pos] != '\r' && data[pos] != '\n')
            continue;

        /* Save the name */
        name = data.substr(beg, end - beg);
        break;
    }

    return name;
}

TweakableState parseTweakables(std::string& name, const std::string& filename, const std::string& data, std::vector<TweakableVariable>& variables, std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>>& scopes) {
    /* For simplicity disallow a space in front of ( */
    name += '(';

    /* Count the lines, count the variables */
    int line = 1;
    std::size_t variable = 0;

    /* Go through all occurences, parse them and figure out the final state
       from that */
    std::size_t lastNewlinePos = 0;
    std::size_t pos = 0;
    TweakableState state = TweakableState::NoChange;
    while((pos = data.find(name, pos)) != std::string::npos) {
        /* Count the newlines until this occurence */
        while((lastNewlinePos = data.find('\n', lastNewlinePos)) < pos) {
            ++lastNewlinePos;
            ++line;
        }

        /* If the immediately preceding character is one of these (and we are
           not at the start of the file), it's something else */
        if(pos && ((data[pos - 1] >= 'A' && data[pos - 1] <= 'Z') ||
                   (data[pos - 1] >= 'a' && data[pos - 1] <= 'z') ||
                   (data[pos - 1] >= '0' && data[pos - 1] <= '9') ||
                    data[pos - 1] == '_' || data[pos - 1] < 0)) {
            pos += name.size();
            continue;
        }

        /* Skip what we found */
        std::size_t beg = pos + name.size();

        /* Get rid of whitespace after */
        eatWhitespace(data, beg);
        if(beg == data.size()) {
            Error{} << "Utility::Tweakable::update(): unterminated" << name << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
            return TweakableState::Error;
        }

        /* Everything between beg and end is the literal */
        std::size_t end = beg;

        /* A string -- parse until the next unescaped " */
        if(data[beg] == '"') {
            end = beg + 1;
            while((end = data.find('"', end)) != std::string::npos) {
                if(data[end - 1] != '\\') break;
                ++end;
            }
            if(end == std::string::npos) {
                Error{} << "Utility::Tweakable::update(): unterminated string" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                return TweakableState::Error;
            }

            ++end;

        /* A char -- parse until the next unescaped ' */
        } else if(data[beg] == '\'') {
            end = beg + 1;
            while((end = data.find('\'', end)) != std::string::npos) {
                if(data[end - 1] != '\\') break;
                ++end;
            }
            if(end == std::string::npos) {
                Error{} << "Utility::Tweakable::update(): unterminated char" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                return TweakableState::Error;
            }

            ++end;

        /* I will *never* implement this awful thing */
        } else if(data[beg] == 'L') {
            Error{} << "Utility::Tweakable::update(): unsupported wide char/string literal" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
            return TweakableState::Error;

        /** @todo implement */
        } else if(data[beg] == 'u' || data[beg] == 'U' || data[beg] == 'R') {
            Error{} << "Utility::Tweakable::update(): unsupported unicode/raw char/string literal" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
            return TweakableState::Error;

        /* Something else, simply take everything that makes sense in a literal */
        } else {
            end = beg;
            while(end < data.size() &&
                /* Besides the true/false keywords, custom literals can have
                   any letter. ' is for C++14 thousands separator. */
                ((data[end] >= 'A' && data[end] <= 'Z') ||
                 (data[end] >= 'a' && data[end] <= 'z') ||
                 (data[end] >= '0' && data[end] <= '9') ||
                  data[end] == '+' || data[end] == '-' ||
                  data[end] == '.' || data[end] == 'x' ||
                  data[end] == 'X' || data[end] == '\'' ||
                  data[end] == '_')) ++end;
        }

        /* Save the value range */
        const Containers::ArrayView<const char> value{data.data() + beg, end - beg};

        /* Get rid of whitespace after, after that there should be the
           ending parenthesis */
        eatWhitespace(data, end);
        if(end == data.size() || data[end] != ')') {
            Error{} << "Utility::Tweakable::update(): unterminated" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
            return TweakableState::Error;
        }

        /* If the variable doesn't have a parser assigned, it means the app
           haven't run this code path yet. That's not a critical problem. */
        if(variables.size() <= variable || !variables[variable].parser) {
            Warning{} << "Utility::Tweakable::update(): ignoring unknown new value" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;

        /* Otherwise we should have a parser that can convert the string
           representation to the target type */
        } else {
            Implementation::TweakableVariable& v = variables[variable];

            /* If the variable is not on the same line as before, the code
               changed. Request a recompile. */
            /** @todo SHA-1 the source (minus tweakables) and compare that for full verification */
            if(v.line != line) {
                Warning{} << "Utility::Tweakable::update(): code changed around" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line << Debug::nospace << ", requesting a recompile";
                return TweakableState::Recompile;
            }

            /* Parse the variable. If a recompile is requested or an error
               occured, exit immediately. */
            const TweakableState variableState = v.parser(value, Containers::staticArrayView(v.storage));
            if(variableState == TweakableState::Recompile) {
                Warning{} << "Utility::Tweakable::update(): change of" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line << "requested a recompile";
                return TweakableState::Recompile;
            }
            if(variableState == TweakableState::Error) {
                Error{} << "Utility::Tweakable::update(): error parsing" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                return TweakableState::Error;
            }

            /* If a change occured, add a corresponding scope to update */
            if(variableState != TweakableState::NoChange) {
                CORRADE_INTERNAL_ASSERT(variableState == TweakableState::Success);
                Debug{} << "Utility::Tweakable::update(): updating" << data.substr(pos, end + 1 - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                if(v.scopeLambda) {
                    #ifndef CORRADE_GCC47_COMPATIBILITY
                    scopes.emplace(v.scopeLambda, v.scopeUserCall, v.scopeUserData);
                    #else
                    scopes.insert(std::make_tuple(v.scopeLambda, v.scopeUserCall, v.scopeUserData));
                    #endif
                }
                state = TweakableState::Success;
            }
        }

        /* Increase variable ID for the next round to match __COUNTER__,
           update pos to restart the search after this variable */
        pos = end + 1;
        ++variable;
    }

    return state;
}

}

TweakableState Tweakable::update() {
    if(!_data) return TweakableState::NoChange;

    /* Set of unique scopes that have to be re-run after variable updates.
       Using a classic set with a tuple so it does all the and comparison for
       me. STL is good sometimes. Can't use unordered_set because tuple doesn't
       have a hash specialization. */
    std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;

    /* Go through all watchers and check for changes */
    TweakableState state = TweakableState::NoChange;
    for(auto& file: _data->files) {
        /** @todo suggest recompile if the watcher is not valid anymore */
        if(!file.second.watcher.hasChanged()) continue;

        /* First go through all defines and search if there is any alias. There
           shouldn't be many. If no alias is found, assume CORRADE_TWEAKABLE. */
        const std::string data = Directory::readString(file.second.watchPath);
        std::string name = Implementation::findTweakableAlias(data);

        Debug{} << "Utility::Tweakable::update(): looking for updated" << name << Debug::nospace << "() macros in" << file.first;

        /* Now find all annotated constants and update them. If there's a
           problem, exit immediately, otherwise just accumulate the state. */
        const TweakableState fileState = Implementation::parseTweakables(name, file.first, data, file.second.variables, scopes);
        if(fileState == TweakableState::NoChange)
            continue;
        else if(fileState == TweakableState::Success)
            state = TweakableState::Success;
        else return fileState;
    }

    if(!scopes.empty()) {
        Debug{} << "Utility::Tweakable::update():" << scopes.size() << "scopes affected";

        /* Go through all scopes and call them */
        for(auto& scope: scopes)
            std::get<0>(scope)(std::get<1>(scope), std::get<2>(scope));
    }

    return state;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
Debug& operator<<(Debug& debug, const TweakableState value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case TweakableState::value: return debug << "Utility::TweakableState::" #value;
        _c(NoChange)
        _c(Success)
        _c(Recompile)
        _c(Error)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "Utility::TweakableState(" << Debug::nospace << reinterpret_cast<void*>(std::uint8_t(value)) << Debug::nospace << ")";
}
#endif

}}
