include_directories(${Map2XCore_SOURCE_DIR}/src ${Map2XCore_BINARY_DIR}/src)

add_definitions(-Wall -pedantic)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AbstractPluginManagerConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AbstractPluginManagerConfigure.h)

set(Map2X_PluginManager_SRCS
    Plugin.cpp
    AbstractPluginManager.cpp
)

add_library(Map2XPluginManager STATIC ${Map2X_PluginManager_SRCS})
target_link_libraries(Map2XPluginManager Map2XUtility)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
