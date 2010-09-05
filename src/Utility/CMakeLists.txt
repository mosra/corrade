include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(TestBigEndian)
test_big_endian(ENDIANNESS_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EndiannessConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EndiannessConfigure.h)

set(Map2X_Utility_SRCS
    Endianness.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationGroup.cpp
    Resource.cpp
    utilities.cpp
)

add_library(Map2XUtility STATIC ${Map2X_Utility_SRCS})

if(USE_FPIC)
    set_target_properties(Map2XUtility PROPERTIES COMPILE_FLAGS -fPIC)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
