/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019, 2020 Jonathan Hale <squareys@googlemail.com>

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

#define _CORRADE_NO_DEPRECATED_DIRECTORY /* So it doesn't yell here */

#include "Directory.h"

#include <string>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/PairStl.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/System.h"

namespace Corrade { namespace Utility { namespace Directory {

std::string fromNativeSeparators(const std::string& path) {
    return Path::fromNativeSeparators(path);
}

std::string toNativeSeparators(const std::string& path) {
    return Path::toNativeSeparators(path);
}

std::string path(const std::string& filename) {
    return Path::split(filename).first();
}

std::string filename(const std::string& filename) {
    return Path::split(filename).second();
}

std::pair<std::string, std::string> splitExtension(const std::string& filename) {
    return std::pair<Containers::StringView, Containers::StringView>(Path::splitExtension(filename));
}

std::string join(const std::string& path, const std::string& filename) {
    return Path::join(path, filename);
}

std::string join(const std::initializer_list<std::string> paths) {
    const auto pathsView = Containers::arrayView(paths);
    Containers::Array<Containers::StringView> pathViews{paths.size()};
    for(std::size_t i = 0; i != pathsView.size(); ++i)
        pathViews[i] = pathsView[i];
    return Path::join(pathViews);
}

bool mkpath(const std::string& path) {
    return Path::make(path);
}

bool rm(const std::string& path) {
    return Path::remove(path);
}

bool move(const std::string& from, const std::string& to) {
    return Path::move(from, to);
}

bool exists(const std::string& filename) {
    return Path::exists(filename);
}

Containers::Optional<std::size_t> fileSize(const std::string& filename) {
    return Path::size(filename);
}

bool isDirectory(const std::string& path) {
    return Path::isDirectory(path);
}

bool isSandboxed() {
    return System::isSandboxed();
}

std::string current() {
    Containers::Optional<Containers::String> out = Path::currentDirectory();
    return out ? std::string{*out} : std::string{};
}

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
std::string libraryLocation(const void* address) {
    Containers::Optional<Containers::String> out = Path::libraryLocation(address);
    return out ? std::string{*out} : std::string{};
}

#ifndef DOXYGEN_GENERATING_OUTPUT
std::string libraryLocation(Path::Implementation::FunctionPointer address) {
    CORRADE_IGNORE_DEPRECATED_PUSH
    return libraryLocation(address.address);
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif
#endif

std::string executableLocation() {
    Containers::Optional<Containers::String> out = Path::executableLocation();
    return out ? std::string{*out} : std::string{};
}

std::string home() {
    Containers::Optional<Containers::String> out = Path::homeDirectory();
    return out ? std::string{*out} : std::string{};
}

std::string configurationDir(const std::string& applicationName) {
    Containers::Optional<Containers::String> out = Path::configurationDirectory(applicationName);
    return out ? std::string{*out} : std::string{};
}

std::string tmp() {
    Containers::Optional<Containers::String> out = Path::temporaryDirectory();
    return out ? std::string{*out} : std::string{};
}

std::vector<std::string> list(const std::string& path, Path::ListFlags flags) {
    Containers::Optional<Containers::Array<Containers::String>> out = Path::list(path, flags);
    return out ? std::vector<std::string>{out->begin(), out->end()} : std::vector<std::string>{};
}

Containers::Array<char> read(const std::string& filename) {
    Containers::Optional<Containers::Array<char>> out = Path::read(filename);
    return out ? *std::move(out) : nullptr;
}

std::string readString(const std::string& filename) {
    Containers::Optional<Containers::String> out = Path::readString(filename);
    return out ? std::string{*out} : std::string{};
}

bool write(const std::string& filename, const Containers::ArrayView<const void> data) {
    return Path::write(filename, data);
}

bool writeString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return Path::write(filename, Containers::StringView{data});
}

bool append(const std::string& filename, const Containers::ArrayView<const void> data) {
    return Path::append(filename, data);
}

bool appendString(const std::string& filename, const std::string& data) {
    static_assert(sizeof(std::string::value_type) == 1, "std::string doesn't have 8-bit characters");
    return Path::append(filename, Containers::StringView{data});
}

bool copy(const std::string& from, const std::string& to) {
    return Path::copy(from, to);
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
Containers::Array<char, Path::MapDeleter> map(const std::string& filename) {
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> out = Path::map(filename);
    return out ? *std::move(out) : nullptr;
}

Containers::Array<const char, Path::MapDeleter> mapRead(const std::string& filename) {
    Containers::Optional<Containers::Array<const char, Path::MapDeleter>> out = Path::mapRead(filename);
    return out ? *std::move(out) : nullptr;
}

Containers::Array<char, Path::MapDeleter> mapWrite(const std::string& filename, const std::size_t size) {
    Containers::Optional<Containers::Array<char, Path::MapDeleter>> out = Path::mapWrite(filename, size);
    return out ? *std::move(out) : nullptr;
}

Containers::Array<char, Path::MapDeleter> map(const std::string& filename, const std::size_t size) {
    CORRADE_IGNORE_DEPRECATED_PUSH
    return mapWrite(filename, size);
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif

}}}
