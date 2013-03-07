add_definitions(-Wall -pedantic)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/corradePluginManagerConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/corradePluginManagerConfigure.h)

set(CorradePluginManager_SRCS
    AbstractPlugin.cpp
    AbstractPluginManager.cpp
    PluginMetadata.cpp)

set(CorradePluginManager_HEADERS
    AbstractPlugin.h
    AbstractPluginManager.h
    PluginManager.h
    PluginMetadata.h

    corradePluginManagerVisibility.h)

add_library(CorradePluginManager SHARED ${CorradePluginManager_SRCS})
target_link_libraries(CorradePluginManager CorradeUtility)
if(UNIX OR ${CMAKE_SYSTEM_NAME} STREQUAL NaCl)
    target_link_libraries(CorradePluginManager dl)
endif()
set_target_properties(CorradePluginManager PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})

install(TARGETS CorradePluginManager DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradePluginManager_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/PluginManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
