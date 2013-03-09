set(CorradeTestSuiteCompare_HEADERS
    Container.h
    File.h
    FileToString.h
    FloatingPoint.h
    StringToFile.h)

install(FILES ${CorradeTestSuiteCompare_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/TestSuite/Compare)

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
