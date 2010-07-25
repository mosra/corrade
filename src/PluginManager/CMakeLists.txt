include_directories(${Map2XCore_SOURCE_DIR}/src)

add_definitions(-Wall -pedantic)

set(Map2X_PluginManager_SRCS
    AbstractPluginManager.cpp
    PluginManager.cpp
)

add_library(Map2XPluginManager STATIC ${Map2X_PluginManager_SRCS})
target_link_libraries(Map2XPluginManager Map2XUtility)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
