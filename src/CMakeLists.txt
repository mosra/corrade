set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

include_directories(${CORRADE_INCLUDE_DIR})

if(BUILD_TESTS)
    enable_testing()
    include_directories(${QT_INCLUDE_DIR})
endif()

add_subdirectory(PluginManager)
add_subdirectory(Utility)
