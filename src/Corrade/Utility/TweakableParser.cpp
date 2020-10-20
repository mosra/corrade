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

#include "TweakableParser.h"

#include <cstring>
#include <algorithm> /** @todo get rid of this once StringView::find() exists */

#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Tweakable.h"

namespace Corrade { namespace Utility {

namespace {
    std::pair<const char*, int> integerBase(Containers::StringView value) {
        using namespace Containers::Literals;
        if(value.hasPrefix("0x"_s) || value.hasPrefix("0X"_s))
            return {value.data() + 2, 16};
        if(value.hasPrefix("0b"_s) || value.hasPrefix("0B"_s))
            return {value.data() + 2, 2};
        if(value.hasPrefix("0"_s))
            return {value.data() + 1, 8};
        return {value.data(), 10};
    }
}

std::pair<TweakableState, int> TweakableParser<int>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const int result = std::strtol(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end()) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, unsigned int> TweakableParser<unsigned int>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const unsigned int result = std::strtoul(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    /* If value would be empty, the above catches that */
    if(value.back() != 'u' && value.back() != 'U') {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected u";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 1) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, long> TweakableParser<long>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const long result = std::strtol(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    /* If value would be empty, the above catches that */
    if(value.back() != 'l' && value.back() != 'L') {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected l";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 1) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, unsigned long> TweakableParser<unsigned long>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const unsigned long result = std::strtoul(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    if(value.size() < 2 ||
      (value[value.size() - 1] != 'l' && value[value.size() - 1] != 'L' &&
       value[value.size() - 2] != 'u' && value[value.size() - 2] != 'U'))
    {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected ul";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 2) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, long long> TweakableParser<long long>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const long long result = std::strtoll(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    if(value.size() < 2 ||
      (value[value.size() - 1] != 'l' && value[value.size() - 1] != 'L' &&
       value[value.size() - 2] != 'l' && value[value.size() - 2] != 'L'))
    {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected ll";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 2) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, unsigned long long> TweakableParser<unsigned long long>::parse(Containers::StringView value) {
    const std::pair<const char*, int> valueBase = integerBase(value);
    char* end;
    const int result = std::strtoull(valueBase.first, &end, valueBase.second);

    if(end == value.begin()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not an integer literal";
        return {TweakableState::Recompile, {}};
    }

    if(value.size() < 3 ||
      (value[value.size() - 1] != 'l' && value[value.size() - 1] != 'L' &&
       value[value.size() - 2] != 'l' && value[value.size() - 2] != 'L' &&
       value[value.size() - 2] != 'u' && value[value.size() - 2] != 'U'))
    {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected ull";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 3) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after an integer literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, float> TweakableParser<float>::parse(Containers::StringView value) {
    char* end;
    const float result = std::strtof(value.data(), &end);

    if(end == value.begin() || std::find(value.begin(), value.end(), '.') == value.end()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    /* If value would be empty, the above catches that */
    if(value.back() != 'f' && value.back() != 'F') {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected f";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 1) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

std::pair<TweakableState, double> TweakableParser<double>::parse(Containers::StringView value) {
    char* end;
    const double result = std::strtod(value.data(), &end);

    if(end == value.begin() || std::find(value.begin(), value.end(), '.') == value.end()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end()) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
std::pair<TweakableState, long double> TweakableParser<long double>::parse(Containers::StringView value) {
    char* end;
    const long double result = std::strtold(value.data(), &end);

    if(end == value.begin() || std::find(value.begin(), value.end(), '.') == value.end()) {
        Warning{} << "Utility::TweakableParser:" << value << "is not a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    /* If value would be empty, the above catches that */
    if(value.back() != 'l' && value.back() != 'L') {
        Warning{} << "Utility::TweakableParser:" << value << "has an unexpected suffix, expected l";
        return {TweakableState::Recompile, {}};
    }

    if(end != value.end() - 1) {
        Warning{} << "Utility::TweakableParser: unexpected characters" << value.suffix(end) << "after a floating-point literal";
        return {TweakableState::Recompile, {}};
    }

    return {TweakableState::Success, result};
}
#endif

std::pair<TweakableState, char> TweakableParser<char>::parse(Containers::StringView value) {
    if(value.size() < 3 || value.front() != '\'' || value.back() != '\'') {
        Warning{} << "Utility::TweakableParser:" << value << "is not a character literal";
        return {TweakableState::Recompile, {}};
    }

    if(value[1] == '\\') {
        Error{} << "Utility::TweakableParser: escape sequences in char literals are not implemented, sorry";
        return {TweakableState::Error, {}};
    }

    return {TweakableState::Success, value[1]};
}

std::pair<TweakableState, bool> TweakableParser<bool>::parse(Containers::StringView value) {
    using namespace Containers::Literals;

    if(value == "true"_s)
        return {TweakableState::Success, true};
    if(value == "false"_s)
        return {TweakableState::Success, false};

    Warning{} << "Utility::TweakableParser:" << value << "is not a boolean literal";
    return {TweakableState::Recompile, {}};
}

}}

