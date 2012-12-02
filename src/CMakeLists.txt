include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(GCC46_COMPATIBILITY)
    set(CORRADE_GCC46_COMPATIBILITY 1)
endif()

include(TestBigEndian)
test_big_endian(CMAKE_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/corradeConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Install also the configure file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

set(Corrade_HEADERS
    corradeCompatibility.h)

install(FILES ${Corrade_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

if(BUILD_TESTS)
    # Explicitly set TestSuite libraries, because CORRADE_TESTSUITE_LIBRARIES
    # is not available here
    set(CORRADE_TESTSUITE_LIBRARIES CorradeTestSuite CorradeUtility)

    enable_testing()
    include_directories(${QT_INCLUDE_DIR})
endif()

add_subdirectory(Containers)
add_subdirectory(Interconnect)
add_subdirectory(PluginManager)
add_subdirectory(TestSuite)
add_subdirectory(Utility)
