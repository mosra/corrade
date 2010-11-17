include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(TestBigEndian)
test_big_endian(ENDIANNESS_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EndiannessConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EndiannessConfigure.h)

set(Kompas_Utility_SRCS
    Endianness.cpp
    Debug.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationGroup.cpp
    Resource.cpp
    Translator.cpp
    utilities.cpp
)

add_library(KompasUtility SHARED ${Kompas_Utility_SRCS})

if(WIN32)
    set_target_properties(KompasUtility PROPERTIES COMPILE_FLAGS -DUTILITY_EXPORTING)
endif()

add_executable(kompas-rc rc.cpp)
target_link_libraries(kompas-rc KompasUtility)

if(USE_FPIC)
    set_target_properties(KompasUtility PROPERTIES COMPILE_FLAGS -fPIC)
endif()

install(TARGETS KompasUtility DESTINATION ${KOMPAS_LIBRARY_INSTALL_DIR})
install(TARGETS kompas-rc DESTINATION ${KOMPAS_BINARY_INSTALL_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
