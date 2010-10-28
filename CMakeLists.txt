cmake_minimum_required(VERSION 2.6)

project(Map2XCore)

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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Map2XCore_SOURCE_DIR}/modules/")

include(Map2XMacros)

# Default directories
set_parent_scope(MAP2X_CORE_INCLUDE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_parent_scope(MAP2X_PLUGINS_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/Plugins")
set_parent_scope(MAP2X_CORE_LIBRARY         Map2XCore)
set_parent_scope(MAP2X_RC_EXECUTABLE        map2x-rc)

include(FindMap2XCore)

# On 64bit Linux with GCC flag -fPIC is needed for building some static libraries
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCC AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(USE_FPIC ON)
else()
    set(USE_FPIC OFF)
endif()

add_subdirectory(modules)
add_subdirectory(src)

install(DIRECTORY src/ DESTINATION ${MAP2X_CORE_INCLUDE_INSTALL_DIR}
                 FILES_MATCHING PATTERN "*.h"
                 PATTERN "*/Test" EXCLUDE
                 PATTERN src/Plugins EXCLUDE)
install(DIRECTORY src/Plugins/ DESTINATION ${MAP2X_PLUGINS_INCLUDE_INSTALL_DIR}
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*/Test" EXCLUDE)
