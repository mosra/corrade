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

static_plugin(
    name = "Canary",
    metadata_file = "animals/Canary.conf",
    srcs = [
        "animals/Canary.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "animals/Canary.h",
        "AbstractAnimal.h",
    ],
    copts = ["-fPIC"],
)

static_plugin(
    name = "Dird",
    metadata_file = "animals/Dird.conf",
    srcs = [
        "animals/Dird.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "AbstractAnimal.h",
    ],
)

static_plugin(
    name = "InitFiniStatic",
    metadata_file = "dummy.conf",
    srcs = [
        "init-fini/InitFiniStatic.cpp",
    ],
    hdrs = [
        "init-fini/InitFiniStatic.h",
        "init-fini/InitFini.h",
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
    ],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
        ":Canary",
        ":CustomSuffixStatic",
        ":DisabledMetadataStatic",
    ],
)

cc_test(
    name = "PluginManagerInitFiniTest",
    srcs = ["ManagerInitFiniTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        "@corrade//:PluginManager",
        ":InitFiniStatic",
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
        ":Canary",
        ":Dird",
    ],
)

cc_test(
    name = "PluginManagerAbstractPluginTest",
    srcs = [
        "AbstractPluginTest.cpp",
        "AbstractAnimal.cpp",
        "AbstractAnimal.h",
    ],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
        ":Canary",
    ],
)

# this one is tricky because bazel doesn't deal in shared libraries easily
# NOTE: cc_binary with linkshared=True is a shared object but with one caveat:
# it does not link automatically, have to include it manually in srcs
cc_binary(
    name = "PluginManagerGlobalStateAcrossLibrariesLibrary",
    srcs = [
        "GlobalStateAcrossLibrariesLibrary.cpp",
        "GlobalStateAcrossLibrariesLibrary.h",
        "AbstractAnimal.cpp",
        "AbstractAnimal.h",
        ":Canary",
    ],
    deps = [
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    linkshared = True,
)

# TODO: platform compatibility
cc_test(
    name = "PluginManagerGlobalStateAcrossLi___Test",
    srcs = [
        "GlobalStateAcrossLibrariesTest.cpp",
        "GlobalStateAcrossLibrariesLibrary.h",
        "AbstractAnimal.cpp",
        "AbstractAnimal.h",
        ":PluginManagerGlobalStateAcrossLibrariesLibrary",
    ],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
)
