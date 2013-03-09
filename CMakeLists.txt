cmake_minimum_required(VERSION 2.6)
project(Corrade)

option(GCC46_COMPATIBILITY "Enable compatibility mode for GCC 4.6 (might disable some features)" OFF)
option(BUILD_TESTS "Build unit tests." OFF)
if(BUILD_TESTS)
    enable_testing()
endif()

# Initialize macros etc.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CORRADE_UTILITY_LIBRARY CorradeUtility)
set(CORRADE_RC_EXECUTABLE corrade-rc)
if(BUILD_TESTS)
    set(CORRADE_TESTSUITE_LIBRARIES CorradeTestSuite CorradeUtility)
endif()
include(UseCorrade)

# Installation paths
include(CorradeLibSuffix)
set(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)
set(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)
