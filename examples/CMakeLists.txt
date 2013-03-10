cmake_minimum_required(VERSION 2.8)

find_package(Corrade REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CORRADE_CXX_FLAGS}")
include_directories(${CORRADE_INCLUDE_DIR})

add_subdirectory(interconnect)
add_subdirectory(pluginmanager)
add_subdirectory(test)
