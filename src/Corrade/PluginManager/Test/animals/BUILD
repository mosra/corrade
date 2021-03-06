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
load("@corrade//bazel:static_plugin.bzl", "static_plugin")
load("@corrade//bazel:dynamic_plugin.bzl", "dynamic_plugin")

package(default_visibility = ["//src/Corrade/PluginManager/Test:__subpackages__"])

static_plugin(
    name = "Canary",
    metadata_file = "Canary.conf",
    srcs = [
        "Canary.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "Canary.h",
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
    ],
    copts = ["-fPIC"],
)

static_plugin(
    name = "Dird",
    metadata_file = "Dird.conf",
    srcs = [
        "Dird.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
    ],
)

dynamic_plugin(
    name = "Snail",
    metadata_file = "Snail.conf",
    srcs = ["Snail.cpp"],
    hdrs = [
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
    ],
)

dynamic_plugin(
    name = "Dog",
    metadata_file = "Dog.conf",
    srcs = ["Dog.cpp"],
    hdrs = [
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
        "Dog.h",
    ],
)

dynamic_plugin(
    name = "Bulldog",
    metadata_file = "Bulldog.conf",
    srcs = ["Bulldog.cpp"],
    hdrs = [
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
        "Dog.h",
    ],
)

dynamic_plugin(
    name = "PitBull",
    metadata_file = "PitBull.conf",
    srcs = ["PitBull.cpp"],
    hdrs = [
        "//src/Corrade/PluginManager/Test:AbstractAnimal.h",
        "Dog.h",
    ],
)
