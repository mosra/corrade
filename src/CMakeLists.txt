include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTS)
    enable_testing()
    include_directories(${QT_INCLUDE_DIR})
endif()

add_subdirectory(PluginManager)
add_subdirectory(TestSuite)
add_subdirectory(Utility)
