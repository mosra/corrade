include_directories(${Map2XCore_BINARY_DIR}/src)

include(TestBigEndian)
test_big_endian(ENDIANNESS_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EndiannessConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EndiannessConfigure.h)

set(Map2X_Utility_SRCS
    Directory.cpp
    utilities.cpp
)

add_library(Map2XUtility STATIC ${Map2X_Utility_SRCS})

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
