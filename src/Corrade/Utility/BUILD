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
    name = "Utility",
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
        "Implementation/Resource.h",
        "String.cpp",
        "Unicode.cpp",
        "//src/Corrade/Containers:ArrayTuple.cpp",
        "//src/Corrade/Containers:String.cpp",
        "//src/Corrade/Containers:StringView.cpp",
        # TODO: this is selectable
        "FileWatcher.cpp",
        "Tweakable.cpp",
        "Implementation/tweakable.h",
        "TweakableParser.cpp",
        # prereqs from containers only included in .cpp
        "//src/Corrade/Containers:ArrayTuple.h",
        "//src/Corrade/Containers:EnumSet.hpp",
        "//src/Corrade/Containers:GrowableArray.h",
        "//src/Corrade/Containers:StaticArray.h",
        "//src/Corrade/Containers:ScopeGuard.h",
        "//src/Corrade/Containers:Implementation/RawForwardList.h",
    ],
    hdrs = [
        "Algorithms.h",
        "Arguments.h",
        "AbstractHash.h",
        "Assert.h",
        "Configuration.h",
        "ConfigurationGroup.h",
        "ConfigurationValue.h",
        "Debug.h",
        "DebugStl.h",
        "Directory.h",
        "Endianness.h",
        "EndiannessBatch.h",
        "Format.h",
        "FormatStl.h",
        "Macros.h",
        "MurmurHash2.h",
        "Resource.h",
        "Sha1.h",
        "String.h",
        "StlForwardArray.h",
        "StlForwardString.h",
        "StlForwardTuple.h",
        "StlForwardVector.h",
        "StlMath.h",
        "System.h",
        "TypeTraits.h",
        "Unicode.h",
        "utilities.h",
        "Utility.h",
        "visibility.h",
        "VisibilityMacros.h",
        # TODO: this is selectable
        "FileWatcher.h",
        "Tweakable.h",
        "TweakableParser.h",
        # prereqs from containers included in .cpp or .h
        "//src/Corrade/Containers:Array.h",
        "//src/Corrade/Containers:ArrayView.h",
        "//src/Corrade/Containers:constructHelpers.h",
        "//src/Corrade/Containers:Containers.h",
        "//src/Corrade/Containers:EnumSet.h",
        "//src/Corrade/Containers:Optional.h",
        "//src/Corrade/Containers:Pointer.h",
        "//src/Corrade/Containers:sequenceHelpers.h",
        "//src/Corrade/Containers:StridedArrayView.h",
        "//src/Corrade/Containers:String.h",
        "//src/Corrade/Containers:StringStl.h",
        "//src/Corrade/Containers:StringView.h",
        "//src/Corrade/Containers:Tags.h",
    ],
    linkstatic = True,
    linkopts = [
        # TODO: this is selectable
        "-ldl",
    ],
    local_defines = [
        "CORRADE_BUILD_STATIC",
    ],
    deps = [
        "@corrade//:Main",
    ],
)

cc_library(
    name = "UtilityTestLib",
    srcs = [
        # GRACEFUL ASSERT
        "Algorithms.cpp",
        "Arguments.cpp",
        "ConfigurationGroup.cpp",
        "Format.cpp",
        "Resource.cpp",
        "Implementation/Resource.h",
        "String.cpp",
        "Unicode.cpp",
        "//src/Corrade/Containers:ArrayTuple.cpp",
        "//src/Corrade/Containers:String.cpp",
        "//src/Corrade/Containers:StringView.cpp",
    ],
    hdrs = [
        "//src/Corrade/Containers:ArrayViewStl.h",
    ],
    # probably don't need this one?
    linkstatic = True,
    deps = [":Utility"],
    local_defines = [
        "CORRADE_BUILD_STATIC",
        "CORRADE_GRACEFUL_ASSERT",
    ],
    testonly = True,
)


cc_binary(
    name = "corrade-rc",
    srcs = ["rc.cpp"],
    linkstatic = True,
    local_defines = [
        "CORRADE_BUILD_STATIC",
    ],
    deps = ["@corrade//:Utility"],
)

