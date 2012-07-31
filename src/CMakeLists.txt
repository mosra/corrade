set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wold-style-cast -pedantic -fvisibility=hidden -std=c++0x")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTS)
    enable_testing()
    include_directories(${QT_INCLUDE_DIR})
endif()

add_subdirectory(PluginManager)
add_subdirectory(Utility)
