
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

# Already included, nothing to do
if(_CORRADE_USE_INCLUDED)
    return()
endif()

# Check compiler version
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Don't allow to use compilers older than what compatibility mode allows
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.4.0")
        message(FATAL_ERROR "Corrade cannot be used with GCC 4.3")
    endif()
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.5.0" AND NOT CORRADE_GCC44_COMPATIBILITY)
        message(FATAL_ERROR "To use Corrade with GCC 4.4, build it with GCC44_COMPATIBILITY enabled")
    endif()
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.6.0" AND NOT CORRADE_GCC45_COMPATIBILITY)
        message(FATAL_ERROR "To use Corrade with GCC 4.5, build it with GCC45_COMPATIBILITY enabled")
    endif()
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7.0" AND NOT CORRADE_GCC46_COMPATIBILITY)
        message(FATAL_ERROR "To use Corrade with GCC 4.6, build it with GCC46_COMPATIBILITY enabled")
    endif()
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8.1" AND NOT CORRADE_GCC47_COMPATIBILITY)
        message(FATAL_ERROR "To use Corrade with GCC 4.7, build it with GCC47_COMPATIBILITY enabled")
    endif()

    # Don't allow to use compiler newer than what compatibility mode allows
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8.1" AND CORRADE_GCC47_COMPATIBILITY)
        message(FATAL_ERROR "GCC >=4.8.1 cannot be used if Corrade is built with GCC47_COMPATIBILITY")
    endif()
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7.0" AND CORRADE_GCC46_COMPATIBILITY)
        message(FATAL_ERROR "GCC >=4.7 cannot be used if Corrade is built with GCC46_COMPATIBILITY")
    endif()
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.6.0" AND CORRADE_GCC45_COMPATIBILITY)
        message(FATAL_ERROR "GCC >=4.6 cannot be used if Corrade is built with GCC45_COMPATIBILITY")
    endif()
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.5.0" AND CORRADE_GCC44_COMPATIBILITY)
        message(FATAL_ERROR "GCC >=4.5 cannot be used if Corrade is built with GCC44_COMPATIBILITY")
    endif()
endif()

# Mandatory C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
endif()

# Optional C++ flags
set(CORRADE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Winit-self -Werror=return-type -Wmissing-declarations -pedantic -fvisibility=hidden")

# Some flags are not yet supported everywhere
# TODO: do this with check_c_compiler_flags()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7.0")
        set(CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS} -Wzero-as-null-pointer-constant")
    endif()
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.6.0")
        set(CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS} -Wdouble-promotion")
    endif()
endif()

# Disable `-pedantic` for GCC 4.4.3 on NaCl to avoid excessive warnings about
# "comma at the end of enumeration list". My own GCC 4.4.7 doesn't emit these
# warnings.
if(CORRADE_GCC44_COMPATIBILITY AND CORRADE_TARGET_NACL)
    string(REPLACE "-pedantic" "" CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS}")
endif()

function(corrade_add_test test_name)
    # Get DLL and path lists
    foreach(arg ${ARGN})
        if(${arg} STREQUAL LIBRARIES)
            set(__DOING_LIBRARIES ON)
        else()
            if(__DOING_LIBRARIES)
                set(libraries ${libraries} ${arg})
            else()
                set(sources ${sources} ${arg})
            endif()
        endif()
    endforeach()

    add_executable(${test_name} ${sources})
    target_link_libraries(${test_name} ${libraries} ${CORRADE_TESTSUITE_LIBRARIES})
    if(CORRADE_TARGET_EMSCRIPTEN)
        find_package(NodeJs REQUIRED)
        add_test(NAME ${test_name} COMMAND ${NODEJS_EXECUTABLE} $<TARGET_FILE:${test_name}>)
    else()
        add_test(${test_name} ${test_name})
    endif()
endfunction()

function(corrade_add_resource name configurationFile)
    # TODO: remove this when backward compatibility is non-issue
    list(LENGTH ARGN list_length)
    if(NOT ${list_length} EQUAL 0)
        message(FATAL_ERROR "Superfluous arguments to corrade_add_resource():" ${ARGN})
    endif()

    # Add the file as dependency, parse more dependencies from the file
    set(dependencies "${configurationFile}")
    set(filenameRegex "^[ \t]*filename[ \t]*=[ \t]*\"?([^\"]+)\"?[ \t]*$")
    get_filename_component(configurationFilePath ${configurationFile} PATH)
    file(STRINGS "${configurationFile}" files REGEX ${filenameRegex})
    foreach(file ${files})
        string(REGEX REPLACE ${filenameRegex} "\\1" filename "${file}")
        if(NOT IS_ABSOLUTE "${filename}" AND configurationFilePath)
            set(filename "${configurationFilePath}/${filename}")
        endif()
        list(APPEND dependencies "${filename}")
    endforeach()

    # Run command
    set(out "${CMAKE_CURRENT_BINARY_DIR}/resource_${name}.cpp")
    add_custom_command(
        OUTPUT "${out}"
        COMMAND "${CORRADE_RC_EXECUTABLE}" ${name} "${configurationFile}" "${out}"
        DEPENDS "${CORRADE_RC_EXECUTABLE}" ${dependencies}
        COMMENT "Compiling data resource file ${out}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    # Save output filename
    set(${name} "${out}" PARENT_SCOPE)
endfunction()

function(corrade_add_plugin plugin_name install_dir metadata_file)
    # Create dynamic library
    if(WIN32)
        add_library(${plugin_name} SHARED ${ARGN})
    #elseif(APPLE)
    #    add_library(${plugin_name} SHARED ${ARGN})
    else()
        add_library(${plugin_name} MODULE ${ARGN})
    endif()

    # Plugins doesn't have any prefix (e.g. 'lib' on Linux)
    set_target_properties(${plugin_name} PROPERTIES
        PREFIX ""
        COMPILE_FLAGS -DCORRADE_DYNAMIC_PLUGIN)

    # Enable incremental linking on the Mac OS X
    if(APPLE)
        set_target_properties(${plugin_name} PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup")
    endif()

    # Copy metadata next to the binary for testing purposes or install it both
    # somewhere
    if(${install_dir} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
        add_custom_command(
            OUTPUT ${plugin_name}.conf
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} ${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.conf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file})
        add_custom_target(${plugin_name}-metadata ALL DEPENDS ${plugin_name}.conf)
    else()
        install(TARGETS ${plugin_name} DESTINATION "${install_dir}")
        install(FILES ${metadata_file} DESTINATION "${install_dir}" RENAME "${plugin_name}.conf")
    endif()
endfunction()

function(corrade_add_static_plugin plugin_name install_dir metadata_file)
    # Compile resources
    set(resource_file "${CMAKE_CURRENT_BINARY_DIR}/resources_${plugin_name}.conf")
    file(WRITE "${resource_file}" "group=CorradeStaticPlugin_${plugin_name}\n[file]\nfilename=\"${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file}\"\nalias=${plugin_name}.conf")
    corrade_add_resource(${plugin_name} "${resource_file}")

    # Create static library
    add_library(${plugin_name} STATIC ${ARGN} ${${plugin_name}})
    set_target_properties(${plugin_name} PROPERTIES COMPILE_FLAGS "-DCORRADE_STATIC_PLUGIN ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")

    # Install, if not into the same place
    if(NOT ${install_dir} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
        install(TARGETS ${plugin_name} DESTINATION "${install_dir}")
    endif()
endfunction()

set(_CORRADE_USE_INCLUDED TRUE)
