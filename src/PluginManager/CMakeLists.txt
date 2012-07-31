add_definitions(-Wall -pedantic)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AbstractPluginManagerConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/AbstractPluginManagerConfigure.h)

set(CorradePluginManager_SRCS
    Plugin.cpp
    AbstractPluginManager.cpp
    PluginMetadata.cpp)

add_library(CorradePluginManager SHARED ${CorradePluginManager_SRCS})
target_link_libraries(CorradePluginManager CorradeUtility)
set_target_properties(CorradePluginManager PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})

install(TARGETS CorradePluginManager DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
