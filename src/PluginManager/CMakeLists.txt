set(Corrade_PluginManager_SRCS
    Plugin.cpp
    AbstractPluginManager.cpp
    PluginMetadata.cpp
)

add_library(CorradePluginManager SHARED ${Corrade_PluginManager_SRCS})
target_link_libraries(CorradePluginManager CorradeUtility)
set_target_properties(CorradePluginManager PROPERTIES VERSION ${CORRADE_CORE_LIBRARY_VERSION} SOVERSION ${CORRADE_CORE_LIBRARY_SOVERSION})

if(WIN32)
    set_target_properties(CorradePluginManager PROPERTIES COMPILE_FLAGS -DPLUGINMANAGER_EXPORTING)
endif()

if(USE_FPIC)
    set_target_properties(CorradePluginManager PROPERTIES COMPILE_FLAGS -fPIC)
endif()

install(TARGETS CorradePluginManager DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})

add_subdirectory(Plugins)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
