include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CorradeUtility_SRCS
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
