set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

include_directories(${CORRADE_INCLUDE_DIR})

add_subdirectory(PluginManager)
add_subdirectory(Utility)
