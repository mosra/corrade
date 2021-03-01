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

cc_library(
    name = "config",
    hdrs = [
        "configure.h",
        "version.h",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "common_utility_containers",
    hdrs = [
        "//src/Corrade/Containers:Array.h",
        "//src/Corrade/Containers:ArrayView.h",
        "//src/Corrade/Containers:ArrayTuple.h",
        "//src/Corrade/Containers:EnumSet.h",
        "//src/Corrade/Containers:EnumSet.hpp",
        "//src/Corrade/Containers:GrowableArray.h",
        "//src/Corrade/Containers:constructHelpers.h",
        "//src/Corrade/Containers:Containers.h",
        "//src/Corrade/Containers:StaticArray.h",
        "//src/Corrade/Containers:String.h",
        "//src/Corrade/Containers:StringStl.h",
        "//src/Corrade/Containers:StringView.h",
        "//src/Corrade/Containers:Tags.h",
        "//src/Corrade/Utility:Assert.h",
        "//src/Corrade/Utility:Debug.h",
        "//src/Corrade/Utility:DebugStl.h",
        "//src/Corrade/Utility:Macros.h",
        "//src/Corrade/Utility:StlForwardString.h",
        "//src/Corrade/Utility:TypeTraits.h",
        "//src/Corrade/Utility:Utility.h",
        "//src/Corrade/Utility:visibility.h",
        "//src/Corrade/Utility:VisibilityMacros.h",
    ],
    srcs = [
        "//src/Corrade/Containers:ArrayTuple.cpp",
        "//src/Corrade/Containers:String.cpp",
        "//src/Corrade/Containers:StringView.cpp",
        "//src/Corrade/Utility:Debug.cpp",
    ],
    visibility = [
        "//src/Corrade/Containers:__pkg__",
        "//src/Corrade/Utility:__pkg__",
    ],
    copts = ["-Isrc"],
    # TODO: appropriate flags per compiler.
    # Well, actually, just need to run this analysis once, right
    linkopts = ["--no-undefined"],
    deps = [
        ":config",
    ],
)
