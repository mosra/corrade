#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "Corrade/Utility/Test")

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(CONTAINERS_STRING_TEST_DIR ".")
    set(UTILITY_TEST_DIR ".")
    set(UTILITY_BINARY_TEST_DIR "write")
else()
    set(CONTAINERS_STRING_TEST_DIR ${PROJECT_SOURCE_DIR}/src/Corrade/Containers/Test/StringTestFiles)
    set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# In all following corrade_add_test() macros, if a test wants to use
# CorradeUtilityTestLib, it has to link to CorradeTestSuiteTestLib instead.
# Otherwise the implicitly linked CorradeTestSuite would drag in CorradeUtility
# in addition to CorradeUtilityTestLib, leading to ODR violations and making
# ASan builds fail.

corrade_add_test(UtilityAlgorithmsTest AlgorithmsTest.cpp LIBRARIES CorradeTestSuiteTestLib)
target_compile_definitions(UtilityAlgorithmsTest PRIVATE "CORRADE_GRACEFUL_ASSERT")
# In Emscripten 3.1.27, the stack size was reduced from 5 MB (!) to 64 kB:
#   https://github.com/emscripten-core/emscripten/pull/18191
# One benchmark in this test uses 64 kB of stack space (two arrays of 64*64*2
# ints), need to increase it a bit to not make it crash.
if(CORRADE_TARGET_EMSCRIPTEN AND NOT EMSCRIPTEN_VERSION VERSION_LESS 3.1.27)
    set_property(TARGET UtilityAlgorithmsTest APPEND_STRING PROPERTY LINK_FLAGS " -s STACK_SIZE=128kB")
endif()

corrade_add_test(UtilityBitAlgorithmsTest BitAlgorithmsTest.cpp LIBRARIES CorradeTestSuiteTestLib)

corrade_add_test(UtilityArgumentsTest ArgumentsTest.cpp LIBRARIES CorradeTestSuiteTestLib)
set_tests_properties(UtilityArgumentsTest
    PROPERTIES ENVIRONMENT "ARGUMENTSTEST_SIZE=1337;ARGUMENTSTEST_VERBOSE=ON;ARGUMENTSTEST_COLOR=OFF;ARGUMENTSTEST_UNICODE=hýždě")

add_library(AssertTestObjects OBJECT AssertTest.cpp)
add_library(DebugAssertTestObjects OBJECT AssertTest.cpp)
target_include_directories(AssertTestObjects PRIVATE $<TARGET_PROPERTY:CorradeUtility,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(DebugAssertTestObjects PRIVATE $<TARGET_PROPERTY:CorradeUtility,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(DebugAssertTestObjects PRIVATE "TEST_DEBUG_ASSERT")
corrade_add_test(UtilityAssertTest
    $<TARGET_OBJECTS:AssertTestObjects>
    ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP}) # XCode workaround
corrade_add_test(UtilityDebugAssertTest
    $<TARGET_OBJECTS:DebugAssertTestObjects>
    ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP}) # XCode workaround
# WILL_FAIL doesn't work for abort() on desktop, test this only on embedded
# then. Oh well. Also the tests could be just one executable added multiple
# times with different arguments, but corrade_add_test() doesn't support that,
# so I'm doing it at least via an OBJECTS library.
if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    corrade_add_test(UtilityAssertTestFailAssert
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert true)
    corrade_add_test(UtilityDebugAssertTestFailAssert
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert true)
    set_tests_properties(
        UtilityAssertTestFailAssert
        UtilityDebugAssertTestFailAssert
        PROPERTIES PASS_REGULAR_EXPRESSION "A should be zero")
    corrade_add_test(UtilityAssertTestFailConstexprAss___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-constexpr-assert true)
    corrade_add_test(UtilityDebugAssertTestFailConsAss___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-constexpr-assert true)
    set_tests_properties(
        UtilityAssertTestFailConstexprAss___
        UtilityDebugAssertTestFailConsAss___
        PROPERTIES PASS_REGULAR_EXPRESSION "b can't be zero")
    corrade_add_test(UtilityAssertTestFailInternalAssert
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert true)
    corrade_add_test(UtilityDebugAssertTestFailInteAss___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert true)
    set_tests_properties(
        UtilityAssertTestFailInternalAssert
        UtilityDebugAssertTestFailInteAss___
        PROPERTIES PASS_REGULAR_EXPRESSION "Assertion b && !_failInternalAssert failed at ")
    corrade_add_test(UtilityAssertTestFailInternalConA___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-constexpr-assert true)
    corrade_add_test(UtilityDebugAssertTestFailIntConA___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-constexpr-assert true)
    set_tests_properties(
        UtilityAssertTestFailInternalConA___
        UtilityDebugAssertTestFailIntConA___
        PROPERTIES PASS_REGULAR_EXPRESSION "Assertion b failed at ")
    corrade_add_test(UtilityAssertTestFailAssertOutput
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert-output true)
    corrade_add_test(UtilityDebugAssertTestFailAssOutp___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert-output true)
    set_tests_properties(
        UtilityAssertTestFailAssertOutput
        UtilityDebugAssertTestFailAssOutp___
        PROPERTIES PASS_REGULAR_EXPRESSION "foo\\(\\) should succeed")
    corrade_add_test(UtilityAssertTestFailInternAssOut___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-output true)
    corrade_add_test(UtilityDebugAssertTestFailIntAssO___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-output true)
    set_tests_properties(
        UtilityAssertTestFailInternAssOut___
        UtilityDebugAssertTestFailIntAssO___
        PROPERTIES PASS_REGULAR_EXPRESSION "Assertion foo\\(\\) && !_failInternalAssertOutput failed at ")
    corrade_add_test(UtilityAssertTestFailInterAssExpr___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-expression true)
    corrade_add_test(UtilityDebugAssertTestFailInAssEx___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-expression true)
    set_tests_properties(
        UtilityAssertTestFailInterAssExpr___
        UtilityDebugAssertTestFailInAssEx___
        PROPERTIES PASS_REGULAR_EXPRESSION "Assertion c \\+ \\(_failInternalAssertExpression \\? -3 : 3\\) failed at ")
    corrade_add_test(UtilityAssertTestFailAssertUnreac___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert-unreachable true)
    corrade_add_test(UtilityDebugAssertTestFailAssUnre___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-assert-unreachable true)
    set_tests_properties(
        UtilityAssertTestFailAssertUnreac___
        UtilityDebugAssertTestFailAssUnre___
        PROPERTIES PASS_REGULAR_EXPRESSION "C should be 3")
    corrade_add_test(UtilityAssertTestFailInterAssUnre___
        $<TARGET_OBJECTS:AssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-unreachable true)
    corrade_add_test(UtilityDebugAssertTestFailInAssUn___
        $<TARGET_OBJECTS:DebugAssertTestObjects>
        ${_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP} # XCode workaround
        ARGUMENTS --fail-on-internal-assert-unreachable true)
    set_tests_properties(
        UtilityAssertTestFailInterAssUnre___
        UtilityDebugAssertTestFailInAssUn___
        PROPERTIES PASS_REGULAR_EXPRESSION "Reached unreachable code at ")
endif()

# The same as above, but using CORRADE_STANDARD_ASSERT
corrade_add_test(UtilityAssertStandardTest AssertTest.cpp)
corrade_add_test(UtilityDebugAssertStandardTest AssertTest.cpp)
target_compile_definitions(UtilityAssertStandardTest PRIVATE
    "CORRADE_STANDARD_ASSERT")
target_compile_definitions(UtilityDebugAssertStandardTest PRIVATE
    "CORRADE_STANDARD_ASSERT" "TEST_DEBUG_ASSERT")

corrade_add_test(UtilityAssertDisabledTest AssertDisabledTest.cpp)
corrade_add_test(UtilityDebugAssertDisabledTest AssertDisabledTest.cpp)
target_compile_definitions(UtilityDebugAssertDisabledTest PRIVATE
    "TEST_DEBUG_ASSERT")
# The same, but using CORRADE_STANDARD_ASSERT and NDEBUG
corrade_add_test(UtilityAssertStandardDisabledTest AssertDisabledTest.cpp)
corrade_add_test(UtilityDebugAssertStandardDis___Test AssertDisabledTest.cpp)
target_compile_definitions(UtilityAssertStandardDisabledTest PRIVATE
    "CORRADE_STANDARD_ASSERT" "NDEBUG")
target_compile_definitions(UtilityDebugAssertStandardDis___Test PRIVATE
    "CORRADE_STANDARD_ASSERT" "NDEBUG" "TEST_DEBUG_ASSERT")

corrade_add_test(UtilityAssertGracefulTest AssertGracefulTest.cpp)
corrade_add_test(UtilityDebugAssertGracefulTest AssertGracefulTest.cpp)
target_compile_definitions(UtilityDebugAssertGracefulTest PRIVATE
    "TEST_DEBUG_ASSERT")

corrade_add_test(UtilityEndiannessTest EndiannessTest.cpp)
corrade_add_test(UtilityErrorStringTest ErrorStringTest.cpp)
corrade_add_test(UtilityMurmurHash2Test MurmurHash2Test.cpp)

# Emscripten 3.1.3 changed the way files are bundled, putting them directly to
# WASM instead of Base64'd to the JS file. However, it broke UTF-8 handling,
# causing both a compile error (due to a syntax error in the assembly file) and
# if that's patched, also runtime errors later.
#   https://github.com/emscripten-core/emscripten/pull/16050
# TODO: re-add and re-enable the test cases once a fix is made
if(NOT CORRADE_TARGET_EMSCRIPTEN OR EMSCRIPTEN_VERSION VERSION_LESS 3.1.3)
    set(UtilityConfigurationTest_UTF8_FILES
        ConfigurationTestFiles/hýždě.conf)
endif()
corrade_add_test(UtilityConfigurationTest ConfigurationTest.cpp
    LIBRARIES CorradeTestSuiteTestLib
    FILES
        ConfigurationTestFiles/bom.conf
        ConfigurationTestFiles/comments.conf
        ConfigurationTestFiles/comments-saved.conf
        ConfigurationTestFiles/eol-mixed.conf
        ConfigurationTestFiles/eol-unix.conf
        ConfigurationTestFiles/eol-windows.conf
        ConfigurationTestFiles/hierarchic.conf
        ConfigurationTestFiles/hierarchic-empty-group.conf
        ConfigurationTestFiles/hierarchic-empty-subgroup.conf
        ConfigurationTestFiles/hierarchic-missing-bracket.conf
        ConfigurationTestFiles/hierarchic-modified.conf
        ConfigurationTestFiles/hierarchic-shortcuts.conf
        ConfigurationTestFiles/hierarchic-shortcuts-modified.conf
        ${UtilityConfigurationTest_UTF8_FILES}
        ConfigurationTestFiles/iterate.conf
        ConfigurationTestFiles/iterate-modified.conf
        ConfigurationTestFiles/missing-equals.conf
        ConfigurationTestFiles/missing-quote.conf
        ConfigurationTestFiles/missing-multiline-quote.conf
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
corrade_add_test(UtilityMacrosTest MacrosTest.cpp)
if(NOT CORRADE_TARGET_EMSCRIPTEN)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(UtilityDebugTest PRIVATE Threads::Threads)
    target_link_libraries(UtilityMacrosTest PRIVATE Threads::Threads)
endif()

corrade_add_test(UtilityMathTest MathTest.cpp)

# Build these only if there's no explicit -std= passed in the flags
if(NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0.1") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        corrade_add_test(UtilityMacrosCpp14Test MacrosCpp14Test.cpp)
        set_target_properties(
            UtilityMacrosCpp14Test
            PROPERTIES CORRADE_CXX_STANDARD 14)
   endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        corrade_add_test(UtilityDebugStlCpp17Test DebugStlCpp17Test.cpp)
        corrade_add_test(UtilityFormatStlStringViewTest FormatStlStringViewTest.cpp)
        corrade_add_test(UtilityMacrosCpp17Test MacrosCpp17Test.cpp)
        set_target_properties(
            UtilityDebugStlCpp17Test
            UtilityFormatStlStringViewTest
            UtilityMacrosCpp17Test
            PROPERTIES CORRADE_CXX_STANDARD 17)
        target_include_directories(UtilityFormatStlStringViewTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
        corrade_add_test(UtilityMacrosCpp20Test MacrosCpp20Test.cpp)
        set_target_properties(UtilityMacrosCpp20Test PROPERTIES CORRADE_CXX_STANDARD 20)
    endif()
endif()

# It should return with a non-zero exit code
corrade_add_test(UtilityFatalTest FatalTest.cpp)
set_tests_properties(UtilityFatalTest PROPERTIES WILL_FAIL ON)

corrade_add_test(UtilityJsonTest JsonTest.cpp
    LIBRARIES CorradeTestSuiteTestLib
    FILES
        JsonTestFiles/error.json
        JsonTestFiles/parse-error.json)
target_compile_definitions(UtilityJsonTest PRIVATE "CORRADE_GRACEFUL_ASSERT")
target_include_directories(UtilityJsonTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityJsonWriterTest JsonWriterTest.cpp LIBRARIES CorradeTestSuiteTestLib)
target_include_directories(UtilityJsonWriterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityMemoryTest MemoryTest.cpp)
target_compile_definitions(UtilityMemoryTest PRIVATE "CORRADE_GRACEFUL_ASSERT")

corrade_add_test(UtilityMoveTest MoveTest.cpp)

set(UtilityPathTest_SRCS PathTest.cpp)
if(CORRADE_TARGET_IOS)
    set_source_files_properties(
        PathTestFiles
        PathTestFilesSymlink
        PathTestFilesUtf8
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND PathDirectoryTest_SRCS
        PathTestFiles
        PathTestFilesSymlink
        PathTestFilesUtf8)
endif()
# Emscripten 3.1.3 changed the way files are bundled, putting them directly to
# WASM instead of Base64'd to the JS file. However, it broke UTF-8 handling,
# causing both a compile error (due to a syntax error in the assembly file) and
# if that's patched, also runtime errors later.
#   https://github.com/emscripten-core/emscripten/pull/16050
# TODO: re-add and re-enable the test cases once a fix is made
if(NOT CORRADE_TARGET_EMSCRIPTEN OR EMSCRIPTEN_VERSION VERSION_LESS 3.1.3)
    set(UtilityPathTest_UTF8_FILES
        PathTestFilesUtf8/šňůra/dummy
        PathTestFilesUtf8/šňůra/klíče
        PathTestFilesUtf8/hýždě)
endif()
corrade_add_test(UtilityPathTest ${UtilityPathTest_SRCS}
    FILES
        PathTestFiles/dir/dummy
        PathTestFiles/file
        PathTestFilesSymlink/dir/dummy
        PathTestFilesSymlink/dir-symlink
        PathTestFilesSymlink/file
        PathTestFilesSymlink/file-symlink
        ${UtilityPathTest_UTF8_FILES})
if(CORRADE_TARGET_UNIX)
    # Without -fPIE libraryLocation() may sometimes return the executable
    # location and not library location. See its docs for more information.
    target_compile_options(UtilityPathTest PRIVATE -fPIE)
endif()
target_include_directories(UtilityPathTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityFormatTest FormatTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(UtilityFormatStlTest FormatStlTest.cpp)
target_include_directories(UtilityFormatTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(UtilityFormatStlTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityFormatBenchmark FormatBenchmark.cpp)

corrade_add_test(UtilityHashDigestTest HashDigestTest.cpp)

corrade_add_test(UtilitySha1Test Sha1Test.cpp)
corrade_add_test(UtilityStlForwardArrayTest StlForwardArrayTest.cpp)
corrade_add_test(UtilityStlForwardStringTest StlForwardStringTest.cpp)
corrade_add_test(UtilityStlForwardTupleTest StlForwardTupleTest.cpp)
corrade_add_test(UtilityStlForwardTupleSizeEle___Test StlForwardTupleSizeElementTest.cpp)
corrade_add_test(UtilityStlForwardVectorTest StlForwardVectorTest.cpp)

# Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
    corrade_add_test(UtilityStlMathTest StlMathTest.cpp)
    set_target_properties(UtilityStlMathTest PROPERTIES
        CORRADE_CXX_STANDARD 17)
endif()

corrade_add_test(UtilityStringTest StringTest.cpp LIBRARIES CorradeTestSuiteTestLib)

corrade_add_test(UtilityStringBenchmark StringBenchmark.cpp
    LIBRARIES CorradeTestSuiteTestLib
    FILES ${PROJECT_SOURCE_DIR}/src/Corrade/Containers/Test/StringTestFiles/lorem-ipsum.txt)
target_include_directories(UtilityStringBenchmark PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilitySystemTest SystemTest.cpp)
corrade_add_test(UtilityTweakableParserTest TweakableParserTest.cpp)
corrade_add_test(UtilityTypeTraitsTest TypeTraitsTest.cpp)
corrade_add_test(UtilityUnicodeTest UnicodeTest.cpp LIBRARIES CorradeTestSuiteTestLib)

# Compiled-in resource test
corrade_add_resource(ResourceTestData ResourceTestFiles/resources.conf)
corrade_add_resource(ResourceTestEmptyFileData ResourceTestFiles/resources-empty-file.conf)
corrade_add_resource(ResourceTestNothingData ResourceTestFiles/resources-nothing.conf)
corrade_add_resource(ResourceTestSpacesData ResourceTestFiles/resources-spaces.conf)
corrade_add_resource(ResourceTestNullTerminatedAlignedData ResourceTestFiles/resources-null-terminated-aligned.conf)
corrade_add_resource(ResourceTestNullTerminatedLastFileData ResourceTestFiles/resources-null-terminated-last-file.conf)
corrade_add_resource(ResourceTestAlignmentLargerThanDataSizeData ResourceTestFiles/resources-alignment-larger-than-data-size.conf)
corrade_add_resource(ResourceTestSingleData ResourceTestFiles/consequence.bin SINGLE)
corrade_add_resource(ResourceTestSingleEmptyData ResourceTestFiles/empty.bin SINGLE)
corrade_add_test(UtilityResourceTest
    ResourceTest.cpp
    ${ResourceTestData}
    ${ResourceTestEmptyFileData}
    ${ResourceTestNothingData}
    ${ResourceTestSpacesData}
    ${ResourceTestNullTerminatedAlignedData}
    ${ResourceTestNullTerminatedLastFileData}
    ${ResourceTestAlignmentLargerThanDataSizeData}
    ${ResourceTestSingleData}
    ${ResourceTestSingleEmptyData}
    LIBRARIES CorradeTestSuiteTestLib
    FILES
        ResourceTestFiles/17bytes-33.bin
        ResourceTestFiles/17bytes-66.bin
        ResourceTestFiles/55bytes-66.bin
        ResourceTestFiles/64bytes-33.bin
        ResourceTestFiles/consequence.bin
        # Referenced from resources-overriden.conf
        ResourceTestFiles/consequence2.txt
        ResourceTestFiles/empty.bin
        ResourceTestFiles/predisposition.bin
        # Referenced from resources-overriden.conf
        ResourceTestFiles/predisposition2.txt
        ResourceTestFiles/resources-overridden.conf
        ResourceTestFiles/resources-overridden-different.conf
        ResourceTestFiles/resources-overridden-none.conf
        ResourceTestFiles/resources-overridden-nonexistent-file.conf)
target_include_directories(UtilityResourceTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData}
    ${ResourceTestNothingData})
if(CMAKE_GENERATOR STREQUAL Xcode)
    # Xcode's "new build system" doesn't like when the same (generated) source
    # file is used by two different targets (UtilityResourceTest and
    # ResourceTestDataLib) that don't have any other dependency between them.
    # WTF.
    add_dependencies(UtilityResourceTest ResourceTestDataLib)
endif()
target_compile_definitions(ResourceTestDataLib PRIVATE
    "CORRADE_AUTOMATIC_INITIALIZER=CORRADE_NOOP"
    "CORRADE_AUTOMATIC_FINALIZER=CORRADE_NOOP")
target_link_libraries(ResourceTestDataLib PUBLIC CorradeUtility)
corrade_add_test(UtilityResourceStaticTest ResourceStaticTest.cpp
    LIBRARIES ResourceTestDataLib
    FILES
        ResourceTestFiles/consequence.bin
        ResourceTestFiles/predisposition.bin)
target_include_directories(UtilityResourceStaticTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Emscripten 3.1.3 changed the way files are bundled, putting them directly to
# WASM instead of Base64'd to the JS file. However, it broke UTF-8 handling,
# causing both a compile error (due to a syntax error in the assembly file) and
# if that's patched, also runtime errors later.
#   https://github.com/emscripten-core/emscripten/pull/16050
# TODO: re-add and re-enable the test cases once a fix is made
if(NOT CORRADE_TARGET_EMSCRIPTEN OR EMSCRIPTEN_VERSION VERSION_LESS 3.1.3)
    set(UtilityResourceCompileTest_UTF8_FILES
        ResourceTestFiles/hýždě.bin
        ResourceTestFiles/hýždě.conf)
endif()
corrade_add_test(UtilityResourceCompileTest
    ResourceCompileTest.cpp
    FILES
        ResourceTestFiles/17bytes-33.bin
        ResourceTestFiles/17bytes-66.bin
        ResourceTestFiles/55bytes-66.bin
        ResourceTestFiles/64bytes-33.bin
        ResourceTestFiles/compiled.cpp
        ResourceTestFiles/compiled-alignment-larger-than-data-size.cpp
        ResourceTestFiles/compiled-empty.cpp
        ResourceTestFiles/compiled-nothing.cpp
        ResourceTestFiles/compiled-null-terminated-aligned.cpp
        ResourceTestFiles/compiled-null-terminated-last-file.cpp
        ResourceTestFiles/compiled-single.cpp
        ResourceTestFiles/compiled-single-empty.cpp
        ResourceTestFiles/compiled-unicode.cpp
        ResourceTestFiles/consequence.bin
        ResourceTestFiles/empty.bin
        ${UtilityResourceCompileTest_UTF8_FILES}
        ResourceTestFiles/predisposition.bin
        ResourceTestFiles/resources.conf
        ResourceTestFiles/resources-alignment-larger-than-data-size.conf
        ResourceTestFiles/resources-empty-alias.conf
        ResourceTestFiles/resources-empty-filename.conf
        ResourceTestFiles/resources-empty-group.conf
        ResourceTestFiles/resources-no-group.conf
        ResourceTestFiles/resources-nonexistent.conf
        ResourceTestFiles/resources-nothing.conf
        ResourceTestFiles/resources-npot-align.conf
        ResourceTestFiles/resources-npot-global-align.conf
        ResourceTestFiles/resources-null-terminated-aligned.conf
        ResourceTestFiles/resources-null-terminated-last-file.conf
        ResourceTestFiles/resources-too-large-align.conf
        ResourceTestFiles/resources-too-large-global-align.conf
        ResourceTestFiles/resources-zero-align.conf
        ResourceTestFiles/resources-zero-global-align.conf)
target_include_directories(UtilityResourceCompileTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(UtilityResourceCompileTest PRIVATE "CORRADE_GRACEFUL_ASSERT")

if(CORRADE_BUILD_STATIC AND NOT CORRADE_TARGET_EMSCRIPTEN AND NOT CORRADE_TARGET_IOS AND NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_WINDOWS_RT)
    add_library(UtilityGlobalStateAcrossLibrariesLibrary SHARED
        GlobalStateAcrossLibrariesLibrary.cpp
        ${ResourceTestData})
    if(CMAKE_GENERATOR STREQUAL Xcode)
        # Xcode's "new build system" doesn't like when the same (generated)
        # source file is used by two different targets (ResourceTestDataLib and
        # UtilityGlobalStateAcrossLibrariesLibrary) that don't have any other
        # dependency between them. WTF.
        add_dependencies(UtilityGlobalStateAcrossLibrariesLibrary ResourceTestDataLib)
    endif()
    target_link_libraries(UtilityGlobalStateAcrossLibrariesLibrary PRIVATE CorradeUtility)

    corrade_add_test(UtilityGlobalStateAcrossLibrariesTest
        GlobalStateAcrossLibrariesTest.cpp
        LIBRARIES UtilityGlobalStateAcrossLibrariesLibrary)
endif()

# Unix-specific / non-RT-Windows-specific functionality. Also Emscripten.
if(CORRADE_TARGET_UNIX OR (CORRADE_TARGET_WINDOWS AND NOT CORRADE_TARGET_WINDOWS_RT) OR CORRADE_TARGET_EMSCRIPTEN)
    corrade_add_test(UtilityFileWatcherTest FileWatcherTest.cpp)
    target_include_directories(UtilityFileWatcherTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    corrade_add_test(UtilityTweakableTest TweakableTest.cpp)
    corrade_add_test(UtilityTweakableIntegrationTest TweakableIntegrationTest.cpp
        FILES TweakableIntegrationTest.cpp)
    target_include_directories(UtilityTweakableIntegrationTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(CORRADE_BUILD_DEPRECATED)
    set(UtilityDirectoryTest_SRCS DirectoryTest.cpp)
    if(CORRADE_TARGET_IOS)
        set_source_files_properties(
            PathTestFiles
            PathTestFilesSymlink
            PathTestFilesUtf8
            PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        list(APPEND UtilityDirectoryTest_SRCS
            PathTestFiles
            PathTestFilesSymlink
            PathTestFilesUtf8)
    endif()
    # Emscripten 3.1.3 changed the way files are bundled, putting them directly
    # to WASM instead of Base64'd to the JS file. However, it broke UTF-8
    # handling, causing both a compile error (due to a syntax error in the
    # assembly file) and if that's patched, also runtime errors later.
    #   https://github.com/emscripten-core/emscripten/pull/16050
    # TODO: re-add and re-enable the test cases once a fix is made
    if(NOT CORRADE_TARGET_EMSCRIPTEN OR EMSCRIPTEN_VERSION VERSION_LESS 3.1.3)
        set(UtilityDirectoryTest_UTF8_FILES
            PathTestFilesUtf8/šňůra/dummy
            PathTestFilesUtf8/hýždě)
    endif()
    corrade_add_test(UtilityDirectoryTest ${UtilityDirectoryTest_SRCS}
        FILES
            PathTestFiles/dir/dummy
            PathTestFiles/file
            PathTestFilesSymlink/dir/dummy
            PathTestFilesSymlink/dir-symlink
            PathTestFilesSymlink/file
            PathTestFilesSymlink/file-symlink
            ${UtilityDirectoryTest_UTF8_FILES})
    if(CORRADE_TARGET_UNIX)
        # Without -fPIE libraryLocation() may sometimes return the executable
        # location and not library location. See its docs for more information.
        target_compile_options(UtilityDirectoryTest PRIVATE -fPIE)
    endif()
    target_include_directories(UtilityDirectoryTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()
