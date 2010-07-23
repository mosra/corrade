include_directories(${Map2XCore_SOURCE_DIR}/src)

add_definitions(-Wall -pedantic)

set(Map2X_PluginManager_SRCS
    PluginManagerStatic.cpp
    PluginManager.cpp
)

add_library(Map2XPluginManager STATIC ${Map2X_PluginManager_SRCS})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
