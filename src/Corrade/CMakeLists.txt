#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "Corrade")

if(NOT CORRADE_BUILD_STATIC)
    set(SHARED_OR_STATIC SHARED)
else()
    set(SHARED_OR_STATIC STATIC)
endif()

# Generate configure header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# Generate version header. If Git is found and this is a Git working copy,
# extract values from there, otherwise use just CORRADE_VERSION_YEAR/MONTH that
# are set in project root CMakeLists.
find_package(Git)
if(Git_FOUND)
    # Match only tags starting with `v`, always use the long format so we have
    # a commit hash also on a tagged version
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "v*" --long
        OUTPUT_VARIABLE CORRADE_VERSION_STRING
        RESULT_VARIABLE _CORRADE_VERSION_RESULT
        ERROR_VARIABLE _CORRADE_VERSION_ERROR
        # Otherwise this gets executed in the build dir, which might be inside
        # a totally different Git working copy. But that's not enough, if it
        # would be run in ${CMAKE_CURRENT_SOURCE_DIR}, on Windows the
        # `--match "v*"` would get expanded to the `version.h.cmake` file,
        # causing Git to obviously not find any such tag. The reason is
        # probably that on Windows the wildcard expansion is done on the
        # application side and not by the shell, thus being performed even
        # though CMake docs say `execute_process()` doesn't involve a shell.
        # The package/git directory is thus dedicated for that operation,
        # *guaranteed* to never contain any file starting with `v` (or `V` for
        # that matter because, again, HELLO WINDOWS).
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/package/git
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(CORRADE_VERSION_STRING MATCHES "^v([0-9][0-9][0-9][0-9])\\.0?([0-9][0-9]?)-([0-9]+)-g([a-f0-9]+)$")
        set(CORRADE_VERSION_YEAR ${CMAKE_MATCH_1})
        set(CORRADE_VERSION_MONTH ${CMAKE_MATCH_2})
        set(CORRADE_VERSION_COMMIT ${CMAKE_MATCH_3})
        # Otherwise if commit is 0, it would #undef the variable
        set(CORRADE_VERSION_COMMIT " ${CORRADE_VERSION_COMMIT}")
        set(CORRADE_VERSION_HASH ${CMAKE_MATCH_4})
    elseif(_CORRADE_VERSION_RESULT EQUAL 0)
        message(WARNING "Can't match Git version from ${CORRADE_VERSION_STRING}, generating a fallback version.h instead")
    else()
        message(WARNING "Can't match Git version, generating a fallback version.h instead: ${_CORRADE_VERSION_ERROR}")
    endif()
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set(Corrade_HEADERS
    Corrade.h
    Cpu.h
    Tags.h)

# Force IDEs to display all header files in project view
add_custom_target(Corrade SOURCES ${Corrade_HEADERS})

install(FILES ${Corrade_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/configure.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

if(CORRADE_WITH_UTILITY) # Cyclic dependency of Containers and Utility
    add_subdirectory(Containers)
endif()

if(CORRADE_WITH_INTERCONNECT)
    add_subdirectory(Interconnect)
endif()

if(CORRADE_WITH_PLUGINMANAGER)
    add_subdirectory(PluginManager)
endif()

if(CORRADE_WITH_TESTSUITE)
    add_subdirectory(TestSuite)
endif()

if(CORRADE_WITH_UTILITY OR CORRADE_WITH_RC)
    add_subdirectory(Utility)
endif()

if(CORRADE_BUILD_TESTS)
    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

if(CORRADE_WITH_MAIN)
    if(CORRADE_TARGET_WINDOWS)
        add_library(CorradeMain STATIC CorradeMain.cpp)
        target_include_directories(CorradeMain PUBLIC
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_BINARY_DIR}/src)
        set_property(TARGET CorradeMain PROPERTY
            INTERFACE_CORRADE_CXX_STANDARD 11) # TODO: interface, really?
        set_property(TARGET CorradeMain APPEND PROPERTY
            COMPATIBLE_INTERFACE_NUMBER_MAX CORRADE_CXX_STANDARD)
        set_target_properties(CorradeMain PROPERTIES DEBUG_POSTFIX "-d")
        if(NOT MINGW)
            # Abusing INTERFACE_LINK_LIBRARIES because INTERFACE_LINK_OPTIONS
            # is only since 3.13. They treat things with `-` in front as linker
            # flags and fortunately I can use `-ENTRY` instead of `/ENTRY`.
            # https://gitlab.kitware.com/cmake/cmake/issues/16543
            set_property(TARGET CorradeMain APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "-ENTRY:$<$<NOT:$<BOOL:$<TARGET_PROPERTY:WIN32_EXECUTABLE>>>:wmainCRTStartup>$<$<BOOL:$<TARGET_PROPERTY:WIN32_EXECUTABLE>>:wWinMainCRTStartup>")
        else()
            set_property(TARGET CorradeMain APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "-municode")
        endif()
        install(TARGETS CorradeMain
            RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
            LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
            ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})

    # On sane system it's just a dummy interface target
    else()
        add_library(CorradeMain INTERFACE)
        target_include_directories(CorradeMain INTERFACE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_BINARY_DIR}/src)
    endif()

    # Corrade::Main target alias for superprojects
    add_library(Corrade::Main ALIAS CorradeMain)
endif()

# Corrade configure file for superprojects
set(_CORRADE_CONFIGURE_FILE ${CMAKE_CURRENT_BINARY_DIR}/configure.h CACHE INTERNAL "")
