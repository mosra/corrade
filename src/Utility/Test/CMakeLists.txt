set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTILITY_BINARY_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/testConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test2(AbstractHashTest AbstractHashTest.cpp LIBRARIES CorradeUtility)
corrade_add_test(DirectoryTest DirectoryTest.h DirectoryTest.cpp CorradeUtility)
corrade_add_test(EndianTest EndianTest.h EndianTest.cpp CorradeUtility)
corrade_add_test2(MurmurHash2Test MurmurHash2Test.cpp LIBRARIES CorradeUtility)
corrade_add_test2(UtilitiesTest UtilitiesTest.cpp LIBRARIES CorradeUtility)
corrade_add_test(ConfigurationTest ConfigurationTest.h ConfigurationTest.cpp CorradeUtility)
corrade_add_test(DebugTest DebugTest.h DebugTest.cpp CorradeUtility)
corrade_add_test(Sha1Test Sha1Test.h Sha1Test.cpp CorradeUtility)
corrade_add_test2(StringTest StringTest.cpp LIBRARIES CorradeUtility)
corrade_add_test(TranslatorTest TranslatorTest.h TranslatorTest.cpp CorradeUtility)

# Compiled-in resource test
corrade_add_resource(ResourceTestData test
    ResourceTestFiles/predisposition.bin ALIAS predisposition.bin
    ResourceTestFiles/consequence.bin ALIAS consequence.bin
    )
set(ResourceTest_MOC ResourceTest.h)
set(ResourceTest_SRCS ResourceTest.cpp ${ResourceTestData})
corrade_add_multifile_test(ResourceTest ResourceTest_MOC ResourceTest_SRCS CorradeUtility)

# Static lib resource test
add_library(ResourceTestDataLib STATIC ${ResourceTestData})
set(ResourceTestStatic_SRCS ${ResourceTest_SRCS} registerStaticResource)
corrade_add_multifile_test(ResourceTestStatic ResourceTest_MOC ResourceTestStatic_SRCS CorradeUtility ResourceTestDataLib)
