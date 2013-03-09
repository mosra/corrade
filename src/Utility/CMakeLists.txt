include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CorradeUtility_SRCS
    Debug.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationGroup.cpp
    ConfigurationValue.cpp
    MurmurHash2.cpp
    Resource.cpp
    Sha1.cpp
    String.cpp
    Translator.cpp
    TypeTraits.cpp
    Unicode.cpp
    utilities.cpp)

set(CorradeUtility_HEADERS
    AbstractHash.h
    Assert.h
    Configuration.h
    ConfigurationGroup.h
    ConfigurationValue.h
    Debug.h
    Directory.h
    Endianness.h
    MurmurHash2.h
    Resource.h
    Sha1.h
    String.h
    Translator.h
    TypeTraits.h
    Unicode.h
    utilities.h

    corradeUtilityVisibility.h)

add_library(CorradeUtility SHARED ${CorradeUtility_SRCS})
set_target_properties(CorradeUtility PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})

install(TARGETS CorradeUtility DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeUtility_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Utility)

if(NOT CMAKE_CROSSCOMPILING)
    add_executable(corrade-rc rc.cpp)
    target_link_libraries(corrade-rc CorradeUtility)
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})
endif()

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
