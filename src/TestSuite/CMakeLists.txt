set(CorradeTestSuite_SRCS
    Tester.cpp

    Compare/File.cpp
    Compare/FileToString.cpp
    Compare/FloatingPoint.cpp
    Compare/StringToFile.cpp)

set(CorradeTestSuite_HEADERS
    Comparator.h
    Tester.h

    corradeTestSuiteVisibility.h)

add_library(CorradeTestSuite SHARED ${CorradeTestSuite_SRCS})
set_target_properties(CorradeTestSuite PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
target_link_libraries(CorradeTestSuite CorradeUtility)

install(TARGETS CorradeTestSuite DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeTestSuite_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/TestSuite)

add_subdirectory(Compare)

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
