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

#include "Tweakable.h"

#include <cstring>
#include <set>
#include <unordered_map>

#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h"
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

std::pair<bool, void*> Tweakable::registerVariable(const char* const file, const int line, const std::size_t variable, TweakableState(*parser)(Containers::StringView, Containers::StaticArrayView<Implementation::TweakableStorageSize, char>)) {
    CORRADE_INTERNAL_ASSERT(_data);

    /* Find the file in the map */
    /** @todo this allocates and copies the string. std::map::find() in C++14
        has an overload that allows a zero-allocation lookup, but
        std::unordered_map has that only since C++17. */
    auto found = _data->files.find(file);
    if(found == _data->files.end()) {
        /* Strip the directory prefix from the file. If that means the filename
           would then start with a slash, strip that too so Directory::join()
           works correctly -- but don't do that in case the directory prefix
           was empty, in that case the file path was absolute. */
        /** @todo maybe some Directory utility for this? */
        std::string stripped = String::stripPrefix(Directory::fromNativeSeparators(file), _data->prefix);
        if(!_data->prefix.empty() && !stripped.empty() && stripped.front() == '/')
            stripped.erase(0, 1);

        const std::string watchPath = Directory::join(_data->replace, stripped);

        Debug{} << "Utility::Tweakable: watching for changes in" << watchPath;
        /* Ignore errors and do not signal changes if the file is empty in
           order to make everything more robust -- editors are known to be
           doing both */
        found = _data->files.emplace(file, File{watchPath, FileWatcher{watchPath, FileWatcher::Flag::IgnoreChangeIfEmpty|FileWatcher::Flag::IgnoreErrors}, {}}).first;
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
    using namespace Containers::Literals;

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
        /** @todo convert all this to operate on StringViews when we have
            find() as well */
        if(!Containers::StringView{data}.suffix(pos).hasPrefix("CORRADE_TWEAKABLE"_s))
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

TweakableState parseTweakables(const std::string& name, const std::string& filename, const std::string& data, std::vector<TweakableVariable>& variables, std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>>& scopes) {
    /* Prepare "matchers" */
    CORRADE_INTERNAL_ASSERT(!name.empty());
    const char findAnything[] = { '/', '\'', '"', '\n', name[0], 0 };
    constexpr const char findLineCommentEnd[] = "\n";
    constexpr const char findBlockCommentEnd[] = "\n*";
    constexpr const char findStringEnd[] = "\n\"";
    constexpr const char findCharEnd[] = "\n'";
    constexpr const char findRawStringEnd[] = "\n)";

    /* Count the lines, count the variables */
    int line = 1;
    std::size_t variable = 0;

    /* State controlling which matchers we use */
    bool insideLineComment = false;
    bool insideBlockComment = false;
    bool insideString = false;
    bool insideChar = false;
    /* Raw string end delimiter. The sequence is at most 16 chars long
       according to https://en.cppreference.com/w/cpp/language/string_literal,
       including the right parenthesis and the final quote it's 18 chars. */
    char rawStringEndDelimiter[18]{};
    std::size_t rawStringEndDelimiterLength = 0;

    /* Parse the file */
    std::size_t pos = 0;
    const char* find = findAnything;
    TweakableState state = TweakableState::NoChange;
    while((pos = data.find_first_of(find, pos)) != std::string::npos) {
        /* We should be only in one of these at a time */
        CORRADE_INTERNAL_ASSERT(int(insideLineComment) + int(insideBlockComment)  + int(insideChar) + int(insideString) <= 1);

        /* Got a newline */
        if(data[pos] == '\n') {
            ++pos;

            /* Ends a line comment */
            if(insideLineComment) {
                insideLineComment = false;
                find = findAnything;

            /* Doesn't do anything for a block comment */
            } else if(insideBlockComment) {
                /* nothing */

            /* If inside a char or a non-raw string literal, it's an error.
               This will cause unterminated string to be reported after the
               loop. */
            } else if(insideChar || (insideString && !rawStringEndDelimiterLength)) break;

            /* Update the line counter */
            ++line;

        /* Got a potential comment start */
        } else if(data[pos] == '/') {
            /* We're not looking for this character when inside a comment or a
               string, so this shouldn't happen */
            CORRADE_INTERNAL_ASSERT(!insideBlockComment && !insideLineComment && !insideChar && !insideString);

            ++pos;

            /* There should be something after, if not, it's an unterminated
               comment; it'll get reported after the loop ends */
            if(pos == data.size()) break;

            /* Start of a line comment */
            if(data[pos] == '/') {
                ++pos;
                insideLineComment = true;
                find = findLineCommentEnd;

            /* Start of a block comment */
            } else if(data[pos] == '*') {
                ++pos;
                insideBlockComment = true;
                find = findBlockCommentEnd;
            }

            /* Otherwise something else (operator/), no need to do anything */

        /* Got a potential block comment end */
        } else if(data[pos] == '*') {
            /* We should get here only from inside a block comment, never
               directly (i.e., not looking for operator*) */
            CORRADE_INTERNAL_ASSERT(insideBlockComment);

            ++pos;

            /* There should be something after, if not, it's an unterminated
               comment; it'll get reported after the loop ends */
            if(pos == data.size()) break;

            /* End of a block comment */
            if(data[pos] == '/') {
                ++pos;
                insideBlockComment = false;
                find = findAnything;
            }

            /* Otherwise something else (an asterisk inside a comment), no need
               to do anything */

        /* Got a char start or a potential end. In very pathological cases the
           4-char literals like `'_(0)'` (compiler extension) may get mistaken
           as a tweakables, so don't allow that either. */
        } else if(data[pos] == '\'') {
            /* We should get here only when not inside a comment, so either
               from outside or from within a char */
            CORRADE_INTERNAL_ASSERT(!insideLineComment && !insideBlockComment);

            /* Potential char end */
            if(insideChar) {
                /* We should appear here only from within a char. If not
                   escaped, it's the end. */
                CORRADE_INTERNAL_ASSERT(pos);
                if(data[pos - 1] != '\\') {
                    insideChar = false;
                    find = findAnything;
                }

                /* Escaped or not, move after */
                ++pos;

            /* Char start */
            } else {
                insideChar = true;
                ++pos;
                find = findCharEnd;
            }

        /* Got a string start or a potential end */
        } else if(data[pos] == '"') {
            /* We should get here only when not inside a comment or char, so
               either from outside or from within a string */
            CORRADE_INTERNAL_ASSERT(!insideLineComment && !insideBlockComment && !insideChar);

            /* Potential string end */
            if(insideString) {
                /* We should appear here only from within a non-raw stirng. Raw
                   strings search for right parenthesis instead. */
                CORRADE_INTERNAL_ASSERT(pos && !rawStringEndDelimiterLength);

                /* If not escaped, it's the end */
                if(data[pos - 1] != '\\') {
                    insideString = false;
                    find = findAnything;
                }

                /* Escaped or not, move after */
                ++pos;

            /* String start */
            } else {
                insideString = true;

                /* Raw string */
                if(pos && data[pos - 1] == 'R') {
                    ++pos;

                    /* Consume the delimiter, at most 16 characters (+ 1 for
                       the initial parenthesis) */
                    rawStringEndDelimiter[0] = ')';
                    rawStringEndDelimiterLength = 1;
                    while(pos != data.size() && data[pos] != '(' && rawStringEndDelimiterLength < 17)
                        rawStringEndDelimiter[rawStringEndDelimiterLength++] = data[pos++];
                    if(pos == data.size() || data[pos] != '(') {
                        Error{} << "Utility::Tweakable::update(): unterminated raw string delimiter in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                        return TweakableState::Error;
                    }

                    /* Skip the opening parenthesis, finalize the end delimiter
                       and find it in the next round. We need to count newlines
                       inside, so can't just do it directly here. */
                    ++pos;
                    rawStringEndDelimiter[rawStringEndDelimiterLength++] = '"';
                    find = findRawStringEnd;

                /* Classic string */
                } else {
                    ++pos;
                    find = findStringEnd;
                }
            }

        /* Got a potential raw string end */
        } else if(data[pos] == ')') {
            /* We should get here only from within a raw string */
            CORRADE_INTERNAL_ASSERT(insideString && rawStringEndDelimiterLength);

            /* If the delimiter end matches, end the string */
            if(data.compare(pos, rawStringEndDelimiterLength, rawStringEndDelimiter, rawStringEndDelimiterLength) == 0) {
                pos += rawStringEndDelimiterLength;
                insideString = false;
                rawStringEndDelimiterLength = 0;
                find = findAnything;

            /* Otherwise it's just some parenthesis inside, skip it */
            } else ++pos;

        /* Got a potential tweakable macro */
        } else if(data[pos] == name[0]) {
            /* Not a tweakable macro, continue */
            if(data.compare(pos, name.size(), name) != 0) {
                ++pos;
                continue;
            }

            /* We should not get here from comments or raw strings */
            CORRADE_INTERNAL_ASSERT(!insideBlockComment && !insideLineComment && !insideString);

            /* If the immediately preceding character is one of these (and we
               are not at the start of the file), it's something else */
            if(pos && ((data[pos - 1] >= 'A' && data[pos - 1] <= 'Z') ||
                       (data[pos - 1] >= 'a' && data[pos - 1] <= 'z') ||
                       (data[pos - 1] >= '0' && data[pos - 1] <= '9') ||
                        data[pos - 1] == '_' || (data[pos - 1] & 0x80))) {
                pos += name.size();
                continue;
            }

            /* Skip what we found */
            std::size_t beg = pos + name.size();

            /* Get rid of whitespace before the left parenthesis */
            eatWhitespace(data, beg);

            /* If there's no left parenthesis, it's something else */
            if(beg == data.size() || data[beg] != '(') {
                pos = beg;
                continue;
            }

            /* Get rid of whitespace after the parenthesis */
            {
                const std::size_t paren = ++beg;
                eatWhitespace(data, beg);
                if(beg == data.size()) {
                    Error{} << "Utility::Tweakable::update(): unterminated" << data.substr(pos, paren) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                    return TweakableState::Error;
                }
            }

            /* Everything between beg and end is the literal */
            std::size_t end = beg;

            /* A string -- parse until the next unescaped " */
            /** @todo once string parsers are possible (they need heap alloc),
                    combine this with the global string ignore, then also test
                    for multiline strings (which are a syntax error) */
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
            /** @todo once string parsers are possible (they need heap alloc),
                    combine this with the global char ignore, then also test
                    for multiline chars (which are a syntax error) */
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

            /** @todo once string parsers are possible (they need heap alloc),
                    combine this with the global (raw) string ignore, then also
                    test for raw strings */
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

            ++end;

            /* If the variable doesn't have a parser assigned, it means the app
               haven't run this code path yet. That's not a critical problem. */
            if(variables.size() <= variable || !variables[variable].parser) {
                Warning{} << "Utility::Tweakable::update(): ignoring unknown new value" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;

            /* Otherwise we should have a parser that can convert the string
               representation to the target type */
            } else {
                Implementation::TweakableVariable& v = variables[variable];

                /* If the variable is not on the same line as before, the code
                   changed. Request a recompile. */
                /** @todo SHA-1 the source (minus tweakables) and compare that for full verification */
                if(v.line != line) {
                    Warning{} << "Utility::Tweakable::update(): code changed around" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line << Debug::nospace << ", requesting a recompile";
                    return TweakableState::Recompile;
                }

                /* Parse the variable. If a recompile is requested or an error
                   occured, exit immediately. */
                const TweakableState variableState = v.parser(value, Containers::staticArrayView(v.storage));
                if(variableState == TweakableState::Recompile) {
                    Warning{} << "Utility::Tweakable::update(): change of" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line << "requested a recompile";
                    return TweakableState::Recompile;
                }
                if(variableState == TweakableState::Error) {
                    Error{} << "Utility::Tweakable::update(): error parsing" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                    return TweakableState::Error;
                }

                /* If a change occured, add a corresponding scope to update */
                if(variableState != TweakableState::NoChange) {
                    CORRADE_INTERNAL_ASSERT(variableState == TweakableState::Success);
                    Debug{} << "Utility::Tweakable::update(): updating" << data.substr(pos, end - pos) << "in" << filename << Debug::nospace << ":" << Debug::nospace << line;
                    if(v.scopeLambda) scopes.emplace(v.scopeLambda, v.scopeUserCall, v.scopeUserData);
                    state = TweakableState::Success;
                }
            }

            /* Increase variable ID for the next round to match __COUNTER__,
               update pos to restart the search after this variable */
            pos = end;
            ++variable;

        /* Shouldn't get here */
        } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    /* Being inside a line comment is okay, being inside a block comment is not */
    if(insideBlockComment) {
        Error{} << "Utility::Tweakable::update(): unterminated block comment in" << filename << Debug::nospace << ":" << Debug::nospace << line;
        return TweakableState::Error;
    }

    /* Being inside a char is not okay */
    if(insideChar) {
        Error{} << "Utility::Tweakable::update(): unterminated character literal in" << filename << Debug::nospace << ":" << Debug::nospace << line;
        return TweakableState::Error;
    }

    /* Being inside any string is also not okay */
    if(insideString) {
        Error{} << "Utility::Tweakable::update(): unterminated" << (rawStringEndDelimiterLength ? "raw string" : "string") << "literal in" << filename << Debug::nospace << ":" << Debug::nospace << line;
        return TweakableState::Error;
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

        /* Print helpful message in case no alias was found. Don't do name ==
           "CORRADE_TWEAKABLE" to avoid a temporary allocation of std::string.
           (Ugh, why can't it have an overload for this?!) */
        if(name.compare("CORRADE_TWEAKABLE") == 0)
            Warning{} << "Utility::Tweakable::update(): no alias found in" << file.first << Debug::nospace << ", fallback to looking for CORRADE_TWEAKABLE()";
        else
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
