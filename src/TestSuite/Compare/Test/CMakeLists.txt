set(FILETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FileTestFiles)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(TestSuiteCompareContainerTest ContainerTest.cpp)
corrade_add_test(TestSuiteCompareFileTest FileTest.cpp)
corrade_add_test(TestSuiteCompareFileToStringTest FileToStringTest.cpp)
corrade_add_test(TestSuiteCompareFloatingPointTest FloatingPointTest.cpp)
corrade_add_test(TestSuiteCompareStringToFileTest StringToFileTest.cpp)
