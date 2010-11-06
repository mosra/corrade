find_package(Qt4 REQUIRED)

enable_testing()
include(Map2XMacros)

set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/testConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

map2x_add_test(DirectoryTest DirectoryTest.h DirectoryTest.cpp Map2XUtility)
map2x_add_test(EndianTest EndianTest.h EndianTest.cpp Map2XUtility)
map2x_add_test(UtilitiesTest UtilitiesTest.h UtilitiesTest.cpp Map2XUtility)
map2x_add_test(ConfigurationTest ConfigurationTest.h ConfigurationTest.cpp Map2XCore)

# Compiled-in resource test
map2x_add_resource(ResourceTestData test
    ResourceTestFiles/predisposition.bin ALIAS predisposition.bin
    ResourceTestFiles/consequence.bin ALIAS consequence.bin
    )
set(ResourceTest_MOC ResourceTest.h)
set(ResourceTest_SRCS ResourceTest.cpp ${ResourceTestData})
map2x_add_multifile_test(ResourceTest ResourceTest_MOC ResourceTest_SRCS Map2XUtility)

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
set(ResourceTestStatic_SRCS ${ResourceTest_SRCS} registerStaticResource)
map2x_add_multifile_test(ResourceTestStatic ResourceTest_MOC ResourceTestStatic_SRCS Map2XUtility ResourceTestDataLib)
