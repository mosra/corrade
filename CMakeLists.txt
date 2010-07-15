cmake_minimum_required(VERSION 2.6)

project(Map2XCore)

set(CMAKE_MODULE_PATH "${Map2XCore_SOURCE_DIR}/modules/")

add_subdirectory(src)

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
