add_definitions(-Wall -pedantic)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AbstractPluginManagerConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AbstractPluginManagerConfigure.h)

set(Kompas_PluginManager_SRCS
    Plugin.cpp
    AbstractPluginManager.cpp
    PluginMetadata.cpp
)

add_library(KompasPluginManager SHARED ${Kompas_PluginManager_SRCS})
target_link_libraries(KompasPluginManager KompasUtility)
set_target_properties(KompasPluginManager PROPERTIES VERSION ${KOMPAS_CORE_LIBRARY_VERSION} SOVERSION ${KOMPAS_CORE_LIBRARY_SOVERSION})

if(WIN32)
    set_target_properties(KompasPluginManager PROPERTIES COMPILE_FLAGS -DPLUGINMANAGER_EXPORTING)
endif()

if(USE_FPIC)
    set_target_properties(KompasPluginManager PROPERTIES COMPILE_FLAGS -fPIC)
endif()

install(TARGETS KompasPluginManager DESTINATION ${KOMPAS_LIBRARY_INSTALL_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
