set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CORRADE_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(GCC46_COMPATIBILITY)
    set(CORRADE_GCC46_COMPATIBILITY 1)
endif()

include(TestBigEndian)
test_big_endian(CMAKE_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/corradeConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h)

set(Corrade_HEADERS
    corradeCompatibility.h)

install(FILES ${Corrade_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

add_subdirectory(Containers)
add_subdirectory(Interconnect)
add_subdirectory(PluginManager)
add_subdirectory(TestSuite)
add_subdirectory(Utility)
