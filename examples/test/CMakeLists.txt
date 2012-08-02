find_package(Corrade REQUIRED)

include_directories(${CORRADE_INCLUDE_DIR})

add_executable(mytest MyTest.cpp)
target_link_libraries(mytest ${CORRADE_UTILITY_LIBRARY})
