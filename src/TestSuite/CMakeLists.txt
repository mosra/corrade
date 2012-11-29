add_subdirectory(Compare)

set(CorradeTestSuite_SRCS
    Compare/File.cpp
    Compare/FileToString.cpp)

add_library(CorradeTestSuite SHARED ${CorradeTestSuite_SRCS})
set_target_properties(CorradeTestSuite PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})

install(TARGETS CorradeTestSuite DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
