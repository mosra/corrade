include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(GCC44_COMPATIBILITY)
    set(CORRADE_GCC44_COMPATIBILITY 1)
    set(GCC45_COMPATIBILITY 1)
endif()

if(GCC45_COMPATIBILITY)
    set(CORRADE_GCC45_COMPATIBILITY 1)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/corradeConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Install also the configure file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

if(BUILD_TESTS)
    enable_testing()
    include_directories(${QT_INCLUDE_DIR})
endif()

add_subdirectory(PluginManager)
add_subdirectory(TestSuite)
add_subdirectory(Utility)
