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
load(
    "@corrade//:index.bzl",
    "corrade_configure_header",
    "corrade_static_plugin",
    "corrade_dynamic_plugin",
)

corrade_configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        "PLUGINS_DIR": "plugins",
        "DOG_PLUGIN_FILENAME": "$(rootpath :Dog)",
        "DOGGO_PLUGIN_FILENAME": "$(rootpath :Doggo)",
        "PITBULL_PLUGIN_FILENAME": "$(rootpath :PitBull)",
    },
    deps = [
        ":Dog",
        ":Doggo",
        ":PitBull",
    ],
    local = True,
)


corrade_static_plugin(
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

corrade_static_plugin(
    name = "DisabledMetadataStatic",
    srcs = [
        "disabled-metadata/DisabledMetadataStatic.cpp",
        "//src/Corrade/PluginManager:PluginManagerTestLib",
    ],
    hdrs = [
        "AbstractDisabledMetadata.h",
    ],
)

corrade_static_plugin(
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

corrade_static_plugin(
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

corrade_static_plugin(
    name = "InitFiniStatic",
    metadata_file = "dummy.conf",
    srcs = [
        "init-fini/InitFiniStatic.cpp",
    ],
    hdrs = [
        "init-fini/InitFiniStatic.h",
        "init-fini/InitFini.h",
    ],
    deps = [
        ":configure",
    ],
)

[corrade_dynamic_plugin(
    name = "%s" % dog,
    metadata_file = "animals/%s.conf" % dog,
    subdir = "plugins/animals",
    srcs = [
        "animals/%s.cpp" % dog,
        "animals/Dog.h",
        "AbstractAnimal.h",
    ],
    deps = ["//src/Corrade/PluginManager:PluginManagerTestLib"],
)
for dog in [
    "Dog",
    "Bulldog",
    "PitBull",
]]

corrade_dynamic_plugin(
    name = "Snail",
    metadata_file = "animals/Snail.conf",
    subdir = "plugins/animals",
    srcs = [
        "animals/Snail.cpp",
        "AbstractAnimal.h",
    ],
    deps = ["//src/Corrade/PluginManager:PluginManagerTestLib"],
)

corrade_dynamic_plugin(
    name = "CustomSuffix",
    metadata_file = "custom-suffix/CustomSuffix.modconf",
    subdir = "plugins/custom-suffix",
    srcs = [
        "custom-suffix/CustomSuffix.cpp",
        "AbstractCustomSuffix.h",
    ],
    override_suffix = "mod",
)

corrade_dynamic_plugin(
    name = "Deletable",
    metadata_file = "deletable/Deletable.conf",
    subdir = "plugins/deletable",
    srcs = [
        "deletable/Deletable.cpp",
        "AbstractDeletable.h",
    ],
)

corrade_dynamic_plugin(
    name = "DisabledMetadata",
    subdir = "plugins/disabled-metadata",
    srcs = [
        "disabled-metadata/DisabledMetadata.cpp",
        "AbstractDisabledMetadata.h",
    ],
)

corrade_dynamic_plugin(
    name = "Doggo",
    metadata_file = "doggo/Doggo.conf",
    subdir = "plugins/doggo",
    srcs = [
        "doggo/Doggo.cpp",
        "AbstractAnimal.h",
    ],
    deps = ["//src/Corrade/PluginManager:PluginManagerTestLib"],
)

[corrade_dynamic_plugin(
    name = "%s" % food,
    metadata_file = "food/%s.conf" % food,
    subdir = "plugins/food",
    srcs = [
        "food/HotDog.cpp",
        "animals/Dog.h",
        "AbstractAnimal.h",
        "AbstractFood.h",
    ],
    deps = ["//src/Corrade/PluginManager:PluginManagerTestLib"],
)
for food in [
    "HotDog",
    "HotDogWithSnail",
]]

[corrade_dynamic_plugin(
    name = "%s" % food,
    metadata_file = "food/%s.conf" % food,
    subdir = "plugins/food",
    srcs = [
        "food/%s.cpp" % food,
        "AbstractFood.h",
    ],
    deps = ["//src/Corrade/PluginManager:PluginManagerTestLib"],
)
for food in [
    "OldBread",
    "RottenTomato",
]]

corrade_dynamic_plugin(
    name = "InitFiniDynamic",
    metadata_file = "init-fini/InitFiniDynamic.conf",
    subdir = "plugins/init-fini",
    srcs = [
        "init-fini/InitFiniDynamic.cpp",
        "init-fini/InitFini.h",
        "init-fini/InitFiniStatic.h",
    ],
    deps = [
        "@corrade//:PluginManager",
        ":configure",
    ]
)

corrade_dynamic_plugin(
    name = "WrongMetadata",
    metadata_file = "wrong-metadata/WrongMetadata.conf",
    subdir = "plugins/wrong-metadata",
    srcs = [
        "wrong-metadata/WrongMetadata.cpp",
        "wrong-metadata/WrongMetadata.h",
    ],
    deps = [
        ":configure",
    ]
)

[corrade_dynamic_plugin(
    name = "%s" % wrong,
    metadata_file = "dummy.conf",
    subdir = "plugins/wrong",
    srcs = [
        "wrong/%s.cpp" % wrong,
    ],
)
for wrong in [
    "NoPluginFinalizer",
    "NoPluginInitializer",
    "NoPluginInstancer",
    "NoPluginInterface",
    "NoPluginVersion",
]]

corrade_dynamic_plugin(
    name = "UnresolvedReference",
    metadata_file = "dummy.conf",
    subdir = "plugins/wrong",
    srcs = [
        "wrong/UnresolvedReference.cpp",
    ],
    deps = ["//src/Corrade/Interconnect:headers"],
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
        ":Canary",
        ":CustomSuffixStatic",
        ":DisabledMetadataStatic",
        ":configure",
    ],
    data = [
        ":Dog",
        ":Bulldog",
        ":PitBull",
        ":Snail",
        ":CustomSuffix",
        ":Deletable",
        ":DisabledMetadata",
        ":Doggo",
        ":HotDog",
        ":HotDogWithSnail",
        ":OldBread",
        ":RottenTomato",
        ":InitFiniDynamic",
        ":WrongMetadata",
        ":NoPluginFinalizer",
        ":NoPluginInitializer",
        ":NoPluginInstancer",
        ":NoPluginInterface",
        ":NoPluginVersion",
        ":UnresolvedReference",
    ]
)

cc_test(
    name = "PluginManagerManagerInitFiniTest",
    srcs = ["ManagerInitFiniTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        "@corrade//:PluginManager",
        ":InitFiniStatic",
    ],
    data = [
        ":InitFiniDynamic",
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
        ":configure",
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
        ":configure",
    ],
)

# this one is tricky because bazel doesn't deal in shared libraries easily
# NOTE: cc_binary with linkshared=True is a shared object but with one caveat:
# it does not link automatically, have to include it manually in srcs
cc_library(
    name = "PluginManagerGlobalStateAcrossLibrariesLibrary",
    srcs = [
        "GlobalStateAcrossLibrariesLibrary.cpp",
        "GlobalStateAcrossLibrariesLibrary.h",
        "AbstractAnimal.h",
        ":configure",
        ":Canary",
    ],
    deps = [
        "//src/Corrade/PluginManager:PluginManagerTestLib",
        ":configure",
    ],
)

# TODO: platform compatibility
#cc_test(
#    name = "PluginManagerGlobalStateAcrossLi___Test",
#    srcs = [
#        "GlobalStateAcrossLibrariesTest.cpp",
#        "GlobalStateAcrossLibrariesLibrary.h",
#        "AbstractAnimal.cpp",
#        "AbstractAnimal.h",
#    ],
#    deps = [
#        ":PluginManagerGlobalStateAcrossLibrariesLibrary",
#        "@corrade//:TestSuite",
#        "//src/Corrade/PluginManager:PluginManagerTestLib",
#        ":configure",
#    ],
#)
