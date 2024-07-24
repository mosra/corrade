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
load("@corrade//:index.bzl", "corrade_configure_header")

exports_files([
    "Corrade.h",
    "Corrade.cpp",
])

corrade_configure_header(
    name = "version",
    src = "version.h.cmake",
    output = "version.h",
    defines = {
        # just some sample data i suppose
        "CORRADE_VERSION_YEAR": "2020",
        "CORRADE_VERSION_MONTH": "6",
        "CORRADE_VERSION_COMMIT": " 252",
        "CORRADE_VERSION_HASH": "b329d056",
        "CORRADE_VERSION_STRING": "v2020.06-252-ga1038a9b",
    },
    visibility = ["//:__pkg__"]
)

corrade_configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        "CORRADE_BUILD_DEPRECATED": "",
        #"CORRADE_BUILD_STATIC": "",
        #"CORRADE_BUILD_STATIC_UNIQUE_GLOBALS": "",
        "CORRADE_BUILD_MULTITHREADED": "",
        "CORRADE_TARGET_UNIX": "",
        #"CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT": "",
    },
    visibility = ["//:__pkg__"]
)
