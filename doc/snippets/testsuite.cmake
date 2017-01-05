# [0]
if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(JPEG_TEST_DIR ".")
else()
    set(JPEG_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Contains just
#  #define JPEG_TEST_DIR "${JPEG_TEST_DIR}"
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

corrade_add_test(JpegTest JpegTest.cpp
    LIBRARIES ${JPEG_LIBRARIES}
    FILES rgb.jpg rgba.jpg grayscale.jpg)
target_include_directories(JpegTest ${CMAKE_CURRENT_BINARY_DIR})
# [0]
