find_package(Qt4 REQUIRED)

enable_testing()
include(Map2XMacros)

set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DirectoryTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DirectoryTestConfigure.h)

include_directories(${Map2XCore_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

map2x_add_test(DirectoryTest DirectoryTest.h DirectoryTest.cpp Map2XUtility)
map2x_add_test(EndianTest EndianTest.h EndianTest.cpp)
map2x_add_test(UtilitiesTest UtilitiesTest.h UtilitiesTest.cpp Map2XUtility)
