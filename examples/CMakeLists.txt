cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wold-style-cast -Winit-self -pedantic -fvisibility=hidden -std=c++0x")

find_package(Corrade REQUIRED)

include_directories(${CORRADE_INCLUDE_DIR})

add_subdirectory(test)
