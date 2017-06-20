#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017 Vladimír Vondruš <mosra@centrum.cz>
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

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(UTILITY_TEST_DIR ".")
    set(UTILITY_BINARY_TEST_DIR "./write")
else()
    set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

corrade_add_test(UtilityArgumentsTest ArgumentsTest.cpp LIBRARIES CorradeUtilityTestLib)
set_tests_properties(UtilityArgumentsTest
    PROPERTIES ENVIRONMENT "ARGUMENTSTEST_SIZE=1337;ARGUMENTSTEST_VERBOSE=ON;ARGUMENTSTEST_COLOR=OFF;ARGUMENTSTEST_UNICODE=hýždě")
corrade_add_test(UtilityEndianTest EndianTest.cpp)
corrade_add_test(UtilityMurmurHash2Test MurmurHash2Test.cpp)
corrade_add_test(UtilityConfigurationTest ConfigurationTest.cpp
    LIBRARIES CorradeUtilityTestLib
    FILES
        ConfigurationTestFiles/comments.conf
        ConfigurationTestFiles/comments-saved.conf
        ConfigurationTestFiles/eol-mixed.conf
        ConfigurationTestFiles/eol-unix.conf
        ConfigurationTestFiles/eol-windows.conf
        ConfigurationTestFiles/hierarchic.conf
        ConfigurationTestFiles/hierarchic-modified.conf
        ConfigurationTestFiles/hýždě.conf
        ConfigurationTestFiles/invalid.conf
        ConfigurationTestFiles/multiLine.conf
        ConfigurationTestFiles/multiLine-crlf.conf
        ConfigurationTestFiles/multiLine-crlf-saved.conf
        ConfigurationTestFiles/multiLine-saved.conf
        ConfigurationTestFiles/parse.conf
        ConfigurationTestFiles/parse-modified.conf
        ConfigurationTestFiles/whitespaces.conf
        ConfigurationTestFiles/whitespaces-saved.conf)
target_include_directories(UtilityConfigurationTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
corrade_add_test(UtilityConfigurationValueTest ConfigurationValueTest.cpp)
corrade_add_test(UtilityDebugTest DebugTest.cpp)

# It should return with a non-zero exit code
corrade_add_test(UtilityFatalTest FatalTest.cpp)
set_tests_properties(UtilityFatalTest PROPERTIES WILL_FAIL ON)

set(UtilityDirectoryTest_SRCS DirectoryTest.cpp)
if(CORRADE_TARGET_IOS)
    set(UtilityDirectoryTest_RESOURCES
        DirectoryTestFiles)
    set_source_files_properties(DirectoryTestFiles PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND UtilityDirectoryTest_SRCS DirectoryTestFiles)
endif()
corrade_add_test(UtilityDirectoryTest ${UtilityDirectoryTest_SRCS}
    FILES
        DirectoryTestFiles/dir/dummy
        DirectoryTestFiles/file
        DirectoryTestFilesUtf8/šňůra/dummy
        DirectoryTestFilesUtf8/hýždě)
target_include_directories(UtilityDirectoryTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityHashDigestTest HashDigestTest.cpp)
corrade_add_test(UtilityMacrosTest MacrosTest.cpp)
corrade_add_test(UtilitySha1Test Sha1Test.cpp)
corrade_add_test(UtilityStringTest StringTest.cpp)
target_include_directories(UtilityStringTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
corrade_add_test(UtilitySystemTest SystemTest.cpp)
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
    LIBRARIES CorradeUtilityTestLib
    FILES
        ResourceTestFiles/compiled.cpp
        ResourceTestFiles/compiled-empty.cpp
        ResourceTestFiles/compiled-nothing.cpp
        ResourceTestFiles/compiled-unicode.cpp
        ResourceTestFiles/consequence.bin
        ResourceTestFiles/consequence2.txt
        ResourceTestFiles/empty.bin
        ResourceTestFiles/hýždě.bin
        ResourceTestFiles/hýždě.conf
        ResourceTestFiles/predisposition.bin
        ResourceTestFiles/predisposition2.txt
        ResourceTestFiles/resources.conf
        ResourceTestFiles/resources-empty-alias.conf
        ResourceTestFiles/resources-empty-file.conf
        ResourceTestFiles/resources-empty-filename.conf
        ResourceTestFiles/resources-empty-group.conf
        ResourceTestFiles/resources-no-group.conf
        ResourceTestFiles/resources-nonexistent.conf
        ResourceTestFiles/resources-nothing.conf
        ResourceTestFiles/resources-overriden.conf
        ResourceTestFiles/resources-overriden-different.conf
        ResourceTestFiles/resources-overriden-none.conf
        ResourceTestFiles/resources-overriden-nonexistent-file.conf)
target_include_directories(UtilityResourceTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
target_link_libraries(ResourceTestDataLib CorradeUtility)
corrade_add_test(UtilityResourceStaticTest ResourceStaticTest.cpp
    LIBRARIES ResourceTestDataLib
    FILES
        ResourceTestFiles/consequence.bin
        ResourceTestFiles/predisposition.bin)
target_include_directories(UtilityResourceStaticTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(
    UtilityArgumentsTest
    UtilityEndianTest
    UtilityMurmurHash2Test
    UtilityConfigurationTest
    UtilityConfigurationValueTest
    UtilityDebugTest
    UtilityFatalTest
    UtilityDirectoryTest
    UtilityHashDigestTest
    UtilityMacrosTest
    UtilityResourceTest
    UtilityResourceStaticTest
    UtilitySha1Test
    UtilityStringTest
    UtilitySystemTest
    UtilityTypeTraitsTest
    UtilityUnicodeTest

    ResourceTestDataLib
    ResourceTestData-dependencies
    ResourceTestEmptyFileData-dependencies
    ResourceTestNothingData-dependencies
    PROPERTIES FOLDER "Corrade/Utility/Test")
