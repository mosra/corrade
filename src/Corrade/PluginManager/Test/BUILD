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
load("@corrade//bazel:configure_header.bzl", "configure_header")
load("@corrade//bazel:static_plugin.bzl", "static_plugin")
load("@corrade//bazel:dynamic_plugin.bzl", "dynamic_plugin")

package(default_visibility = ["//src/Corrade/PluginManager/Test:__subpackages__"])
exports_files([
    "AbstractAnimal.h",
])

configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        # TODO: fix
        #"PLUGINS_DIR": "bazel-out/k8-fastbuild/bin/src/Corrade/PluginManager/Test",
        "PLUGINS_DIR": "",
        #"DOG_PLUGIN_FILENAME": "$(rootpath //src/Corrade/PluginManager/Test/animals:Dog)",
        #"DOGGO_PLUGIN_FILENAME": "$(rootpath //src/Corrade/PluginManager/Test/doggo:Doggo)",
        #"PITBULL_PLUGIN_FILENAME": "$(rootpath //src/Corrade/PluginManager/Test/animals:PitBull)",
    },
    deps = [
        "//src/Corrade/PluginManager/Test/animals:Dog",
        "//src/Corrade/PluginManager/Test/animals:PitBull",
        "//src/Corrade/PluginManager/Test/doggo:Doggo",
    ],
    local = True,
)

static_plugin(
    name = "CustomSuffixStatic",
    metadata_file = "custom-suffix/CustomSuffixStatic.modconf",
    srcs = [
        "custom-suffix/CustomSuffixStatic.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "AbstractCustomSuffix.h",
    ],
)

static_plugin(
    name = "DisabledMetadataStatic",
    srcs = [
        "disabled-metadata/DisabledMetadataStatic.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "AbstractDisabledMetadata.h",
    ],
)

cc_test(
    name = "PluginManagerManagerTest",
    srcs = [
        "ManagerTest.cpp",
        "AbstractAnimal.cpp",
        "AbstractCustomSuffix.cpp",
        "AbstractDeletable.cpp",
        "AbstractDisabledMetadata.cpp",
        "AbstractFood.cpp",
        "AbstractAnimal.h",
        "AbstractCustomSuffix.h",
        "AbstractDeletable.h",
        "AbstractDisabledMetadata.h",
        "AbstractFood.h",
        "wrong-metadata/WrongMetadata.h",
    ],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
        "//src/Corrade/PluginManager/Test/animals:Canary",
        ":CustomSuffixStatic",
        ":DisabledMetadataStatic",
        ":configure",
    ],
    data = [
        "//src/Corrade/PluginManager/Test/animals:Snail",
        "//src/Corrade/PluginManager/Test/animals:Dog",
        "//src/Corrade/PluginManager/Test/animals:Bulldog",
        "//src/Corrade/PluginManager/Test/animals:PitBull",
        "//src/Corrade/PluginManager/Test/doggo:Doggo",
    ],
)

cc_test(
    name = "PluginManagerImportStaticTest",
    srcs = [
        "ImportStaticTest.cpp",
        "AbstractAnimal.cpp",
        "AbstractAnimal.h",
    ],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
        "//src/Corrade/PluginManager/Test/animals:Canary",
        "//src/Corrade/PluginManager/Test/animals:Dird",
        ":configure",
    ],
)
