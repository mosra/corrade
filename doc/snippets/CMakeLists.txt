#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022
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
set(CMAKE_FOLDER "Corrade/doc/snippets")

# On MSVC remove /W3, as we are replacing it with /W4
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set_directory_properties(PROPERTIES
    CORRADE_CXX_STANDARD 11
    CORRADE_USE_PEDANTIC_FLAGS ON)

add_library(snippets STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET}
    Corrade.cpp
    Containers.cpp
    Containers-stl.cpp
    Utility.cpp)
target_link_libraries(snippets PRIVATE CorradeUtility)
if(CORRADE_TESTSUITE_TEST_TARGET)
    add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets)
endif()

# Build these only if there's no explicit -std= passed in the flags
if(NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.9") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
        add_library(snippets-cpp14 STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET}
            Containers-cpp14.cpp)
        target_link_libraries(snippets-cpp14 PRIVATE CorradeUtility)
        set_target_properties(snippets-cpp14 PROPERTIES
            CORRADE_CXX_STANDARD 14)
        if(CORRADE_TESTSUITE_TEST_TARGET)
            add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-cpp14)
        endif()
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        add_library(snippets-cpp17 STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET}
            Corrade-cpp17.cpp
            Containers-stl17.cpp)
        target_link_libraries(snippets-cpp17 PRIVATE CorradeUtility)
        set_target_properties(snippets-cpp17 PROPERTIES
            CORRADE_CXX_STANDARD 17)
        if(CORRADE_TESTSUITE_TEST_TARGET)
            add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-cpp17)
        endif()
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
        add_library(snippets-cpp2a STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET}
            Containers-stl2a.cpp)
        target_link_libraries(snippets-cpp2a PRIVATE CorradeUtility)
        set_target_properties(snippets-cpp2a PROPERTIES
            CORRADE_CXX_STANDARD 20)
        if(CORRADE_TESTSUITE_TEST_TARGET)
            add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-cpp2a)
        endif()
    endif()
endif()

if(CORRADE_WITH_INTERCONNECT)
    add_library(snippets-Interconnect STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} Interconnect.cpp)
    target_link_libraries(snippets-Interconnect PRIVATE CorradeInterconnect)
    if(CORRADE_TESTSUITE_TEST_TARGET)
        add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-Interconnect)
    endif()
endif()

if(CORRADE_WITH_PLUGINMANAGER)
    add_library(snippets-PluginManager STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} PluginManager.cpp)
    target_link_libraries(snippets-PluginManager PRIVATE CorradePluginManager)
    if(CORRADE_TESTSUITE_TEST_TARGET)
        add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-PluginManager)
    endif()
endif()

# TODO: causes spurious linker errors on Travis iOS build, so I'm disabling it
if(CORRADE_WITH_TESTSUITE AND NOT CORRADE_TARGET_IOS)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    # All define the same symbols so they need to be in separate libs
    add_library(snippets-TestSuite STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} TestSuite.cpp)
    target_link_libraries(snippets-TestSuite PRIVATE CorradeTestSuite)
    add_library(snippets-TestSuite2 STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} TestSuite2.cpp)
    target_link_libraries(snippets-TestSuite2 PRIVATE CorradeTestSuite)
    add_library(snippets-TestSuite3 STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} TestSuite3.cpp)
    target_link_libraries(snippets-TestSuite3 PRIVATE CorradeTestSuite)

    # TestSuite documentation snippets. I need them executable so I can
    # copy&paste the output to the documentation. Also mot using
    # corrade_add_test() because these tests shouldn't be run as part of CTest
    # -- some of them fail on purpose
    add_executable(testsuite-basic ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-basic.cpp)
    add_executable(testsuite-templated ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-templated.cpp)
    add_executable(testsuite-repeated ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-repeated.cpp)
    add_executable(testsuite-instanced ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-instanced.cpp)
    add_executable(testsuite-save-diagnostic ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-save-diagnostic.cpp)
    add_executable(testsuite-iteration ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-iteration.cpp)
    add_executable(testsuite-benchmark ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-benchmark.cpp)
    add_executable(testsuite-benchmark-custom ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-benchmark-custom.cpp)
    add_executable(testsuite-description-source-location ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} testsuite-description-source-location.cpp)

    target_link_libraries(testsuite-basic PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-templated PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-repeated PUBLIC CorradeTestSuite Threads::Threads)
    target_link_libraries(testsuite-instanced PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-save-diagnostic PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-iteration PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-benchmark PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-benchmark-custom PUBLIC CorradeTestSuite)
    target_link_libraries(testsuite-description-source-location PUBLIC CorradeTestSuite)

    if(CORRADE_TARGET_EMSCRIPTEN)
        # Wasn't a problem for Emscripten <= 2.0.13 and isn't a problem for
        # 3.1+, but for some reason 2.0.20 needs this. The corrade_add_test()
        # macro does this automatically, but here we need to do it manually.
        set_property(TARGET
            testsuite-basic
            testsuite-templated
            testsuite-repeated
            testsuite-instanced
            testsuite-save-diagnostic
            testsuite-iteration
            testsuite-benchmark
            testsuite-benchmark-custom
            testsuite-description-source-location
            APPEND_STRING PROPERTY LINK_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
    endif()

    if(CORRADE_TESTSUITE_TEST_TARGET)
        add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET}
            testsuite-basic
            testsuite-templated
            testsuite-repeated
            testsuite-instanced
            testsuite-save-diagnostic
            testsuite-iteration
            testsuite-benchmark
            testsuite-benchmark-custom
            testsuite-description-source-location)
    endif()
endif()

if(CORRADE_TARGET_ANDROID)
    add_library(snippets-android STATIC ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} android.cpp)
    target_link_libraries(snippets-android PRIVATE CorradeUtility)
    if(CORRADE_TESTSUITE_TEST_TARGET)
        add_dependencies(${CORRADE_TESTSUITE_TEST_TARGET} snippets-android)
    endif()
endif()
