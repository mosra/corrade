cmake_minimum_required(VERSION 2.6)

project(map2x-lib)

add_subdirectory(src)

if(BUILD_TESTS)
    find_package(Qt4)

    if(QT4_FOUND)
        add_subdirectory(tests)
    elseif()
        message(WARNING "Qt4 is required for building unit tests. No tests will be build.")
    endif()

endif()
