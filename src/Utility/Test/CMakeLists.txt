set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/testConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(UtilityAbstractHashTest AbstractHashTest.cpp)
corrade_add_test(UtilityDirectoryTest DirectoryTest.cpp)
corrade_add_test(UtilityEndianTest EndianTest.cpp)
corrade_add_test(UtilityMurmurHash2Test MurmurHash2Test.cpp)
corrade_add_test(UtilityUtilitiesTest UtilitiesTest.cpp)
corrade_add_test(UtilityConfigurationTest ConfigurationTest.cpp)
corrade_add_test(UtilityDebugTest DebugTest.cpp)
corrade_add_test(UtilitySha1Test Sha1Test.cpp)
corrade_add_test(UtilityStringTest StringTest.cpp)
corrade_add_test(UtilityTranslatorTest TranslatorTest.cpp)

# Compiled-in resource test
corrade_add_resource(ResourceTestData test
    ResourceTestFiles/predisposition.bin ALIAS predisposition.bin
    ResourceTestFiles/consequence.bin ALIAS consequence.bin
    )
corrade_add_test(UtilityResourceTest ResourceTest.cpp ${ResourceTestData})

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
corrade_add_test(UtilityResourceTestStatic
    ResourceTest.cpp ${ResourceTestData} registerStaticResource.cpp
    LIBRARIES ResourceTestDataLib)
