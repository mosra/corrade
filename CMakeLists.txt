cmake_minimum_required(VERSION 2.6)

project(Corrade)

option(BUILD_TESTS "Build unit tests (requires Qt4)." OFF)

if(BUILD_TESTS)
    find_package(Qt4)

    if(NOT QT4_FOUND)
        message(WARNING "Qt4 is required for building unit tests. No tests will be build.")
        set(BUILD_TESTS OFF)
    else()
        enable_testing()
    endif()

endif()

set(CMAKE_MODULE_PATH "${Corrade_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

include(CorradeMacros)

# Default directories
set_parent_scope(CORRADE_INCLUDE_DIR            "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_parent_scope(CORRADE_UTILITY_LIBRARY        CorradeUtility)
set_parent_scope(CORRADE_PLUGINMANAGER_LIBRARY  CorradePluginManager)
set_parent_scope(CORRADE_RC_EXECUTABLE          corrade-rc)

include(FindCorrade)

# On 64bit Linux with GCC flag -fPIC is needed for building some static libraries
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCC AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(USE_FPIC ON)
else()
    set(USE_FPIC OFF)
endif()

# Library version
set(CORRADE_CORE_LIBRARY_VERSION 0.1.60)
set(CORRADE_CORE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)

if(NOT WIN32)
    install(DIRECTORY src/ DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}
                 FILES_MATCHING PATTERN "*.h"
                 PATTERN "*/Test" EXCLUDE
                 PATTERN "*/doc" EXCLUDE)
endif()
