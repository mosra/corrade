#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

corrade_add_test(UtilityArgumentsTest ArgumentsTest.cpp LIBRARIES CorradeUtilityTestLib)
corrade_add_test(UtilityDirectoryTest DirectoryTest.cpp)
corrade_add_test(UtilityEndianTest EndianTest.cpp)
corrade_add_test(UtilityMurmurHash2Test MurmurHash2Test.cpp)
corrade_add_test(UtilityConfigurationTest ConfigurationTest.cpp LIBRARIES CorradeUtilityTestLib)
corrade_add_test(UtilityDebugTest DebugTest.cpp)
corrade_add_test(UtilityHashDigestTest HashDigestTest.cpp)
corrade_add_test(UtilitySha1Test Sha1Test.cpp)
corrade_add_test(UtilityStringTest StringTest.cpp)
corrade_add_test(UtilityTypeTraitsTest TypeTraitsTest.cpp)
corrade_add_test(UtilityUnicodeTest UnicodeTest.cpp LIBRARIES CorradeUtilityTestLib)

# Compiled-in resource test
corrade_add_resource(ResourceTestData ResourceTestFiles/resources.conf)
corrade_add_resource(ResourceTestEmptyFileData ResourceTestFiles/resources-empty-file.conf)
corrade_add_resource(ResourceTestNothingData ResourceTestFiles/resources-nothing.conf)
corrade_add_test(UtilityResourceTest
    ResourceTest.cpp
    ${ResourceTestData}
    ${ResourceTestEmptyFileData}
    ${ResourceTestNothingData}
    LIBRARIES CorradeUtilityTestLib)

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
corrade_add_test(UtilityResourceStaticTest ResourceStaticTest.cpp LIBRARIES ResourceTestDataLib)
