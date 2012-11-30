set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/testConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test2(UtilityAbstractHashTest AbstractHashTest.cpp)
corrade_add_test(UtilityDirectoryTest DirectoryTest.h DirectoryTest.cpp CorradeUtility)
corrade_add_test(UtilityEndianTest EndianTest.h EndianTest.cpp CorradeUtility)
corrade_add_test2(UtilityMurmurHash2Test MurmurHash2Test.cpp)
corrade_add_test2(UtilityUtilitiesTest UtilitiesTest.cpp)
corrade_add_test2(UtilityConfigurationTest ConfigurationTest.cpp)
corrade_add_test(UtilityDebugTest DebugTest.h DebugTest.cpp CorradeUtility)
corrade_add_test(UtilitySha1Test Sha1Test.h Sha1Test.cpp CorradeUtility)
corrade_add_test2(UtilityStringTest StringTest.cpp)
corrade_add_test2(UtilityTranslatorTest TranslatorTest.cpp)

# Compiled-in resource test
corrade_add_resource(ResourceTestData test
    ResourceTestFiles/predisposition.bin ALIAS predisposition.bin
    ResourceTestFiles/consequence.bin ALIAS consequence.bin
    )
set(ResourceTest_MOC ResourceTest.h)
set(ResourceTest_SRCS ResourceTest.cpp ${ResourceTestData})
corrade_add_multifile_test(UtilityResourceTest ResourceTest_MOC ResourceTest_SRCS CorradeUtility)

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
set(ResourceTestStatic_SRCS ${ResourceTest_SRCS} registerStaticResource)
corrade_add_multifile_test(UtilityResourceTestStatic ResourceTest_MOC ResourceTestStatic_SRCS CorradeUtility ResourceTestDataLib)
