#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021
#             Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob([
        "**/*.h",
        "**/*.hpp",
    ]),
)

cc_library(
    name = "utility",
    srcs = [
        # UTILITY SRCS
        "Debug.cpp",
        "Directory.cpp",
        "Configuration.cpp",
        "ConfigurationValue.cpp",
        "MurmurHash2.cpp",
        "Sha1.cpp",
        "System.cpp",
        # GRACEFUL ASSERT
        "Algorithms.cpp",
        "Arguments.cpp",
        "ConfigurationGroup.cpp",
        "Format.cpp",
        "Resource.cpp",
        "String.cpp",
        "Unicode.cpp",
        # TODO: this is selectable
        "FileWatcher.cpp",
        "Tweakable.cpp",
        "TweakableParser.cpp",
    ],
    copts = ["-Isrc"],
    deps = [
        "//src/Corrade/Containers:__containers_internal",
    ],
)

cc_binary(
    name = "corrade-rc",
    srcs = [
        "Arguments.cpp",
        "Debug.cpp",
        "Directory.cpp",
        "Configuration.cpp",
        "ConfigurationGroup.cpp",
        "Format.cpp",
        "Resource.cpp",
        "String.cpp",
        "rc.cpp",
        "FileWatcher.cpp",
        "Tweakable.cpp",
        "TweakableParser.cpp",
    ],
    #TODO: linux
    linkopts = [
        "-ldl",
    ],
    copts = ["-Isrc"],
    deps = [
        "//src/Corrade:config",
        "//src/Corrade/Utility:headers",
        "//src/Corrade/Containers:headers",
    ],
)

