include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(TestBigEndian)
test_big_endian(ENDIANNESS_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EndiannessConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/EndiannessConfigure.h)

set(CorradeUtility_SRCS
    Endianness.cpp
    Debug.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationGroup.cpp
    MurmurHash2.cpp
    Resource.cpp
    Sha1.cpp
    Translator.cpp
    TypeTraits.cpp
    utilities.cpp)

add_library(CorradeUtility SHARED ${CorradeUtility_SRCS})
set_target_properties(CorradeUtility PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})

install(TARGETS CorradeUtility DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})

if(NOT CMAKE_CROSSCOMPILING)
    add_executable(corrade-rc rc.cpp)
    target_link_libraries(corrade-rc CorradeUtility)
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
