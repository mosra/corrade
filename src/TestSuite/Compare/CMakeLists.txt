set(CorradeTestSuiteCompare_HEADERS
    Container.h
    File.h
    FileToString.h
    StringToFile.h)

install(FILES ${CorradeTestSuiteCompare_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/TestSuite/Compare)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
