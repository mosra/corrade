#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
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

# On MSVC remove /W3, as we are replacing it with /W4
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set_directory_properties(PROPERTIES
    CORRADE_CXX_STANDARD 11
    CORRADE_USE_PEDANTIC_FLAGS ON)

add_library(snippets STATIC
    Containers.cpp
    Containers-stl.cpp
    Utility.cpp)
target_link_libraries(snippets PRIVATE CorradeUtility)
set_target_properties(snippets PROPERTIES FOLDER "Corrade/doc/snippets")

# Build these only if there's no explicit -std= passed in the flags
if(NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        add_library(snippets-cpp17 STATIC
            Containers-stl17.cpp)
        target_link_libraries(snippets-cpp17 PRIVATE CorradeUtility)
        set_target_properties(snippets-cpp17 PROPERTIES
            CORRADE_CXX_STANDARD 17
            FOLDER "Corrade/doc/snippets")
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
        add_library(snippets-cpp2a STATIC
            Containers-stl2a.cpp)
        target_link_libraries(snippets-cpp2a PRIVATE CorradeUtility)
        set_target_properties(snippets-cpp2a PROPERTIES
            CORRADE_CXX_STANDARD 20
            FOLDER "Corrade/doc/snippets")
    endif()
endif()

if(WITH_INTERCONNECT)
    add_library(snippets-Interconnect STATIC Interconnect.cpp)
    target_link_libraries(snippets-Interconnect PRIVATE CorradeInterconnect)
    set_target_properties(snippets-Interconnect PROPERTIES FOLDER "Corrade/doc/snippets")
endif()

if(WITH_PLUGINMANAGER)
    add_library(snippets-PluginManager STATIC PluginManager.cpp)
    target_link_libraries(snippets-PluginManager PRIVATE CorradePluginManager)
    set_target_properties(snippets-PluginManager PROPERTIES FOLDER "Corrade/doc/snippets")
endif()

# TODO: causes spurious linker errors on Travis iOS build, so I'm disabling it
if(WITH_TESTSUITE AND NOT CORRADE_TARGET_IOS)
    find_package(Threads REQUIRED)

    # All define the same symbols so they need to be in separate libs
    add_library(snippets-TestSuite STATIC TestSuite.cpp)
    target_link_libraries(snippets-TestSuite PRIVATE CorradeTestSuite)
    add_library(snippets-TestSuite2 STATIC TestSuite2.cpp)
    target_link_libraries(snippets-TestSuite2 PRIVATE CorradeTestSuite)
    add_library(snippets-TestSuite3 STATIC TestSuite3.cpp)
    target_link_libraries(snippets-TestSuite3 PRIVATE CorradeTestSuite)

    # TestSuite documentation snippets. I need them executable so I can
    # copy&paste the output to the documentation. Also mot using
    # corrade_add_test() because these tests shouldn't be run as part of CTest
    # -- some of them fail on purpose
    add_executable(testsuite-basic testsuite-basic.cpp)
    add_executable(testsuite-templated testsuite-templated.cpp)
    add_executable(testsuite-repeated testsuite-repeated.cpp)
    add_executable(testsuite-instanced testsuite-instanced.cpp)
    add_executable(testsuite-save-diagnostic testsuite-save-diagnostic.cpp)
    add_executable(testsuite-iteration testsuite-iteration.cpp)
    add_executable(testsuite-benchmark testsuite-benchmark.cpp)
    add_executable(testsuite-benchmark-custom testsuite-benchmark-custom.cpp)
    set_target_properties(
        testsuite-basic
        testsuite-templated
        testsuite-repeated
        testsuite-instanced
        testsuite-save-diagnostic
        testsuite-iteration
        testsuite-benchmark
        testsuite-benchmark-custom
        PROPERTIES FOLDER "Corrade/doc/snippets")

    target_link_libraries(testsuite-basic CorradeTestSuite)
    target_link_libraries(testsuite-templated CorradeTestSuite)
    target_link_libraries(testsuite-repeated CorradeTestSuite ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(testsuite-instanced CorradeTestSuite)
    target_link_libraries(testsuite-save-diagnostic CorradeTestSuite)
    target_link_libraries(testsuite-iteration CorradeTestSuite)
    target_link_libraries(testsuite-benchmark CorradeTestSuite)
    target_link_libraries(testsuite-benchmark-custom CorradeTestSuite)
endif()

if(CORRADE_TARGET_ANDROID)
    add_library(snippets-android STATIC android.cpp)
    target_link_libraries(snippets-android PRIVATE CorradeUtility)
    set_target_properties(snippets-android PROPERTIES FOLDER "Corrade/doc/snippets")
endif()
