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

configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        "UTILITY_TEST_DIR": "$(execpath .)",
        "UTILITY_BINARY_TEST_DIR": "$(execpath .)",
    },
    deps = ["."],
    local = True,
)

# depends on containers too, oddly enough
cc_test(
    name = "UtilityAlgorithmsTest",
    srcs = ["AlgorithmsTest.cpp"],
    local_defines = ["CORRADE_GRACEFUL_ASSERT"],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/Utility:UtilityTestLib",
    ],
)

cc_test(
    name = "UtilityArgumentsTest",
    srcs = ["ArgumentsTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/Utility:UtilityTestLib",
    ],
    env = {
        "ARGUMENTSTEST_SIZE": "1337",
        "ARGUMENTSTEST_VERBOSE": "ON",
        "ARGUMENTSTEST_COLOR": "OFF",
        "ARGUMENTSTEST_UNICODE": "hýždě",
    },
)

cc_library(
    name = "AssertTestObjects",
    srcs = ["AssertTest.cpp"],
    testonly = True,
    deps = ["@corrade//:TestSuite"],
)


cc_test(
    name = "UtilityAssertTest",
    srcs = ["//src:dummy.cpp"],
    deps = [
        "@corrade//:TestSuite",
        ":AssertTestObjects",
    ],
)

cc_test(
    name = "UtilityAssertStandardTest",
    srcs = ["AssertTest.cpp"],
    deps = ["@corrade//:TestSuite"],
    local_defines = [
        "CORRADE_STANDARD_ASSERT",
    ],
)

cc_test(
    name = "UtilityAssertStandardDisabledTest",
    srcs = ["AssertDisabledTest.cpp"],
    deps = ["@corrade//:TestSuite"],
    local_defines = [
        "CORRADE_STANDARD_ASSERT",
        "NDEBUG",
    ],
)

[cc_test(
    name = "Utility%s" % t,
    srcs = ["%s.cpp" % t],
    deps = ["@corrade//:TestSuite"],
)
for t in [
    "AssertDisabledTest",
    "AssertGracefulTest",
    "EndiannessTest",
    "MurmurHash2Test",
    "HashDigestTest",
    "Sha1Test",
    "StlForwardArrayTest",
    "StlForwardStringTest",
    "StlForwardTupleTest",
    "StlForwardVectorTest",
    "SystemTest",
    "TweakableParserTest",
    # TODO: potentially selectable
    "StlMathTest",
]]

[cc_test(
    name = "Utility%s" % t,
    srcs = ["%s.cpp" % t],
    deps = ["@corrade//:TestSuite"],
    # TODO: selectable
    linkopts = ["-pthread"],
)
for t in [
    "DebugTest",
    "MacrosTest",
]]

[cc_test(
    name = "Utility%s" % t,
    srcs = ["%s.cpp" % t],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/Utility:UtilityTestLib",
        ":configure",
    ],
)
for t in [
    "FormatTest",
    "StringTest",
    "UnicodeTest",
]]

cc_test(
    name = "UtilityTypeTraitsTest",
    srcs = [
        "TypeTraitsTest.cpp",
        # This one should be clearly documented
        "//src/Corrade/Containers:LinkedList.h",
    ],
    deps = ["@corrade//:TestSuite"],
)

# These two fail miserably. The weird thing is how inplace array init
# works incorrectly

cc_test(
    name = "UtilityDirectoryTest",
    srcs = ["DirectoryTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        ":configure",
    ],
    includes = ["."],
    data = [
        "DirectoryTestFiles/dir/dummy",
        "DirectoryTestFiles/file",
        "DirectoryTestFilesUtf8/šňůra/dummy",
        "DirectoryTestFilesUtf8/hýždě",
    ],
)


cc_test(
    name = "UtilityConfigurationTest",
    srcs = ["ConfigurationTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        "//src/Corrade/Utility:UtilityTestLib",
        ":configure",
    ],
    data = [
        "ConfigurationTestFiles/bom.conf",
        "ConfigurationTestFiles/comments.conf",
        "ConfigurationTestFiles/comments-saved.conf",
        "ConfigurationTestFiles/eol-mixed.conf",
        "ConfigurationTestFiles/eol-unix.conf",
        "ConfigurationTestFiles/eol-windows.conf",
        "ConfigurationTestFiles/hierarchic.conf",
        "ConfigurationTestFiles/hierarchic-empty-group.conf",
        "ConfigurationTestFiles/hierarchic-empty-subgroup.conf",
        "ConfigurationTestFiles/hierarchic-missing-bracket.conf",
        "ConfigurationTestFiles/hierarchic-modified.conf",
        "ConfigurationTestFiles/hierarchic-shortcuts.conf",
        "ConfigurationTestFiles/hierarchic-shortcuts-modified.conf",
        "ConfigurationTestFiles/hýždě.conf",
        "ConfigurationTestFiles/missing-equals.conf",
        "ConfigurationTestFiles/missing-quote.conf",
        "ConfigurationTestFiles/missing-multiline-quote.conf",
        "ConfigurationTestFiles/multiLine.conf",
        "ConfigurationTestFiles/multiLine-crlf.conf",
        "ConfigurationTestFiles/multiLine-crlf-saved.conf",
        "ConfigurationTestFiles/multiLine-saved.conf",
        "ConfigurationTestFiles/parse.conf",
        "ConfigurationTestFiles/parse-modified.conf",
        "ConfigurationTestFiles/whitespaces.conf",
        "ConfigurationTestFiles/whitespaces-saved.conf",
    ],
)
