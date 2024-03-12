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

load("@corrade//:index.bzl", "corrade_configure_header")

corrade_configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    # TODO: this is selectable
    defines = {
        "CMAKE_SHARED_MODULE_SUFFIX": ".so",
    },
)

cc_library(
    name = "headers",
    hdrs = [
        "AbstractPlugin.h",
        "AbstractManager.h",
        "AbstractManagingPlugin.h",
        "Manager.h",
        "PluginManager.h",
        "PluginMetadata.h",
        "visibility.h",
    ],
    deps = [
        "//src/Corrade/Utility:headers",
        ":configure",
    ],
)

cc_library(
    name = "PluginManager",
    srcs = [
        "AbstractPlugin.cpp",
        "PluginMetadata.cpp",
        # GracefulAssertSrcs
        "AbstractManager.cpp",
        "//src/Corrade/Containers:Reference.h",
    ],
    deps = [
        "@corrade//:Utility",
        ":headers",
    ],
)

cc_library(
    name = "PluginManagerTestLib",
    srcs = ["AbstractManager.cpp"],
    deps = [":PluginManager"],
    defines = ["CORRADE_GRACEFUL_ASSERT"],
    visibility = ["//src/Corrade:__subpackages__"],
)
