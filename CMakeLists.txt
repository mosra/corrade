cmake_minimum_required(VERSION 2.6)

project(Corrade)

option(GCC46_COMPATIBILITY "Enable compatibility mode for GCC 4.6 (might disable some features)" OFF)

option(BUILD_TESTS "Build unit tests (requires Qt4)." OFF)
option(BUILD_EXAMPLES "Build examples." OFF)

if(BUILD_TESTS)
    find_package(Qt4)

    if(NOT QT4_FOUND)
        message(FATAL_ERROR "Qt4, required for building unit tests, was not found. Set BUILD_TESTS to OFF to skip building them.")
    endif()

    enable_testing()
endif()

set(CMAKE_MODULE_PATH "${Corrade_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

set(CORRADE_UTILITY_LIBRARY CorradeUtility)
set(CORRADE_RC_EXECUTABLE corrade-rc)
include(CorradeMacros)
include(CorradeLibSuffix)

set(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)
set(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wold-style-cast -Winit-self -pedantic -fvisibility=hidden -std=c++0x")

add_subdirectory(modules)
add_subdirectory(src)

if(BUILD_EXAMPLES)
    # Work around find_package(Corrade)
    set(CORRADE_UTILITY_LIBRARY CorradeUtility)
    set(CORRADE_PLUGINMANAGER_LIBRARY CorradePluginManager)
    set(CORRADE_RC_EXECUTABLE corrade-rc)
    set(CORRADE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")

    add_subdirectory(examples)
endif()

install(DIRECTORY src/ DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*/Test" EXCLUDE
        PATTERN "*/doc" EXCLUDE)
