find_package(Qt4 REQUIRED)

enable_testing()

set(UTILITY_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DirectoryTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DirectoryTestConfigure.h)

include_directories(${Map2XCore_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

qt4_wrap_cpp(Map2X_Utility_DirectoryTest_MOC DirectoryTest.h)
qt4_wrap_cpp(Map2X_Utility_EndianTest_MOC EndianTest.h)

add_executable(DirectoryTest DirectoryTest.cpp ${Map2X_Utility_DirectoryTest_MOC})
add_executable(EndianTest EndianTest.cpp ${Map2X_Utility_EndianTest_MOC})

target_link_libraries(DirectoryTest Map2XUtility ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
target_link_libraries(EndianTest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

add_test(DirectoryTest DirectoryTest)
add_test(EndianTest EndianTest)
